//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#include "lomse_chord_engraver.h"

#include "lomse_im_note.h"
#include "lomse_shape_note.h"
#include "lomse_gm_basic.h"
#include "lomse_note_engraver.h"
#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_logger.h"

#include <cstdlib>      //abs
#include <stdexcept>
using namespace std;


namespace lomse
{


//=======================================================================================
// ClefEngraver implementation
//=======================================================================================
ChordEngraver::ChordEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                             int numNotes)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pChord(nullptr)
    , m_pBaseNoteData(nullptr)
    , m_fStemDown(false)
    , m_fCrossStaffChord(false)
    , m_fHasStem(false)
    , m_fHasFlag(false)
    , m_fSomeNoteReversed(false)
    , m_noteType(0)
    , m_stemWidth(0.0f)
    , m_numNotesMissing(numNotes)
{
}

//---------------------------------------------------------------------------------------
ChordEngraver::~ChordEngraver()
{
    std::list<ChordNoteData*>::iterator it;
    for (it = m_notes.begin(); it != m_notes.end(); ++it)
        delete *it;
    m_notes.clear();
}

//---------------------------------------------------------------------------------------
void ChordEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                       GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                       int UNUSED(iSystem), int UNUSED(iCol),
                                       LUnits UNUSED(xStaffLeft), LUnits UNUSED(xStaffRight),
                                       LUnits UNUSED(yTop),
                                       int idxStaff, VerticalProfile* UNUSED(pVProfile))
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_idxStaff = idxStaff;
    m_pChord = dynamic_cast<ImoChord*>(pRO);

    add_note(pSO, pStaffObjShape, idxStaff);
}

//---------------------------------------------------------------------------------------
void ChordEngraver::set_middle_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                        GmoShape* pStaffObjShape, int UNUSED(iInstr),
                                        int UNUSED(iStaff), int UNUSED(iSystem),
                                        int UNUSED(iCol), LUnits UNUSED(xStaffLeft),
                                        LUnits UNUSED(xStaffRight), LUnits UNUSED(yTop),
                                        int idxStaff, VerticalProfile* UNUSED(pVProfile))
{
    add_note(pSO, pStaffObjShape, idxStaff);
}

//---------------------------------------------------------------------------------------
void ChordEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                     GmoShape* pStaffObjShape, int UNUSED(iInstr),
                                     int UNUSED(iStaff), int UNUSED(iSystem),
                                     int UNUSED(iCol), LUnits UNUSED(xStaffLeft),
                                     LUnits UNUSED(xStaffRight), LUnits UNUSED(yTop),
                                     int idxStaff, VerticalProfile* UNUSED(pVProfile))
{
    add_note(pSO, pStaffObjShape, idxStaff);
    if (m_numNotesMissing != 0)
    {
        LOMSE_LOG_ERROR("[ChordEngraver::set_end_staffobj] Num added notes doesn't match expected notes in chord");
        throw runtime_error("[ChordEngraver::set_end_staffobj] Num added notes doesn't match expected notes in chord");
    }
}

//---------------------------------------------------------------------------------------
int ChordEngraver::create_shapes(Color color)
{
    m_color = color;
    decide_on_stem_direction();
    layout_noteheads();
    layout_accidentals();
    add_stem_and_flag();
    set_anchor_offset();
    return 0;
}

//---------------------------------------------------------------------------------------
void ChordEngraver::add_note(ImoStaffObj* pSO, GmoShape* pStaffObjShape, int idxStaff)
{
    m_numNotesMissing--;
    ImoNote* pNote = static_cast<ImoNote*>(pSO);
    GmoShapeNote* pNoteShape = static_cast<GmoShapeNote*>(pStaffObjShape);
    int posOnStaff = pNoteShape->get_pos_on_staff();

    if (m_notes.size() == 0)
    {
        ChordNoteData* pData = LOMSE_NEW ChordNoteData(pNote, pNoteShape, posOnStaff, m_iInstr);
	    m_notes.push_back(pData);
        m_pBaseNoteData = pData;
        m_stemWidth = tenths_to_logical(LOMSE_STEM_THICKNESS);
    }
    else
    {
        //keep notes sorted by pitch
        DiatonicPitch newPitch(pNote->get_step(), pNote->get_octave());
        std::list<ChordNoteData*>::iterator it;
        for (it = m_notes.begin(); it != m_notes.end(); ++it)
        {
            DiatonicPitch curPitch((*it)->pNote->get_step(), (*it)->pNote->get_octave());
            if (newPitch < curPitch)
            {
                ChordNoteData* pData =
                    LOMSE_NEW ChordNoteData(pNote, pNoteShape, posOnStaff, m_iInstr);
	            m_notes.insert(it, 1, pData);

                m_fCrossStaffChord |= (m_idxStaff != idxStaff);
                return;
            }
        }
        ChordNoteData* pData = LOMSE_NEW ChordNoteData(pNote, pNoteShape, posOnStaff, m_iInstr);
	    m_notes.push_back(pData);

	    m_fCrossStaffChord |= (m_idxStaff != idxStaff);
    }
}

//---------------------------------------------------------------------------------------
void ChordEngraver::decide_on_stem_direction()
{
    //  Rules (taken from ref. [2] www.coloradocollege.edu)
    //
    //  a) Two notes in chord:
    //    a1. If the interval above the middle line is greater than the interval below
    //      the middle line: downward stems. i.e. (a4,d5) (f4,f5) (a4,g5)
    //      ==>   (MaxNotePos + MinNotePos)/2 > MiddleLinePos
    //
    //    a2. If the interval below the middle line is greater than the interval above
    //      the middle line: upward stems. i.e. (e4,c5)(g4,c5)(d4,e5)
    //
    //    a3. If the two notes are at the same distance from the middle line: stem can
    //      go in either direction, but most engravers prefer downward stems.
    //      i.e. (g4.d5)(a4,c5)
    //
    //
    //  b) More than two notes in chord:
    //
    //    b1. If the interval of the highest note above the middle line is greater than
    //      the interval of the lowest note below the middle line: downward stems.
    //      ==>   same than a1
    //
    //    b2. If the interval of the lowest note below the middle line is greater than
    //      the interval of the highest note above the middle line: upward stems.
    //      ==>   same than a2
    //
    //    b3. If the highest and the lowest notes are the same distance from the middle
    //      line use the majority rule to determine stem direction: If the majority of
    //      the notes are above the middle: downward stems. Else: upward stems.
    //      ==>   Mean(NotePos) > MiddleLinePos -> downward
    //
    //  Additional rules (mine):
    //  c) chords without stem (notes longer than half notes):
    //    c1. layout as if stem was up

    ImoNote* pBaseNote = get_base_note();
    m_noteType = pBaseNote->get_note_type();
    int stemType = pBaseNote->get_stem_direction();

    m_fHasStem = m_noteType >= k_half
                 && stemType != k_stem_none;
    m_fHasFlag = m_fHasStem && m_noteType > k_quarter
                 && !is_chord_beamed();


    if (m_noteType < k_half)
        m_fStemDown = false;                    //c1. layout as if stem up

    else if (stemType == k_stem_up)
        m_fStemDown = false;                    //force stem up

    else if (stemType == k_stem_down)
        m_fStemDown = true;                     //force stem down

    else if (stemType == k_stem_none)
        m_fStemDown = false;                    //c1. layout as if stem up

    else if (stemType == k_stem_default)     //as decided by program
    {
        //majority rule
        int weight = 0;
        std::list<ChordNoteData*>::iterator it;
        for(it=m_notes.begin(); it != m_notes.end(); ++it)
            weight += (*it)->posOnStaff;

        m_fStemDown = ( weight >= 6 * int(m_notes.size()) );
    }
}

//---------------------------------------------------------------------------------------
bool ChordEngraver::is_chord_beamed()
{
    return get_base_note()->is_beamed();
}

//---------------------------------------------------------------------------------------
void ChordEngraver::layout_noteheads()
{
    align_noteheads();
    arrange_notheads_to_avoid_collisions();
}

//---------------------------------------------------------------------------------------
void ChordEngraver::align_noteheads()
{
    LUnits maxLeft = 0.0f;
    std::list<ChordNoteData*>::iterator it;
    for (it = m_notes.begin(); it != m_notes.end(); ++it)
        maxLeft = max(maxLeft, (*it)->pNoteShape->get_notehead_left());

    for (it = m_notes.begin(); it != m_notes.end(); ++it)
    {
        LUnits xShift = maxLeft - (*it)->pNoteShape->get_notehead_left();
        if (xShift > 0.0f)
        {
            USize shift(xShift, 0.0f);
            (*it)->pNoteShape->shift_origin(shift);
        }
    }

}

//---------------------------------------------------------------------------------------
void ChordEngraver::arrange_notheads_to_avoid_collisions()
{
    //Arrange noteheads at left/right of stem to avoid collisions
    //The algorithm asumes that the stem direction has been computed and that
    //notes are already sorted by pitch. The algorithm is simple:
    // 1. If the stem goes down, start with the highest note and go downwards. Else
    //    start with the lowes note and go upwards.
    // 2. Place each note on the normal side of the stem, unless the interval with the
    //    previous note is a second and previous not is not reversed.

    int nPosPrev = 1000;    // a very high number not posible in real world
    m_fSomeNoteReversed = false;
    if (m_fStemDown)
    {
        std::list<ChordNoteData*>::reverse_iterator it;
        for (it = m_notes.rbegin(); it != m_notes.rend(); ++it)
        {
		    //do processing
		    int pos = (*it)->posOnStaff;
		    if (abs(nPosPrev - pos) < 2)
		    {
			    //collision. Reverse position of this notehead
			    (*it)->fNoteheadReversed = true;
			    m_fSomeNoteReversed = true;
			    reverse_notehead((*it)->pNoteShape);
			    nPosPrev = 1000;
		    }
		    else
			    nPosPrev = pos;
        }
    }
    else
    {
        std::list<ChordNoteData*>::iterator it;
        for (it = m_notes.begin(); it != m_notes.end(); ++it)
        {
		    //do processing
		    int pos = (*it)->posOnStaff;
		    if (abs(nPosPrev - pos) < 2)
		    {
			    //collision. Reverse position of this notehead
			    (*it)->fNoteheadReversed = true;
			    m_fSomeNoteReversed = true;
			    reverse_notehead((*it)->pNoteShape);
			    nPosPrev = 1000;
		    }
		    else
			    nPosPrev = pos;
        }
    }
}

//---------------------------------------------------------------------------------------
void ChordEngraver::reverse_notehead(GmoShapeNote* pNoteShape)
{
    LUnits xShift;
    if (is_stem_up())
        xShift = pNoteShape->get_notehead_width() - m_stemWidth;
    else
        xShift =  -pNoteShape->get_notehead_width() + m_stemWidth;

    USize shift(xShift, 0.0f);
    pNoteShape->shift_origin(shift);
}

//---------------------------------------------------------------------------------------
void ChordEngraver::layout_accidentals()
{
    std::list<ChordNoteData*>::reverse_iterator it;
    for(it = m_notes.rbegin(); it != m_notes.rend(); ++it)
    {
        GmoShapeNote* pNoteShape = (*it)->pNoteShape;
        GmoShapeAccidentals* pCurAcc = pNoteShape->get_accidentals_shape();
        pNoteShape->unlock();

        if (pCurAcc)
        {
            //check if conflict with next two noteheads
            std::list<ChordNoteData*>::reverse_iterator itCur = it;
            ++itCur;
            if (itCur != m_notes.rend())
            {
                GmoShapeNotehead* pHead = (*itCur)->pNoteShape->get_notehead_shape();
                shift_acc_if_confict_with_shape(pCurAcc, pHead);
                ++itCur;
            }
            if (itCur != m_notes.rend())
            {
                GmoShapeNotehead* pHead = (*itCur)->pNoteShape->get_notehead_shape();
                shift_acc_if_confict_with_shape(pCurAcc, pHead);
            }

            //check if conflict with two previous noteheads
            if (it != m_notes.rbegin())
            {
                itCur = it;
                --itCur;
                GmoShapeNotehead* pHead = (*itCur)->pNoteShape->get_notehead_shape();
                shift_acc_if_confict_with_shape(pCurAcc, pHead);
                if (itCur != m_notes.rbegin())
                {
                    --itCur;
                    GmoShapeNotehead* pHead = (*itCur)->pNoteShape->get_notehead_shape();
                    shift_acc_if_confict_with_shape(pCurAcc, pHead);
                }
            }

            //check if conflict with any previous accidental
            shift_accidental_if_conflict_with_previous(pCurAcc, it);
        }
    }
}

//---------------------------------------------------------------------------------------
void ChordEngraver::shift_accidental_if_conflict_with_previous(
                                GmoShapeAccidentals* pCurAcc,
                                std::list<ChordNoteData*>::reverse_iterator& itCur)
{
	std::list<ChordNoteData*>::reverse_iterator it;
	for(it = m_notes.rbegin(); it != itCur; ++it)
	{
		GmoShapeNote* pNoteShape = (*it)->pNoteShape;
		GmoShapeAccidentals* pPrevAcc = pNoteShape->get_accidentals_shape();
        if (pPrevAcc)
            shift_acc_if_confict_with_shape(pCurAcc, pPrevAcc);
	}
}

//---------------------------------------------------------------------------------------
void ChordEngraver::shift_acc_if_confict_with_shape(GmoShapeAccidentals* pCurAcc,
                                                    GmoShape* pShape)
{
    LUnits xOverlap = check_if_overlap(pShape, pCurAcc);
    if (xOverlap > 0.0f)
    {
        LUnits space = tenths_to_logical(LOMSE_SPACE_BETWEEN_ACCIDENTALS);
        LUnits shift = pCurAcc->get_right() - pShape->get_left();
        xOverlap = - (shift + space);
        if (xOverlap != 0.0f)
        {
            USize shift(xOverlap, 0.0f);
            pCurAcc->shift_origin(shift);
        }
    }
}

//---------------------------------------------------------------------------------------
void ChordEngraver::set_anchor_offset()
{
	std::list<ChordNoteData*>::iterator it;
	for(it = m_notes.begin(); it != m_notes.end(); ++it)
	{
		GmoShapeNote* pNoteShape = (*it)->pNoteShape;
		pNoteShape->lock();

        //compute anchor pos
        LUnits offset;
        if (!m_fSomeNoteReversed)
            //a) if no reversed noteads, anchor is notehead x left. Stem direction
            //   doesn't matter.
            offset = pNoteShape->get_notehead_left();

        else
        {
            //b) At least a note with reversed notehead:
            if (is_stem_down())
            {
                //b.1) if stem goes down:
                if ((*it)->fNoteheadReversed)
                    //b.1.1) if note is reversed, anchor is x left of notehead.
                    offset = pNoteShape->get_notehead_left();
                else
                    //b.1.2) if note is not reversed, anchor is x left of notehead plus
                    //       stem width minus notehead width.
                    offset = pNoteShape->get_notehead_left() + m_stemWidth
                             - pNoteShape->get_notehead_width();
            }
            else
            {
                //b.2) if stem goes up:
                if (!(*it)->fNoteheadReversed)
                    //b.2.1) if note is not reversed, anchor is x left of notehead.
                    offset = pNoteShape->get_notehead_left();
                else
                    //b.2.2) if note is reversed, anchor is x left of notehead plus
                    //       stem width minus notehead width.
                    offset = pNoteShape->get_notehead_left() + m_stemWidth
                             - pNoteShape->get_notehead_width();
            }
        }
        offset = pNoteShape->get_origin().x - offset;

        if (offset != 0.0f)
            pNoteShape->set_anchor_offset(offset);
	}
}

//---------------------------------------------------------------------------------------
LUnits ChordEngraver::check_if_overlap(GmoShape* pShape, GmoShape* pNewShape)
{
    URect overlap = pShape->get_bounds();
    overlap.intersection( pNewShape->get_bounds() );
    return overlap.get_width();
}

//---------------------------------------------------------------------------------------
LUnits ChordEngraver::check_if_accidentals_overlap(GmoShapeAccidentals* pPrevAcc,
                                                   GmoShapeAccidentals* pCurAcc)
{
    URect overlap = pPrevAcc->get_bounds();
    overlap.intersection( pCurAcc->get_bounds() );
    return overlap.get_width();
}

//---------------------------------------------------------------------------------------
void ChordEngraver::add_stem_and_flag()
{
    //  Rules (taken from ref. [1] Music Publishers' Association)
    //
    //  p.3, b) ... When there is more than one note head on a stem,as in a chord, the
    //          stem length is calculated from the note closest to the end of the stem.

    if (!has_stem())
        return;

    //the stem length must be increased with the distance from min note to max note.
    GmoShapeNote* pMinNoteShape = m_notes.front()->pNoteShape;
    GmoShapeNote* pMaxNoteShape = m_notes.back()->pNoteShape;

    //stem and the flag is computed for max/min note, depending on stem direction
    ChordNoteData* pDataFlag = (is_stem_down() ? m_notes.front() : m_notes.back());
    ImoNote* pNoteFlag = pDataFlag->pNote;  //min note for stem down, max for stem up
    int instr = pDataFlag->iInstr;
    int staff = pNoteFlag->get_staff();
    int nPosOnStaff = pDataFlag->posOnStaff;

    //create the shape and attach it to notes
    Tenths length = NoteEngraver::get_standard_stem_length(nPosOnStaff, is_stem_down());
    if (!is_chord_beamed() && length < 35.0f && m_noteType > k_eighth)
        length = 35.0f;     // 3.5 spaces
    bool fShortFlag = (length < 35.0f);
    LUnits stemLength = tenths_to_logical(length);
    StemFlagEngraver engrv(m_libraryScope, m_pMeter, pNoteFlag, instr, staff);

    engrv.add_stem_flag_to_chord(pMinNoteShape, pMaxNoteShape, m_pBaseNoteData->pNoteShape,
                        m_noteType, is_stem_down(), has_flag(), fShortFlag,
                        m_fCrossStaffChord, stemLength, m_color);
}


}  //namespace lomse

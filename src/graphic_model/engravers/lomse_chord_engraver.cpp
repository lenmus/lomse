//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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
#include "lomse_staffobjs_table.h"
#include "lomse_staffobjs_cursor.h"

#include <cstdlib>      //abs
#include <stdexcept>
using namespace std;


namespace lomse
{


//=======================================================================================
// ChordEngraver implementation
//=======================================================================================
ChordEngraver::ChordEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                             int numNotes, double fontSize, int symbolSize)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pChord(nullptr)
    , m_pBaseNoteData(nullptr)
    , m_fStemDown(false)
    , m_fHasStem(false)
    , m_fHasFlag(false)
    , m_fSomeNoteReversed(false)
    , m_noteType(0)
    , m_stemWidth(0.0f)
    , m_numNotesMissing(numNotes)
    , m_fontSize(fontSize)
    , m_symbolSize(symbolSize)
    , m_fSomeAccidentalsShifted(false)
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
void ChordEngraver::set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc)
{
    m_iInstr = aoc.iInstr;
    m_iStaff = aoc.iStaff;
    m_idxStaff = aoc.idxStaff;

    m_pChord = dynamic_cast<ImoChord*>(pRO);

    add_note(aoc.pSO, aoc.pStaffObjShape, aoc.idxStaff);
}

//---------------------------------------------------------------------------------------
void ChordEngraver::set_middle_staffobj(ImoRelObj* UNUSED(pRO), const AuxObjContext& aoc)
{
    add_note(aoc.pSO, aoc.pStaffObjShape, aoc.idxStaff);
}

//---------------------------------------------------------------------------------------
void ChordEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), const AuxObjContext& aoc)
{
    add_note(aoc.pSO, aoc.pStaffObjShape, aoc.idxStaff);
    if (m_numNotesMissing != 0)
    {
        LOMSE_LOG_ERROR("Num added notes doesn't match expected notes in chord");
    }
}

//---------------------------------------------------------------------------------------
void ChordEngraver::save_applicable_clefs(StaffObjsCursor* pCursor, int iInstr)
{
    m_clefs = pCursor->get_applicable_clefs_for_instrument(iInstr);
}

//---------------------------------------------------------------------------------------
int ChordEngraver::create_shapes(Color color)
{
    m_color = color;
    decide_stem_direction();
    find_reference_notes();
    layout_noteheads();
    layout_accidentals();
    layout_arpeggio();
    add_stem_and_flag();
    set_anchor_offset();
    return 0;
}

//---------------------------------------------------------------------------------------
void ChordEngraver::add_note(ImoStaffObj* pSO, GmoShape* pStaffObjShape, int UNUSED(idxStaff))
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
                return;
            }
        }
        ChordNoteData* pData = LOMSE_NEW ChordNoteData(pNote, pNoteShape, posOnStaff, m_iInstr);
	    m_notes.push_back(pData);
    }
}

//---------------------------------------------------------------------------------------
void ChordEngraver::find_reference_notes()
{
    m_pStartNoteData = (m_fStemDown ? m_notes.back() : m_notes.front());
    m_pFlagNoteData = (m_fStemDown ? m_notes.front() : m_notes.back());

    m_pLinkNoteData = nullptr;
    if (m_pChord->is_cross_staff())
    {
        int staff = m_pFlagNoteData->pNote->get_staff();
        if (m_fStemDown)
        {
            std::list<ChordNoteData*>::iterator it;
            for(it=m_notes.begin(); it != m_notes.end(); ++it)
            {
                if ((*it)->pNote->get_staff() != staff)
                    break;
                m_pLinkNoteData = *it;
            }
        }
        else
        {
            std::list<ChordNoteData*>::reverse_iterator it;
            for(it=m_notes.rbegin(); it != m_notes.rend(); ++it)
            {
                if ((*it)->pNote->get_staff() != staff)
                    break;
                m_pLinkNoteData = *it;
            }
        }
    }

    //Fix for special cases
    if (!m_pLinkNoteData)
    {
        //For single-staff chords, the link note is the same than the start note and
        //thus, start note does no exists as no extensible segment exists.
        m_pLinkNoteData = m_pStartNoteData;
        m_pStartNoteData = nullptr;
    }
    else if (m_pLinkNoteData == m_pFlagNoteData )
    {
        //For cross-staff chords, when only two notes, one on each staff, the link note
        //and the link segment do not exist. But the loop for finding the link note
        //will point it to the flag note
        m_pLinkNoteData = nullptr;
    }

    //set ptrs. to reference notes in base note

    //set pointer to flag segment in chord base note shape
    GmoShapeChordBaseNote* pBaseNoteShape =
                    static_cast<GmoShapeChordBaseNote*>(m_pBaseNoteData->pNoteShape);
    pBaseNoteShape->set_flag_note(m_pFlagNoteData->pNoteShape);

    if (m_pLinkNoteData)
        pBaseNoteShape->set_link_note(m_pLinkNoteData->pNoteShape);
    if (m_pStartNoteData)
        pBaseNoteShape->set_start_note(m_pStartNoteData->pNoteShape);

    //set pointers to the base note
    for (ChordNoteData* pData : m_notes)
    {
        pData->pNoteShape->set_base_note_shape(pBaseNoteShape);
    }
}

//---------------------------------------------------------------------------------------
void ChordEngraver::decide_stem_direction()
{
    ImoNote* pBaseNote = get_base_note();
    ImoChord* pChord = pBaseNote->get_chord();
    m_noteType = pBaseNote->get_note_type();
    int stemType = pBaseNote->get_stem_direction();

    m_fHasStem = (m_noteType >= k_half) && (stemType != k_stem_none);
    m_fHasFlag = m_fHasStem && (m_noteType > k_quarter) && !is_chord_beamed();

    //if stem is forced, we have finished
    if (stemType == k_stem_down || stemType == k_stem_up)
    {
        m_fStemDown = (stemType == k_stem_down);
        pChord->set_stem_direction(m_fStemDown ? k_computed_stem_forced_down
                                               : k_computed_stem_forced_up);
        return;
    }

    //if beamed chord, compute stem direction for all chords and single notes in the beam
    ImoBeam* pBeam = pBaseNote->get_beam();
    if (pBeam && pBaseNote == pBeam->get_start_object())
    {
        BeamedChordHelper helper(pBeam, &m_clefs);
        m_fStemDown = helper.compute_stems_directions();
    }

    //do not compute chord stem direction if already computed in the beam
    if (pChord->is_stem_direction_decided())
        m_fStemDown = pChord->is_stem_down();
    else
    {
        int meanPos = 0;
        std::list<ChordNoteData*>::iterator it;
        for(it=m_notes.begin(); it != m_notes.end(); ++it)
            meanPos += (*it)->posOnStaff;
        meanPos /= int(m_notes.size());
        m_fStemDown = decide_stem_direction(pBaseNote, pChord, meanPos);
    }
}

//---------------------------------------------------------------------------------------
bool ChordEngraver::decide_stem_direction(ImoNote* pBaseNote, ImoChord* pChord,
                                          int meanPos)
{
    //returns TRUE if stem down, false if stem up

    //  Rules from E.Gould, p.47-48
    //  coincide with rules from ref. [2] www.coloradocollege.edu
    //
    //  a) Two notes in chord:
    //    a1. If the interval above the middle line is greater than the interval below
    //      the middle line: downward stems. e.g., (a4,d5) (f4,f5) (a4,g5)
    //      ==>   (MaxNotePos + MinNotePos)/2 > MiddleLinePos
    //
    //    a2. If the interval below the middle line is greater than the interval above
    //      the middle line: upward stems. e.g., (e4,c5)(g4,c5)(d4,e5)
    //
    //    a3. If the two notes are at the same distance from the middle line: stem can
    //      go in either direction, but most engravers prefer downward stems.
    //      e.g., (g4.d5)(a4,c5)
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
    //  c) chords without stem (notes longer than half notes) (from E.Gould):
    //      c1. layout as if they had stem and apply the previous rules.


    //proceed to compute stem direction
    bool fStemDown = false;
    int noteType = pBaseNote->get_note_type();
    int stemType = pBaseNote->get_stem_direction();

    if (pBaseNote->is_grace_note())
    {
        //for grace notes stem is always up unless stem down explicitly requested
        fStemDown = (stemType == k_stem_down);
        pChord->set_stem_direction(fStemDown ? k_computed_stem_down
                                             : k_computed_stem_up);
    }

    else if ((noteType < k_half)               //c1. layout as if they had stem
        || (stemType == k_stem_none)        //b3. majority rule
        || (stemType == k_stem_default))    //b3. majority rule
    {
        ////b3. majority rule
        fStemDown = ( meanPos >= 6);
        pChord->set_stem_direction(fStemDown ? k_computed_stem_down
                                             : k_computed_stem_up);
    }

    else    // (stemType == k_stem_down || stemType == k_stem_up)
    {
        fStemDown = (stemType == k_stem_down);
        pChord->set_stem_direction(fStemDown ? k_computed_stem_forced_down
                                             : k_computed_stem_forced_up);
    }

    return fStemDown;
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
        (*it)->pNoteShape->set_up_oriented(!m_fStemDown);
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
            m_fSomeAccidentalsShifted = true;
        }
    }
}

//---------------------------------------------------------------------------------------
void ChordEngraver::layout_arpeggio()
{
    //assume for now that arpeggio is always attached to all notes of one chord
    ImoArpeggio* pArpeggio = m_pBaseNoteData->pNote->get_arpeggio();

    if (!pArpeggio)
        return;

    ensure_note_bounds_updated();

    LUnits xLeft = m_notes.front()->pNoteShape->get_left();
    LUnits yTop = m_notes.front()->pNoteShape->get_notehead_top();
    LUnits yBottom = yTop;

    for (const ChordNoteData* pNoteData : m_notes)
    {
        const GmoShapeNote* pNoteShape = pNoteData->pNoteShape;

        //consider absolute left edge, including accidentals, but only noteheads' top and bottom
        const LUnits noteLeft = pNoteShape->get_left();
        const LUnits noteTop = pNoteShape->get_notehead_top();
        const LUnits noteBottom = pNoteShape->get_notehead_bottom();

        if (noteLeft < xLeft)
            xLeft = noteLeft;

        if (noteTop < yTop)
            yTop = noteTop;

        if (noteBottom > yBottom)
            yBottom = noteBottom;
    }

    bool fArpeggioUp;
    bool fArpeggioHasArrow;

    switch (pArpeggio->get_type())
    {
        case k_arpeggio_standard:
            fArpeggioUp = true;
            fArpeggioHasArrow = false;
            break;
        case k_arpeggio_arrow_up:
            fArpeggioUp = true;
            fArpeggioHasArrow = true;
            break;
        case k_arpeggio_arrow_down:
            fArpeggioUp = false;
            fArpeggioHasArrow = true;
            break;
    }

    //allow arpeggio take a bit more vertical space
    const LUnits extraSpace = tenths_to_logical(LOMSE_ARPEGGIO_MAX_OUTGOING);
    yTop -= extraSpace;
    yBottom += extraSpace;

    const LUnits xArpeggioRight = xLeft - tenths_to_logical(LOMSE_ARPEGGIO_SPACE_TO_CHORD);
    const double fontSize = determine_font_size();
    const ShapeId idx = 0;
    ChordNoteData* pArpeggioNoteData = m_pStartNoteData ? m_pStartNoteData : m_pBaseNoteData;

    GmoShapeArpeggio* pArpeggioShape = LOMSE_NEW GmoShapeArpeggio(pArpeggioNoteData->pNote, idx,
                                                                  xArpeggioRight, yTop, yBottom,
                                                                  fArpeggioUp, fArpeggioHasArrow,
                                                                  pArpeggio->get_color(), m_libraryScope, fontSize);
    pArpeggioNoteData->pNoteShape->add(pArpeggioShape);

    GmoShapeChordBaseNote* pBaseNoteShape =
                    static_cast<GmoShapeChordBaseNote*>(m_pBaseNoteData->pNoteShape);
    pBaseNoteShape->set_arpeggio(pArpeggioShape);
}

//---------------------------------------------------------------------------------------
void ChordEngraver::set_anchor_offset()
{
    //The anchor line for a chord is the anchor line of any of its notes, and any of
    //them will provide the right alinment position.

	std::list<ChordNoteData*>::iterator it;
	for(it = m_notes.begin(); it != m_notes.end(); ++it)
	{
		GmoShapeNote* pNoteShape = (*it)->pNoteShape;

		//as no more changes in shapes, let's ensure bounds are properly set
		pNoteShape->force_recompute_bounds();

        //compute anchor pos
        LUnits offset;
        if (!(*it)->fNoteheadReversed)
        {
            //if no reversed notehead, anchor is notehead x left. Stem direction
            //doesn't matter.
            offset = pNoteShape->get_notehead_left();
        }
        else
        {
            //when notehead is reversed, anchor line depends on stem direction
            if (is_stem_down())
            {
                //reversed notehead and stem down: anchor is x right of notehead.
                offset = pNoteShape->get_notehead_right();
            }
            else
            {
                //reversed notehead and stem up: anchor is x left of notehead plus
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
void ChordEngraver::ensure_note_bounds_updated()
{
    if (m_fSomeAccidentalsShifted)
    {
        for (ChordNoteData* pNoteData : m_notes)
        {
            pNoteData->pNoteShape->force_recompute_bounds();
        }

        m_fSomeAccidentalsShifted = false;
    }
}

//---------------------------------------------------------------------------------------
void ChordEngraver::add_stem_and_flag()
{
    if (!has_stem())
        return;

    ImoNote* pNoteFlag = m_pFlagNoteData->pNote;
    int instr = m_pFlagNoteData->iInstr;
    int staff = pNoteFlag->get_staff();

    StemFlagEngraver engrv(m_libraryScope, m_pMeter, pNoteFlag, instr, staff, m_fontSize);

    determine_stem_x_left();
    add_stem_flag_segment(&engrv);
    add_stem_link_segment();
    add_stem_extensible_segment_if_required();
    add_stroke_for_graces_if_required(&engrv);
}

//---------------------------------------------------------------------------------------
void ChordEngraver::add_stem_flag_segment(StemFlagEngraver* engrv)
{
    bool fHasBeam = is_chord_beamed();
    bool fShortFlag = false;
    Tenths length = 0.0f;

    if (m_symbolSize == k_size_cue)
    {
        length = 22.5f;     // 2 1/4 spaces. E.Gould p.126
    }
    else
    {
        int nPosOnStaff = m_pFlagNoteData->posOnStaff;
        length = NoteEngraver::get_standard_stem_length(nPosOnStaff, is_stem_down());
        if (!fHasBeam && length < 35.0f && m_noteType > k_eighth)
            length = 35.0f;     // 3.5 spaces

        fShortFlag = (length < 35.0f);
   }

    LUnits stemLength = tenths_to_logical(length);
    GmoShapeNote* pFlagNoteShape = m_pFlagNoteData->pNoteShape;

    engrv->add_stem_flag_to_note(pFlagNoteShape, m_noteType, is_stem_down(), has_flag(),
                                 fShortFlag, fHasBeam, stemLength,
                                 m_pFlagNoteData->fNoteheadReversed, m_color);
}

//---------------------------------------------------------------------------------------
void ChordEngraver::add_stem_link_segment()
{
    //For cross-staff chords, when only two notes, one on each staff, the link note is
    //the same than the start note and, thus, link note does no exists
    if (!m_pLinkNoteData)
        return;

    GmoShapeNote* pFlagNoteShape = m_pFlagNoteData->pNoteShape;
    GmoShapeNote* pLinkNoteShape = m_pLinkNoteData->pNoteShape;

    GmoShape* pTopNotehead = (m_fStemDown ? pLinkNoteShape : pFlagNoteShape)->get_notehead_shape();
    GmoShape* pBottomNotehead = (m_fStemDown ? pFlagNoteShape : pLinkNoteShape)->get_notehead_shape();
    LUnits halfNotehead = pTopNotehead->get_height() / 2.0f;

    LUnits yTop = pTopNotehead->get_top() + halfNotehead;
    LUnits yBottom = pBottomNotehead->get_top() + halfNotehead;

    GmoShapeStem* pShape = LOMSE_NEW GmoShapeStem(m_pLinkNoteData->pNote, m_uxStem, yTop,
                                                  yBottom, m_fStemDown,
                                                  m_uStemThickness, m_color);
    add_voice(pShape);
    pLinkNoteShape->add_stem(pShape);
}

//---------------------------------------------------------------------------------------
void ChordEngraver::add_stem_extensible_segment_if_required()
{
    if (!m_pChord->is_cross_staff())
        return;

    //for cross-staff chords the flag note can be the only one in that staff and then
    //there is no link note
    GmoShapeNote* pRefNoteShape = (m_pLinkNoteData ? m_pLinkNoteData : m_pFlagNoteData)->pNoteShape;
    GmoShapeNote* pStartNoteShape = m_pStartNoteData->pNoteShape;

    GmoShape* pTopNotehead = (m_fStemDown ? pStartNoteShape : pRefNoteShape)->get_notehead_shape();
    GmoShape* pBottomNotehead = (m_fStemDown ? pRefNoteShape : pStartNoteShape)->get_notehead_shape();
    LUnits halfNotehead = pTopNotehead->get_height() / 2.0f;

    LUnits yTop = pTopNotehead->get_top() + halfNotehead;
    LUnits yBottom = pBottomNotehead->get_top() + halfNotehead;

    GmoShapeStem* pShape = LOMSE_NEW GmoShapeStem(m_pStartNoteData->pNote, m_uxStem, yTop,
                                                  yBottom, m_fStemDown,
                                                  m_uStemThickness, m_color);
    add_voice(pShape);
    pStartNoteShape->add_stem(pShape);
}

//---------------------------------------------------------------------------------------
void ChordEngraver::add_stroke_for_graces_if_required(StemFlagEngraver* engrv)
{
    ImoNote* pBaseNote = get_base_note();
    if (pBaseNote->is_grace_note() && !pBaseNote->is_beamed())
    {
        ImoGraceRelObj* pRO = static_cast<ImoGraceRelObj*>(
                                    pBaseNote->get_grace_relobj() );
        if (pRO && pRO->has_slash()
            && pBaseNote == static_cast<ImoNote*>(pRO->get_start_object()) )
        {
            engrv->add_stroke_shape();
        }
    }
}

//---------------------------------------------------------------------------------------
void ChordEngraver::add_voice(VoiceRelatedShape* pVRS)
{
    VoiceRelatedShape* pNote = static_cast<VoiceRelatedShape*>(m_pBaseNoteData->pNoteShape);
    pVRS->set_voice(pNote->get_voice());
}

//---------------------------------------------------------------------------------------
void ChordEngraver::determine_stem_x_left()
{
    GmoShapeNote* pFlagNoteShape = m_pFlagNoteData->pNoteShape;
    m_uStemThickness = tenths_to_logical(LOMSE_STEM_THICKNESS);

    bool fAtLeft = m_fStemDown;
    if (m_pFlagNoteData->fNoteheadReversed)
        fAtLeft = !fAtLeft;

    if (fAtLeft)
		m_uxStem = pFlagNoteShape->get_notehead_left();
    else
		m_uxStem = pFlagNoteShape->get_notehead_right() - m_uStemThickness;
}


//=======================================================================================
// BeamedChordHelper implementation
//=======================================================================================
BeamedChordHelper::BeamedChordHelper(ImoBeam* pBeam, std::vector<int>* pClefs)
    : m_numStaves(pClefs->size())
    , m_nUpForced(0)
    , m_nDownForced(0)
    , m_pBeam(pBeam)
    , m_pClefs(pClefs)
    , m_pStemsDir(nullptr)
{
    m_totalPosOnStaff.assign(m_numStaves, 0);
    m_numNotes.assign(m_numStaves, 0);
    m_maxPosOnStaff.assign(m_numStaves, -20);
    m_minPosOnStaff.assign(m_numStaves, 20);
    if (pBeam)      //in unit tests pBeam can be nullptr
        m_pStemsDir = LOMSE_NEW vector<int>;    //(pBeam->get_num_objects());
}

//---------------------------------------------------------------------------------------
BeamedChordHelper::~BeamedChordHelper()
{
    if (!m_fTransferred)
        delete m_pStemsDir;
}

//---------------------------------------------------------------------------------------
bool BeamedChordHelper::compute_stems_directions()
{
    //returns first chord/single note stem direction

    //get ImoBeam. It contains the base notes for all chords in the beam.
    //And loop for each chord base note to determine its chord stem direction
    list< pair<ImoStaffObj*, ImoRelDataObj*> >& baseNotes = m_pBeam->get_related_objects();
    list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    ImoNote* pPrevBase = nullptr;
    for(it = baseNotes.begin(); it != baseNotes.end(); ++it)
    {
        if (((*it).first)->is_note())       //there can be rests in the beam
        {
            ImoNote* pBase = static_cast<ImoNote*>((*it).first);
            if (!pPrevBase)
                pPrevBase = pBase;

            if (pBase->is_in_chord())
                compute_stem_direction_for_chord(pBase, pPrevBase, m_pClefs);
            else
                compute_stem_direction_for_note(pBase, pPrevBase, m_pClefs);

            pPrevBase = pBase;
        }
        else
            m_pStemsDir->push_back(k_computed_stem_none);
    }

    //all stems directions are computed
    int beamPos = determine_beam_position();
    transfer_stem_directions_to_notes(m_pBeam, beamPos);

    return m_pStemsDir->front() == k_computed_stem_down
        || m_pStemsDir->front() == k_computed_stem_forced_down;
}

//---------------------------------------------------------------------------------------
bool BeamedChordHelper::compute_stem_direction_for_chord(ImoNote* pBaseNote,
                                                         ImoNote* pLastNote,
                                                         vector<int>* pLastClefs)
{
    //receives the base note for the new chord whose stem direction we have to compute,
    //a vector of applicable clefs for each staff, and the last note for which this
    //vector was computed
    //returns TRUE if stem down

    find_applicable_clefs(pBaseNote, pLastNote, pLastClefs);

    ImoChord* pChord = pBaseNote->get_chord();
    m_chords.push_back(pChord);
    int computedStem = k_computed_stem_undecided;
    int stemType = pBaseNote->get_stem_direction();
    bool fStemDown = (stemType == k_stem_down);

    if (stemType == k_stem_down || stemType == k_stem_up)
    {
        //stem is forced, we have finished
        fStemDown ? ++m_nDownForced : ++m_nUpForced;
        computedStem = (fStemDown ? k_computed_stem_forced_down
                                  : k_computed_stem_forced_up);
    }
    else
    {
        //stem not forced. collect chord notes and compute stem direction
        vector<ImoNote*> chordNotes = collect_chord_notes(pChord);
        int meanPosOnStaff = determine_mean_pos_on_staff(chordNotes, pLastClefs);
        fStemDown = ChordEngraver::decide_stem_direction(pBaseNote, pChord, meanPosOnStaff);
        computedStem = (fStemDown ? k_computed_stem_down : k_computed_stem_up);
    }

    pChord->set_stem_direction(computedStem);
    m_pStemsDir->push_back(computedStem);
    return fStemDown;
}

//---------------------------------------------------------------------------------------
bool BeamedChordHelper::compute_stem_direction_for_note(ImoNote* pNote,
                                                        ImoNote* pLastNote,
                                                        vector<int>* pClefs)
{
    //receives a single note whose stem direction we have to compute,
    //a vector of applicable clefs for each staff, and the last note for which this
    //vector was computed.
    //returns TRUE if stem down

    find_applicable_clefs(pNote, pLastNote, pClefs);

    //if stem is forced, we have finished
    int stemType = pNote->get_stem_direction();
    bool fStemDown = (stemType == k_stem_down);

    if (stemType == k_stem_down || stemType == k_stem_up)
    {
        //stem is forced, we have finished
        fStemDown ? ++m_nDownForced : ++m_nUpForced;
        m_pStemsDir->push_back(fStemDown ? k_computed_stem_forced_down
                                         : k_computed_stem_forced_up);
    }
    else
    {
        //stem not forced. compute stem direction
        int staff = pNote->get_staff();
        int clefType =  pClefs->at(staff);
        int pos = NoteEngraver::pitch_to_pos_on_staff(pNote, clefType, 0);

        m_totalPosOnStaff[staff] += pos;
        m_maxPosOnStaff[staff] = max(m_maxPosOnStaff[staff], pos);
        m_minPosOnStaff[staff] = min(m_minPosOnStaff[staff], pos);
        ++m_numNotes[staff];

        fStemDown = (pos >= 6);
        m_pStemsDir->push_back(fStemDown ? k_computed_stem_down : k_computed_stem_up);
    }
    return fStemDown;
}

//---------------------------------------------------------------------------------------
vector<ImoNote*> BeamedChordHelper::collect_chord_notes(ImoChord* pChord)
{
    //access the ColStaffObj table and collect all the chord notes and their applicable
    //clefs and updates the pLastKnown vector. Returns the chord notes and their clefs

    vector<ImoNote*> chordNotes;
    list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes = pChord->get_related_objects();
    list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    for(it = notes.begin(); it != notes.end(); ++it)
    {
        ImoNote* pNote = static_cast<ImoNote*>((*it).first);
        chordNotes.push_back(pNote);
    }

    return chordNotes;
}

//---------------------------------------------------------------------------------------
int BeamedChordHelper::determine_mean_pos_on_staff(vector<ImoNote*>& chordNotes,
                                                   vector<int>* pClefs)
{
    int posOnStaff = 0;
    for (size_t i = 0; i < chordNotes.size(); ++i)
    {
        ImoNote* pNote = chordNotes[i];
        int clefType =  pClefs->at(pNote->get_staff());
        int pos = NoteEngraver::pitch_to_pos_on_staff(pNote, clefType, 0);

        int staff = pNote->get_staff();
        m_totalPosOnStaff[staff] += pos;
        m_maxPosOnStaff[staff] = max(m_maxPosOnStaff[staff], pos);
        m_minPosOnStaff[staff] = min(m_minPosOnStaff[staff], pos);
        ++m_numNotes[staff];

        posOnStaff += pos;
    }

    return posOnStaff / int(chordNotes.size());
}

//---------------------------------------------------------------------------------------
void BeamedChordHelper::find_applicable_clefs(ImoNote* pBaseNote, ImoNote* pLastNote,
                                              vector<int>* pLastClefs)
{
    //access the ColStaffObj table and collect the applicable clefs in all staves
    //at location given by pBaseNote. pLastNote points to the note for which its
    //applicable clefs are known. These clefs are in vector pLastClefs that is
    //updated.

    ColStaffObjsEntry* pEntry = pLastNote->get_colstaffobjs_entry();
    int iInstr = pEntry->num_instrument();
    while(pEntry && pEntry->imo_object() != pBaseNote)
    {
        if (pEntry->num_instrument() == iInstr && pEntry->imo_object()->is_clef())
        {
            ImoClef* clef = static_cast<ImoClef*>( pEntry->imo_object() );
            int staff = clef->get_staff();
            pLastClefs->at(staff) = clef->get_clef_type();
        }
        pEntry = pEntry->get_next();
    }
}

//---------------------------------------------------------------------------------------
int BeamedChordHelper::determine_beam_position()
{
    //Rules:
    //  if some chords direction forced:
    //      all in the same direction: force all others
    //      in different directions: double-stemmed
    //  else (no chord forced)
    //      apply majority rule using all chords notes

    bool fStemsDown = false;
    bool fDoubleStemmed = false;
    if (m_nUpForced > 0)
    {
        if (m_nDownForced == 0)
            fStemsDown = false;     //stems forced up. beam above
        else
            fDoubleStemmed = true;  //forced in mixed directions: double-stemmed beam
    }
    else if (m_nDownForced > 0)
        fStemsDown = true;   //stems forced down. beam below
    else
    {
        //apply stem direction rules
        if (m_numStaves > 1)
        {
            //when two staves, the chord can be on any staff or be cross-staff
            if (m_numNotes[0] == 0)
            {
                //412. all chords in staff 1. stem direction rules for staff 1
                fStemsDown = apply_stem_direction_rules_for_staff(1);
            }
            else if (m_numNotes[1] == 0)
            {
                //408, 410, 411. all chords in staff 0. stem direction rules for staff 0
                fStemsDown = apply_stem_direction_rules_for_staff(0);
            }
            else
            {
                //413, 414
                //notes on both staves. Either, chords in both staves or cross staff.
                //Prefer single-stemmed so apply stem direction rules for combined staves
                fStemsDown = apply_stem_direction_rules_for_both_staves();
            }
        }
        else    //409. only one staff. stem direction rules for staff 0
        {
            fStemsDown = apply_stem_direction_rules_for_staff(0);
        }
    }

    //decide beam position
    if (fDoubleStemmed)
        return k_beam_double_stemmed;
    else
        return (fStemsDown ? k_beam_below : k_beam_above);
}

//---------------------------------------------------------------------------------------
bool BeamedChordHelper::apply_stem_direction_rules_for_staff(int iStaff)
{
    int sum = m_maxPosOnStaff[iStaff] + m_minPosOnStaff[iStaff];
    if (sum > 12)
        return true;  //stem down
    else if (sum == 12)
        return (m_totalPosOnStaff[0] >= 6 * m_numNotes[0]);   //majority rule
    else
        return false;     //stem up
}

//---------------------------------------------------------------------------------------
bool BeamedChordHelper::apply_stem_direction_rules_for_both_staves()
{
    int sum = m_maxPosOnStaff[0] + m_minPosOnStaff[1];

    if (sum > 12)   //furthest note. stem down
        return true;

    else if (sum == 12) //both furthest equal. Majority rule
        return ((m_totalPosOnStaff[0] + m_totalPosOnStaff[1]) >=
                6 * (m_numNotes[0] + m_numNotes[1]) );

    else    //furthest note. stem up
        return false;
}

//---------------------------------------------------------------------------------------
void BeamedChordHelper::transfer_stem_directions_to_notes(ImoBeam* pBeam, int beamPos)
{
    list< pair<ImoStaffObj*, ImoRelDataObj*> >& baseNotes = pBeam->get_related_objects();
    list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    int iItem = 0;
    int iChord = 0;
    for(it = baseNotes.begin(); it != baseNotes.end(); ++it, ++iItem)
    {
        if (((*it).first)->is_note())       //there can be rests in the beam
        {
            int stem = m_pStemsDir->at(iItem);
            if (beamPos == k_beam_above && stem != k_computed_stem_forced_up)
                stem = k_computed_stem_up;
            else if (beamPos == k_beam_below && stem != k_computed_stem_forced_down)
                stem = k_computed_stem_down;

            ImoNote* pNote = static_cast<ImoNote*>((*it).first);
            pNote->set_computed_stem(stem);
            if (pNote->is_start_of_chord())
            {
                m_chords[iChord++]->set_stem_direction(stem);
            }
            m_pStemsDir->at(iItem) = stem;
        }
    }

    pBeam->set_stems_direction(m_pStemsDir);
    m_fTransferred = true;
}


}  //namespace lomse

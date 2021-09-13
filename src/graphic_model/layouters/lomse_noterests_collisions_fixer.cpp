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

#include "lomse_noterests_collisions_fixer.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_table.h"
#include "lomse_shape_note.h"
#include "lomse_logger.h"
#include "lomse_score_meter.h"

#include <cmath>   //abs
using namespace std;


namespace lomse
{

#define LOMSE_NOTERESTS_COLLISIONS_COLOURED     0       //1=dbg, 0=release
const Color m_colorMoved(0, 192, 0);            //green
const Color m_colorCollision(0, 0, 255);        //blue
const Color m_colorWarning(255, 128, 0);        //orange
const Color m_colorInvestigate(255, 0, 255);    //magenta


//=====================================================================================
//NoterestsCollisionsFixer implementation
//=====================================================================================
NoterestsCollisionsFixer::NoterestsCollisionsFixer(GmoShape* pShape,
                                                   ColStaffObjsEntry* pEntry,
                                                   ScoreMeter* pMeter)
    : m_iFirstLine(pEntry->line())
    , m_pMeter(pMeter)
    , m_iInstr(pEntry->num_instrument())
    , m_idxStaff( pMeter->staff_index(m_iInstr, pEntry->staff()) )
{
    add_noterest(pShape, pEntry);
}

//---------------------------------------------------------------------------------------
NoterestsCollisionsFixer::~NoterestsCollisionsFixer()
{
    for (auto note : m_notes)
        delete note;
    m_notes.clear();
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::add_noterest(GmoShape* pShape, ColStaffObjsEntry* pEntry)
{
//    LOMSE_ASSERT(pEntry->imo_object()->is_note_rest());
//    LOMSE_ASSERT(pShape->is_shape_note() || pShape->is_shape_rest());

    ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pEntry->imo_object());
    int posOnStaff = (pNR->is_note() ? static_cast<GmoShapeNote*>(pShape)->get_pos_on_staff()
                                     : static_cast<GmoShapeRest*>(pShape)->get_pos_on_staff());
    NoterestData* pData = LOMSE_NEW NoterestData(pNR, pShape, posOnStaff);
    m_notes.push_back(pData);

    //classify the objects as they are added
    if (pNR->is_rest())
        pData->type = k_nrdata_rest;      //it is a rest
    else
    {
        ImoNote* pNote = static_cast<ImoNote*>(pNR);
        if (pNote->is_in_chord())
            pData->type = k_nrdata_chord;         //it is a chord (note included in a chord)
        else
            pData->type = k_nrdata_note;          //it is a note (isolated note not part of a chord)
    }

    //and collect additional information
    pData->line = pEntry->line();
    m_fMoreThanOneLine |= (pEntry->line() != m_iFirstLine);
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::fix_spacing_issues()
{
    if (!m_fMoreThanOneLine)
        return;

    //for each note/rest check that it does not overlap any other note/rest that is
    //in a different line, by checking bounding boxes. Note that note/rests
    //in the same line never overlap.
    for (size_t i=0; i < m_notes.size() - 1; ++i)
    {
        for (size_t j=i; j < m_notes.size(); ++j)
        {
            //check if conflict between the two noterests when in different lines
            if (m_notes[i]->line != m_notes[j]->line)
            {
                URect bbox = m_notes[i]->pShape->get_bounds();
                bbox.intersection( m_notes[j]->pShape->get_bounds() );
                if (bbox.height > 0.0f && bbox.width > 0.0f)
                {
                    #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
                        //debug, to visualy identify the involved noterests
                        m_notes[i]->pShape->set_color(m_colorCollision);
                        m_notes[j]->pShape->set_color(m_colorCollision);
                    #endif

                    //possible conflict detected. Identify conflict type
                    int conflictType = m_notes[i]->type + 10 * m_notes[j]->type;
                    //there are nine posibilities:
                    //  1+10 = 11   rest & rest
                    //  1+20 = 21   rest & note
                    //  1+30 = 31   rest & chord
                    //  2+10 = 12   note & rest
                    //  2+20 = 22   note & note
                    //  2+30 = 32   note & chord
                    //  3+10 = 13   chord & rest
                    //  3+20 = 23   chord & note
                    //  3+30 = 33   chord & chord
                    switch (conflictType)
                    {
                        case 11: fix_two_rests_overlap(i, j);           break;
                        case 21: fix_note_and_rest_overlap(j, i);       break;
                        case 31: fix_chord_and_rest_overlap(j, i);      break;
                        case 12: fix_note_and_rest_overlap(i, j);       break;
                        case 22: fix_two_notes_overlap(i, j);           break;
                        case 32: fix_chord_and_note_overlap(j, i);      break;
                        case 13: fix_chord_and_rest_overlap(i, j);      break;
                        case 23: fix_chord_and_note_overlap(i, j);      break;
                        case 33: fix_two_chords_overlap(i, j);          break;
                        default:
                        {
                            stringstream ss;
                            ss << "Invalid conflict type: " << conflictType
                               << ", obj1:" << m_notes[i]->pNR->get_name()
                               << ", type1:" << m_notes[i]->type
                               << ", obj2:" << m_notes[j]->pNR->get_name()
                               << ", type2:" << m_notes[j]->type;
                            LOMSE_LOG_ERROR(ss.str());

                            return;     //do not touch anything
                        }
                    }

                    //for now, assume only one conflict, until more evidence/experience
                    //about scenarios with more than one collision
                    return;

                }   //conflict detected
            }   //both in different line
        }   //j loop
    }   //i loop
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::fix_two_rests_overlap(size_t i, size_t j)
{
    //get position on staff
    GmoShapeRest* pRestShape1 = static_cast<GmoShapeRest*>(m_notes[i]->pShape);
    GmoShapeRest* pRestShape2 = static_cast<GmoShapeRest*>(m_notes[j]->pShape);
    int posOnStaff1 = pRestShape1->get_pos_on_staff();
    int posOnStaff2 = pRestShape2->get_pos_on_staff();

    //if 'unison' (both at same position and same duration) better keep them overlapped
    if (posOnStaff1 == posOnStaff2
        && m_notes[i]->pNR->get_note_type() == m_notes[j]->pNR->get_note_type())
    {
        #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
            //debug, to visualy identify cases
            pRestShape1->set_color(m_colorWarning);
            pRestShape2->set_color(m_colorWarning);
        #endif
        return;
    }

    //assume the upper voice is the first one defined.
    bool fFirstIsUpperVoice = (m_notes[i]->line < m_notes[j]->line);

    //determine shifts to apply so that they are symetrically positioned around the center line
    int offset1 = (fFirstIsUpperVoice ? posOnStaff1 - 10 : posOnStaff1 - 2);
    int offset2 = (fFirstIsUpperVoice ? posOnStaff2 - 2 : posOnStaff2 - 10);

    //move the rests
    if (offset1 != 0)
    {
        LUnits yShift = m_pMeter->tenths_to_logical(float(offset1)*5.0f, m_iInstr, m_idxStaff);
        move_rest(i, yShift);
    }
    if (offset2 != 0)
    {
        LUnits yShift = m_pMeter->tenths_to_logical(float(offset2)*5.0f, m_iInstr, m_idxStaff);
        move_rest(j, yShift);
    }
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::fix_note_and_rest_overlap(size_t i, size_t j)
{
    //shift rest one line appart

    LUnits xShift = 0.0f;
    LUnits yShift = 0.0f;
    GmoShapeNote* pNoteShape = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
    GmoShapeNotehead* pNotehead = pNoteShape->get_notehead_shape();
    GmoShapeRest* pRest = static_cast<GmoShapeRest*>(m_notes[j]->pShape);
    bool fMoveUp = !pNoteShape->is_up(); //stget_top() > pRest->get_top();

    //determine if collision with the notehead
    URect overlap = pNotehead->get_bounds();
    overlap.intersection(pRest->get_bounds());
    if (overlap.height > 0.0f)
    {
        //overlap with notehead
        LUnits space = m_pMeter->tenths_to_logical(10.0f, m_iInstr, m_idxStaff);
        if (fMoveUp)
            yShift = pRest->get_bottom() - pNotehead->get_top() + space;
        else
            yShift = pNotehead->get_bottom() - pRest->get_top() + space;
    }

    move_rest(j, (fMoveUp ? -yShift : yShift));

    //determine if collision with the stem
    GmoShapeStem* pStem = pNoteShape->get_stem_shape();
    if (pStem)
    {
        overlap = pStem->get_bounds();
        overlap.intersection(pRest->get_bounds());
        if (overlap.width > 0.0f)
        {
            //fix overlap with stem
            xShift = overlap.width + m_pMeter->tenths_to_logical(2.0f, m_iInstr, m_idxStaff);
            pRest->increment_anchor_offset(-xShift);
        }
    }
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::fix_chord_and_rest_overlap(size_t i, size_t j)
{
    //shift rest one line appart

    //find start and end of chord
    size_t iStart = find_start_of_chord(i);
    size_t iEnd = find_end_of_chord(i);

    //find distance from rest to each note
    LUnits dxStart = m_notes[iStart]->pShape->get_top() - m_notes[j]->pShape->get_top();
    LUnits dxEnd = m_notes[iEnd]->pShape->get_top() - m_notes[j]->pShape->get_top();

    //determine shift amount and direction
    LUnits yShift = dxStart;
    if (abs(dxStart) > abs(dxEnd))
    {
        yShift = dxEnd;
    }
    LUnits space = m_pMeter->tenths_to_logical(10.0f, m_iInstr, m_idxStaff);
    if (yShift > 0)
        yShift += space;
    else
        yShift -= space;

    move_rest(j, yShift);

    //determine if collision with the stem
    GmoShapeNote* pNoteShape = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
    GmoShapeChordBaseNote* pBaseNote = pNoteShape->get_base_note_shape();
    GmoShapeNote* pFlagNote = pBaseNote->get_flag_note();
    GmoShapeStem* pStem = pFlagNote->get_stem_shape();
    if (pStem)
    {
        GmoShapeRest* pRest = static_cast<GmoShapeRest*>(m_notes[j]->pShape);
        URect overlap = pStem->get_bounds();
        overlap.intersection(pRest->get_bounds());
        if (overlap.width > 0.0f)
        {
            //fix overlap with stem
            LUnits xShift = overlap.width + m_pMeter->tenths_to_logical(2.0f, m_iInstr, m_idxStaff);
            pRest->increment_anchor_offset(-xShift);
        }
    }
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::fix_two_notes_overlap(size_t i, size_t j)
{
    //The collision is not only when noteheads overlap. When noteheads do not
    //overlap, if voices crossed (stem up note below stem down note), the lower voice
    //(stem down note) has to be moved slightly to the right.

    //get shapes and position on staff
    GmoShapeNote* pNoteShape1 = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
    GmoShapeNote* pNoteShape2 = static_cast<GmoShapeNote*>(m_notes[j]->pShape);
    int posNote1 = m_notes[i]->posOnStaff;
    int posNote2 = m_notes[j]->posOnStaff;

    //check for crossed-voice cases
    bool fCrossedVoice = (posNote1 > posNote2 && pNoteShape2->is_up())
                         || (posNote2 > posNote1 && pNoteShape1->is_up());

    //check if noteheads conflict by comparing pitches
    bool fNoteheadsOverlap = false;
    switch ( check_if_noteheads_overlap(i, j) )
    {
        case k_overlap_unison:      //total overlap: Unison
        {
            //confirm unison, by checking that notated accidentals are the same
            EAccidentals acc1 = static_cast<ImoNote*>(m_notes[i]->pNR)->get_notated_accidentals();
            EAccidentals acc2 = static_cast<ImoNote*>(m_notes[j]->pNR)->get_notated_accidentals();
            if (acc1 == acc2)
            {
                //decide what to do depeding on options: two stems or overlap
                fNoteheadsOverlap = false;
            }
            else
                fNoteheadsOverlap = true;
            break;
        }
        case k_overlap_second:      //a second appart. Shift one of the notes
        {
            fNoteheadsOverlap = true;
            break;
        }
        case k_overlap_third:       //a third appart: posible false positive
        {
            #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
                if (!fCrossedVoice)
                {
                    //debug, to visualy identify cases.  Orange
                    pNoteShape1->set_color(m_colorWarning);
                    pNoteShape2->set_color(m_colorWarning);
                }
            #endif
            break;
        }
        case k_overlap_none:        //no overlap. false positive
        {
            #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
                if (!fCrossedVoice)
                {
                    //debug, to visualy identify cases.  Red
                    pNoteShape1->set_color(m_colorInvestigate);
                    pNoteShape2->set_color(m_colorInvestigate);
                }
            #endif
            break;
        }
    }

    //when importing MusicXML there are cases in which both voices have stems in the
    //same direction and this creates a false positive due to stems collision. Check
    //this case
    if (pNoteShape1->is_up() == pNoteShape2->is_up() && !fNoteheadsOverlap)
        return;     //false positive


    //move one note if collision confirmed: fNoteheadsOverlap || fCrossedVoice
    if (fNoteheadsOverlap && !fCrossedVoice)
    {
        //only noteheads overlap and not crossed-voice:
        //move down-stemmed note to the right
        if (pNoteShape1->is_up())
        {
            LUnits xShift = pNoteShape2->get_notehead_width();  //TODO: shift when flag or dots
            move_note_and_accidentals(j, i, xShift);
        }
        else
        {
            LUnits xShift = pNoteShape1->get_notehead_width();  //TODO: shift when flag or dots
            move_note_and_accidentals(i , j, xShift);
        }
    }
    else if (fCrossedVoice)
    {
        //move up-stemmed note to the right
        LUnits xShift = m_pMeter->tenths_to_logical(LOMSE_SHIFT_WHEN_NOTEHEADS_OVERLAP,
                                                    m_iInstr, m_idxStaff);
        if (fNoteheadsOverlap)
            xShift += pNoteShape2->get_notehead_width();  //TODO: shift when flag or dots

        if (pNoteShape1->is_up())
            move_note_and_accidentals(i , j, xShift);
        else
            move_note_and_accidentals(j, i, xShift);
    }
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::fix_chord_and_note_overlap(size_t i, size_t j)
{
    //find start and end of chord
    size_t iStart = find_start_of_chord(i);
    size_t iEnd = find_end_of_chord(i);

    //get the shapes of the two notes that overlap
    GmoShapeNote* pShapeChord = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
    GmoShapeNote* pShapeNote = static_cast<GmoShapeNote*>(m_notes[j]->pShape);

    //get position on staff
    int posStart = m_notes[iStart]->posOnStaff;
    int posEnd = m_notes[iEnd]->posOnStaff;
    int posNote = m_notes[j]->posOnStaff;

    //check if the note is outside the chord
    bool fTheNoteIsOutsideChord = (posNote >= posStart && posNote >= posEnd)
                                  || (posNote <= posStart && posNote <= posEnd);

    //check if notehead overlap
    std::pair<size_t, int> overlap = find_chord_note_that_overlap_with(j, iStart, iEnd);
    size_t iNote = overlap.first;
    bool fDoNothing = false;
    switch (overlap.second)
    {
        case k_overlap_unison:      //total overlap: Unison
        {
            if (iNote == iStart || iNote == iEnd)
            {
                //confirm unison, by checking notated accidentals
                EAccidentals accChord = static_cast<ImoNote*>(m_notes[i]->pNR)->get_notated_accidentals();
                EAccidentals accNote = static_cast<ImoNote*>(m_notes[j]->pNR)->get_notated_accidentals();
                if (accChord == accNote)
                {
                    //decide what to do depeding on options: two stems or overlap
                    fDoNothing = true;
                }
                //else move the note or the chord
            }
            else
            {
                //overlap in the middle. Move the note or the chord
            }
            break;
        }

        case k_overlap_second:      //a second appart. Move the note or the chord
            break;

        case k_overlap_third:       //a third appart: posible false positive
        {
            if (fTheNoteIsOutsideChord)
            {
                //false positive
                fDoNothing = true;
                #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
                    //debug, to visualy identify cases
                    pShapeChord->set_color(m_colorWarning);
                    pShapeNote->set_color(m_colorWarning);
                #endif
            }
            break;
        }

        case k_overlap_none:        //no noteheads overlap. If note in between requires a minimal shift
        {
            if (fTheNoteIsOutsideChord)
            {
                //false positive. impossible case ?
                fDoNothing = true;
                #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
                    //debug, to visualy identify cases
                    pShapeChord->set_color(m_colorInvestigate);
                    pShapeNote->set_color(m_colorInvestigate);
                #endif
            }
            break;
        }
    }
    if (fDoNothing)
        return;

    //Always move to the right
    //When the note is ouside the chord, the lower part is moved to the right
    //But if the note is inside the chord, then move the up-stemmed part to the right
    bool fMoveNote = (fTheNoteIsOutsideChord && pShapeChord->is_up())
                     || (!fTheNoteIsOutsideChord && pShapeNote->is_up());

    //determine shift amount
    LUnits xShift = pShapeNote->get_notehead_width();

    //TODO: shift when flag or dots. Dots and collisions with flags introduce much
    //      more complexity. This needs study. Some cases:
    //when chord moved to right by minimal space (overlap none or third):
    //- if note has flag and flag collides with one or more chord notes
    //      xShift = note.width - note.anchor_offset + hairline
    //- if note has dots
    //      xShift = note.width - note.anchor_offset + hairline
    //- else
    //      xShift = hairline
    //
    //when note moved to right by minimal space (overlap none or third):
    //- if chord has dots in notes above this note
    //      xShift = chord_note.width - chord_note.anchor_offset + hairline
    //- else
    //      xShift = hairline

    if (overlap.second == k_overlap_none || overlap.second == k_overlap_third)
        xShift = m_pMeter->tenths_to_logical(LOMSE_SHIFT_WHEN_NOTEHEADS_OVERLAP,
                                             m_iInstr, m_idxStaff);
    else if (overlap.second == k_overlap_second
             && (!fMoveNote && pShapeChord->is_up() && !fTheNoteIsOutsideChord) )
        xShift *= 0.65f;

    if (fMoveNote)
        move_note_and_chord_accidentals(j, iStart, iEnd, xShift);
    else
        move_chord_and_note_accidentals(j, iStart, iEnd, xShift);
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::fix_two_chords_overlap(size_t i, size_t j)
{
    //find start and end of first chord
    size_t iStart1 = find_start_of_chord(i);
    size_t iEnd1 = find_end_of_chord(i);

    //find start and end of second chord
    size_t iStart2 = find_start_of_chord(j);
    size_t iEnd2 = find_end_of_chord(j);

    //get positions on staff and normalize (posStart > posEnd)
    int posStart1 = m_notes[iStart1]->posOnStaff;
    int posEnd1 = m_notes[iEnd1]->posOnStaff;
    int posStart2 = m_notes[iStart2]->posOnStaff;
    int posEnd2 = m_notes[iEnd2]->posOnStaff;
    if (posStart1 < posEnd1)
    {
        int i = posStart1;
        posStart1 = posEnd1;
        posEnd1 = i;
    }
    if (posStart2 < posEnd2)
    {
        int i = posStart2;
        posStart2 = posEnd2;
        posEnd2 = i;
    }

    //determine if overlapping parts
    bool fChordsOverlap = (posStart1 >= posStart2 && posStart2 > posEnd1)
                           || (posStart2 >= posStart1 && posStart1 > posEnd2);

    //if no overlap it must be a unison, a second of a false positive
    if (!fChordsOverlap)
    {
        //check if unison
        if (posStart1 == posEnd2)
        {
            //confirm unison, by checking notated accidentals
            EAccidentals acc1 = static_cast<ImoNote*>(m_notes[iStart1]->pNR)->get_notated_accidentals();
            EAccidentals acc2 = static_cast<ImoNote*>(m_notes[iEnd2]->pNR)->get_notated_accidentals();
            if (acc1 == acc2)
            {
                //TODO: decide what to do depeding on options: shift appart or overlap
                return;     //for now, allow overlap
            }
        }
        else if (posStart2 == posEnd1)
        {
            //confirm unison, by checking notated accidentals
            EAccidentals acc1 = static_cast<ImoNote*>(m_notes[iStart2]->pNR)->get_notated_accidentals();
            EAccidentals acc2 = static_cast<ImoNote*>(m_notes[iEnd1]->pNR)->get_notated_accidentals();
            if (acc1 == acc2)
            {
                //TODO: decide what to do depeding on options: shift appart or overlap
                return;     //for now, allow overlap
            }
        }
        else if (posStart1 == posEnd2+1 || posStart2 == posEnd1+1)
        {
            //it is a second. move one chord
        }
        else
        {
            //a third or greater. False positive
            return;
        }

    }

    //when no overlapped, move the down-stemmed chord to the right. Otherwise, move
    //the up-stemmed chord to the right
    GmoShapeNote* pShapeNote1 = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
    bool fMoveChord1 = (!pShapeNote1->is_up() && !fChordsOverlap)
                       || (pShapeNote1->is_up() && fChordsOverlap);

    LUnits xShift = pShapeNote1->get_notehead_width();  //TODO: shift when flag or dots
    if (fMoveChord1)
        move_chord(iStart1, iEnd1, iStart2, iEnd2, xShift);
    else
        move_chord(iStart2, iEnd2, iStart1, iEnd1, xShift);
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::move_rest(size_t i, LUnits yShift)
{
    //move shape down when yShift is positive, otherwise move it up
    //To shift the rest left/right just modify its anchor offset

    GmoShapeRest* pRestShape = static_cast<GmoShapeRest*>(m_notes[i]->pShape);

    pRestShape->shift_origin(0.0f, yShift);
    pRestShape->force_recompute_bounds();
    #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
        pRestShape->set_color(m_colorMoved);     //debug, to visualy identify moved shape
    #endif
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::move_note_and_accidentals(size_t i, size_t j, LUnits xShift)
{
    //move note to left (xShift < 0) or to right (xShif > 0).
    //move the notehead first and fix accidentals, if any, on both notes

    GmoShapeNote* pShapeNote1 = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
    GmoShapeNote* pShapeNote2 = static_cast<GmoShapeNote*>(m_notes[j]->pShape);
    GmoShape* pAccShape1 = pShapeNote1->get_accidentals_shape();
    GmoShape* pAccShape2 = pShapeNote2->get_accidentals_shape();

    if (!pAccShape1 || !pAccShape2)
    {
        //only one note have accidentals. So move the note and leave
        //the only accidental in current position
        move_notehead(pShapeNote1, xShift);
    }
    else
    {
        //both notes have accidentals
        if (xShift > 0.0f)
        {
            //move the note to the rigth

            //first move notehead to right and leave accidental at current position,
            //colliding with the other note accidental
            move_notehead(pShapeNote1, xShift);

            //now move the accidental before the other note accidental and keep anchor
            //line after the accidental
            xShift = pAccShape1->get_width()
                     + m_pMeter->tenths_to_logical(LOMSE_SPACE_BETWEEN_ACCIDENTALS, m_iInstr, m_idxStaff);
            move_accidental_to_left(pShapeNote2, xShift);
        }
        else
        {
            //move the note to the left
            stringstream ss;
            ss << "Impossible case: in current code this method is always invoked to "
               << "move the note to right. xShift=" << xShift;
            LOMSE_LOG_ERROR(ss.str());
        }
    }
    #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
        pShapeNote1->set_color(m_colorMoved);     //debug, to visualy identify cases
    #endif
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::move_chord_and_note_accidentals(size_t iNote, size_t iStart,
                                                               size_t iEnd, LUnits xShift)
{
    //move chord to the right. Chord accidentals remain at the same place. Note
    //accidental will be moved to the left, before the chord accidentals

    //move chord noteheads to the right. Chord accidentals remain at the same place
    bool fChordHasAccidentals = false;
    for(size_t i = iStart; i <= iEnd; ++i)
    {
        GmoShapeNote* pShapeNote = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
        fChordHasAccidentals |= (pShapeNote->get_accidentals_shape() != nullptr);
        move_notehead(pShapeNote, xShift);
        #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
            pShapeNote->set_color(m_colorMoved);     //debug, to visualy identify cases
        #endif
    }

    //if the chord has accidentals and the note has accidental, move the note accidental
    //to the left, before chord accidentals
    if (fChordHasAccidentals)
    {
        GmoShapeNote* pShapeNote = static_cast<GmoShapeNote*>(m_notes[iNote]->pShape);
        GmoShape* pAccShapeNote = pShapeNote->get_accidentals_shape();
        if (pAccShapeNote)
        {
            //determine shift
            xShift = 0.0f;
            for(size_t i = iStart; i <= iEnd; ++i)
            {
                GmoShapeNote* pShapeChord = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
                GmoShape* pAccShapeChord = pShapeChord->get_accidentals_shape();
                if (pAccShapeChord)
                {
                    URect bbox = pAccShapeChord->get_bounds();
                    LUnits width = bbox.width;
                    bbox.intersection( pAccShapeNote->get_bounds() );
                    if (bbox.width > 0.0f)
                        xShift = max(xShift, width);
                }
            }
            xShift += m_pMeter->tenths_to_logical(LOMSE_SPACE_BETWEEN_ACCIDENTALS,
                                                  m_iInstr, m_idxStaff);

            //shift the note accidental to the left
            move_accidental_to_left(pShapeNote, xShift);
        }
    }
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::move_note_and_chord_accidentals(size_t iNote, size_t iStart,
                                                               size_t iEnd, LUnits xShift)
{
    //move note to the left (xShift < 0) or to the right (xShif > 0) of a chord.
    //when to the left, chord accidentals are moved before the note and note accidental
    //before them.
    //When to the right, note is moved after the chord, its accidental is moved before
    //chord noteheads and chord accidentals before note accidental


    //determine if chord has accidentals.
    //index i remains pointing to the first chord note with accidentals or to iEnd+1
    bool fChordHasAccidentals = false;
    size_t i = iStart;
    for (; i <= iEnd; ++i)
    {
        EAccidentals acc = static_cast<ImoNote*>(m_notes[i]->pNR)->get_notated_accidentals();
        if (acc != k_no_accidentals)
        {
            fChordHasAccidentals = true;
            break;
        }
    }

    //proceed depending on case
    GmoShapeNote* pShapeNote = static_cast<GmoShapeNote*>(m_notes[iNote]->pShape);
    GmoShape* pAccShapeNote = pShapeNote->get_accidentals_shape();

    if (!pAccShapeNote || !fChordHasAccidentals)
    {
        //only the chord or the note have accidentals. So move the note and leave
        //the accidentals in current position
        move_notehead(pShapeNote, xShift);
    }
    else
    {
        //both, note and chord, have accidentals
        GmoShapeNote* pShapeNoteChord = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
        GmoShape* pAccShapeChord = pShapeNoteChord->get_accidentals_shape();
        if (xShift > 0.0f)
        {
            //move note to the right

            //first move notehead to rigth and leave the accidental at current place
            move_notehead(pShapeNote, xShift);

            //now move left any chord accidental colliding with note accidental.
            //TODO: check other chord accidentals
            xShift = pAccShapeNote->get_width()
                     + m_pMeter->tenths_to_logical(LOMSE_SPACE_BETWEEN_ACCIDENTALS, m_iInstr, m_idxStaff);
            move_accidental_to_left(pShapeNoteChord, xShift);
        }
        else
        {
            //move note to the left

            //first move the note in block and then determine necessary
            //shifts in accidentals (in chord and note) to avoid collisions

            //TODO: This is incorrect, only moves note accidental but not any
            //chord accidental colliding with notehead
            URect bbox = pAccShapeChord->get_bounds();
            bbox.intersection( pAccShapeNote->get_bounds() );
            if (bbox.width > 0.0f)
            {
                pAccShapeNote->shift_origin(-bbox.width, 0.0f);
                pShapeNote->force_recompute_bounds();
            }
            pShapeNote->increment_anchor_offset(xShift - bbox.width);
        }
    }
    #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
        pShapeNote->set_color(m_colorMoved);     //debug, to visualy identify cases
    #endif
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::move_chord(size_t iStart1, size_t iEnd1,
                                          size_t iStart2, size_t iEnd2, LUnits xShift)
{
    //it is assumed that xShift > 0
    //move chord1 to the right (xShift > 0) of chord2. Accidentals will be re-arranged
    //before chord2.

    //move the chord notes. When moving right, the accidentals remain in place. When
    //moving left, accidentals are moved with the notes
    bool fChord1HasAccidentals = false;
    for (size_t i=iStart1; i <= iEnd1; ++i)
    {
        GmoShapeNote* pNoteShape = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
        move_notehead(pNoteShape, xShift);
        #if (LOMSE_NOTERESTS_COLLISIONS_COLOURED == 1)
            pNoteShape->set_color(m_colorMoved);     //debug, to visualy identify moved shape
        #endif
        fChord1HasAccidentals |= (pNoteShape->get_accidentals_shape() != nullptr);
    }

    //now check and fix accidentals' collisions.
    //If moved chord does not have accidentals, nothing to do
    if (fChord1HasAccidentals)
    {
        //determine if chord2 has accidentals.
        bool fChord2HasAccidentals = false;
        for (size_t i = iStart2; i <= iEnd2; ++i)
        {
            EAccidentals acc = static_cast<ImoNote*>(m_notes[i]->pNR)->get_notated_accidentals();
            if (acc != k_no_accidentals)
            {
                fChord2HasAccidentals = true;
                break;
            }
        }

        if (!fChord2HasAccidentals)
        {
            //nothing to do. Chord1 acidentals do not overlap chord2 accidentals

            //TODO: even if chord2 does not have accidentals, it is necessay to check
            //      the accidentals just in case any collides with a notehead
        }
        else
        {
            //move chord1 accidentals before chord2

            //determine chord2 xLeft
            LUnits xAnchor = 0.0f;
            for (size_t i=iStart2; i <= iEnd2; ++i)
            {
                xAnchor = min(xAnchor, m_notes[i]->pShape->get_anchor_offset());
            }

            //move accidentals before xLeft
            LUnits space = m_pMeter->tenths_to_logical(LOMSE_SPACE_BETWEEN_ACCIDENTALS,
                                                       m_iInstr, m_idxStaff);
            for (size_t i=iStart1; i <= iEnd1; ++i)
            {
                GmoShapeNote* pNoteShape = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
                GmoShapeAccidentals* pAccShape = pNoteShape->get_accidentals_shape();
                if (pAccShape)
                {
                    LUnits xShift = -xAnchor + space;
                    move_accidental_to_left(pNoteShape, xShift);
                }
            }

//TODO: tentative code for re-organizing accidentals. Need some fixes
//            //the order of accidentals is the same as for single-stemmed chords, starting with
//            //the uppermost accidental closest to the notes. Thus, it is necessary to
//            //traverse the notes ordered by pitch
//            list<GmoShapeNote*> notes;
//            for (size_t i=iStart2; i <= iEnd2; ++i)
//            {
//                GmoShapeNote* pNoteShape = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
//                insert_note_in_list(notes, pNoteShape);
//            }
//            for (size_t i=iStart1; i <= iEnd1; ++i)
//            {
//                GmoShapeNote* pNoteShape = static_cast<GmoShapeNote*>(m_notes[i]->pShape);
//                insert_note_in_list(notes, pNoteShape);
//            }
//
//            layout_accidentals(notes);
        }
    }
}

//---------------------------------------------------------------------------------------
size_t NoterestsCollisionsFixer::find_start_of_chord(size_t i)
{
    //given the index to a note of the chord, returns the index to the first note

    int line = m_notes[i]->line;
    size_t iStart = i;
    while ((iStart > 0) && (m_notes[iStart]->line == line))
        --iStart;
    if (m_notes[iStart]->line != line)
        ++iStart;

    return iStart;
}

//---------------------------------------------------------------------------------------
size_t NoterestsCollisionsFixer::find_end_of_chord(size_t i)
{
    //given the index to a note of the chord, returns the index to the last note

    int line = m_notes[i]->line;
    size_t iEnd = i;
    while ((iEnd < m_notes.size()) && (m_notes[iEnd]->line == line))
        ++iEnd;
    --iEnd;

    return iEnd;
}

//---------------------------------------------------------------------------------------
std::pair<size_t, int> NoterestsCollisionsFixer::find_chord_note_that_overlap_with(
                                                    size_t i, size_t iStart, size_t iEnd)
{
    //check if notehead i overlaps any chord note, and returns the index to that note or
    //-1 if no overlap found

    int maxOverlap = int(k_overlap_none);
    size_t iNote = -1;

    for (size_t k=iStart; k <= iEnd; ++k)
    {
        int overlap = check_if_noteheads_overlap(i, k);
        if (maxOverlap <= overlap)
        {
            iNote = k;
            maxOverlap = overlap;
        }
    }

    return make_pair(iNote, maxOverlap);
}

//---------------------------------------------------------------------------------------
int NoterestsCollisionsFixer::check_if_noteheads_overlap(size_t i, size_t j)
{
    //check if two noteheads overlap by comparing its position on the staff.
    //Returns a value from enum ENoteheadOverlap

    int posOnStaff1 = m_notes[i]->posOnStaff;
    int posOnStaff2 = m_notes[j]->posOnStaff;

    if (posOnStaff1 == posOnStaff2)
        return k_overlap_unison;    //Overlap. Unison

    else if (abs(posOnStaff1 - posOnStaff2) == 1)
        return k_overlap_second;    //Overlap. A second appart

    else if (abs(posOnStaff1 - posOnStaff2) == 2)
    {
        //A third appart. Possible false positive. Notes will overlap only when
        //the noteheads have different sizes (e.g. quarter and half notes)
        return k_overlap_third;
    }

    else
        return k_overlap_none;    //overlap is not possible
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::move_notehead(GmoShapeNote* pShapeNote, LUnits xShift)
{
    //move a note to the left side (xShift < 0) or to the right side (xShif > 0) of other
    //note (or chord). When moving to the right, the note accidental is not moved and
    //remains at current position. When moving the note to the right, the note is moved
    //in block with its accidental, if any.
    //In both cases the anchor line is not moved and remains at right side of the
    //accidental

    GmoShape* pShapeAcc = pShapeNote->get_accidentals_shape();
    if (pShapeAcc && xShift > 0.0f)
    {
        //move notehead to the right of the other note or chord. The accidental must
        //remain at current position. The anchor offset does not change so that anchor
        //line remains at accidental right border. To achieve this:
        // - move the composite shape to rigth. This shifts all shapes (notehead, flag,
        //   dots, accidental, etc) to right.
        // - shift the accidental in the opposite direction, so that it remains at
        //   original position.
        // - no need to change anchor offset as current offset sets the anchor line
        //   at accidental right border.
        pShapeNote->shift_origin(xShift, 0.0f); //shift all to right
        pShapeAcc->shift_origin(-xShift, 0.0f); //shift acc to left
        pShapeNote->force_recompute_bounds();
    }
    else
    {
        //move note in block to the left (notehead and accidental, if any).
        pShapeNote->increment_anchor_offset(xShift);
    }
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::move_accidental_to_left(GmoShapeNote* pShapeNote,
                                                       LUnits xShift)
{
    //it is assumed that xShift > 0
    //Increment anchor offset by shift amount to place anchor line at original
    //position (notehead left if stem up or at notehead right if stem down).

    GmoShape* pShapeAcc = pShapeNote->get_accidentals_shape();
    pShapeAcc->shift_origin(-xShift, 0.0f);
    pShapeNote->force_recompute_bounds();   //this recomputes the origin, at acc top/left
    pShapeNote->increment_anchor_offset(-xShift);
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::insert_note_in_list(list<GmoShapeNote*>& notes,
                                                   GmoShapeNote* pNoteShape)
{
    //keep notes sorted by pitch

    if (notes.size() == 0)
    {
	    notes.push_back(pNoteShape);
    }
    else
    {
        int newPos = pNoteShape->get_pos_on_staff();
        std::list<GmoShapeNote*>::iterator it;
        for (it = notes.begin(); it != notes.end(); ++it)
        {
            int curPos((*it)->get_pos_on_staff());
            if (newPos < curPos)
            {
	            notes.insert(it, 1, pNoteShape);
                return;
            }
            else if (newPos == curPos)
            {
                //unison. when this note has a natural keep first note accidental
                //first. For this this note has to be inserted after existing one
                ImoNote* pNote = static_cast<ImoNote*>(pNoteShape->get_creator_imo());
                EAccidentals acc = pNote->get_notated_accidentals();
                if (acc != k_natural)
                    notes.insert(it, pNoteShape);
                else
                {
                    std::list<GmoShapeNote*>::iterator itNext = it;
                    notes.insert(++itNext, pNoteShape);
                }
                return;
            }
        }
	    notes.push_back(pNoteShape);
    }
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::layout_accidentals(list<GmoShapeNote*>& notes)
{
    std::list<GmoShapeNote*>::reverse_iterator it;
    for(it = notes.rbegin(); it != notes.rend(); ++it)
    {
        GmoShapeNote* pNoteShape = (*it);
        GmoShapeAccidentals* pCurAcc = pNoteShape->get_accidentals_shape();

        if (pCurAcc)
        {
            //check if conflict with next two noteheads
            std::list<GmoShapeNote*>::reverse_iterator itCur = it;
            ++itCur;
            if (itCur != notes.rend())
            {
                GmoShapeNotehead* pHead = (*itCur)->get_notehead_shape();
                shift_acc_if_confict_with_shape(pNoteShape, pHead);
                ++itCur;
            }
            if (itCur != notes.rend())
            {
                GmoShapeNotehead* pHead = (*itCur)->get_notehead_shape();
                shift_acc_if_confict_with_shape(pNoteShape, pHead);
            }

            //check if conflict with two previous notes or their accidentals
            if (it != notes.rbegin())
            {
                itCur = it;
                --itCur;
                shift_acc_if_confict_with_shape(pNoteShape, *itCur);
                if (itCur != notes.rbegin())
                {
                    --itCur;
                    shift_acc_if_confict_with_shape(pNoteShape, *itCur);
                }
            }

            //check if conflict with any previous accidental
            shift_accidental_if_conflict_with_previous(pNoteShape, notes, it);
        }
    }
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::shift_accidental_if_conflict_with_previous(
                                        GmoShapeNote* pCurAcc,
                                        list<GmoShapeNote*>& notes,
                                        list<GmoShapeNote*>::reverse_iterator& itCur)
{
	std::list<GmoShapeNote*>::reverse_iterator it;
	for(it = notes.rbegin(); it != itCur; ++it)
	{
		GmoShapeAccidentals* pPrevAcc = (*it)->get_accidentals_shape();
        if (pPrevAcc)
            shift_acc_if_confict_with_shape(pCurAcc, pPrevAcc);
	}
}

//---------------------------------------------------------------------------------------
void NoterestsCollisionsFixer::shift_acc_if_confict_with_shape(GmoShapeNote* pShapeNote,
                                                    GmoShape* pShape)
{
    GmoShapeAccidentals* pCurAcc = pShapeNote->get_accidentals_shape();
    LUnits xOverlap = check_if_overlap(pShape, pCurAcc);
    if (xOverlap > 0.0f)
    {
        LUnits space = m_pMeter->tenths_to_logical(LOMSE_SPACE_BETWEEN_ACCIDENTALS,
                                                   m_iInstr, m_idxStaff);
        LUnits shift = pCurAcc->get_right() - pShape->get_left();
        move_accidental_to_left(pShapeNote, shift + space);
    }
}

//---------------------------------------------------------------------------------------
LUnits NoterestsCollisionsFixer::check_if_overlap(GmoShape* pShape, GmoShape* pNewShape)
{
    URect overlap = pShape->get_bounds();
    overlap.intersection( pNewShape->get_bounds() );
    return overlap.get_width();
}



}  //namespace lomse

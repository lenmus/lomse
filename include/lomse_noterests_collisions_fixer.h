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

#ifndef __LOMSE_NOTERESTS_COLLISIONS_FIXER_H__        //to avoid nested includes
#define __LOMSE_NOTERESTS_COLLISIONS_FIXER_H__

#include "lomse_basic.h"

#include <vector>
#include <list>

namespace lomse
{

//forward declarations
class ColStaffObjsEntry;
class GmoShape;
class GmoShapeAccidentals;
class GmoShapeNote;
class ImoNoteRest;
class ScoreMeter;


//---------------------------------------------------------------------------------------
// NoterestsCollisionsFixer
// Auxiliary class responsible for detecting and fixing overlaps between notes/rests on
// the same staff, at the same timepos, caused by voices overlap
class NoterestsCollisionsFixer
{
protected:
    struct NoterestData         //aux. for holding info. about a note/rest
    {
        ImoNoteRest* pNR;
        GmoShape* pShape;
        int line;
        int posOnStaff;
        int type;

        NoterestData(ImoNoteRest* pImo, GmoShape* pS, int pos)
            : pNR(pImo), pShape(pS), line(0), posOnStaff(pos)
            , type(k_nrdata_unknown) {}

        //helper
        inline bool is_rest() { return type == k_nrdata_rest; }
        inline bool is_note() { return type == k_nrdata_note; }
        inline bool is_chord() { return type == k_nrdata_chord; }
    };

    enum ENoterestDataType
    {
        //AWARE: values are used in 'NoterestsCollisionsFixer::fix_spacing_issues()' to
        // determine collision type. Do not change values without reviewing this method!
        k_nrdata_unknown = 0,
        k_nrdata_rest = 1,          //it is a rest
        k_nrdata_note = 2,          //it is a note (isolated note not part of a chord)
        k_nrdata_chord = 3,         //it is a chord (note included in a chord)
    };

    std::vector<NoterestData*> m_notes;
    bool m_fMoreThanOneLine = false;
    int m_iFirstLine = 0;
    ScoreMeter* m_pMeter;
    int m_iInstr = 0;
    int m_idxStaff = 0;

public:
    NoterestsCollisionsFixer(GmoShape* pShape, ColStaffObjsEntry* pEntry,
                             ScoreMeter* pMeter);
    ~NoterestsCollisionsFixer();

    NoterestsCollisionsFixer(const NoterestsCollisionsFixer&) = delete;
    NoterestsCollisionsFixer& operator= (const NoterestsCollisionsFixer&) = delete;
    NoterestsCollisionsFixer(NoterestsCollisionsFixer&&) = delete;
    NoterestsCollisionsFixer& operator= (NoterestsCollisionsFixer&&) = delete;

    void add_noterest(GmoShape* pShape, ColStaffObjsEntry* pEntry);
    void fix_spacing_issues();

protected:

    enum ENoteheadOverlap
    {
        //AWARE: ordered by overlap size none < third < second < unison, for comparisons

        k_overlap_none = 0,     //no overlap
        k_overlap_third,        //posible false positive. One third  appart
        k_overlap_second,       //Overlap. A second appart
        k_overlap_unison,       //Overlap. Unison
    };

    void fix_two_rests_overlap(size_t i, size_t j);
    void fix_note_and_rest_overlap(size_t i, size_t j);
    void fix_chord_and_rest_overlap(size_t i, size_t j);
    void fix_two_notes_overlap(size_t i, size_t j);
    void fix_chord_and_note_overlap(size_t i, size_t j);
    void fix_two_chords_overlap(size_t i, size_t j);

    void move_note_and_accidentals(size_t i, size_t j, LUnits xShift);
    void move_note_and_chord_accidentals(size_t iNote, size_t iStart, size_t iEnd,
                                         LUnits xShift);
    void move_chord_and_note_accidentals(size_t iNote, size_t iStart, size_t iEnd,
                                         LUnits xShift);
    void move_chord(size_t iStart1, size_t iEnd1, size_t iStart2, size_t iEnd2,
                    LUnits xShift);
    void move_rest(size_t i, LUnits yShift);

    void move_notehead(GmoShapeNote* pShapeNote, LUnits xShift);
    void move_accidental_to_left(GmoShapeNote* pShapeNote, LUnits xShift);

    void insert_note_in_list(std::list<GmoShapeNote*>& notes, GmoShapeNote* pNoteShape);
    void layout_accidentals(std::list<GmoShapeNote*>& notes);
    void shift_accidental_if_conflict_with_previous(GmoShapeNote* pCurAcc,
                                      std::list<GmoShapeNote*>& notes,
                                      std::list<GmoShapeNote*>::reverse_iterator& itCur);
    void shift_acc_if_confict_with_shape(GmoShapeNote* pCurAcc, GmoShape* pShape);
    LUnits check_if_overlap(GmoShape* pShape, GmoShape* pNewShape);


    size_t find_start_of_chord(size_t i);
    size_t find_end_of_chord(size_t i);
    std::pair<size_t, int> find_chord_note_that_overlap_with(size_t i, size_t iStart, size_t iEnd);
    int check_if_noteheads_overlap(size_t i, size_t j);

};


}   //namespace lomse

#endif    // __LOMSE_NOTERESTS_COLLISIONS_FIXER_H__


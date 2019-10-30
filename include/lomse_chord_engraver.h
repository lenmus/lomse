//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#ifndef __LOMSE_CHORD_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_CHORD_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class ImoNote;
class ImoChord;
class ImoRelObj;
class GmoShapeNote;
class GmoShapeAccidentals;

//---------------------------------------------------------------------------------------
class ChordEngraver : public RelObjEngraver
{
public:
    //public for unit tests
    struct ChordNoteData
    {
        ImoNote* pNote;
        GmoShapeNote* pNoteShape;
        int posOnStaff;
        int iInstr;
        bool fNoteheadReversed;

        ChordNoteData(ImoNote* pN, GmoShapeNote* pNS, int pos, int instr)
            : pNote(pN), pNoteShape(pNS), posOnStaff(pos), iInstr(instr)
            , fNoteheadReversed(false) {}
    };


protected:
    ImoChord* m_pChord;

    std::list<ChordNoteData*> m_notes;
    ChordNoteData* m_pBaseNoteData;
    bool m_fStemDown;
    bool m_fCrossStaffChord;

    bool m_fHasStem;
    bool m_fHasFlag;
    bool m_fSomeNoteReversed;
    int m_noteType;
    LUnits m_stemWidth;
    int m_numNotesMissing;


public:
    ChordEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter, int numNotes);
    virtual ~ChordEngraver();

    //implementation of virtual methods from RelObjEngraver
    void set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol,
                            LUnits xStaffLeft, LUnits xStaffRight, LUnits yTop,
                            int idxStaff, VerticalProfile* pVProfile) override;
    void set_middle_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                             GmoShape* pStaffObjShape, int iInstr, int iStaff,
                             int iSystem, int iCol,
                             LUnits xStaffLeft, LUnits xStaffRight, LUnits yTop,
                             int idxStaff, VerticalProfile* pVProfile) override;
    void set_end_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol,
                          LUnits xStaffLeft, LUnits xStaffRight, LUnits yTop,
                          int idxStaff, VerticalProfile* pVProfile) override;
    int create_shapes(Color color=Color(0,0,0));

    inline int notes_missing() { return m_numNotesMissing; }


protected:
    void add_note(ImoStaffObj* pSO, GmoShape* pStaffObjShape, int idxStaff);

    void decide_on_stem_direction();
    void layout_noteheads();
    void layout_accidentals();
    void add_stem_and_flag();
    void set_anchor_offset();

    void align_noteheads();
    void arrange_notheads_to_avoid_collisions();

    void reverse_notehead(GmoShapeNote* pNoteShape);
    void shift_accidental_if_conflict_with_previous(GmoShapeAccidentals* pCurAcc,
                                      std::list<ChordNoteData*>::reverse_iterator& itCur);
    void shift_acc_if_confict_with_shape(GmoShapeAccidentals* pCurAcc, GmoShape* pShape);
    LUnits check_if_overlap(GmoShape* pShape, GmoShape* pNewShape);

    LUnits check_if_accidentals_overlap(GmoShapeAccidentals* pPrevAcc,
                                        GmoShapeAccidentals* pCurAcc);

    //helpers
    inline bool has_stem() { return m_fHasStem; }
    inline bool is_stem_down() { return m_fStemDown; }
    inline bool has_flag() { return m_fHasFlag; }
    bool is_chord_beamed();
    inline ImoNote* get_min_note() { return m_notes.front()->pNote; }
    inline ImoNote* get_max_note() { return m_notes.back()->pNote; }
	inline ImoNote* get_base_note() { return m_pBaseNoteData->pNote; }
    inline bool is_stem_up() { return !m_fStemDown; }

};


}   //namespace lomse

#endif    // __LOMSE_CHORD_ENGRAVER_H__


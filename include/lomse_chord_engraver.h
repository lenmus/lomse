//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
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
class GmoShapeNote;
class GmoShapeAccidentals;

//---------------------------------------------------------------------------------------
class ChordEngraver : public Engraver
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
    std::list<ChordNoteData*> m_notes;
    ChordNoteData* m_pBaseNoteData;
    bool m_fStemDown;

public:
    ChordEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    virtual ~ChordEngraver();

    void add_note(ImoNote* pNote, GmoShapeNote* pNoteShape, int posOnStaff, int iInstr);
    void layout_chord();

    //should be protected but made public fot unit tests
    inline ImoNote* get_min_note() { return m_notes.front()->pNote; }
    inline ImoNote* get_max_note() { return m_notes.back()->pNote; }
	inline ImoNote* get_base_note() { return m_pBaseNoteData->pNote; }
    inline bool is_stem_up() { return !m_fStemDown; }

    //Only for unit tests
    inline std::list<ChordNoteData*>& get_notes() { return m_notes; }


private:
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
    LUnits tenths_to_logical(Tenths value);


    inline bool has_stem() { return m_fHasStem; }
    inline bool is_stem_down() { return m_fStemDown; }
    inline bool has_flag() { return m_fHasFlag; }
    bool is_chord_beamed();

    bool m_fHasStem;
    bool m_fHasFlag;
    bool m_fSomeNoteReversed;
    int m_noteType;
    int m_iInstr;
    int m_iStaff;
    LUnits m_stemWidth;
};


}   //namespace lomse

#endif    // __LOMSE_CHORD_ENGRAVER_H__


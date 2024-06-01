//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_CHORD_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_CHORD_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

#include <list>
#include <vector>

namespace lomse
{

//forward declarations
class GmoShapeAccidentals;
class GmoShapeNote;
class ImoBeam;
class ImoChord;
class ImoNote;
class ImoRelObj;
class StaffObjsCursor;
class StemFlagEngraver;
class VoiceRelatedShape;

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
    bool m_fHasStem;
    bool m_fHasFlag;
    bool m_fSomeNoteReversed;
    int m_noteType;
    LUnits m_stemWidth;
    int m_numNotesMissing;
    double m_fontSize;
    int m_symbolSize;
    LUnits m_uStemThickness;
    LUnits m_uxStem;                //stem x position
    std::vector<int> m_clefs;       //applicable clef for each staff

    ChordNoteData* m_pFlagNoteData;     //Flag note is the note that has the flag: the
                                        //min pitch note for stem down or the max pitch
                                        //note for stem up.

    ChordNoteData* m_pStartNoteData;    //Start note is opposite one: the max pitch note
                                        //for stem down or the min pitch note for stem up.

    ChordNoteData* m_pLinkNoteData;     //for cross-staff chords, the last note in the
                                        //same staff than the flag note

    bool m_fSomeAccidentalsShifted;

public:
    ChordEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter, int numNotes,
                  double fontSize, int symbolSize);
    virtual ~ChordEngraver();

    //implementation of virtual methods from RelObjEngraver
    void set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;
    void set_middle_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;
    void set_end_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;
    int create_shapes(Color color=Color(0,0,0));

    void save_applicable_clefs(StaffObjsCursor* pCursor, int iInstr);

    inline int notes_missing() { return m_numNotesMissing; }

    //static methods
    static bool decide_stem_direction(ImoNote* pBaseNote, ImoChord* pChord, int meanPos);


protected:
    void add_note(ImoStaffObj* pSO, GmoShape* pStaffObjShape, int idxStaff);

    void decide_stem_direction();
    void find_reference_notes();
    void layout_noteheads();
    void layout_accidentals();
    void layout_arpeggio();
    void determine_stem_x_left();
    void determine_stem_flag_color();
    void add_stem_and_flag();
    void add_stem_flag_segment(StemFlagEngraver* engrv);
    void add_stem_link_segment();
    void add_stem_extensible_segment_if_required();
    void add_stroke_for_graces_if_required(StemFlagEngraver* engrv);
    void set_anchor_offset();
    void add_voice(VoiceRelatedShape* pVRS);

    void align_noteheads();
    void arrange_notheads_to_avoid_collisions();

    void reverse_notehead(GmoShapeNote* pNoteShape);
    void shift_accidental_if_conflict_with_previous(GmoShapeAccidentals* pCurAcc,
                                      std::list<ChordNoteData*>::reverse_iterator& itCur);
    void shift_acc_if_confict_with_shape(GmoShapeAccidentals* pCurAcc, GmoShape* pShape);
    LUnits check_if_overlap(GmoShape* pShape, GmoShape* pNewShape);

    LUnits check_if_accidentals_overlap(GmoShapeAccidentals* pPrevAcc,
                                        GmoShapeAccidentals* pCurAcc);

    void ensure_note_bounds_updated();

    //helpers
    inline bool has_stem() { return m_fHasStem; }
    inline bool is_stem_down() { return m_fStemDown; }
    inline bool has_flag() { return m_fHasFlag; }
    bool is_chord_beamed();
    inline ImoNote* get_min_note() { return m_notes.front()->pNote; }
    inline ImoNote* get_max_note() { return m_notes.back()->pNote; }
	inline ImoNote* get_base_note() { return m_pBaseNoteData->pNote; }
    inline bool is_stem_up() { return !m_fStemDown; }
    bool is_tablature();

};


//---------------------------------------------------------------------------------------
/** BeamedChordHelper: helper class to wrap the algorithm and methods to decide
    beam position (above, below) and chords stems direction for beamed chords.

    This class is only used by ChordEngraver and all methods are easily
*/
class BeamedChordHelper
{
protected:
    size_t m_numStaves;
    int m_nUpForced;                    //number of forced chords/single notes up
    int m_nDownForced;                  //number of forced chords/single notes down
    bool m_fTransferred = false;        //ownership of m_pStemsDir transferred to m_pBeam
    ImoBeam* m_pBeam;                   //the beam to compute
    std::vector<int>* m_pClefs;         //current clefs for each staff
    std::vector<ImoChord*> m_chords;     //the beamed chords
    std::vector<int> m_totalPosOnStaff;  //sum of notes pos. for each staff
    std::vector<int> m_numNotes;         //num notes used to determine oposOnStaff, per staff
    std::vector<int> m_maxPosOnStaff;    //max pos on staff, per staff
    std::vector<int> m_minPosOnStaff;    //min pos on staff, per staff
    std::vector<int>* m_pStemsDir;       //stem directions for each chord/single notes.
                                         //  For rests, value is k_computed_stem_none

public:
    BeamedChordHelper(ImoBeam* pBeam, std::vector<int>* pLastClefs);
    ~BeamedChordHelper();


    //methods and variables to determine stems direction for beamed chords


    bool compute_stems_directions();
    vector<ImoNote*> collect_chord_notes(ImoChord* pChord);
    bool compute_stem_direction_for_chord(ImoNote* pBaseNote, ImoNote* pLastNote,
                                          std::vector<int>* pLastClefs);
    bool compute_stem_direction_for_note(ImoNote* pNote, ImoNote* pLastNote,
                                         std::vector<int>* pLastClefs);
    int determine_mean_pos_on_staff(std::vector<ImoNote*>& chordNotes,
                                    std::vector<int>* pClefs);
    void find_applicable_clefs(ImoNote* pBaseNote, ImoNote* pLastNote,
                               std::vector<int>* pLastClefs);
    int determine_beam_position();
    bool apply_stem_direction_rules_for_staff(int iStaff);
    bool apply_stem_direction_rules_for_both_staves();
    void transfer_stem_directions_to_notes(ImoBeam* pBeam, int beamPos);

};


}   //namespace lomse

#endif    // __LOMSE_CHORD_ENGRAVER_H__


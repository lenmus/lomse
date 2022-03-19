//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_NOTE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_NOTE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_im_note.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class EngraversMap;
class FontStorage;
class GmoShape;
class GmoShapeAccidentals;
class GmoShapeFlag;
class GmoShapeNote;
class GmoShapeNotehead;
class GmoShapeStem;
class ScoreMeter;
class StaffObjsCursor;
class VoiceRelatedShape;

//---------------------------------------------------------------------------------------
class NoteEngraver : public StaffObjEngraver
{
protected:
    ImoNote* m_pNote;
    int m_clefType;
    int m_octaveShift;
    int m_idxStaff;
    int m_symbolSize;
    EngraversMap* m_pEngravers;

    bool m_fStemDown;
    int m_nPosOnStaff;
    LUnits m_uyStaffTopLine;
    LUnits m_uxLeft, m_uyTop;       //current position
    GmoShapeNote* m_pNoteShape;
    GmoShapeNotehead* m_pNoteheadShape;
    GmoShapeAccidentals* m_pAccidentalsShape;
    StaffObjsCursor* m_pCursor;
    int m_nDots;
    int m_noteType;
    EAccidentals m_acc;

    LUnits m_lineSpacing;
    Color m_color;
    double m_fontSize;

public:
    NoteEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 EngraversMap* pEngravers, int iInstr, int iStaff);
    virtual ~NoteEngraver() {}

    GmoShape* create_shape(ImoNote* pNote, int clefType, int octaveShift, UPoint uPos,
                           StaffObjsCursor* pCursor=nullptr, Color color=Color(0,0,0));
    GmoShape* create_tool_dragged_shape(int noteType, EAccidentals acc, int dots);
    UPoint get_drag_offset();

    void add_to_chord_if_in_chord();

    //static methods
    static Tenths get_standard_stem_length(int nPosOnStaff, bool fStemDown);
    static int pitch_to_pos_on_staff(ImoNoteRest* pNR, int clefType, int octaveShift);

protected:
    void create_shape();
    void add_notehead_shape();
    void add_stem_and_flag_if_required();
    void add_leger_lines_if_necessary();
    void add_shapes_for_dots_if_required();
    void add_stroke_to_grace_note_if_required();

    void add_voice(VoiceRelatedShape* pVRS);
    void determine_stem_direction();
    int decide_notehead_type();
    void add_shapes_for_accidentals_if_required();
    LUnits add_dot_shape(LUnits x, LUnits y, Color color);
    int get_glyph_for_notehead(int notehead);
    int get_glyph_for_tablature();
    int get_glyphs_for_accidentals();
    LUnits get_pitch_shift();
    int get_pos_on_staff();
    static int pos_for_top_ledger_line(int numLines);
    static int pos_for_bottom_ledger_line(int numLines);

    void create_chord();
    void add_to_chord();
    void layout_chord();
    inline bool is_in_chord() { return m_pNote && m_pNote->is_in_chord(); }
    inline bool is_last_note_of_chord() { return m_pNote && m_pNote->is_end_of_chord(); }

    //helper
    inline bool has_stem() { return m_noteType >= k_half && !m_pNote->is_stem_none(); }
    inline bool has_flag() { return m_noteType >= k_eighth && !m_pNote->is_stem_none(); }
    inline bool is_beamed() { return m_pNote && m_pNote->is_beamed(); }
    Tenths get_glyph_offset(int iGlyph);
    inline bool is_tablature() { return m_clefType == k_clef_TAB; }


    //overrides for Engraver
    double determine_font_size() override;

};

//---------------------------------------------------------------------------------------
class StemFlagEngraver : public StaffSymbolEngraver
{
protected:
    int m_noteType;
    bool m_fStemDown;
    bool m_fWithFlag;
    bool m_fShortFlag;
    bool m_fHasBeam;
    bool m_fNoteheadReversed;
    LUnits m_uStemLength;
    Color m_color;
    double m_fontSize;
    ImoObj* m_pCreatorImo;
    GmoShapeNote* m_pNoteShape;
    GmoShapeFlag* m_pFlagShape;
    GmoShapeNotehead* m_pNoteheadShape;

    LUnits m_uStemThickness;
    LUnits m_uxStem;
    LUnits m_yStemTop;          //top of fixed/extensible when up/down, respectively
    LUnits m_yStemBottom;       //bottom of extensible/fixed when up/down, respectively

public:
    StemFlagEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                     ImoObj* pCreatorImo, int iInstr, int iStaff, double fontSize=0.0);
    virtual ~StemFlagEngraver() {}

    void add_stem_flag_to_note(GmoShapeNote* pNoteShape, int noteType, bool fStemDown,
                               bool fWithFlag, bool fShortFlag, bool fHasBeam,
                               LUnits stemLength, bool fNoteheadReversed, Color color);

    void add_stroke_shape();

protected:
    void determine_stem_x_left();
    void determine_stem_y_pos();
    void add_stem_shape();
    void add_flag_shape_if_required();
    void add_voice(VoiceRelatedShape* pVRS);

    LUnits get_glyph_offset(int iGlyph);
    int get_glyph_for_flag();

};


}   //namespace lomse

#endif    // __LOMSE_NOTE_ENGRAVER_H__


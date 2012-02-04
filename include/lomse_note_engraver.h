//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_NOTE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_NOTE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_noterest_engraver.h"
#include "lomse_im_note.h"

namespace lomse
{

//forward declarations
class GmoShape;
class FontStorage;
class GmoShapeNotehead;
class GmoShapeNote;
class GmoShapeStem;
class GmoShapeAccidentals;
class GmoShapeFlag;
class ScoreMeter;
class ShapesStorage;

//---------------------------------------------------------------------------------------
class NoteEngraver : public NoterestEngraver
{
protected:
    ImoNote* m_pNote;
    int m_clefType;

public:
    NoteEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 ShapesStorage* pShapesStorage);
    virtual ~NoteEngraver() {}

    GmoShape* create_shape(ImoNote* pNote, int iInstr, int iStaff, int clefType,
                           UPoint uPos);

    static Tenths get_standard_stem_length(int nPosOnStaff, bool fStemDown);
    void add_to_chord_if_in_chord();

protected:
    void add_notehead_shape();
    void add_stem_and_flag_if_required();
    void add_leger_lines_if_necessary();
    void add_shapes_for_dots_if_required();

    void determine_stem_direction();
    int decide_notehead_type();
    void add_shapes_for_accidentals_if_required();
    LUnits add_dot_shape(LUnits x, LUnits y, Color color);
    int get_glyph_for_notehead(int notehead);
    int get_glyphs_for_accidentals();
    LUnits get_pitch_shift();
    int pitch_to_pos_on_staff(int clefType);
    int get_pos_on_staff();
    double determine_font_size();

    void create_chord();
    void add_to_chord();
    void layout_chord();
    inline bool is_in_chord() { return m_pNote->is_in_chord(); }
    inline bool is_last_note_of_chord() { return m_pNote->is_end_of_chord(); }

    //helper
    inline bool has_stem() { return m_pNote->get_note_type() >= k_half; }
    inline bool has_flag() { return m_pNote->get_note_type() >= k_eighth; }
    Tenths get_glyph_offset(int iGlyph);
    LUnits tenths_to_logical(Tenths tenths);


    bool m_fStemDown;
    int m_nPosOnStaff;
    LUnits m_uyStaffTopLine;
    LUnits m_uxLeft, m_uyTop;       //current position
    GmoShapeNote* m_pNoteShape;
    GmoShapeNotehead* m_pNoteheadShape;
    GmoShapeAccidentals* m_pAccidentalsShape;
};

//---------------------------------------------------------------------------------------
class StemFlagEngraver : public Engraver
{
protected:
    int m_iInstr;
    int m_iStaff;
    GmoShapeNote* m_pNoteShape;
    GmoShapeNote* m_pBaseNoteShape;
    int m_noteType;
    bool m_fStemDown;
    bool m_fWithFlag;
    LUnits m_uStemLength;
    Color m_color;
    double m_fontSize;
    ImoObj* m_pCreatorImo;

public:
    StemFlagEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                     ImoObj* pCreatorImo, int iInstr, int iStaff);
    virtual ~StemFlagEngraver() {}

    void add_stem_flag(GmoShapeNote* pNoteShape, GmoShapeNote* pBaseNoteShape,
                       int noteType, bool fStemDown, bool fWithFlag,
                       LUnits stemLength, Color color);

protected:
    void determine_position_and_size_of_stem();
    void add_stem_shape();
    void add_flag_shape_if_required();

    void determine_stem_x_left();
    void determine_stem_y_note();
    void cut_stem_size_to_add_flag();

    LUnits m_uStemThickness;
    LUnits m_uxStem;
    LUnits m_uyStemNote;        //the nearest point to notehead
    LUnits m_uyStemFlag;        //the nearest point to the flag
    GmoShapeNotehead* m_pNoteheadShape;

    LUnits get_glyph_offset(int iGlyph);
    int get_glyph_for_flag();
    LUnits tenths_to_logical(Tenths value);
    double determine_font_size();

};


}   //namespace lomse

#endif    // __LOMSE_NOTE_ENGRAVER_H__


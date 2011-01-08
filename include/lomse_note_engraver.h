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

#ifndef __LOMSE_NOTE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_NOTE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_im_note.h"

namespace lomse
{

//forward declarations
//class ImoNote;
class GmoBoxSliceInstr;
class GmoShape;
class GmoBox;
class FontStorage;
class GmoShapeNotehead;
class GmoShapeNote;
class GmoShapeStem;

//---------------------------------------------------------------------------------------
class NoteEngraver
{
protected:
    LibraryScope& m_libraryScope;

    //data for note being engraved
    ImoNote* m_pNote;
    int m_clefType;
    LUnits m_lineSpacing;
    Color m_color;
    bool m_fNoteEngraved;
    int m_nPosOnStaff;
    LUnits m_uxLeft, m_uyTop;       //current position
    LUnits m_uyFlag;
    LUnits m_uStemThickness;
    LUnits m_uxStem;
    LUnits m_uyStemStart;           //the nearest point to notehead
    LUnits m_uyStemEnd;             //the nearest point to the flag
    LUnits m_uStemLength;
    GmoShapeNote* m_pNoteShape;
    GmoShapeNotehead* m_pNoteheadShape;
    GmoShapeStem* m_pStemShape;

public:
    NoteEngraver(LibraryScope& libraryScope);
    virtual ~NoteEngraver() {}

    GmoShape* create_shape(ImoNote* pNote, int clefType, UPoint uPos, LUnits lineSpacing);

protected:
    Tenths get_glyph_offset(int iGlyph);
    inline LUnits tenths_to_logical(Tenths tenths) {
        return (tenths * m_lineSpacing) / 10.0f;
    }
    void add_space_before_note();
    bool add_notehead_shape();
    int decide_notehead_type();
    void add_shapes_for_dots_if_required();
    LUnits add_dot_shape(LUnits x, LUnits y, Color color);
    void determine_stem_x_pos();
    void determine_stem_y_start();
    void set_position_and_size_of_stem();
    void add_shape_for_flag_if_required();
    void add_shapes_for_accidentals_if_required();
    void add_flag_shape(UPoint uPos, Color color);
    int get_glyph_for_notehead(int notehead);
    int get_glyph_for_flag();
    LUnits get_stem_x_left();
    LUnits get_stem_x_right();
    LUnits get_pitch_shift();
    int get_pos_on_staff();
    LUnits get_standard_stem_length();

    //helper
    inline bool is_beamed() { return m_pNote->is_beamed(); }
    inline bool is_in_chord() { return m_pNote->is_in_chord(); }
    inline bool has_stem() { return m_pNote->get_note_type() >= ImoNoteRest::k_quarter; }
    inline bool has_flag() { return m_pNote->get_note_type() >= ImoNoteRest::k_eighth; }

};


}   //namespace lomse

#endif    // __LOMSE_NOTE_ENGRAVER_H__


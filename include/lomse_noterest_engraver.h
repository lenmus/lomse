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

#ifndef __LOMSE_NOTEREST_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_NOTEREST_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_im_note.h"

namespace lomse
{

//forward declarations
class GmoShape;
class FontStorage;
class GmoShapeBeam;
class GmoShapeTuplet;
class ScoreMeter;
class ShapesStorage;
class BeamEngraver;
class TupletEngraver;

//---------------------------------------------------------------------------------------
class NoterestEngraver : public Engraver
{
protected:
    ShapesStorage* m_pShapesStorage;
    int m_iInstr;
    int m_iStaff;
    LUnits m_lineSpacing;
    Color m_color;
    double m_fontSize;
    ImoNoteRest* m_pNoteRest;
    GmoShape* m_pNoteRestShape;

public:
    virtual ~NoterestEngraver() {}


protected:
    NoterestEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 ShapesStorage* pShapesStorage);

    void layout_tuplet();
    double determine_font_size();

    inline bool is_beamed() { return m_pNoteRest->is_beamed(); }

    //helper
    inline bool has_stem() { return m_pNoteRest->get_note_type() >= k_half; }
    inline bool has_flag() { return m_pNoteRest->get_note_type() >= k_eighth; }
    Tenths get_glyph_offset(int iGlyph);
    LUnits tenths_to_logical(Tenths tenths);


    LUnits m_uyStaffTopLine;
    LUnits m_uxLeft, m_uyTop;       //current position
    GmoShapeBeam* m_pStartBeamShape;
    GmoShapeBeam* m_pEndBeamShape;
    GmoShapeTuplet* m_pTupletShape;
};


}   //namespace lomse

#endif    // __LOMSE_NOTEREST_ENGRAVER_H__


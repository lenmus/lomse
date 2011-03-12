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

#ifndef __LOMSE_REST_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_REST_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_noterest_engraver.h"

namespace lomse
{

//forward declarations
class ImoRest;
class GmoShapeRest;
class ScoreMeter;
class ShapesStorage;
class ImoRest;
class GmoShapeBeam;

//---------------------------------------------------------------------------------------
class RestEngraver : public NoterestEngraver
{
protected:
    int m_restType;
    int m_numDots;
    ImoRest* m_pRest;
    ImoObj* m_pCreatorImo;

public:
    RestEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 ShapesStorage* pShapesStorage);
    ~RestEngraver() {}

    GmoShapeRest* create_shape(ImoObj* pCreatorImo, int iInstr, int iStaff, UPoint uPos,
                               int restType, int numDots, ImoRest* pRest);

protected:
    void determine_position();
    void create_main_shape();
    void add_shapes_for_dots_if_required();
    void add_to_beam_if_beamed();

    LUnits tenths_to_logical(Tenths tenths);
    int find_glyph();
    LUnits get_glyph_offset(int iGlyph);
    LUnits add_dot_shape(LUnits x, LUnits y, Color color);

    ImoBeam* get_beam() { return m_pRest->get_beam(); }
    ImoTuplet* get_tuplet() { return m_pRest->get_tuplet(); }

    LUnits m_uxLeft, m_uyTop;       //current position
    int m_iGlyph;
    GmoShapeRest* m_pRestShape;
};


}   //namespace lomse

#endif    // __LOMSE_REST_ENGRAVER_H__


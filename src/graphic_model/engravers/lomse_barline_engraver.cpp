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

#include "lomse_barline_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_shape_barline.h"
#include "lomse_box_slice_instr.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// BarlineEngraver implementation
//---------------------------------------------------------------------------------------
BarlineEngraver::BarlineEngraver(LibraryScope& libraryScope)
    : m_libraryScope(libraryScope)
{
}

//---------------------------------------------------------------------------------------
GmoShape* BarlineEngraver::create_shape(ImoBarline* pBarline, LUnits xPos, LUnits yTop,
                                        LUnits yBottom, LUnits lineSpacing)
{
    m_nBarlineType = pBarline->get_type();
    m_lineSpacing = lineSpacing;
    LUnits thinLineWidth = tenths_to_logical(LOMSE_THIN_LINE_WIDTH);    // thin line width
    LUnits thickLineWidth = tenths_to_logical(LOMSE_THICK_LINE_WIDTH);  // thick line width
    LUnits spacing = tenths_to_logical(LOMSE_LINES_SPACING);            // space between lines: 4 tenths
    LUnits radius = tenths_to_logical(LOMSE_BARLINE_RADIOUS);           // dots radius: 2 tenths

    //force selection rectangle to have at least a width of half line (5 tenths)
    LUnits uMinWidth = tenths_to_logical(5.0f);

    GmoShape* pShape = new GmoShapeBarline(0, m_nBarlineType, xPos, yTop, yBottom,
                                           thinLineWidth, thickLineWidth, spacing,
                                           radius, Color(0,0,0), uMinWidth);

    return pShape;
}



}  //namespace lomse

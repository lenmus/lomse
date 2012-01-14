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
#include "lomse_score_meter.h"
#include "lomse_shapes.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// BarlineEngraver implementation
//---------------------------------------------------------------------------------------
BarlineEngraver::BarlineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : Engraver(libraryScope, pScoreMeter)
{
}

//---------------------------------------------------------------------------------------
GmoShape* BarlineEngraver::create_shape(ImoBarline* pBarline, int iInstr, LUnits xPos,
                                        LUnits yTop, LUnits yBottom)
{
    m_nBarlineType = pBarline->get_type();
    LUnits thinLineWidth = m_pMeter->tenths_to_logical(LOMSE_THIN_LINE_WIDTH, iInstr, 0);
    LUnits thickLineWidth = m_pMeter->tenths_to_logical(LOMSE_THICK_LINE_WIDTH, iInstr, 0);
    LUnits spacing = m_pMeter->tenths_to_logical(LOMSE_LINES_SPACING, iInstr, 0);
    LUnits radius = m_pMeter->tenths_to_logical(LOMSE_BARLINE_RADIOUS, iInstr, 0);

    //force selection rectangle to have at least a width of half line (5 tenths)
    LUnits uMinWidth = 0;   //m_pMeter->tenths_to_logical(5.0f, iInstr, 0);

    return LOMSE_NEW GmoShapeBarline(pBarline, 0, m_nBarlineType, xPos, yTop,
                               yBottom,
                               thinLineWidth, thickLineWidth, spacing,
                               radius, Color(0,0,0), uMinWidth);
}



}  //namespace lomse

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

#include "lomse_line_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_shape_line.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// LineEngraver implementation
//---------------------------------------------------------------------------------------
LineEngraver::LineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : Engraver(libraryScope, pScoreMeter)
{
}

//---------------------------------------------------------------------------------------
GmoShapeLine* LineEngraver::create_shape(ImoScoreLine* pLine, int iInstr,
                                         int iStaff, UPoint pos,
                                         GmoShape* pParentShape)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pLine = pLine;
    m_pParentShape = pParentShape;

    pos = compute_location(pos);
    UPoint start(pos.x + tenths_to_logical(pLine->get_x_start()),
                 pos.y + tenths_to_logical(pLine->get_y_start()) );
    UPoint end(pos.x + tenths_to_logical(pLine->get_x_end()),
               pos.y + tenths_to_logical(pLine->get_y_end()) );

    m_pLineShape = LOMSE_NEW GmoShapeLine(pLine, 0,
                                          start.x, start.y,
                                          end.x, end.y,
                                          tenths_to_logical(pLine->get_line_width()),
                                          0.5f,
                                          pLine->get_line_style(),
                                          pLine->get_line_color(),
                                          pLine->get_start_edge(),
                                          pLine->get_start_cap(),
                                          pLine->get_end_cap() );
    return m_pLineShape;
}

//---------------------------------------------------------------------------------------
UPoint LineEngraver::compute_location(UPoint pos)
{
    pos.y += tenths_to_logical(55.5f);

	return pos;
}

//---------------------------------------------------------------------------------------
LUnits LineEngraver::tenths_to_logical(Tenths tenths)
{
    return m_pMeter->tenths_to_logical(tenths, m_iInstr, m_iStaff);
}


}  //namespace lomse

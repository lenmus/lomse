//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
LineEngraver::LineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           int UNUSED(iInstr), int UNUSED(iStaff))
    : Engraver(libraryScope, pScoreMeter)
{
}

//---------------------------------------------------------------------------------------
GmoShapeLine* LineEngraver::create_shape(ImoScoreLine* pLine, UPoint pos)
{
    UPoint start(pos.x + tenths_to_logical(pLine->get_x_start()),
                 pos.y + tenths_to_logical(pLine->get_y_start()) );
    UPoint end(pos.x + tenths_to_logical(pLine->get_x_end()),
               pos.y + tenths_to_logical(pLine->get_y_end()) );

    ShapeId idx = 0;
    return LOMSE_NEW GmoShapeLine(pLine, idx, GmoObj::k_shape_line,
                                  start.x, start.y,
                                  end.x, end.y,
                                  tenths_to_logical(pLine->get_line_width()),
                                  0.5f,
                                  pLine->get_line_style(),
                                  pLine->get_color(),
                                  pLine->get_start_edge(),
                                  pLine->get_start_cap(),
                                  pLine->get_end_cap() );
}

//---------------------------------------------------------------------------------------
GmoShapeLine* LineEngraver::dbg_create_shape(UPoint start, UPoint end, Color color,
                                             LUnits lineWidth)
{
    ShapeId idx = 0;
    return LOMSE_NEW GmoShapeLine(nullptr, idx, GmoObj::k_shape_line,
                                  start.x, start.y, end.x, end.y,
                                  lineWidth, 0.5f, k_line_solid, color,
                                  k_edge_normal, k_cap_none, k_cap_none);
}


}  //namespace lomse

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
                                          pLine->get_color(),
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

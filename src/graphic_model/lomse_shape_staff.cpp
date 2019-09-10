//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_shape_staff.h"

#include "lomse_internal_model.h"
#include "lomse_drawer.h"
//#include <sstream>
//using namespace std;

namespace lomse
{

//#define lmNO_LEDGER_LINES   -100000.0f

//---------------------------------------------------------------------------------------
GmoShapeStaff::GmoShapeStaff(ImoObj* pCreatorImo, ShapeId idx, ImoStaffInfo* pStaff,
                             int iStaff, LUnits width, Color color)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_staff, idx, color)
    , m_pStaff(pStaff)
	, m_iStaff(iStaff)
    , m_lineThickness( m_pStaff->get_line_thickness() )
{
    //bounding box
    set_width(width);
    set_height(m_pStaff->get_height());
}

//---------------------------------------------------------------------------------------
GmoShapeStaff::~GmoShapeStaff()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeStaff::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
//    if (!m_fVisible) return;
//
//    //update selection rectangle
//    m_uSelRect = GetBounds();
//
    //draw the staff
    double xStart = m_origin.x;
    double xEnd = xStart + m_size.width;
    double yPos = m_origin.y;
    double spacing = m_pStaff->get_line_spacing();

    Color color = determine_color_to_use(opt);
    pDrawer->begin_path();
    pDrawer->stroke(color);
    pDrawer->stroke_width(m_lineThickness);
    for (int iL=0; iL < m_pStaff->get_num_lines(); iL++ )
	{
        pDrawer->move_to(xStart, yPos);
        pDrawer->line_to(xEnd, yPos);
        yPos += spacing;
    }
    pDrawer->end_path();

    GmoSimpleShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
int GmoShapeStaff::line_space_at(LUnits yPos)
{
    //returns the position (line/space number) for the received point. Position is
    //referred to the first ledger line of the staff:
    //        0 - on first ledger line (C note in G clef)
    //        1 - on next space (D in G clef)
    //        2 - on first line (E not in G clef)
    //        3 - on first space
    //        4 - on second line
    //        5 - on second space
    //        etc.

    //compute the number of steps (half lines) from line 5 (top staff line = step #10)
    LUnits spacing = m_pStaff->get_line_spacing();
	LUnits uHalfLine = spacing / 2.0f;
    float rStep = (m_origin.y - yPos)/uHalfLine;
    int nStep = (rStep > 0.0f ? int(rStep + 0.5f) : int(rStep - 0.5f) );
//    //wxLogMessage(_T("[GmoShapeStaff::line_space_at] yPos=%.2f, spacing=%.2f, Top.y=%.2f, rStep=%.2f, nStep=%d"),
//    //             yPos, spacing, m_origin.y, rStep, nStep);
	return  10 + nStep;     //AWARE: remember: y axis is reversed.
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeStaff::get_staff_line_spacing()
{
    return m_pStaff->get_line_spacing();
}

//---------------------------------------------------------------------------------------
int GmoShapeStaff::get_staff_num_lines()
{
    return m_pStaff->get_num_lines();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeStaff::get_staff_margin()
{
    return m_pStaff->get_staff_margin();
}


}  //namespace lomse

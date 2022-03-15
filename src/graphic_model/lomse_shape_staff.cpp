//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
    if (pDrawer->accepts_id_class())
        pDrawer->start_simple_notation(get_notation_id()+"-staff", "staff-lines");

    pDrawer->begin_path();
    pDrawer->stroke(color);
    pDrawer->stroke_width(m_lineThickness);
    int iMax = max(m_pStaff->get_num_lines(), 5);
    for (int iL=0; iL < iMax; iL++ )
	{
	    if (m_pStaff->is_line_visible(iL))
	    {
            pDrawer->move_to(xStart, yPos);
            pDrawer->line_to(xEnd, yPos);
	    }
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

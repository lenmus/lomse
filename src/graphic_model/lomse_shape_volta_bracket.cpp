//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_shape_volta_bracket.h"

#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_gm_basic.h"
#include "lomse_drawer.h"
#include "lomse_shape_barline.h"

#include <cmath>   //abs

namespace lomse
{


//=======================================================================================
// GmoShapeVoltaBracket implementation
//=======================================================================================
GmoShapeVoltaBracket::GmoShapeVoltaBracket(ImoObj* pCreatorImo, ShapeId idx, Color color)
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_volta_bracket, idx, color)
    , m_pShapeText(nullptr)
    , m_fTwoBrackets(false)
    , m_fStopJog(true)
    , m_uJogLength(400.0f)
    , m_uLineThick(4.0f)
    , m_uStaffLeft(0.0f)
    , m_uStaffRight(0.0f)
    , m_uBracketDistance(600.0f)
{
}

//---------------------------------------------------------------------------------------
GmoShapeVoltaBracket::~GmoShapeVoltaBracket()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::add_label(GmoShapeText* pShape)
{
    m_pShapeText = pShape;
    add(pShape);
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::set_layout_data(LUnits xStart, LUnits xEnd, LUnits yPos,
                                           LUnits uBracketDistance, LUnits uJogLength,
                                           LUnits uLineThick, LUnits uLeftSpaceToText,
                                           LUnits uTopSpaceToText, LUnits xStaffLeft,
                                           LUnits xStaffRight)
{
    //save data
    m_uBracketDistance = uBracketDistance;
	m_uJogLength = uJogLength;
    m_uLineThick = uLineThick;
    m_uStaffLeft = xStaffLeft;
    m_uStaffRight = xStaffRight;

	//move text shape to its position
    if (m_pShapeText)
        m_pShapeText->set_origin(xStart + uLeftSpaceToText, yPos + uTopSpaceToText);

    compute_bounds(xStart, xEnd, yPos);
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);      //Color(255,0,0);

    LUnits xStart = m_origin.x;
    LUnits xEnd = xStart + m_size.width;
    LUnits yTop = m_origin.y;
    LUnits yBottom = yTop + m_uJogLength;

    if (pDrawer->accepts_id_class())
        pDrawer->start_composite_notation(get_notation_id(), get_notation_class());

    pDrawer->begin_path();
    pDrawer->fill(color);

    //start jog
    if (!m_fTwoBrackets || get_shape_id() == 0)
        pDrawer->line(xStart, yBottom, xStart, yTop, m_uLineThick, k_edge_normal);

    //horizontal line
    pDrawer->line(xStart, yTop, xEnd, yTop, m_uLineThick, k_edge_normal);

    //end jog
    if (m_fStopJog)
        pDrawer->line(xEnd, yTop, xEnd, yBottom, m_uLineThick, k_edge_normal);

    pDrawer->end_path();
    pDrawer->render();

    GmoCompositeShape::on_draw(pDrawer, opt);

    if (pDrawer->accepts_id_class())
        pDrawer->end_composite_notation();
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::compute_bounds(LUnits xStart, LUnits xEnd, LUnits yPos)
{
    m_origin.x = xStart;
    m_size.width = abs(xEnd - xStart);
    m_origin.y = yPos;

    if (m_pShapeText)
        m_size.height = max(m_uJogLength, m_pShapeText->get_height());
    else
        m_size.height = m_uJogLength;
}

//---------------------------------------------------------------------------------------
int GmoShapeVoltaBracket::get_num_handlers()
{
    return 4;
}

//---------------------------------------------------------------------------------------
UPoint GmoShapeVoltaBracket::get_handler_point(int UNUSED(i))
{
    //TODO
    return UPoint(0.0f, 0.0f);
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::on_handler_dragged(int UNUSED(iHandler), UPoint UNUSED(newPos))
{
    //TODO
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::on_end_of_handler_drag(int UNUSED(iHandler), UPoint UNUSED(newPos))
{
    //TODO
}


}  //namespace lomse

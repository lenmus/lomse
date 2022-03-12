//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_handler.h"

#include "lomse_bitmap_drawer.h"

#include <math.h>


namespace lomse
{

//=======================================================================================
// Handler implementation
//=======================================================================================
Handler::Handler(GraphicView* view, LibraryScope& libraryScope,
                 GmoObj* pControlledGmo, int index)
    : VisualEffect(view, libraryScope)
    , m_pControlledGmo(pControlledGmo)
    , m_index(index)
{
}


//=======================================================================================
// HandlerCircle implementation
//=======================================================================================
HandlerCircle::HandlerCircle(GraphicView* view, LibraryScope& libraryScope,
                             GmoObj* pControlledGmo, int index)
    : Handler(view, libraryScope, pControlledGmo, index)
    , m_origin(0.0, 0.0)
    , m_radius(100.0)
    , m_borderColor( Color(255, 0, 0, 255) )
    , m_fillColor( Color(255, 0, 0, 32) )
{
}

//---------------------------------------------------------------------------------------
HandlerCircle::~HandlerCircle()
{
}

//---------------------------------------------------------------------------------------
void HandlerCircle::on_draw(BitmapDrawer* pDrawer)
{
    compute_radius(pDrawer);

    pDrawer->begin_path();
    pDrawer->fill(m_fillColor);
    pDrawer->stroke(m_borderColor);
    pDrawer->stroke_width(10.0);    //0.1 mm
    pDrawer->circle(m_origin.x, m_origin.y, m_radius);
    pDrawer->end_path();

    pDrawer->render();
}

//---------------------------------------------------------------------------------------
void HandlerCircle::compute_radius(BitmapDrawer* UNUSED(pDrawer))
{
    //TODO. The idea is to compute the needed value so that it is at least painted
    // with 0.7 mm radius
    m_radius = 70.0;
}

//---------------------------------------------------------------------------------------
bool HandlerCircle::hit_test(LUnits x, LUnits y)
{
    return compute_distance(x, y, m_origin.x, m_origin.y) <= m_radius;
}

//---------------------------------------------------------------------------------------
URect HandlerCircle::get_bounds()
{
    LUnits diameter = m_radius + m_radius;
    return URect(m_origin, USize(diameter, diameter));
}

//---------------------------------------------------------------------------------------
void HandlerCircle::move_to(UPoint pos)
{
    m_origin = pos;
    m_pControlledGmo->on_handler_dragged(m_index, pos);
}


}  //namespace lomse

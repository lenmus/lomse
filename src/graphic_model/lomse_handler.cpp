//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#include "lomse_handler.h"

#include "lomse_screen_drawer.h"

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
void HandlerCircle::on_draw(ScreenDrawer* pDrawer)
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
void HandlerCircle::compute_radius(ScreenDrawer* UNUSED(pDrawer))
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

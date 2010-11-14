//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
//  Credits:
//  -------------------------
//  This file is based on Anti-Grain Geometry version 2.4 examples' code.
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev 
//  (http://www.antigrain.com). AGG 2.4 is distributed under BSD license.
//
//---------------------------------------------------------------------------------------

#include "lomse_screen_renderer.h"

#include "agg_trans_affine.h"
#include "lomse_exceptions.h"
#include "lomse_agg_drawer.h"
#include "lomse_gm_basic.h"

using namespace agg;

namespace lomse
{

//---------------------------------------------------------------------------------------
ScreenRenderer::ScreenRenderer()
    : Renderer()
    , m_curved(m_storage)
    , m_curved_count(m_curved)

    , m_curved_stroked(m_curved_count)
    , m_curved_stroked_trans(m_curved_stroked, m_transform)

    , m_curved_trans(m_curved_count, m_transform)
    , m_curved_trans_contour(m_curved_trans)
    , m_drawer(NULL)
{
    m_curved_trans_contour.auto_detect_orientation(false);
}

//---------------------------------------------------------------------------------------
ScreenRenderer::~ScreenRenderer()
{
    delete_drawer();
}

//---------------------------------------------------------------------------------------
void ScreenRenderer::delete_drawer()
{
    if (m_drawer)
        delete m_drawer;
    m_drawer = NULL;
}

//---------------------------------------------------------------------------------------
void ScreenRenderer::reset()
{
    m_storage.remove_all();
    m_attr_storage.remove_all();
    m_attr_stack.remove_all();
    m_transform.reset();
    delete_drawer();
}

//---------------------------------------------------------------------------------------
Drawer* ScreenRenderer::paths_generation_start()
{
    reset();
    m_drawer = new AggDrawer(m_storage, m_attr_storage);
    return m_drawer;
}

//---------------------------------------------------------------------------------------
void ScreenRenderer::paths_generation_end()
{
    arrange_orientations();
}


}   //namespace lomse

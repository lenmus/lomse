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

#include "lomse_shape_base.h"

#include "lomse_drawer.h"

namespace lomse
{

//=======================================================================================
// Implementation of class GmoShape: any renderizable object, such as a line,
// a glyph, a note head, an arch, etc.
//=======================================================================================
GmoShape::GmoShape(ImoObj* pCreatorImo, int objtype, int idx, Color color)
    : GmoObj(objtype, pCreatorImo)
    , Linkable<USize>()
    , m_idx(idx)
    , m_layer(GmoShape::k_layer_background)
    , m_color(color)
    , m_pRelatedShapes(NULL)
{
}

//---------------------------------------------------------------------------------------
GmoShape::~GmoShape()
{
    delete m_pRelatedShapes;
}

//---------------------------------------------------------------------------------------
void GmoShape::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
//    if (IsVisible())
//        Render(pPaper, (IsSelected() ? g_pColors->ScoreSelected() : m_color) );

    //draw bounding box if selected (just for testing)
    if (is_selected())
    {
        pDrawer->begin_path();
        pDrawer->fill(Color(0, 0, 0, 0));
        pDrawer->stroke(Color(0, 0, 255));
        pDrawer->stroke_width(15.0);
        pDrawer->move_to(m_origin.x, m_origin.y);
        pDrawer->hline_to(m_origin.x + m_size.width);
        pDrawer->vline_to(m_origin.y + m_size.height);
        pDrawer->hline_to(m_origin.x);
        pDrawer->vline_to(m_origin.y);
        pDrawer->end_path();
    }
}

//---------------------------------------------------------------------------------------
Color GmoShape::determine_color_to_use(RenderOptions& opt)
{
    if (is_highlighted())
        return opt.highlighted_color;
    else if (is_selected())
        return opt.selected_color;
    else if (is_hover())
        return Color(255,0,0);
    else
        return get_normal_color();
}

//---------------------------------------------------------------------------------------
void GmoShape::handle_link_event(Linkable<USize>* pShape, int type, USize shift)
{
    shift_origin(shift);
    notify_linked_observers(shift);
}

//---------------------------------------------------------------------------------------
void GmoShape::on_linked_to(Linkable<USize>* pShape, int type)
{

}

//---------------------------------------------------------------------------------------
void GmoShape::set_origin_and_notify_observers(LUnits xLeft, LUnits yTop)
{
    USize shift(xLeft - m_origin.x, yTop);
    shift_origin(shift);
    notify_linked_observers(shift);
}

//---------------------------------------------------------------------------------------
void GmoShape::add_related_shape(GmoShape* pShape)
{
    if (!m_pRelatedShapes)
        m_pRelatedShapes = LOMSE_NEW std::list<GmoShape*>();

    m_pRelatedShapes->push_back(pShape);
}

//---------------------------------------------------------------------------------------
GmoShape* GmoShape::find_related_shape(int type)
{
    if (!m_pRelatedShapes)
        return NULL;;

    std::list<GmoShape*>::iterator it;
    for (it = m_pRelatedShapes->begin(); it != m_pRelatedShapes->end(); ++it)
    {
        if ((*it)->get_gmobj_type() == type)
            return *it;
    }
    return NULL;
}



//=======================================================================================
// Implementation of class GmoSimpleShape
//=======================================================================================
GmoSimpleShape::GmoSimpleShape(ImoObj* pCreatorImo, int objtype, int idx, Color color)
    : GmoShape(pCreatorImo, objtype, idx, color)
{
}

//---------------------------------------------------------------------------------------
GmoSimpleShape::~GmoSimpleShape()
{
}



//=======================================================================================
// Implementation of class GmoCompositeShape
//=======================================================================================
GmoCompositeShape::GmoCompositeShape(ImoObj* pCreatorImo, int objtype, int idx,
                                     Color color)
    : GmoShape(pCreatorImo, objtype, idx, color)
    , m_fLocked(true)
{
}

//---------------------------------------------------------------------------------------
GmoCompositeShape::~GmoCompositeShape()
{
    std::list<GmoShape*>::iterator it;
    for (it = m_components.begin(); it != m_components.end(); ++it)
    {
        delete *it;
    }
    m_components.clear();
}

//---------------------------------------------------------------------------------------
int GmoCompositeShape::add(GmoShape* pShape)
{
    m_components.push_back(pShape);

	if (m_components.size() == 1)
	{
		//copy bounds
		m_origin = pShape->get_origin();
		m_size = pShape->get_size();
	}
	else
	{
	    //TODO: Note from LenMus:
//        lmCompositeShape: the selection rectangle should not be the boundling rectangle
//        but each rectangle of each component shape. This will save the need to define
//        specific shapes just to override selection rectangle. i.i. metronome marks

		//compute new selection rectangle by union of individual selection rectangles
		URect bbox = get_bounds();
		bbox.Union(pShape->get_bounds());
		m_origin = bbox.get_top_left();
		m_size.width = bbox.get_width();
		m_size.height = bbox.get_height();
	}

	//return index to added shape
	return (int)m_components.size() - 1;
}

//---------------------------------------------------------------------------------------
void GmoCompositeShape::shift_origin(const USize& shift)
{
    m_origin.x += shift.width;
    m_origin.y += shift.height;

    //shift components
    std::list<GmoShape*>::iterator it;
    for (it = m_components.begin(); it != m_components.end(); ++it)
        (*it)->shift_origin(shift);
}

//---------------------------------------------------------------------------------------
void GmoCompositeShape::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    GmoShape::on_draw(pDrawer, opt);

	//Default behaviour: render all components
    std::list<GmoShape*>::iterator it;
    for (it = m_components.begin(); it != m_components.end(); ++it)
        (*it)->on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoCompositeShape::set_selected(bool value)
{
    GmoShape::set_selected(value);

    std::list<GmoShape*>::iterator it;
    for (it = m_components.begin(); it != m_components.end(); ++it)
        (*it)->set_selected(value);
}

//---------------------------------------------------------------------------------------
void GmoCompositeShape::set_highlighted(bool value)
{
    GmoShape::set_highlighted(value);

    std::list<GmoShape*>::iterator it;
    for (it = m_components.begin(); it != m_components.end(); ++it)
        (*it)->set_highlighted(value);
}

//---------------------------------------------------------------------------------------
void GmoCompositeShape::lock()
{
    m_fLocked = true;
    recompute_bounds();
}

//---------------------------------------------------------------------------------------
void GmoCompositeShape::recompute_bounds()
{
 	URect bbox;
    std::list<GmoShape*>::iterator it;
    for (it = m_components.begin(); it != m_components.end(); ++it)
		bbox.Union((*it)->get_bounds());

	m_origin = bbox.get_top_left();
	m_size.width = bbox.get_width();
	m_size.height = bbox.get_height();
}


}  //namespace lomse

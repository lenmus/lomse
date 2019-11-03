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

#include "lomse_shape_base.h"

#include "lomse_drawer.h"
#include "lomse_logger.h"
#include "lomse_time.h"      //round_half_up

namespace lomse
{

//=======================================================================================
// Implementation of class GmoShape: any renderizable object, such as a line,
// a glyph, a note head, an arch, etc.
//=======================================================================================
GmoShape::GmoShape(ImoObj* pCreatorImo, int objtype, ShapeId idx, Color color)
    : GmoObj(objtype, pCreatorImo)
//    , Linkable<USize>()
    , m_idx(idx)
    , m_layer(GmoShape::k_layer_background)
    , m_color(color)
    , m_pRelatedShapes(nullptr)
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
    if (opt.draw_shape_bounds)
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
    if (opt.draw_shapes_highlighted)
        return opt.highlighted_color;
    else if (opt.draw_shapes_dragged)
        return opt.dragged_color;
    else if (opt.draw_shapes_selected)
        return opt.selected_color;
//    else if (is_selected())
//        return opt.selected_color;
    else if (is_hover())
        return Color(255,0,0);
    else
    {
        VoiceRelatedShape* pSO = dynamic_cast<VoiceRelatedShape*>(this);
        if (!opt.read_only_mode)
        {
            if (pSO && pSO->get_voice() != 0 && pSO->get_voice() != opt.highlighted_voice)
                return opt.not_highlighted_voice_color;
        }
        else if (pSO && opt.draw_voices_coloured)
        {
            return opt.voiceColor[pSO->get_voice()];
        }
        return get_normal_color();
    }
}

////---------------------------------------------------------------------------------------
//void GmoShape::handle_link_event(Linkable<USize>* pShape, int type, USize shift)
//{
//    shift_origin(shift);
//    notify_linked_observers(shift);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShape::on_linked_to(Linkable<USize>* pShape, int type)
//{
//
//}

//---------------------------------------------------------------------------------------
void GmoShape::set_origin_and_notify_observers(LUnits xLeft, LUnits yTop)
{
    USize shift(xLeft - m_origin.x, yTop);
    shift_origin(shift);
//    notify_linked_observers(shift);
}

//---------------------------------------------------------------------------------------
void GmoShape::reposition_shape(LUnits yShift)
{
    shift_origin(0.0f, yShift);
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
        return nullptr;;

    std::list<GmoShape*>::iterator it;
    for (it = m_pRelatedShapes->begin(); it != m_pRelatedShapes->end(); ++it)
    {
        if ((*it)->get_gmobj_type() == type)
            return *it;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
void GmoShape::dump(ostream& outStream, int level)
{
    std::ios_base::fmtflags f( outStream.flags() );  //save formating options

    outStream << setw(level*3) << level << " [" << setw(3) << m_objtype << "] "
              << get_name(m_objtype)
              << "[" << m_idx << "]"
              << fixed << setprecision(2) << setfill(' ')
              << setw(10) << round_half_up(m_origin.x) << ", "
              << setw(10) << round_half_up(m_origin.y) << ", "
              << setw(10) << round_half_up(m_size.width) << ", "
              << setw(10) << round_half_up(m_size.height) << endl;

    outStream.flags( f );  //restore formating options
}

//---------------------------------------------------------------------------------------
bool GmoShape::hit_test(LUnits x, LUnits y)
{
    URect bbox = get_bounds();
    return bbox.contains(x, y);
}


//=======================================================================================
// Implementation of class GmoSimpleShape
//=======================================================================================
GmoSimpleShape::GmoSimpleShape(ImoObj* pCreatorImo, int objtype, ShapeId idx, Color color)
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
GmoCompositeShape::GmoCompositeShape(ImoObj* pCreatorImo, int objtype, ShapeId idx,
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
void GmoCompositeShape::reposition_shape(LUnits yShift)
{
    m_origin.y += yShift;

    //shift components
    std::list<GmoShape*>::iterator it;
    for (it = m_components.begin(); it != m_components.end(); ++it)
        (*it)->reposition_shape(yShift);
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

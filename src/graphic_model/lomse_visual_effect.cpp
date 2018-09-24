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

#include "lomse_visual_effect.h"

#include "lomse_screen_drawer.h"
#include "lomse_gm_basic.h"
#include "lomse_logger.h"
#include "lomse_graphic_view.h"
#include "lomse_selections.h"


namespace lomse
{

//=======================================================================================
// VisualEffect implementation
//=======================================================================================
VisualEffect::VisualEffect(GraphicView* view, LibraryScope& libraryScope)
    : m_libraryScope(libraryScope)
    , m_pView(view)
    , m_fVisible(false)
    , m_fEnabled(true)
{
}


//=======================================================================================
// DraggedImage implementation
//=======================================================================================
DraggedImage::DraggedImage(GraphicView* view, LibraryScope& libraryScope)
    : VisualEffect(view, libraryScope)
    , m_pShape(nullptr)
    , m_fShapeIsOwned(true)
{
}

//---------------------------------------------------------------------------------------
DraggedImage::~DraggedImage()
{
    delete_shape();
}

//---------------------------------------------------------------------------------------
void DraggedImage::set_shape(GmoShape* pShape, bool fGetOwnership, UPoint offset)
{
    delete_shape();
    m_pShape = pShape;
    m_fShapeIsOwned = fGetOwnership;
    m_offset = offset;
}

//---------------------------------------------------------------------------------------
void DraggedImage::delete_shape()
{
    if (m_fShapeIsOwned)
        delete m_pShape;
    m_pShape = nullptr;
}

//---------------------------------------------------------------------------------------
void DraggedImage::move_to(LUnits x, LUnits y)
{
    m_origin.x = x;
    m_origin.y = y;
}

//---------------------------------------------------------------------------------------
void DraggedImage::on_draw(ScreenDrawer* pDrawer)
{
   if (m_pShape && m_fEnabled)
    {
        UPoint pos = m_pShape->get_origin();
        m_pShape->set_origin(m_origin.x - m_offset.x, m_origin.y - m_offset.y);
        RenderOptions options;
        options.draw_shapes_dragged = true;
        m_pShape->on_draw(pDrawer, options);
        m_pShape->set_origin(pos.x, pos.y);
        pDrawer->render();
    }
}

//---------------------------------------------------------------------------------------
URect DraggedImage::get_bounds()
{
    if (m_pShape)
        return m_pShape->get_bounds();
    else
        return URect(0.0, 0.0, 0.0, 0.0);
}


//=======================================================================================
// SelectionRectangle implementation
//=======================================================================================
SelectionRectangle::SelectionRectangle(GraphicView* view, LibraryScope& libraryScope)
    : VisualEffect(view, libraryScope)
    , m_xStart(0.0f)
    , m_yStart(0.0f)
    , m_left(0.0f)
    , m_top(0.0f)
    , m_right(0.0f)
    , m_bottom(0.0f)
{
}

//---------------------------------------------------------------------------------------
void SelectionRectangle::set_start_point(LUnits x, LUnits y)
{
    m_xStart = x;
    m_yStart = y;
}

//---------------------------------------------------------------------------------------
void SelectionRectangle::set_end_point(LUnits x, LUnits y)
{
    //normalize rectangle
    if (m_xStart < x)
    {
        m_left = m_xStart;
        m_right = x;
    }
    else
    {
        m_right = m_xStart;
        m_left = x;
    }

    if (m_yStart < y)
    {
        m_top = m_yStart;
        m_bottom = y;
    }
    else
    {
        m_bottom = m_yStart;
        m_top = y;
    }
}

//---------------------------------------------------------------------------------------
void SelectionRectangle::on_draw(ScreenDrawer* pDrawer)
{
    double x1 = double( m_left );
    double y1 = double( m_top );
    double x2 = double( m_right );
    double y2 = double( m_bottom );

    double line_width = double( pDrawer->Pixels_to_LUnits(1) );

    pDrawer->begin_path();
    pDrawer->fill( Color(0, 0, 255, 16) );        //background blue transparent
    pDrawer->stroke( Color(0, 0, 255, 255) );     //solid blue
    pDrawer->stroke_width(line_width);
    pDrawer->move_to(x1, y1);
    pDrawer->hline_to(x2);
    pDrawer->vline_to(y2);
    pDrawer->hline_to(x1);
    pDrawer->vline_to(y1);
    pDrawer->end_path();

    pDrawer->render();
}

//---------------------------------------------------------------------------------------
URect SelectionRectangle::get_bounds()
{
    return URect(UPoint(m_left, m_top), UPoint(m_right, m_bottom));
}


//=======================================================================================
// PlaybackHighlight implementation
//=======================================================================================
PlaybackHighlight::PlaybackHighlight(GraphicView* view, LibraryScope& libraryScope)
    : VisualEffect(view, libraryScope)
    , m_color(Color(255,0,0))   //red
{
}

//---------------------------------------------------------------------------------------
void PlaybackHighlight::add_highlight(GmoShape* pShape)
{
    m_noterests.push_back(pShape);
}

//---------------------------------------------------------------------------------------
void PlaybackHighlight::remove_highlight(GmoShape* pShape)
{
    m_noterests.remove(pShape);
}

//---------------------------------------------------------------------------------------
void PlaybackHighlight::remove_all_highlight()
{
    m_noterests.clear();
}

//---------------------------------------------------------------------------------------
void PlaybackHighlight::on_draw(ScreenDrawer* pDrawer)
{
    m_bounds = URect(0.0, 0.0, 0.0, 0.0);
    RenderOptions options;
    options.draw_shapes_highlighted = true;
    Color savedColor = options.highlighted_color;
    options.highlighted_color = m_color;
    list<GmoShape*>::const_iterator it;
    for (it = m_noterests.begin(); it != m_noterests.end(); ++it)
    {
        GmoShape* pShape = *it;
        if (pShape)
        {
            UPoint org = m_pView->get_page_origin_for(pShape);
            pDrawer->set_shift(-org.x, -org.y);
            pShape->on_draw(pDrawer, options);
            m_bounds.Union( pShape->get_bounds() );
//            LOMSE_LOG_DEBUG(Logger::k_events, "draw note: xPos=%f, org=%f",
//                            m_bounds.x, org.x);
        }
    }
    pDrawer->render();
    pDrawer->remove_shift();
    options.highlighted_color = savedColor;
}

//---------------------------------------------------------------------------------------
URect PlaybackHighlight::get_bounds()
{
    return m_bounds;
}


//=======================================================================================
// SelectionHighlight implementation
//=======================================================================================
SelectionHighlight::SelectionHighlight(GraphicView* view, LibraryScope& libraryScope,
                                       SelectionSet* pSelectionSet)
    : VisualEffect(view, libraryScope)
    , m_pSelectionSet(pSelectionSet)
{
    m_fVisible = true;
}

//---------------------------------------------------------------------------------------
void SelectionHighlight::on_draw(ScreenDrawer* pDrawer)
{
    m_bounds = URect(0.0, 0.0, 0.0, 0.0);
    RenderOptions options;
    options.draw_shapes_selected = true;
//    options.selected_color = Color(255, 255, 0);       //just a test
    list<GmoObj*>& gmos = m_pSelectionSet->get_all_gmo_objects();
    list<GmoObj*>::const_iterator it;
    for (it = gmos.begin(); it != gmos.end(); ++it)
    {
        if ((*it)->is_shape())
        {
            GmoShape* pShape = static_cast<GmoShape*>(*it);
            UPoint org = m_pView->get_page_origin_for(pShape);
            pDrawer->set_shift(-org.x, -org.y);
            pShape->on_draw(pDrawer, options);
            m_bounds.Union( pShape->get_bounds() );
        }
    }
    pDrawer->render();
    pDrawer->remove_shift();
}

//---------------------------------------------------------------------------------------
URect SelectionHighlight::get_bounds()
{
    return m_bounds;
}

//---------------------------------------------------------------------------------------
bool SelectionHighlight::are_handlers_needed()
{
    list<GmoObj*>& gmos = m_pSelectionSet->get_all_gmo_objects();
    if (gmos.size() == 1)
    {
        GmoObj* pGmo = gmos.front();
        return pGmo->has_handlers();
    }
    return false;
}

//---------------------------------------------------------------------------------------
GmoObj* SelectionHighlight::get_object_needing_handlers()
{
    list<GmoObj*>& gmos = m_pSelectionSet->get_all_gmo_objects();
    return gmos.front();
}


}  //namespace lomse

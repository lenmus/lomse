//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_overlays_generator.h"

#include "lomse_bitmap_drawer.h"
#include "lomse_logger.h"
#include "lomse_visual_effect.h"
#include "lomse_renderer.h"

namespace lomse
{

//=======================================================================================
// OverlaysGenerator implementation
//=======================================================================================
OverlaysGenerator::OverlaysGenerator(GraphicView* view, LibraryScope& libraryScope)
    : m_libraryScope(libraryScope)
    , m_pView(view)
    , m_fBackgroundDirty(false)
    , m_fFullRectangle(true)
    , m_pSaveBytes(nullptr)
    , m_damagedRect(0.0, 0.0, 0.0, 0.0)
    , m_prevDamagedRect(0.0, 0.0, 0.0, 0.0)
    , m_pHandlersOwner(nullptr)
{
}

//---------------------------------------------------------------------------------------
OverlaysGenerator::~OverlaysGenerator()
{
    free(m_pSaveBytes);

    //delete all VisualEffects
    list<VisualEffect*>::iterator it = m_effects.begin();
    while (it != m_effects.end())
    {
        VisualEffect* pEffect = *it;
        it = m_effects.erase(it);
        delete pEffect;
    }
}

//---------------------------------------------------------------------------------------
void OverlaysGenerator::add_visual_effect(VisualEffect* pEffect)
{
    m_effects.push_back(pEffect);
}

//---------------------------------------------------------------------------------------
void OverlaysGenerator::remove_visual_effect(VisualEffect* pEffect)
{
    m_effects.remove(pEffect);
}

//---------------------------------------------------------------------------------------
void OverlaysGenerator::update_all_visual_effects(BitmapDrawer* pDrawer)
{
    if (m_fBackgroundDirty)
        m_canvasBuffer.copy_from(m_savedBuffer);

    m_damagedRect = URect(0.0, 0.0, 0.0, 0.0);
    int overlays = 0;
    list<VisualEffect*>::const_iterator it;
    for (it = m_effects.begin(); it != m_effects.end(); ++it)
    {
        if ((*it)->is_visible())
        {
            (*it)->on_draw(pDrawer);
            ++overlays;
            m_damagedRect.Union( (*it)->get_bounds() );
        }
    }

    if (overlays == 0)
        m_damagedRect = m_prevDamagedRect;
    else
        expand_damaged_rectangle();

    m_fBackgroundDirty = (overlays > 0);
}

//---------------------------------------------------------------------------------------
void OverlaysGenerator::update_visual_effect(VisualEffect* pEffect,
                                             BitmapDrawer* pDrawer)
{
    update_all_visual_effects(pDrawer);
    if (pEffect->is_visible())
    {
        m_damagedRect = pEffect->get_bounds();
        expand_damaged_rectangle();
    }
    else
        m_damagedRect = m_prevDamagedRect;
}

//---------------------------------------------------------------------------------------
void OverlaysGenerator::set_rendering_buffer(unsigned char* buf, unsigned width,
                                             unsigned height)
{
    int pixFmt = m_libraryScope.get_pixel_format();
    int stride = Renderer::bytesPerPixel(pixFmt) * width;
    m_canvasBuffer.attach(buf, width, height, stride);
    m_fBackgroundDirty = false;
    m_fFullRectangle = true;
}

//---------------------------------------------------------------------------------------
void OverlaysGenerator::on_new_background()
{
    save_rendering_buffer();
    m_fFullRectangle = true;
}

//---------------------------------------------------------------------------------------
void OverlaysGenerator::save_rendering_buffer()
{
    //if necessary, allocate buffer for saving screen buffer
    unsigned w = m_canvasBuffer.width();
    unsigned h = m_canvasBuffer.height();
    int stride = m_canvasBuffer.stride();
    size_t bytes = size_t(h) * size_t(abs(stride));
    if (bytes == 0)
        return;     //in Unit Tests
    if (m_pSaveBytes == nullptr
        || bytes != size_t(m_savedBuffer.height()) * size_t(m_savedBuffer.stride()))
    {
        free(m_pSaveBytes);
        m_pSaveBytes = static_cast<int8u*>( malloc(bytes) );
        m_savedBuffer.attach(m_pSaveBytes, w, h, stride);

//        stringstream msg;
//        msg << "Allocated new buffer. w=" << w << ", h=" << h << ", stride="
//            << stride << ", bytes=" << bytes;
//        LOMSE_LOG_INFO(msg.str());
    }

    m_savedBuffer.copy_from(m_canvasBuffer);
    m_fBackgroundDirty = false;
}

//---------------------------------------------------------------------------------------
void OverlaysGenerator::expand_damaged_rectangle()
{
    //increase damaged rectangle (1mm increment at each side) to take into account
    //any additional pixels due to anti-aliasing.

    m_damagedRect.x -= 100.0;   //1mm = 100 LUnits
    m_damagedRect.y -= 100.0;
    m_damagedRect.width += 200.0;
    m_damagedRect.height += 200.0;
}

//---------------------------------------------------------------------------------------
URect OverlaysGenerator::get_damaged_rectangle()
{
    if (m_fFullRectangle)
    {
        m_fFullRectangle = false;
        m_prevDamagedRect = m_damagedRect;
        return URect(0.0, 0.0, 0.0, 0.0);
    }
    else if (m_prevDamagedRect != URect(0.0, 0.0, 0.0, 0.0))
    {
        URect total = m_damagedRect;
        total.Union(m_prevDamagedRect);
        m_prevDamagedRect = m_damagedRect;
        return total;
    }
    else
    {
        m_prevDamagedRect = m_damagedRect;
        return m_damagedRect;
    }
}


}  //namespace lomse

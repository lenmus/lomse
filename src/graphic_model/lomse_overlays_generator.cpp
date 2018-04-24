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

#include "lomse_overlays_generator.h"

#include "lomse_screen_drawer.h"
//#include "lomse_graphic_view.h"
#include "lomse_logger.h"
#include "lomse_visual_effect.h"


namespace lomse
{

//=======================================================================================
// OverlaysGenerator implementation
//=======================================================================================
OverlaysGenerator::OverlaysGenerator(GraphicView* view, LibraryScope& libraryScope)
    : m_libraryScope(libraryScope)
    , m_pView(view)
    , m_pCanvasBuffer(nullptr)
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
    delete m_pSaveBytes;

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
void OverlaysGenerator::update_all_visual_effects(ScreenDrawer* pDrawer)
{
    if (m_fBackgroundDirty)
        m_pCanvasBuffer->copy_from(m_savedBuffer);

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
                                             ScreenDrawer* pDrawer)
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
void OverlaysGenerator::set_rendering_buffer(RenderingBuffer* rbuf)
{
    m_pCanvasBuffer = rbuf;
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
    unsigned w = m_pCanvasBuffer->width();
    unsigned h = m_pCanvasBuffer->height();
    int stride = m_pCanvasBuffer->stride();
    size_t bytes = h * abs(stride);
    if (m_pSaveBytes == nullptr || bytes != m_savedBuffer.height() * m_savedBuffer.stride())
    {
        delete m_pSaveBytes;
        m_pSaveBytes = static_cast<int8u*>( malloc(bytes) );
        m_savedBuffer.attach(m_pSaveBytes, w, h, stride);

//        stringstream msg;
//        msg << "Allocated new buffer. w=" << w << ", h=" << h << ", stride="
//            << stride << ", bytes=" << bytes;
//        LOMSE_LOG_INFO(msg.str());
    }

    m_savedBuffer.copy_from(*m_pCanvasBuffer);
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

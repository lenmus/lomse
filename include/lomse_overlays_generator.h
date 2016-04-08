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

#ifndef __LOMSE_OVERLAYS_GENERATOR_H__
#define __LOMSE_OVERLAYS_GENERATOR_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_agg_types.h"        //RenderingBuffer

//other
#include <iostream>
using namespace std;


namespace lomse
{

//forward declarations
class GraphicView;
class ScreenDrawer;
class VisualEffect;
class GmoObj;


//---------------------------------------------------------------------------------------
// OverlaysGenerator:
//  responsible for generating all visual sprites and overlaying them onto the
//  rendering buffer
class OverlaysGenerator
{
protected:
    LibraryScope& m_libraryScope;
    GraphicView* m_pView;               //the owner of this generator
    list<VisualEffect*> m_effects;      //managed visual effects
    RenderingBuffer* m_pCanvasBuffer;   //the rendering buffer
    RenderingBuffer m_savedBuffer;      //clean copy of rendering buffer
    bool m_fBackgroundDirty;            //overlays already applied to rendering buffer
    bool m_fFullRectangle;              //damaged rectangle is all screen
    int8u* m_pSaveBytes;                //the real buffer for the clean copy
    URect m_damagedRect;
    URect m_prevDamagedRect;
    GmoObj* m_pHandlersOwner;           //object owning current defined handlers

public:
    OverlaysGenerator(GraphicView* view, LibraryScope& libraryScope);
    ~OverlaysGenerator();

    //operations
    void update_all_visual_effects(ScreenDrawer* pDrawer);
    void update_visual_effect(VisualEffect* pEffect, ScreenDrawer* pDrawer);
    void set_rendering_buffer(RenderingBuffer* rbuf);
    void on_new_background();
    void add_visual_effect(VisualEffect* pEffect);
    void remove_visual_effect(VisualEffect* pEffect);

    //info
    URect get_damaged_rectangle();
    inline void set_handlers_owner(GmoObj* pGmo) { m_pHandlersOwner = pGmo; }
    inline GmoObj* get_handlers_owner() { return m_pHandlersOwner; }

protected:
    void save_rendering_buffer();
    void expand_damaged_rectangle();


};


}   //namespace lomse

#endif      //__LOMSE_OVERLAYS_GENERATOR_H__

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
class BitmapDrawer;
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
    RenderingBuffer m_canvasBuffer;    //the rendering buffer
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
    void update_all_visual_effects(BitmapDrawer* pDrawer);
    void update_visual_effect(VisualEffect* pEffect, BitmapDrawer* pDrawer);
    void set_rendering_buffer(unsigned char* buf, unsigned width, unsigned height);
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

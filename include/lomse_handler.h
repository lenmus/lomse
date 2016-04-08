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

#ifndef __LOMSE_HANDLER_H__
#define __LOMSE_HANDLER_H__

#include "lomse_visual_effect.h"


namespace lomse
{

//forward declarations
class Drawer;
struct RenderOptions;
class ScreenDrawer;

//---------------------------------------------------------------------------------------
// Base class for controls used for interacting and re-shaping objects
class Handler : public VisualEffect
{
protected:
    GmoObj*   m_pControlledGmo;
    int       m_index;           //the index the Gmo assigned to this handler

    Handler(GraphicView* view, LibraryScope& libraryScope,
            GmoObj* pControlledGmo, int index);

public:
    virtual ~Handler() {}

    virtual bool hit_test(LUnits x, LUnits y) = 0;
    virtual void move_to(UPoint pos) = 0;

    inline GmoObj* get_controlled_gmo() { return m_pControlledGmo; }
    inline int get_handler_index() { return m_index; }

};

//---------------------------------------------------------------------------------------
// A handler drawn as a circle
class HandlerCircle : public Handler
{
protected:
    UPoint      m_origin;       //circle center
    LUnits      m_radius;
    Color       m_borderColor;
    Color       m_fillColor;

public:
    HandlerCircle(GraphicView* view, LibraryScope& libraryScope,
                  GmoObj* pControlledGmo, int index);
    virtual ~HandlerCircle();

    //mandatory overrides from VisualEffect
    void on_draw(ScreenDrawer* pDrawer);
    URect get_bounds();

    //mandatory overrides from Handler
    bool hit_test(LUnits x, LUnits y);
    void move_to(UPoint pos);


protected:
    void compute_radius(ScreenDrawer* pDrawer);

};


//class HandlerSquare : public Handler
//{
//protected:
//    UPoint    m_uTopLeft;     //square top left point
//    LUnits    m_uSide;        //square side lenght
//
//public:
//    HandlerSquare(lmScoreObj* pOwner, lmGMObject* pOwnerGMO, long nHandlerID,
//                    wxStockCursor nCursorId = wxCURSOR_SIZENESW);
//    HandlerSquare(lmScoreObj* pOwner, lmGMObject* pOwnerGMO, long nHandlerID,
//                    wxCursor* pCursor);
//    virtual ~HandlerSquare() {}
//
//	//implementation of pure virtual methods in base classes
//    void Render(lmPaper* pPaper, wxColour color);
//    wxString Dump(int nIndent);
//
////    //dragging the handler
////    void OnMouseIn(wxWindow* pWindow, lmUPoint& uPoint);
////
////    //operations
////    void SetHandlerCenterPoint(lmLUnits uxPos, lmLUnits uyPos);
////    void SetHandlerCenterPoint(lmUPoint uPos);
////    void SetHandlerTopLeftPoint(lmUPoint uPos);
////    lmUPoint GetHandlerCenterPoint();
////    inline lmUPoint GetTopCenterDistance() { return lmUPoint(m_uSide/2.0, m_uSide/2.0); }
//
//protected:
//    void OnPointsChanged();
//
//};


}   //namespace lomse

#endif      //__LOMSE_HANDLER_H__

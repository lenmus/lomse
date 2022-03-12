//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_HANDLER_H__
#define __LOMSE_HANDLER_H__

#include "lomse_visual_effect.h"


namespace lomse
{

//forward declarations
class Drawer;
struct RenderOptions;
class BitmapDrawer;

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
    void on_draw(BitmapDrawer* pDrawer) override;
    URect get_bounds() override;

    //mandatory overrides from Handler
    bool hit_test(LUnits x, LUnits y) override;
    void move_to(UPoint pos) override;


protected:
    void compute_radius(BitmapDrawer* pDrawer);

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

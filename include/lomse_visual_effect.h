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

#ifndef __LOMSE_VISUAL_EFFECT_H__
#define __LOMSE_VISUAL_EFFECT_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"

//other
#include <list>
using namespace std;


namespace lomse
{

//forward declarations
class ScreenDrawer;
class GraphicView;
class GmoShape;
class GmoObj;
class SelectionSet;

typedef std::shared_ptr<GmoShape>  SpGmoShape;


//---------------------------------------------------------------------------------------
// VisualEffect: any real time visual graphical effect. Base class
class VisualEffect
{
protected:
    LibraryScope& m_libraryScope;
    GraphicView* m_pView;       //the view to which this visual effect is associated
    bool m_fVisible;
    bool m_fEnabled;

    VisualEffect(GraphicView* view, LibraryScope& libraryScope);

public:
    virtual ~VisualEffect() {}

    //state
    inline void set_visible(bool value) { m_fVisible = value; }
    inline void hide() { set_visible(false); }
    inline void show() { set_visible(true); }
    inline bool is_visible() { return m_fVisible; }
    inline void enable(bool fEnabled) { m_fEnabled = fEnabled; }

    //drawing
    virtual void on_draw(ScreenDrawer* pDrawer) = 0;

    //size when rendered
    virtual URect get_bounds() = 0;

protected:

};

//---------------------------------------------------------------------------------------
// DraggedImage: an image to be attached to mouse cursor
class DraggedImage : public VisualEffect
{
protected:
    UPoint m_origin;
    UPoint m_offset;
    GmoShape* m_pShape;     //the shape to draw
    bool m_fShapeIsOwned;

public:
    DraggedImage(GraphicView* view, LibraryScope& libraryScope);
    virtual ~DraggedImage();

    //settings
    void set_shape(GmoShape* pShape, bool fGetOwnership, UPoint offset);

    //positioning
    void move_to(LUnits x, LUnits y);

    //mandatory overrides from VisualEffect
    void on_draw(ScreenDrawer* pDrawer);
    URect get_bounds();

protected:
    void delete_shape();

};

//---------------------------------------------------------------------------------------
// SelectionRectangle: a rectangle for displaying selected area
class SelectionRectangle : public VisualEffect
{
protected:
    LUnits m_xStart;
    LUnits m_yStart;
    LUnits m_left;
    LUnits m_top;
    LUnits m_right;
    LUnits m_bottom;

public:
    SelectionRectangle(GraphicView* view, LibraryScope& libraryScope);
    virtual ~SelectionRectangle() {}

    //position and size
    void set_start_point(LUnits x, LUnits y);
    void set_end_point(LUnits x, LUnits y);

    //mandatory overrides from VisualEffect
    void on_draw(ScreenDrawer* pDrawer);
    URect get_bounds();
};

//---------------------------------------------------------------------------------------
// PlaybackHighlight: an effect for highlighting note/rests during playback
class PlaybackHighlight : public VisualEffect
{
protected:
    list<GmoShape*> m_noterests;
    URect m_bounds;

public:
    PlaybackHighlight(GraphicView* view, LibraryScope& libraryScope);
    virtual ~PlaybackHighlight() {}

    //settings
    void add_highlight(GmoShape* pShape);
    void remove_highlight(GmoShape* pShape);
    void remove_all_highlight();

    //mandatory overrides from VisualEffect
    void on_draw(ScreenDrawer* pDrawer);
    URect get_bounds();
};

//---------------------------------------------------------------------------------------
// SelectionHighlight: an effect for highlighting selected objects
class SelectionHighlight : public VisualEffect
{
protected:
    SelectionSet* m_pSelectionSet;
    URect m_bounds;

public:
    SelectionHighlight(GraphicView* view, LibraryScope& libraryScope,
                       SelectionSet* pSelectionSet);
    virtual ~SelectionHighlight() {}

    //mandatory overrides from VisualEffect
    void on_draw(ScreenDrawer* pDrawer);
    URect get_bounds();

    //other
    bool are_handlers_needed();
    GmoObj* get_object_needing_handlers();

};


}   //namespace lomse

#endif      //__LOMSE_VISUAL_EFFECT_H__

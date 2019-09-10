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


///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class ScreenDrawer;
class GraphicView;
class GmoShape;
class GmoObj;
class SelectionSet;

typedef std::shared_ptr<GmoShape>  SpGmoShape;


//---------------------------------------------------------------------------------------
/** %VisualEffect is the base class for any real time visual graphic effect, that is,
    for any overlay to be displayed on top of the document layer, such as
    dragged images, selection rectangles, visual tracking effects during playback, etc.
*/
class VisualEffect
{
protected:
    LibraryScope& m_libraryScope;
    GraphicView* m_pView;       //the view to which this visual effect is associated
    bool m_fVisible;
    bool m_fEnabled;

    VisualEffect(GraphicView* view, LibraryScope& libraryScope);


///@cond INTERNALS
//excluded from public API. Only for internal use.

public:

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
    friend class OverlaysGenerator;
    virtual ~VisualEffect() {}

    //Returns the GraphicView to which this visual effect is associated
    inline GraphicView* get_view() { return m_pView; }


///@endcond
};


///@cond INTERNALS
//excluded from public API. Only for internal use.
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
///@endcond


//---------------------------------------------------------------------------------------
/** %PlaybackHighlight is a visual tracking effect for highlighting notes and rests
    during playback.
*/
class PlaybackHighlight : public VisualEffect
{
protected:
    list<GmoShape*> m_noterests;
    URect m_bounds;
    Color m_color;

public:
    //customizable properties

    /** Return the current color for highlighting notes and rests. */
    inline Color get_color() const { return m_color; }

    /** Set the color to use for highlighting notes and rests. It is recommended to
        use a solid (not transparent) color.
    */
    inline void set_color(Color color) { m_color = color; }

///@cond INTERNALS
//excluded from public API. Only for internal use.

    PlaybackHighlight(GraphicView* view, LibraryScope& libraryScope);
    virtual ~PlaybackHighlight() {}

    //settings
    void add_highlight(GmoShape* pShape);
    void remove_highlight(GmoShape* pShape);
    void remove_all_highlight();

    //mandatory overrides from VisualEffect
    void on_draw(ScreenDrawer* pDrawer);
    URect get_bounds();

///@endcond
};


///@cond INTERNALS
//excluded from public API. Only for internal use.
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
///@endcond


}   //namespace lomse

#endif      //__LOMSE_VISUAL_EFFECT_H__

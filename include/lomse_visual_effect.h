//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
class BitmapDrawer;
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
    virtual void on_draw(BitmapDrawer* pDrawer) = 0;

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
    void on_draw(BitmapDrawer* pDrawer) override;
    URect get_bounds() override;

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
    void on_draw(BitmapDrawer* pDrawer) override;
    URect get_bounds() override;
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
    void on_draw(BitmapDrawer* pDrawer) override;
    URect get_bounds() override;

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
    void on_draw(BitmapDrawer* pDrawer) override;
    URect get_bounds() override;

    //other
    bool are_handlers_needed();
    GmoObj* get_object_needing_handlers();

};
///@endcond


}   //namespace lomse

#endif      //__LOMSE_VISUAL_EFFECT_H__

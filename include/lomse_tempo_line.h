//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_TEMPO_LINE_H__
#define __LOMSE_TEMPO_LINE_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_visual_effect.h"

//other
#include <iostream>
using namespace std;


///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class GmoBoxSystem;


//---------------------------------------------------------------------------------------
/** %TempoLine is a playback visual tracking effect to display a vertical line at the
    location of the beat being played back.
*/
class TempoLine : public VisualEffect
{
protected:
    Color m_color;
    LUnits m_width;
    GmoBoxSystem* m_pBoxSystem;
    URect m_bounds;
    int m_iPage;

public:

    //customizable properties

    /** Return the current color for the line. */
    inline Color get_color() const { return m_color; }

    /** Set the color to use for drawing the line. If the line width is large, use
        a transparent color so that the line does not totally hide the notes/rests.
        @param color New color to use.
    */
    inline void set_color(Color color) { m_color = color; }

    /** Returns the current width (in logical units) of the line. */
    inline LUnits get_width() const { return m_width; }

    /** Set the width to use for drawing the line.
        @param width New line width, in logical units.
    */
    inline void set_width(LUnits width) { m_width = width; }


///@cond INTERNALS
//excluded from public API. Only for internal use.
    TempoLine(GraphicView* view, LibraryScope& libraryScope);
    virtual ~TempoLine() {}

    //operations
    void move_to(GmoShape* pShape, GmoBoxSystem* pBoxSystem);
    void move_to(LUnits xPos, GmoBoxSystem* pBoxSystem, int iPage);
    void remove_tempo_line();

    //mandatory overrides from VisualEffect
    void on_draw(BitmapDrawer* pDrawer) override;
    URect get_bounds() override { return m_bounds; }

protected:

///@endcond
};


}   //namespace lomse

#endif      //__LOMSE_TEMPO_LINE_H__

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2018. All rights reserved.
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
class ScreenDrawer;
class GraphicView;
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
    void on_draw(ScreenDrawer* pDrawer);
    URect get_bounds() { return m_bounds; }

protected:

///@endcond
};


}   //namespace lomse

#endif      //__LOMSE_TEMPO_LINE_H__

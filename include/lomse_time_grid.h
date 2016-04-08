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

#ifndef __LOMSE_TIME_GRID_H__
#define __LOMSE_TIME_GRID_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_visual_effect.h"

//other
#include <iostream>
using namespace std;


namespace lomse
{

//forward declarations
class ScreenDrawer;
class GraphicView;
class GmoBoxSystem;


//---------------------------------------------------------------------------------------
// TimeGrid: a grid to show time positions on the score
class TimeGrid : public VisualEffect
{
protected:
    Color m_color;
    GmoBoxSystem* m_pBoxSystem;     //active system: the one on which the caret is placed
    URect m_bounds;

public:
    TimeGrid(GraphicView* view, LibraryScope& libraryScope);
    virtual ~TimeGrid() {}

    //operations
    void on_draw(ScreenDrawer* pDrawer);
    URect get_bounds() { return m_bounds; }

    //getters
    inline Color get_color() const { return m_color; }

    //set properties
    inline void set_color(Color color) { m_color = color; }
    inline void set_system(GmoBoxSystem* pSystem) { m_pBoxSystem = pSystem; }

protected:

};


}   //namespace lomse

#endif      //__LOMSE_TIME_GRID_H__

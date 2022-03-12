//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
class BitmapDrawer;
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
    void on_draw(BitmapDrawer* pDrawer) override;
    URect get_bounds() override { return m_bounds; }

    //getters
    inline Color get_color() const { return m_color; }

    //set properties
    inline void set_color(Color color) { m_color = color; }
    inline void set_system(GmoBoxSystem* pSystem) { m_pBoxSystem = pSystem; }

protected:

};


}   //namespace lomse

#endif      //__LOMSE_TIME_GRID_H__

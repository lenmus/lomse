//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#ifndef __LOMSE_SHAPE_PEDAL_H__
#define __LOMSE_SHAPE_PEDAL_H__

#include "lomse_basic.h"
#include "lomse_drawer.h"
#include "lomse_shape_base.h"

#include <vector>

namespace lomse
{

//forward declarations
class ImoObj;


//---------------------------------------------------------------------------------------
class GmoShapePedalLine : public GmoCompositeShape
{
protected:
    struct LineGap {
        LUnits xStart;
        LUnits xEnd;
    };

    std::vector<LineGap> m_lineGaps;

    LUnits m_xLineStart;
    LUnits m_yLineStart;
    LUnits m_yLineEnd;
    LUnits m_uLineThick;
    bool m_fStartCorner;
    bool m_fEndCorner;

public:
    GmoShapePedalLine(ImoObj* pCreatorImo, ShapeId idx, Color color);

    void set_layout_data(LUnits xStart, LUnits xEnd, LUnits yStart, LUnits yEnd,
                         LUnits uLineThick, bool fStartCorner, bool fEndCorner);
    void add_line_gap(LUnits xStart, LUnits xEnd);

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;
};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_PEDAL_H__


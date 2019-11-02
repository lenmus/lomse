//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#ifndef __LOMSE_SHAPE_BEAM_H__
#define __LOMSE_SHAPE_BEAM_H__

#include "lomse_basic.h"
#include "lomse_shape_base.h"

namespace lomse
{

//forward declarations
class ImoObj;
class Drawer;
class BeamEngraver;


//---------------------------------------------------------------------------------------
class GmoShapeBeam : public GmoSimpleShape, public VoiceRelatedShape
{
protected:
    LUnits m_uBeamThickness;
    std::list<LUnits> m_segments;
	UPoint m_outerLeftPoint;
    UPoint m_outerRightPoint;
    unsigned int m_BeamFlags;

    friend class BeamEngraver;
    GmoShapeBeam(ImoObj* pCreatorImo, LUnits uBeamThickness,
                 Color color = Color(0,0,0));

public:
    ~GmoShapeBeam();

    void set_layout_data(std::list<LUnits>& segments, UPoint origin, USize size,
                         UPoint outerLeft, UPoint outerRight);
    void on_draw(Drawer* pDrawer, RenderOptions& opt);

    //provide geometry reference info, for other related shapes
    UPoint get_outer_left_reference_point() { return m_outerLeftPoint; }
    UPoint get_outer_right_reference_point() { return m_outerRightPoint; }

    //layout
    inline bool is_cross_staff() { return (m_BeamFlags & k_cross_staff) != 0; }


protected:
	void draw_beam_segment(Drawer* pDrawer, LUnits uxStart, LUnits uyStart,
                           LUnits uxEnd, LUnits uyEnd, Color color);

    //flag values
    enum {
        k_cross_staff       = 0x0001,   //the beam has stems (flag segment) in at least two staves
    };

    inline void set_cross_staff(bool value) {
        value ? m_BeamFlags |= k_cross_staff : m_BeamFlags &= ~k_cross_staff;
    }

};


}   //namespace lomse

#endif      //__LOMSE_SHAPE_BEAM_H__

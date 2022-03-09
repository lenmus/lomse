//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
    unsigned int m_BeamFlags;
    int m_staff;

public:
    GmoShapeBeam(ImoObj* pCreatorImo, LUnits uBeamThickness,
                 Color color = Color(0,0,0));
    ~GmoShapeBeam();

    void set_layout_data(std::list<LUnits>& segments, UPoint origin, USize size,
                         bool fCrossStaff, bool fChord, int beamPos, int staff);
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //provide geometry reference info, for tuplets and other related shapes
    //Reference points are:
    //- top of principal beam when beam above
    //- bottom of principal beam when beam below
    //- center line of principal beam when double-stemmed
    UPoint get_outer_left_reference_point();
    UPoint get_outer_right_reference_point();

    //layout
    inline bool is_cross_staff() { return (m_BeamFlags & k_cross_staff) != 0; }
    inline bool has_chords() { return (m_BeamFlags & k_has_chords) != 0; }
    inline bool get_staff() { return m_staff; }
    inline bool is_beam_below() { return (m_BeamFlags & k_beam_below) != 0; }
    inline bool is_beam_above() { return (m_BeamFlags & k_beam_above) != 0; }
    inline bool is_double_stemmed_beam() { return (m_BeamFlags & k_beam_double_stemmed) != 0; }


protected:
	void draw_beam_segment(Drawer* pDrawer, LUnits uxStart, LUnits uyStart,
                           LUnits uxEnd, LUnits uyEnd, Color color);

    //flag values
    enum {
        k_cross_staff           = 0x0001,   //the beam has stems (flag segment) in at least two staves
        k_has_chords            = 0x0002,   //the beam has at least one chord
        k_beam_below            = 0x0004,   //the beam is placed below
        k_beam_above            = 0x0008,   //the beam is placed above
        k_beam_double_stemmed   = 0x0010,   //the beam is double stemmed
    };

};


}   //namespace lomse

#endif      //__LOMSE_SHAPE_BEAM_H__

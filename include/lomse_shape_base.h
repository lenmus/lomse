//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_BASE_H__        //to avoid nested includes
#define __LOMSE_SHAPE_BASE_H__

#include "lomse_gm_basic.h"
//#include <vector>
#include <list>
using namespace std;

namespace lomse
{

////---------------------------------------------------------------------------------------
////level of shape
//enum lmEShapeLevel
//{
//    lm_eMainShape = 0,      //the normal, single shape generated by an ScoreObj
//    lm_eSecondaryShape,     //key and time signatures in system 1 for staves > 1
//    lm_ePrologShape,        //clef and key signatures in systems > 1
//};
//
//#define lmSELECTABLE         true
//#define lmNO_SELECTABLE      false





//---------------------------------------------------------------------------------------
// enums for common values: aligment, justification, placement, etc.

//line styles
enum ELineStyle { k_line_none=0, k_line_solid, k_line_long_dash,
                  k_line_short_dash, k_line_dot, k_line_dot_dash, };

//line termination styles
enum ELineCap { k_cap_none = 0, k_cap_arrowhead, k_cap_arrowtail,
                k_cap_circle, k_cap_square, k_cap_diamond, };

//line edges
enum ELineEdge
{
    k_edge_normal = 0,        // edge is perpendicular to line
    k_edge_vertical,          // edge is always a vertical line
    k_edge_horizontal         // edge is always a horizontal line
};


//---------------------------------------------------------------------------------------
//EVAlign is used to indicate vertical alignment within a block: to the top,
//middle or bottom
enum EVAlign
{
    k_valign_default = 0,   //alignment is not specified
    k_valign_top,
    k_valign_middle,
    k_valign_bottom,
};

//---------------------------------------------------------------------------------------
// EHAlign is used to indicate object justification
enum EHAlign
{
    k_halign_default = 0,   //alignment is not specified
    k_halign_left,          //object aligned on left side
    k_halign_right,         //object aligned on right side
    k_halign_justify,       //object justified on both sides
    k_halign_center,        //object centered
};

//---------------------------------------------------------------------------------------
enum ELinkType
{
	k_link_simple = 0,
    k_link_start,
	k_link_middle,
	k_link_end,
};



//---------------------------------------------------------------------------------------
// auxiliary, to identify staffobjs associated to a voice and manage voice data
class VoiceRelatedShape
{
protected:
    int m_voice;

public:
    VoiceRelatedShape() : m_voice(0) {}
    virtual ~VoiceRelatedShape() {}

    //overrides required by shapes related to scores
    bool is_voice_related_shape() { return true; }

    //specific info
    inline void set_voice(int voice) { m_voice = voice; }
    inline int get_voice() { return m_voice; }
};



//---------------------------------------------------------------------------------------
class GmoSimpleShape : public GmoShape
{
public:
    virtual ~GmoSimpleShape();

protected:
    GmoSimpleShape(ImoObj* pCreatorImo, int objtype, ShapeId idx, Color color);
};

//---------------------------------------------------------------------------------------
class GmoCompositeShape : public GmoShape
{
protected:
	std::list<GmoShape*> m_components;	//constituent shapes

public:
    ~GmoCompositeShape() override;

    virtual int add(GmoShape* pShape);

    //may be needed in case child shapes changed their positions independently of the entire composite shape
    void force_recompute_bounds() { recompute_bounds(); }

    //virtual methods from base class
    virtual void shift_origin(const USize& shift) override;
    virtual void reposition_shape(LUnits yShift) override;
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //for tests and debug
    inline std::list<GmoShape*>& get_components() { return m_components; }
    void set_color(Color color) override;

protected:
    GmoCompositeShape(ImoObj* pCreatorImo, int objtype, ShapeId idx, Color color);
	void recompute_bounds();
};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_BASE_H__


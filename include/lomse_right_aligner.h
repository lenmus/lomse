//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_RIGHT_ALIGNER_H__        //to avoid nested includes
#define __LOMSE_RIGHT_ALIGNER_H__

#include "lomse_basic.h"
#include "lomse_shape_base.h"

namespace lomse
{

//---------------------------------------------------------------------------------------
// RightAligner: algorithm for aligning shapes to the right, as if the right
// was the bottom and the shapes were drop from top.
class RightAligner
{
protected:
    list<URect> m_boxes;
    list<UPoint> m_border;      //border segment points (x, y)
    LUnits m_width;             //surrounding rectangle width

    friend class PartsEngraver;
    RightAligner();

public:
	virtual ~RightAligner();

    int add_box(URect rect);
    URect get_box(int iBox);
    LUnits get_total_height();
    inline LUnits get_total_width() { return m_width; }

protected:
    void shift_boxes_right(LUnits shift);
    void shift_border(LUnits shift);
    LUnits get_touch_x_pos(LUnits y0, LUnits y1);
    void add_border_segment(LUnits y0, LUnits y1, LUnits x);

};


}   //namespace lomse

#endif    // __LOMSE_RIGHT_ALIGNER_H__


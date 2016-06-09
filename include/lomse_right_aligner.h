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


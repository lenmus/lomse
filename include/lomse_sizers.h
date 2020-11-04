//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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

#ifndef __LOMSE_SIZERS_H__
#define __LOMSE_SIZERS_H__

#include "lomse_basic.h"
#include <list>
//#include <iostream>

using namespace std;

namespace lomse
{

//forward declarations
class GmoBox;

#define _LOMSE_CAN_GROW   -1.0f   //for width, heigt. Means: grow as much as needed

// the root abstract class for any sizer content item
//------------------------------------------------------------------------------
class SizerChild
{
protected:
    LUnits m_minWidth;      //minimal size
    LUnits m_minHeight;
    LUnits m_marginLeft;    //margins
    LUnits m_marginTop;
    LUnits m_marginRight;
    LUnits m_marginBottom;
    int m_alignment;
    int m_stretchFactor;
    GmoBox* m_pControlledBox;

public:
    SizerChild(GmoBox* pBox, int factor=0, int alignment=k_expand);
    ~SizerChild();

    enum { k_align_top=0, k_align_bottom, k_center, k_expand,
           k_align_left, k_align_right };

    LUnits get_width();
    LUnits get_height();
    void set_width(LUnits width);
    void set_height(LUnits height);
};

// the root abstract class for sizers
//------------------------------------------------------------------------------
class Sizer
{
protected:
    std::list<SizerChild*> m_children;

public:
    Sizer();
    virtual ~Sizer();

    virtual void layout(LUnits width, LUnits height)=0;
    inline void add_child(SizerChild* pChild) { m_children.push_back(pChild); }

protected:
};

// FlowSizer
//------------------------------------------------------------------------------
class FlowSizer : public Sizer
{
protected:
    int m_orientation;

public:
    FlowSizer(int orientation=k_vertical);
    virtual ~FlowSizer() {}

    enum { k_vertical=0, k_horizontal, };

    inline int get_orientation() const { return m_orientation; }
    inline void set_orientation(int orientation) { m_orientation = orientation; }
    inline bool is_vertical() { return m_orientation == k_vertical; }

    void layout(LUnits width, LUnits height) override;

protected:
    void layout_vertical(LUnits width, LUnits height);
    void layout_horizontal(LUnits width, LUnits height);

};


}   //namespace lomse

#endif      //__LOMSE_SIZERS_H__

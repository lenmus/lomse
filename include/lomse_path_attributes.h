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

#ifndef __LOMSE_PATH_ATTRIBUTES_H__        //to avoid nested includes
#define __LOMSE_PATH_ATTRIBUTES_H__

#include "lomse_build_options.h"
#include "lomse_agg_types.h"

using namespace agg;

namespace lomse
{

//---------------------------------------------------------------------------------------
enum EFillMode
{
    k_fill_none = 0,
    k_fill_solid,
    k_fill_gradient_linear,
    k_fill_gradient_radial,
};



//---------------------------------------------------------------------------------------
// GradientAttributes: struct to contain attributes for a gradient
// - colors: an array of 256 colors with the gradient colors
// - transform: the surface to apply the gradient to is not necessarily 256 pixels long
//       so this transform doest the mapping between the surface and the colors.
// - d1, d2: are two values that define the range to scale the gradient, that is, the
//      total number of steps desired. For instance d1=0, d2=100 means 100 steps

struct GradientAttributes
{
    GradientColors  colors;
    TransAffine     transform;
    double          d1;
    double          d2;

    // Empty constructor
    GradientAttributes()
        : transform()
        , d1(0.0)
        , d2(0.0)
    {
        Color c1(255, 255, 255, 0);     //transparent white
        for (int i=0; i < 256; ++i)
        {
            colors[i] = c1;
        }
    }

    // Copy constructor
    GradientAttributes(GradientAttributes* grad)
        : colors(grad->colors)
        , transform(grad->transform)
        , d1(grad->d1)
        , d2(grad->d2)
    {
    }

};


//---------------------------------------------------------------------------------------
// PathAttributes: struct to contain attributes for a path
struct PathAttributes
{
    unsigned            path_index;
    Color               fill_color;
    Color               stroke_color;
    EFillMode           fill_mode;
    bool                stroke_flag;
    bool                even_odd_flag;
    line_join_e         line_join;
    line_cap_e          line_cap;
    double              miter_limit;
    double              stroke_width;
    TransAffine         transform;
    GradientAttributes* fill_gradient;

    // Empty constructor
    PathAttributes(unsigned idx=0)
        : path_index(idx)
        , fill_color(Color(0,0,0))
        , stroke_color(Color(0,0,0))
        , fill_mode(k_fill_solid)
        , stroke_flag(false)
        , even_odd_flag(false)
        , line_join(miter_join)
        , line_cap(butt_cap)
        , miter_limit(4.0)
        , stroke_width(1.0)
        , transform()
        , fill_gradient(nullptr)
    {
    }

    // Copy constructor
    PathAttributes(const PathAttributes& attr)
        : path_index(attr.path_index)
        , fill_color(attr.fill_color)
        , stroke_color(attr.stroke_color)
        , fill_mode(attr.fill_mode)
        , stroke_flag(attr.stroke_flag)
        , even_odd_flag(attr.even_odd_flag)
        , line_join(attr.line_join)
        , line_cap(attr.line_cap)
        , miter_limit(attr.miter_limit)
        , stroke_width(attr.stroke_width)
        , transform(attr.transform)
        , fill_gradient(nullptr)
    {
        if (attr.fill_gradient)
            fill_gradient = LOMSE_NEW GradientAttributes(attr.fill_gradient);
    }

    // Copy constructor with new index value
    PathAttributes(const PathAttributes& attr, unsigned idx)
        : path_index(idx)
        , fill_color(attr.fill_color)
        , stroke_color(attr.stroke_color)
        , fill_mode(attr.fill_mode)
        , stroke_flag(attr.stroke_flag)
        , even_odd_flag(attr.even_odd_flag)
        , line_join(attr.line_join)
        , line_cap(attr.line_cap)
        , miter_limit(attr.miter_limit)
        , stroke_width(attr.stroke_width)
        , transform(attr.transform)
        , fill_gradient(nullptr)
    {
        if (attr.fill_gradient)
            fill_gradient = LOMSE_NEW GradientAttributes(attr.fill_gradient);
    }

    //destructor
    ~PathAttributes()
    {
        //AWARE: This destructor IS NEVER INVOKED
        //AttrStorage objects are pod_bvector<PathAttributes>
        //and pod_bvector doesn't invoke destructors, just dealloc memory. Therefore,
        //memory allocated for GradientAttributes is released in
        //ScreenDrawer::delete_paths().
        //fill_gradient is deleted there, so DON'T DO IT HERE

        //delete fill_gradient;
    }
};

typedef pod_bvector<PathAttributes>         AttrStorage;


}   //namespace lomse

#endif    // __LOMSE_PATH_ATTRIBUTES_H__


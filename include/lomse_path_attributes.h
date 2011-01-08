//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_PATH_ATTRIBUTES_H__        //to avoid nested includes
#define __LOMSE_PATH_ATTRIBUTES_H__

#include "lomse_agg_types.h"

using namespace agg;

namespace lomse
{

// PathAttributes: struct to contain attributes for a path
//---------------------------------------------------------------------------------------
struct PathAttributes
{
    unsigned     index;
    Color        fill_color;
    Color        stroke_color;
    bool         fill_flag;
    bool         stroke_flag;
    bool         even_odd_flag;
    line_join_e  line_join;
    line_cap_e   line_cap;
    double       miter_limit;
    double       stroke_width;
    TransAffine  transform;

    // Empty constructor
    PathAttributes()
        : index(0)
        , fill_color(Color(0,0,0))
        , stroke_color(Color(0,0,0))
        , fill_flag(true)
        , stroke_flag(false)
        , even_odd_flag(false)
        , line_join(miter_join)
        , line_cap(butt_cap)
        , miter_limit(4.0)
        , stroke_width(1.0)
        , transform()
    {
    }

    // Copy constructor
    PathAttributes(const PathAttributes& attr)
        : index(attr.index)
        , fill_color(attr.fill_color)
        , stroke_color(attr.stroke_color)
        , fill_flag(attr.fill_flag)
        , stroke_flag(attr.stroke_flag)
        , even_odd_flag(attr.even_odd_flag)
        , line_join(attr.line_join)
        , line_cap(attr.line_cap)
        , miter_limit(attr.miter_limit)
        , stroke_width(attr.stroke_width)
        , transform(attr.transform)
    {
    }

    // Copy constructor with new index value
    PathAttributes(const PathAttributes& attr, unsigned idx)
        : index(idx)
        , fill_color(attr.fill_color)
        , stroke_color(attr.stroke_color)
        , fill_flag(attr.fill_flag)
        , stroke_flag(attr.stroke_flag)
        , even_odd_flag(attr.even_odd_flag)
        , line_join(attr.line_join)
        , line_cap(attr.line_cap)
        , miter_limit(attr.miter_limit)
        , stroke_width(attr.stroke_width)
        , transform(attr.transform)
    {
    }
};


typedef pod_bvector<PathAttributes>     AttrStorage;



}   //namespace lomse

#endif    // __LOMSE_PATH_ATTRIBUTES_H__


//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
//
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_BASIC_H__
#define __LOMSE_BASIC_H__

namespace lomse
{

//internationalization
//#include <libintl.h>
#define TRT(String) (String)      //gettext(String)
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)

//----------------------------------------------------------------------------------
struct FloatPoint
{
    float x;
    float y;

    // constructors
    FloatPoint() : x(0.0f), y(0.0f) { }
    FloatPoint(float rx, float ry) : x(rx), y(ry) { }

    // no copy ctor or assignment operator - defaults are ok

    bool operator==(const FloatPoint& pt) const { return x == pt.x && y == pt.y; }
    bool operator!=(const FloatPoint& pt) const { return x != pt.x || y != pt.y; }

    FloatPoint operator+(const FloatPoint& pt) const { return FloatPoint(x + pt.x, y + pt.y); }
    FloatPoint operator-(const FloatPoint& pt) const { return FloatPoint(x - pt.x, y - pt.y); }

    FloatPoint& operator+=(const FloatPoint& pt) { x += pt.x; y += pt.y; return *this; }
    FloatPoint& operator-=(const FloatPoint& pt) { x -= pt.x; y -= pt.y; return *this; }
};

//----------------------------------------------------------------------------------
struct FloatSize
{
    float width;
    float height;

    // constructors
    FloatSize() : width(0.0f), height(0.0f) {}
    FloatSize(float w, float h) : width(w), height(h) {}

    // no copy ctor or assignment operator - the defaults are ok

    bool operator==(const FloatSize& sz) const { return width == sz.width && height == sz.height; }
    bool operator!=(const FloatSize& sz) const { return width != sz.width || height != sz.height; }

    FloatSize operator+(const FloatSize& sz) const { return FloatSize(width + sz.width, height + sz.height); }
    FloatSize operator-(const FloatSize& sz) const { return FloatSize(width - sz.width, height - sz.height); }
    FloatSize operator/(float i) const { return FloatSize(width / i, height / i); }
    FloatSize operator*(float i) const { return FloatSize(width * i, height * i); }

    FloatSize& operator+=(const FloatSize& sz) { width += sz.width; height += sz.height; return *this; }
    FloatSize& operator-=(const FloatSize& sz) { width -= sz.width; height -= sz.height; return *this; }
    FloatSize& operator/=(const float i) { width /= i; height /= i; return *this; }
    FloatSize& operator*=(const float i) { width *= i; height *= i; return *this; }
};

//specific types
typedef float Tenths;       //relative unit: one tenth of staff interline space
typedef FloatPoint TPoint;  //point, in Tenths
typedef FloatSize TSize;    //size, in Tenths
typedef float LUnits;       //absolute unit (logical unit): one cent of mm
typedef FloatPoint UPoint;  //point, in LUnits
typedef FloatSize USize;    //size, in LUnits
typedef unsigned short int16u;

//------------------------------------------------------------------------------
// color type: rgba 16bits.
// I will copy this definition from agg::rgba16. The idea is to
// replace my definition by agg::rgba16 when the graphic layer is developed.
// Remember: alpha 0 means transparent and 255 means opaque
//------------------------------------------------------------------------------

struct rgba16
{
    int16u r;
    int16u g;
    int16u b;
    int16u a;

    enum 
    {
        base_mask  = 255
    };

    rgba16() : r(0), g(0), b(0), a(base_mask) {}

    rgba16(unsigned r_, unsigned g_, unsigned b_, unsigned a_= base_mask) :
        r(int16u(r_)),
        g(int16u(g_)),
        b(int16u(b_)),
        a(int16u(a_)) {}

    rgba16(const rgba16& c, unsigned a_) :
        r(c.r), g(c.g), b(c.b), a(int16u(a_)) {}

    bool operator == (rgba16 color)
    {
        return r == color.r && g == color.g && b == color.b && a == color.a ;
    }

    bool operator != (rgba16 color)
    {
        return r != color.r || g != color.g || b != color.b || a != color.a ;
    }

    void clear()
    {
        r = g = b = a = 0;
    }

    const rgba16& transparent()
    {
        a = 0;
        return *this;
    }

    static rgba16 no_color() { return rgba16(0,0,0,0); }

};


}   //namespace lomse

#endif	// __LOMSE_BASIC_H__


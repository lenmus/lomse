//---------------------------------------------------------------------------------------
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
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_BASIC_H__
#define __LOMSE_BASIC_H__

namespace lomse
{

//internationalization
//#include <libintl.h>
#define TRT(String) (String)      //gettext(String)
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)

//---------------------------------------------------------------------------------------
template<class T>
struct Point
{
    T x;
    T y;

    // constructors
    Point() : x(0), y(0) { }
    Point(T rx, T ry) : x(rx), y(ry) { }

    // no copy ctor or assignment operator - defaults are ok

    bool operator==(const Point<T>& pt) const { return x == pt.x && y == pt.y; }
    bool operator!=(const Point<T>& pt) const { return x != pt.x || y != pt.y; }

    Point<T> operator+(const Point<T>& pt) const { return Point<T>(x + pt.x, y + pt.y); }
    Point<T> operator-(const Point<T>& pt) const { return Point<T>(x - pt.x, y - pt.y); }

    Point<T>& operator+=(const Point<T>& pt) { x += pt.x; y += pt.y; return *this; }
    Point<T>& operator-=(const Point<T>& pt) { x -= pt.x; y -= pt.y; return *this; }
};

//---------------------------------------------------------------------------------------
template<class T>
struct Size
{
    T width;
    T height;

    // constructors
    Size() : width(0), height(0) {}
    Size(T w, T h) : width(w), height(h) {}

    // no copy ctor or assignment operator - the defaults are ok

    bool operator==(const Size<T>& sz) const { return width == sz.width && height == sz.height; }
    bool operator!=(const Size<T>& sz) const { return width != sz.width || height != sz.height; }

    Size<T> operator+(const Size<T>& sz) const { return Size<T>(width + sz.width, height + sz.height); }
    Size<T> operator-(const Size<T>& sz) const { return Size<T>(width - sz.width, height - sz.height); }
    Size<T> operator/(T i) const { return Size<T>(width / i, height / i); }
    Size<T> operator*(T i) const { return Size<T>(width * i, height * i); }

    Size<T>& operator+=(const Size<T>& sz) { width += sz.width; height += sz.height; return *this; }
    Size<T>& operator-=(const Size<T>& sz) { width -= sz.width; height -= sz.height; return *this; }
    Size<T>& operator/=(const T i) { width /= i; height /= i; return *this; }
    Size<T>& operator*=(const T i) { width *= i; height *= i; return *this; }
};

//---------------------------------------------------------------------------------------
template<class T>
struct Rectangle
{
    T width;
    T height;
    T x, y;     //top left point

    Rectangle()
        : width(0), height(0), x(0), y(0)
        { }
    Rectangle(T xx, T yy, T ww, T hh)
        : x(xx), y(yy), width(ww), height(hh)
        { }
    Rectangle(const Point<T>& topLeft, const Point<T>& bottomRight);
    Rectangle(const Point<T>& pt, const Size<T>& size)
        : x(pt.x), y(pt.y), width(size.x), height(size.y)
        { }
    Rectangle(const Size<T>& size)
        : x(0), y(0), width(size.x), height(size.y)
        { }

    // default copy ctor and assignment operators ok

    T get_x() const { return x; }
    void set_x(T xx) { x = xx; }

    T get_y() const { return y; }
    void set_y(T yy) { y = yy; }

    T get_width() const { return width; }
    void set_width(T w) { width = w; }

    T get_height() const { return height; }
    void set_height(T h) { height = h; }

    Point<T> position() const { return Point<T>(x, y); }
    void position( const Point<T> &p ) { x = p.x; y = p.y; }

    Size<T> size() const { return Size<T>(width, height); }
    void size( const Size<T> &s ) { width = s.width(); height = s.height(); }

    bool is_empty() const { return (width <= 0) || (height <= 0); }

    T left()   const { return x; }
    T top()    const { return y; }
    T bottom() const { return y + height - 1; }
    T right()  const { return x + width - 1; }

    void left(T left) { x = left; }
    void right(T right) { width = right - x + 1; }
    void top(T top) { y = top; }
    void bottom(T bottom) { height = bottom - y + 1; }

    Point<T> top_left_point() const { return position(); }
    void top_left_point(const Point<T> &p) { position(p); }

    Point<T> bottom_right_point() const { return Point<T>(right(), bottom()); }
    void bottom_right_point(const Point<T> &p) { right(p.x); bottom(p.y); }
    void SetRightBottom(const Point<T> &p) { bottom_right_point(p); }

    Point<T> get_top_right() const { return Point<T>(right(), top()); }
    Point<T> get_right_top() const { return get_top_right(); }
    void SetTopRight(const Point<T> &p) { right(p.x); top(p.y); }
    void SetRightTop(const Point<T> &p) { top_left_point(p); }

    Point<T> get_bottom_left() const { return Point<T>(left(), bottom()); }
    Point<T> GetLeftBottom() const { return get_bottom_left(); }
    void SetBottomLeft(const Point<T> &p) { left(p.x); bottom(p.y); }
    void SetLeftBottom(const Point<T> &p) { SetBottomLeft(p); }

    Rectangle& intersect(const Rectangle& rect);
    Rectangle intersect(const Rectangle& rect) const
    {
        Rectangle r = *this;
        r.intersect(rect);
        return r;
    }

    Rectangle& Union(const Rectangle& rect);
    Rectangle Union(const Rectangle& rect) const
    {
        Rectangle r = *this;
        r.Union(rect);
        return r;
    }

    // compare rectangles
    bool operator==(const Rectangle& rect) const;
    bool operator!=(const Rectangle& rect) const { return !(*this == rect); }

    // return true if the point is (not strictly) inside the rect
    bool contains(T px, T py) const
    {
        return ( (px >= x)
                 && (py >= y)
                 && ((py - y) < height)
                 && ((px - x) < width)
               );
    }

    bool contains(const Point<T>& pt) const { return contains(pt.x, pt.y); }
    // return true if the rectangle is (not strictly) inside the rect
    bool contains(const Rectangle& rect) const
    {
        return contains(rect.get_top_left()) && contains(rect.get_bottom_right());
    }


    // return true if the rectangles have a non empty intersection
    bool intersects(const Rectangle& rect) const;
};


//---------------------------------------------------------------------------------------
//specific types
typedef float Tenths;           //relative unit: one tenth of staff interline space
typedef Point<Tenths> TPoint;   //point, in Tenths
typedef Size<Tenths> TSize;     //size, in Tenths
typedef Rectangle<Tenths> TRect; //rectangle, in tenths

typedef float LUnits;           //absolute unit (logical unit): one cent of mm
typedef Point<LUnits> UPoint;   //point, in LUnits
typedef Size<LUnits> USize;     //size, in LUnits
typedef Rectangle<LUnits> URect; //rectangle, in LUnits

typedef int Pixels;             //device units: pixels
typedef Point<Pixels> VPoint;   //point, in pixels
typedef Size<Pixels> VSize;     //size, in pixels
typedef Rectangle<Pixels> VRect; //rectangle, in pixels

typedef unsigned short Int16u;
typedef unsigned char Int8u;


//---------------------------------------------------------------------------------------
// color type: rgba 8bits.
// Remember: alpha 0 means transparent and 255 means opaque
struct Color
{
    Int8u r;
    Int8u g;
    Int8u b;
    Int8u a;

    Color() : r(0), g(0), b(0), a(255) {}

    Color(unsigned r_, unsigned g_, unsigned b_, unsigned a_= 255)
        : r(Int8u(r_)), g(Int8u(g_)), b(Int8u(b_)), a(Int8u(a_)) {}

    Color(const Color& c, unsigned a_) :
        r(c.r), g(c.g), b(c.b), a(Int8u(a_)) {}

    bool operator == (Color color) {
        return r == color.r && g == color.g && b == color.b && a == color.a ;
    }

    bool operator != (Color color){
        return r != color.r || g != color.g || b != color.b || a != color.a ;
    }

    void clear() { r = g = b = a = 0; }

    const Color& transparent() {
        a = 0;
        return *this;
    }

    void red(unsigned value) { r = value; }
    void green(unsigned value) { g = value; }
    void blue(unsigned value) { b = value; }
    void opacity(unsigned value) { a = value; }

    static Color no_color() { return Color(0,0,0,0); }

};


}   //namespace lomse

#endif	// __LOMSE_BASIC_H__


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
    T xTop;
    T yLeft;
    T width;
    T height;
    T x, y;

    Rectangle()
        : x(0), y(0), width(0), height(0)
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

    T GetX() const { return x; }
    void SetX(T xx) { x = xx; }

    T GetY() const { return y; }
    void SetY(T yy) { y = yy; }

    T GetWidth() const { return width; }
    void SetWidth(T w) { width = w; }

    T GetHeight() const { return height; }
    void SetHeight(T h) { height = h; }

    Point<T> GetPosition() const { return Point<T>(x, y); }
    void SetPosition( const Point<T> &p ) { x = p.x; y = p.y; }

    Size<T> GetSize() const { return Size<T>(width, height); }
    void SetSize( const Size<T> &s ) { width = s.GetWidth(); height = s.GetHeight(); }

    bool IsEmpty() const { return (width <= 0) || (height <= 0); }

    T GetLeft()   const { return x; }
    T GetTop()    const { return y; }
    T GetBottom() const { return y + height - 1; }
    T GetRight()  const { return x + width - 1; }

    void SetLeft(T left) { x = left; }
    void SetRight(T right) { width = right - x + 1; }
    void SetTop(T top) { y = top; }
    void SetBottom(T bottom) { height = bottom - y + 1; }

    Point<T> GetTopLeft() const { return GetPosition(); }
    Point<T> GetLeftTop() const { return GetTopLeft(); }
    void SetTopLeft(const Point<T> &p) { SetPosition(p); }
    void SetLeftTop(const Point<T> &p) { SetTopLeft(p); }

    Point<T> GetBottomRight() const { return Point<T>(GetRight(), GetBottom()); }
    Point<T> GetRightBottom() const { return GetBottomRight(); }
    void SetBottomRight(const Point<T> &p) { SetRight(p.x); SetBottom(p.y); }
    void SetRightBottom(const Point<T> &p) { SetBottomRight(p); }

    Point<T> GetTopRight() const { return Point<T>(GetRight(), GetTop()); }
    Point<T> GetRightTop() const { return GetTopRight(); }
    void SetTopRight(const Point<T> &p) { SetRight(p.x); SetTop(p.y); }
    void SetRightTop(const Point<T> &p) { SetTopLeft(p); }

    Point<T> GetBottomLeft() const { return Point<T>(GetLeft(), GetBottom()); }
    Point<T> GetLeftBottom() const { return GetBottomLeft(); }
    void SetBottomLeft(const Point<T> &p) { SetLeft(p.x); SetBottom(p.y); }
    void SetLeftBottom(const Point<T> &p) { SetBottomLeft(p); }

    Rectangle& Intersect(const Rectangle& rect);
    Rectangle Intersect(const Rectangle& rect) const
    {
        Rectangle r = *this;
        r.Intersect(rect);
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
    bool Contains(T x, T y) const;
    bool Contains(const Point<T>& pt) const { return Contains(pt.x, pt.y); }
    // return true if the rectangle is (not strictly) inside the rect
    bool Contains(const Rectangle& rect) const;

    // return true if the rectangles have a non empty intersection
    bool Intersects(const Rectangle& rect) const;
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

    static Color no_color() { return Color(0,0,0,0); }

};


}   //namespace lomse

#endif	// __LOMSE_BASIC_H__


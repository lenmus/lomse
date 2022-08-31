//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_BASIC_H__
#define __LOMSE_BASIC_H__

#include "agg_color_rgba.h"

#include <string>
#include <vector>
#include <memory>
#include <algorithm>    //min
#include <cmath>        //fabs



//---------------------------------------------------------------------------------------
// macro for avoiding warnings when a parameter is not used
#ifdef UNUSED
#elif defined(__GNUC__)
    #define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
    #define UNUSED(x) /*@unused@*/ x
#else
    #define UNUSED(x) /* x */
#endif


//---------------------------------------------------------------------------------------
// macro to mark functions as being deprecated and generate a warning message,
// e.g.:
//    LOMSE_DEPRECATED_MSG("use get_distance() instead")
//    LUnits get_approximate_distance() const;

#if defined(__GNUC__)
    #define LOMSE_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#elif defined(__clang__) && defined(__has_extension)
    #if __has_extension(attribute_deprecated_with_message)
        #define LOMSE_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
    #else
        #define LOMSE_DEPRECATED_MSG(msg) __attribute__((deprecated))
    #endif
#elif defined(__VISUALC__)
    #define LOMSE_DEPRECATED_MSG(msg) __declspec(deprecated("deprecated: " msg))
#else
    #define LOMSE_DEPRECATED_MSG(msg)
#endif


///@cond INTERNALS
namespace lomse
{

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
    T x, y;     //top left point
    T width;
    T height;

    Rectangle()
        : x(0), y(0), width(0), height(0)
        { }
    Rectangle(T xx, T yy, T ww, T hh)
        : x(xx), y(yy), width(ww), height(hh)
        { }
    Rectangle(const Point<T>& topLeft, const Point<T>& bottomRight)
        : x(topLeft.x)
        , y(topLeft.y)
        , width(bottomRight.x - topLeft.x)
        , height(bottomRight.y - topLeft.y)
        { }
    Rectangle(const Point<T>& pt, const Size<T>& size)
        : x(pt.x), y(pt.y), width(size.width), height(size.height)
        { }
    Rectangle(const Size<T>& size)
        : x(0), y(0), width(size.width), height(size.height)
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
    T bottom() const { return y + height; }
    T right()  const { return x + width; }

    void left(T left) { x = left; }
    void right(T right) { width = right - x; }
    void top(T top) { y = top; }
    void bottom(T bottom) { height = bottom - y; }

    Point<T> get_top_left() const { return position(); }
    void set_top_left(const Point<T> &p) { position(p); }

    Point<T> get_bottom_right() const { return Point<T>(right(), bottom()); }
    void set_bottom_right(const Point<T> &p) { right(p.x); bottom(p.y); }

    Rectangle& intersection(const Rectangle& rect)
    {
        T x2 = right();
        T y2 = bottom();

        if (x < rect.x)
            x = rect.x;
        if (y < rect.y)
            y = rect.y;
        if (x2 > rect.right())
            x2 = rect.right();
        if (y2 > rect.bottom())
            y2 = rect.bottom();

        width = x2 - x;
        height = y2 - y;

        if (width <= 0 || height <= 0)
        {
            width = 0;
            height = 0;
        }

        return *this;
    }

//    Rectangle intersection(const Rectangle& rect) const
//    {
//        Rectangle r = *this;
//        r.intersect(rect);
//        return r;
//    }

    Rectangle& Union(const Rectangle& rect)
    {
        // ignore empty rectangles: union with an empty rectangle shouldn't extend
        // this one to (0, 0)
        if ( !width || !height )
        {
            *this = rect;
        }
        else if ( rect.width && rect.height )
        {
            T x1 = std::min(x, rect.x);
            T y1 = std::min(y, rect.y);
            T y2 = std::max(y + height, rect.height + rect.y);
            T x2 = std::max(x + width, rect.width + rect.x);

            x = x1;
            y = y1;
            width = x2 - x1;
            height = y2 - y1;
        }
        //else: we're not empty and rect is empty

        return *this;
    }

    Rectangle Union(const Rectangle& rect) const
    {
        Rectangle r = *this;
        r.Union(rect);
        return r;
    }

    // compare rectangles
    bool operator==(const Rectangle& rect) const
    {
        return this->x == rect.x
               && this->y == rect.y
               && this->width == rect.width
               && this->height == rect.height;
    }
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


//    // return true if the rectangles have a non empty intersection
//    bool intersects(const Rectangle& rect) const;
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
const LUnits k_out_of_model = -100000000000.0f;      //LUnits, when value is out of model

typedef int Pixels;             //device units: pixels
typedef Point<Pixels> VPoint;   //point, in pixels
typedef Size<Pixels> VSize;     //size, in pixels
typedef Rectangle<Pixels> VRect; //rectangle, in pixels

//for compilers that not use <stdint.h>  (e.g., MS VisualStudio 2003)
#ifndef UINT32_MAX
    typedef int             int_least32_t;
    typedef unsigned int    uint_least32_t;
    typedef unsigned short  uint_least16_t;
    typedef unsigned char   uint_least8_t;
#endif

//types for agg
typedef uint_least16_t Int16u;
typedef uint_least8_t Int8u;

//other specific types and constants
typedef int_least32_t ShapeId;      //identifier for GmoShape objects

typedef int_least32_t ImoId;        //identifier for ImoObj objects
const ImoId k_no_imoid = -1;        //value for undefined imo id (MUST BE -1. See Cursor)

typedef std::pair<ImoId, ImoId> GmoRef;        //identifier for GmoObj objects
const GmoRef k_no_gmo_ref = std::make_pair(-1, -1);

typedef double TimeUnits;           //time units (TU). Relative, depends on metronome speed

///@endcond

//=======================================================================================
/** Struct describing the measure location of a musical event or other.

    There are a variety of situations in which the position of an object needs to be
    described not by its absolute timepos but referencing the measure number.

    But although for most common scores just providing the measure number does the job,
    it is necessary to take into account that for polymetric music (music in which not
    all instruments have the same time signature), the measure number is not an absolute
    value, common to all score instruments (score parts), but it is relative to
    each instrument.

    Due to this, the measure location struct also needs a reference to the instrument
    to which the measure number refers.

    The first measure (anacruxis or not) is always measure 0.
    The first instrument is always instrument 0.
*/
struct MeasureLocator
{
    int iInstr;             ///instrument number (0..n)
    int iMeasure;           ///measure number (0..m), for the instrument
    TimeUnits location;     ///TimeUnits from start of measure

    MeasureLocator() : iInstr(-1), iMeasure(-1), location(0.0) {}
    MeasureLocator(int i, int m, TimeUnits l) : iInstr(i), iMeasure(m), location(l) {}

    bool is_valid() const { return iInstr >=0 && iMeasure >= 0; }

};

//forward declarations
class ImoObj;

//=======================================================================================
/** Struct that describes the content of a point on current bitmap rendition
    (e.g. a mouse click)

    The struct provides a ptr. to the clicked object. In addition, if the clicked point
    is on a score, it also provides:
    - A MeasureLocator struct with information about measure, instrument, and time
      position.
    - The staff index, relative to instrument staves.

    Otherwise, if clicked point is not on a score, the MeasureLocator is invalid
    and the staff index is -1.

*/
struct ClickPointData
{
    ImoObj* pImo;           //clicked object or nullptr if out of document
    MeasureLocator ml;      //locator when clicked point is on a score; otherwise invalid locator
    int iStaff;             //staff index, relative to instrument, or -1 when clicked point is not on a score

    ClickPointData() : pImo(nullptr), ml(-1, -1, 0.0), iStaff(-1) {}

};


///@cond INTERNALS

//---------------------------------------------------------------------------------------
// Logical Units comparison (LUnits, Tenths)
inline bool is_equal_pos(float c1, float c2) {
    return fabs(c1-c2) < 0.001f;
}


//---------------------------------------------------------------------------------------
// float comparison
inline bool is_equal_float(float c1, float c2) {
    return fabs(c1-c2) < 0.001f;
}


//---------------------------------------------------------------------------------------
//some common constants
enum {
    k_failure = 0,
    k_success = 1,
};


//---------------------------------------------------------------------------------------
//utility functions
inline URect normalized_rectangle(LUnits xLeft, LUnits yTop,
                                  LUnits xRight, LUnits yBottom)
{
    LUnits xL, xR, yT, yB;

    if (xLeft > xRight)
    {
        xL = xRight;
        xR  = xLeft;
    }
    else
    {
        xL = xLeft;
        xR = xRight;
    }

    if (yTop > yBottom)
    {
        yT = yBottom;
        yB = yTop;
    }
    else
    {
        yT = yTop;
        yB = yBottom;
    }

    return URect(xL, yT, xR-xL, yB-yT);
}

//---------------------------------------------------------------------------------------
// color type: rgba 8bits.
// Remember: alpha 0 means transparent and 255 means opaque
typedef agg::rgba8      Color;
inline bool is_equal(Color c1, Color c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a ;
}

inline bool is_different(Color c1, Color c2) {
    return c1.r != c2.r || c1.g != c2.g || c1.b != c2.b || c1.a != c2.a ;
}

//struct Color
//{
//    Int8u r;
//    Int8u g;
//    Int8u b;
//    Int8u a;
//
//    Color() : r(0), g(0), b(0), a(255) {}
//
//    Color(unsigned r_, unsigned g_, unsigned b_, unsigned a_= 255)
//        : r(Int8u(r_)), g(Int8u(g_)), b(Int8u(b_)), a(Int8u(a_)) {}
//
//    Color(const Color& c, unsigned a_) :
//        r(c.r), g(c.g), b(c.b), a(Int8u(a_)) {}
//
//    bool operator == (Color color) {
//        return r == color.r && g == color.g && b == color.b && a == color.a ;
//    }
//
//    bool operator != (Color color) {
//        return r != color.r || g != color.g || b != color.b || a != color.a ;
//    }
//
//    void clear() { r = g = b = a = 0; }
//
//    const Color& transparent() {
//        a = 0;
//        return *this;
//    }
//
//    void red(unsigned value) { r = value; }
//    void green(unsigned value) { g = value; }
//    void blue(unsigned value) { b = value; }
//    void opacity(unsigned value) { a = value; }
//
//    static Color no_color() { return Color(0,0,0,0); }
//
//    Color gradient(Color c1, double k) {
//        unsigned rx = unsigned( double(r) + (double(r) - double(c1.r)) * k );
//        unsigned gx = unsigned( double(g) + (double(g) - double(c1.g)) * k );
//        unsigned bx = unsigned( double(b) + (double(b) - double(c1.b)) * k );
//        unsigned ax = unsigned( double(a) + (double(a) - double(c1.a)) * k );
//        return Color(rx, gx, bx, ax);
//    }
//
//
//};






///@endcond


}   //namespace lomse

#endif	// __LOMSE_BASIC_H__


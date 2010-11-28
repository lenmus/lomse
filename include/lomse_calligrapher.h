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
//  Credits:
//  -------------------------
//  This file is based on Anti-Grain Geometry version 2.4 examples' code.
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev 
//  (http://www.antigrain.com). AGG 2.4 is distributed under BSD license.
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_CALLIGRAPHER_H__
#define __LOMSE_CALLIGRAPHER_H__

#include "lomse_injectors.h"
#include "lomse_renderer.h"


using namespace agg;

namespace lomse
{

//forward declarations
class Renderer;
class FontStorage;


// Calligrapher: A speciallized drawer that knows how to create bitmaps and 
//               paths to render fonts
//---------------------------------------------------------------------------------------
class Calligrapher
{
protected:
    FontStorage* m_pFonts;
    Renderer* m_pRenderer;

public:
    Calligrapher(FontStorage* fonts, Renderer* renderer);
    ~Calligrapher();

    int draw_text(double x, double y, const std::string& str, Color color,
                  double scale=1.0);
    void draw_glyph(double x, double y, unsigned int ch, Color color, double scale);

protected:
    void draw_glyph(double x, double y, unsigned int ch, Color color);
    void set_scale(double scale);

};


//---------------------------------------------------------------------------------------
// TextMeter: Knows how to measure texts and glyphs
//---------------------------------------------------------------------------------------
class TextMeter
{
protected:
    FontStorage* m_pFonts;
    double m_scale;

public:
    TextMeter(LibraryScope& libraryScope);
    ~TextMeter();

    LUnits measure_width(const std::string& str);
    LUnits get_ascender();
    LUnits get_descender();
    LUnits get_font_height();
    URect bounding_rectangle(unsigned int ch);

    bool select_font(const std::string& fontName, double height,
                     bool fBold=false, bool fItalic=false);
    bool select_raster_font(const std::string& fontName, double height,
                            bool fBold=false, bool fItalic=false);
    bool select_vector_font(const std::string& fontName, double height,
                            bool fBold=false, bool fItalic=false);

protected:
    void set_transform();

};



}   //namespace lomse

#endif      // __LOMSE_CALLIGRAPHER_H__

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

//  This file is based on Anti-Grain Geometry version 2.4 examples' code.
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev
//  (http://www.antigrain.com). AGG 2.4 is distributed as follows:
//    "Permission to copy, use, modify, sell and distribute this software
//    is granted provided this copyright notice appears in all copies.
//    This software is provided "as is" without express or implied
//    warranty, and with no claim as to its suitability for any purpose."
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_CALLIGRAPHER_H__
#define __LOMSE_CALLIGRAPHER_H__

#include "lomse_injectors.h"
#include "lomse_basic.h"


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
    int draw_text(double x, double y, const wstring& str, Color color,
                  double scale=1.0);
    void draw_glyph(double x, double y, unsigned int ch, Color color, double scale);
    void draw_glyph_rotated(double x, double y, unsigned int ch, Color color, double scale, double rotation);

protected:
    void draw_glyph(double x, double y, unsigned int ch, Color color);
    void set_scale(double scale);
    void set_scale_and_rotation(double scale, double rotation);

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
    LUnits measure_width(const wstring& str);
    LUnits get_advance_x(unsigned int ch);
    LUnits get_ascender();
    LUnits get_descender();
    LUnits get_font_height();
    const string& get_font_file();
    URect bounding_rectangle(unsigned int ch);
    void measure_glyphs(wstring* glyphs, std::vector<LUnits>& glyphWidths);

    bool select_font(const std::string& language,
                     const std::string& fontFile,
                     const std::string& fontName, double height,
                     bool fBold=false, bool fItalic=false);
    bool select_raster_font(const std::string& language,
                            const std::string& fontFile,
                            const std::string& fontName, double height,
                            bool fBold=false, bool fItalic=false);
    bool select_vector_font(const std::string& language,
                            const std::string& fontFile,
                            const std::string& fontName, double height,
                            bool fBold=false, bool fItalic=false);

protected:
    void set_transform();

};



}   //namespace lomse

#endif      // __LOMSE_CALLIGRAPHER_H__

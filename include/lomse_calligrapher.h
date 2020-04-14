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
    LUnits measure_width(const wstring& str);
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

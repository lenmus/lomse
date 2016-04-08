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

#include "lomse_calligrapher.h"

#include "lomse_font_storage.h"
#include "lomse_renderer.h"
#include "lomse_logger.h"
#include "utf8.h"
#include <vector>

using namespace agg;

namespace lomse
{

extern LUnits pt_to_LUnits(float pt);

//---------------------------------------------------------------------------------------
// Calligrapher implementation
//---------------------------------------------------------------------------------------
Calligrapher::Calligrapher(FontStorage* fonts, Renderer* renderer)
    : m_pFonts(fonts)
    , m_pRenderer(renderer)
{
}

//---------------------------------------------------------------------------------------
Calligrapher::~Calligrapher()
{
}

//---------------------------------------------------------------------------------------
int Calligrapher::draw_text(double x, double y, const std::string& str, Color color,
                            double scale)
{
    //convert to utf-32
    const char* utf8str = str.c_str();
    wstring utf32result;
    utf8::utf8to32(utf8str, utf8str + strlen(utf8str), std::back_inserter(utf32result));

    return draw_text(x, y, utf32result, color, scale);
}

//---------------------------------------------------------------------------------------
int Calligrapher::draw_text(double x, double y, const wstring& str, Color color,
                            double scale)
{
    //returns the number of chars drawn

    if (!m_pFonts->is_font_valid())
        return 0;

    set_scale(scale);

    //loop to render glyphs
    int num_glyphs = 0;
    wstring::const_iterator it;
    for (it = str.begin(); it != str.end(); ++it)
    {
        const lomse::glyph_cache* glyph = m_pFonts->get_glyph_cache(*it);
        if(glyph)
        {
            m_pFonts->add_kerning(&x, &y);
            m_pFonts->init_adaptors(glyph, x, y);

            //render the glyph using method agg::glyph_ren_agg_gray8
            m_pRenderer->render(m_pFonts->get_gray8_adaptor(),
                                m_pFonts->get_gray8_scanline(),
                                color);

            // increment pen position
            x += glyph->advance_x;
            ++num_glyphs;
        }
    }
    return num_glyphs;
}

//---------------------------------------------------------------------------------------
void Calligrapher::draw_glyph(double x, double y, unsigned int ch, Color color,
                              double scale)
{
    set_scale(scale);
    draw_glyph(x, y, ch, color);
}

//---------------------------------------------------------------------------------------
void Calligrapher::draw_glyph(double x, double y, unsigned int ch, Color color)
{
    //ch is the glyph (utf-32)

   if (!m_pFonts->is_font_valid())
        return;

    const lomse::glyph_cache* glyph = m_pFonts->get_glyph_cache(ch);
    if(glyph)
    {
        m_pFonts->add_kerning(&x, &y);
        m_pFonts->init_adaptors(glyph, x, y);

        //render the glyph using method agg::glyph_ren_agg_gray8
        m_pRenderer->render(m_pFonts->get_gray8_adaptor(),
                            m_pFonts->get_gray8_scanline(),
                            color);
    }
}

//---------------------------------------------------------------------------------------
void Calligrapher::set_scale(double scale)
{
   if (!m_pFonts->is_font_valid())
        return;

    agg::trans_affine mtx;
    mtx *= agg::trans_affine_scaling(scale);
    m_pFonts->set_transform(mtx);
}


//---------------------------------------------------------------------------------------
// TextMeter implementation
//---------------------------------------------------------------------------------------
TextMeter::TextMeter(LibraryScope& libraryScope)
    : m_pFonts( libraryScope.font_storage() )
    , m_scale( libraryScope.get_screen_ppi() / 2540.0 )
{
}

//---------------------------------------------------------------------------------------
TextMeter::~TextMeter()
{
}

//---------------------------------------------------------------------------------------
LUnits TextMeter::measure_width(const std::string& str)
{
    //convert to utf-32
    const char* utf8str = str.c_str();
    wstring utf32result;
    utf8::utf8to32(utf8str, utf8str + strlen(utf8str), std::back_inserter(utf32result));

    return measure_width(utf32result);
}

//---------------------------------------------------------------------------------------
LUnits TextMeter::measure_width(const wstring& str)
{
   if (!m_pFonts->is_font_valid())
        return 0.0f;

    set_transform();

    //loop to measure glyphs
    LUnits width = 0.0f;
    wstring::const_iterator it;
    for (it = str.begin(); it != str.end(); ++it)
    {
        const lomse::glyph_cache* glyph = m_pFonts->get_glyph_cache(*it);
        if(glyph)
            width += static_cast<LUnits>( glyph->advance_x );
    }
    return width;
}

//---------------------------------------------------------------------------------------
void TextMeter::measure_glyphs(wstring* glyphs, std::vector<LUnits>& glyphWidths)
{
    if (!m_pFonts->is_font_valid())
    {
        string msg("[TextMeter::measure_glyphs] Not valid font");
        LOMSE_LOG_ERROR(msg);
        throw std::runtime_error(msg);
    }

    set_transform();

    //loop to measure glyphs
    wstring::iterator it;
    for (it = glyphs->begin(); it != glyphs->end(); ++it)
    {
        const lomse::glyph_cache* glyph = m_pFonts->get_glyph_cache(*it);
        if(glyph)
            glyphWidths.push_back( static_cast<LUnits>( glyph->advance_x ) );
        else
            glyphWidths.push_back( 0.0f );
    }
}

//---------------------------------------------------------------------------------------
LUnits TextMeter::get_ascender()
{
    if (!m_pFonts->is_font_valid())
        return 0.0f;
    else
        return pt_to_LUnits( float(m_pFonts->get_ascender()) );
}

//---------------------------------------------------------------------------------------
LUnits TextMeter::get_descender()
{
    if (!m_pFonts->is_font_valid())
        return 0.0f;
    else
        return pt_to_LUnits( float(m_pFonts->get_descender()) );
}

//---------------------------------------------------------------------------------------
LUnits TextMeter::get_font_height()
{
    if (!m_pFonts->is_font_valid())
        return 0.0f;
    else
        return pt_to_LUnits( float(m_pFonts->get_font_height_in_points()) );
}

//---------------------------------------------------------------------------------------
void TextMeter::set_transform()
{
    agg::trans_affine mtx;
    mtx *= agg::trans_affine_scaling(1.0 / m_scale);
    m_pFonts->set_transform(mtx);
}

//---------------------------------------------------------------------------------------
URect TextMeter::bounding_rectangle(unsigned int ch)
{
    URect rect;
    if (!m_pFonts->is_font_valid())
        return rect;

    //set_transform();
    agg::trans_affine mtx;
    mtx *= agg::trans_affine_scaling(1.0 / m_scale);
    m_pFonts->set_transform(mtx);

    const lomse::glyph_cache* glyph = m_pFonts->get_glyph_cache(ch);
    if(glyph)
    {
        //m_pFonts->init_adaptors(glyph, x, y);
        agg::rect_i bbox = glyph->bounds;

        //bbox is a rectangle with integer values (type agg::rect_i)
        //(x1,y1) is left-top corner and (x2,y2) is right-bottom corner.
        rect.width = Tenths(bbox.x2 - bbox.x1);
        rect.height = Tenths(bbox.y2 - bbox.y1);
        rect.x = Tenths(bbox.x1);
        rect.y = Tenths(bbox.y1);
    }
    return rect;
}

//---------------------------------------------------------------------------------------
bool TextMeter::select_font(const std::string& language,
                            const std::string& fontFile,
                            const std::string& fontName, double height,
                            bool fBold, bool fItalic)
{
    return m_pFonts->select_font(language, fontFile, fontName, height, fBold, fItalic);
}

//---------------------------------------------------------------------------------------
bool TextMeter::select_raster_font(const std::string& language,
                                   const std::string& fontFile,
                                   const std::string& fontName, double height,
                                   bool fBold, bool fItalic)
{
    return m_pFonts->select_raster_font(language, fontFile, fontName,
                                         height, fBold, fItalic);
}

//---------------------------------------------------------------------------------------
bool TextMeter::select_vector_font(const std::string& language,
                                   const std::string& fontFile,
                                   const std::string& fontName, double height,
                                   bool fBold, bool fItalic)
{
    return m_pFonts->select_vector_font(language, fontFile, fontName,
                                         height, fBold, fItalic);
}



}   //namespace lomse

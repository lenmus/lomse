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

#include "lomse_calligrapher.h"

#include "lomse_font_storage.h"
#include "utf8.h"
#include <vector>

using namespace agg;

namespace lomse
{

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
int Calligrapher::draw_text(double x, double y, const std::string& str, double scale)
{
    //returns the number of chars drawn

   if (!m_pFonts->is_font_valid())
        return 0;

    //set scaling factor
    agg::trans_affine mtx;
    mtx *= agg::trans_affine_scaling(scale);
    m_pFonts->set_transform(mtx);

    //convert to utf-32
    const char* utf8str = str.c_str();
    std::vector<unsigned int> utf32result;
    utf8::utf8to32(utf8str, utf8str + strlen(utf8str), std::back_inserter(utf32result));

    //loop to render glyphs
    int num_glyphs = 0;
    std::vector<unsigned int>::iterator it;
    for (it = utf32result.begin(); it != utf32result.end(); ++it)
    {
        const agg::glyph_cache* glyph = m_pFonts->get_glyph_cache(*it);
        if(glyph)
        {
            m_pFonts->add_kerning(&x, &y);
            m_pFonts->init_adaptors(glyph, x, y);

            //render the glyph using method agg::glyph_ren_agg_gray8
            m_pRenderer->render(m_pFonts->get_gray8_adaptor(),
                                m_pFonts->get_gray8_scanline() );

            // increment pen position
            x += glyph->advance_x;
            ++num_glyphs;
        }
    }
    return num_glyphs;
}



//---------------------------------------------------------------------------------------
// TextMeter implementation
//---------------------------------------------------------------------------------------
TextMeter::TextMeter(FontStorage* fonts)
    : m_pFonts(fonts)
{
}

//---------------------------------------------------------------------------------------
TextMeter::~TextMeter()
{
}

//---------------------------------------------------------------------------------------
LUnits TextMeter::measure_width(const std::string& str)
{
   if (!m_pFonts->is_font_valid())
        return 0.0f;

    //set scaling factor
    agg::trans_affine mtx;
    mtx *= agg::trans_affine_scaling(1/0.0375);
    m_pFonts->set_transform(mtx);

    //convert to utf-32
    const char* utf8str = str.c_str();
    std::vector<unsigned int> utf32result;
    utf8::utf8to32(utf8str, utf8str + strlen(utf8str), std::back_inserter(utf32result));

    //loop to measure glyphs
    LUnits width = 0.0f;
    std::vector<unsigned int>::iterator it;
    for (it = utf32result.begin(); it != utf32result.end(); ++it)
    {
        const agg::glyph_cache* glyph = m_pFonts->get_glyph_cache(*it);
        if(glyph)
            width += static_cast<LUnits>( glyph->advance_x );
    }
    return width;
}

//---------------------------------------------------------------------------------------
LUnits TextMeter::get_ascender()
{
    if (!m_pFonts->is_font_valid())
        return 0.0f;
    else
        return LUnits( m_pFonts->get_ascender() / 0.0375 );
}

//---------------------------------------------------------------------------------------
LUnits TextMeter::get_descender()
{
    if (!m_pFonts->is_font_valid())
        return 0.0f;
    else
        return LUnits( m_pFonts->get_descender() / 0.0375 );
}

//---------------------------------------------------------------------------------------
LUnits TextMeter::get_font_height()
{
    if (!m_pFonts->is_font_valid())
        return 0.0f;
    else
        return LUnits( m_pFonts->get_font_height() / 0.0375 );
}



}   //namespace lomse

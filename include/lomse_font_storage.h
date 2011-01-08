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
//  -------------------------
//  Credits:
//  This file is based on Anti-Grain Geometry version 2.4 examples' code.
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev
//  (http://www.antigrain.com). AGG 2.4 is distributed under BSD license.
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_FONT_STORAGE_H__
#define __LOMSE_FONT_STORAGE_H__

#include "lomse_basic.h"
#include "lomse_vertex_source.h"
#include "lomse_agg_types.h"
#include "lomse_renderer.h"
#include <string>
using namespace std;

using namespace agg;

namespace lomse
{

//forward declarations
class Renderer;


//---------------------------------------------------------------------------------------
enum EFontCacheType
{
    k_raster_font_cache,
    k_vector_font_cache
};

typedef lomse::font_cache_manager<FontEngine>::gray8_adaptor_type   Gray8Adaptor;
typedef lomse::font_cache_manager<FontEngine>::gray8_scanline_type  Gary8Scanline;


// FontStorage: Provides fonts and glyphs
//---------------------------------------------------------------------------------------
class FontStorage
{
protected:
    FontEngine          m_fontEngine;
    FontCacheManager    m_fontCacheManager;

    double  m_fontHeight;
    double  m_fontWidth;
    bool    m_fHinting;
    bool    m_fValidFont;       //there is a font loaded
    bool    m_fKerning;
    bool    m_fFlip_y;
    EFontCacheType      m_fontCacheType;

public:
    FontStorage();
    ~FontStorage();

    inline bool is_font_valid() { return m_fValidFont; }

    inline double get_font_height() { return m_fontHeight; }
    inline double get_ascender() { return m_fontEngine.ascender(); }
    inline double get_descender() { return m_fontEngine.descender(); }

    void set_font_size(double rPoints);
    void set_font_height(double rPoints);
    void set_font_width(double rPoints);

    bool select_font(const std::string& fontName, double height,
                     bool fBold=false, bool fItalic=false);
    bool select_raster_font(const std::string& fontName, double height,
                            bool fBold=false, bool fItalic=false);
    bool select_vector_font(const std::string& fontName, double height,
                            bool fBold=false, bool fItalic=false);

    //transitional. For Calligrapher
    inline const lomse::glyph_cache* get_glyph_cache(unsigned int nChar) {
        return m_fontCacheManager.glyph(nChar);
    }
    inline void add_kerning(double* x, double* y) {
        if(m_fKerning)
            m_fontCacheManager.add_kerning(x, y);
    }
    inline void init_adaptors(const lomse::glyph_cache* glyph, double x, double y) {
        m_fontCacheManager.init_embedded_adaptors(glyph, x, y);
    }
    inline Gray8Adaptor& get_gray8_adaptor() {
        return m_fontCacheManager.gray8_adaptor();
    }
    inline Gary8Scanline& get_gray8_scanline() {
        return m_fontCacheManager.gray8_scanline();
    }
    inline void set_transform(agg::trans_affine& mtx) {
        m_fontEngine.transform(mtx);
    }

protected:
    bool set_font(const std::string& fontFullName, double height,
                  EFontCacheType type = k_raster_font_cache);

};

//---------------------------------------------------------------------------------------
// FontSelector
class FontSelector
{
public:
    FontSelector() {}
    ~FontSelector() {}

    std::string find_font(const std::string& name, bool fBold=false, bool fItalic=false);
};



}   //namespace lomse

#endif      // __LOMSE_FONT_STORAGE_H__

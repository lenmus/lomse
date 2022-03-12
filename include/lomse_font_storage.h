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

#ifndef __LOMSE_FONT_STORAGE_H__
#define __LOMSE_FONT_STORAGE_H__

//lomse
#include "lomse_basic.h"
#include "lomse_vertex_source.h"
#include "lomse_agg_types.h"
#include "lomse_injectors.h"

//std
#include <string>
#include <map>
using namespace std;

using namespace agg;

namespace lomse
{


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
    LibraryScope*       m_pLibScope;

    double  m_fontHeight;
    double  m_fontWidth;
    bool    m_fHinting;
    bool    m_fValidFont;       //there is a font loaded
    bool    m_fKerning;
    bool    m_fFlip_y;
    EFontCacheType      m_fontCacheType;
    string m_fontFullName;

public:
    FontStorage(LibraryScope* pLibScope);
    ~FontStorage();

    inline bool is_font_valid() { return m_fValidFont; }

    inline double get_font_height_in_points() { return m_fontHeight; }
    inline double get_ascender() { return m_fontEngine.ascender(); }
    inline double get_descender() { return m_fontEngine.descender(); }
    inline const string& get_font_file() { return m_fontFullName; }

    void set_font_size(double rPoints);
    void set_font_height(double rPoints);
    void set_font_width(double rPoints);

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
/** FontSelector
*/
class FontSelector
{
protected:
    LibraryScope* m_pLibScope;
    std::map<string, string> m_cache;

public:
    FontSelector(LibraryScope* pLibScope) : m_pLibScope(pLibScope) {}
    ~FontSelector() {}

    std::string find_font(const std::string& language,
                          const std::string& fontFile,
                          const std::string& name,
                          bool fBold=false, bool fItalic=false);

};



}   //namespace lomse

#endif      // __LOMSE_FONT_STORAGE_H__

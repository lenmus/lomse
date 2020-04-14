//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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

#include "lomse_font_storage.h"

#include "lomse_build_options.h"
#include "lomse_logger.h"

#include <locale>   //to upper conversion
using namespace agg;


namespace lomse
{

//=======================================================================================
// FontStorage implementation
//=======================================================================================
FontStorage::FontStorage(LibraryScope* pLibScope)
    : m_fontEngine(1000)        //1000 = number of faces in cache
    , m_fontCacheManager(m_fontEngine)
    , m_pLibScope(pLibScope)
    , m_fontHeight(14.0)
    , m_fontWidth(14.0)
    , m_fHinting(false)
    , m_fValidFont(false)
    , m_fKerning(true)
    , m_fFlip_y(true)
    , m_fontCacheType(k_raster_font_cache)
{
    //AWARE:
    //Apple Computer, Inc., owns three patents that are related to the
    //hinting process of glyph outlines within TrueType fonts. Hinting (also named
    //grid-fitting) is used to enhance the quality of glyphs at small bitmap sizes.
    //Therefore, you can not use hinting unless you are authorized (you purchased
    //a license from Apple, or because you are in a country where the patents do
    //not apply, etc.). Nevertheless lenmus font doesn't include hinting information
    //and, so, previous flag value doesn't matter. But its value is important
    //if I finally use FreeType for all fonts.

    //font settings
    unsigned ppi = unsigned(pLibScope->get_screen_ppi());
    m_fontEngine.resolution(ppi);
    m_fontEngine.gamma(agg::gamma_none());
    m_fontEngine.hinting(m_fHinting);
    m_fontEngine.flip_y(m_fFlip_y);

    //load music font
    string fullname = m_pLibScope->get_music_font_path();
    fullname += m_pLibScope->get_music_font_file();
    set_font(fullname, 24.0);
}

//---------------------------------------------------------------------------------------
FontStorage::~FontStorage()
{
}

//---------------------------------------------------------------------------------------
bool FontStorage::set_font(const std::string& fontFullName, double height,
                           EFontCacheType type)
{
    m_fValidFont = false;
    lomse::glyph_rendering gren = lomse::glyph_ren_agg_gray8;
    if(! m_fontEngine.select_font(fontFullName, 0, gren))
        return !m_fValidFont;    //error

    //set curren values for renderization
    m_fontCacheType = type;
    set_font_size(height);

    ////un-comment this to rotate/skew/translate the text
    ////agg::trans_affine mtx;
    ////mtx *= agg::trans_affine_rotation(agg::deg2rad(-4.0));
    //////mtx *= agg::trans_affine_skewing(-0.4, 0);
    //////mtx *= agg::trans_affine_translation(1, 0);
    ////m_fontEngine.transform(mtx);

    m_fValidFont = true;
    m_fontFullName = fontFullName;
    return !m_fValidFont;
}

//---------------------------------------------------------------------------------------
void FontStorage::set_font_size(double rPoints)
{
    m_fontHeight = rPoints;
    m_fontWidth = rPoints;
    m_fontEngine.height(m_fontHeight);
    m_fontEngine.width(m_fontWidth);
}

//---------------------------------------------------------------------------------------
void FontStorage::set_font_height(double rPoints)
{
    m_fontHeight = rPoints;
    m_fontEngine.height(rPoints);
}

//---------------------------------------------------------------------------------------
void FontStorage::set_font_width(double rPoints)
{
    m_fontWidth = rPoints;
    m_fontEngine.width(rPoints);
}

//---------------------------------------------------------------------------------------
bool FontStorage::select_font(const std::string& language,
                               const std::string& fontFile,
                               const std::string& fontName, double height,
                               bool fBold, bool fItalic)
{
    return select_raster_font(language, fontFile, fontName, height, fBold, fItalic);
}

//---------------------------------------------------------------------------------------
bool FontStorage::select_raster_font(const std::string& language,
                                      const std::string& fontFile,
                                      const std::string& fontName, double height,
                                      bool fBold, bool fItalic)
{
    //Returns true if any error
    FontSelector* fs = m_pLibScope->get_font_selector();
    string fullFile = fs->find_font(language, fontFile, fontName, fBold, fItalic);
    return set_font(fullFile, height, k_raster_font_cache);
}

//---------------------------------------------------------------------------------------
bool FontStorage::select_vector_font(const std::string& language,
                                      const std::string& fontFile,
                                      const std::string& fontName, double height,
                                      bool fBold, bool fItalic)
{
    //Returns true if any error
    FontSelector* fs = m_pLibScope->get_font_selector();
    string fullFile = fs->find_font(language, fontFile, fontName, fBold, fItalic);
    return set_font(fullFile, height, k_vector_font_cache);
}


}   //namespace lomse

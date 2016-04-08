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

//  This code is based on file agg_font_freetype.h from
//  Anti-Grain Geometry version 2.4
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev
//  (http://www.antigrain.com). AGG 2.4 is distributed as follows:
//    "Permission to copy, use, modify, sell and distribute this software
//    is granted provided this copyright notice appears in all copies.
//    This software is provided "as is" without express or implied
//    warranty, and with no claim as to its suitability for any purpose."
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_FONT_FREETYPE_H__
#define __LOMSE_FONT_FREETYPE_H__

#include <ft2build.h>
#include FT_FREETYPE_H

#include "agg_scanline_storage_aa.h"
#include "agg_scanline_storage_bin.h"
#include "agg_scanline_u.h"
#include "agg_scanline_bin.h"
#include "agg_path_storage_integer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_conv_curve.h"
#include "lomse_font_cache_manager.h"
#include "agg_trans_affine.h"
#include <string>
using namespace std;

using namespace agg;

namespace lomse
{


//---------------------------------------------------------------------------------------
// font_engine_freetype_base
class font_engine_freetype_base
{
public:
    typedef serialized_scanlines_adaptor_aa<int8u>    gray8_adaptor_type;
    typedef serialized_scanlines_adaptor_bin          mono_adaptor_type;
    typedef scanline_storage_aa8                      scanlines_aa_type;
    typedef scanline_storage_bin                      scanlines_bin_type;

    ~font_engine_freetype_base();
    font_engine_freetype_base(bool flag32, unsigned max_faces = 32);

    // Set font parameters
    void resolution(unsigned dpi);
    bool select_font(const std::string& font_name, unsigned face_index,
                     glyph_rendering ren_type, const char* font_mem = 0,
                     const long font_mem_size = 0);
    bool attach(const char* file_name);
    bool char_map(FT_Encoding map);
    bool height(double h);
    bool width(double w);
    void hinting(bool h);
    void flip_y(bool f);
    void transform(const trans_affine& affine);

    // Set Gamma
    template<class GammaF> void gamma(const GammaF& f)
    {
        m_rasterizer.gamma(f);
    }

    // Accessors
    int         last_error()   const { return m_last_error; }
    unsigned    resolution()   const { return m_resolution; }
    const char* name()         const { return m_name;       }
    unsigned    num_faces()    const;
    FT_Encoding char_map()     const { return m_char_map;   }
    double      height()       const { return double(m_height) / 64.0;    }
    double      width()        const { return double(m_width) / 64.0;     }
    double      ascender()     const;
    double      descender()    const;
    bool        hinting()      const { return m_hinting;    }
    bool        flip_y()       const { return m_flip_y;     }


    // Interface mandatory to implement for font_cache_manager
    const char*     font_signature() const { return m_signature;    }
    int             change_stamp()   const { return m_change_stamp; }
    bool            prepare_glyph(unsigned glyph_code);
    unsigned        glyph_index() const { return m_glyph_index; }
    unsigned        data_size()   const { return m_data_size;   }
    glyph_data_type data_type()   const { return m_data_type;   }
    const rect_i&   bounds()      const { return m_bounds;      }
    double          advance_x()   const { return m_advance_x;   }
    double          advance_y()   const { return m_advance_y;   }
    void            write_glyph_to(int8u* data) const;
    bool            add_kerning(unsigned first, unsigned second,
                                double* x, double* y);

private:
    font_engine_freetype_base(const font_engine_freetype_base&);
    const font_engine_freetype_base& operator = (const font_engine_freetype_base&);

    void update_char_size();
    void update_signature();
    int  find_face(const char* face_name) const;
    void determine_rendering_mode(glyph_rendering ren_type);
    void load_font_file(const std::string& font_name, unsigned face_index,
                        const char* font_mem, const long font_mem_size);

    bool            m_flag32;
    int             m_change_stamp;
    int             m_last_error;
    char*           m_name;
    unsigned        m_name_len;
    unsigned        m_face_index;
    FT_Encoding     m_char_map;
    char*           m_signature;
    unsigned        m_height;
    unsigned        m_width;
    bool            m_hinting;
    bool            m_flip_y;
    bool            m_library_initialized;
    FT_Library      m_library;    // handle to library
    FT_Face*        m_faces;      // A pool of font faces
    char**          m_face_names;
    unsigned        m_num_faces;
    unsigned        m_max_faces;
    FT_Face         m_cur_face;  // handle to the current face object
    int             m_resolution;
    glyph_rendering m_glyph_rendering;
    unsigned        m_glyph_index;
    unsigned        m_data_size;
    glyph_data_type m_data_type;
    rect_i          m_bounds;
    double          m_advance_x;
    double          m_advance_y;
    trans_affine    m_affine;

    path_storage_integer<int16, 6>              m_path16;
    path_storage_integer<int32, 6>              m_path32;
    conv_curve<path_storage_integer<int16, 6> > m_curves16;
    conv_curve<path_storage_integer<int32, 6> > m_curves32;
    scanline_u8              m_scanline_aa;
    scanline_bin             m_scanline_bin;
    scanlines_aa_type        m_scanlines_aa;
    scanlines_bin_type       m_scanlines_bin;
    rasterizer_scanline_aa<> m_rasterizer;
};



//---------------------------------------------------------------------------------------
// font_engine_freetype_int32
// This class uses values of type int32 (26.6 format) for the vector cache.
// It allows you to render glyphs of very large sizes.
//
class font_engine_freetype_int32 : public font_engine_freetype_base
{
public:
    typedef serialized_integer_path_adaptor<int32, 6>     path_adaptor_type;
    typedef font_engine_freetype_base::gray8_adaptor_type gray8_adaptor_type;
    typedef font_engine_freetype_base::mono_adaptor_type  mono_adaptor_type;
    typedef font_engine_freetype_base::scanlines_aa_type  scanlines_aa_type;
    typedef font_engine_freetype_base::scanlines_bin_type scanlines_bin_type;

    font_engine_freetype_int32(unsigned max_faces = 32) :
        font_engine_freetype_base(true, max_faces) {}
};


}

#endif      //__LOMSE_FONT_FREETYPE_H__

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

//  This code is based on file agg_font_cache_manager.h from
//  Anti-Grain Geometry version 2.4
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev
//  (http://www.antigrain.com). AGG 2.4 is distributed as follows:
//    "Permission to copy, use, modify, sell and distribute this software
//    is granted provided this copyright notice appears in all copies.
//    This software is provided "as is" without express or implied
//    warranty, and with no claim as to its suitability for any purpose."
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_FONT_CACHE_MANAGER_H__
#define __LOMSE_FONT_CACHE_MANAGER_H__

#include "lomse_config.h"

#include <string.h>
#include "agg_array.h"


using namespace agg;

//---------------------------------------------------------------------------------------
//AWARE: Microsoft deprecated strncpy() but the "security enhanced" new function
//strncpy_s() is only defined by Microsoft which "coincidentally" makes your code
//non-portable.
//
//The addressed security issue is, basically, that strncpy does not ensure that the
//string is properly ended with '\0'. But this is not needed in this source code.
//
//This code addresses the need to suppress the annoying Microsoft warnings, which
//is not easy. Thanks Microsoft!
//
//See:
//  https://stackoverflow.com/questions/858252/alternatives-to-ms-strncpy-s
//
#if (defined(_MSC_VER) && (_MSC_VER >= 1400) )
    #include <string>
    using namespace std;

    #define strncpy(dest, source, count)     strcpy_s((dest), (count), (source))
#else
    #define strncpy_s(dest, source, count)   strncpy((dest), (source), (count))
#endif
//---------------------------------------------------------------------------------------


namespace lomse
{

//---------------------------------------------------------------------------------------
// glyph_cache: data for a glyph

enum glyph_data_type
{
    glyph_data_invalid = 0,
    glyph_data_mono    = 1,
    glyph_data_gray8   = 2,
    glyph_data_outline = 3
};

struct glyph_cache
{
    unsigned        glyph_index;
    int8u*          data;
    unsigned        data_size;
    glyph_data_type data_type;
    rect_i          bounds;
    double          advance_x;
    double          advance_y;
};


//---------------------------------------------------------------------------------------
// font_cache: storage for the latest 256 used glyphs

    class font_cache
    {
    private:
        block_allocator m_allocator;
        glyph_cache**   m_glyphs[256];
        char*           m_font_signature;

    public:
        enum block_size_e { block_size = 16384-16 };

        //--------------------------------------------------------------------
        font_cache()
            : m_allocator(block_size)
            , m_font_signature(0)
        {
        }

        //--------------------------------------------------------------------
        void signature(const char* font_signature)
        {
            int len = strlen(font_signature) + 1;
            m_font_signature = (char*)m_allocator.allocate(len);
            strncpy(m_font_signature, font_signature, len);
            memset(m_glyphs, 0, sizeof(m_glyphs));
        }

        //--------------------------------------------------------------------
        bool font_is(const char* font_signature) const
        {
            return strcmp(font_signature, m_font_signature) == 0;
        }

        //--------------------------------------------------------------------
        const glyph_cache* find_glyph(unsigned glyph_code) const
        {
            unsigned msb = (glyph_code >> 8) & 0xFF;
            if(m_glyphs[msb])
            {
                return m_glyphs[msb][glyph_code & 0xFF];
            }
            return 0;
        }

        //--------------------------------------------------------------------
        glyph_cache* cache_glyph(unsigned        glyph_code,
                                 unsigned        glyph_index,
                                 unsigned        data_size,
                                 glyph_data_type data_type,
                                 const rect_i&   bounds,
                                 double          advance_x,
                                 double          advance_y)
        {
            unsigned msb = (glyph_code >> 8) & 0xFF;
            if(m_glyphs[msb] == 0)
            {
                m_glyphs[msb] =
                    (glyph_cache**)m_allocator.allocate(sizeof(glyph_cache*) * 256,
                                                        sizeof(glyph_cache*));
                memset(m_glyphs[msb], 0, sizeof(glyph_cache*) * 256);
            }

            unsigned lsb = glyph_code & 0xFF;
            if(m_glyphs[msb][lsb]) return 0; // Already exists, do not overwrite

            glyph_cache* glyph =
                (glyph_cache*)m_allocator.allocate(sizeof(glyph_cache),
                                                   sizeof(double));

            glyph->glyph_index        = glyph_index;
            glyph->data               = m_allocator.allocate(data_size);
            glyph->data_size          = data_size;
            glyph->data_type          = data_type;
            glyph->bounds             = bounds;
            glyph->advance_x          = advance_x;
            glyph->advance_y          = advance_y;
            return m_glyphs[msb][lsb] = glyph;
        }
    };






//---------------------------------------------------------------------------------------
// font_cache_pool: fonts?

class font_cache_pool
{
private:
    font_cache** m_fonts;
    unsigned     m_max_fonts;
    unsigned     m_num_fonts;
    font_cache*  m_cur_font;

public:
    //--------------------------------------------------------------------
    ~font_cache_pool()
    {
        unsigned i;
        for(i = 0; i < m_num_fonts; ++i)
        {
            obj_allocator<font_cache>::deallocate(m_fonts[i]);
        }
        pod_allocator<font_cache*>::deallocate(m_fonts, m_max_fonts);
    }

    //--------------------------------------------------------------------
    font_cache_pool(unsigned max_fonts=32) :
        m_fonts(pod_allocator<font_cache*>::allocate(max_fonts)),
        m_max_fonts(max_fonts),
        m_num_fonts(0),
        m_cur_font(0)
    {}


    //--------------------------------------------------------------------
    void font(const char* font_signature, bool reset_cache = false)
    {
        int idx = find_font(font_signature);
        if(idx >= 0)
        {
            if(reset_cache)
            {
                obj_allocator<font_cache>::deallocate(m_fonts[idx]);
                m_fonts[idx] = obj_allocator<font_cache>::allocate();
                m_fonts[idx]->signature(font_signature);
            }
            m_cur_font = m_fonts[idx];
        }
        else
        {
            if(m_num_fonts >= m_max_fonts)
            {
                obj_allocator<font_cache>::deallocate(m_fonts[0]);
                memmove(m_fonts,
                        m_fonts + 1,
                        (m_max_fonts - 1) * sizeof(font_cache*));
                m_num_fonts = m_max_fonts - 1;
            }
            m_fonts[m_num_fonts] = obj_allocator<font_cache>::allocate();
            m_fonts[m_num_fonts]->signature(font_signature);
            m_cur_font = m_fonts[m_num_fonts];
            ++m_num_fonts;
        }
    }

    //--------------------------------------------------------------------
    const font_cache* font() const
    {
        return m_cur_font;
    }

    //--------------------------------------------------------------------
    const glyph_cache* find_glyph(unsigned glyph_code) const
    {
        if(m_cur_font) return m_cur_font->find_glyph(glyph_code);
        return 0;
    }

    //--------------------------------------------------------------------
    glyph_cache* cache_glyph(unsigned        glyph_code,
                                unsigned        glyph_index,
                                unsigned        data_size,
                                glyph_data_type data_type,
                                const rect_i&   bounds,
                                double          advance_x,
                                double          advance_y)
    {
        if(m_cur_font)
        {
            return m_cur_font->cache_glyph(glyph_code,
                                            glyph_index,
                                            data_size,
                                            data_type,
                                            bounds,
                                            advance_x,
                                            advance_y);
        }
        return 0;
    }


    //--------------------------------------------------------------------
    int find_font(const char* font_signature)
    {
        unsigned i;
        for(i = 0; i < m_num_fonts; i++)
        {
            if(m_fonts[i]->font_is(font_signature)) return int(i);
        }
        return -1;
    }

};




    //------------------------------------------------------------------------
    enum glyph_rendering
    {
        glyph_ren_native_mono,
        glyph_ren_native_gray8,
        glyph_ren_outline,
        glyph_ren_agg_mono,
        glyph_ren_agg_gray8
    };



//---------------------------------------------------------------------------------------
// font_cache_manager: orchestrates font usage? (just one font: selected one)

template<class FontEngine> class font_cache_manager
{
public:
    typedef FontEngine font_engine_type;
    typedef font_cache_manager<FontEngine> self_type;
    typedef typename font_engine_type::path_adaptor_type   path_adaptor_type;
    typedef typename font_engine_type::gray8_adaptor_type  gray8_adaptor_type;
    typedef typename gray8_adaptor_type::embedded_scanline gray8_scanline_type;
    typedef typename font_engine_type::mono_adaptor_type   mono_adaptor_type;
    typedef typename mono_adaptor_type::embedded_scanline  mono_scanline_type;

private:
    font_cache_pool     m_fonts;
    font_engine_type&   m_engine;
    int                 m_change_stamp;
    double              m_dx;
    double              m_dy;
    const glyph_cache*  m_prev_glyph;
    const glyph_cache*  m_last_glyph;
    path_adaptor_type   m_path_adaptor;
    gray8_adaptor_type  m_gray8_adaptor;
    gray8_scanline_type m_gray8_scanline;
    mono_adaptor_type   m_mono_adaptor;
    mono_scanline_type  m_mono_scanline;

public:
    //--------------------------------------------------------------------
    font_cache_manager(font_engine_type& engine, unsigned max_fonts=32) :
        m_fonts(max_fonts),
        m_engine(engine),
        m_change_stamp(-1),
        m_dx(0.0),
        m_dy(0.0),
        m_prev_glyph(0),
        m_last_glyph(0)
    {}

    //--------------------------------------------------------------------
    void reset_last_glyph()
    {
        m_prev_glyph = m_last_glyph = 0;
    }

    //--------------------------------------------------------------------
    const glyph_cache* glyph(unsigned glyph_code)
    {
        synchronize();
        const glyph_cache* gl = m_fonts.find_glyph(glyph_code);
        if(gl)
        {
            m_prev_glyph = m_last_glyph;
            return m_last_glyph = gl;
        }
        else
        {
            if(m_engine.prepare_glyph(glyph_code))
            {
                m_prev_glyph = m_last_glyph;
                m_last_glyph = m_fonts.cache_glyph(glyph_code,
                                                    m_engine.glyph_index(),
                                                    m_engine.data_size(),
                                                    m_engine.data_type(),
                                                    m_engine.bounds(),
                                                    m_engine.advance_x(),
                                                    m_engine.advance_y());
                m_engine.write_glyph_to(m_last_glyph->data);
                return m_last_glyph;
            }
        }
        return 0;
    }

    //--------------------------------------------------------------------
    void init_embedded_adaptors(const glyph_cache* gl,
                                double x, double y,
                                double scale=1.0)
    {
        if(gl)
        {
            switch(gl->data_type)
            {
            default: return;
            case glyph_data_mono:
                m_mono_adaptor.init(gl->data, gl->data_size, x, y);
                break;

            case glyph_data_gray8:
                m_gray8_adaptor.init(gl->data, gl->data_size, x, y);
                break;

            case glyph_data_outline:
                m_path_adaptor.init(gl->data, gl->data_size, x, y, scale);
                break;
            }
        }
    }


    //--------------------------------------------------------------------
    path_adaptor_type&   path_adaptor()   { return m_path_adaptor;   }
    gray8_adaptor_type&  gray8_adaptor()  { return m_gray8_adaptor;  }
    gray8_scanline_type& gray8_scanline() { return m_gray8_scanline; }
    mono_adaptor_type&   mono_adaptor()   { return m_mono_adaptor;   }
    mono_scanline_type&  mono_scanline()  { return m_mono_scanline;  }

    //--------------------------------------------------------------------
    const glyph_cache* perv_glyph() const { return m_prev_glyph; }
    const glyph_cache* last_glyph() const { return m_last_glyph; }

    //--------------------------------------------------------------------
    bool add_kerning(double* x, double* y)
    {
        if(m_prev_glyph && m_last_glyph)
        {
            return m_engine.add_kerning(m_prev_glyph->glyph_index,
                                        m_last_glyph->glyph_index,
                                        x, y);
        }
        return false;
    }

    //--------------------------------------------------------------------
    void precache(unsigned from, unsigned to)
    {
        for(; from <= to; ++from) glyph(from);
    }

    //--------------------------------------------------------------------
    void reset_cache()
    {
        m_fonts.font(m_engine.font_signature(), true);
        m_change_stamp = m_engine.change_stamp();
        m_prev_glyph = m_last_glyph = 0;
    }

private:
    //--------------------------------------------------------------------
    font_cache_manager(const self_type&);
    const self_type& operator = (const self_type&);

    //--------------------------------------------------------------------
    void synchronize()
    {
        if(m_change_stamp != m_engine.change_stamp())
        {
            m_fonts.font(m_engine.font_signature());
            m_change_stamp = m_engine.change_stamp();
            m_prev_glyph = m_last_glyph = 0;
        }
    }

};


}

#endif  //__LOMSE_FONT_CACHE_MANAGER_H__

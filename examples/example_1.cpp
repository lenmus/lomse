//--------------------------------------------------------------------------------------
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
//  -------------------------
//  Credits:
//  This file is based on Anti-Grain Geometry version 2.4 examples' code.
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev
//  (http://www.antigrain.com). AGG 2.4 is distributed under BSD license.
//
//-------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgba.h"
#include "platform/agg_platform_support.h"
#include "ctrl/agg_slider_ctrl.h"

#include "lomse_exceptions.h"
#include "lomse_drawer.h"
#include "lomse_agg_drawer.h"
#include "lomse_gm_basic.h"

#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_graphic_view.h"
#include "platform/lomse_platform.h"
#include "lomse_screen_drawer.h"

#include <sstream>
using namespace std;

//#include "agg_gamma_lut.h"

enum { flip_y = false };

using namespace lomse;

class the_application;

//---------------------------------------------------------------------------------------
class MyPlatformSupport : public PlatformSupport
{
protected:
    bool m_fUpdateWindowInvoked;
    bool m_fSetWindowTitleInvoked;
    std::string m_title;
    the_application& m_app;
    RenderingBuffer& m_buffer;

public:
    MyPlatformSupport(bool flip_y, the_application& app);
    virtual ~MyPlatformSupport() {}

    void update_window() { m_fUpdateWindowInvoked = true; }
    void set_window_title(const std::string& title) {
        m_fSetWindowTitleInvoked = true;
        m_title = title;
    }
    void force_redraw();

    RenderingBuffer& get_window_buffer() { return m_buffer; }

    bool set_window_title_invoked() { return m_fSetWindowTitleInvoked; }
    bool update_window_invoked() { return m_fUpdateWindowInvoked; }
    const std::string& get_title() { return m_title; }

    void start_timer();
    double elapsed_time() const;

};

//---------------------------------------------------------------------------------------
class the_application : public agg::platform_support
{
    agg::slider_ctrl<agg::rgba8> m_expand;
    agg::slider_ctrl<agg::rgba8> m_gamma;
    agg::slider_ctrl<agg::rgba8> m_scale;
    agg::slider_ctrl<agg::rgba8> m_rotate;

    MyPlatformSupport* m_pPlatform;
    GraphicView* m_pView;
    LibraryScope& m_libraryScope;
    ScreenDrawer m_drawer;

public:

    the_application(agg::pix_format_e format, bool flip_y, LibraryScope& libraryScope)
        : agg::platform_support(format, flip_y)
        , m_expand(5,     5,    256-5, 11,    !flip_y)
        , m_gamma (5,     5+15, 256-5, 11+15, !flip_y)
        , m_scale (256+5, 5,    512-5, 11,    !flip_y)
        , m_rotate(256+5, 5+15, 512-5, 11+15, !flip_y)
        , m_pPlatform(NULL)
        , m_pView(NULL)
        , m_libraryScope(libraryScope)
        , m_drawer(libraryScope)
    {

        //add_ctrl(m_expand);
        //add_ctrl(m_gamma);
        //add_ctrl(m_scale);
        //add_ctrl(m_rotate);

        //m_expand.label("Expand=%3.2f");
        //m_expand.range(-1, 1.2);
        //m_expand.value(0.0);

        //m_gamma.label("Gamma=%3.2f");
        //m_gamma.range(0.0, 3.0);
        //m_gamma.value(1.0);

        //m_scale.label("Scale=%3.2f");
        //m_scale.range(0.2, 10.0);
        //m_scale.value(1.0);

        //m_rotate.label("Rotate=%3.2f");
        //m_rotate.range(-180.0, 180.0);
        //m_rotate.value(0.0);
    }

    ~the_application()
    {
        if (m_pView)
            delete m_pView;
        if (m_pPlatform)
            delete m_pPlatform;
    }

    void open_document(Document* pDoc, const char* fname)
    {
        m_pPlatform = new MyPlatformSupport(false, *this);
        pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            //"(instrument (musicData (clef G)(clef F3)(clef C1)(clef F4)) )))" );

            //"(instrument (name \"Violin\")(musicData (clef G)(clef F4)(clef C1)) )))" );

            //"(instrument (musicData )) )))" );

            //"(instrument (staves 2) (musicData )) )))" );
            //"(instrument (musicData )) (instrument (musicData )) )))" );

            "(instrument (name \"Violin\")(abbrev \"Vln.\")(musicData (clef G))) "
            "(instrument (name \"pilano\")(abbrev \"P\")(staves 2)(musicData (clef G p1)(clef F4 p2)) )))" );

        //m_pView = new SimpleView(m_libraryScope, pDoc, m_pPlatform, &m_drawer);
        //m_pView = new VerticalBookView(m_libraryScope, pDoc, m_pPlatform, &m_drawer);
        m_pView = new HorizontalBookView(m_libraryScope, pDoc, m_pPlatform, &m_drawer);
    }

    virtual void on_resize(int cx, int cy)
    {
    }

    virtual void on_draw()
    {
        if (!m_pView) return;
        m_pView->on_paint();
    }

    virtual void on_mouse_button_down(int x, int y, unsigned flags)
    {
        if (!m_pView) return;
        m_pView->on_mouse_button_down(x, y, flags);
    }

    virtual void on_mouse_move(int x, int y, unsigned flags)
    {
        if (!m_pView) return;
        m_pView->on_mouse_move(x, y, flags);
    }

    virtual void on_mouse_button_up(int x, int y, unsigned flags)
    {
        if (!m_pView) return;
        m_pView->on_mouse_button_up(x, y, flags);
    }

    virtual void on_key(int x, int y, unsigned key, unsigned flags)
    {
        if (!m_pView) return;
        //m_pView->on_key(x, y, key, flags);

        switch (key)
        {
            case '1':
                m_pView->set_option_draw_box_doc_page_content(true);
                break;
            case '2':
                m_pView->set_option_draw_box_score_page(true);
                break;
            case '3':
                m_pView->set_option_draw_box_system(true);
                break;
            case '4':
                m_pView->set_option_draw_box_slice(true);
                break;
            case '5':
                m_pView->set_option_draw_box_slice_instr(true);
                break;
            case '0':
                m_pView->set_option_draw_box_doc_page_content(false);
                m_pView->set_option_draw_box_score_page(false);
                m_pView->set_option_draw_box_system(false);
                m_pView->set_option_draw_box_slice(false);
                m_pView->set_option_draw_box_slice_instr(false);
                break;
            case '+':
                m_pView->zoom_in(x, y);
                break;
            case '-':
                m_pView->zoom_out(x, y);
                break;
            case ' ':
            {
                //agg::trans_affine mtx;
                //mtx *= agg::trans_affine_translation((m_min_x + m_max_x) * -0.5, (m_min_y + m_max_y) * -0.5);
                //mtx *= agg::trans_affine_scaling(m_scale.value());
                //mtx *= agg::trans_affine_rotation(agg::deg2rad(m_rotate.value()));
                //mtx *= agg::trans_affine_translation((m_min_x + m_max_x) * 0.5, (m_min_y + m_max_y) * 0.5);
                //mtx *= agg::trans_affine_translation(m_x, m_y);

                //double m[6];
                //mtx.store_to(m);

                //char buf[128];
                //sprintf(buf, "%3.3f, %3.3f, %3.3f, %3.3f, %3.3f, %3.3f",
                //            m[0], m[1], m[2], m[3], m[4], m[5]);

                char buf[128];
                sprintf(buf, "scale = %3.3f", m_pView->get_scale() );

                message(buf);
                //FILE* fd = fopen(full_file_name("transform.txt"), "a");
                //fprintf(fd, "%s\n", buf);
                //fclose(fd);
                break;
            }
            default:
                ;
        }

        m_pPlatform->force_redraw();
    }


};




int agg_main(int argc, char* argv[])
{
    LibraryScope m_libraryScope(cout);
    Document m_doc(m_libraryScope);

    the_application app(agg::pix_format_bgra32, flip_y, m_libraryScope);
    //reverse this: ----------------------------------------------------
    //  lomse->init_library(agg::pix_format_bgra32, flip_y, m_libraryScope);
    //provide call backs
    //  PlatformInterface interface;
    //  interface.set_on_draw_callbak();
    //  interface.set_force_redraw_callbak();
    //  interface.set_start_timer_callbak();
    //  interface.set_end_timer_callbak();
    //  ....
    //  lomse->set_callbacks(&interface);




    const char* fname = "tiger.svg";
    //if(argc <= 1)
    //{
    //    FILE* fd = fopen(app.full_file_name(fname), "r");
    //    if(fd == 0)
    //    {
    //        app.message("Usage: demo_1 <ldp_file>");
    //        return 1;
    //    }
    //    fclose(fd);
    //}
    //else
    {
        fname = argv[1];
    }

    try
    {
        app.open_document(&m_doc, app.full_file_name(fname));
    }
    catch(lomse::exception& e)
    {
        app.message(e.msg());
        return 1;
    }

    if(app.init(900, 630, agg::window_resize))
    {
        return app.run();
    }

    return 1;
}


MyPlatformSupport::MyPlatformSupport(bool flip_y, the_application& app)
    : PlatformSupport(flip_y)
    , m_app(app)
    , m_buffer(app.rbuf_window())
{
}

void MyPlatformSupport::force_redraw()
{
    m_app.force_redraw();
}

void MyPlatformSupport::start_timer()
{
    m_app.start_timer();
}

double MyPlatformSupport::elapsed_time() const
{
    return m_app.elapsed_time();
}



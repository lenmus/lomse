//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#include "lomse_caret.h"

#include "lomse_screen_drawer.h"
#include "lomse_graphic_view.h"
#include "lomse_logger.h"


namespace lomse
{

//=======================================================================================
// Caret implementation
//=======================================================================================
Caret::Caret(GraphicView* view, LibraryScope& libraryScope)
    : m_libraryScope(libraryScope)
    , m_pView(view)
    , m_color( Color(0, 0, 255) )               //solid blue
    , m_topLevelSelected( Color(255, 0, 0) )    //solid red
    , m_fVisible(true)              //display caret
    , m_fBlinkStateOn(false)        //currently not drawn
    , m_fBlinkEnabled(false)
    , m_type(k_top_level)
    , m_timecode("1.0.0.0")
#if (LOMSE_USE_BOOST_ASIO == 1)
    , m_blinkTime(500)              //500 milliseconds
    , m_timer( libraryScope.get_io_service() )
#endif
{
    schedule_the_timer();
}

#if (LOMSE_USE_BOOST_ASIO == 1)
//---------------------------------------------------------------------------------------
void Caret::handle_timeout(boost::system::error_code const& cError)
{
    if (cError.value() == boost::asio::error::operation_aborted)
        return;

    if (cError && cError.value() != boost::asio::error::operation_aborted)
        return;     //TODO: throw an exception?

    //repaint/erase the caret
    m_fBlinkStateOn = !m_fBlinkStateOn;
//    m_pView->update_caret();

    // Schedule the timer again...
    schedule_the_timer();
}
#endif

//---------------------------------------------------------------------------------------
void Caret::schedule_the_timer()
{
#if (LOMSE_USE_BOOST_ASIO == 1)
    m_timer.expires_from_now(boost::posix_time::milliseconds(m_blinkTime));
    m_timer.async_wait(boost::bind(&Caret::handle_timeout, this,
                       boost::asio::placeholders::error));
#endif
}

//---------------------------------------------------------------------------------------
void Caret::on_draw(ScreenDrawer* pDrawer)
{
    if (!is_visible())
        return;

    draw_top_level_box(pDrawer);
    if (!m_fBlinkStateOn)
        draw_caret(pDrawer);
    else
        remove_caret(pDrawer);
}

//---------------------------------------------------------------------------------------
void Caret::draw_caret(ScreenDrawer* pDrawer)
{
    switch(m_type)
    {
        case k_top_level:   draw_caret_as_top_level(pDrawer);   break;
        case k_block:       draw_caret_as_block(pDrawer);       break;
        case k_box:         draw_caret_as_box(pDrawer);         break;
        case k_line:        draw_caret_as_line(pDrawer);        break;
        default:
        {
            LOMSE_LOG_ERROR("[Caret::draw_caret] Invalid caret type");
            throw runtime_error("[Caret::draw_caret] Invalid caret type");
        }
    }
}

//---------------------------------------------------------------------------------------
void Caret::draw_caret_as_top_level(ScreenDrawer* pDrawer)
{
    double x1 = double( m_pos.x - 100 );
    double y1 = double( m_pos.y );
    double x2 = double( m_pos.x - 50 );
    double y2 = double( m_pos.y + 500 );

    double line_width = double( pDrawer->Pixels_to_LUnits(1) );

    pDrawer->begin_path();
    pDrawer->fill(m_color);
    pDrawer->stroke(m_color);
    pDrawer->stroke_width(line_width);
    pDrawer->move_to(x1, y1);
    pDrawer->hline_to(x2);
    pDrawer->vline_to(y2);
    pDrawer->hline_to(x1);
    pDrawer->vline_to(y1);
    pDrawer->end_path();

    pDrawer->render();
}

//---------------------------------------------------------------------------------------
void Caret::draw_caret_as_line(ScreenDrawer* pDrawer)
{
//    double x1 = double( m_vline.left() );
//    double y1 = double( m_vline.top() );
//    double x2 = double( m_vline.right() );
//    double y2 = double( m_vline.bottom() );
//
//    pDrawer->screen_point_to_model(&x1, &y1);
//    pDrawer->screen_point_to_model(&x2, &y2);


    double line_width = double( pDrawer->Pixels_to_LUnits(2) );
    double x1 = double( m_pos.x );
    double y1 = double( m_pos.y );
    double x2 = double( m_pos.x + line_width );
    double y2 = double( m_pos.y + m_size.height );

    pDrawer->begin_path();
    pDrawer->fill(m_color);
    pDrawer->stroke(m_color);
    pDrawer->stroke_width(line_width);

    //vertical line
    pDrawer->move_to(x1, y1);
    pDrawer->vline_to(y2);

	//draw horizontal segments
	pDrawer->move_to(x1 - 100, y1);
    pDrawer->hline_to(x2 + 100);
	pDrawer->move_to(x1 - 100, y2);
    pDrawer->hline_to(x2 + 100);

    pDrawer->end_path();
    pDrawer->render();
}

//---------------------------------------------------------------------------------------
void Caret::draw_caret_as_block(ScreenDrawer* pDrawer)
{
    //TODO
}

//---------------------------------------------------------------------------------------
void Caret::draw_caret_as_box(ScreenDrawer* pDrawer)
{
    double line_width = double( pDrawer->Pixels_to_LUnits(1) );

    pDrawer->begin_path();
    pDrawer->fill(Color(0,0,0,0));    //transparent fill
    pDrawer->stroke(m_color);
    pDrawer->stroke_width(line_width);
    pDrawer->rect(m_pos, m_size, 0.0f);
    pDrawer->end_path();

    pDrawer->render();
}

//---------------------------------------------------------------------------------------
void Caret::draw_top_level_box(ScreenDrawer* pDrawer)
{
    Color color = (m_type == k_top_level ? m_color : m_topLevelSelected);
    double line_width = double( pDrawer->Pixels_to_LUnits(1) );

    pDrawer->begin_path();
    pDrawer->fill(Color(0,0,0,0));    //transparent fill
    pDrawer->stroke(color);
    pDrawer->stroke_width(line_width);
    pDrawer->rect(m_box.get_top_left(), USize(m_box.get_width(), m_box.get_height()), 0.0f);
    pDrawer->end_path();

    pDrawer->render();
}

//---------------------------------------------------------------------------------------
void Caret::remove_caret(ScreenDrawer* pDrawer)
{
    //TODO: Restore saved bitmap (?)
}


}  //namespace lomse

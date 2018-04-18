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

#include "lomse_score_player_ctrl.h"

#include "lomse_score_player.h"
#include "lomse_internal_model.h"
#include "lomse_shapes.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_text.h"
#include "lomse_drawer.h"
#include "lomse_calligrapher.h"
#include "lomse_events.h"
#include "lomse_interactor.h"
#include "lomse_logger.h"


namespace lomse
{

//=======================================================================================
// ScorePlayerCtrl implementation
//=======================================================================================
ScorePlayerCtrl::ScorePlayerCtrl(LibraryScope& libScope, ImoScorePlayer* pOwner,
                                 Document* pDoc)
    : Control(libScope, pDoc, nullptr)
    , PlayerGui()
    , m_pOwnerImo(pOwner)
    , m_pMainBox(nullptr)
    , m_width(1000.0f)
    , m_height(600.0f)
    , m_hoverColor( Color(255,80,80) )  //220, 255, 0) )      //greenish yellow
    , m_metronome(60)
    , m_playButtonState(k_play)
    , m_fFullView(false)
{
    m_style = create_default_style();

    m_normalColor = m_style->color();
    m_currentColor = m_normalColor;

    measure();
}

//---------------------------------------------------------------------------------------
ImoStyle* ScorePlayerCtrl::create_default_style()
{
    ImoStyle* style = m_pDoc->create_private_style();
    style->border_width(0.0f)->padding(0.0f)->margin(0.0f);
    style->color( Color(255,255,255) );
    return style;
}

//---------------------------------------------------------------------------------------
USize ScorePlayerCtrl::measure()
{
    return USize(m_width, m_height+500.0f);     //500 = bottom margin
}

//---------------------------------------------------------------------------------------
GmoBoxControl* ScorePlayerCtrl::layout(LibraryScope& UNUSED(libraryScope), UPoint pos)
{
    m_pos = pos;
    m_pMainBox = LOMSE_NEW GmoBoxControl(this, m_pos, m_width, m_height, m_style);
    return m_pMainBox;
}

//---------------------------------------------------------------------------------------
void ScorePlayerCtrl::handle_event(SpEventInfo pEvent)
{
    SpEventMouse pEv( static_pointer_cast<EventMouse>(pEvent) );
    if (!pEv->is_still_valid())
        return;

    if (m_fEnabled)
    {
        if (pEvent->is_mouse_in_event())
        {
            LOMSE_LOG_DEBUG(Logger::k_events | Logger::k_score_player,
                            "Mouse in event received");
            m_currentColor = m_hoverColor;
            m_fFullView = true;
            m_pMainBox->set_dirty(true);
        }
        else if (pEvent->is_mouse_out_event())
        {
            LOMSE_LOG_DEBUG(Logger::k_events | Logger::k_score_player,
                            "Mouse out event received");
            m_fFullView = false;
            m_currentColor = m_normalColor;
            m_pMainBox->set_dirty(true);
        }
        else if (pEvent->is_on_click_event())
        {
            LOMSE_LOG_DEBUG(Logger::k_events | Logger::k_score_player,
                            "Mouse on-click event received");
            m_fFullView = true;
            bool fPlay = (m_playButtonState == k_play );
            if (fPlay)
            {
                set_play_button_state(k_stop);
            }
            else
            {
                set_play_button_state(k_play);
            }

            //create event for user app
            EEventType evType = (fPlay ? k_do_play_score_event : k_stop_playback_event); //k_pause_score_event);
            WpInteractor wpIntor = pEv->get_interactor();
            if (SpInteractor p = wpIntor.lock())
            {
                ImoScore* pScore = m_pOwnerImo->get_score();
                SpEventPlayCtrl event(
                        LOMSE_NEW EventPlayCtrl(evType, wpIntor, pEv->get_document(),
                                                pScore, this) );

                //AWARE: we notify directly to user app. (to observers of Interactor)
                p->notify_observers(event, p.get());
            }
        }
        else
        {
            LOMSE_LOG_WARN("Unknown event received. Type=%d",
                           pEvent->get_event_type() );
        }
    }
}

//---------------------------------------------------------------------------------------
void ScorePlayerCtrl::on_end_of_playback()
{
    set_play_button_state(k_play);
}

//---------------------------------------------------------------------------------------
void ScorePlayerCtrl::set_play_button_state(int value)
{
    m_playButtonState = value;
    if (m_pMainBox)
        m_pMainBox->set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ScorePlayerCtrl::set_tooltip(const string& UNUSED(text))
{
    //TODO: method ScorePlayerCtrl::set_tooltip
}

//---------------------------------------------------------------------------------------
void ScorePlayerCtrl::on_draw(Drawer* pDrawer, RenderOptions& UNUSED(opt))
{
    Color color = (m_fEnabled ? m_currentColor : Color(192, 192, 192));
    Color white(255, 255, 255);
    Color black(0, 0, 0);
    Color dark(83, 80, 72);
    Color clear = white;
    LUnits x = m_pos.x;
    LUnits y = m_pos.y;
    LUnits width = m_width;     //m_fFullView ? 4000.0f : m_width;

    //draw box gradient and border
    //dark.a = 45;
    Color light(dark);
    light = light.gradient(white, 0.2);

    pDrawer->begin_path();
    pDrawer->fill(dark);
    pDrawer->stroke(black);
    pDrawer->stroke_width(15.0);

    pDrawer->gradient_color(white, 0.0, 0.1);
    pDrawer->gradient_color(white, dark, 0.1, 0.7);
    pDrawer->gradient_color(dark, light, 0.7, 1.0);
    pDrawer->fill_linear_gradient(m_pos.x, m_pos.y,
                                  m_pos.x, m_pos.y + m_height);

    pDrawer->rect(UPoint(x, y), USize(width, m_height), 100.0f);
    pDrawer->end_path();

    switch (m_playButtonState)
    {
        case k_play:
//            //triangle (play)
//            pDrawer->begin_path();
//            pDrawer->fill(clear);
//            pDrawer->stroke(color);
//            pDrawer->move_to(x + 360.0f, y + 130.0f);
//            pDrawer->line_to(x + 640.0f, y + 300.0f);
//            pDrawer->line_to(x + 360.0f, y + 470.0f);
//            pDrawer->close_subpath();
//            pDrawer->end_path();
//            break;

            //triangle (play)
            if (!m_fFullView)
            {
                //mouse out
                pDrawer->begin_path();
                pDrawer->fill(clear);
                pDrawer->stroke(color);
                pDrawer->move_to(x + 360.0f, y + 130.0f);
                pDrawer->line_to(x + 640.0f, y + 300.0f);
                pDrawer->line_to(x + 360.0f, y + 470.0f);
                pDrawer->close_subpath();
                pDrawer->end_path();
            }
            else
            {
                //mouse in
                pDrawer->begin_path();
                Color blurred(255, 255, 220);   // = color;
                Color none(255, 255, 255, 0);
                blurred.a = 150;
                pDrawer->fill(blurred);
                pDrawer->stroke(none);
                pDrawer->move_to(x + 360.0f - 50.0f, y + 130.0f - 80.0f);
                pDrawer->line_to(x + 640.0f + 100.0f, y + 300.0f);
                pDrawer->line_to(x + 360.0f - 50.0f, y + 470.0f + 80.0f);
                pDrawer->close_subpath();
                pDrawer->end_path();

                pDrawer->begin_path();
                blurred.a = 220;
                pDrawer->fill(blurred);
                pDrawer->stroke(none);
                pDrawer->move_to(x + 360.0f - 25.0f, y + 130.0f - 40.0f);
                pDrawer->line_to(x + 640.0f + 50.0f, y + 300.0f);
                pDrawer->line_to(x + 360.0f - 25.0f, y + 470.0f + 40.0f);
                pDrawer->close_subpath();
                pDrawer->end_path();

//                pDrawer->begin_path();
//                pDrawer->fill(color);
//                pDrawer->stroke(color);
//                pDrawer->move_to(x + 360.0f, y + 130.0f);
//                pDrawer->line_to(x + 640.0f, y + 300.0f);
//                pDrawer->line_to(x + 360.0f, y + 470.0f);
//                pDrawer->close_subpath();
//                pDrawer->end_path();


// see www.crossgl.com/aggpas/documentation

                Color red(255,0,0);
                Color yellow(255,255,0);
                red = red.gradient(red, 0.2);

                pDrawer->begin_path();
                pDrawer->fill(red);
                pDrawer->stroke(red);
                pDrawer->stroke_width(10.0);
                pDrawer->gradient_color(red, 0.0, 0.1);
                pDrawer->gradient_color(red, yellow, 0.1, 0.90);
                pDrawer->gradient_color(yellow, white, 0.90, 1.0);
                pDrawer->fill_linear_gradient(x, y+130.0f, x, y+470.0f);
                pDrawer->move_to(x + 360.0f, y + 130.0f);
                pDrawer->line_to(x + 640.0f, y + 300.0f);
                pDrawer->line_to(x + 360.0f, y + 470.0f);
                pDrawer->close_subpath();
                pDrawer->end_path();

            }
            break;

//            //draw glowing line
// glow effect is just drawing a blured layer in light color, under main layer.
// in this test, instaed of blurring the original shape, just do a secon drawing more
// light
//            set_alpha(0.2)
//            set_line_width(5)
//            draw_line
//            set_alpha(0.07)
//            set_line_width(15)
//            draw_line


        case k_stop:
            //square (stop)
            pDrawer->begin_path();
            pDrawer->fill(clear);
            pDrawer->stroke(color);
            pDrawer->rect(UPoint(x+370.0f, y+170.0f), USize(260.0f, 260.0f), 0.0f);
            pDrawer->end_path();
            break;

        case k_pause:
            //two bars (pause)
            pDrawer->begin_path();
            pDrawer->fill(clear);
            pDrawer->stroke(color);
            pDrawer->rect(UPoint(x+150.0f, y+150.0f), USize(120.0f, 300.0f), 0.0f);
            pDrawer->rect(UPoint(x+330.0f, y+150.0f), USize(120.0f, 300.0f), 0.0f);
            pDrawer->end_path();
            break;

        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
bool ScorePlayerCtrl::countoff_status()
{
    return false;
}

//---------------------------------------------------------------------------------------
bool ScorePlayerCtrl::metronome_status()
{
    return false;
}

//---------------------------------------------------------------------------------------
int ScorePlayerCtrl::get_play_mode()
{
    return k_play_normal_instrument;
}

//---------------------------------------------------------------------------------------
int ScorePlayerCtrl::get_metronome_mm()
{
    return m_metronome;
}

//---------------------------------------------------------------------------------------
Metronome* ScorePlayerCtrl::get_metronome()
{
    return m_libraryScope.get_global_metronome();
}


}   //namespace lomse

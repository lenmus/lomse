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

#include "lomse_half_page_view.h"

#include "lomse_graphical_model.h"
#include "lomse_gm_basic.h"
#include "lomse_screen_drawer.h"
#include "lomse_interactor.h"
#include "lomse_box_system.h"

namespace lomse
{


//=======================================================================================
// HalfPageView implementation
//=======================================================================================
HalfPageView::HalfPageView(LibraryScope& libraryScope, ScreenDrawer* pDrawer)
    : SinglePageView(libraryScope, pDrawer)
    , m_fPlaybackMode(false)
    , m_fSplitMode(false)
    , m_fIsScore(false)
    , m_buf(nullptr)
    , m_vxOrgPlay{0,0}
    , m_vyOrgPlay{0,0}
{
    m_backgroundColor = Color(255, 255, 255);
}

//---------------------------------------------------------------------------------------
bool HalfPageView::is_valid_for_this_view(Document* pDoc)
{
    m_fIsScore = pDoc->get_num_content_items() == 1
                 && pDoc->get_content_item(0)->is_score();
    return true;
}

//---------------------------------------------------------------------------------------
void HalfPageView::compute_buffer_split()
{
    unsigned char* buf =  m_pRenderBuf->buf();
    unsigned width = m_pRenderBuf->width();
    unsigned height = m_pRenderBuf->height();

    if (buf != nullptr)
    {
        if (m_buf == nullptr || width != m_bufWidth)
        {
            LOMSE_LOG_DEBUG(Logger::k_mvc, "New buffer: first buffer or different width");
        }
        else if (height == m_bufHeight && buf == m_buf)
            return;
        else if (height == m_SplitHeight && (buf == m_TopBuf || buf == m_BottomBuf))
            return;
        else
        {
            LOMSE_LOG_DEBUG(Logger::k_mvc, "New buffer: different height");
        }

        //save new buffer info
        m_buf = buf;
        m_bufWidth = width;
        m_bufHeight = height;
        m_bufStride = m_pRenderBuf->stride();

        //compute window split
        m_SplitHeight = m_bufHeight / 2 - 5;
        m_BottomBuf = m_buf + (m_bufHeight / 2 + 5) * m_bufStride;
        m_TopBuf = m_buf;
    }
}

//---------------------------------------------------------------------------------------
void HalfPageView::draw_all()
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, std::string());

    compute_buffer_split();
    decide_split_or_normal_view();
    do_draw_all();
}

//---------------------------------------------------------------------------------------
void HalfPageView::do_draw_all()
{
    if (!m_fSplitMode)
    {
        //single page view
        m_pRenderBuf->attach(m_buf, m_bufWidth, m_bufHeight, m_bufStride);
        GraphicView::draw_all();
    }
    else
    {
        //we are in playback mode. Window being played must be the last one rendered,
        //so that buffer remains positioned to receive highlight events
        if (m_iPlayWindow == 0)
        {
            draw_bottom_window();
            draw_separation_line();
            draw_top_window();
        }
        else
        {
            draw_top_window();
            draw_separation_line();
            draw_bottom_window();
        }
    }
}

//---------------------------------------------------------------------------------------
void HalfPageView::draw_bottom_window()
{
    m_pRenderBuf->attach(m_BottomBuf, m_bufWidth, m_SplitHeight, m_bufStride);
    GraphicView::do_change_viewport(m_vxOrgPlay[1], m_vyOrgPlay[1]);
    GraphicView::draw_all();
}

//---------------------------------------------------------------------------------------
void HalfPageView::draw_separation_line()
{
    unsigned char* buf = m_buf + m_SplitHeight * m_bufStride;
    m_pRenderBuf->attach(buf, m_bufWidth, 10, m_bufStride);
    m_pDrawer->reset(*m_pRenderBuf, Color(128, 128, 200));
}

//---------------------------------------------------------------------------------------
void HalfPageView::draw_top_window()
{
    m_pRenderBuf->attach(m_TopBuf, m_bufWidth, m_SplitHeight, m_bufStride);
    GraphicView::do_change_viewport(m_vxOrgPlay[0], m_vyOrgPlay[0]);
    GraphicView::draw_all();
}

//---------------------------------------------------------------------------------------
void HalfPageView::do_change_viewport(Pixels x, Pixels y)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, "New viewport x=%d, y=%d", x, y);

    if (!m_fSplitMode)
    {
        m_vxOrgPlay[0] = x;
        m_vyOrgPlay[0] = y;
    }
    m_vxOrgPlay[1] = x;
    m_vyOrgPlay[1] = y;

    GraphicView::do_change_viewport(x, y);
}

//---------------------------------------------------------------------------------------
void HalfPageView::on_mode_changed(int mode)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, "Change to mode = %d (0=read-only, 1=edition, 2=playback)", mode);

    bool fWasInSplitMode = m_fSplitMode;
    m_fPlaybackMode = (mode == Interactor::k_mode_playback);
    decide_split_or_normal_view();

    if (m_fSplitMode)
    {
        //playback started in split mode
        send_enable_scroll_event(false);
        decide_systems_to_display();
    }
    else if (fWasInSplitMode)
    {
        //playback finished and was in split mode
        send_enable_scroll_event(true);
        remove_split();
    }

    set_visual_effects_for_mode(mode);
    do_draw_all();
}

//---------------------------------------------------------------------------------------
void HalfPageView::decide_split_or_normal_view()
{
    m_fSplitMode = false;
    if (m_fPlaybackMode && m_fIsScore)
    {
        //convert buffer height to LUnits
        LUnits bufHeight = m_pDrawer->Pixels_to_LUnits(m_bufHeight);

        //get maximum system height
        GraphicModel* pGModel = get_graphic_model();
        GmoBoxDocument* pBDoc = pGModel->get_root();
        GmoBoxDocPage* pBDocPage = pBDoc->get_page(0);
        GmoBoxDocPageContent* pBPC = static_cast<GmoBoxDocPageContent*>(pBDocPage->get_child_box(0));
        m_pBSP = static_cast<GmoBoxScorePage*>(pBPC->get_child_box(0));
        LUnits maxHeight = m_pBSP->get_max_system_height();

        LOMSE_LOG_DEBUG(Logger::k_mvc, "maxHeight = %f, bufHeight=%f", maxHeight, bufHeight);

        //Do not split window if height lower than 2 * maxSystemHeight + some pixels (40px)
        LUnits extra = m_pDrawer->Pixels_to_LUnits(40);
        if (bufHeight < (2.0f * maxHeight + extra))
            return;

        //Do not split window if width lower than system width
        GmoBoxSystem* pSys = m_pBSP->get_system(0);
        LUnits bufWidth = m_pDrawer->Pixels_to_LUnits(m_bufWidth);
        if (bufWidth < (2.0f * extra + pSys->get_width()))
            return;

        m_fSplitMode = true;

        //determine how many systems fit in a sub-window
        m_nSysIncr = int(((bufHeight / 2.0f) - extra) / maxHeight);
    }
}

//---------------------------------------------------------------------------------------
void HalfPageView::decide_systems_to_display()
{
    m_iSystem = 0;                                  //system to display: first system
    m_curSystem = 0;                                //system being played
    m_iPlayWindow = 0;                              //playback window: top
    int iNextWindow = (m_iPlayWindow==0 ? 1 : 0);     //next window: the other one

    //set viewport for first window
    //viewport is in pixels. It refers to user space converted to pixels
    GmoBoxSystem* pSys = m_pBSP->get_system(m_iSystem);
////    m_vxOrgPlay[m_iPlayWindow] = 0;
    m_vyOrgPlay[m_iPlayWindow] = m_pDrawer->LUnits_to_Pixels(pSys->get_origin().y);

    //set viewport for next window
    m_iSystem += m_nSysIncr;
    //TODO: limit when max number of systems reached
    pSys = m_pBSP->get_system(m_iSystem);
////    m_vxOrgPlay[iNextWindow] = 0;
    m_vyOrgPlay[iNextWindow] = m_pDrawer->LUnits_to_Pixels(pSys->get_origin().y);
}

//---------------------------------------------------------------------------------------
void HalfPageView::change_viewport_if_necessary(ImoId id)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, std::string());
    if (!m_fSplitMode)
        SinglePageView::change_viewport_if_necessary(id);
}

//---------------------------------------------------------------------------------------
//void HalfPageView::do_change_viewport_if_necessary()
void HalfPageView::do_move_tempo_line_and_change_viewport(ImoId scoreId, TimeUnits timepos,
                                                          bool fTempoLine, bool fViewport)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, std::string());

    if (!m_fSplitMode)
        SinglePageView::do_move_tempo_line_and_change_viewport(scoreId, timepos,
                                                               fTempoLine, fViewport);

    //Here it is necessary to determine if playback has arrived to switch point. If so,
    //switch windows and display next system

    //Determine the system (in m_pScrollSystem)
    if (!determine_page_system_and_position_for(scoreId, timepos))
        return;

    if (m_pScrollSystem->get_system_number() - m_curSystem >= m_nSysIncr)
    {
        //playback has arrived to switch point. Switch windows and display next system
        int iNextWindow = m_iPlayWindow;
        m_iPlayWindow = (m_iPlayWindow==0 ? 1 : 0);
        //update system being played
        m_curSystem  += m_nSysIncr;
        //update viewport for top window
        m_iSystem += m_nSysIncr;
        int numSystems = m_pBSP->get_num_systems();
        if (m_iSystem < numSystems)
        {
            GmoBoxSystem* pSys = m_pBSP->get_system(m_iSystem);
            m_vyOrgPlay[iNextWindow] = m_pDrawer->LUnits_to_Pixels(pSys->get_origin().y);
        }
        else
        {
            //set viewport after last system
            GmoBoxSystem* pSys = m_pBSP->get_system(numSystems - 1);
            LUnits yAfter = pSys->get_origin().y + pSys->get_height() + 2000.0f;    //2cm
            m_vyOrgPlay[iNextWindow] = m_pDrawer->LUnits_to_Pixels(yAfter);
        }

        do_draw_all();
    }
}

//---------------------------------------------------------------------------------------
void HalfPageView::remove_split()
{
    //playback has finished. Remove the split and keep last system at same window pos.

    if (m_iPlayWindow == 0)
    {
        //nothing to do. Last system is displayed in top window. Viewport correctly set
    }
    else
    {
        //Last system is displayed in bottom window. Set viewport so that last system
        //doesn't move from current position

        int numSystems = m_pBSP->get_num_systems();
        GmoBoxSystem* pSys = m_pBSP->get_system(numSystems - 1);
        Pixels y = m_pDrawer->LUnits_to_Pixels( pSys->get_origin().y );
        m_vyOrgPlay[0] = y - m_SplitHeight - 10;
        GraphicView::do_change_viewport(m_vxOrgPlay[0], m_vyOrgPlay[0]);
    }
}

//---------------------------------------------------------------------------------------
void HalfPageView::send_enable_scroll_event(bool enable)
{
}


}  //namespace lomse

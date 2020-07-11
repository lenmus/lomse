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
#include "private/lomse_internal_model_p.h"
#include "lomse_midi_table.h"

namespace lomse
{


//=======================================================================================
// HalfPageView implementation
//=======================================================================================
const int k_splitLineHeight = 2;      //line heigth = 2px
const Color k_splitLineColor = Color(128, 128, 200);

//---------------------------------------------------------------------------------------
HalfPageView::HalfPageView(LibraryScope& libraryScope, ScreenDrawer* pDrawer)
    : SinglePageView(libraryScope, pDrawer)
    , m_fPlaybackMode(false)
    , m_fSplitMode(false)
    , m_pScore(nullptr)
    , m_buf(nullptr)
    , m_vxOrgPlay{0,0}
    , m_vyOrgPlay{0,0}
{
    m_backgroundColor = Color(255, 255, 255);
}

//---------------------------------------------------------------------------------------
bool HalfPageView::is_valid_for_this_view(Document* pDoc)
{
    if (pDoc->get_num_content_items() == 1
        && pDoc->get_content_item(0)->is_score())
    {
        m_pScore = static_cast<ImoScore*>(pDoc->get_first_content_item());
    }

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
        m_SplitHeight = (m_bufHeight - k_splitLineHeight) / 2;
        m_BottomBuf = m_buf + ((m_bufHeight + k_splitLineHeight) / 2) * m_bufStride;
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
    m_pRenderBuf->attach(buf, m_bufWidth, k_splitLineHeight, m_bufStride);
    m_pDrawer->reset(*m_pRenderBuf, k_splitLineColor);
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
    if (m_pScore && m_fPlaybackMode)
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

        //Do not split window if height lower than 2 * maxSystemHeight
        if (bufHeight < (2.0f * maxHeight))
            return;

        //Do not split window if width lower than system width
        GmoBoxSystem* pSys = m_pBSP->get_system(0);
        LUnits bufWidth = m_pDrawer->Pixels_to_LUnits(m_bufWidth);
        if (bufWidth < pSys->get_width())
            return;

        //do not split if the full score fits in the window
        if (bufHeight > m_pBSP->get_height())
            return;

        m_fSplitMode = true;

        m_winHeight = bufHeight / 2.0f;
    }
}

//---------------------------------------------------------------------------------------
void HalfPageView::decide_systems_to_display()
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, std::string());

    create_systems_jumps_table();

    m_iSystem = 0;                          //index on m_systems: system to display, first one
    m_curSystem = m_systems[m_iSystem];     //system being played
    m_iPlayWindow = 0;                      //playback window: 0=top window

    determine_how_many_systems_fit_and_effective_window_height(m_iPlayWindow, m_curSystem);

    //set viewport for first window. Viewport is in pixels
    GmoBoxSystem* pSys = m_pBSP->get_system( m_curSystem );
////    m_vxOrgPlay[m_iPlayWindow] = 0;
    m_vyOrgPlay[m_iPlayWindow] = m_pDrawer->LUnits_to_Pixels(pSys->get_origin().y);

    //now in top window there are m_nSys[0] systems displayed, starting with m_curSystem
    //display systems in the other window
    set_viewport_for_next(1);   //1 = bottom window
}

//---------------------------------------------------------------------------------------
void HalfPageView::determine_how_many_systems_fit_and_effective_window_height(int iWindow, int iSys)
{
    m_nSys[iWindow] = 0;
    m_height[iWindow] = 0.0f;
    while (iSys < m_pBSP->get_num_systems())
    {
        GmoBoxSystem* pSys = m_pBSP->get_system(iSys);
        if (m_winHeight - m_height[iWindow] >= pSys->get_height())
        {
            //system fits. Add it
            ++m_nSys[iWindow];
            m_height[iWindow] += pSys->get_height();
        }
        else
            break;
    }
//    assert(m_nSys[iWindow] > 0);

    LOMSE_LOG_DEBUG(Logger::k_mvc, "iWindow = %d, iSys=%d, m_nSys[iWindow]=%d, m_height[iWindow]=%f",
                    iWindow, iSys, m_nSys[iWindow], m_height[iWindow]);

}

//---------------------------------------------------------------------------------------
void HalfPageView::change_viewport_if_necessary(ImoId id)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, std::string());
    if (!m_fSplitMode)
        SinglePageView::change_viewport_if_necessary(id);
}

//---------------------------------------------------------------------------------------
void HalfPageView::do_move_tempo_line_and_change_viewport(ImoId scoreId, TimeUnits timepos,
                                                          bool fTempoLine, bool fViewport)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, std::string());

    if (!m_fSplitMode)
    {
        SinglePageView::do_move_tempo_line_and_change_viewport(scoreId, timepos,
                                                               fTempoLine, fViewport);
        return;
    }

    //Here it is necessary to determine if playback has arrived to switch point. If so,
    //switch windows and display next system

    //Determine the system (in m_pScrollSystem)
    if (!determine_page_system_and_position_for(scoreId, timepos))
        return;

    //check if next system is already displayed in playback window
    int iFirst = m_pScrollSystem->get_system_number();
    bool fDisplayed = (iFirst >= m_curSystem) && (iFirst < (m_curSystem + m_nSys[m_iPlayWindow]));

    LOMSE_LOG_DEBUG(Logger::k_mvc, "iFirst = %d, m_iPlayWindow=%d, m_curSystem=%d, m_nSys[m_iPlayWindow]=%d, fDisplayed=%d",
                    iFirst, m_iPlayWindow, m_curSystem, m_nSys[m_iPlayWindow], (fDisplayed ? 1:0));
    if (!fDisplayed)
    {
        //playback has arrived to switch point. Switch windows and display next system
        int iNextWindow = m_iPlayWindow;
        m_iPlayWindow = (m_iPlayWindow==0 ? 1 : 0);
        //update system being played
        m_curSystem = iFirst;

        set_viewport_for_next(iNextWindow);
        do_draw_all();
    }
}

//---------------------------------------------------------------------------------------
void HalfPageView::set_viewport_for_next(int iNextWindow)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, "iNextWindow=%d, m_iSystem=%d", iNextWindow, m_iSystem);
    //now in playback window there are m_nSys[iPlay] systems displayed, starting with m_curSystem

    //advance while system already displayed in the playback window
    int iPlayWindow = (iNextWindow==0 ? 1 : 0);
    bool fDisplayed = true;     //m_iSystem is displayed
    int iFirst = -1;
    while (fDisplayed && m_iSystem < int(m_systems.size()-1))
    {
        ++m_iSystem;
        iFirst = m_systems[m_iSystem];
        //check if already displayed in playback window
        fDisplayed = (iFirst >= m_curSystem) && (iFirst < (m_curSystem + m_nSys[iPlayWindow]));
    }
    LOMSE_LOG_DEBUG(Logger::k_mvc, "iFirst = %d, m_iSystem=%d, fDisplayed=%d",
                    iFirst, m_iSystem, (fDisplayed ? 1:0));

    //update viewport for next window
    if (!fDisplayed && iFirst >= 0)
    {
        determine_how_many_systems_fit_and_effective_window_height(iNextWindow, iFirst);

        //display m_nSys[iNextWindow] systems, starting with iFirst
        GmoBoxSystem* pSys = m_pBSP->get_system(iFirst);
        m_vyOrgPlay[iNextWindow] = m_pDrawer->LUnits_to_Pixels(pSys->get_origin().y);
    }
    else
    {
        //set viewport after last system
        GmoBoxSystem* pSys = m_pBSP->get_system(m_systems.back());
        LUnits yAfter = pSys->get_origin().y + pSys->get_height() + 2000.0f;    //2cm
        m_vyOrgPlay[iNextWindow] = m_pDrawer->LUnits_to_Pixels(yAfter);

        m_nSys[iNextWindow] = 0;     //nothing displayed
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

        GmoBoxSystem* pSys = m_pBSP->get_system(m_systems.back());
        Pixels y = m_pDrawer->LUnits_to_Pixels( pSys->get_origin().y );
        m_vyOrgPlay[0] = y - m_SplitHeight - 10;
        GraphicView::do_change_viewport(m_vxOrgPlay[0], m_vyOrgPlay[0]);
    }
}

//---------------------------------------------------------------------------------------
void HalfPageView::send_enable_scroll_event(bool enable)
{
}

//---------------------------------------------------------------------------------------
void HalfPageView::create_systems_jumps_table()
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, std::string());

    if (m_pScore)
    {
        GraphicModel* pGModel = get_graphic_model();
        if (!pGModel)
            return;

        SoundEventsTable* pSM = m_pScore->get_midi_table();
        vector<MeasuresJumpsEntry*> measuresTable = pSM->get_measures_jumps();

        //now just transform measures into systems

        ImoId scoreId = m_pScore->get_id();
        int iPrevSys = -1;
        m_systems.clear();
        for(auto it : measuresTable)
        {
            LOMSE_LOG_INFO(it->dump_entry());
            GmoBoxSystem* pFromSys = pGModel->get_system_for(scoreId, it->get_from_timepos());
            GmoBoxSystem* pToSys = pGModel->get_system_for(scoreId, it->get_to_timepos());
            int iFrom = pFromSys->get_system_number();
            int iTo = pToSys->get_system_number();
            LOMSE_LOG_INFO("from s=%d to s=%d", pFromSys->get_system_number(),
                            pToSys->get_system_number());
            int i = (iPrevSys != iFrom ? iFrom : iFrom+1);
            for(; i <= iTo; ++i)
            {
                m_systems.push_back(i);
            }
            iPrevSys = iTo;
        }

        for(auto it : m_systems)
            LOMSE_LOG_INFO("system %d", it);
    }
}


}  //namespace lomse

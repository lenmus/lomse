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

#ifndef __LOMSE_HALF_PAGE_VIEW_H__
#define __LOMSE_HALF_PAGE_VIEW_H__

#include "lomse_graphic_view.h"


///@cond INTERNAL
namespace lomse
{
///@endcond


//---------------------------------------------------------------------------------------
/** %HalfPageView is a GraphicView for rendering scores.

    This view has a double behaviour. In normal mode (no playback) it behaves as
    SinglePageView, that is the score is rendered on a single page as high as necessary
    to contain all the score (e.g., an HTML page having a body of fixed size).

    But when in playback mode, the bitmap to be rendered in the application window is
    split horizontally in two halves, originating two virtual vertical windows, one at
    top and the other at bottom. In the top window it is displayed the first score chunk
    and the second chunk is displayed in the bottom window. When playback enters in the
    bottom window the top window is updated with the third chunk. Next, when the playback
    enters again in the top window, the bottom window is updated with the next chunk,
    and so on.

    When playback is finished (because the end of the score is reached or because the
    app stops playback  -- but not when playback is paused --) the display returns
    automatically to SinglePageView mode.

    This view allows to solve the problem page turning and the problem of repetition
    marks and jumps:  when the window being played has a jump to a system not visible
    in that window, the other window is updated  with the next logical system (the one
    containing the jump destination) instead of with the next system. The behaviour for
    the user is very simple: when the end of the last system in current window is reached
    or when user finds a jump to a point that is not displayed in current window, he/she
    will know that it is necessary to jump to the other half window.

    You should note that when the view is split, only full systems are displayed in
    the sub-windows. This is to avoid doubts about were the music continues, that could
    appear in cases in which next system is practically fully displayed after current
    one. It also makes the display more clear.

    The view does not enter in split mode in the following cases:
    - when window height is smaller than two times the highest system,
    - when window width is lower than system width,
    - when the score fits completely in the window height, or
    - when the document is not only one score
    In these cases, playback behaviour will be that of SinglePageView

    When the view is in split mode (during playback) all settings for controlling what
    to display and how, are fully controlled by the view. Therefore, the user application
    should disable all user controls that could invoke methods to do changes, such as
    scroll position, zoom factor or other. Invoking these methods will not cause crashes
    but will cause unnecessary interference with what is displayed.

*/
class LOMSE_EXPORT HalfPageView : public SinglePageView
{
public:
///@cond INTERNALS
//excluded from public API because the View methods are managed from Interactor

    HalfPageView(LibraryScope& libraryScope, BitmapDrawer* pDrawer, BitmapDrawer* pPrintDrawer);
    virtual ~HalfPageView() {}

    //overrides for SinglePageView
    bool is_valid_for_this_view(Document* pDoc) override;

    //overrides for GraphicView
    void on_mode_changed(int mode) override;
    void change_viewport_if_necessary(ImoId id) override;


protected:
    //overrides for GraphicView
    void draw_all() override;
    void do_change_viewport(Pixels x, Pixels y) override;
    void do_move_tempo_line_and_change_viewport(ImoId scoreId, TimeUnits timepos,
                                                bool fTempoLine, bool fViewport) override;

    //specific internal methods for this view
    void compute_buffer_split();
    void decide_split_or_normal_view();
    void decide_systems_to_display();
    void do_draw_all();
    void draw_bottom_window();
    void draw_separation_line();
    void draw_top_window();
    void remove_split();
    void send_enable_scroll_event(bool enable);
    void set_viewport_for_next(int iNextWindow);
    void create_systems_jumps_table();
    void determine_how_many_systems_fit_and_effective_window_height(int iWindow, int iSys);


protected:
    //data to control split mode
    bool    m_fPlaybackMode;    //true during playback (pause also included)
    bool    m_fSplitMode;       //true when the view must be split
    ImoScore* m_pScore;         //if the document is only one score; otherwise, nullptr
    BitmapDrawer* m_pBmpDrawer;     //m_pDrawer casted to BitmapDrawer

    //temporary data: last processed rendering buffer.
    //User app. can change the buffer but the view does not receive notifications.
    //Therefore, for detecting a buffer change, we need to save this information
    unsigned char* m_buf;
    unsigned m_bufWidth;
    unsigned m_bufHeight;

    //temporary data: split buffer parameters
    unsigned m_SplitHeight;
    unsigned m_yShiftBottom;
    unsigned m_usedHeight[2];   //real used height by sub-window

    //temporary data: for controlling real used heigh
    LUnits m_winHeight;     //total sub-window height
    LUnits m_height[2];     //sub-window occupied height
    int m_nSys[2];          //number of systems displayed in each sub-window

    //temporary data: viewport origin for each sub-window
    Pixels m_vxOrgPlay[2], m_vyOrgPlay[2];

    //temporary data, to control display while playback
    GmoBoxScorePage* m_pBSP;    //score-page, to access systems
    int m_iPlayWindow;      //index (0=top or 1=bottom) to playback window
    int m_curSystem;        //current system being played (0..n-1)
    int m_iSystem;          //index on m_systems: next system to display
    std::vector<int> m_systems;     //indices to systems to play, in playback order


///@endcond
};




}   //namespace lomse

#endif      //__LOMSE_HALF_PAGE_VIEW_H__

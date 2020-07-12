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

    It has a double behaviour. For displayin the score, it behaves as a SinglePageView,
    that is, the score is displayed as ...

    But during playback, the behaviour is different. When playback starts,
    The window is split horizontally in two halfs, originating two virtual vertical
    windows, one at top and the other at bottom. In top window it is displayed the first
    score chunk and the second chunk is displayed in the bottom windows. When playback
    enters in the bottom window the top windows is updated with the third chunk. Next,
    when the playback enters again in top window, the bottom window is updated with the
    next chunk, and so on.

    This view solves the problem of repetition marks and jumps: when the half window
    being played has a jump, the other half window is updated with the next logical system
    positioned at right measure instead of with the next chunk. The behaviour for the
    user is simple: when the user finds a jump he/she will know that it is necessary to
    jump to the other half window, unless the repetition is over and the music should
    continue from that point.

    But it is only useful when system height is not big, smaller than half window.

    <b>Margins</b>

    The document is displayed on a white paper and the view has no margins, that is, the
    default view origin point is (0.0, 0.0). Therefore, document content will be
    displayed with the margins defined in the document.


    <b>Background color</b>

    The white paper is surrounded by the background, that will be visible only when the
    user application changes the viewport (e.g., by scrolling right).
    In %HalfPageView the default background color is white and, as with all Views,
    the background color can be changed by invoking Interactor::set_view_background().
*/
class LOMSE_EXPORT HalfPageView : public SinglePageView
{
public:
///@cond INTERNALS
//excluded from public API because the View methods are managed from Interactor

    HalfPageView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
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

    //temporary data: last processed rendering buffer.
    //User app. can change the buffer but the view does not receive notifications.
    //Therefore, for detecting a buffer change, we need to save this information
    unsigned char* m_buf;
    unsigned m_bufWidth;
    unsigned m_bufHeight;
    int m_bufStride;

    //temporary data: split buffer parameters
    unsigned m_SplitHeight;
    unsigned char* m_BottomBuf;
    unsigned char* m_TopBuf;
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

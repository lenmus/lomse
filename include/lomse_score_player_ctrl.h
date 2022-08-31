//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef _LOMSE_SCORE_PLAYER_CTRL_H__
#define _LOMSE_SCORE_PLAYER_CTRL_H__

#include "lomse_config.h"
#if (LOMSE_ENABLE_THREADS == 1)

#include "lomse_control.h"
#include "lomse_player_gui.h"


namespace lomse
{

//forward declarations
class ImoScorePlayer;
class Metronome;


//---------------------------------------------------------------------------------------
// A Control containing a static text element and/or and image which links to an URL
class ScorePlayerCtrl : public Control, public PlayerGui
{
protected:
    GmoBoxControl* m_pMainBox;
    UPoint  m_pos;
    LUnits  m_width;
    LUnits  m_height;

    Color   m_normalColor;
    Color   m_hoverColor;
    Color   m_currentColor;

    int     m_metronome;
    int     m_playButtonState;
    bool    m_fFullView;

public:
    ScorePlayerCtrl(LibraryScope& libScope, ImoScorePlayer* pOwner, Document* pDoc);

    //Control mandatory overrides
    USize measure() override;
    GmoBoxControl* layout(LibraryScope& libraryScope, UPoint pos) override;
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;
    void handle_event(SpEventInfo pEvent) override;
    LUnits width() override { return m_width; }
    LUnits height() override { return m_height; }
    LUnits top() override { return m_pos.y; }
    LUnits bottom() override { return m_pos.y + m_height; }
    LUnits left() override { return m_pos.x; }
    LUnits right() override { return m_pos.x + m_width; }

    //PlayerGui mandatory overrides
    void on_end_of_playback() override;
    int get_play_mode() override;
    int get_metronome_mm() override;
    Metronome* get_metronome() override;
    bool countoff_status() override;
    bool metronome_status() override;

    //specific methods
    void set_text(const string& UNUSED(text)) {}
    void set_tooltip(const string& text);
    inline void set_metronome_mm(int value) { m_metronome = value; }
    void set_play_button_state(int value);

    enum { k_stop=0, k_pause, k_play};  //play button state

protected:
    ImoStyle* create_default_style();

};


} //namespace lomse

#endif //LOMSE_ENABLE_THREADS == 1

#endif    //_LOMSE_SCORE_PLAYER_CTRL_H__

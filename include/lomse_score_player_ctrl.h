//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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
//---------------------------------------------------------------------------------------

#ifndef _LOMSE_SCORE_PLAYER_CTRL_H__
#define _LOMSE_SCORE_PLAYER_CTRL_H__

#include "lomse_control.h"


namespace lomse
{

//forward declarations
class ImoScorePlayer;


//---------------------------------------------------------------------------------------
// A Control containing s static text element and/or and image which links to an URL
class ScorePlayerCtrl : public Control
{
protected:
    LibraryScope& m_libraryScope;
    ImoScorePlayer* m_pOwnerImo;
    string m_label;
    GmoBoxControl* m_pMainBox;
    ImoStyle* m_style;
    UPoint  m_pos;
    LUnits  m_width;
    LUnits  m_height;
    LUnits  m_xCenter;
    LUnits  m_yCenter;

    Color   m_normalColor;
    Color   m_hoverColor;
    Color   m_visitedColor;
    Color   m_prevColor;
    Color   m_currentColor;
    bool    m_visited;

public:
    ScorePlayerCtrl(LibraryScope& libScope, ImoScorePlayer* pOwner, Document* pDoc);
    virtual ~ScorePlayerCtrl() {}

    //Control mandatory overrides
    USize measure();
    GmoBoxControl* layout(LibraryScope& libraryScope, UPoint pos);
    void on_draw(Drawer* pDrawer, RenderOptions& opt);
    void handle_event(SpEventInfo pEvent);
    LUnits width() { return m_width; }
    LUnits height() { return m_height; }
    LUnits top() { return m_pos.y; }
    LUnits bottom() { return m_pos.y + m_height; }
    LUnits left() { return m_pos.x; }
    LUnits right() { return m_pos.x + m_width; }

    //specific methods
    void set_text(const string& text);
    void set_tooltip(const string& text);
    void change_label(const string& text);

protected:
    void select_font();
    URect determine_text_position_and_size();
    ImoStyle* create_default_style();

};


} //namespace lomse

#endif    //_LOMSE_SCORE_PLAYER_CTRL_H__

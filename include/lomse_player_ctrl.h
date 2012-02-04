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

#ifndef _LOMSE_PLAYER_CTRL_H__
#define _LOMSE_PLAYER_CTRL_H__

namespace lomse
{

//---------------------------------------------------------------------------------------
// PlayerCtrl: Interface for any GUI control for score playback. It will receive 
//  end_of_playback events and is responsible for ensuring GUI consistency (i.e.
//  changing the state of "playing now" display or "stop play" button)
class PlayerCtrl
{
protected:
    //LibraryScope& m_libraryScope;

public:
    PlayerCtrl() {}     //LibraryScope& libScope);
    virtual ~PlayerCtrl() {}

    //mandatory overrides
    virtual void on_end_of_playback() = 0;

};


} //namespace lomse

#endif    //_LOMSE_PLAYER_CTRL_H__

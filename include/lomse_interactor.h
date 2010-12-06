//---------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_INTERACTOR_H__
#define __LOMSE_INTERACTOR_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include <iostream>
using namespace std;

namespace lomse
{

//forward declarations
class Document;
class DocCursor;
//class UserCommandExecuter;
//class LdpCompiler;


//---------------------------------------------------------------------------------------
//Abstract class from which all Interactors must derive
class Interactor
{
protected:
    LibraryScope&           m_libScope;
    Document*               m_pDoc;
    //UserCommandExecuter*    m_pExec;
    //LdpCompiler*            m_pCompiler;

public:
    Interactor(LibraryScope& libraryScope, Document* pDoc); //, UserCommandExecuter* pExec);
    virtual ~Interactor();

    ////abstract class implements all possible commands. Derived classes override
    ////them as needed, either programming a diferent behaviour or as empty methods 
    ////for those commands not allowed
    //virtual void insert_rest(DocCursor& cursor, const std::string& source);

    // event handlers. Interface with platform dependent code
    virtual void on_mouse_move(Pixels x, Pixels y, unsigned flags);
    virtual void on_mouse_button_down(Pixels x, Pixels y, unsigned flags);
    virtual void on_mouse_button_up(Pixels x, Pixels y, unsigned flags);
    //virtual void on_init();
    //virtual void on_resize(Pixels x, Pixels y);
    //virtual void on_idle();
    //virtual void on_key(Pixels x, Pixels y, unsigned key, unsigned flags);
    //virtual void on_ctrl_change();


};


//---------------------------------------------------------------------------------------
//A view to edit the score in full page
class EditInteractor : public Interactor
{
protected:

public:

    EditInteractor(LibraryScope& libraryScope, Document* pDoc); //, UserCommandExecuter* pExec);
    virtual ~EditInteractor();

};



}   //namespace lomse

#endif      //__LOMSE_INTERACTOR_H__

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

#include "lomse_interactor.h"

//#include "lomse_user_command.h"
//#include "lomse_command.h"
#include "lomse_compiler.h"
#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_basic.h"
#include <sstream>
using namespace std;
//#define TRT(a) (a)

namespace lomse
{

//=======================================================================================
// Interactor implementation
//=======================================================================================
Interactor::Interactor(LibraryScope& libraryScope, Document* pDoc)
                       //UserCommandExecuter* pExec)
    : m_libScope(libraryScope)
    , m_pDoc(pDoc)
    //, m_pExec(pExec)
{
    //DocumentScope* pDocScope = pDoc->get_scope();
    //m_pCompiler  = Injector::inject_LdpCompiler(m_libScope, *pDocScope);
}

//---------------------------------------------------------------------------------------
Interactor::~Interactor()
{
    //delete m_pCompiler;
}

////---------------------------------------------------------------------------------------
//void Interactor::insert_rest(DocCursor& cursor, const std::string& source)
//{
//    LdpElement* pElm = m_pCompiler->create_element(source);
//    CmdInsertElement cmd(TRT("Insert rest"), cursor, pElm, m_pCompiler);
//    m_pExec->execute(cmd);
//
//    //place cursor after inserted object
//    cursor.reset_and_point_to( pElm->get_id() );
//    cursor.move_next();
//
//    m_pDoc->notify_that_document_has_been_modified();
//}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_button_down(Pixels x, Pixels y, unsigned flags)
{
    //m_dx = x - m_vxOrg;
    //m_dy = y - m_vyOrg;
    //m_drag_flag = true;
}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_move(Pixels x, Pixels y, unsigned flags)
{
    //if(flags == 0)
    //{
    //    m_drag_flag = false;
    //}

    //if(m_drag_flag)
    //{
    //    m_vxOrg = x - m_dx;
    //    m_vyOrg = y - m_dy;
    //    
    //    m_transform.tx = double(m_vxOrg);
    //    m_transform.ty = double(m_vyOrg);

    //    m_pDoorway->force_redraw();
    //}
}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_button_up(Pixels x, Pixels y, unsigned flags)
{
    //m_drag_flag = false;
}



//=======================================================================================
// EditInteractor implementation
//=======================================================================================
EditInteractor::EditInteractor(LibraryScope& libraryScope, Document* pDoc)
                               //UserCommandExecuter* pExec)
    : Interactor(libraryScope, pDoc)    //, pExec)
{
}

//---------------------------------------------------------------------------------------
EditInteractor::~EditInteractor()
{
}



}  //namespace lomse

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#include "lomse_command.h"
#include "lomse_build_options.h"

#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_im_factory.h"


#include <sstream>
using namespace std;


namespace lomse
{

//=======================================================================================
// DocCommand
//=======================================================================================
void DocCommand::create_checkpoint(Document* pDoc)
{
    if (m_checkpoint.empty())
        m_checkpoint = pDoc->get_checkpoint_data();

    log_forensic_data();
}

//---------------------------------------------------------------------------------------
void DocCommand::undo_action(Document* pDoc, DocCursor* pCursor)
{
    //default implementation based on restoring back to saved checkpoint

//    //Default implementation: Restore previous state from LDP source code
//    //Returns true to indicate that the action has taken place, false otherwise.
//    //Returning false will indicate to the command processor that the action is
//    //not redoable and no change should be made to the command history.
//
//
//    //ask document to replace current score by the old one
//    pScore->ResetUndoMode();
    pDoc->from_checkpoint(m_checkpoint);
}

//---------------------------------------------------------------------------------------
void DocCommand::log_forensic_data()
{
    //save data for forensic analysis if a crash

    ofstream logger;
    logger.open("forensic_log.txt");
//    //log time stamp
//    logger <<
//    m_pForensic->Write( (wxDateTime::Now()).Format(_T("%Y/%m/%d %H:%M:%S")) + _T("\n") );
    logger << "Command class: "
           //<<  this->GetClassInfo()->GetClassName(),
           << ", Command name: '" << m_name << "'" << endl;
    logger << m_checkpoint << endl;
    logger.close();
}

//---------------------------------------------------------------------------------------
void DocCommand::set_command_name(const string& name, ImoObj* pImo)
{
    m_name = name;
    int type = pImo->get_obj_type();
    switch(type)
    {
        case k_imo_note:        m_name.append("note");          break;
        case k_imo_para:        m_name.append("paragraph");     break;
        case k_imo_rest:        m_name.append("rest");          break;
        default:
            m_name.append( ImoObj::get_name(type) );
    }
}

////=======================================================================================
//// DocCommandInsert
////=======================================================================================
//DocCommandInsert::DocCommandInsert(DocIterator& it,ImoObj* pNewObj)
//    : DocCmdComposite(it, pNewObj, NULL)
//{
//}
//
////---------------------------------------------------------------------------------------
//DocCommandInsert::~DocCommandInsert()
//{
//    if (!m_applied)
//        delete m_added;
//}
//
////---------------------------------------------------------------------------------------
//void DocCommandInsert::undo(Document* pDoc)
//{
//    (*m_itInserted)->reset_modified();
//    pDoc->remove(m_itInserted);
//    m_applied = false;
//}
//
////---------------------------------------------------------------------------------------
//void DocCommandInsert::redo(Document* pDoc)
//{
//    m_itInserted = pDoc->insert(m_position, m_added);
//    (*m_itInserted)->set_modified();
//    m_applied = true;
//}


////=======================================================================================
//// DocCommandPushBack
////=======================================================================================
//DocCommandPushBack::DocCommandPushBack(Document::iterator& it, LdpElement* added)
//    : DocCmdComposite(it, added, NULL)
//{
//}
//
////---------------------------------------------------------------------------------------
//DocCommandPushBack::~DocCommandPushBack()
//{
//    if (!m_applied)
//        delete m_added;
//}
//
////---------------------------------------------------------------------------------------
//void DocCommandPushBack::undo(Document* pDoc)
//{
//    (*m_position)->reset_modified();
//    pDoc->remove_last_param(m_position);
//    m_applied = false;
//}
//
////---------------------------------------------------------------------------------------
//void DocCommandPushBack::redo(Document* pDoc)
//{
//    pDoc->add_param(m_position, m_added);
//    (*m_position)->set_modified();
//    m_applied = true;
//}
//
//
////=======================================================================================
//// DocCommandRemove
////=======================================================================================
//DocCommandRemove::DocCommandRemove(Document::iterator& it)
//    : DocCmdComposite(it, NULL, *it)
//{
//    m_parent = (*it)->get_parent();
//    m_nextSibling = (*it)->get_next_sibling();
//}
//
////---------------------------------------------------------------------------------------
//DocCommandRemove::~DocCommandRemove()
//{
//    if (m_applied)
//        delete m_removed;
//}
//
////---------------------------------------------------------------------------------------
//void DocCommandRemove::undo(Document* pDoc)
//{
//    m_parent->reset_modified();
//    if (!m_nextSibling)
//    {
//        Document::iterator it(m_parent);
//        pDoc->add_param(it, m_removed);
//    }
//    else
//    {
//        Document::iterator it(m_nextSibling);
//        pDoc->insert(it, m_removed);
//    }
//    m_applied = false;
//}
//
////---------------------------------------------------------------------------------------
//void DocCommandRemove::redo(Document* pDoc)
//{
//    pDoc->remove(m_position);
//    m_parent->set_modified();
//    m_applied = true;
//}


//=======================================================================================
// DocCommandExecuter
//=======================================================================================
DocCommandExecuter::DocCommandExecuter(Document* target)
    : m_pDoc(target)
    , m_pCursor(NULL)
{
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::execute(DocCursor* pCursor, DocCommand* pCmd)
{
    m_pCursor = pCursor;
    m_stack.push(pCmd);
    pCmd->perform_action(m_pDoc, m_pCursor);
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::undo()
{
    DocCommand* cmd = m_stack.pop();
    cmd->undo_action(m_pDoc, m_pCursor);
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::redo()
{
    DocCommand* cmd = m_stack.undo_pop();
    cmd->perform_action(m_pDoc, m_pCursor);
}




//=======================================================================================
// CmdCursor implementation
//=======================================================================================
CmdCursor::CmdCursor(int cmd, ImoId id)
    : DocCmdSimple()
    , m_operation(cmd)
    , m_targetId(id)
{
}

//---------------------------------------------------------------------------------------
void CmdCursor::perform_action(Document* pDoc, DocCursor* pCursor)
{
    m_curState = pCursor->get_state();
    switch(m_operation)
    {
        case k_move_next:
            m_name = "Cursor: move next";
            pCursor->move_next();
            break;
        case k_move_prev:
            m_name = "Cursor: move prev";
            pCursor->move_prev();
            break;
        case k_enter:
            m_name = "Cursor: enter element";
            pCursor->enter_element();
            break;
        case k_exit:
            m_name = "Cursor: exit element";
            pCursor->exit_element();
            break;
        case k_point_to:
            m_name = "Cursor: point to";
            pCursor->point_to(m_targetId);
            break;
        case k_refresh:
            m_name = "Cursor: refresh";
            pCursor->update_after_deletion();
            break;
        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void CmdCursor::undo_action(Document* pDoc, DocCursor* pCursor)
{
    pCursor->restore_state(m_curState);
}


//=======================================================================================
// CmdDeleteBlockLevelObj implementation
//=======================================================================================
CmdDeleteBlockLevelObj::CmdDeleteBlockLevelObj()
    : DocCmdSimple()
{
}

//---------------------------------------------------------------------------------------
void CmdDeleteBlockLevelObj::perform_action(Document* pDoc, DocCursor* pCursor)
{
    create_checkpoint(pDoc);

    ImoBlockLevelObj* pImo = dynamic_cast<ImoBlockLevelObj*>( **pCursor );
    if (pImo)
    {
        set_command_name("Delete ", pImo);
        ImoDocument* pImoDoc = pDoc->get_imodoc();
        pImoDoc->delete_block_level_obj(pImo);
    }
}


//=======================================================================================
// CmdDeleteStaffObj implementation
//=======================================================================================
CmdDeleteStaffObj::CmdDeleteStaffObj()
    : DocCmdSimple()
{
}

//---------------------------------------------------------------------------------------
void CmdDeleteStaffObj::perform_action(Document* pDoc, DocCursor* pCursor)
{
    create_checkpoint(pDoc);

    ImoStaffObj* pImo = dynamic_cast<ImoStaffObj*>( pCursor->get_pointee() );
    if (pImo)
    {
        set_command_name("Delete ", pImo);
        ImoInstrument* pInstr = pImo->get_instrument();
        pInstr->delete_staffobj(pImo);

        ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_top_object() );
        pScore->close();
    }
}


//=======================================================================================
// CmdInsertBlockLevelObj implementation
//=======================================================================================
CmdInsertBlockLevelObj::CmdInsertBlockLevelObj(int type)
    : DocCmdSimple()
    , m_insertedId(k_no_imoid)
    , m_type(type)
{
}

//---------------------------------------------------------------------------------------
void CmdInsertBlockLevelObj::undo_action(Document* pDoc, DocCursor* pCursor)
{
    if (m_insertedId != k_no_imoid)
    {
        ImoDocument* pImoDoc = pDoc->get_imodoc();
        ImoObj* pImo = pDoc->get_pointer_to_imo(m_insertedId);
        TreeNode<ImoObj>::iterator it(pImo);
        pImoDoc->erase(it);
        m_insertedId = k_no_imoid;
    }
}

//---------------------------------------------------------------------------------------
void CmdInsertBlockLevelObj::perform_action(Document* pDoc, DocCursor* pCursor)
{
    ImoBlockLevelObj* pImo =
        static_cast<ImoBlockLevelObj*>( ImFactory::inject(m_type, pDoc) );
    set_command_name("Insert ", pImo);
    ImoDocument* pImoDoc = pDoc->get_imodoc();
    ImoBlockLevelObj* pAt = dynamic_cast<ImoBlockLevelObj*>( pCursor->get_pointee() );
    pImoDoc->insert_block_level_obj(pAt, pImo);
    m_insertedId = pImo->get_id();
}


////=======================================================================================
//// CmdInsertStaffObj implementation
////=======================================================================================
//CmdInsertStaffObj::CmdInsertStaffObj(DocCursor& cursor, int type)
//    : DocCmdSimple()
//    , m_cursorId( cursor.get_pointee_id() )
//    , m_insertedId(k_no_imoid)
//    , m_type(type)
//{
//    m_name = "Insert ";
////    switch(type)
////    {
////        case k_imo_para:        m_name.append("paragraph");     break;
////        default:
////            m_name.append( ImoObj::get_name(type) );
////    }
//}
//
////---------------------------------------------------------------------------------------
//void CmdInsertStaffObj::undo_action(Document* pDoc, DocCursor* pCursor)
//{
////    if (m_insertedId != k_no_imoid)
////    {
////        ImoDocument* pImoDoc = pDoc->get_imodoc();
////        ImoObj* pImo = pDoc->get_pointer_to_imo(m_insertedId);
////        TreeNode<ImoObj>::iterator it(pImo);
////        pImoDoc->erase(it);
////        m_insertedId = k_no_imoid;
////    }
//}
//
////---------------------------------------------------------------------------------------
//void CmdInsertStaffObj::perform_action(Document* pDoc, DocCursor* pCursor)
//{
////    ImoBlockLevelObj* pImo =
////        static_cast<ImoBlockLevelObj*>( ImFactory::inject(m_type, pDoc) );
////    ImoDocument* pImoDoc = pDoc->get_imodoc();
////    ImoBlockLevelObj* pAt = NULL;
////    if (m_cursorId != k_no_imoid)
////    {
////        pAt = dynamic_cast<ImoBlockLevelObj*>( pDoc->get_pointer_to_imo(m_cursorId) );
////    }
////    pImoDoc->insert_block_level_obj(pAt, pImo);
////    m_insertedId = pImo->get_id();
//}


}  //namespace lomse

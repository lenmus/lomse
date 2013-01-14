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
void DocCommand::undo_action(Document* pDoc)
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
{
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::execute(DocCommand* pCmd)
{
    m_stack.push(pCmd);
    pCmd->perform_action(m_pDoc);
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::undo()
{
    DocCommand* cmd = m_stack.pop();
    cmd->undo_action(m_pDoc);
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::redo()
{
    DocCommand* cmd = m_stack.undo_pop();
    cmd->perform_action(m_pDoc);
}



//=======================================================================================
// CmdDeleteStaffObj implementation
//=======================================================================================
CmdDeleteStaffObj::CmdDeleteStaffObj(DocCursor& cursor)
    : DocCmdSimple()
    , m_idScore( cursor.get_top_id() )
    , m_idStaffobj( cursor.get_inner_id() )
{
    ImoObj* pImo = *cursor;
    m_name = "Delete ";
    int type = pImo->get_obj_type();
    switch(type)
    {
        case k_imo_note:        m_name.append("note");     break;
        case k_imo_rest:        m_name.append("rest");     break;
        default:
            m_name.append( ImoObj::get_name(type) );
    }
}

//---------------------------------------------------------------------------------------
void CmdDeleteStaffObj::perform_action(Document* pDoc)
{
    create_checkpoint(pDoc);

    ImoStaffObj* pImo = static_cast<ImoStaffObj*>( pDoc->get_pointer_to_imo(m_idStaffobj) );
    ImoInstrument* pInstr = pImo->get_instrument();
    pInstr->delete_staffobj(pImo);

    ImoScore* pScore = static_cast<ImoScore*>( pDoc->get_pointer_to_imo(m_idScore) );
    pScore->close();
}



//=======================================================================================
// CmdInsertBlockLevelObj implementation
//=======================================================================================
//CmdInsertBlockLevelObj::CmdInsertBlockLevelObj(const string& name, DocCursor& cursor,
//                                               ImoBlockLevelObj* pImo)
//    : DocCmdSimple()
//    , m_pImo(pImo)
//{
//    if (cursor.is_at_end_of_staff())
//    {
//        m_fPushBack = true;
//        m_it = cursor.get_musicData_for_current_instrument();
//        m_pGoBackElm = NULL;
//        m_pGoFwdElm = NULL;
//    }
//    else
//    {
//        m_fPushBack = false;
//        m_it = determine_source_insertion_point(cursor, pElm);
//        m_pGoBackElm = determine_if_go_back_needed(cursor, pElm);
//        m_pGoFwdElm = determine_if_go_fwd_needed(cursor, pElm);
//    }
//}

//---------------------------------------------------------------------------------------
CmdInsertBlockLevelObj::CmdInsertBlockLevelObj(DocCursor& cursor, int type)
    : DocCmdSimple()
    , m_cursor(cursor)
    , m_pImo(NULL)
    , m_type(type)
{
    m_name = "Insert ";
    switch(type)
    {
        case k_imo_para:        m_name.append("paragraph");     break;
        default:
            m_name.append( ImoObj::get_name(type) );
    }
}

//---------------------------------------------------------------------------------------
void CmdInsertBlockLevelObj::undo_action(Document* pDoc)
{
    ImoDocument* pImoDoc = pDoc->get_imodoc();
    TreeNode<ImoObj>::iterator it(m_pImo);
    pImoDoc->erase(it);
    m_pImo = NULL;
}

//---------------------------------------------------------------------------------------
void CmdInsertBlockLevelObj::perform_action(Document* pDoc)
{
    m_pImo = static_cast<ImoBlockLevelObj*>( ImFactory::inject(m_type, pDoc) );
    ImoDocument* pImoDoc = pDoc->get_imodoc();
    ImoBlockLevelObj* pAt = dynamic_cast<ImoBlockLevelObj*>( *m_cursor );
    pImoDoc->insert_block_level_obj(pAt, m_pImo);

//    m_itInserted = pDoc->insert(m_position, m_added);
//    (*m_itInserted)->set_modified();
//    m_applied = true;

}


////---------------------------------------------------------------------------------------
//bool CmdInsertBlockLevelObj::do_actions(DocCommandExecuter* pExec)
//{
//    if (m_pGoBackElm)
//        execute_insert(pExec, m_it, m_pGoBackElm);
//    if (m_pGoFwdElm)
//        execute_insert(pExec, m_it, m_pGoFwdElm);
//    execute_insert(pExec, m_it, m_pImo);
//    return true;
//}
//
////---------------------------------------------------------------------------------------
//LdpElement* CmdInsertBlockLevelObj::determine_source_insertion_point(DocCursor& cursor, LdpElement* pElm)
//{
//    if (pElm->is_type(k_note) || pElm->is_type(k_rest))
//    {
//        ImNoteRest* pNR = dynamic_cast<ImNoteRest*>( (*cursor)->get_imobj() );
//        int nCursorVoice = pNR->get_voice();
//        ImNoteRest* pNewNR = dynamic_cast<ImNoteRest*>( pElm->get_imobj() );
//        int nNewVoice = pNewNR->get_voice();
//        if (nCursorVoice == nNewVoice)
//            return *cursor;
//        else
//        {
//            DocCursor cursor2(cursor);
//            //advance to barline or to end of staff
//            while (*cursor2 != NULL && (*cursor2)->get_type() != k_barline)
//                cursor2.move_next();
//            //if at end of staff, change command to push back
//            if (*cursor2 == NULL)
//            {
//                m_fPushBack = true;
//                return cursor2.get_musicData_for_current_instrument();
//            }
//            else
//                return *cursor2;
//        }
//    }
//    else
//        return *cursor;
//}
//
////---------------------------------------------------------------------------------------
//LdpElement* CmdInsertBlockLevelObj::determine_if_go_back_needed(DocCursor& cursor, LdpElement* pElm)
//{
//    if (pElm->is_type(k_note) || pElm->is_type(k_rest))
//    {
//        ImNoteRest* pNR = dynamic_cast<ImNoteRest*>( (*cursor)->get_imobj() );
//        int nCursorVoice = pNR->get_voice();
//        ImNoteRest* pNewNR = dynamic_cast<ImNoteRest*>( pElm->get_imobj() );
//        int nNewVoice = pNewNR->get_voice();
//        if (nCursorVoice == nNewVoice)
//            return NULL;
//        else
//        {
//            LdpElement* goBack = m_pCompiler->create_element("(goBack start)");
//            return goBack;
//        }
//    }
//    else
//    {
//        LdpElement* goBack = m_pCompiler->create_element("(goBack start)");
//        return goBack;
//    }
//}
//
////---------------------------------------------------------------------------------------
//LdpElement* CmdInsertBlockLevelObj::determine_if_go_fwd_needed(DocCursor& cursor, LdpElement* pElm)
//{
//    if (m_pGoBackElm && is_greater_time(cursor.time(), 0.0f))
//    {
//        stringstream s;
//        s << "(goFwd " << cursor.time() << ")";
//        LdpElement* goFwd = m_pCompiler->create_element(s.str());
//        return goFwd;
//    }
//    else
//        return NULL;
//}
//
////---------------------------------------------------------------------------------------
//void CmdInsertBlockLevelObj::execute_insert(DocCommandExecuter* pExec,
//                                      Document::iterator& it, LdpElement* pNewElm)
//{
//    if (m_fPushBack)
//        pExec->execute( new DocCommandPushBack(it, pNewElm) );
//    else
//        pExec->execute( new DocCommandInsert(it, pNewElm) );
//}
//

}  //namespace lomse

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
#include "lomse_logger.h"
#include "lomse_ldp_analyser.h"     //class Autobeamer


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
//    switch(type)
//    {
//        case k_imo_note:        m_name.append("note");          break;
//        case k_imo_para:        m_name.append("paragraph");     break;
//        case k_imo_rest:        m_name.append("rest");          break;
//        default:
            m_name.append( ImoObj::get_name(type) );
//    }
}


//=======================================================================================
// DocCommandExecuter
//=======================================================================================
DocCommandExecuter::DocCommandExecuter(Document* target)
    : m_pDoc(target)
{
}

//---------------------------------------------------------------------------------------
int DocCommandExecuter::execute(DocCursor* pCursor, DocCommand* pCmd)
{
    UndoElement* pUE = NULL;
    if (pCmd->is_reversible())
        pUE = LOMSE_NEW UndoElement(pCmd, pCursor->get_state());

    int result = pCmd->perform_action(m_pDoc, pCursor);
    m_error = pCmd->get_error();
    if ( result == k_success && pCmd->is_reversible())
    {
        m_stack.push( pUE );
        update_cursor(pCursor, pCmd);
    }
    else
        delete pUE;

    return result;
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::update_cursor(DocCursor* pCursor, DocCommand* pCmd)
{
    int policy = pCmd->get_cursor_update_policy();
    switch (policy)
    {
        case DocCommand::k_do_nothing:
            break;

        case DocCommand::k_update_after_insertion:
        {
            CmdInsertObj* cmd = static_cast<CmdInsertObj*>(pCmd);
            pCursor->update_after_insertion( cmd->last_inserted_id() );
            break;
        }

        case DocCommand::k_update_after_deletion:
            pCursor->update_after_deletion();
            break;

        default:
        {
            LOMSE_LOG_ERROR("Unknown cursor update policy.");
        }
    }
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::undo(DocCursor* pCursor)
{
    UndoElement* pUE = m_stack.pop();
    if (pUE)
    {
        DocCommand* cmd = pUE->pCmd;
        cmd->undo_action(m_pDoc, pCursor);

        pCursor->restore_state( pUE->cursorState );
    }
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::redo(DocCursor* pCursor)
{
    UndoElement* pUE = m_stack.undo_pop();
    if (pUE)
    {
        pCursor->restore_state( pUE->cursorState );
        DocCommand* cmd = pUE->pCmd;
        cmd->perform_action(m_pDoc, pCursor);

        update_cursor(pCursor, cmd);
    }
}

////---------------------------------------------------------------------------------------
//void DocCommandExecuter::replay(DocCursor* pCursor)
//{
//    UndoElement* pUE = m_stack.undo_pop();  <-- replace by m_recording stack
//    if (pUE)
//    {
//        DocCommand* cmd = pUE->pCmd;
//        cmd->perform_action(m_pDoc, pCursor);
//
//        update_cursor(pCursor, cmd->get_cursor_update_policy());
//    }
//}


//=======================================================================================
// CmdAddTie implementation
//=======================================================================================
CmdAddTie::CmdAddTie(ImoId start, ImoId end)
    : DocCmdSimple()
    , m_startId(start)
    , m_endId(end)
    , m_tieId(k_no_imoid)
{
    m_flags = k_recordable | k_reversible;
    m_name = "Add tie";
}

//---------------------------------------------------------------------------------------
int CmdAddTie::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: direct undo, as it only implies deleting the tuplet

    stringstream msg;
    ImoNote* pStart = static_cast<ImoNote*>(
                                    pDoc->get_pointer_to_imo(m_startId) );
    ImoNote* pEnd = static_cast<ImoNote*>(
                                    pDoc->get_pointer_to_imo(m_endId) );
    ImoTie* pTie = pDoc->tie_notes(pStart, pEnd, msg);
    if (pTie)
        m_tieId = pTie->get_id();
    m_error = msg.str();
    return (pTie != NULL ? k_success : k_failure);
}

//---------------------------------------------------------------------------------------
void CmdAddTie::undo_action(Document* pDoc, DocCursor* pCursor)
{
    if (m_tieId != k_no_imoid)
    {
        ImoTie* pTie = static_cast<ImoTie*>(
                                pDoc->get_pointer_to_imo(m_tieId) );
        pDoc->delete_relation(pTie);
        m_tieId = k_no_imoid;
    }
}


//=======================================================================================
// CmdAddTuplet implementation
//=======================================================================================
CmdAddTuplet::CmdAddTuplet(ImoId startNR, ImoId endNR, const string& src)
    : DocCmdSimple()
    , m_startId(startNR)
    , m_endId(endNR)
    , m_tupletId(k_no_imoid)
    , m_source(src)
{
    m_flags = k_recordable | k_reversible;
    m_name = "Add tuplet";
}

//---------------------------------------------------------------------------------------
int CmdAddTuplet::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: direct undo, as it only implies deleting the tuplet

    stringstream msg;
    ImoNoteRest* pStart = static_cast<ImoNoteRest*>(
                                        pDoc->get_pointer_to_imo(m_startId) );
    ImoNoteRest* pEnd = static_cast<ImoNoteRest*>(
                                        pDoc->get_pointer_to_imo(m_endId) );
    ImoTuplet* pTuplet = pDoc->add_tuplet(pStart, pEnd, m_source, msg);
    if (pTuplet)
        m_tupletId = pTuplet->get_id();
    m_error = msg.str();
    return (pTuplet != NULL ? k_success : k_failure);
}

//---------------------------------------------------------------------------------------
void CmdAddTuplet::undo_action(Document* pDoc, DocCursor* pCursor)
{
    if (m_tupletId != k_no_imoid)
    {
        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(
                                pDoc->get_pointer_to_imo(m_tupletId) );
        pDoc->delete_relation(pTuplet);
        m_tupletId = k_no_imoid;
    }
}


//=======================================================================================
// CmdBreakBeam implementation
//=======================================================================================
CmdBreakBeam::CmdBreakBeam(ImoNoteRest* pBeforeNR)
    : DocCmdSimple()
    , m_beforeId( pBeforeNR->get_id() )
{
    m_flags = k_recordable | k_reversible;
    m_name = "Break beam";
}

//---------------------------------------------------------------------------------------
int CmdBreakBeam::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: Checkpoint.
    //TODO: Posible optimization: partial checkpoint: save only source code for beam
    create_checkpoint(pDoc);

    //it is previously verified that pBeforeNR is beamed and it is not the first one
    //of the beam

    //get the beam
    ImoNoteRest* pBeforeNR = static_cast<ImoNoteRest*>(
                                pDoc->get_pointer_to_imo(m_beforeId) );
    ImoBeam* pBeam = pBeforeNR->get_beam();

    //save pointers to the note/rests before break point
    int nNotesBefore = 0;
    list<ImoNoteRest*> notesBefore;
    list< pair<ImoStaffObj*, ImoRelDataObj*> >& objs = pBeam->get_related_objects();
    list< pair<ImoStaffObj*, ImoRelDataObj*> >::const_iterator it;
    it = objs.begin();
    ImoNoteRest* pNR = static_cast<ImoNoteRest*>( (*it).first );
    ImoNoteRest* pPrevNR = NULL;
    ++it;
    while (pNR && pNR != pBeforeNR && it != objs.end())
    {
        notesBefore.push_back( pNR );

        pPrevNR = pNR;
        ++nNotesBefore;
        pNR = static_cast<ImoNoteRest*>( (*it).first );
        ++it;
    }

    //pBeforeNR must be found and it must not be the first one in the beam
    if (pNR == NULL || pPrevNR == NULL || nNotesBefore == 0)
        return k_failure;

    pPrevNR->set_dirty(true);
    pBeforeNR->set_dirty(true);

    int nNotesAfter = int(objs.size()) - nNotesBefore;

    //four cases:
    //  a) two single notes         (nNotesBefore == 1 && nNotesAfter == 1)
    //  b) single note + new beam   (nNotesBefore == 1 && nNotesAfter > 1)
    //  c) new beam + new beam      (nNotesBefore > 1 && nNotesAfter > 1)
    //  d) new beam + single note   (nNotesBefore > 1 && nNotesAfter == 1)

    //Case a) two single notes
    if (nNotesBefore == 1 && nNotesAfter == 1)
    {
        //just remove the beam
        pDoc->delete_relation(pBeam);
        return k_success;
    }

    //case b) single note + new beam
    if (nNotesBefore == 1 && nNotesAfter > 1)
    {
        //remove first note from beam
        pPrevNR->remove_from_relation(pBeam);
        AutoBeamer autobeamer(pBeam);
        autobeamer.do_autobeam();
        return k_success;
    }

    //case c) new beam + new beam
    if (nNotesBefore > 1 && nNotesAfter > 1)
    {
        //split the beam. Create a new beam for first group and keep the existing one
        //for the second group

        //remove 'before' notes from existing beam
        list<ImoNoteRest*>::iterator it = notesBefore.begin();
        for (it = notesBefore.begin(); it != notesBefore.end(); ++it)
            (*it)->remove_from_relation(pBeam);

        //create a new beam for the removed notes
        pDoc->add_beam(notesBefore);

        //adjust the old beam
        AutoBeamer autobeamer(pBeam);
        autobeamer.do_autobeam();

        return k_success;
    }

    //case d) new beam + single note
    if (nNotesBefore > 1 && nNotesAfter == 1)
    {
        //remove last note from beam
        pBeforeNR->remove_from_relation(pBeam);
        AutoBeamer autobeamer(pBeam);
        autobeamer.do_autobeam();
        return k_success;
    }

    return k_failure;
}


//=======================================================================================
// CmdChangeDots implementation
//=======================================================================================
CmdChangeDots::CmdChangeDots(const list<ImoId>& noteRests, int dots)
    : DocCmdSimple()
    , m_dots(dots)
{
    m_flags = k_recordable | k_reversible;
    m_name = "Change dots";

    //save affected note/rests
    list<ImoId>::const_iterator it;
    for (it = noteRests.begin(); it != noteRests.end(); ++it)
        m_noteRests.push_back(*it);
}

//---------------------------------------------------------------------------------------
int CmdChangeDots::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: direct undo, as it only implies restoring dots

    bool fSaveDots = (m_oldDots.size() == 0);
    list<ImoId>::iterator it;
    for (it = m_noteRests.begin(); it != m_noteRests.end(); ++it)
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>( pDoc->get_pointer_to_imo(*it) );
        if (fSaveDots)
            m_oldDots.push_back( pNR->get_dots() );
        pNR->set_dots(m_dots);
        pNR->set_dirty(true);
    }
    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdChangeDots::undo_action(Document* pDoc, DocCursor* pCursor)
{
    list<ImoId>::iterator itNR;
    list<int>::iterator itD = m_oldDots.begin();
    for (itNR = m_noteRests.begin(); itNR != m_noteRests.end(); ++itNR, ++itD)
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>( pDoc->get_pointer_to_imo(*itNR) );
        pNR->set_dots(*itD);
    }
}

//=======================================================================================
// CmdCursor implementation
//=======================================================================================
CmdCursor::CmdCursor(int cmd, ImoId id)
    : DocCmdSimple()
    , m_operation(cmd)
    , m_targetId(id)
{
    m_flags = k_recordable;
}

//---------------------------------------------------------------------------------------
int CmdCursor::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: this command is not reversible

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
    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdCursor::undo_action(Document* pDoc, DocCursor* pCursor)
{
    //CmdCursor is not reversible. Nothing to do here.
    //pCursor->restore_state(m_curState);
}


//=======================================================================================
// CmdDeleteBlockLevelObj implementation
//=======================================================================================
CmdDeleteBlockLevelObj::CmdDeleteBlockLevelObj()
    : DocCmdSimple()
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdDeleteBlockLevelObj::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: Checkpoint.
    //TODO: Posible optimization: partial checkpoint: save only source code for deleted
    //block

    create_checkpoint(pDoc);

    ImoBlockLevelObj* pImo = dynamic_cast<ImoBlockLevelObj*>( **pCursor );
    if (pImo)
    {
        set_command_name("Delete ", pImo);
        ImoDocument* pImoDoc = pDoc->get_imodoc();
        pImoDoc->delete_block_level_obj(pImo);
        return k_success;
    }
    return k_failure;
}


//=======================================================================================
// CmdDeleteRelation implementation
//=======================================================================================
CmdDeleteRelation::CmdDeleteRelation(ImoRelObj* pRO)
    : DocCmdSimple()
{
    m_flags = k_recordable | k_reversible;
    m_relId = pRO->get_id();
    m_name = "Delete " + pRO->get_name();
}

//---------------------------------------------------------------------------------------
int CmdDeleteRelation::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: checkpoint, because the relation could have some attributes
    //modified (color, user positioned, ...).
    //TODO: OPTIMIZATION direct undo via partial checkpoint, that is, only relation
    //source code
    create_checkpoint(pDoc);

    ImoRelObj* pRO = static_cast<ImoRelObj*>( pDoc->get_pointer_to_imo(m_relId) );
    pDoc->delete_relation(pRO);
    return k_success;
}


//=======================================================================================
// CmdDeleteStaffObj implementation
//=======================================================================================
CmdDeleteStaffObj::CmdDeleteStaffObj()
    : DocCmdSimple()
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdDeleteStaffObj::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: Deleted objects could have relations with other objects (for
    //instance, beamed groups, ligatures, etc.). Therefore, an 'undo' operation
    //would require to identify and rebuild these relations. As this can be complex,
    //it is simpler to use checkpoints.
    create_checkpoint(pDoc);


    ImoStaffObj* pImo = dynamic_cast<ImoStaffObj*>( pCursor->get_pointee() );
    if (pImo)
    {
        set_command_name("Delete ", pImo);

        //get and save relations
        vector<ImoId> relIds;
        ImoRelations* pRels = pImo->get_relations();
        if (pRels)
        {
            list<ImoRelObj*>& relations = pRels->get_relations();
            if (relations.size() > 0)
            {
                list<ImoRelObj*>::iterator it;
                for (it = relations.begin(); it != relations.end(); ++it)
                {
                    relIds.push_back( (*it)->get_id() );
                }
            }
        }

        //delete object
        ImoInstrument* pInstr = pImo->get_instrument();
        pInstr->delete_staffobj(pImo);

        //ask relations to reorganize themselves
        if (relIds.size() > 0)
        {
            vector<ImoId>::iterator itV;
            for (itV = relIds.begin(); itV != relIds.end(); ++itV)
            {
                ImoRelObj* pRO = static_cast<ImoRelObj*>( pDoc->get_pointer_to_imo(*itV) );
                if (pRO)
                    pRO->reorganize_after_object_deletion();
            }
        }

        //rebuild StaffObjs collection
        ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_top_object() );
        pScore->close();

        return k_success;
    }
    return k_failure;
}


//=======================================================================================
// CmdInsertObj implementation
//=======================================================================================
void CmdInsertObj::remove_object(Document* pDoc, ImoId id)
{
    if (id != k_no_imoid)
    {
        //get object to remove
        ImoStaffObj* pImo = static_cast<ImoStaffObj*>( pDoc->get_pointer_to_imo(id) );

        //remove object from imo tree
        ImoDocument* pImoDoc = pDoc->get_imodoc();
        TreeNode<ImoObj>::iterator it(pImo);
        pImoDoc->erase(it);
        pImoDoc->set_dirty(true);
        delete pImo;
    }
}

//---------------------------------------------------------------------------------------
int CmdInsertObj::validate_source()
{
    //TODO: refactor. Source code exploration must be done by LdpParser. It should return
    //the number of top level elements and a flag for signaling no parenthesis missmatch.
    //Here, we should only ask parser for a quick check and validate num of top level
    //elements

    //starts and ends with parenthesis
    size_t size = m_source.size();
    if (size < 3 || m_source.at(0) != '(' || m_source.at(size-1) != ')')
    {
        m_error = "Missing start or end parenthesis";
        return k_failure;
    }

    int open = 1;
    bool fPerhapsMoreThanOneElement = false;
    for (size_t i=1; i < size; ++i)
    {
        if (m_source.at(i) == '(')
        {
            if (open == 0)
                fPerhapsMoreThanOneElement = true;
            open++;
        }
        else if (m_source.at(i) == ')')
            open--;
        //TODO: skip parenthesis inside strings!
    }
    if (open != 0)
    {
        m_error = "Parenthesis missmatch";
        return k_failure;
    }
    if (fPerhapsMoreThanOneElement)
    {
        m_error = "More than one LDP elements";
        return k_failure;
    }

    return k_success;
}



//=======================================================================================
// CmdInsertBlockLevelObj implementation
//=======================================================================================
CmdInsertBlockLevelObj::CmdInsertBlockLevelObj(int type)
    : CmdInsertObj()
    , m_blockType(type)
    , m_fFromSource(false)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdInsertBlockLevelObj::CmdInsertBlockLevelObj(const string& source)
    : CmdInsertObj()
    , m_blockType(k_imo_block_level_obj)
    , m_fFromSource(true)
{
    m_source = source;
    m_flags = k_recordable | k_reversible;
    m_name = "Insert block";
}

//---------------------------------------------------------------------------------------
void CmdInsertBlockLevelObj::undo_action(Document* pDoc, DocCursor* pCursor)
{
    if (m_lastInsertedId != k_no_imoid)
    {
        ImoDocument* pImoDoc = pDoc->get_imodoc();
        ImoBlockLevelObj* pImo = static_cast<ImoBlockLevelObj*>(
                                            pDoc->get_pointer_to_imo(m_lastInsertedId) );
        pImoDoc->delete_block_level_obj(pImo);
        m_lastInsertedId = k_no_imoid;
    }
}

//---------------------------------------------------------------------------------------
int CmdInsertBlockLevelObj::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: direct undo, as it only implies to delete the inserted object

    if (m_fFromSource)
        perform_action_from_source(pDoc, pCursor);
    else
        perform_action_from_type(pDoc, pCursor);
    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdInsertBlockLevelObj::perform_action_from_type(Document* pDoc, DocCursor* pCursor)
{
    ImoBlockLevelObj* pImo =
        static_cast<ImoBlockLevelObj*>( ImFactory::inject(m_blockType, pDoc) );
    set_command_name("Insert ", pImo);
    ImoDocument* pImoDoc = pDoc->get_imodoc();
    ImoBlockLevelObj* pAt = dynamic_cast<ImoBlockLevelObj*>( pCursor->get_pointee() );
    pImoDoc->insert_block_level_obj(pAt, pImo);
    m_lastInsertedId = pImo->get_id();
}

//---------------------------------------------------------------------------------------
void CmdInsertBlockLevelObj::perform_action_from_source(Document* pDoc, DocCursor* pCursor)
{
    //create object
    ImoObj* pImo = pDoc->create_object_from_lmd(m_source);

    if (pImo && pImo->is_block_level_obj())
    {
        //insert the created subtree at desired point
        ImoBlockLevelObj* pAt = dynamic_cast<ImoBlockLevelObj*>( pCursor->get_pointee() );
        ImoDocument* pImoDoc = pDoc->get_imodoc();
        pImoDoc->insert_block_level_obj(pAt, static_cast<ImoBlockLevelObj*>(pImo));
        m_lastInsertedId = pImo->get_id();
    }
    else
    {
        delete pImo;
    }
}



//=======================================================================================
// CmdInsertStaffObj implementation
//=======================================================================================
CmdInsertStaffObj::CmdInsertStaffObj(const string& source)
    : CmdInsertObj()
{
    m_source = source;
    m_flags = k_recordable | k_reversible;
    m_name = "Insert ";
}

//---------------------------------------------------------------------------------------
void CmdInsertStaffObj::undo_action(Document* pDoc, DocCursor* pCursor)
{
    if (m_lastInsertedId != k_no_imoid)
    {
        remove_object(pDoc, m_lastInsertedId);
        //TODO: set document modified

        //update ColStaffObjs
        ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_top_object() );
        pScore->close();
    }
}

//---------------------------------------------------------------------------------------
int CmdInsertStaffObj::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: direct undo, as it only implies to delete the inserted object

    if (validate_source() != k_success)
        return k_failure;

    //get instrument that will be modified
    DocCursorState state = pCursor->get_state();
    SpElementCursorState elmState = state.get_delegate_state();
    ScoreCursorState* pState = static_cast<ScoreCursorState*>( elmState.get() );
    ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_top_object() );
    ImoInstrument* pInstr = pScore->get_instrument( pState->instrument() );

    //create and insert object
    if (pInstr)
    {
        //insert the created object at desired point
        stringstream errormsg;
        ImoStaffObj* pAt = dynamic_cast<ImoStaffObj*>( pCursor->get_pointee() );
        ImoStaffObj* pImo = pInstr->insert_staffobj_at(pAt, m_source, errormsg);
        if (pImo)
        {
            if (m_lastInsertedId == k_no_imoid)
            {
                m_lastInsertedId = pImo->get_id();

                //inject id in source code
                size_t i = m_source.find(' ');
                if (i == string::npos)
                    i = m_source.find(')');

                stringstream source;
                source << m_source.substr(0, i) << "#" << m_lastInsertedId
                       << m_source.substr(i);
                m_source = source.str();
            }

            //update ColStaffObjs table
            pScore->close();

            //assign name to this command
            m_name.append( pImo->get_name() );

            return k_success;
        }
        else
        {
            m_error = errormsg.str();
            return k_failure;
        }
    }
    return k_failure;
}


//=======================================================================================
// CmdInsertManyStaffObjs implementation
//=======================================================================================
CmdInsertManyStaffObjs::CmdInsertManyStaffObjs(const string& source,  const string& name)
    : CmdInsertObj()
    , m_fSaved(false)
{
    m_source = source;
    m_flags = k_recordable | k_reversible;
    m_name = name;
}

//---------------------------------------------------------------------------------------
int CmdInsertManyStaffObjs::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: The inserted objects can create relations with other objects (for
    //instance, beamed groups, ligatures, etc.). Therefore, an 'undo' operation
    //would require to identify and remove these relations before removing the
    //staff objects. As this can be complex, it is simpler to use checkpoints.
    create_checkpoint(pDoc);

    //get instrument that will be modified
    DocCursorState state = pCursor->get_state();
    SpElementCursorState elmState = state.get_delegate_state();
    ScoreCursorState* pState = static_cast<ScoreCursorState*>( elmState.get() );
    ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_top_object() );
    ImoInstrument* pInstr = pScore->get_instrument( pState->instrument() );

    //create and insert objects
    if (pInstr)
    {
        //insert the created object at desired point
        stringstream errormsg;
        ImoStaffObj* pAt = dynamic_cast<ImoStaffObj*>( pCursor->get_pointee() );
        list<ImoStaffObj*> objects = pInstr->insert_staff_objects_at(pAt, m_source, errormsg);
        if (objects.size() > 0)
        {
            pScore->close();        //update ColStaffObjs table
            save_source_code_with_ids(pDoc, objects);
            m_lastInsertedId = objects.back()->get_id();
            objects.clear();
            return k_success;
        }
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
void CmdInsertManyStaffObjs::save_source_code_with_ids(Document* pDoc,
                                                       const list<ImoStaffObj*>& objects)
{
    //TODO: Remove this method
    //This is not necessary. As undo/redo is based on checkpoints, the undo operation
    //rebuilds the document and resets ImoId counter. Therefore, redo operation reassigns
    //*the same* ids, without the need of saving them in this command source code.

    if (!m_fSaved)
    {
        //generate source code with ids.

        m_fSaved = true;
    }
}


//=======================================================================================
// CmdJoinBeam implementation
//=======================================================================================
CmdJoinBeam::CmdJoinBeam(const list<ImoId>& notes)
    : DocCmdSimple()
{
    m_flags = k_recordable | k_reversible;
    m_name = "Join beam";

    //save note/rests Ids
    list<ImoId>::const_iterator it;
    for (it = notes.begin(); it != notes.end(); ++it)
        m_notesId.push_back(*it);
}

//---------------------------------------------------------------------------------------
int CmdJoinBeam::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: Note/rests to be beamed could have beams. For now, it is
    //simpler to use a checkpoint
    create_checkpoint(pDoc);

    //get pointers to notes/rests
    list<ImoNoteRest*> notes;
    list<ImoId>::const_iterator it;
    for (it = m_notesId.begin(); it != m_notesId.end(); ++it)
        notes.push_back( static_cast<ImoNoteRest*>( pDoc->get_pointer_to_imo(*it) ) );

    //remove all existing beams
    list<ImoNoteRest*>::const_iterator itNR;
    for (itNR = notes.begin(); itNR != notes.end(); ++itNR)
    {
        if ((*itNR)->is_beamed())
            pDoc->delete_relation( (*itNR)->get_beam() );
    }

    //create the new beam
    pDoc->add_beam(notes);

    return k_success;
}


}  //namespace lomse

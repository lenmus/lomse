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
#include "lomse_model_builder.h"
#include "lomse_selections.h"
#include "lomse_score_meter.h"


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
int DocCommandExecuter::execute(DocCursor* pCursor, DocCommand* pCmd,
                                SelectionSet* pSelection)
{
    int result = k_success;
    if (!pCmd->is_target_set_in_constructor())
        result = pCmd->set_target(m_pDoc, pCursor, pSelection);

    if (result == k_success)
    {
        UndoElement* pUE = NULL;
        if (pCmd->is_reversible())
            pUE = LOMSE_NEW UndoElement(pCmd, pCursor->get_state());

        result = pCmd->perform_action(m_pDoc, pCursor);
        m_error = pCmd->get_error();
        if ( result == k_success && pCmd->is_reversible())
        {
            m_stack.push( pUE );
            update_cursor(pCursor, pCmd);
            m_pDoc->set_modified();
        }
        else
            delete pUE;
    }
    else
        m_error = "Command ignored. Can not set target";

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

        if (cmd->is_reversible())
            m_pDoc->reset_modified();
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

        if (cmd->is_reversible())
            m_pDoc->set_modified();
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
CmdAddTie::CmdAddTie(const string& name)
    : DocCmdSimple(name)
    , m_startId(k_no_imoid)
    , m_endId(k_no_imoid)
    , m_tieId(k_no_imoid)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdAddTie::set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection)
{
    ImoNote* pStartNote;
    ImoNote* pEndNote;
    if (pSelection && pSelection->is_valid_to_add_tie(&pStartNote, &pEndNote))
    {
        m_startId = pStartNote->get_id();
        m_endId = pEndNote->get_id();
        return k_success;
    }
    return k_failure;
}

////---------------------------------------------------------------------------------------
//int CmdAddTie::set_target(ImoId startId, ImoId endId)
//{
//    m_startId = startId;
//    m_endId = endId;
//    return k_success;
//}

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
    {
        pStart->set_dirty(true);
        pEnd->set_dirty(true);
        m_tieId = pTie->get_id();
    }

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
CmdAddTuplet::CmdAddTuplet(const string& src, const string& name)
    : DocCmdSimple(name)
    , m_startId(k_no_imoid)
    , m_endId(k_no_imoid)
    , m_tupletId(k_no_imoid)
    , m_source(src)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdAddTuplet::set_target(Document* pDoc, DocCursor* pCursor,
                             SelectionSet* pSelection)
{
    if (pSelection && !pSelection->empty())
    {
        ImoNoteRest* pStart = NULL;
        ImoNoteRest* pEnd = NULL;
        pSelection->get_start_end_note_rests(&pStart, &pEnd);
        if (pStart && pEnd)
        {
            m_startId = pStart->get_id();
            m_endId = pEnd->get_id();
            return k_success;
        }
    }
    return k_failure;
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
    {
        pStart->set_dirty(true);
        pEnd->set_dirty(true);
        m_tupletId = pTuplet->get_id();
    }

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
CmdBreakBeam::CmdBreakBeam(const string& name)
    : DocCmdSimple(name)
    , m_beforeId(k_no_imoid)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdBreakBeam::set_target(Document* pDoc, DocCursor* pCursor,
                             SelectionSet* pSelection)
{
    ImoNoteRest* pBeforeNR = dynamic_cast<ImoNoteRest*>( pCursor->get_pointee() );
    if (pBeforeNR)
    {
        m_beforeId = pBeforeNR->get_id();
        return k_success;
    }
    return k_failure;
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
// CmdChangeAccidentals implementation
//=======================================================================================
CmdChangeAccidentals::CmdChangeAccidentals(EAccidentals acc, const string& name)
    : DocCmdSimple(name)
    , m_acc(acc)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdChangeAccidentals::set_target(Document* pDoc, DocCursor* pCursor,
                                     SelectionSet* pSelection)
{
    if (pSelection && !pSelection->empty())
    {
        m_notes = pSelection->filter_notes_rests();
        return k_success;
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
int CmdChangeAccidentals::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: direct undo, as it only implies restoring accidentals
    //AWARE: changing accidentals in one note could affect many notes in the
    //same measure

    bool fSavePitch = (m_oldPitch.size() == 0);
    ImoScore* pScore = NULL;
    list<ImoId>::iterator it;
    for (it = m_notes.begin(); it != m_notes.end(); ++it)
    {
        ImoNote* pNote = static_cast<ImoNote*>( pDoc->get_pointer_to_imo(*it) );
        if (fSavePitch)
            m_oldPitch.push_back( pNote->get_fpitch() );
        pNote->set_notated_accidentals(m_acc);
        pNote->set_actual_accidentals(k_acc_not_computed);
        pNote->set_dirty(true);
        if (!pScore)
            pScore = pNote->get_score();
    }

    PitchAssigner tuner;
    tuner.assign_pitch(pScore);

    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdChangeAccidentals::undo_action(Document* pDoc, DocCursor* pCursor)
{
    //AWARE: restoring pitch of selected notes is not enough as changing accidentals
    // in one note could affect many notes in the same measure. Therefore, it is
    // necessary to recompute pitch of all notes

    ImoScore* pScore = NULL;
    list<ImoId>::iterator itN;
    list<FPitch>::iterator itP = m_oldPitch.begin();
    for (itN = m_notes.begin(); itN != m_notes.end(); ++itN, ++itP)
    {
        ImoNote* pNote = static_cast<ImoNote*>( pDoc->get_pointer_to_imo(*itN) );
        FPitch fp = *itP;
        pNote->set_notated_accidentals( fp.accidentals() );
        pNote->set_actual_accidentals( fp.num_accidentals() );
        pNote->set_dirty(true);
        if (!pScore)
            pScore = pNote->get_score();
    }

    PitchAssigner tuner;
    tuner.assign_pitch(pScore);
}


//=======================================================================================
// CmdChangeAttribute implementation
//=======================================================================================
CmdChangeAttribute::CmdChangeAttribute(EImoAttribute attrb,
                                       const string& value, const string& cmdName)
    : DocCmdSimple(cmdName)
    , m_targetId(k_no_imoid)
    , m_attrb(attrb)
    , m_newString(value)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdChangeAttribute::CmdChangeAttribute(EImoAttribute attrb,
                                       double value, const string& cmdName)
    : DocCmdSimple(cmdName)
    , m_targetId(k_no_imoid)
    , m_attrb(attrb)
    , m_newDouble(value)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdChangeAttribute::CmdChangeAttribute(EImoAttribute attrb,
                                       int value, const string& cmdName)
    : DocCmdSimple(cmdName)
    , m_targetId(k_no_imoid)
    , m_attrb(attrb)
    , m_newInt(value)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdChangeAttribute::CmdChangeAttribute(EImoAttribute attrb,
                                       Color value, const string& cmdName)
    : DocCmdSimple(cmdName)
    , m_targetId(k_no_imoid)
    , m_attrb(attrb)
    , m_newColor(value)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdChangeAttribute::CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb,
                                       const string& value, const string& cmdName)
    : DocCmdSimple(cmdName)
    , m_targetId(k_no_imoid)
    , m_attrb(attrb)
    , m_newString(value)
{
    m_flags = k_recordable | k_reversible | k_target_set_in_constructor;
    set_target(pImo);
}

//---------------------------------------------------------------------------------------
CmdChangeAttribute::CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb,
                                       double value, const string& cmdName)
    : DocCmdSimple(cmdName)
    , m_targetId(k_no_imoid)
    , m_attrb(attrb)
    , m_newDouble(value)
{
    m_flags = k_recordable | k_reversible | k_target_set_in_constructor;
    set_target(pImo);
}

//---------------------------------------------------------------------------------------
CmdChangeAttribute::CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb,
                                       int value, const string& cmdName)
    : DocCmdSimple(cmdName)
    , m_targetId(k_no_imoid)
    , m_attrb(attrb)
    , m_newInt(value)
{
    m_flags = k_recordable | k_reversible | k_target_set_in_constructor;
    set_target(pImo);
}

//---------------------------------------------------------------------------------------
CmdChangeAttribute::CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb,
                                       Color value, const string& cmdName)
    : DocCmdSimple(cmdName)
    , m_targetId(k_no_imoid)
    , m_attrb(attrb)
    , m_newColor(value)
{
    m_flags = k_recordable | k_reversible | k_target_set_in_constructor;
    set_target(pImo);
}

//---------------------------------------------------------------------------------------
int CmdChangeAttribute::set_target(Document* pDoc, DocCursor* pCursor,
                                   SelectionSet* pSelection)
{
    //TODO: treatment for selections
    ImoObj* pImo = pCursor->get_pointee();
    return set_target(pImo);
}

//---------------------------------------------------------------------------------------
int CmdChangeAttribute::set_target(ImoObj* pImo)
{
    if (pImo)
    {
        m_targetId = pImo->get_id();
        save_current_value(pImo);
        return k_success;
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
int CmdChangeAttribute::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: direct undo, as it only implies restoring attrib value

    ImoObj* pImo = pDoc->get_pointer_to_imo( m_targetId );
    switch (m_dataType)
    {
        case k_type_bool:
            pImo->set_bool_attribute(m_attrb, m_newInt);       break;
        case k_type_color:
            pImo->set_color_attribute(m_attrb, m_newColor);   break;
        case k_type_double:
            pImo->set_double_attribute(m_attrb, m_newDouble);  break;
        case k_type_int:
            pImo->set_int_attribute(m_attrb, m_newInt);        break;
        case k_type_string:
            pImo->set_string_attribute(m_attrb, m_newString);  break;
        default:
            return k_failure;
    }
    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdChangeAttribute::undo_action(Document* pDoc, DocCursor* pCursor)
{
    ImoObj* pImo = pDoc->get_pointer_to_imo( m_targetId );
    switch (m_dataType)
    {
        case k_type_bool:
            pImo->set_bool_attribute(m_attrb, m_oldInt);       break;
        case k_type_color:
            pImo->set_color_attribute(m_attrb, m_oldColor);   break;
        case k_type_double:
            pImo->set_double_attribute(m_attrb, m_oldDouble);  break;
        case k_type_int:
            pImo->set_int_attribute(m_attrb, m_oldInt);        break;
        case k_type_string:
            pImo->set_string_attribute(m_attrb, m_oldString);  break;
        default:
            return;
    }
}

//---------------------------------------------------------------------------------------
void CmdChangeAttribute::save_current_value(ImoObj* pImo)
{
    AttributesData data = AttributesTable::get_data_for(m_attrb);
    m_dataType = data.type;
    switch (m_dataType)
    {
        case k_type_bool:
            m_oldInt = pImo->get_bool_attribute(m_attrb);       break;
        case k_type_color:
            m_oldColor = pImo->get_color_attribute(m_attrb);    break;
        case k_type_double:
            m_oldDouble = pImo->get_double_attribute(m_attrb);  break;
        case k_type_int:
            m_oldInt = pImo->get_int_attribute(m_attrb);        break;
        case k_type_string:
            m_oldString = pImo->get_string_attribute(m_attrb);  break;
        default:
            ;
    }
}

//=======================================================================================
// CmdChangeDots implementation
//=======================================================================================
CmdChangeDots::CmdChangeDots(int dots, const string& name)
    : DocCmdSimple(name)
    , m_dots(dots)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdChangeDots::set_target(Document* pDoc, DocCursor* pCursor,
                              SelectionSet* pSelection)
{
    if (pSelection && !pSelection->empty())
    {
        m_noteRests = pSelection->filter_notes_rests();
        return k_success;
    }
    return k_failure;
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
        pNR->set_dirty(true);
    }
}

//=======================================================================================
// CmdCursor implementation
//=======================================================================================
CmdCursor::CmdCursor(int cmd, ImoId id, const string& name)
    : DocCmdSimple(name)
    , m_operation(cmd)
    , m_targetId(id)
{
    m_flags = k_recordable;
    if (name=="")
        set_default_name();
}

//---------------------------------------------------------------------------------------
CmdCursor::CmdCursor(int cmd, const string& name)
    : DocCmdSimple(name)
    , m_operation(cmd)
    , m_targetId(k_no_imoid)
{
    m_flags = k_recordable;
    if (name=="")
        set_default_name();
}

//---------------------------------------------------------------------------------------
CmdCursor::CmdCursor(DocCursorState& state, const string& name)
    : DocCmdSimple(name)
    , m_operation(k_to_state)
    , m_targetId(k_no_imoid)
    , m_targetState(state)
{
    m_flags = k_recordable;
    if (name=="")
        set_default_name();
}

//---------------------------------------------------------------------------------------
int CmdCursor::set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection)
{
    //CmdCursor does not need a target. It is set in constructor.
    return k_success;
}

//---------------------------------------------------------------------------------------
int CmdCursor::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: this command is not reversible

    m_curState = pCursor->get_state();
    switch(m_operation)
    {
        case k_move_next:
            pCursor->move_next();
            break;
        case k_move_prev:
            pCursor->move_prev();
            break;
        case k_enter:
            pCursor->enter_element();
            break;
        case k_exit:
            pCursor->exit_element();
            break;
        case k_point_to:
            pCursor->point_to(m_targetId);
            break;
        case k_to_state:
            pCursor->to_inner_point(m_targetState);
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
    ////pCursor->restore_state(m_curState);
}

//---------------------------------------------------------------------------------------
void CmdCursor::set_default_name()
{
    switch(m_operation)
    {
        case k_move_next:
            m_name = "Cursor: move next";
            break;
        case k_move_prev:
            m_name = "Cursor: move prev";
            break;
        case k_enter:
            m_name = "Cursor: enter element";
            break;
        case k_exit:
            m_name = "Cursor: exit element";
            break;
        case k_point_to:
            m_name = "Cursor: point to";
            break;
        case k_to_state:
            m_name = "Cursor: jump to new place";
            break;
        default:
            ;
    }
}


//=======================================================================================
// CmdDeleteBlockLevelObj implementation
//=======================================================================================
CmdDeleteBlockLevelObj::CmdDeleteBlockLevelObj(const string& name)
    : DocCmdSimple(name)
    , m_targetId(k_no_imoid)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdDeleteBlockLevelObj::set_target(Document* pDoc, DocCursor* pCursor,
                                       SelectionSet* pSelection)
{
    //TODO: treatment for selections
    ImoBlockLevelObj* pImo = dynamic_cast<ImoBlockLevelObj*>( **pCursor );
    if (pImo)
    {
        m_targetId = pImo->get_id();
        return k_success;
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
int CmdDeleteBlockLevelObj::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: Checkpoint.
    //TODO: Posible optimization: partial checkpoint: save only source code for deleted
    //block

    create_checkpoint(pDoc);

    ImoBlockLevelObj* pImo = dynamic_cast<ImoBlockLevelObj*>(
                                    pDoc->get_pointer_to_imo( m_targetId ) );
    if (pImo)
    {
        if (m_name == "")
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
CmdDeleteRelation::CmdDeleteRelation(const string& name)
    : DocCmdSimple(name)
    , m_type(-1)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdDeleteRelation::CmdDeleteRelation(int type, const string& name)
    : DocCmdSimple(name)
    , m_type(type)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdDeleteRelation::set_target(Document* pDoc, DocCursor* pCursor,
                                  SelectionSet* pSelection)
{
    if (pSelection && !pSelection->empty())
    {
        if (m_type == -1)
        {
            //first selected object
            ImoRelObj* pRO = dynamic_cast<ImoRelObj*>( pSelection->front() );
            if (pRO)
            {
                m_relobjs.push_back( pRO->get_id() );
                if (m_name == "")
                    m_name = "Delete " + pRO->get_name();
                return k_success;
            }
        }
        else
        {
            //all objects of specified type in selection
            m_relobjs = pSelection->filter(m_type);
            if (!m_relobjs.empty())
            {
                if (m_name == "")
                    m_name = "Delete " + ImoObj::get_name(m_type);
                return k_success;
            }
        }
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
int CmdDeleteRelation::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: checkpoint, because the relation could have some attributes
    //modified (color, user positioned, ...).
    //TODO: OPTIMIZATION direct undo via partial checkpoint, that is, only relation
    //source code
    create_checkpoint(pDoc);

    list<ImoId>::iterator it;
    for (it = m_relobjs.begin(); it != m_relobjs.end(); ++it)
    {
        ImoRelObj* pRO = static_cast<ImoRelObj*>( pDoc->get_pointer_to_imo(*it) );
        pDoc->delete_relation(pRO);
    }
    return k_success;
}


//=======================================================================================
// CmdDeleteSelection implementation
//=======================================================================================
CmdDeleteSelection::CmdDeleteSelection(const string& name)
    : DocCmdSimple(name)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdDeleteSelection::set_target(Document* pDoc, DocCursor* pCursor,
                                    SelectionSet* pSelection)
{
    if (pSelection && !pSelection->empty())
    {
        //staffobjs
        ColStaffObjs* pCSO = pSelection->get_staffobjs_collection();
        ColStaffObjsIterator itSO;
        for (itSO = pCSO->begin(); itSO != pCSO->end(); ++itSO)
        {
            ImoId id = (*itSO)->element_id();
            m_idSO.push_back(id);
        }

        //all other objects
        list<ImoObj*>& objects = pSelection->get_all_objects();
        list<ImoObj*>::const_iterator it;
        for (it = objects.begin(); it != objects.end(); ++it)
        {
            ImoId id = (*it)->get_id();
            if ((*it)->is_staffobj())
                ;   //ignore
            else if ((*it)->is_relobj())
                m_idRO.push_back(id);
            else if ((*it)->is_auxobj())
                m_idAO.push_back(id);
            else
                m_idOther.push_back(id);
        }
        return k_success;
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
int CmdDeleteSelection::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: As many objects could be deleted and many relations
    //could be involved, the best approach is to use a checkpoint.
    create_checkpoint(pDoc);

    prepare_cursor_for_deletion(pCursor);
    delete_staffobjs(pDoc);
    delete_relobjs(pDoc);
    delete_auxobjs(pDoc);
    delete_other(pDoc);

    //rebuild StaffObjs collection
    ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_top_object() );
    pScore->close();

    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdDeleteSelection::prepare_cursor_for_deletion(DocCursor* pCursor)
{
    //For recovering from a deletion, cursor prev. position must be valid after
    //deletion. Thus, in this method it is ensured that cursor is moved to a safe
    //place before deletion

    if (pCursor->is_at_top_level())
        return;
    if (!pCursor->get_top_object()->is_score())
        return;
    ScoreCursor* pSC = static_cast<ScoreCursor*>( pCursor->get_inner_cursor() );
    ImoId prevId = pSC->prev_pos_id();
    while (prevId >= 0 && is_going_to_be_deleted(prevId))
        pCursor->move_prev();
}

//---------------------------------------------------------------------------------------
bool CmdDeleteSelection::is_going_to_be_deleted(ImoId id)
{
    list<ImoId>::iterator it = find(m_idSO.begin(), m_idSO.end(), id);
    return it != m_idSO.end();
}

//---------------------------------------------------------------------------------------
void CmdDeleteSelection::delete_staffobjs(Document* pDoc)
{
    list<ImoId>::iterator it;
    for (it = m_idSO.begin(); it != m_idSO.end(); ++it)
    {
        if (*it != k_no_imoid)
        {
            ImoStaffObj* pSO = static_cast<ImoStaffObj*>( pDoc->get_pointer_to_imo(*it) );
            if (pSO)
                delete_staffobj(pSO);
        }
    }
    reorganize_relations(pDoc);
}

//---------------------------------------------------------------------------------------
void CmdDeleteSelection::delete_staffobj(ImoStaffObj* pImo)
{
    if (pImo)
    {
        //get and save relations
        ImoRelations* pRels = pImo->get_relations();
        if (pRels)
        {
            list<ImoRelObj*>& relations = pRels->get_relations();
            if (relations.size() > 0)
            {
                list<ImoRelObj*>::iterator it;
                for (it = relations.begin(); it != relations.end(); ++it)
                {
                    m_relIds.push_back( (*it)->get_id() );
                }
            }
        }

        //delete object
        ImoInstrument* pInstr = pImo->get_instrument();
        pInstr->delete_staffobj(pImo);
    }
}

//---------------------------------------------------------------------------------------
void CmdDeleteSelection::reorganize_relations(Document* pDoc)
{
    //ask relations to reorganize themselves
    if (m_relIds.size() > 0)
    {
        list<ImoId>::iterator itV;
        for (itV = m_relIds.begin(); itV != m_relIds.end(); ++itV)
        {
            ImoRelObj* pRO = static_cast<ImoRelObj*>( pDoc->get_pointer_to_imo(*itV) );
            if (pRO)
                pRO->reorganize_after_object_deletion();
        }
    }
}

//---------------------------------------------------------------------------------------
void CmdDeleteSelection::delete_relobjs(Document* pDoc)
{
    list<ImoId>::iterator it;
    for (it = m_idRO.begin(); it != m_idRO.end(); ++it)
    {
        if (*it != k_no_imoid)
        {
            ImoRelObj* pRO = static_cast<ImoRelObj*>( pDoc->get_pointer_to_imo(*it) );
            if (pRO)
                pDoc->delete_relation(pRO);
        }
    }
}

//---------------------------------------------------------------------------------------
void CmdDeleteSelection::delete_auxobjs(Document* pDoc)
{
    list<ImoId>::iterator it;
    for (it = m_idAO.begin(); it != m_idAO.end(); ++it)
    {
        if (*it != k_no_imoid)
        {
            ImoAuxObj* pAO = static_cast<ImoAuxObj*>( pDoc->get_pointer_to_imo(*it) );
            if (pAO)
                pDoc->delete_auxobj(pAO);
        }
    }
}

//---------------------------------------------------------------------------------------
void CmdDeleteSelection::delete_other(Document* pDoc)
{
    //TODO: what to delete and how?
//    list<ImoId>::iterator it;
//    for (it = m_idSO.begin(); it != m_idSO.end(); ++it)
//    {
//        if (*it != k_no_imoid)
//        {
//            ImoStaffObj* pSO = static_cast<ImoStaffObj*>( pDoc->get_pointer_to_imo(*it) );
//            delete_staffobj(pSO);
//        }
//    }
//    reorganize_relations(pDoc);
}



//=======================================================================================
// CmdDeleteStaffObj implementation
//=======================================================================================
CmdDeleteStaffObj::CmdDeleteStaffObj(const string& name)
    : DocCmdSimple(name)
{
    m_flags = k_recordable | k_reversible;
    m_id = k_no_imoid;
}

//---------------------------------------------------------------------------------------
int CmdDeleteStaffObj::set_target(Document* pDoc, DocCursor* pCursor,
                                  SelectionSet* pSelection)
{
    //TODO: treatment for selections
    ImoStaffObj* pImo = dynamic_cast<ImoStaffObj*>( pCursor->get_pointee() );
    if (pImo)
    {
        m_id = pImo->get_id();
        return k_success;
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
int CmdDeleteStaffObj::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: Deleted objects could have relations with other objects (for
    //instance, beamed groups, ligatures, etc.). Therefore, an 'undo' operation
    //would require to identify and rebuild these relations. As this can be complex,
    //it is simpler to use checkpoints.
    create_checkpoint(pDoc);

    ImoStaffObj* pImo = dynamic_cast<ImoStaffObj*>( pDoc->get_pointer_to_imo(m_id) );
    if (pImo)
    {
        if (m_name == "")
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
int CmdInsertObj::set_target(Document* pDoc, DocCursor* pCursor,
                             SelectionSet* pSelection)
{
    ImoObj* pImo = pCursor->get_pointee();
    if (pImo)
        m_idAt = pImo->get_id();
    else
        m_idAt = k_no_imoid;

    return k_success;
}

//---------------------------------------------------------------------------------------
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
CmdInsertBlockLevelObj::CmdInsertBlockLevelObj(int type, const string& name)
    : CmdInsertObj(name)
    , m_blockType(type)
    , m_fFromSource(false)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdInsertBlockLevelObj::CmdInsertBlockLevelObj(const string& source,
                                               const string& name)
    : CmdInsertObj(name)
    , m_blockType(k_imo_block_level_obj)
    , m_fFromSource(true)
{
    m_source = source;
    m_flags = k_recordable | k_reversible;
    if (m_name == "")
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
    if (m_name == "")
        set_command_name("Insert ", pImo);
    ImoDocument* pImoDoc = pDoc->get_imodoc();
    ImoBlockLevelObj* pAt = dynamic_cast<ImoBlockLevelObj*>(
                                pDoc->get_pointer_to_imo(m_idAt) );
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
        ImoBlockLevelObj* pAt = dynamic_cast<ImoBlockLevelObj*>(
                                    pDoc->get_pointer_to_imo(m_idAt) );
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
// CmdInsertManyStaffObjs implementation
//=======================================================================================
CmdInsertManyStaffObjs::CmdInsertManyStaffObjs(const string& source,  const string& name)
    : CmdInsertObj(name)
    , m_fSaved(false)
{
    m_source = source;
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdInsertManyStaffObjs::set_target(Document* pDoc, DocCursor* pCursor,
                                       SelectionSet* pSelection)
{
    if (pCursor->get_top_object()->is_score())
    {
        ScoreCursor* pSC = static_cast<ScoreCursor*>( pCursor->get_inner_cursor() );
        m_idAt = pSC->staffobj_id_internal();
        return k_success;
    }
    else
        return k_failure;
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
    ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_top_object() );
    ScoreCursor* pSC = static_cast<ScoreCursor*>( pCursor->get_inner_cursor() );
    ImoInstrument* pInstr = pScore->get_instrument( pSC->instrument() );

    //create and insert objects
    if (pInstr)
    {
        //insert the created object at desired point
        stringstream errormsg;
        ImoStaffObj* pAt = dynamic_cast<ImoStaffObj*>( pDoc->get_pointer_to_imo(m_idAt) );
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
// CmdInsertStaffObj implementation
//=======================================================================================
CmdInsertStaffObj::CmdInsertStaffObj(const string& source, const string& name)
    : CmdInsertObj(name)
{
    m_source = source;
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdInsertStaffObj::set_target(Document* pDoc, DocCursor* pCursor,
                                  SelectionSet* pSelection)
{
    if (pCursor->get_top_object()->is_score())
    {
        ScoreCursor* pSC = static_cast<ScoreCursor*>( pCursor->get_inner_cursor() );
        m_idAt = pSC->staffobj_id_internal();
        return k_success;
    }
    else
        return k_failure;
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
        ImoStaffObj* pAt = dynamic_cast<ImoStaffObj*>(
                                    pDoc->get_pointer_to_imo(m_idAt) );
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
            if (m_name == "")
            {
                m_name = "Insert ";
                m_name.append( pImo->get_name() );
            }

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
// CmdJoinBeam implementation
//=======================================================================================
CmdJoinBeam::CmdJoinBeam(const string& name)
    : DocCmdSimple(name)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdJoinBeam::set_target(Document* pDoc, DocCursor* pCursor,
                            SelectionSet* pSelection)
{
    if (pSelection && !pSelection->empty())
    {
        m_noteRests = pSelection->filter_notes_rests();
        return k_success;
    }
    return k_failure;
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
    for (it = m_noteRests.begin(); it != m_noteRests.end(); ++it)
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

//=======================================================================================
// CmdMoveObjectPoint implementation
//=======================================================================================
CmdMoveObjectPoint::CmdMoveObjectPoint(int pointIndex, UPoint shift,
                                       const string& name)
    : DocCmdSimple(name)
    , m_pointIndex(pointIndex)
    , m_shift(shift)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdMoveObjectPoint::set_target(Document* pDoc, DocCursor* pCursor,
                                   SelectionSet* pSelection)
{
    if (pSelection && !pSelection->empty())
    {
        ImoObj* pImo = pSelection->front();
        if (pImo)
        {
            m_targetId = pImo->get_id();
            return k_success;
        }
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
int CmdMoveObjectPoint::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: direct undo, as it only implies restoring old position

    ImoObj* pImo = pDoc->get_pointer_to_imo(m_targetId);
    //TODO: This code is just a test. Assumes it is a Tie and it is the first bezier
    if (pImo && pImo->is_tie())
    {
        ImoTie* pTie = static_cast<ImoTie*>(pImo);
        ImoBezierInfo* pBz = pTie->get_start_bezier_or_create();
        m_oldPos = pBz->get_point(m_pointIndex);

        //convert shift (LUnits) to Tenths
        ImoNote* pNote = pTie->get_start_note();
        ImoInstrument* pInstr = pNote->get_instrument();
        int iStaff = pNote->get_staff();
        ImoScore* pScore = pInstr->get_score();
        int iInstr = pInstr->get_instrument();
        ScoreMeter meter(pScore);
        TPoint pos(m_oldPos.x + meter.logical_to_tenths(m_shift.x, iInstr, iStaff),
                   m_oldPos.y + meter.logical_to_tenths(m_shift.y, iInstr, iStaff) );

        pBz->set_point(m_pointIndex, pos);
//    if (pImo && pImo->has_control_points())
//    {
//        m_oldPos = pImo->get_control_point(m_pointIndex);
//        pImo->set_control_point(m_pointIndex, m_newPos);
        pNote->set_dirty(true);
        return k_success;
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
void CmdMoveObjectPoint::undo_action(Document* pDoc, DocCursor* pCursor)
{
    ImoObj* pImo = pDoc->get_pointer_to_imo(m_targetId);
    //TODO: This code is just a test. Assumes it is a Tie and it is the first bezier
//    pImo->set_control_point(m_pointIndex, m_oldPos);
//    pImo->set_dirty(true);
    if (pImo && pImo->is_tie())
    {
        ImoTie* pTie = static_cast<ImoTie*>(pImo);
        ImoBezierInfo* pBz = pTie->get_start_bezier();
        pBz->set_point(m_pointIndex, m_oldPos);
        ImoNote* pNote = pTie->get_start_note();
        pNote->set_dirty(true);
    }
}


}  //namespace lomse

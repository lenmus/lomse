//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#define LOMSE_INTERNAL_API
#include "lomse_command.h"
#include "lomse_build_options.h"

#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_im_factory.h"
#include "lomse_logger.h"
#include "lomse_ldp_analyser.h"         //ldp_pitch_to_components
#include "lomse_autobeamer.h"
#include "lomse_model_builder.h"
#include "lomse_selections.h"
#include "lomse_score_meter.h"
#include "lomse_im_algorithms.h"
#include "lomse_staffobjs_table.h"      //class ScoreAlgorithms
#include "lomse_ldp_exporter.h"
#include "lomse_internal_model.h"
#include "lomse_score_algorithms.h"
#include "lomse_score_utilities.h"

#include <sstream>
using namespace std;

namespace lomse
{

//=======================================================================================
// DocCommand
//=======================================================================================
void DocCommand::create_checkpoint(Document* pDoc)
{
    if (!is_included_in_composite_cmd())
    {
        if (m_checkpoint.empty())
        {
            if (get_undo_policy() == k_undo_policy_partial_checkpoint)
                m_checkpoint = pDoc->get_checkpoint_data_for(m_idChk);
            else
                m_checkpoint = pDoc->get_checkpoint_data();
        }
    }
}

//---------------------------------------------------------------------------------------
void DocCommand::undo_action(Document* pDoc, DocCursor* UNUSED(pCursor))
{
    //default implementation based on restoring from saved checkpoint data

    //log command for forensic analysis
    ofstream logger;
    logger.open("forensic_log.txt", std::ofstream::out | std::ofstream::app);

    logger << "---------------------------------------------"
           << "---------------------------------------------" << endl;
    logger << "Before Undo, time="
           << to_simple_string(chrono::system_clock::now()) << endl;
    if (get_undo_policy() == k_undo_policy_partial_checkpoint)
        logger << "Undo policy: Partial checkpoint. Obj: " << m_idChk << endl;
    else
        logger << "Undo policy: Full checkpoint" << endl;

    logger << "IdAssigner. Before: " << pDoc->dump_ids() << endl;

    //execute undo
    if (get_undo_policy() == k_undo_policy_partial_checkpoint)
        pDoc->replace_object_from_checkpoint_data(m_idChk, m_checkpoint);
    else
        pDoc->from_checkpoint(m_checkpoint);

    logger << "IdAssigner. After: " << pDoc->dump_ids() << endl;
    logger.close();
}

//---------------------------------------------------------------------------------------
void DocCommand::log_forensic_data(Document* UNUSED(pDoc), DocCursor* pCursor)
{
    //save data for forensic analysis if a crash

    ofstream logger;
    logger.open("forensic_log.txt", std::ofstream::out | std::ofstream::app);

    logger << "---------------------------------------------"
           << "---------------------------------------------" << endl;
    logger << "Before executing command, time="
           << to_simple_string(chrono::system_clock::now()) << endl;
    log_command(logger);
    logger << "Cursor: " << pCursor->dump_cursor();
    logger << "Checkpoint data (last id " << m_idChk << "):" << endl;
    logger << m_checkpoint << endl;
    logger.close();
}

//---------------------------------------------------------------------------------------
void DocCommand::log_command(ofstream &logger)
{
    //default implementation. Should be overriden in specific commands
    logger << "Command. Name: " << this->get_name() << endl;
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

//---------------------------------------------------------------------------------------
int DocCommand::validate_source(const string& source)
{
    //TODO: refactor. Source code exploration must be done by LdpParser. It should return
    //the number of top level elements and a flag for signaling no parenthesis mismatch.
    //Here, we should only ask parser for a quick check and validate num of top level
    //elements

    //starts and ends with parenthesis
    size_t size = source.size();
    if (size < 3 || source.at(0) != '(' || source.at(size-1) != ')')
    {
        m_error = "Missing start or end parenthesis";
        return k_failure;
    }

    int open = 1;
    bool fPerhapsMoreThanOneElement = false;
    for (size_t i=1; i < size; ++i)
    {
        if (source.at(i) == '(')
        {
            if (open == 0)
                fPerhapsMoreThanOneElement = true;
            open++;
        }
        else if (source.at(i) == ')')
            open--;
        //TODO: skip parenthesis inside strings!
    }
    if (open != 0)
    {
        m_error = "Parenthesis mismatch";
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
// DocCmdComposite
//=======================================================================================
DocCmdComposite::DocCmdComposite(const string& name)
    : DocCommand(name)
    , m_undoPolicy(k_undo_policy_specific)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
DocCmdComposite::~DocCmdComposite()
{
    list<DocCommand*>::iterator it = m_commands.begin();
    while (it != m_commands.end())
    {
        DocCommand* pCmd = *it;
        it = m_commands.erase(it);
        delete pCmd;
    }
}

//---------------------------------------------------------------------------------------
void DocCmdComposite::add_child_command(DocCommand* pCmd)
{
    m_commands.push_back(pCmd);
    pCmd->mark_as_included_in_composite_cmd();
    if (pCmd->get_undo_policy() == k_undo_policy_full_checkpoint)
        m_undoPolicy = k_undo_policy_full_checkpoint;
}

//---------------------------------------------------------------------------------------
int DocCmdComposite::set_target(Document* pDoc, DocCursor* pCursor,
                                SelectionSet* pSelection)
{
    int result = k_success;
    list<DocCommand*>::iterator it;
    for (it=m_commands.begin(); it != m_commands.end(); ++it)
    {
        result &= (*it)->set_target(pDoc, pCursor, pSelection);
    }

    return result;
}

//---------------------------------------------------------------------------------------
int DocCmdComposite::perform_action(Document* pDoc, DocCursor* pCursor)
{
    int result = k_success;

    if (m_undoPolicy == k_undo_policy_full_checkpoint
        || m_undoPolicy == k_undo_policy_partial_checkpoint)
    {
        create_checkpoint(pDoc);
        log_forensic_data(pDoc, pCursor);
    }

    list<DocCommand*>::iterator it;
    for (it=m_commands.begin(); it != m_commands.end(); ++it)
    {
        if ((*it)->get_cursor_update_policy() == DocCommand::k_refresh)
            (*it)->set_final_cursor_pos( pCursor->get_pointee_id() );

        result &= (*it)->perform_action(pDoc, pCursor);
    }

    return result;
}

//---------------------------------------------------------------------------------------
void DocCmdComposite::undo_action(Document* pDoc, DocCursor* pCursor)
{
    if (m_undoPolicy == k_undo_policy_full_checkpoint
        || m_undoPolicy == k_undo_policy_partial_checkpoint)
    {
        DocCommand::undo_action(pDoc, pCursor);
    }
    else
    {
        list<DocCommand*>::reverse_iterator it;
        for (it=m_commands.rbegin(); it != m_commands.rend(); ++it)
        {
            (*it)->undo_action(pDoc, pCursor);
        }
    }
}

//---------------------------------------------------------------------------------------
void DocCmdComposite::update_cursor(DocCursor* pCursor, DocCommandExecuter* pExecuter)
{
    list<DocCommand*>::iterator it;
    for (it=m_commands.begin(); it != m_commands.end(); ++it)
    {
        pExecuter->update_cursor(pCursor, *it);
    }
}

//---------------------------------------------------------------------------------------
void DocCmdComposite::update_selection(SelectionSet* pSelection,
                                       DocCommandExecuter* pExecuter)
{
    list<DocCommand*>::iterator it;
    for (it=m_commands.begin(); it != m_commands.end(); ++it)
    {
        pExecuter->update_selection(pSelection, *it);
    }
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
        UndoElement* pUE = nullptr;
        if (pCmd->is_reversible())
            pUE = LOMSE_NEW UndoElement(pCmd,
                                        pCursor->get_state(),
                                        pSelection->get_state()
                                       );

        if (pCmd->get_cursor_update_policy() == DocCommand::k_refresh)
            pCmd->set_final_cursor_pos( pCursor->get_pointee_id() );

        result = pCmd->perform_action(m_pDoc, pCursor);
        m_error = pCmd->get_error();
        if ( result == k_success && pCmd->is_reversible())
        {
            m_stack.push( pUE );
            update_cursor(pCursor, pCmd);
            update_selection(pSelection, pCmd);
            m_pDoc->set_modified();
        }
        else
            delete pUE;
    }
    else
    {
        string error = pCmd->get_error();
        if (error.empty())
            m_error = "Command ignored. Can not set target";
        else
            m_error = error;
    }

    return result;
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::update_cursor(DocCursor* pCursor, DocCommand* pCmd)
{
    if (pCmd->is_composite())
    {
        (static_cast<DocCmdComposite*>(pCmd))->update_cursor(pCursor, this);
    }
    else
    {
        int policy = pCmd->get_cursor_update_policy();
        switch (policy)
        {
            case DocCommand::k_do_nothing:
            {
                break;
            }
            case DocCommand::k_update_after_insertion:
            {
                CmdInsert* cmd = static_cast<CmdInsert*>(pCmd);
                pCursor->reset_and_point_to( cmd->last_inserted_id() );
                pCursor->move_next();
                break;
            }
            case DocCommand::k_update_after_deletion:
            {
                CmdDelete* cmd = static_cast<CmdDelete*>(pCmd);
                pCursor->reset_and_point_after( cmd->cursor_final_pos_id() );
                break;
            }
            case DocCommand::k_refresh:
            {
                pCursor->reset_and_point_to( pCmd->get_final_cursor_pos() );
                break;
            }
            default:
            {
                LOMSE_LOG_ERROR("Unknown cursor update policy.");
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::update_selection(SelectionSet* pSelection, DocCommand* pCmd)
{
    if (pCmd->is_composite())
    {
        (static_cast<DocCmdComposite*>(pCmd))->update_selection(pSelection, this);
    }
    else
    {
        int policy = pCmd->get_selection_update_policy();
        switch (policy)
        {
            case DocCommand::k_sel_do_nothing:
            {
                break;
            }
            case DocCommand::k_sel_clear:
            {
                if (pSelection)
                    pSelection->clear();

                break;
            }
            case DocCommand::k_sel_command_specific:
            {
                if (pSelection)
                {
                    pSelection->clear();
                    pCmd->update_selection(pSelection);
                }
                break;
            }
            default:
            {
                LOMSE_LOG_ERROR("Unknown selection update policy.");
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::undo(DocCursor* pCursor, SelectionSet* pSelection)
{
    UndoElement* pUE = m_stack.pop();
    if (pUE)
    {
        DocCommand* cmd = pUE->pCmd;
        cmd->undo_action(m_pDoc, pCursor);

        pCursor->restore_state( pUE->cursorState );
        pSelection->restore_state( pUE->selState );

        if (cmd->is_reversible())
            m_pDoc->reset_modified();
    }
}

//---------------------------------------------------------------------------------------
void DocCommandExecuter::redo(DocCursor* pCursor, SelectionSet* pSelection)
{
    UndoElement* pUE = m_stack.undo_pop();
    if (pUE)
    {
        pCursor->restore_state( pUE->cursorState );
        pSelection->restore_state( pUE->selState );
        DocCommand* cmd = pUE->pCmd;
        cmd->perform_action(m_pDoc, pCursor);

        update_cursor(pCursor, cmd);
        update_selection(pSelection, cmd);

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
// CmdAddChordNote implementation
//=======================================================================================
CmdAddChordNote::CmdAddChordNote(const string& pitch, const string& name)
    : DocCmdSimple(name)
    , m_baseId(k_no_imoid)
    , m_noteId(k_no_imoid)
    , m_pitch(pitch)
    , m_step(0)
    , m_octave(0)
    , m_accidentals(k_no_accidentals)
    , m_type(0)
    , m_dots(0)
    , m_voice(0)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
void CmdAddChordNote::log_command(ofstream &logger)
{
    logger << "Command CmdAddChordNote. Name: '" << this->get_name()
        << ", pitch: " << m_pitch << endl;
}

//---------------------------------------------------------------------------------------
int CmdAddChordNote::set_target(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor),
                                SelectionSet* pSelection)
{
    m_baseId = k_no_imoid;
    m_noteId = k_no_imoid;

    //A note and only one must be selected
    if (pSelection == nullptr
        || pSelection->num_selected() != 1
        || !pSelection->front()->is_note()
       )
    {
        m_error = "Command ignored. No note selected or more than one.";
        return k_failure;
    }

    ImoNote* pBaseNote = static_cast<ImoNote*>( pSelection->front() );
    m_baseId = pBaseNote->get_id();
    m_voice = pBaseNote->get_voice();
    TimeUnits duration = pBaseNote->get_duration();
    NoteTypeAndDots td = duration_to_note_type_and_dots(duration);
    m_type = td.noteType;
    m_dots = td.dots;

    //undo based on partial checkpoint: get id of element to save
    ImoScore* pScore = pBaseNote->get_score();
    m_idChk = pScore->get_id();

    //validate pitch and extract components
    if (LdpAnalyser::ldp_pitch_to_components(m_pitch, &m_step, &m_octave, &m_accidentals))
        return k_failure;

    return k_success;
}

//---------------------------------------------------------------------------------------
int CmdAddChordNote::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: Checkpoint, as more notes are involved in the changes
    create_checkpoint(pDoc);
    log_forensic_data(pDoc, pCursor);

    //get data about insertion context
    ImoNote* pBaseNote = static_cast<ImoNote*>(
                                pDoc->get_pointer_to_imo(m_baseId) );
    ImoInstrument* pInstr = pBaseNote->get_instrument();
    ImoScore* pScore = pInstr->get_score();

    //create note to insert
    ImoNote* pNewNote = static_cast<ImoNote*>( ImFactory::inject(k_imo_note, pDoc) );
    pNewNote->set_note_type_and_dots(m_type, m_dots);
    pNewNote->set_voice(m_voice);
    pNewNote->set_notated_pitch(m_step, m_octave, m_accidentals);
    m_noteId = pNewNote->get_id();

    //add new note to Imo tree
    pInstr->insert_staffobj_after(pBaseNote, pNewNote);

    //create or update the chord
    ImoTreeAlgoritms::add_note_to_chord(pBaseNote, pNewNote, pDoc);

    //force to rebuild ColStaffObjs table
    pScore->end_of_changes();

    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdAddChordNote::update_selection(SelectionSet* pSelection)
{
    //AWARE: When arriving here selection has been cleared
    if (m_noteId != k_no_imoid)
        pSelection->add(m_noteId);
}



//=======================================================================================
// CmdAddNoteRest implementation
//=======================================================================================
CmdAddNoteRest::CmdAddNoteRest(const string& source, int UNUSED(editMode),
                               const string& name)
    : DocCmdSimple(name)
    , m_idAt(k_no_imoid)
    , m_source(source)
    //
    , m_pDoc(nullptr)
    , m_pCursor(nullptr)
    , m_pScore(nullptr)
    , m_pInstr(nullptr)
    , m_pSC(nullptr)
    , m_instr(0)
    , m_newVoice(0)
    , m_pNewNR(nullptr)
    , m_insertionTime(0.0)
    , m_newDuration(0.0)
    , m_pAt(nullptr)
    , m_pLastOverlapped(nullptr)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::log_command(ofstream &logger)
{
    logger << "Command CmdAddNoteRest. Name: '" << this->get_name()
        << ", source: " << m_source << endl;
}

//---------------------------------------------------------------------------------------
int CmdAddNoteRest::set_target(Document* UNUSED(pDoc), DocCursor* pCursor,
                               SelectionSet* UNUSED(pSelection))
{
    //undo based on partial checkpoint: get id of element to save
    ImoObj* pParent = pCursor->get_parent_object();
    if (pCursor->get_parent_object()->is_score())
    {
        m_idChk = pParent->get_id();

        //target will be set when performing the action. Here just a couple of checks
        return validate_source(m_source);
    }
    else
        return k_failure;
}

//---------------------------------------------------------------------------------------
int CmdAddNoteRest::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: Checkpoint, as more notes/rests can be modified/deleted.
    create_checkpoint(pDoc);
    log_forensic_data(pDoc, pCursor);

    m_pDoc = pDoc;
    m_pCursor = pCursor;
    m_finalSrc = m_source;

    get_data_about_insertion_point();
    get_data_about_noterest_to_insert();
    find_and_classify_overlapped_noterests();
    determine_insertion_point();
    if (!m_overlaps.empty())
    {
        reduce_duration_of_overlapped_at_end();
        remove_fully_overlapped();
        insert_new_content();
        reduce_duration_of_overlapped_at_start();
        add_new_note_to_existing_beam_if_necessary();
    }
    else
    {
        add_go_fwd_if_needed();
        insert_new_content();
    }

    clear_temporary_objects();

    //rebuild ColStaffObjs table, as there are objects added/removed
    m_pScore->end_of_changes();
    update_cursor();

    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::update_cursor()
{
    //AWARE: For now, command CmdAddNoteRest is working only in replace mode. Therefore,
    //depending on voice and insertion point, the pointed object could have been replaced
    //and thus no longer exist.
    //Therefore, it is necessary to update the cursor refresh information

    m_pCursor->reset_and_point_to(m_idAt);
    if (m_pCursor->get_pointee_id() != m_idAt)
    {
        //object replaced. Point to last inserted object and move next
        ImoStaffObj* pSO = m_insertedObjs.back();
        m_pCursor->reset_and_point_to(pSO->get_id());
        m_pCursor->move_next();
        set_final_cursor_pos( m_pCursor->get_pointee_id() );
    }
    else
        set_final_cursor_pos(m_idAt);
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::get_data_about_insertion_point()
{
    //get score that will be modified
    m_pSC = static_cast<ScoreCursor*>( m_pCursor->get_inner_cursor() );
    m_pScore = static_cast<ImoScore*>( m_pCursor->get_parent_object() );

    //data about insertion point
    m_insertionTime = m_pSC->time();
    m_instr = m_pSC->instrument();
    m_pInstr = m_pScore->get_instrument(m_instr);
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::get_data_about_noterest_to_insert()
{
    stringstream errormsg;
    m_pNewNR =
        static_cast<ImoNoteRest*>( m_pDoc->create_object_from_ldp(m_source, errormsg) );
    m_newDuration = m_pNewNR->get_duration();
    m_newVoice = m_pNewNR->get_voice();

    set_command_name();
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::find_and_classify_overlapped_noterests()
{
    m_overlaps =
        ScoreAlgorithms::find_and_classify_overlapped_noterests_at(m_pScore,
            m_instr, m_newVoice, m_insertionTime, m_newDuration);
    m_pLastOverlapped = nullptr;    //Will be computed in
                                //   reduce_duration_of_overlapped_at_start()
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::determine_insertion_point()
{

    if (!m_overlaps.empty())
    {
        //AWARE: This is executed *before* any change
        //insertion point is after last overlapped full, if any exists.
        //otherwise, before first overlapped at start, if any exists
        //otherwise, after last overlapped at end.
        ImoNoteRest* pLastFullNR = nullptr;
        ImoNoteRest* pFirstStartNR = nullptr;
        ImoNoteRest* pLastEndNR = nullptr;
        list<OverlappedNoteRest*>::const_iterator it;
        for (it = m_overlaps.begin(); it != m_overlaps.end(); ++it)
        {
            if ((*it)->type == k_overlap_full)
            {
                pLastFullNR = (*it)->pNR;
            }
            else if (pFirstStartNR == nullptr && (*it)->type == k_overlap_at_start)
            {
                pFirstStartNR = (*it)->pNR;
            }
            else
                pLastEndNR = (*it)->pNR;
        }

        if (pLastFullNR)
        {
            m_pSC->point_to(pLastFullNR);
            m_pSC->move_next();
        }
        else if (pFirstStartNR)
        {
            m_pSC->point_to(pFirstStartNR);
        }
        else
        {
            m_pSC->point_to(pLastEndNR);
            m_pSC->move_next();
        }
    }

    m_idAt = m_pSC->staffobj_id_internal();
    m_pAt = static_cast<ImoStaffObj*>(m_pDoc->get_pointer_to_imo(m_idAt));
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::reduce_duration_of_overlapped_at_end()
{
    list<OverlappedNoteRest*>::const_iterator it;
    for (it = m_overlaps.begin(); it != m_overlaps.end(); ++it)
    {
        if ((*it)->type == k_overlap_at_end)
        {
            ImoNoteRest* pNR = (*it)->pNR;
            TimeUnits duration = pNR->get_duration() - (*it)->overlap;
            ImoTreeAlgoritms::change_noterest_duration(pNR, duration);
        }
    }
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::remove_fully_overlapped()
{
    list<OverlappedNoteRest*>::const_iterator it;
    for (it = m_overlaps.begin(); it != m_overlaps.end(); ++it)
    {
        if ((*it)->type == k_overlap_full)
        {
            ImoTreeAlgoritms::remove_staffobj(m_pDoc, (*it)->pNR);
        }
    }
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::insert_new_content()
{
    m_insertedObjs = ImoTreeAlgoritms::insert_staffobjs(m_pInstr, m_pAt, m_finalSrc);
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::reduce_duration_of_overlapped_at_start()
{
    list<OverlappedNoteRest*>::const_iterator it;
    for (it = m_overlaps.begin(); it != m_overlaps.end(); ++it)
    {
        if ((*it)->type == k_overlap_at_start)
        {
            ImoNoteRest* pNR = (*it)->pNR;
            TimeUnits duration = pNR->get_duration() - (*it)->overlap;
            ImoTreeAlgoritms::change_noterest_duration(pNR, duration);
            m_pLastOverlapped = pNR;
        }
    }
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::add_new_note_to_existing_beam_if_necessary()
{
    if (m_pNewNR->get_note_type() > k_quarter && m_pLastOverlapped)
    {
        if (m_pLastOverlapped->is_beamed())
        {
            ImoBeam* pBeam = m_pLastOverlapped->get_beam();

            //add new note to the set of notes that will form the new beamed group
            list<ImoNoteRest*> notes;
            notes.push_back( static_cast<ImoNoteRest*>(m_insertedObjs.back()) );

            //add existing notes/rests in the beam
            list< pair<ImoStaffObj*, ImoRelDataObj*> >& relatedObjects =
                                                            pBeam->get_related_objects();
            list< pair<ImoStaffObj*, ImoRelDataObj*> >::const_iterator it;
            for (it = relatedObjects.begin(); it != relatedObjects.end(); ++it)
                notes.push_back( static_cast<ImoNoteRest*>(it->first) );

            //remove the existing beam
            m_pDoc->delete_relation(pBeam);

            //create the new beam
            m_pDoc->add_beam(notes);
        }
    }
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::update_selection(SelectionSet* pSelection)
{
    //AWARE: When arriving here selection has been cleared
    if (!m_insertedObjs.empty())
    {
        //get inserted object
        ImoStaffObj* pSO = m_insertedObjs.back();
        pSelection->add(pSO->get_id());
    }
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::clear_temporary_objects()
{
    delete m_pNewNR;

    //overlapped notes
    list<OverlappedNoteRest*>::iterator it = m_overlaps.begin();
    while (it != m_overlaps.end())
    {
        OverlappedNoteRest* pOV = *it;
        it = m_overlaps.erase(it);
        delete pOV;
    }
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::set_command_name()
{
    if (m_name == "")
    {
        m_name = "Add ";
        m_name.append( m_pNewNR->get_name() );
    }
}

//---------------------------------------------------------------------------------------
void CmdAddNoteRest::add_go_fwd_if_needed()
{
    TimeUnits endTime = ScoreAlgorithms::find_end_time_for_voice(m_pScore,
                                                m_instr, m_newVoice, m_insertionTime);

    if (is_lower_time(endTime, m_insertionTime))
    {
        NoteTypeAndDots ntd = duration_to_note_type_and_dots(m_insertionTime - endTime);
        stringstream src;
        src << "(goFwd " << LdpExporter::notetype_to_string(ntd.noteType, ntd.dots)
            << " v" << m_newVoice << ")"
            << m_source;
        m_finalSrc = src.str();
    }
}



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
int CmdAddTie::set_target(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor),
                          SelectionSet* pSelection)
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

//---------------------------------------------------------------------------------------
void CmdAddTie::log_command(ofstream &logger)
{
    logger << "Command CmdAddTie. Name: '" << this->get_name()
        << ", start & end notes: " << m_startId << ", " << m_endId << endl;
}

//---------------------------------------------------------------------------------------
int CmdAddTie::perform_action(Document* pDoc, DocCursor* UNUSED(pCursor))
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
    return (pTie != nullptr ? k_success : k_failure);
}

//---------------------------------------------------------------------------------------
void CmdAddTie::undo_action(Document* pDoc, DocCursor* UNUSED(pCursor))
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
int CmdAddTuplet::set_target(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor),
                             SelectionSet* pSelection)
{
    if (pSelection && !pSelection->empty())
    {
        ImoNoteRest* pStart = nullptr;
        ImoNoteRest* pEnd = nullptr;
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
void CmdAddTuplet::log_command(ofstream &logger)
{
    logger << "Command CmdAddTuplet. Name: '" << this->get_name()
        << ", start & end notes: " << m_startId << ", " << m_endId
        << ", source: " << m_source << endl;
}

//---------------------------------------------------------------------------------------
int CmdAddTuplet::perform_action(Document* pDoc, DocCursor* UNUSED(pCursor))
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
    return (pTuplet != nullptr ? k_success : k_failure);
}

//---------------------------------------------------------------------------------------
void CmdAddTuplet::undo_action(Document* pDoc, DocCursor* UNUSED(pCursor))
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
int CmdBreakBeam::set_target(Document* UNUSED(pDoc), DocCursor* pCursor,
                             SelectionSet* UNUSED(pSelection))
{
    ImoObj* pParent = pCursor->get_parent_object();
    if (pCursor->get_parent_object()->is_score())
    {
        m_idChk = pParent->get_id();
        ImoNoteRest* pBeforeNR = dynamic_cast<ImoNoteRest*>( pCursor->get_pointee() );
        if (pBeforeNR)
        {
            m_beforeId = pBeforeNR->get_id();
            return k_success;
        }
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
void CmdBreakBeam::log_command(ofstream &logger)
{
    logger << "Command CmdBreakBeam. Name: '" << this->get_name()
        << ", before note: " << m_beforeId << endl;
}

//---------------------------------------------------------------------------------------
int CmdBreakBeam::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: Checkpoint.
    //TODO: Posible optimization: minimal checkpoint: save only source code for beam
    create_checkpoint(pDoc);
    log_forensic_data(pDoc, pCursor);

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
    ImoNoteRest* pPrevNR = nullptr;
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
    if (pNR == nullptr || pPrevNR == nullptr || nNotesBefore == 0)
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
        //Coverity false positive. pBeam is not freed as it has more than one note
        // coverity[use_after_free]
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
            //Coverity false positive. pBeam is not freed as it has more than one note
            // coverity[use_after_free]
            (*it)->remove_from_relation(pBeam);

        //create a new beam for the removed notes
        pDoc->add_beam(notesBefore);

        //adjust the old beam
        //Coverity false positive. pBeam is not freed as it has more than one note
        // coverity[use_after_free]
        AutoBeamer autobeamer(pBeam);
        autobeamer.do_autobeam();

        return k_success;
    }

    //case d) new beam + single note
    if (nNotesBefore > 1 && nNotesAfter == 1)
    {
        //remove last note from beam
        pBeforeNR->remove_from_relation(pBeam);
        //Coverity false positive. pBeam is not freed as it has more than one note
        // coverity[use_after_free]
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
int CmdChangeAccidentals::set_target(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor),
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
void CmdChangeAccidentals::log_command(ofstream &logger)
{
    logger << "Command CmdChangeAccidentals. Name: '" << this->get_name() << endl;
}

//---------------------------------------------------------------------------------------
int CmdChangeAccidentals::perform_action(Document* pDoc, DocCursor* UNUSED(pCursor))
{
    //Undo strategy: direct undo, as it only implies restoring accidentals
    //AWARE: changing accidentals in one note could affect many notes in the
    //same measure

    bool fSavePitch = (m_oldPitch.size() == 0);     //AWARE: for redo pitch is already saved
    ImoScore* pScore = nullptr;
    list<ImoId>::iterator it;
    for (it = m_notes.begin(); it != m_notes.end(); ++it)
    {
        ImoNote* pNote = static_cast<ImoNote*>( pDoc->get_pointer_to_imo(*it) );
        if (fSavePitch)
            m_oldPitch.push_back( pNote->get_fpitch() );
        pNote->set_notated_accidentals(m_acc);
        pNote->request_pitch_recomputation();
        pNote->set_dirty(true);
        if (!pScore)
            pScore = pNote->get_score();
    }

    PitchAssigner tuner;
    tuner.assign_pitch(pScore);

    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdChangeAccidentals::undo_action(Document* pDoc, DocCursor* UNUSED(pCursor))
{
    //AWARE: restoring pitch of selected notes is not enough as changing accidentals
    // in one note could affect many notes in the same measure. Therefore, it is
    // necessary to recompute pitch of all notes

    ImoScore* pScore = nullptr;
    list<ImoId>::iterator itN;
    list<FPitch>::iterator itP = m_oldPitch.begin();
    for (itN = m_notes.begin(); itN != m_notes.end(); ++itN, ++itP)
    {
        ImoNote* pNote = static_cast<ImoNote*>( pDoc->get_pointer_to_imo(*itN) );
        FPitch fp = *itP;
        pNote->set_notated_accidentals( fp.accidentals() );
        pNote->set_actual_accidentals( float(fp.num_accidentals()) );
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
    , m_dataType(k_type_string)
    , m_newString(value)
    , m_oldDouble(0.0)
    , m_newDouble(0.0)
    , m_oldInt(0)
    , m_newInt(0)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdChangeAttribute::CmdChangeAttribute(EImoAttribute attrb,
                                       double value, const string& cmdName)
    : DocCmdSimple(cmdName)
    , m_targetId(k_no_imoid)
    , m_attrb(attrb)
    , m_dataType(k_type_double)
    , m_oldDouble(0.0)
    , m_newDouble(value)
    , m_oldInt(0)
    , m_newInt(0)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdChangeAttribute::CmdChangeAttribute(EImoAttribute attrb,
                                       int value, const string& cmdName)
    : DocCmdSimple(cmdName)
    , m_targetId(k_no_imoid)
    , m_attrb(attrb)
    , m_dataType(k_type_int)
    , m_oldDouble(0.0)
    , m_newDouble(0.0)
    , m_oldInt(0)
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
    , m_dataType(k_type_color)
    , m_oldDouble(0.0)
    , m_newDouble(0.0)
    , m_oldInt(0)
    , m_newInt(0)
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
    , m_dataType(k_type_string)
    , m_newString(value)
    , m_oldDouble(0.0)
    , m_newDouble(0.0)
    , m_oldInt(0)
    , m_newInt(0)
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
    , m_dataType(k_type_double)
    , m_oldDouble(0.0)
    , m_newDouble(value)
    , m_oldInt(0)
    , m_newInt(0)
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
    , m_dataType(k_type_int)
    , m_oldDouble(0.0)
    , m_newDouble(0.0)
    , m_oldInt(0)
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
    , m_dataType(k_type_color)
    , m_oldDouble(0.0)
    , m_newDouble(0.0)
    , m_oldInt(0)
    , m_newInt(0)
    , m_newColor(value)
{
    m_flags = k_recordable | k_reversible | k_target_set_in_constructor;
    set_target(pImo);
}

//---------------------------------------------------------------------------------------
int CmdChangeAttribute::set_target(Document* UNUSED(pDoc), DocCursor* pCursor,
                                   SelectionSet* UNUSED(pSelection))
{
    //TODO: treatment for selections
    if (m_name == "")
        m_name = "Change attribute";

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
int CmdChangeAttribute::perform_action(Document* pDoc, DocCursor* UNUSED(pCursor))
{
    //Undo strategy: direct undo, as it only implies restoring attrib value

    ImoObj* pImo = pDoc->get_pointer_to_imo( m_targetId );
    switch (m_dataType)
    {
        case k_type_bool:
            pImo->set_bool_attribute(m_attrb, m_newInt != 0); break;
        case k_type_color:
            pImo->set_color_attribute(m_attrb, m_newColor);    break;
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
void CmdChangeAttribute::undo_action(Document* pDoc, DocCursor* UNUSED(pCursor))
{
    ImoObj* pImo = pDoc->get_pointer_to_imo( m_targetId );
    switch (m_dataType)
    {
        case k_type_bool:
            pImo->set_bool_attribute(m_attrb, m_oldInt != 0); break;
        case k_type_color:
            pImo->set_color_attribute(m_attrb, m_oldColor);    break;
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
int CmdChangeDots::set_target(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor),
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

    //rebuild StaffObjs collection, as duration of some objects have changed and this
    //affects to timepos of objects after them
    ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_parent_object() );
    pScore->end_of_changes();

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

    //rebuild StaffObjs collection
    ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_parent_object() );
    pScore->end_of_changes();
}


//=======================================================================================
// CmdCursor implementation
//=======================================================================================
CmdCursor::CmdCursor(ImoId id, const string& name)
    : DocCmdSimple(name)
    , m_operation(k_point_to)
    , m_targetId(id)
    , m_measure(0)
    , m_instrument(0)
    , m_staff(0)
    , m_time(0.0)
{
    initialize();
}

//---------------------------------------------------------------------------------------
CmdCursor::CmdCursor(ECursorAction cmd, const string& name)
    : DocCmdSimple(name)
    , m_operation(cmd)
    , m_targetId(k_no_imoid)
    , m_measure(0)
    , m_instrument(0)
    , m_staff(0)
    , m_time(0.0)
{
    initialize();
}

//---------------------------------------------------------------------------------------
CmdCursor::CmdCursor(int measure, int instr, int staff, const string& name)
    : DocCmdSimple(name)
    , m_operation(k_to_measure)
    , m_targetId(k_no_imoid)
    , m_measure(measure)
    , m_instrument(instr)
    , m_staff(staff)
    , m_time(0.0)
{
    initialize();
}

//---------------------------------------------------------------------------------------
CmdCursor::CmdCursor(TimeUnits time, int instr, int staff, const string& name)
    : DocCmdSimple(name)
    , m_operation(k_to_time)
    , m_targetId(k_no_imoid)
    , m_measure(0)
    , m_instrument(instr)
    , m_staff(staff)
    , m_time(time)
{
    initialize();
}

//---------------------------------------------------------------------------------------
CmdCursor::CmdCursor(DocCursorState& state, const string& name)
    : DocCmdSimple(name)
    , m_operation(k_to_state)
    , m_targetId(k_no_imoid)
    , m_targetState(state)
    , m_measure(0)
    , m_instrument(0)
    , m_staff(0)
    , m_time(0.0)
{
    initialize();
}

//---------------------------------------------------------------------------------------
void CmdCursor::initialize()
{
    m_flags = k_recordable;
    if (m_name=="")
        set_default_name();
}

//---------------------------------------------------------------------------------------
int CmdCursor::set_target(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor),
                          SelectionSet* UNUSED(pSelection))
{
    //CmdCursor does not need a target. It is set in constructor.
    return k_success;
}

//---------------------------------------------------------------------------------------
int CmdCursor::perform_action(Document* UNUSED(pDoc), DocCursor* pCursor)
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
            pCursor->restore_state(m_targetState);
            break;
        case k_to_measure:
        {
            ImoObj* pImo = pCursor->get_parent_object();
            if (!pImo || !pImo->is_score())
                return k_failure;
            ScoreCursor* pSC = static_cast<ScoreCursor*>(pCursor->get_inner_cursor());
            pSC->to_measure(m_measure, m_instrument, m_staff);
            break;
        }
        case k_to_time:
        {
            ImoObj* pImo = pCursor->get_parent_object();
            if (!pImo || !pImo->is_score())
                return k_failure;
            ScoreCursor* pSC = static_cast<ScoreCursor*>(pCursor->get_inner_cursor());
            pSC->to_time(m_instrument, m_staff, m_time);
            break;
        }
        case k_move_up:
            pCursor->move_up();
            break;
        case k_move_down:
            pCursor->move_down();
            break;
        case k_cursor_dump:
            m_error = pCursor->dump_cursor();
            break;
        default:
            ;
    }
    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdCursor::undo_action(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor))
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
        case k_move_up:
            m_name = "Cursor: move up";
            break;
        case k_move_down:
            m_name = "Cursor: move down";
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
        case k_to_measure:
        case k_to_time:
            m_name = "Cursor: jump to new place";
            break;
        default:
            ;
    }
}


//=======================================================================================
// CmdDelete implementation
//=======================================================================================
void CmdDelete::prepare_cursor_for_deletion(DocCursor* pCursor)
{
//    pCursor->move_prev();
//    m_cursorFinalId = pCursor->get_pointee_id();
    DocCursorState state = pCursor->find_previous_pos_state();
    m_cursorFinalId = state.pointee_id();   //pCursor->find_previous_pos_state().pointee_id();
}


//=======================================================================================
// CmdDeleteBlockLevelObj implementation
//=======================================================================================
CmdDeleteBlockLevelObj::CmdDeleteBlockLevelObj(const string& name)
    : CmdDelete(name)
    , m_targetId(k_no_imoid)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdDeleteBlockLevelObj::set_target(Document* UNUSED(pDoc), DocCursor* pCursor,
                                       SelectionSet* UNUSED(pSelection))
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
    log_forensic_data(pDoc, pCursor);

    ImoBlockLevelObj* pImo = dynamic_cast<ImoBlockLevelObj*>(
                                    pDoc->get_pointer_to_imo( m_targetId ) );
    if (pImo)
    {
        prepare_cursor_for_deletion(pCursor);
        if (m_name == "")
            set_command_name("Delete ", pImo);
        pDoc->delete_block_level_obj(pImo);
        return k_success;
    }
    return k_failure;
}


//=======================================================================================
// CmdDeleteRelation implementation
//=======================================================================================
CmdDeleteRelation::CmdDeleteRelation(const string& name)
    : CmdDelete(name)
    , m_type(-1)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdDeleteRelation::CmdDeleteRelation(int type, const string& name)
    : CmdDelete(name)
    , m_type(type)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdDeleteRelation::set_target(Document* pDoc, DocCursor* UNUSED(pCursor),
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
                return set_score_id(pDoc);
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
                return set_score_id(pDoc);
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
    //      source code
    create_checkpoint(pDoc);
    log_forensic_data(pDoc, pCursor);

    list<ImoId>::iterator it;
    for (it = m_relobjs.begin(); it != m_relobjs.end(); ++it)
    {
        ImoRelObj* pRO = static_cast<ImoRelObj*>( pDoc->get_pointer_to_imo(*it) );
        pDoc->delete_relation(pRO);
    }
    return k_success;
}

//---------------------------------------------------------------------------------------
int CmdDeleteRelation::set_score_id(Document* pDoc)
{
    ImoId id = m_relobjs.front();
    ImoRelObj* pRO = static_cast<ImoRelObj*>( pDoc->get_pointer_to_imo(id) );
    ImoObj* pParent = pRO->find_block_level_parent();
    if (pParent && pParent->is_score())
    {
        m_idChk = pParent->get_id();
        return k_success;
    }
    return k_failure;
}


//=======================================================================================
// CmdDeleteSelection implementation
//=======================================================================================
CmdDeleteSelection::CmdDeleteSelection(const string& name)
    : CmdDelete(name)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdDeleteSelection::set_target(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor),
                                    SelectionSet* pSelection)
{
    if (pSelection && !pSelection->empty())
    {
        //staffobjs
        ColStaffObjs* pCSO = pSelection->get_staffobjs_collection();
        if (pCSO)
        {
            ColStaffObjsIterator itSO;
            for (itSO = pCSO->begin(); itSO != pCSO->end(); ++itSO)
            {
                ImoId id = (*itSO)->element_id();
                m_idSO.push_back(id);
            }
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
    log_forensic_data(pDoc, pCursor);

    prepare_cursor_for_deletion(pCursor);
    delete_staffobjs(pDoc);
    delete_relobjs(pDoc);
    delete_auxobjs(pDoc);
    delete_other(pDoc);

    //rebuild StaffObjs collection
    ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_parent_object() );
    pScore->end_of_changes();

    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdDeleteSelection::prepare_cursor_for_deletion(DocCursor* pCursor)
{
    //For recovering from a deletion, cursor must be moved to a safe place before
    //deletion. This method moves cursor before first object to be deleted.

    if (!pCursor->is_inside_terminal_node())
    {
        //TODO
        pCursor->move_prev();
        m_cursorFinalId = pCursor->get_pointee_id();
        return;
    }
    if (!pCursor->get_parent_object()->is_score())
    {
        //TODO
        pCursor->move_prev();
        m_cursorFinalId = pCursor->get_pointee_id();
        return;
    }

    //strategy for score objects
    ScoreCursor* pSC = static_cast<ScoreCursor*>( pCursor->get_inner_cursor() );
    pSC->move_prev();
    ImoId prevId = pSC->get_pointee_id();
    while (prevId >= 0 && is_going_to_be_deleted(prevId))
    {
        if (pSC->is_at_start_of_score())
        {
            m_cursorFinalId = k_cursor_before_start_of_child;
            return;
        }
        pSC->move_prev();
        prevId = pSC->get_pointee_id();
    }

    m_cursorFinalId = pCursor->get_pointee_id();
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
void CmdDeleteSelection::delete_other(Document* UNUSED(pDoc))
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
    : CmdDelete(name)
{
    m_flags = k_recordable | k_reversible;
    m_id = k_no_imoid;
}

//---------------------------------------------------------------------------------------
int CmdDeleteStaffObj::set_target(Document* UNUSED(pDoc), DocCursor* pCursor,
                                  SelectionSet* UNUSED(pSelection))
{
    //TODO: treatment for selections
    ImoStaffObj* pImo = dynamic_cast<ImoStaffObj*>( pCursor->get_pointee() );
    if (pImo)
    {
        m_idChk = pImo->get_score()->get_id();
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
    log_forensic_data(pDoc, pCursor);

    ImoStaffObj* pImo = dynamic_cast<ImoStaffObj*>( pDoc->get_pointer_to_imo(m_id) );
    if (pImo)
    {
        prepare_cursor_for_deletion(pCursor);
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
        ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_parent_object() );
        pScore->end_of_changes();

        return k_success;
    }
    return k_failure;
}


//=======================================================================================
// CmdInsert implementation
//=======================================================================================
int CmdInsert::set_target(Document* UNUSED(pDoc), DocCursor* pCursor,
                             SelectionSet* UNUSED(pSelection))
{
    ImoObj* pImo = pCursor->get_pointee();
    if (pImo)
        m_idAt = pImo->get_id();
    else
        m_idAt = k_no_imoid;

    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdInsert::remove_object(Document* pDoc, ImoId id)
{
    if (id != k_no_imoid)
    {
        //get object to remove
        ImoStaffObj* pImo = static_cast<ImoStaffObj*>( pDoc->get_pointer_to_imo(id) );

        //remove object from imo tree
        ImoDocument* pImoDoc = pDoc->get_im_root();
        TreeNode<ImoObj>::iterator it(pImo);
        pImoDoc->erase(it);
        pImoDoc->set_dirty(true);
        delete pImo;
    }
}



//=======================================================================================
// CmdInsertBlockLevelObj implementation
//=======================================================================================
CmdInsertBlockLevelObj::CmdInsertBlockLevelObj(int type, const string& name)
    : CmdInsert(name)
    , m_blockType(type)
    , m_fFromSource(false)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
CmdInsertBlockLevelObj::CmdInsertBlockLevelObj(const string& source,
                                               const string& name)
    : CmdInsert(name)
    , m_blockType(k_imo_block_level_obj)
    , m_fFromSource(true)
{
    m_source = source;
    m_flags = k_recordable | k_reversible;
    if (m_name == "")
        m_name = "Insert block";
}

//---------------------------------------------------------------------------------------
void CmdInsertBlockLevelObj::undo_action(Document* pDoc, DocCursor* UNUSED(pCursor))
{
    if (m_lastInsertedId != k_no_imoid)
    {
        ImoDocument* pImoDoc = pDoc->get_im_root();
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
void CmdInsertBlockLevelObj::perform_action_from_type(Document* pDoc,
                                                      DocCursor* UNUSED(pCursor))
{
    ImoBlockLevelObj* pImo =
        static_cast<ImoBlockLevelObj*>( ImFactory::inject(m_blockType, pDoc) );
    if (m_name == "")
        set_command_name("Insert ", pImo);
    ImoDocument* pImoDoc = pDoc->get_im_root();
    ImoBlockLevelObj* pAt = dynamic_cast<ImoBlockLevelObj*>(
                                pDoc->get_pointer_to_imo(m_idAt) );
    pImoDoc->insert_block_level_obj(pAt, pImo);
    m_lastInsertedId = pImo->get_id();
}

//---------------------------------------------------------------------------------------
void CmdInsertBlockLevelObj::perform_action_from_source(Document* pDoc,
                                                        DocCursor* UNUSED(pCursor))
{
    //create object
    ImoObj* pImo = pDoc->create_object_from_lmd(m_source);

    if (pImo && pImo->is_block_level_obj())
    {
        //insert the created subtree at desired point
        ImoBlockLevelObj* pAt = dynamic_cast<ImoBlockLevelObj*>(
                                    pDoc->get_pointer_to_imo(m_idAt) );
        ImoDocument* pImoDoc = pDoc->get_im_root();
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
    : CmdInsert(name)
    , m_fSaved(false)
{
    m_source = source;
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdInsertManyStaffObjs::set_target(Document* UNUSED(pDoc), DocCursor* pCursor,
                                       SelectionSet* UNUSED(pSelection))
{
    ImoObj* pParent = pCursor->get_parent_object();
    if (pParent->is_score())
    {
        m_idChk = pParent->get_id();
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
    log_forensic_data(pDoc, pCursor);

    //get instrument that will be modified
    ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_parent_object() );
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
            pScore->end_of_changes();        //update ColStaffObjs table
            save_source_code_with_ids(pDoc, objects);
            m_lastInsertedId = objects.back()->get_id();
            objects.clear();
            return k_success;
        }
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
void CmdInsertManyStaffObjs::save_source_code_with_ids(Document* UNUSED(pDoc),
                                            const list<ImoStaffObj*>& UNUSED(objects))
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
    : CmdInsert(name)
{
    m_source = source;
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdInsertStaffObj::set_target(Document* UNUSED(pDoc), DocCursor* pCursor,
                                  SelectionSet* UNUSED(pSelection))
{
    if (pCursor->get_parent_object()->is_score())
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
        ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_parent_object() );
        pScore->end_of_changes();
    }
}

//---------------------------------------------------------------------------------------
int CmdInsertStaffObj::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: direct undo, as it only implies to delete the inserted object

    if (validate_source(m_source) != k_success)
        return k_failure;

    //get instrument that will be modified
    DocCursorState state = pCursor->get_state();
    SpElementCursorState elmState = state.get_delegate_state();
    ScoreCursorState* pState = static_cast<ScoreCursorState*>( elmState.get() );
    ImoScore* pScore = static_cast<ImoScore*>( pCursor->get_parent_object() );
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
            pScore->end_of_changes();

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
int CmdJoinBeam::set_target(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor),
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
void CmdJoinBeam::log_command(ofstream &logger)
{
    logger << "Command CmdJoinBeam. Name: '" << this->get_name()
        << ", notes:";
    list<ImoId>::const_iterator it;
    for (it = m_noteRests.begin(); it != m_noteRests.end(); ++it)
        logger << " " << *it;
    logger << endl;
}

//---------------------------------------------------------------------------------------
int CmdJoinBeam::perform_action(Document* pDoc, DocCursor* pCursor)
{
    //Undo strategy: Note/rests to be beamed could have beams. For now, it is
    //simpler to use a checkpoint
    create_checkpoint(pDoc);
    log_forensic_data(pDoc, pCursor);

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
    , m_targetId(k_no_imoid)
    , m_pointIndex(pointIndex)
    , m_shift(shift)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdMoveObjectPoint::set_target(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor),
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
int CmdMoveObjectPoint::perform_action(Document* pDoc, DocCursor* UNUSED(pCursor))
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
        int iInstr = pScore->get_instr_number_for(pInstr);
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
void CmdMoveObjectPoint::undo_action(Document* pDoc, DocCursor* UNUSED(pCursor))
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


//=======================================================================================
// CmdSelection implementation
//=======================================================================================
CmdSelection::CmdSelection(int cmd, ImoId id, const string& name)
    : DocCmdSimple(name)
    , m_operation(cmd)
    , m_targetId(id)
    , m_pSelection(nullptr)
{
    initialize();
}

//---------------------------------------------------------------------------------------
CmdSelection::CmdSelection(int cmd, const string& name)
    : DocCmdSimple(name)
    , m_operation(cmd)
    , m_targetId(k_no_imoid)
    , m_pSelection(nullptr)
{
    initialize();
}

////---------------------------------------------------------------------------------------
//CmdSelection::CmdSelection(DocCursorState& state, const string& name)
//    : DocCmdSimple(name)
//    , m_operation(k_to_state)
//    , m_targetId(k_no_imoid)
//    , m_targetState(state)
//{
//    initialize();
//}

//---------------------------------------------------------------------------------------
void CmdSelection::initialize()
{
    m_flags = k_recordable;
    if (m_name=="")
        set_default_name();
}

//---------------------------------------------------------------------------------------
int CmdSelection::set_target(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor),
                             SelectionSet* pSelection)
{
    m_pSelection = pSelection;
    return k_success;
}

//---------------------------------------------------------------------------------------
int CmdSelection::perform_action(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor))
{
    //Undo strategy: this command is not reversible

    switch(m_operation)
    {
        case k_set:
            m_pSelection->clear();
            m_pSelection->add(m_targetId);
            break;

        case k_add:
            m_pSelection->add(m_targetId);
            break;

        case k_remove:
            m_pSelection->remove(m_targetId);
            break;

        case k_clear:
            m_pSelection->clear();
            break;

//        case k_cursor_dump:
//            m_error = pCursor->dump_cursor();
//            break;

        default:
            ;
    }
    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdSelection::undo_action(Document* UNUSED(pDoc), DocCursor* UNUSED(pCursor))
{
    //CmdSelection is not reversible. Nothing to do here.
}

//---------------------------------------------------------------------------------------
void CmdSelection::set_default_name()
{
    switch(m_operation)
    {
        case k_set:
            m_name = "Selection: set selection";
            break;
        case k_add:
            m_name = "Selection: add obj. to selection";
            break;
        case k_remove:
            m_name = "Selection: remove obj. from selection";
            break;
        case k_clear:
            m_name = "Selection: clear selection";
            break;
        default:
            ;
    }
}


//=======================================================================================
// CmdTranspose implementation
//=======================================================================================
CmdTranspose::CmdTranspose(const string& name)
    : DocCmdSimple(name)
{
    m_flags = k_recordable | k_reversible;
}

//---------------------------------------------------------------------------------------
int CmdTranspose::set_target(Document* UNUSED(pDoc),
                             DocCursor* UNUSED(pCursor),
                             SelectionSet* pSelection)
{
    if (pSelection && !pSelection->empty())
    {
        m_notes = pSelection->filter(k_imo_note);
        m_keys = pSelection->filter(k_imo_key_signature);
        return k_success;
    }
    return k_failure;
}

//---------------------------------------------------------------------------------------
int CmdTranspose::perform_action(Document* pDoc, DocCursor* UNUSED(pCursor))
{
    //Undo strategy: direct undo, as it only implies performing the symmetrical
    //               transposition

    ImoScore* pScore = nullptr;

    //transpose notes
    list<ImoId>::iterator itN;
    for (itN = m_notes.begin(); itN != m_notes.end(); ++itN)
    {
        //get note and score
        ImoNote* pNote = static_cast<ImoNote*>( pDoc->get_pointer_to_imo(*itN) );
        if (!pScore)
            pScore = pNote->get_score();
        if (!pScore)
            return k_failure;

        //transpose note
        transpose_note(pNote);
        pNote->set_dirty(true);
    }

    //transpose keys
    list<ImoId>::iterator itK;
    for (itK=m_keys.begin(); itK != m_keys.end(); ++itK)
    {
        ImoKeySignature* pKey = static_cast<ImoKeySignature*>( pDoc->get_pointer_to_imo(*itK) );
        if (pKey)
            transpose_key(pKey);
    }

    //assign pitch to all notes
    PitchAssigner tuner;
    tuner.assign_pitch(pScore);

    return k_success;
}

//---------------------------------------------------------------------------------------
void CmdTranspose::undo_action(Document* pDoc, DocCursor* UNUSED(pCursor))
{
    ImoScore* pScore = nullptr;

    //transpose notes back
    list<ImoId>::iterator itN;
    for (itN = m_notes.begin(); itN != m_notes.end(); ++itN)
    {
        //get note and score
        ImoNote* pNote = static_cast<ImoNote*>( pDoc->get_pointer_to_imo(*itN) );
        if (!pScore)
            pScore = pNote->get_score();
        if (!pScore)
            return;

        //transpose note back
        transpose_note_back(pNote);
        pNote->set_dirty(true);
    }

    //transpose keys back
    list<ImoId>::iterator itK;
    for (itK=m_keys.begin(); itK != m_keys.end(); ++itK)
    {
        ImoKeySignature* pKey = static_cast<ImoKeySignature*>( pDoc->get_pointer_to_imo(*itK) );
        if (pKey)
            transpose_key_back(pKey);
    }

    //assign pitch to all notes
    PitchAssigner tuner;
    tuner.assign_pitch(pScore);
}

//---------------------------------------------------------------------------------------
void CmdTranspose::transpose_chromatically(ImoNote* pNote, FIntval interval, bool fUp)
{
    FPitch fp = pNote->get_fpitch();
    if (fUp)
        fp += interval;
    else
        fp -= interval;
    pNote->set_pitch(fp);
}


//=======================================================================================
// CmdTransposeChromatically implementation
//=======================================================================================
CmdTransposeChromatically::CmdTransposeChromatically(FIntval interval,
                                                     const string& name)
    : CmdTranspose(name)
    , m_interval(interval)
    , m_fUp(interval.is_ascending())
{
    m_interval.make_ascending();
    m_flags = k_recordable | k_reversible;
    if (m_name=="")
        m_name = "Chromatic transposition";
}

//---------------------------------------------------------------------------------------
void CmdTransposeChromatically::transpose_note(ImoNote* pNote)
{
    transpose_chromatically(pNote, m_interval, m_fUp);
}

//---------------------------------------------------------------------------------------
void CmdTransposeChromatically::transpose_note_back(ImoNote* pNote)
{
    transpose_chromatically(pNote, m_interval, !m_fUp);
}


//=======================================================================================
// CmdTransposeDiatonically implementation
//=======================================================================================
CmdTransposeDiatonically::CmdTransposeDiatonically(int steps, bool fUp,
                                                   const string& name)
    : CmdTranspose(name)
    , m_steps(steps)
    , m_fUp(fUp)
{
    m_flags = k_recordable | k_reversible;
    if (m_name=="")
        m_name = "Diatonic transposition";
}

//---------------------------------------------------------------------------------------
void CmdTransposeDiatonically::transpose_note(ImoNote* pNote)
{
    transpose(pNote, m_fUp);
}

//---------------------------------------------------------------------------------------
void CmdTransposeDiatonically::transpose_note_back(ImoNote* pNote)
{
    transpose(pNote, !m_fUp);
}

//---------------------------------------------------------------------------------------
void CmdTransposeDiatonically::transpose(ImoNote* pNote, bool fUp)
{
    int step = pNote->get_step();
    if (fUp)
    {
        step += m_steps;
        if (step > 6)
        {
            step -= 7;
            pNote->set_octave( pNote->get_octave() + 1 );
        }
    }
    else
    {
        step -= m_steps;
        if (step < 0)
        {
            step += 7;
            pNote->set_octave( pNote->get_octave() - 1 );
        }
    }
    pNote->set_step(step);
    pNote->set_actual_accidentals(k_acc_not_computed);
}


//=======================================================================================
// CmdTransposeKey implementation
//=======================================================================================
CmdTransposeKey::CmdTransposeKey(FIntval interval, const string& name)
    : CmdTranspose(name)
    , m_interval(interval)
    , m_fUp(interval.is_ascending())
{
    m_interval.make_ascending();
    m_flags = k_recordable | k_reversible;
    if (m_name=="")
        m_name = "Transpose key signature";
}

//---------------------------------------------------------------------------------------
void CmdTransposeKey::transpose_note(ImoNote* pNote)
{
    transpose_chromatically(pNote, m_interval, m_fUp);
}

//---------------------------------------------------------------------------------------
void CmdTransposeKey::transpose_note_back(ImoNote* pNote)
{
    transpose_chromatically(pNote, m_interval, !m_fUp);
}

//---------------------------------------------------------------------------------------
void CmdTransposeKey::transpose_key(ImoKeySignature* pKey)
{
    EKeySignature oldType = static_cast<EKeySignature>(pKey->get_key_type());
    EKeySignature newType = KeyUtilities::transpose(oldType, m_interval, m_fUp);
    pKey->set_key_type(newType);
}

//---------------------------------------------------------------------------------------
void CmdTransposeKey::transpose_key_back(ImoKeySignature* pKey)
{
    EKeySignature oldType = static_cast<EKeySignature>(pKey->get_key_type());
    EKeySignature newType = KeyUtilities::transpose(oldType, m_interval, !m_fUp);
    pKey->set_key_type(newType);
}



}  //namespace lomse

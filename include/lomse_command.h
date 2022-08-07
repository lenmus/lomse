//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_COMMAND_H__
#define __LOMSE_COMMAND_H__

#include "lomse_stack.h"
#include "lomse_internal_model.h"
#include "lomse_document_cursor.h"
#include "lomse_pitch.h"            //EAccidentals
#include "lomse_im_attributes.h"
#include "lomse_selections.h"
#include "lomse_interval.h"

#include <sstream>
using namespace std;

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class Document;
class SelectionSet;
class DocCommandExecuter;
class OverlappedNoteRest;

//---------------------------------------------------------------------------------------
//edition modes
/** @ingroup enumerations

    This enum describes the behavior of edition commands related to adding or removing
    notes and rests.

    See @ref edit-modes.

    @#include <lomse_command.h>
*/
enum EEditMode
{
    k_edit_mode_replace = 0,    ///< <i>Replace mode</i>: Notes and rests are not inserted.
                                ///< Instead they @a overwrite the music so that measure duration
                                ///< is maintained. Also, notes are not deleted but replaced by rests.

    k_edit_mode_ripple,         ///< <i>Limited ripple mode</i>: when inserting or deleting
                                ///< notes/rests the barlines are also shifted.
};


//---------------------------------------------------------------------------------------
/**
    Abstract base class from which all commands must derive. Represents an edition command.
*/
class DocCommand
{
protected:
    string m_name;          //displayable name for undo/redo actions display
    string m_checkpoint;    //checkpoint data
    ImoId m_idChk;          //id for target object in case of partial checkpoint
    ImoId m_idRefresh;      //id for cursor object, for k_refresh policy
    string m_error;
    uint_least16_t m_flags;

    enum {
        k_reversible                    = 0x0001,
        k_recordable                    = 0x0002,
        k_target_set_in_constructor     = 0x0004,
        k_included_in_composite_cmd     = 0x0008,
    };

    DocCommand(const string& name)
        : m_name(name)
        , m_idChk(k_no_imoid)
        , m_idRefresh(k_no_imoid)
        , m_flags(0)
    {
    }

public:
    /// Destructor.
    virtual ~DocCommand() {}

    /** This enum describes the policies for updating the cursor after executing a
		command.    */
    enum ECmdCursorPolicy {
        k_do_nothing=0,             ///< Do nothing. All cursor data remains valid after command execution.
        k_update_after_deletion,    ///< Update the cursor. Current pointed object has been deleted and cursor must now point to next object.
        k_update_after_insertion,   ///< Update the cursor. Cursor must point to inserted object.
        k_refresh,                  ///< Refresh cursor. Pointed object is valid but other related info. could have changed.
    };

    /** Returns a value from #ECmdCursorPolicy that indicates the update policy
        followed by this command.    */
    virtual int get_cursor_update_policy() = 0;

    /// This enum describes the available undo policies for commands
    enum ECmdUndoPolicy {
        k_undo_policy_full_checkpoint=0,    ///< Undo based on a full checkpoint
        k_undo_policy_partial_checkpoint,   ///< Undo based on a partial checkpoint
        k_undo_policy_specific,             ///< Undo implemented by the command
        k_undo_policy_replay_from_scratch,  ///< Undo based on replaying commands
    };

    /** Returns a value from #ECmdUndoPolicy that indicates the undo policy
        followed by this command.    */
    virtual int get_undo_policy() = 0;

    /** This enum describes the available policies for updating the SelectionSet after
        executing a command.    */
    enum ECmdSelectionPolicy {
        k_sel_do_nothing=0,         ///< Do nothing. The selection is not changed after command execution.
        k_sel_clear,                ///< Clear the selection after executing the command.
        k_sel_command_specific,     ///< Specific. The command will do whatever is needed.
    };

    /** Returns a value from #ECmdSelectionPolicy that indicates the undo policy
        followed by this command.    */
    virtual int get_selection_update_policy() = 0;

    //information
    /** Returns the name of the command.

        @remarks For some commands, if this method is invoked before executing
        the command, the name could have not been set and the returned value could be
        empty or could be an incomplete string.
    */
    inline std::string get_name() { return m_name; }

    /** Returns @true if the command is reversible, that is, if the command supports
        undo/redo (e.g. insert object), and @false if it does not support undo/redo
        (e.g. print).    */
    inline bool is_reversible() { return m_flags &  k_reversible; }

    /** Returns @true if the command is recordable.

        Command sequences can be recorded and replayed at other point or on another
        document, allowing the creation of <i>macros</i> or <i>scripts</i>. But not all
        commands are suitable to be recorded and replayed.
        Recordable commands are those that can be recorded and replayed safely. All
        reversible commands are recordable by nature, as this is required for supporting
        redo. See is_reversible().
    */
    inline bool is_recordable() { return (m_flags &  k_recordable) != 0; }

    /** Returns an error message with the error explanation. This method should be
        invoked after executing a command that fails. Otherwise it will return an
        empty string.    */
    inline string get_error() { return m_error; }

    /// Returns @true if the command is composite.
    virtual bool is_composite() = 0;


//excluded from public API. Only for internal use.
///@cond INTERNALS
public:
    inline bool is_target_set_in_constructor() {
        return (m_flags &  k_target_set_in_constructor) != 0;
    }
    inline bool is_included_in_composite_cmd() {
        return (m_flags &  k_included_in_composite_cmd) != 0;
    }

    virtual void update_selection(SelectionSet* UNUSED(pSelection)) {}
    inline void set_final_cursor_pos(ImoId id) { m_idRefresh = id; }
    inline ImoId get_final_cursor_pos() { return m_idRefresh; }

    //settings
    inline void mark_as_included_in_composite_cmd() { m_flags |= k_included_in_composite_cmd; }

    //actions
    virtual int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection)=0;
    virtual int perform_action(Document* pDoc, DocCursor* pCursor)=0;
    virtual void undo_action(Document* pDoc, DocCursor* pCursor);
///@endcond

protected:
    void create_checkpoint(Document* pDoc);
    void log_forensic_data(Document* pDoc, DocCursor* pCursor);
    void set_command_name(const string& name, ImoObj* pImo);
    int validate_source(const string& source);
    virtual void log_command(ostream &logger);

};

//---------------------------------------------------------------------------------------
/** Abstract class, base for all simple (atomic) commands.
*/
class DocCmdSimple : public DocCommand
{
protected:

    DocCmdSimple(const string& name) : DocCommand(name) {}

public:
	/// Destructor
    virtual ~DocCmdSimple() {}

    /// Returns @true if the command is composite.
    bool is_composite() override { return false; }

};

//---------------------------------------------------------------------------------------
/** Abstract class, base for all composite commands.
*/
class DocCmdComposite : public DocCommand
{
protected:
    list<DocCommand*> m_commands;
    int m_undoPolicy;

public:
	/// Constructor
    DocCmdComposite(const string& name);
	/// Destructor
    virtual ~DocCmdComposite();

	///@{
    /// Change default settings for this command
    inline void mark_as_no_reversible() { m_flags &= ~k_reversible; }
    inline void mark_as_no_recordable() { m_flags &= ~k_recordable; }
    inline void mark_as_target_set_in_constructor() { m_flags |= k_target_set_in_constructor; }
	///@}

    //children commands
	/// Add a child command
    void add_child_command(DocCommand* pCmd);

    //overrides
    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return m_undoPolicy; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }
    bool is_composite() override { return true; }

    ///@cond INTERNALS
    //mandatory interface implementation
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;

    //operations delegated by DocCommandExecuter
    void update_cursor(DocCursor* pCursor, DocCommandExecuter* pExecuter);
    using DocCommand::update_selection; //tell the compiler that we want both, the
                                        //update_selection() from DocCommand and next one
    void update_selection(SelectionSet* pSelection, DocCommandExecuter* pExecuter);
    ///@endcond

protected:

};


//---------------------------------------------------------------------------------------
/** Class %UndoElement holds the necessary information to perform an undo/redo
    operation.
*/
class UndoElement
{
public:
    DocCommand*         pCmd;           ///< ptr. to executed command object.
    DocCursorState      cursorState;    ///< Cursor state before executing the command.
    SelectionState      selState;       ///< SelectionSet before executing the command.

public:
    /// Constructor
    UndoElement(DocCommand* cmd, DocCursorState state, SelectionState sel)
        : pCmd(cmd), cursorState(state), selState(sel)
    {
    }
    ///destructor
    ~UndoElement()
    {
        delete pCmd;
    }
};



//---------------------------------------------------------------------------------------
// Helper, to manage the undo/redo stack
typedef UndoableStack< UndoElement* >   UndoStack;


//---------------------------------------------------------------------------------------
/// Keeps the stack of executed commands and performs undo/redo
class DocCommandExecuter
{
private:
    Document*   m_pDoc;
    UndoStack   m_stack;
    string      m_error;

public:
    /// Constructor
    DocCommandExecuter(Document* target);
    virtual ~DocCommandExecuter() {}

    /** Executes a command and saves the necessary information for undo/redo operation.
        Returns value 0 if the command successfully executed. Otherwise returns value 1
        and a relevant error message can be retrieved by invoking
        method get_error().    */
    virtual int execute(DocCursor* pCursor, DocCommand* pCmd,
                        SelectionSet* pSelection);

    /// Undo the command
    virtual void undo(DocCursor* pCursor, SelectionSet* pSelection);

    /// Redo the command
    virtual void redo(DocCursor* pCursor, SelectionSet* pSelection);

    /// In case and execute() operation Returns an string with the error message
    inline string get_error() { return m_error; }

    //info
    /// Returns @true if there are undo-able commands in the undo/redo stack.
    bool is_undo_possible() { return m_stack.size() > 0; }
    /// Returns @true if there are redo-able commands in the undo/redo stack.
    bool is_redo_possible() { return m_stack.history_size() > 0; }
    /// Returns the number of undo/redo elements in the undo/redo stack.
    virtual size_t undo_stack_size() { return m_stack.size(); }

protected:
    friend class DocCmdComposite;
    void update_cursor(DocCursor* pCursor, DocCommand* pCmd);
    void update_selection(SelectionSet* pSelection, DocCommand* pCmd);

};

//---------------------------------------------------------------------------------------
/** A command for adding a note to an existing note and thus to create a chord, or for
    adding a note to an existing chord.

    See constructor for details.
*/
class CmdAddChordNote : public DocCmdSimple
{
protected:
    ImoId m_baseId;
    ImoId m_noteId;
    string m_pitch;
    int m_step;
    int m_octave;
    EAccidentals m_accidentals;
    int m_type;
    int m_dots;
    int m_voice;

public:
    /**
        This command adds a new note to an existing note or chord. If the selected note
        prior to executing this command is not in a chord, a new chord will be created by combining
        this new note with the existing one. Otherwise, if the selected note is part of a chord,
        the new note is also added to the existing chord.
        @param pitch A string with the pitch for the note to insert. The pitch must be in LDP format
            (step letter followed by octave, e.g. "c4", "b6", "f3"). Octave numbers are the same than in MIDI format,
            that is, octave 4 is the central one (a4 is 440Hz).
        @param name The displayable name for the command. If not specified will default to "Add chord note".

        <b>Remarks</b>
        - The target for this command must be a selected note. The selection must contain only the
            target note, otherwise the command will not be executed and will return an error.
        - Only note pitch is needed as all other attributes (duration, voice, etc.) will be
            taken from the target note.
        - After executing the command:
            - the selection will be cleared and the added note will be
                selected. That is, the selection will contain only the new added note.
            - the cursor will be pointing to added note.

        <b>Example</b>

        Consider an edition application in which the expected user interaction pattern is as follows:
        - First, user selects a note.
        - As a consequence, the application enables and disables allowed tools for the new context. In this
            example, as the user has selected a note the tool "add chord note" is enabled (among other tools).
        - The user then clicks on the "add chord note" tool and execution arrives to the following method:

        @code
        void CommandHandler::add_chord_note(const string& stepLetter, int octave)
        {
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                stringstream src;
                src << stepLetter << octave;
                string pitch = src.str();
	            string name = gettext("Add chord note") + "(" + pitch + ")";
	            SpInteractor->exec_command( new CmdAddChordNote(pitch, name) );
            }
        }
        @endcode
    */
    CmdAddChordNote(const string& pitch, const string& name="Add chord note");
    virtual ~CmdAddChordNote() {};

    int get_cursor_update_policy() override { return k_refresh; }
    int get_undo_policy() override { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() override { return k_sel_command_specific; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void update_selection(SelectionSet* pSelection) override;
    void log_command(ostream &logger) override;

};

//---------------------------------------------------------------------------------------
/** A command for inserting a note or a rest.

    See constructor for details.
*/
class CmdAddNoteRest : public DocCmdSimple
{
protected:
    ImoId m_idAt;
    string m_source;

public:
    /**
        This command adds a new note or rest at current cursor position.
        @param source LDP source code for the note or rest to insert.
        @param editMode Edition mode for executing the command. It must be a value from #EEditMode,
            either <i>k_edit_mode_replace</i> or <i>k_edit_mode_ripple</i>. See @ref edit-modes.
            <b>Attention:</b> Currently this parameter is ignored and the command always work in
            replace mode.
        @param name The displayable name for the command. If not specified or empty will default
            to "Add note" or "Add rest" depending on object to insert.

        <b>Remarks</b>
        - In ripple mode, the new note or rest will be inserted @b before the object currently
          pointed by the cursor. In replace mode, the new note or rest will replace the object
          currently pointed by the cursor.
        - After executing the command:
            - the selection will be cleared and the added note or rest will be
                selected. That is, the selection will contain only the new added note or rest.
            - In ripple mode, the cursor will not change its position. The inserted note/rest
              will be just behind the cursor. In replace mode the cursor will be pointing
              after the inserted note/rest.

        <b>Example</b>

        @code
        void CommandHandler::insert_note(string stepLetter, int octave, EAccidentals acc,
                                         ENoteType noteType, int dots,
                                         int voice, int staff)
        {
	        //insert a note at current cursor position

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                stringstream src;
                src << "(n ";
                if (acc != k_no_accidentals)
                    src << LdpExporter::accidentals_to_string(acc);
                src << stepLetter << octave
                    << " " << LdpExporter::notetype_to_string(noteType, dots)
                    << " v" << voice
                    << " p" << staff+1
                    << ")";

                string name = gettext("Insert note") + " (" + stepLetter + octave + ")";
                int editMode = k_edit_mode_replace;
                SpInteractor->exec_command( new CmdAddNoteRest(src.str(), editMode, name) );
            }
        }
        @endcode
    */
    CmdAddNoteRest(const string& source, int editMode, const string& name="");
    virtual ~CmdAddNoteRest() {};

    int get_cursor_update_policy() override { return k_refresh; }
    int get_undo_policy() override { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() override { return k_sel_command_specific; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    //temporary data and helper methods using it
    Document* m_pDoc;
    DocCursor* m_pCursor;
    ImoScore* m_pScore;
    ImoInstrument* m_pInstr;
    ScoreCursor* m_pSC;
    int m_instr;
    int m_newVoice;
    ImoNoteRest* m_pNewNR;
    TimeUnits m_insertionTime;
    TimeUnits m_newDuration;
    ImoStaffObj* m_pAt;
    list<OverlappedNoteRest*> m_overlaps;
    string m_finalSrc;
    list<ImoStaffObj*> m_insertedObjs;
    ImoNoteRest* m_pLastOverlapped;

    void get_data_about_insertion_point();
    void get_data_about_noterest_to_insert();
    void find_and_classify_overlapped_noterests();
    void determine_insertion_point();
    void add_new_note_to_existing_beam_if_necessary();
    void reduce_duration_of_overlapped_at_end();
    void remove_fully_overlapped();
    void insert_new_content();
    void reduce_duration_of_overlapped_at_start();
    void update_cursor();
    void update_selection(SelectionSet* pSelection) override;
    void clear_temporary_objects();
    void add_go_fwd_if_needed();

    //overrides and mandatory virtual methods
    void set_command_name();
    void log_command(ostream &logger) override;

};

//---------------------------------------------------------------------------------------
/** A command for adding a tie to the two selected notes.

    See constructor for details.
*/
class CmdAddTie : public DocCmdSimple
{
protected:
    ImoId m_startId;
    ImoId m_endId;
    ImoId m_tieId;

public:
    /**
        This command adds a tie to the two selected notes.
        @param name The displayable name for the command. If not specified will default to "Add tie".

        <b>Remarks</b>
        - The two notes to tie must be selected and must have the same pitch.
        - Before executing the command Lomse will check that the selection is valid for adding a tie.
            Otherwise the command will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            - the cursor will not change its position.

        <b>Example</b>

        @code
        void CommandHandler::add_tie()
        {
            //Tie the selected notes

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
	            string name = gettext("Add tie");
	            SpInteractor->exec_command( new CmdAddTie(name) );
	        }
        }
        @endcode
    */
    CmdAddTie(const string& name="Add tie");
    virtual ~CmdAddTie() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void log_command(ostream &logger) override;
};

//---------------------------------------------------------------------------------------
/** A command for creating a tuplet.

    See constructor for details.
*/
class CmdAddTuplet : public DocCmdSimple
{
protected:
    ImoId m_startId;
    ImoId m_endId;
    ImoId m_tupletId;
    string m_source;

public:
    /**
        This command adds a tuplet to the selected notes/rests.
        @param src LDP source code for the tuplet to add.
        @param name The displayable name for the command. If not specified will default to "Add tuplet".

        <b>Remarks</b>
        - The notes/rest that will form the tuplet must be selected. There could be other objects selected
          between the notes and rests (e.g. a barline).
        - Before executing the command Lomse will check that the selection is not empty.
            Otherwise the command will not be executed and will return a failure code.
        - Before asking to execute this command your application should do more checks (e.g. all selected notes and rests
            are in the same voice and instrument; Lomse should do this but currently it does not (see issue #39).
        - After executing the command:
            - the selection will not be changed.
            - the cursor will not change its position.

        <b>Example</b>

        @code
        void CommandHandler::add_triplet()
        {
            // Add a triplet to the selected notes/rests (there could be other objects selected
            // between the notes)
            //
            // Precondition:
            //      your application has checked that there are three notes/rest in the selection,
            //      that they are not in a tuplet, are consecutive, have the same duration and are
            //      in the same instrument and voice.

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                string name = gettext("Add triplet");
                SpInteractor->exec_command( new CmdAddTuplet("(t + 2 3)", name) );
	        }
        }
        @endcode
    */
    CmdAddTuplet(const string& src, const string& name="Add tuplet");
    virtual ~CmdAddTuplet() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void log_command(ostream &logger) override;
};

//---------------------------------------------------------------------------------------
/** A command to break a beam at the note/rest pointed by the cursor.

    See constructor for details.
*/
class CmdBreakBeam : public DocCmdSimple
{
protected:
    ImoId m_beforeId;

public:
    /**
        This command breaks a beam at the note/rest pointed by cursor.
        @param name The displayable name for the command. If not specified will default to "Break beam".

        <b>Remarks</b>
        - If the note pointed by the cursor is not beamed to the previous note, the command
            will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            - the cursor will not change its position.

        <b>Example</b>

        @code
        void CommandHandler::break_beam()
        {
            //Break a beam at the note/rest pointed by the cursor

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                ImoNoteRest* pNR = dynamic_cast<ImoNoteRest*>( m_cursor->get_pointee() );
                if (pNR)
                {
                    string name = gettext("Break a beam");
                    SpInteractor->exec_command( new CmdBreakBeam(name) );
                }
	        }
        }
        @endcode
    */
    CmdBreakBeam(const string& name="Break beam");
    virtual ~CmdBreakBeam() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void log_command(ostream &logger) override;
};

//---------------------------------------------------------------------------------------
/** A command for changing the accidentals of the selected notes.

    See constructor for details.
*/
class CmdChangeAccidentals : public DocCmdSimple
{
protected:
    EAccidentals m_acc;
    list<ImoId> m_notes;
    list<FPitch> m_oldPitch;

public:
    /**
        This command changes the accidentals of the selected notes.
        @param acc The new accidentals to put on the selected notes. It must be a value from
            enum #EAccidentals.
        @param name The displayable name for the command. If not specified will default to "Change accidentals".

        <b>Remarks</b>
        - If the selection is empty or does not contain notes, the command
            will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            - the cursor will not change its position.

        <b>Example</b>

        @code
        void CommandHandler::change_note_accidentals(EAccidentals acc)
        {
	        //change note accidentals for current selected notes

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
	            string name = gettext("Change note accidentals");
                SpInteractor->exec_command( new CmdChangeAccidentals(acc, name) )
	        }
        }
        @endcode
    */
    CmdChangeAccidentals(EAccidentals acc, const string& name="Change accidentals");
    virtual ~CmdChangeAccidentals() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void log_command(ostream &logger) override;
};

//---------------------------------------------------------------------------------------
/** A command for changing the value of an attribute.

    See constructors for details. There is a constructor for each type of value (string,
    double, int or colour).
*/
class CmdChangeAttribute : public DocCmdSimple
{
protected:
    ImoId           m_targetId;
    EImoAttribute   m_attrb;
    EDataType       m_dataType;
    string          m_oldString;
    string          m_newString;
    double          m_oldDouble;
    double          m_newDouble;
    int             m_oldInt;
    int             m_newInt;
    Color           m_oldColor;
    Color           m_newColor;

public:

    ///@{
    /**
        This command changes the value of an attribute on cursor pointed object.
        @param attrb    The attribute whose value is going to be changed. It must be a
            value from enum #EImoAttribute.
        @param value    The new value for the attribute.
        @param cmdName  The displayable name for the command. If not specified or
            empty will default to "Change attribute".

        <b>Remarks</b>
        - If target object is not valid (e.g. nullptr), the command
            will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            - the cursor will not change its position.
    */
    CmdChangeAttribute(EImoAttribute attrb, const string& value,
                       const string& cmdName="");
    CmdChangeAttribute(EImoAttribute attrb, double value,
                       const string& cmdName="");
    CmdChangeAttribute(EImoAttribute attrb, int value,
                       const string& cmdName="");
    CmdChangeAttribute(EImoAttribute attrb, Color value,
                       const string& cmdName="");
    ///@}


    /**
        This command changes the value of an attribute in the passed object.
        @param pImo Pointer to the object whose attribute is going to be changed.
        @param attrb The attribute whose value is going to be changed. It must be a value from
            enum #EImoAttribute.
        @param value An <i>string</i> with the new value for the attribute.
        @param cmdName The displayable name for the command. If not specified or empty will default to "Change attribute".

        <b>Remarks</b>
        - If target object is not valid (e.g. nullptr), the command
            will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            - the cursor will not change its position.

        <b>Example</b>

        @todo Replace parameter pImo to use object ID instead of a pointer (?)

        @code
        void CommandHandler::change_attribute(ImoObj* pImo, int attrb, const string& newValue)
        {
	        //change attribute value in given object

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                string name = gettext("Change string property");
                SpInteractor->exec_command( new CmdChangeAttribute(pImo, EImoAttribute(attrb), newValue) );
	        }
        }
        @endcode
    */
    CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb, const string& value,
                       const string& cmdName="");

    /**
        This command changes the value of an attribute in the passed object.
        @param pImo Pointer to the object whose attribute is going to be changed.
        @param attrb The attribute whose value is going to be changed. It must be a value from
            enum #EImoAttribute.
        @param value A <i>double</i> number with the new value for the attribute.
        @param cmdName The displayable name for the command. If not specified or empty will default to "Change attribute".

        <b>Remarks</b>
        - If target object is not valid (e.g. nullptr), the command
            will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            - the cursor will not change its position.

        @todo Replace parameter pImo to use object ID instead of a pointer (?)
    */
    CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb, double value,
                       const string& cmdName="");

    /**
        This command changes the value of an attribute in the passed object.
        @param pImo Pointer to the object whose attribute is going to be changed.
        @param attrb The attribute whose value is going to be changed. It must be a value from
            enum #EImoAttribute.
        @param value An <i>int</i> number with the new value for the attribute.
        @param cmdName The displayable name for the command. If not specified or empty will default to "Change attribute".

        <b>Remarks</b>
        - If target object is not valid (e.g. nullptr), the command
            will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            - the cursor will not change its position.

        @todo Replace parameter pImo to use object ID instead of a pointer (?)
    */
    CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb, int value,
                       const string& cmdName="");

    /**
        This command changes the value of an attribute in the passed object.
        @param pImo Pointer to the object whose attribute is going to be changed.
        @param attrb The attribute whose value is going to be changed. It must be a value from
            enum #EImoAttribute.
        @param value The new <i>Color</i> value for the attribute.
        @param cmdName The displayable name for the command. If not specified or empty will default to "Change attribute".

        <b>Remarks</b>
        - If target object is not valid (e.g. nullptr), the command
            will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            - the cursor will not change its position.

        @todo Replace parameter pImo to use object ID instead of a pointer (?)
    */
    CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb, Color value,
                       const string& cmdName="");

	/// Destructor
    virtual ~CmdChangeAttribute() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void set_default_name();
    void save_current_value(ImoObj* pImo);
    int set_target(ImoObj* pImo);

};

//---------------------------------------------------------------------------------------
/** A command for changing the number of dots in the selected notes and rests.

    See constructor for details.
*/
class CmdChangeDots : public DocCmdSimple
{
protected:
    int m_dots;
    list<ImoId> m_noteRests;
    list<int> m_oldDots;

public:
    /**
        This command changes the number of dots in the selected notes and rests.
        @param dots The new number of dots (0..n) to put in the notes and rests+.
        @param name The displayable name for the command. If not specified will default to "Change dots".

        <b>Remarks</b>
            - If the selection is empty or does not contain notes or rests, the command
                will not be executed and will return a failure code.
            - After executing the command:
                - the selection will not be changed.
                - the cursor will not change its position.

        <b>Example</b>

        @code
        void CommandHandler::change_dots(int dots)
        {
	        //change dots for current selected notes/rests

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                string name = gettext("Change note/rest dots");
                SpInteractor->exec_command( new CmdChangeDots(dots, name) );
	        }
        }
        @endcode
    */
    CmdChangeDots(int dots, const string& name="Change dots");
    virtual ~CmdChangeDots() {};

    int get_cursor_update_policy() override { return k_refresh; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

};

//---------------------------------------------------------------------------------------
/** A command for moving the cursor to a new position.

    See constructors for details. Each constructor offers a different alternative for specifying
    the new cursor position.
*/
class CmdCursor : public DocCmdSimple
{
protected:
    int             m_operation;
    ImoId           m_targetId;
    DocCursorState  m_curState;
    DocCursorState  m_targetState;
    int             m_measure;
    int             m_instrument;
    int             m_staff;
    TimeUnits       m_time;

public:

    /// This enum describes the posible actions for CmdCursor commands.
    enum ECursorAction {
        k_move_next=0,      ///< Move cursor to the next valid position.
        k_move_prev,        ///< Move cursor to the previous valid position.
        k_enter,            ///< When cursor is pointing to a top level element, this
                            ///<    action causes the cursor to enter into this top
                            ///<    level element and point to first sub-element.
        k_exit,             ///< When cursor is inside a top level element, this action
                            ///< causes the cursor to move up, out of this top level
                            ///< element, and to point to it.
        k_point_to,         ///< Move cursor to the specified element.
        k_to_state,         ///< Move cursor to the specified state.
        k_to_measure,       ///< When cursor is inside of a music score, this action
                            ///< moves the cursor to the specified measure.
        k_to_time,          ///< When cursor is inside of a music score, this action
                            ///<    moves the cursor to the specified time position.
        k_move_up,          ///< When cursor is inside of a music score, this action
                            ///<    moves the cursor to the previous staff or system,
                            ///<    pointing to the nearest note or rest to current vertical position.
        k_move_down,        ///< When cursor is inside of a music score, this action
                            ///<    moves the cursor to the next staff or system,
                            ///<    pointing to the nearest note or rest to current vertical position.
        k_cursor_dump,      ///< This action is restricted for debugging Lomse. Your application
                            ///<    should not use it.
    };

    /**
        This command moves the cursor to a new position, as specified by parameter <i>cmd</i>.
        @param cmd The action to be performed. It must be one of the following values from
            enum #ECursorAction: <i>k_move_next</i>, <i>k_move_prev</i>, <i>k_enter</i>,
            <i>k_exit</i>, <i>k_move_up</i>, <i>k_move_down</i> or <i>k_cursor_dump</i>.
        @param name The displayable name for the command. If not specified or empty
            will default to one of the following vaues, depending on the requested action:
            "Cursor: move next", "Cursor: move prev", "Cursor: move up", "Cursor: move down",
            "Cursor: enter element" or "Cursor: exit element".

        <b>Remarks</b>
            - After executing the command:
                - the selection will not be changed.
                - the cursor will point to a new position.
            - %CmdCursor is a non-reversible command, meaning that it is not saved in
                the undo/redo history and, therefore, undo and redo operations are not
                available for this command.

        <b>Example</b>

        @code
        bool CommandHandler::process_cursor_key_command(int keyCmd)
        {
	        //A key has been pressed in the keyboard. The command implied by the
            //pressed key is passed to this method. If it is a cursor command
            //it will be processed here and TRUE is returned.

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                //cursor is only valid if document edition is enabled
                if (!spInteractor->is_edition_enabled())
                    return false;

                switch (keyCmd)
                {
                    //cursor keys
                    case k_cmd_cursor_move_prev:
                        SpInteractor->exec_command( new CmdCursor(CmdCursor::k_move_prev) );
                        return true;

                    case k_cmd_cursor_move_next:
                        SpInteractor->exec_command( new CmdCursor(CmdCursor::k_move_next) );
                        return true;

                    case k_cmd_cursor_exit:
                        SpInteractor->exec_command( new CmdCursor(CmdCursor::k_exit) );
                        return true;

                    case k_cmd_cursor_enter:
                    {
                        if (!m_cursor->is_inside_terminal_node())
                        {
                            ImoObj* pImo = m_cursor->get_pointee();
                            if (pImo && pImo->is_score())
                            {
                                SpInteractor->exec_command( new CmdCursor(CmdCursor::k_enter) );
                                return true;
                            }
                        }
                        return false;
                    }

                    case k_cmd_cursor_to_next_measure:
                    case k_cmd_cursor_to_prev_measure:
                    case k_cmd_cursor_to_first_measure:
                    case k_cmd_cursor_to_last_measure:
                    {
                        ImoObj* pImo = m_cursor->get_parent_object();
                        if (pImo && pImo->is_score())
                        {
                            ScoreCursor* pSC =
                                static_cast<ScoreCursor*>(m_cursor->get_inner_cursor());
                            int measure = pSC->measure();
                            if (keyCmd == k_cmd_cursor_to_next_measure)
                                ++measure;
                            else if (keyCmd == k_cmd_cursor_to_prev_measure)
                            {
                                if (measure > 0) --measure;
                            }
                            else if (keyCmd == k_cmd_cursor_to_first_measure)
                                measure = 0;
                            else if (keyCmd == k_cmd_cursor_to_last_measure)
                                measure = 9999999;
                            else;

                            SpInteractor->exec_command( new CmdCursor(measure, -1, -1) );
                            return true;
                        }
                        return false;
                    }

                    case k_cmd_cursor_move_up:
                    case k_cmd_cursor_move_down:
                    {
                        ImoObj* pImo = m_cursor->get_parent_object();
                        if (pImo && pImo->is_score())
                        {
                            if (keyCmd == k_cmd_cursor_move_up)
                                SpInteractor->exec_command( new CmdCursor(CmdCursor::k_move_up) );
                            else
                                SpInteractor->exec_command( new CmdCursor(CmdCursor::k_move_down) );
                            return true;
                        }
                        return false;
                    }

                    default:
                        return false;
                }
	        }
            return false;
        }
        @endcode
    */
    CmdCursor(ECursorAction cmd, const string& name="");

    /**
        This command moves the cursor for pointing to element specified by parameter <i>id</i>.
        @param id The ID of the object that will be pointed by the cursor.
        @param name The displayable name for the command. If not specified or empty
            will default to "Cursor: point to".

        <b>Remarks</b>
            - After executing the command:
                - the selection will not be changed.
                - the cursor will point to a new position.
            - %CmdCursor is a non-reversible command, meaning that it is not saved in
                the undo/redo history and, therefore, undo and redo operations are not
                available for this command.

    */
    CmdCursor(ImoId id, const string& name="");

    /**
        When the cursor is inside a music score, this command moves the cursor for
            pointing to the first element (note, rest, clef, etc.)
            in the measure specified by parameter <i>measure</i>.
        @param measure The measure to move the cursor to (0..n).
        @param instr The instrument to move the cursor to. Value -1 means
            "remain in current instrumnent". For moving to another instrument use
            instrument number (0 ... num_instruments - 1).
        @param staff The staff to move the cursor to. Value -1 means
            "remain in current staff". For moving to another staff use
            staff number (0 ... num_staves - 1).
        @param name The displayable name for the command. If not specified or empty
            will default to "Cursor: jump to new place".

        <b>Remarks</b>
            - After executing the command:
                - the selection will not be changed.
                - the cursor will point to a new position.
            - %CmdCursor is a non-reversible command, meaning that it is not saved in
                the undo/redo history and, therefore, undo and redo operations are not
                available for this command.

        <b>Example</b>

        @code
        switch (keyCmd)
        {
            case k_cmd_cursor_to_next_measure:
            case k_cmd_cursor_to_prev_measure:
            case k_cmd_cursor_to_first_measure:
            case k_cmd_cursor_to_last_measure:
            {
                ImoObj* pImo = m_cursor->get_parent_object();
                if (pImo && pImo->is_score())
                {
                    ScoreCursor* pSC =
                        static_cast<ScoreCursor*>(m_cursor->get_inner_cursor());
                    int measure = pSC->measure();
                    if (keyCmd == k_cmd_cursor_to_next_measure)
                        ++measure;
                    else if (keyCmd == k_cmd_cursor_to_prev_measure)
                    {
                        if (measure > 0) --measure;
                    }
                    else if (keyCmd == k_cmd_cursor_to_first_measure)
                        measure = 0;
                    else if (keyCmd == k_cmd_cursor_to_last_measure)
                        measure = 9999999;
                    else;

                    SpInteractor->exec_command( new CmdCursor(measure, -1, -1) );
                    return true;
                }
                return false;
            }
            ...
        }
        @endcode
    */
    CmdCursor(int measure, int instr, int staff, const string& name="");

    /**
        When the cursor is inside a music score, this command moves the cursor for
            pointing to a time position equal than the specified by parameter <i>time</i>.
        @param time The new time position to move the cursor to.
        @param instr The instrument to move the cursor to. Value -1 means
            "remain in current instrument". For moving to another instrument use
            instrument number (0 ... num_instruments - 1).
        @param staff The staff to move the cursor to. Value -1 means
            "remain in current staff". For moving to another staff use
            staff number (0 ... num_staves - 1).
        @param name The displayable name for the command. If not specified or empty
            will default to "Cursor: jump to new place".

        <b>Remarks</b>
            - After executing the command:
                - the selection will not be changed.
                - the cursor will point to a new position.
            - %CmdCursor is a non-reversible command, meaning that it is not saved in
                the undo/redo history and, therefore, undo and redo operations are not
                available for this command.
    */
    CmdCursor(TimeUnits time, int instr, int staff, const string& name="");

    /**
        This command moves the cursor to the state specified by parameter <i>state</i>.
        @param state The new state for the cursor.
        @param name The displayable name for the command. If not specified or empty
            will default to "Cursor: jump to new place".

        <b>Remarks</b>
            - After executing the command:
                - the selection will not be changed.
                - the cursor will point to a new position.
            - %CmdCursor is a non-reversible command, meaning that it is not saved in
                the undo/redo history and, therefore, undo and redo operations are not
                available for this command.

    */
    CmdCursor(DocCursorState& state, const string& name="");
    virtual ~CmdCursor() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void set_default_name();
    void initialize();

};

//---------------------------------------------------------------------------------------
/** Abstract class, base for all delete commands.
*/
class CmdDelete : public DocCmdSimple
{
protected:
    //For recovering from a deletion, cursor must be moved to a safe place before
    //deletion. Therefore, before executing a CmdDelete, the cursor is moved before
    //the first object to be deleted. The ID of this object is saved here.
    ImoId m_cursorFinalId;

    CmdDelete(const string& name) : DocCmdSimple(name), m_cursorFinalId(k_no_imoid) {}

public:
    virtual ~CmdDelete() {}

protected:
    virtual void prepare_cursor_for_deletion(DocCursor* pCursor);

    friend class DocCommandExecuter;
    inline ImoId cursor_final_pos_id() { return m_cursorFinalId; }

};

//---------------------------------------------------------------------------------------
/** A command for deleting a block level object (e.g. paragraph, music score, header, etc.).

    See constructor for details.
*/
class CmdDeleteBlockLevelObj : public CmdDelete
{
protected:
    ImoId m_targetId;

public:
    /**
        This command deletes the block level object (e.g. paragraph, music score, header, etc.)
        pointed by the cursor.
        @param name The displayable name for the command. If not specified or empty will
            default to string "Delete " followed by the name of the object to delete.

        <b>Remarks</b>
        - If object pointed by cursor is not valid (e.g. nullptr or not block level object),
          the command will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            - the cursor will point to the object after the deleted one.
    */
    CmdDeleteBlockLevelObj(const string& name="");
    virtual ~CmdDeleteBlockLevelObj() {};

    int get_cursor_update_policy() override { return k_update_after_deletion; }
    int get_undo_policy() override { return k_undo_policy_full_checkpoint; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond
};

//---------------------------------------------------------------------------------------
/** A command for deleting a relation object (e.g. beam, chord, slur, tie, tuplet, etc.).

    See constructors for details.
*/
class CmdDeleteRelation : public CmdDelete
{
protected:
    int m_type;
    list<ImoId> m_relobjs;

public:
    /**
        This command deletes the first object in current selection. It must be a relation
        object (e.g. beam, chord, slur, tie, tuplet, etc.).
        @param name The displayable name for the command. If not specified or empty will
            default to string "Delete " followed by the name of the object to delete.

        <b>Remarks</b>
        - If the first object in current selection is not a relation object (imoRelObj)
          the command will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            - the cursor will not change its position.
    */
    CmdDeleteRelation(const string& name="");

    /**
        This command deletes all selected relation objects of type given by parameter <i>type</i>.
        @param type Type of relation object to insert. Must be one of the values for
            relation objects (ImoRelObj) in enum #EImoObjType.
        @param name The displayable name for the command. If not specified or empty will
            default to string "Delete " followed by the name of the object to delete.

        <b>Remarks</b>
        - If the first object in current selection is not a relation object (imoRelObj)
          the command will not be executed and will return a failure code.
        - After executing the command:
            - the selection will not be changed.
            @todo Check. Selection should only contain all other objects but not the deleted ones.
            - the cursor will not change its position.

        @code
        void CommandHandler::delete_tuplet()
        {
	        //Remove all selected tuplets

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                string name = gettext("Delete tuplet");
                SpInteractor->exec_command( new CmdDeleteRelation(k_imo_tuplet, name) );
	        }
        }
        @endcode
    */
    CmdDeleteRelation(int type, const string& name="");
    virtual ~CmdDeleteRelation() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

private:
    int set_score_id(Document* pDoc);

};

//---------------------------------------------------------------------------------------
/** A command for deleting all the selected objects.

    See constructor for details.
*/
class CmdDeleteSelection : public CmdDelete
{
protected:
    list<ImoId> m_idSO;     //StaffObjs to delete
    list<ImoId> m_idRO;     //RelObjs to delete
    list<ImoId> m_idAO;     //AuxObjs to delete
    list<ImoId> m_idOther;  //other objects to delete

public:
    /**
        This command deletes all selected objects.
        @param name The displayable name for the command. If not specified will
            default to "Delete selection".

        <b>Remarks</b>
        - If current selection is empty
          the command will not be executed and will return a failure code.
        - After executing the command:
            - the selection will be empty.
            - the cursor will not change its position unless it was pointing to one
              of the deleted objects. In this case, cursor will point to next remaining
              object after the one it was pointing.

        @code
        void CommandHandler::delete_selection()
        {
	        //Delete all objects currently selected.

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                string name = gettext("Delete selection");
                SpInteractor->exec_command( new CmdDeleteSelection(name) );
	        }
        }
        @endcode
    */
    CmdDeleteSelection(const string& name="Delete selection");
    virtual ~CmdDeleteSelection() {};

    int get_cursor_update_policy() override { return k_update_after_deletion; }
    int get_undo_policy() override { return k_undo_policy_full_checkpoint; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    list<ImoId> m_relIds;   //when the action is performed, all relations in deleted
                            //staffobjs are temporarily saved here

    void prepare_cursor_for_deletion(DocCursor* pCursor) override;
    bool is_going_to_be_deleted(ImoId id);
    void delete_staffobjs(Document* pDoc);
    void delete_relobjs(Document* pDoc);
    void delete_auxobjs(Document* pDoc);
    void delete_other(Document* pDoc);
    void delete_staffobj(ImoStaffObj* pImo);
    void reorganize_relations(Document* pDoc);

};

//---------------------------------------------------------------------------------------
/** A command for deleting the staff object (e.g. note, rest, clef, barline, etc.) pointed
    by the cursor.

    See constructor for details.
*/
class CmdDeleteStaffObj : public CmdDelete
{
protected:
    ImoId   m_id;

public:
    /**
        This command deletes the staff object (e.g. note, rest, clef, barline, etc.) pointed
        by the cursor.
        @param name The displayable name for the command. If not specified or empty will
            default to string "Delete " followed by the name of the object to delete.

        <b>Remarks</b>
        - If object pointed by cursor is not valid (e.g. nullptr or is not an staff object),
          the command will not be executed and will return a failure code.
        - After executing the command:
            - the selection will be empty.
            - the cursor will point to next valid position after the deleted object.

        @code
        void CommandHandler::delete_staffobj()
        {
	        //delete the StaffObj pointed by the cursor

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
	            //get object pointed by the cursor
                ImoStaffObj* pSO = dynamic_cast<ImoStaffObj*>( m_cursor->get_pointee() );

                //if no object, ignore command. e.g., user clicked 'Del' key on no object
                if (pSO)
                {
                    string name = gettext("Delete " + pSO->get_name() );
                    SpInteractor->exec_command( new CmdDeleteStaffObj(name)
                }
            }
        }
        @endcode
    */
    CmdDeleteStaffObj(const string& name="");
    virtual ~CmdDeleteStaffObj() {};

    int get_cursor_update_policy() override { return k_update_after_deletion; }
    int get_undo_policy() override { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond
};

//---------------------------------------------------------------------------------------
/** Abstract class, base for all insertion commands.
*/
class CmdInsert : public DocCmdSimple
{
protected:
    ImoId m_idAt;
    ImoId m_lastInsertedId;
    string m_source;

    CmdInsert(const string& name)
        : DocCmdSimple(name)
        , m_idAt(k_no_imoid)
        , m_lastInsertedId(k_no_imoid)
    {
    }

public:
    ~CmdInsert() {}

    /** Once the command is executed, invoking this method returns the ID assigned to
        the new inserted object. If the command inserts several objects this method
        only returns the ID of the last inserted object.

        @remarks If this method is invoked and the command has not yet been executed,
            it will return value <i>k_no_imoid</i>.
    */
    inline ImoId last_inserted_id() { return m_lastInsertedId; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    ///@endcond

protected:
    void remove_object(Document* pDoc, ImoId id);
};

//---------------------------------------------------------------------------------------
/** A command for inserting a new block level object (e.g. paragraph, music score, header, etc.).

    See constructors for details.
*/
class CmdInsertBlockLevelObj : public CmdInsert
{
protected:
    int m_blockType;
    bool m_fFromSource;

public:
    /**
        This command inserts a new block level object (e.g. paragraph, music score, header, etc.).
        @param type Type of block level object to insert. Must be one of the values for block level objects in enum #EImoObjType.
        @param name The displayable name for the command. If not specified or empty will be replaced by the string "Insert "
            followed by the name of the object, as implied by param @a type.

        <b>Remarks</b>
        - The new element will be inserted @b before the object currently pointed by the cursor.
        - After executing the command:
            - the selection will not be altered.
            - the cursor will not change its position. The inserted object will be just behind the cursor.

        <b>Example</b>

        Consider an edition application in which the expected user interaction pattern is as follows:
        - First, user creates a new empty document. As the document is empty the cursor points to 'end of document'.
        - Then the user clicks on the "add music score" tool and execution arrives to the following code:

        @code
        //insert a music score
        if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
        {
            src = "<ldpmusic>(score (vers 2.0)(instrument (musicData)))</ldpmusic>";
            SpInteractor->exec_command( new CmdInsertBlockLevelObj(src, "Add empty music score") );
        }
        @endcode
    */
    CmdInsertBlockLevelObj(int type, const string& name="");

    /**
        This command inserts a new block level object (e.g. paragraph, music score, header, image, etc.).
        @param source The source code of the element to insert. It must be in LDP format.
        @param name The displayable name for the command. If not specified or empty will be replaced by "Insert block".

        <b>Remarks</b>
        - The new element will be inserted @b before the object currently pointed by the cursor.
        - The new inserted object will be empty.
        - After executing the command:
            - the selection will not be altered.
            - the cursor will not change its position. The inserted object will be just behind the cursor.

        <b>Example</b>

        Consider an edition application in which the expected user interaction pattern is as follows:
        - First, user creates a new empty document. As the document is empty the cursor points to 'end of document'.
        - Then the user clicks on the "add paragraph" tool and execution arrives to the following code:

        @code
        //insert a paragraph
        if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            SpInteractor->exec_command( new CmdInsertBlockLevelObj(k_imo_para, "Add empty paragraph") );
        @endcode
    */
    CmdInsertBlockLevelObj(const string& source, const string& name="");
    virtual ~CmdInsertBlockLevelObj() {};

    int get_cursor_update_policy() override { return k_refresh; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void perform_action_from_source(Document* pDoc, DocCursor* pCursor);
    void perform_action_from_type(Document* pDoc, DocCursor* pCursor);

};

//---------------------------------------------------------------------------------------
/** A command for inserting a group of consecutive staff objects (e.g. note, rest, clef, barline, etc.).

    See constructor for details.
*/
class CmdInsertManyStaffObjs : public CmdInsert
{
protected:
    bool m_fSaved;

public:
    /**
        This command inserts, in a single operation, a group of consecutive staff objects
        (e.g. note, rest, clef, barline, etc.) having a common relation between them
        (e.g. a chord, a group of beamed notes, etc.) although it can also be used for
        inserting unrelated consecutive staff objects.
        @param source The source code of the elements to insert. It must be in LDP format.
        @param name The displayable name for the command. If not specified will be replaced
            by "Insert staff objects".

        <b>Remarks</b>
        - The new elements will be inserted @b before the object currently pointed by the cursor.
        - The command will return a failure code if the cursor is not inside a music score.
        - After executing the command:
            - the selection will not be altered.
            - the cursor will not change its position. The last inserted object will be just behind the cursor.
    */
    CmdInsertManyStaffObjs(const string& source,
                           const string& name="Insert staff objects");

    virtual ~CmdInsertManyStaffObjs() {};

    int get_cursor_update_policy() override { return k_refresh; }
    int get_undo_policy() override { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void save_source_code_with_ids(Document* pDoc, const list<ImoStaffObj*>& objects);

};

//---------------------------------------------------------------------------------------
/** A command for inserting one staff object (e.g. a note, rest, clef, barline, etc.).

    See constructor for details.
*/
class CmdInsertStaffObj : public CmdInsert
{
protected:

public:
    /**
        This command inserts one staff object (e.g. a note, rest, clef, barline, etc.) before
        object pointed by the cursor.
        @param source The source code of the elements to insert. It must be in LDP format.
        @param name The displayable name for the command. If not specified or empty will be replaced
            by the string "Insert " followed by the name of the inserted object.

        <b>Remarks</b>
        - The new element will be inserted @b before the object currently pointed by the cursor.
        - The command will return a failure code if the cursor is not inside a music score.
        - After executing the command:
            - the selection will not be altered.
            - the cursor will not change its position. The inserted object will be just behind the cursor.

        <b>Example</b>

        @code
        void CommandHandler::insert_clef(int clefType, int staff)
        {
	        //insert a Clef at current cursor position

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                stringstream src;
                src << "(clef "
                    << LdpExporter::clef_type_to_ldp(clefType)
                    << " p"
                    << staff+1
                    << ")";
                string name = gettext("Insert clef");
                SpInteractor->exec_command( new CmdInsertStaffObj(src.str(), name) );
            }
        }
        @endcode
    */
    CmdInsertStaffObj(const string& source, const string& name="");
    virtual ~CmdInsertStaffObj() {};

    int get_cursor_update_policy() override { return k_refresh; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond
};

//---------------------------------------------------------------------------------------
/** A command for creating and joining beams, and for adding notes to a beam.

    See constructor for details.
*/
class CmdJoinBeam : public DocCmdSimple
{
protected:
    list<ImoId> m_noteRests;

public:
    /**
        This command creates and joins beams, and adds notes to a beam, depending on the content of
        current selection. That is, all selected notes will form a single beamed group.
        @param name The displayable name for the command. If not specified will be replaced
            by the string "Join beam".

        <b>Remarks</b>
        - The command will return a failure code if the selection is empty or the selected notes can not
            be on the same beam (e.g. notes in different voices or different instruments).
        - After executing the command:
            - the selection will not be altered.
            - the cursor will not change its position.

        <b>Example</b>

        @code
        void CommandHandler::join_beam()
        {
            //depending on current selection, either:
            // - create a beamed group with the selected notes,
            // - join two or more beamed groups
            // - or add a note to a beamed group

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
	            string name = gettext("Add beam");
	            SpInteractor->exec_command( new CmdJoinBeam(name) );
            }
        }
        @endcode
    */
    CmdJoinBeam(const string& name="Join beam");
    virtual ~CmdJoinBeam() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_full_checkpoint; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void log_command(ostream &logger) override;
};

//---------------------------------------------------------------------------------------
/**
    A command for moving an object to another position.
	This command is not fully coded. Current code assumes that the object is a Tie and
	the selected point is the first bezier

    See constructor for details.
*/
class CmdMoveObjectPoint : public DocCmdSimple
{
protected:
    ImoId   m_targetId;
    int     m_pointIndex;
    TPoint  m_oldPos;
    UPoint  m_shift;

public:
    /** This command moves an object to another position.
		@warning This command is not fully coded. Current code assumes that the object
            is a Tie and the selected point is the first bezier point.

        @param pointIndex   The control point (handler) number (0...n-1) to which
            this command refers to.
        @param shift    The shift (in logical units) to apply to the handler.
        @param name     The displayable name for the command. If not specified will be
            replaced by the string "Move object point".
    */
    CmdMoveObjectPoint(int pointIndex, UPoint shift,
                       const string& name="Move object point");
    virtual ~CmdMoveObjectPoint() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond
};

//---------------------------------------------------------------------------------------
/** A command for modifying current set of selected objects.

    See constructor for details.
*/
class CmdSelection : public DocCmdSimple
{
protected:
    int             m_operation;
    ImoId           m_targetId;
//    DocCursorState  m_curState;
//    DocCursorState  m_targetState;
    SelectionSet*   m_pSelection;

public:
    /// This enum describes the possible actions for CmdSelection commands.
    enum ESelectionAction {
        k_set=0,    ///< Set selection: clear current selection and add the objects in the command.
        k_add,      ///< Increment the selection by adding the objects in the command.
        k_remove,   ///< Remove the objects in the command from the selection.
        k_clear,    ///< Empty the selection set.
    };

    /**
        This command clears the set of selected objects.
        @param cmd The type of action to do on the selection set. Must be CmdSelection::k_clear
        @param name The displayable name for the command. If not specified or empty will be replaced
            by "Selection: clear selection".

        <b>Remarks</b>
        - After executing the command:
            - the selection will be empty.
            - the cursor will not change its position.

        As parameter <i>cmd</i> must be always <i>k_clear</i>, a convenience class
        CmdClearSelection(const string& name="") has been defined.
    */
    CmdSelection(int cmd, const string& name="");

    /**
        This command changes the content of the set of selected objects.
        @param cmd The type of action to do on the selection set. Must be a value
            from enum #ESelectionAction.
        @param id The ID of the object to add to or remove from the selection set.
        @param name The displayable name for the command. If not specified or empty
            will be replaced by one of the following strings, depending on value for
            parameter <i>cmd</i>:
            - k_set: "Selection: set selection".
            - k_add: "Selection: add obj. to selection".
            - k_remove: "Selection: remove obj. from selection".
            - k_clear: "Selection: clear selection"

        <b>Remarks</b>
        - After executing the command:
            - the selection will be modified as implied by the command.
            - the cursor will not change its position.

        <b>Example</b>

        @code
        void CommandHandler::select_object(ImoId id, bool fClearSelection)
        {
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
	            string name = gettext("Select object");
                int op = fClearSelection ? CmdSelection::k_set : CmdSelection::k_add;
	            SpInteractor->exec_command( new CmdSelection(op, id, name) );
            }
        }
        @endcode
    */
    CmdSelection(int cmd, ImoId id, const string& name="");
    //CmdSelection(DocCursorState& state, const string& name="");
    virtual ~CmdSelection() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:
    void set_default_name();
    void initialize();

};

////Alias for CmdSelection(k_clear, x) commands
//#define CmdClearSelection( name )   CmdSelection(ESelectionAction::k_clear, name)
//#define CmdClearSelection()         CmdSelection(ESelectionAction::k_clear)


//---------------------------------------------------------------------------------------
/** A command for clearing the current set of selected objects.

    See constructor for details.
*/
class CmdClearSelection : public CmdSelection
{
public:
    /**
        This command clears the set of selected objects.
        @param name The displayable name for the command. If not specified or empty
            will be replaced by "Selection: clear selection".

        <b>Remarks</b>
        - After executing the command:
            - the selection will be empty.
            - the cursor will not change its position.

        This command is just a convenience class for a CmdSelection command using the
        parameter <i>k_clear</i>.
    */
    CmdClearSelection(const string& name="")
        : CmdSelection(ESelectionAction::k_clear, name)
    {
    }
    virtual ~CmdClearSelection() {}

};


//---------------------------------------------------------------------------------------
/** Abstract class, base for all transposition commands
*/
class CmdTranspose : public DocCmdSimple
{
protected:
    list<ImoId> m_notes;
    list<ImoId> m_keys;

    CmdTranspose(const string& name="");

public:

    virtual ~CmdTranspose() {};

    int get_cursor_update_policy() override { return k_do_nothing; }
    int get_undo_policy() override { return k_undo_policy_specific; }
    int get_selection_update_policy() override { return k_sel_do_nothing; }

    ///@cond INTERNALS
    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection) override;
    int perform_action(Document* pDoc, DocCursor* pCursor) override;
    void undo_action(Document* pDoc, DocCursor* pCursor) override;
    ///@endcond

protected:

    //methods to be overriden as needed in derived classes
    virtual void transpose_note(ImoNote* pNote) = 0;
    virtual void transpose_note_back(ImoNote* pNote) = 0;
    virtual void transpose_key(ImoKeySignature* UNUSED(pKey)) {};
    virtual void transpose_key_back(ImoKeySignature* UNUSED(pKey)) {};

    //helper methods
    void transpose_chromatically(ImoNote* pNote, FIntval interval, bool fUp);

};


//---------------------------------------------------------------------------------------
/** A command for applying a chromatic transposition to the score or to a selection.

    See constructor for details.
*/
class CmdTransposeChromatically : public CmdTranspose
{
protected:
    FIntval m_interval;
    bool m_fUp;

public:

    /**
        This command shifts chromatically every pitch, up or down, by the interval you
        specify, adding or subtracting accidentals as necessary to maintain original
        intervals between notes.
        The command applies only to the notes in the current selection set.

        This kind of transposition has nothing to do with the key signature which
        remains unchanged.

        @param interval The interval by which you want the selected music transposed.
            If the interval is negative, the direction of the transposition will be
            'down'; otherwise it will be 'up'.

        @param name The displayable name for the command. If not specified or empty
            will be replaced by "Chromatic transposition".

        <b>Remarks</b>
        - If the selection is empty or does not contain notes, the command
            will not be executed and will return a failure code.
        - After executing the command:
            - the selection set will be unmodified.
            - the cursor will not change its position.

        <b>Example</b>

        @code
        void CommandHandler::transpose(FIntval interval)
        {
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
	            string name = gettext("Chromatic transposition");
	            SpInteractor->exec_command(
                    new CmdTransposeChromatically(interval, name) );
            }
        }
        @endcode
    */
    CmdTransposeChromatically(FIntval interval, const string& name="");

    virtual ~CmdTransposeChromatically() {};

protected:
    //mandatory overrides
    virtual void transpose_note(ImoNote* pNote) override;
    virtual void transpose_note_back(ImoNote* pNote) override;

};

//---------------------------------------------------------------------------------------
/** A command for applying a diatonic transposition to the score or to a selection.

    See constructor for details.
*/
class CmdTransposeDiatonically : public CmdTranspose
{
protected:
    int m_steps;
    bool m_fUp;

public:

    /**
        This command shifts diatonically every pitch, up or down, by the number of steps
        you specify. Only note steps changed and displayed accidentals are maintained.
        The command applies only to the notes in the current selection set.

        This kind of transposition has nothing to do with the key signature which
        remains unchanged.

        @param steps The interval by which you want the selected music transposed.
        @param fUp  Boolean for choosing the direction of the transposition: value
            @true means 'up', value @false means 'down'.

        @param name The displayable name for the command. If not specified or empty
            will be replaced by "Diatonic transposition".

        <b>Remarks</b>
        - If the selection is empty or does not contain notes, the command
            will not be executed and will return a failure code.
        - After executing the command:
            - the selection set will be unmodified.
            - the cursor will not change its position.

        <b>Example</b>

        @code
        void CommandHandler::transpose(int steps, bool fUp=true)
        {
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
	            string name = gettext("Diatonic transposition");
	            SpInteractor->exec_command(
                    new CmdTransposeDiatonically(steps, fUp, name) );
            }
        }
        @endcode
    */
    CmdTransposeDiatonically(int steps, bool fUp=true, const string& name="");

    virtual ~CmdTransposeDiatonically() {};

protected:
    //mandatory overrides
    virtual void transpose_note(ImoNote* pNote) override;
    virtual void transpose_note_back(ImoNote* pNote) override;

    //helper
    void transpose(ImoNote* pNote, bool fUp);
};

//---------------------------------------------------------------------------------------
/** A command for transposing a selection to a different key signature.

    Transposing a melody to a new key signature is a two steps process: replace the
    key signature symbol by the new key signature and transpose the affected notes,
    chromatically, by the interval between the new key and the old key signatures.

    Therefore, for transposing by key no special command is needed, just
    CmdTransposeChromatically; and also a method to compute the interval between two key
    signatures. Class KeyUtilities provides three methods for this:
        KeyUtilities::up_interval()
        KeyUtilities::down_interval()
        KeyUtilities::closest_interval()

    Take into account that a command for transposing to a different key signature can
    not be a general basic command because it must deal with notes and with keys, and
    how to deal with them is an application behaviour decision that can not be
    generalized. There are many scenarios to consider:
    - scores with key changes;
    - transposing instruments with a different key;
    - scope of the command: the command must affect to all the score? or only to a
      selection? (e.g. selected notes, a certain number of measures, other);
    - what to do when in the scope of the command there is a key signature change?:
        - apply the same transposition interval to any key signature found in the
          selection set,
        - stop transposing,
        - other;
    - what to do when not all notes are transposed:
        - nothing,
        - insert the old key signature after the last transposed note,
        - other.

    Because all of these considerations and possible application scenarios, Lomse can
    not provide a generic key transposition command. Nevertheless, for convenience, the
    CmdTransposeKey command has been created to be used in simple cases.

    See constructor for details.
*/
class CmdTransposeKey : public CmdTranspose
{
protected:
    FIntval m_interval;
    bool m_fUp;

public:

    /**
        This command changes all the keys in the selection, and transposes chromatically
        all the notes in the selection.

        @param interval The interval by which you want the selected music transposed.
            If the interval is negative, the direction of the transposition will be
            'down'; otherwise it will be 'up'.
        @param name The displayable name for the command. If not specified or empty
            will be replaced by "Transpose key signature".

        <b>Remarks</b>
        - If the selection does not contain key signatures, only the notes will be
            transposed.
        - If the selection only contains key signatures they will be changed but no notes
            will be transposed.
        - After executing the command:
            - the selection set will be unmodified.
            - the cursor will not change its position.

        <b>Example</b>

        @code
        void CommandHandler::transpose(FIntval interval)
        {
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
	            string name = gettext("Transpose key signature");
	            SpInteractor->exec_command(
                    new CmdTransposeKey(interval) );
            }
        }
        @endcode
    */
    CmdTransposeKey(FIntval interval, const string& name="");

    virtual ~CmdTransposeKey() {};

protected:
    //mandatory overrides
    virtual void transpose_note(ImoNote* pNote) override;
    virtual void transpose_note_back(ImoNote* pNote) override;
    virtual void transpose_key(ImoKeySignature* pKey) override;
    virtual void transpose_key_back(ImoKeySignature* pKey) override;

};


}   //namespace lomse

#endif      //__LOMSE_COMMAND_H__

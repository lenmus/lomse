//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2015 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_COMMAND_H__
#define __LOMSE_COMMAND_H__

#include "lomse_stack.h"
#include "lomse_internal_model.h"
#include "lomse_document_cursor.h"
#include "lomse_pitch.h"            //EAccidentals
#include "lomse_im_attributes.h"
#include "lomse_selections.h"

#include <sstream>
using namespace std;

namespace lomse
{

//forward declarations
class Document;
class SelectionSet;
class DocCommandExecuter;

//edition modes
enum EEditMode
{
    k_edit_mode_replace = 0,
    k_edit_mode_ripple,
};


//---------------------------------------------------------------------------------------
// interface for any command to modify the document
class DocCommand
{
protected:
    string m_name;          //displayable name for undo/redo actions display
    string m_checkpoint;    //checkpoint data
    ImoId m_idChk;          //id for target object in case of partial checkpoint
    string m_error;
    uint_least16_t m_flags;

    enum {
        k_reversible                    = 0x0001,
        k_recordable                    = 0x0002,
        k_target_set_in_constructor     = 0x0004,
        k_included_in_composite_cmd     = 0x0008,
    };

    DocCommand(const string& name) : m_name(name), m_idChk(k_no_imoid), m_flags(0) {}

public:
    virtual ~DocCommand() {}

    //cursor update policy to be used with this command
    enum {
        k_do_nothing=0,
        k_update_after_deletion,
        k_update_after_insertion,
        k_refresh,
    };
    virtual int get_cursor_update_policy() = 0;

    //undo policy used by this command
    enum {
        k_undo_policy_full_checkpoint=0,
        k_undo_policy_partial_checkpoint,
        k_undo_policy_specific,
    };
    virtual int get_undo_policy() = 0;

    //selection set update policy to be used with this command
    enum {
        k_sel_do_nothing=0,
        k_sel_clear,
        k_sel_command_specific,
    };
    virtual int get_selection_update_policy() = 0;

    //information
    inline std::string get_name() { return m_name; }
    inline bool is_reversible() { return m_flags &  k_reversible; }
    inline bool is_recordable() { return m_flags &  k_recordable; }
    inline bool is_target_set_in_constructor() { return m_flags &  k_target_set_in_constructor; }
    inline bool is_included_in_composite_cmd() { return m_flags &  k_included_in_composite_cmd; }
    inline string get_error() { return m_error; }
    virtual bool is_composite() = 0;
    virtual void update_selection(SelectionSet* pSelection) {}

    //settings
    inline void mark_as_included_in_composite_cmd() { m_flags |= k_included_in_composite_cmd; }

    //actions
    virtual int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection)=0;
    virtual int perform_action(Document* pDoc, DocCursor* pCursor)=0;
    virtual void undo_action(Document* pDoc, DocCursor* pCursor);

protected:
    void create_checkpoint(Document* pDoc);
    void log_forensic_data(Document* pDoc, DocCursor* pCursor);
    void set_command_name(const string& name, ImoObj* pImo);
    int validate_source(const string& source);
    virtual void log_command(ofstream &logger);

};

//---------------------------------------------------------------------------------------
// interface for any simple command (atomic)
class DocCmdSimple : public DocCommand
{
protected:

    DocCmdSimple(const string& name) : DocCommand(name) {}

public:
    virtual ~DocCmdSimple() {}

    bool is_composite() { return false; }

};

//---------------------------------------------------------------------------------------
// any composite command
class DocCmdComposite : public DocCommand
{
protected:
    list<DocCommand*> m_commands;
    int m_undoPolicy;

public:
    DocCmdComposite(const string& name);
    virtual ~DocCmdComposite();

    //change default settings
    inline void mark_as_no_reversible() { m_flags &= ~k_reversible; }
    inline void mark_as_no_recordable() { m_flags &= ~k_recordable; }
    inline void mark_as_target_set_in_constructor() { m_flags |= k_target_set_in_constructor; }

    //children commands
    void add_child_command(DocCommand* pCmd);

    //mandatory interface implementation
    virtual int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    virtual int perform_action(Document* pDoc, DocCursor* pCursor);
    virtual void undo_action(Document* pDoc, DocCursor* pCursor);

    //overrides
    int get_cursor_update_policy() { return k_do_nothing; }
    int get_undo_policy() { return m_undoPolicy; }
    int get_selection_update_policy() { return k_sel_do_nothing; }
    bool is_composite() { return true; }

    //operations delegated by DocCommandExecuter
    void update_cursor(DocCursor* pCursor, DocCommandExecuter* pExecuter);
    void update_selection(SelectionSet* pSelection, DocCommandExecuter* pExecuter);

protected:

};


//---------------------------------------------------------------------------------------
// undo/redo element: command and cursor
class UndoElement
{
public:
    DocCommand*         pCmd;
    DocCursorState      cursorState;
    SelectionState      selState;

public:
    UndoElement(DocCommand* cmd, DocCursorState state, SelectionState sel)
        : pCmd(cmd), cursorState(state), selState(sel)
    {
    }
};



//---------------------------------------------------------------------------------------
// Helper, to manage the undo/redo stack
typedef UndoableStack< UndoElement* >   UndoStack;


//---------------------------------------------------------------------------------------
// Keeps the stack of executed commands and performs undo/redo
class DocCommandExecuter
{
private:
    Document*   m_pDoc;
    UndoStack   m_stack;
    string      m_error;

public:
    DocCommandExecuter(Document* target);
    virtual ~DocCommandExecuter() {}
    virtual int execute(DocCursor* pCursor, DocCommand* pCmd,
                        SelectionSet* pSelection);
    virtual void undo(DocCursor* pCursor, SelectionSet* pSelection);
    virtual void redo(DocCursor* pCursor, SelectionSet* pSelection);
    inline string get_error() { return m_error; }

    //info
    bool is_undo_possible() { return m_stack.size() > 0; }
    bool is_redo_possible() { return m_stack.history_size() > 0; }

    virtual size_t undo_stack_size() { return m_stack.size(); }

protected:
    friend class DocCmdComposite;
    void update_cursor(DocCursor* pCursor, DocCommand* pCmd);
    void update_selection(SelectionSet* pSelection, DocCommand* pCmd);

};

//---------------------------------------------------------------------------------------
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
    CmdAddChordNote(const string& pitch, const string& name="Add chord note");
    virtual ~CmdAddChordNote() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_refresh; }
    int get_undo_policy() { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() { return k_sel_command_specific; }

protected:
    void update_selection(SelectionSet* pSelection);
    void log_command(ofstream &logger);

};

//---------------------------------------------------------------------------------------
class CmdAddNoteRest : public DocCmdSimple
{
protected:
    ImoId m_idAt;
    string m_source;

public:
    CmdAddNoteRest(const string& source, int editMode, const string& name="");
    virtual ~CmdAddNoteRest() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    int get_undo_policy() { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() { return k_sel_command_specific; }

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
    void update_selection(SelectionSet* pSelection);
    void clear_temporary_objects();
    void add_go_fwd_if_needed();

    //overrides and mandatory virtual methods
    void set_command_name();
    void log_command(ofstream &logger);

};

//---------------------------------------------------------------------------------------
class CmdAddTie : public DocCmdSimple
{
protected:
    ImoId m_startId;
    ImoId m_endId;
    ImoId m_tieId;

public:
    CmdAddTie(const string& name="Add tie");
    virtual ~CmdAddTie() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_undo_policy() { return k_undo_policy_specific; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    void log_command(ofstream &logger);
};

//---------------------------------------------------------------------------------------
class CmdAddTuplet : public DocCmdSimple
{
protected:
    ImoId m_startId;
    ImoId m_endId;
    ImoId m_tupletId;
    string m_source;

public:
    CmdAddTuplet(const string& src, const string& name="Add tuplet");
    virtual ~CmdAddTuplet() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_undo_policy() { return k_undo_policy_specific; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    void log_command(ofstream &logger);
};

//---------------------------------------------------------------------------------------
class CmdBreakBeam : public DocCmdSimple
{
protected:
    ImoId m_beforeId;

public:
    CmdBreakBeam(const string& name="Break beam");
    virtual ~CmdBreakBeam() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    int get_undo_policy() { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    void log_command(ofstream &logger);
};

//---------------------------------------------------------------------------------------
class CmdChangeAccidentals : public DocCmdSimple
{
protected:
    EAccidentals m_acc;
    list<ImoId> m_notes;
    list<FPitch> m_oldPitch;

public:
    CmdChangeAccidentals(EAccidentals acc, const string& name="Change accidentals");
    virtual ~CmdChangeAccidentals() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_undo_policy() { return k_undo_policy_specific; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    void log_command(ofstream &logger);
};

//---------------------------------------------------------------------------------------
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
    CmdChangeAttribute(EImoAttribute attrb, const string& value,
                       const string& cmdName="");
    CmdChangeAttribute(EImoAttribute attrb, double value,
                       const string& cmdName="");
    CmdChangeAttribute(EImoAttribute attrb, int value,
                       const string& cmdName="");
    CmdChangeAttribute(EImoAttribute attrb, Color value,
                       const string& cmdName="");

    CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb, const string& value,
                       const string& cmdName="");
    CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb, double value,
                       const string& cmdName="");
    CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb, int value,
                       const string& cmdName="");
    CmdChangeAttribute(ImoObj* pImo, EImoAttribute attrb, Color value,
                       const string& cmdName="");

    virtual ~CmdChangeAttribute() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    int get_undo_policy() { return k_undo_policy_specific; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    void set_default_name();
    void save_current_value(ImoObj* pImo);
    int set_target(ImoObj* pImo);

};

//---------------------------------------------------------------------------------------
class CmdChangeDots : public DocCmdSimple
{
protected:
    int m_dots;
    list<ImoId> m_noteRests;
    list<int> m_oldDots;

public:
    CmdChangeDots(int dots, const string& name="Change dots");
    virtual ~CmdChangeDots() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_refresh; }
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_undo_policy() { return k_undo_policy_specific; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

};

//---------------------------------------------------------------------------------------
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
    CmdCursor(int cmd, const string& name="");
    CmdCursor(int cmd, ImoId id, const string& name="");
    CmdCursor(int measure, int instr, int staff, const string& name="");
    CmdCursor(TimeUnits time, int instr, int staff, const string& name="");
    CmdCursor(DocCursorState& state, const string& name="");
    virtual ~CmdCursor() {};

    enum { k_move_next=0, k_move_prev, k_enter, k_exit, k_point_to,
           k_to_state, k_to_measure, k_to_time, k_move_up, k_move_down,
           k_cursor_dump, };

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    int get_undo_policy() { return k_undo_policy_specific; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    void set_default_name();
    void initialize();

};

//---------------------------------------------------------------------------------------
// abstract, base class for all delete commands
class CmdDelete : public DocCmdSimple
{
protected:
    ImoId m_cursorFinalId;

    CmdDelete(const string& name) : DocCmdSimple(name), m_cursorFinalId(k_no_imoid) {}

public:
    virtual ~CmdDelete() {}

    inline ImoId cursor_final_pos_id() { return m_cursorFinalId; }

protected:
    virtual void prepare_cursor_for_deletion(DocCursor* pCursor);
};

//---------------------------------------------------------------------------------------
class CmdDeleteBlockLevelObj : public CmdDelete
{
protected:
    ImoId m_targetId;

public:
    CmdDeleteBlockLevelObj(const string& name="");
    virtual ~CmdDeleteBlockLevelObj() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_update_after_deletion; }
    int get_undo_policy() { return k_undo_policy_full_checkpoint; }
    int get_selection_update_policy() { return k_sel_do_nothing; }
};

//---------------------------------------------------------------------------------------
class CmdDeleteRelation : public CmdDelete
{
protected:
    int m_type;
    list<ImoId> m_relobjs;

public:
    CmdDeleteRelation(const string& name="");
    CmdDeleteRelation(int type, const string& name="");
    virtual ~CmdDeleteRelation() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    int get_undo_policy() { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

private:
    int set_score_id(Document* pDoc);

};

//---------------------------------------------------------------------------------------
class CmdDeleteSelection : public CmdDelete
{
protected:
    list<ImoId> m_idSO;     //StaffObjs to delete
    list<ImoId> m_idRO;     //RelObjs to delete
    list<ImoId> m_idAO;     //AuxObjs to delete
    list<ImoId> m_idOther;  //other objects to delete

public:
    CmdDeleteSelection(const string& name="Delete selection");
    virtual ~CmdDeleteSelection() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_update_after_deletion; }
    int get_undo_policy() { return k_undo_policy_full_checkpoint; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    list<ImoId> m_relIds;   //when the action is performed, all relations in deleted
                            //staffobjs are temporarily saved here

    void prepare_cursor_for_deletion(DocCursor* pCursor);
    bool is_going_to_be_deleted(ImoId id);
    void delete_staffobjs(Document* pDoc);
    void delete_relobjs(Document* pDoc);
    void delete_auxobjs(Document* pDoc);
    void delete_other(Document* pDoc);
    void delete_staffobj(ImoStaffObj* pImo);
    void reorganize_relations(Document* pDoc);

};

//---------------------------------------------------------------------------------------
class CmdDeleteStaffObj : public CmdDelete
{
protected:
    ImoId   m_id;

public:
    CmdDeleteStaffObj(const string& name="");
    virtual ~CmdDeleteStaffObj() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_update_after_deletion; }
    int get_undo_policy() { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() { return k_sel_do_nothing; }
};

//---------------------------------------------------------------------------------------
// abstract, base for all CmdInsert commands
class CmdInsert : public DocCmdSimple
{
protected:
    ImoId m_idAt;
    ImoId m_lastInsertedId;
    string m_source;

    CmdInsert(const string& name) : DocCmdSimple(name), m_lastInsertedId(k_no_imoid) {}

public:
    ~CmdInsert() {}

    virtual int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    inline ImoId last_inserted_id() { return m_lastInsertedId; }

protected:
    void remove_object(Document* pDoc, ImoId id);
};

//---------------------------------------------------------------------------------------
class CmdInsertBlockLevelObj : public CmdInsert
{
protected:
    int m_blockType;
    bool m_fFromSource;

public:
    CmdInsertBlockLevelObj(int type, const string& name="");
    CmdInsertBlockLevelObj(const string& source, const string& name="");
    virtual ~CmdInsertBlockLevelObj() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_refresh; }  //k_update_after_insertion; }
    int get_undo_policy() { return k_undo_policy_specific; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    void perform_action_from_source(Document* pDoc, DocCursor* pCursor);
    void perform_action_from_type(Document* pDoc, DocCursor* pCursor);

};

//---------------------------------------------------------------------------------------
class CmdInsertManyStaffObjs : public CmdInsert
{
protected:
    bool m_fSaved;

public:
    CmdInsertManyStaffObjs(const string& source,
                           const string& name="Insert staff objects");
    virtual ~CmdInsertManyStaffObjs() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_refresh; }  //k_update_after_insertion; }
    int get_undo_policy() { return k_undo_policy_partial_checkpoint; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    void save_source_code_with_ids(Document* pDoc, const list<ImoStaffObj*>& objects);

};

//---------------------------------------------------------------------------------------
class CmdInsertStaffObj : public CmdInsert
{
protected:

public:
    CmdInsertStaffObj(const string& source, const string& name="");
    virtual ~CmdInsertStaffObj() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_refresh; }  //k_update_after_insertion; }
    int get_undo_policy() { return k_undo_policy_specific; }
    int get_selection_update_policy() { return k_sel_do_nothing; }
};

//---------------------------------------------------------------------------------------
class CmdJoinBeam : public DocCmdSimple
{
protected:
    list<ImoId> m_noteRests;

public:
    CmdJoinBeam(const string& name="Join beam");
    virtual ~CmdJoinBeam() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    int get_undo_policy() { return k_undo_policy_full_checkpoint; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    void log_command(ofstream &logger);
};

//---------------------------------------------------------------------------------------
class CmdMoveObjectPoint : public DocCmdSimple
{
protected:
    ImoId   m_targetId;
    int     m_pointIndex;
    TPoint  m_oldPos;
    UPoint  m_shift;

public:
    CmdMoveObjectPoint(int pointIndex, UPoint shift,
                       const string& name="Move object point");
    virtual ~CmdMoveObjectPoint() {};

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    int get_undo_policy() { return k_undo_policy_specific; }
    int get_selection_update_policy() { return k_sel_do_nothing; }
};

//---------------------------------------------------------------------------------------
class CmdSelection : public DocCmdSimple
{
protected:
    int             m_operation;
    ImoId           m_targetId;
//    DocCursorState  m_curState;
//    DocCursorState  m_targetState;
    SelectionSet*   m_pSelection;

public:
    CmdSelection(int cmd, const string& name="");
    CmdSelection(int cmd, ImoId id, const string& name="");
    //CmdSelection(DocCursorState& state, const string& name="");
    virtual ~CmdSelection() {};

    enum { k_set=0, k_add, k_remove, k_clear, };

    int set_target(Document* pDoc, DocCursor* pCursor, SelectionSet* pSelection);
    int perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    int get_undo_policy() { return k_undo_policy_specific; }
    int get_selection_update_policy() { return k_sel_do_nothing; }

protected:
    void set_default_name();
    void initialize();

};


}   //namespace lomse

#endif      //__LOMSE_COMMAND_H__

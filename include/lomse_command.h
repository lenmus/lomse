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

#ifndef __LOMSE_COMMAND_H__
#define __LOMSE_COMMAND_H__

#include "lomse_stack.h"
#include "lomse_internal_model.h"
#include "lomse_document_cursor.h"

#include <sstream>
using namespace std;

namespace lomse
{

//forward declarations
class Document;

//---------------------------------------------------------------------------------------
// interface for any command to modify the document
class DocCommand
{
protected:
    string m_name;          //displayable name for undo/redo actions display
    string m_checkpoint;    //checkpoint data
    string m_error;
    uint_least16_t m_flags;

    enum {
        k_reversible        = 0x0001,
        k_recordable        = 0x0002,
    };

    DocCommand() : m_flags(0) {}

public:
    virtual ~DocCommand() {}

    //cursor update policy to be used with this command
    enum { k_do_nothing=0, k_update_after_deletion, k_update_after_insertion, };
    virtual int get_cursor_update_policy() = 0;

    //information
    inline std::string get_name() { return m_name; }
    inline bool is_reversible() { return m_flags &  k_reversible; }
    inline bool is_recordable() { return m_flags &  k_recordable; }
    inline string get_error() { return m_error; }

    //actions
    virtual int perform_action(Document* pDoc, DocCursor* pCursor)=0;
    virtual void undo_action(Document* pDoc, DocCursor* pCursor);

protected:
    void create_checkpoint(Document* pDoc);
    void log_forensic_data();
    void set_command_name(const string& name, ImoObj* pImo);

};

//---------------------------------------------------------------------------------------
// interface for any simple command (atomic)
class DocCmdSimple : public DocCommand
{
protected:

    DocCmdSimple() : DocCommand() {}

public:
    virtual ~DocCmdSimple() {}

};

//---------------------------------------------------------------------------------------
// interface for any composite command
class DocCmdComposite : public DocCommand
{
protected:
    DocCmdComposite() : DocCommand() {}

public:
    virtual ~DocCmdComposite() {}

};


//---------------------------------------------------------------------------------------
// undo/redo element: command and cursor
class UndoElement
{
public:
    DocCommand*         pCmd;
    DocCursorState      cursorState;

public:
    UndoElement(DocCommand* cmd, DocCursorState state)
        : pCmd(cmd), cursorState(state)
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
    virtual int execute(DocCursor* pCursor, DocCommand* pCmd);
    virtual void undo(DocCursor* pCursor);
    virtual void redo(DocCursor* pCursor);
    inline string get_error() { return m_error; }

    //info
    bool is_undo_possible() { return m_stack.size() > 0; }
    bool is_redo_possible() { return m_stack.history_size() > 0; }

    virtual size_t undo_stack_size() { return m_stack.size(); }

protected:
    void update_cursor(DocCursor* pCursor, DocCommand* pCmd);

};

//---------------------------------------------------------------------------------------
class CmdAddTie : public DocCmdSimple
{
protected:
    ImoId m_startId;
    ImoId m_endId;
    ImoId m_tieId;

public:
    CmdAddTie(ImoId startNR, ImoId endNR);
    virtual ~CmdAddTie() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    void undo_action(Document* pDoc, DocCursor* pCursor);
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
    CmdAddTuplet(ImoId startNR, ImoId endNR, const string& src);
    virtual ~CmdAddTuplet() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    void undo_action(Document* pDoc, DocCursor* pCursor);
};

//---------------------------------------------------------------------------------------
class CmdBreakBeam : public DocCmdSimple
{
protected:
    ImoId m_beforeId;

public:
    CmdBreakBeam(ImoNoteRest* pBeforeNR);
    virtual ~CmdBreakBeam() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
};

//---------------------------------------------------------------------------------------
class CmdChangeDots : public DocCmdSimple
{
protected:
    int m_dots;
    list<ImoId> m_noteRests;
    list<int> m_oldDots;

public:
    CmdChangeDots(const list<ImoId>& noteRests, int dots);
    virtual ~CmdChangeDots() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
    void undo_action(Document* pDoc, DocCursor* pCursor);

};

//---------------------------------------------------------------------------------------
class CmdCursor : public DocCmdSimple
{
protected:
    int             m_operation;
    ImoId           m_targetId;
    DocCursorState  m_curState;

public:
    CmdCursor(int cmd, ImoId id=k_no_imoid);
    virtual ~CmdCursor() {};

    enum { k_move_next=0, k_move_prev, k_enter, k_exit, k_point_to, k_refresh, };

    int perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
};

//---------------------------------------------------------------------------------------
class CmdDeleteBlockLevelObj : public DocCmdSimple
{
public:
    CmdDeleteBlockLevelObj();
    virtual ~CmdDeleteBlockLevelObj() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_update_after_deletion; }
};

//---------------------------------------------------------------------------------------
class CmdDeleteRelation : public DocCmdSimple
{
protected:
    ImoId m_relId;

public:
    CmdDeleteRelation(ImoRelObj* pRO);
    virtual ~CmdDeleteRelation() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
};

//---------------------------------------------------------------------------------------
class CmdDeleteStaffObj : public DocCmdSimple
{
public:
    CmdDeleteStaffObj();
    virtual ~CmdDeleteStaffObj() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_update_after_deletion; }
};

//---------------------------------------------------------------------------------------
class CmdInsertObj : public DocCmdSimple
{
protected:
    ImoId m_lastInsertedId;
    string m_source;

    CmdInsertObj() : DocCmdSimple(), m_lastInsertedId(k_no_imoid) {}

public:
    ~CmdInsertObj() {}

    inline ImoId last_inserted_id() { return m_lastInsertedId; }

protected:
    void remove_object(Document* pDoc, ImoId id);
    int validate_source();
};

//---------------------------------------------------------------------------------------
class CmdInsertBlockLevelObj : public CmdInsertObj
{
protected:
    int m_blockType;
    bool m_fFromSource;

public:
    CmdInsertBlockLevelObj(int type);
    CmdInsertBlockLevelObj(const string& source);
    virtual ~CmdInsertBlockLevelObj() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_update_after_insertion; }

protected:
    void perform_action_from_source(Document* pDoc, DocCursor* pCursor);
    void perform_action_from_type(Document* pDoc, DocCursor* pCursor);

};

//---------------------------------------------------------------------------------------
class CmdInsertStaffObj : public CmdInsertObj
{
protected:

public:
    CmdInsertStaffObj(const string& source);
    virtual ~CmdInsertStaffObj() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_update_after_insertion; }
};

//---------------------------------------------------------------------------------------
class CmdInsertManyStaffObjs : public CmdInsertObj
{
protected:
    bool m_fSaved;

public:
    CmdInsertManyStaffObjs(const string& source,
                           const string& name="Insert staff objects");
    virtual ~CmdInsertManyStaffObjs() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_update_after_insertion; }

protected:
    void save_source_code_with_ids(Document* pDoc, const list<ImoStaffObj*>& objects);

};

//---------------------------------------------------------------------------------------
class CmdJoinBeam : public DocCmdSimple
{
protected:
    list<ImoId> m_notesId;

public:
    CmdJoinBeam(const list<ImoId>& notes);
    virtual ~CmdJoinBeam() {};

    int perform_action(Document* pDoc, DocCursor* pCursor);
    int get_cursor_update_policy() { return k_do_nothing; }
};


}   //namespace lomse

#endif      //__LOMSE_COMMAND_H__

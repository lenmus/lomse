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

    DocCommand() {}

public:
    virtual ~DocCommand() {}

    //information
    inline std::string get_name() { return m_name; }

    //actions
    virtual void perform_action(Document* pDoc, DocCursor* pCursor)=0;
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

//    //actions
//    void undo_action(Document* pDoc, DocCursor* pCursor) {}
//    void perform_action(Document* pDoc, DocCursor* pCursor) {}
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
// A class to manage the undo/redo stack
typedef UndoableStack<DocCommand*>     UndoStack;


//---------------------------------------------------------------------------------------
// Keeps the stack of executed commands and performs undo/redo
class DocCommandExecuter
{
private:
    Document*   m_pDoc;
    DocCursor*  m_pCursor;
    UndoStack   m_stack;

public:
    DocCommandExecuter(Document* target, DocCursor* pCursor);
    virtual ~DocCommandExecuter() {}
    virtual void execute(DocCommand* pCmd);
    virtual void undo();
    virtual void redo();

//    virtual bool is_document_modified() { return m_pDoc->is_modified(); }
    virtual size_t undo_stack_size() { return m_stack.size(); }
};

//---------------------------------------------------------------------------------------
class CmdCursor : public DocCmdSimple
{
protected:
    int             m_operation;
    long            m_targetId;
    DocCursorState  m_curState;

public:
    CmdCursor(int cmd, long id=-1L);
    virtual ~CmdCursor() {};

    enum { k_move_next=0, k_move_prev, k_enter, k_exit, k_point_to, k_refresh, };

    void perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
};

//---------------------------------------------------------------------------------------
class CmdDeleteBlockLevelObj : public DocCmdSimple
{
public:
    CmdDeleteBlockLevelObj();
    virtual ~CmdDeleteBlockLevelObj() {};

    void perform_action(Document* pDoc, DocCursor* pCursor);
};

//---------------------------------------------------------------------------------------
class CmdDeleteStaffObj : public DocCmdSimple
{
public:
    CmdDeleteStaffObj();
    virtual ~CmdDeleteStaffObj() {};

    void perform_action(Document* pDoc, DocCursor* pCursor);
};

//---------------------------------------------------------------------------------------
class CmdInsertBlockLevelObj : public DocCmdSimple
{
protected:
    long m_insertedId;
    int m_type;

public:
    CmdInsertBlockLevelObj(int type);
    virtual ~CmdInsertBlockLevelObj() {};

    void perform_action(Document* pDoc, DocCursor* pCursor);
    void undo_action(Document* pDoc, DocCursor* pCursor);
};

////---------------------------------------------------------------------------------------
//class CmdInsertStaffObj : public DocCmdSimple
//{
//protected:
//    long m_cursorId;
//    long m_insertedId;
//    int m_type;
//
//public:
//    CmdInsertStaffObj(DocCursor& cursor, int type);
//    virtual ~CmdInsertStaffObj() {};
////    lmCmdInsertRest(bool fNormalCmd,
////                    const wxString& name, lmDocument *pDoc,
////					lmENoteType nNoteType, float rDuration, int nDots, int nVoice);
//
//    void undo_action(Document* pDoc, DocCursor* pCursor);
//    void perform_action(Document* pDoc, DocCursor* pCursor);
//};


}   //namespace lomse

#endif      //__LOMSE_COMMAND_H__

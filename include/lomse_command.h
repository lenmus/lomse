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

#include <sstream>
using namespace std;

namespace lomse
{

//forward declarations
class Document;
class DocCursor;

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
    virtual void perform_action(Document* pDoc)=0;
    virtual void undo_action(Document* pDoc);

protected:
    void create_checkpoint(Document* pDoc);
    void log_forensic_data();

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
//    void undo_action(Document* pDoc) {}
//    void perform_action(Document* pDoc) {}
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


////---------------------------------------------------------------------------------------
//// a helper class to store information about execution of a user command
//class DocCommandData
//{
//protected:
//    int             m_startPos;
//    int             m_endPos;
//    bool            m_docModified;
//
//public:
//    DocCommandData(const std::string& name, bool modified, int startPos)
//        : m_name(name), m_startPos(startPos), m_endPos(0), m_docModified(modified) {}
//    ~DocCommandData() {}
//
//    inline void set_end_pos(int n) { m_endPos = n; }
//    inline int get_num_actions() { return m_endPos - m_startPos; }
//    inline bool get_modified() { return m_docModified; }
//};
//
//// A class to manage the undo/redo stack of user commands
//typedef UndoableStack<DocCommandData*>     CmdDataUndoStack;



////---------------------------------------------------------------------------------------
//class UserCommandExecuter
//{
//private:
//    Document*           m_pDoc;
//    DocCommandExecuter  m_docCommandExecuter;
//    ModelBuilder*       m_pModelBuilder;
//    CmdDataUndoStack    m_stack;
//
//public:
//    UserCommandExecuter(Document* pDoc, ModelBuilder* pBuilder);     //only for tests
//    UserCommandExecuter(Document* pDoc);
//
//    virtual ~UserCommandExecuter();
//    virtual void execute(DocCmdComposite& cmd);
//    virtual void undo();
//    virtual void redo();
//
//    virtual size_t undo_stack_size() { return m_stack.size(); }
//
//private:
//    void update_model();
//};



////---------------------------------------------------------------------------------------
//class DocCommandInsert : public DocCmdComposite
//{
//public:
//    DocCommandInsert(Document::iterator& it, LdpElement* pNewElm);
//    ~DocCommandInsert();
//
//    void undo(Document* pDoc);
//    void redo(Document* pDoc);
//
//protected:
//    Document::iterator m_itInserted;
//};
//
//
////---------------------------------------------------------------------------------------
//class DocCommandPushBack : public DocCmdComposite
//{
//public:
//    DocCommandPushBack(Document::iterator& it, LdpElement* pNewElm);
//    ~DocCommandPushBack();
//
//    void undo(Document* pDoc);
//    void redo(Document* pDoc);
//};
//
//
////---------------------------------------------------------------------------------------
//class DocCommandRemove : public DocCmdComposite
//{
//public:
//    DocCommandRemove(Document::iterator& it);
//    ~DocCommandRemove();
//
//    void undo(Document* pDoc);
//    void redo(Document* pDoc);
//
//protected:
//    LdpElement*     m_parent;
//    LdpElement*     m_nextSibling;
//};


////---------------------------------------------------------------------------------------
//class CommandBuilder
//{
//private:
//
//public:
//    CommandBuilder(Document* target);   //, CmdExecuter);
//    //methods for creating commands
//    DocCommand* new_cmd_delete_staffobj(DocCursor& cursor);
//    DocCommand* new_cmd_insert_block_level_obj(DocCursor& cursor, int type);
//};

//---------------------------------------------------------------------------------------
// A class to manage the undo/redo stack
typedef UndoableStack<DocCommand*>     UndoStack;


//---------------------------------------------------------------------------------------
// Keeps the stack of executed commands and performs undo/redo
class DocCommandExecuter
{
private:
    Document*   m_pDoc;
    UndoStack   m_stack;

public:
    DocCommandExecuter(Document* target);
    virtual ~DocCommandExecuter() {}
    virtual void execute(DocCommand* pCmd);
    virtual void undo();
    virtual void redo();

//    virtual bool is_document_modified() { return m_pDoc->is_modified(); }
    virtual size_t undo_stack_size() { return m_stack.size(); }
};

//---------------------------------------------------------------------------------------
class CmdDeleteStaffObj : public DocCmdSimple
{
protected:
    int m_idScore;
    int m_idStaffobj;

public:
    CmdDeleteStaffObj(DocCursor& cursor);
    virtual ~CmdDeleteStaffObj() {};

    void perform_action(Document* pDoc);

};

//---------------------------------------------------------------------------------------
class CmdInsertBlockLevelObj : public DocCmdSimple
{
protected:
//    Document::iterator  m_it;
    DocCursor& m_cursor;
    ImoBlockLevelObj* m_pImo;
    int m_type;
//    bool                m_fPushBack;
//    LdpElement*         m_pGoBackElm;
//    LdpElement*         m_pGoFwdElm;
//    LdpCompiler*        m_pCompiler;

public:
//    CmdInsertBlockLevelObj(const string& name, DocCursor& cursor, ImoBlockLevelObj* pImo);
    CmdInsertBlockLevelObj(DocCursor& cursor, int type);
    virtual ~CmdInsertBlockLevelObj() {};

    void undo_action(Document* pDoc);
    void perform_action(Document* pDoc);

//protected:
//    bool do_actions(DocCommandExecuter* pExec);
//    virtual LdpElement* determine_source_insertion_point(DocCursor& cursor, LdpElement* pElm);
//    virtual LdpElement* determine_if_go_back_needed(DocCursor& cursor, LdpElement* pElm);
//    virtual LdpElement* determine_if_go_fwd_needed(DocCursor& cursor, LdpElement* pElm);
//    void execute_insert(DocCommandExecuter* pExec, Document::iterator& it, LdpElement* pNewElm);
//

};

//        CmdInsertBlockLevelObj
//            CmdInsertScore
//            CmdInsertBlocksContainer
//                CmdInsertTable
//                CmdInsertList
//                CmdInsertTableRow
//                CmdInsertListItem
//                CmdInsertTableCell
//
//            CmdInsertInlinesContainer
//                CmdInsertParagraph
//                CmdInsertHeading


}   //namespace lomse

#endif      //__LOMSE_COMMAND_H__

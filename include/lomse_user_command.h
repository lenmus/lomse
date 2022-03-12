//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_USER_COMMAND_H__
#define __LOMSE_USER_COMMAND_H__

#include <sstream>
#include <list>
#include "lomse_ldp_elements.h"
#include "lomse_stack.h"
#include "private/lomse_document_p.h"

using namespace std;

namespace lomse
{

//forward declarations
class UserCommand;
class DocCommandExecuter;
class CmdActionData;
class ModelBuilder;
class DocumentScope;

// a helper class to store information about execution of a user command
class UserCommandData
{
protected:
    std::string     m_name;
    int             m_startPos;
    int             m_endPos;
    bool            m_docModified;

public:
    UserCommandData(const std::string& name, bool modified, int startPos)
        : m_name(name), m_startPos(startPos), m_endPos(0), m_docModified(modified) {}
    ~UserCommandData() {}

    inline void set_end_pos(int n) { m_endPos = n; }
    inline int get_num_actions() { return m_endPos - m_startPos; }
    inline bool get_modified() { return m_docModified; }
};

// A class to manage the undo/redo stack of user commands
typedef UndoableStack<UserCommandData*>     CmdDataUndoStack;



/////
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
//    virtual void execute(UserCommand& cmd);
//    virtual void undo();
//    virtual void redo();
//
//    virtual size_t undo_stack_size() { return m_stack.size(); }
//
//private:
//    void update_model();
//};



// A class to store data for a command
//------------------------------------------------------------------
class UserCommand
{
public:
    UserCommand(const std::string& name) : m_name(name) {}
    virtual ~UserCommand() {}

    inline std::string get_name() { return m_name; }

protected:
    friend class UserCommandExecuter;
    virtual bool do_actions(DocCommandExecuter* dce)=0;

    std::string             m_name;
};




}   //namespace lomse

#endif      //__LOMSE_USER_COMMAND_H__

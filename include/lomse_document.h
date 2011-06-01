//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//--------------------------------------------------------------------------------------

#ifndef __LOMSE_DOCUMENT_H__
#define __LOMSE_DOCUMENT_H__

#include <sstream>
#include "lomse_injectors.h"
#include "lomse_observable.h"
#include "lomse_ldp_elements.h"
#include "lomse_stack.h"

using namespace std;

namespace lomse
{

//forward declarations
class DocCommand;
class DocCommandExecuter;
class LdpCompiler;
class IdAssigner;
class InternalModel;
class ImoDocument;
class ImoScore;

/// A class to manage the undo/redo stack
typedef UndoableStack<DocCommand*>     UndoStack;


//------------------------------------------------------------------------------------
// Base class for lomse document.
// Encapsulates all the library internals, providing the basic API for creating and
// using a document.
//      - an iterator to traverse the document;
//      - support for visitors;
//      - serialization; and
//      - atomic methods to modify the document (no undo/redo capabilities).
//      - methods to set/check a 'document modified' flag (but no logic to
//        manage this flag, only reset when the document is created/loaded)
//------------------------------------------------------------------------------------

class LOMSE_EXPORT Document : public Observable
{
protected:
    DocumentScope*  m_pDocScope;
    LdpCompiler*    m_pCompiler;
    IdAssigner*     m_pIdAssigner;
    InternalModel*  m_pIModel;
    ImoDocument*    m_pImoDoc;

public:
    Document(LibraryScope& libraryScope, ostream& reporter=cout);
    Document(LdpCompiler* pCompiler, IdAssigner* pIdAssigner);  //for testing: direct inyection of dependencies
    virtual ~Document();

    //scope access
    inline DocumentScope* get_scope() { return m_pDocScope; }

    //creation
    int from_file(const std::string& filename);
    int from_string(const std::string& source);
    void create_empty();
    void create_with_empty_score();

    inline bool is_modified() { return false; } //TODO:  return m_pTree->is_modified(); }
//    inline void clear_modified() { m_pTree->clear_modified(); }
//    inline LdpTree* get_tree() { return m_pTree; }
    inline ImoDocument* get_imodoc() const { return m_pImoDoc; }
    inline InternalModel* get_im_model() const { return m_pIModel; }

//    //a low level cursor for the document
//    typedef LdpTree::depth_first_iterator iterator;
//
//	iterator begin() { LdpTree::depth_first_iterator it = m_pTree->begin(); return iterator(it); }
//	iterator end() { LdpTree::depth_first_iterator it = m_pTree->end(); return iterator(it); }
//    iterator content();
//
//    std::string to_string(iterator& it) { return (*it)->to_string(); }
    std::string to_string();    //for tests
//    std::string to_string_with_ids(iterator& it) { return (*it)->to_string_with_ids(); }
//    std::string to_string_with_ids() { return m_pTree->get_root()->to_string_with_ids(); }

    //atomic commands to edit the document. No undo/redo capabilities.
    //In principle, to be used only by DocCommandExecuter

//    void add_content(DocIterator& it, ImoObj* pObj);     //before it
//    void add_staffobj(DocCursor& cursor, ImoStaffObj* pObj);     //before cursor
//    ImoStaffObj* remove_staffobj()
//    void add_attachment()
//    ImoAuxObj* remove_attachment()
//    void add_reldataobj()
//    void remove_reldataobj()
//    void change_property()

//    //inserts param before the element at position referred by iterator 'it'.
//    //Returns iterator pointing to the newly inserted element
//    iterator insert(iterator& it, LdpElement* node);
//
//    //push back a param to element referred by iterator 'it'.
//    void add_param(iterator& it, LdpElement* node);
//
//    //removes element pointed by 'it'.
//    LdpElement* remove(iterator& it);
//
//    //removes last param of element pointed by 'it'.
//    void remove_last_param(iterator& it);

    //To have more control about when to update views, the document doesn't
    //automatically notify views when the document is updated.
    //Views observing the document will be notified about modifications only
    //when the following method is invoked. Commands (atomic, DocCommands and
    //UserCommands)don't invoke it. Invoking this method is a responsibility
    //of the Interactor (or the user application if Interactor is not used)
    void notify_that_document_has_been_modified() { notify_observers(); }


protected:
    void clear();


    //------------------------------------------------------------------
    // Transitional, while moving from score to lenmusdoc
    //------------------------------------------------------------------
public:
    ImoScore* get_score();
    void create_score(ostream& reporter=cout);

};


//// A class to store data for a command
////------------------------------------------------------------------
//class DocCommand
//{
//protected:
//    Document::iterator m_position;
//    LdpElement* m_added;
//    LdpElement* m_removed;
//    bool m_applied;
//
//public:
//    DocCommand(Document::iterator& it, LdpElement* added, LdpElement* removed)
//        : m_position(it), m_added(added), m_removed(removed), m_applied(false) {}
//
//    virtual ~DocCommand() {}
//
//    //getters
//    inline Document::iterator& get_position() { return m_position; }
//    inline LdpElement* get_added() { return m_added; }
//    inline LdpElement* get_removed() { return m_removed; }
//
//    //actions
//    virtual void undo(Document* pDoc)=0;
//    virtual void redo(Document* pDoc)=0;
//};
//
//
//class DocCommandInsert : public DocCommand
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
//class DocCommandPushBack : public DocCommand
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
//class DocCommandRemove : public DocCommand
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
//
//
////
//class DocCommandExecuter
//{
//private:
//    Document*   m_pDoc;
//    UndoStack   m_stack;
//
//public:
//    DocCommandExecuter(Document* target);
//    virtual ~DocCommandExecuter() {}
//    virtual void execute(DocCommand* pCmd);
//    virtual void undo();
//    virtual void redo();
//
//    virtual bool is_document_modified() { return m_pDoc->is_modified(); }
//    virtual size_t undo_stack_size() { return m_stack.size(); }
//};



//// A class to store data for a command
////--------------------------------------------------------------------------------------
//class DocCommand
//{
//protected:
//    DocIterator m_position;
//    ImoObj* m_added;
//    ImoObj* m_removed;
//    bool m_applied;
//
//public:
//    DocCommand(DocIterator& it, ImoObj* added, ImoObj* removed)
//        : m_position(it), m_added(added), m_removed(removed), m_applied(false) {}
//
//    virtual ~DocCommand() {}
//
//    //getters
//    inline DocIterator& get_position() { return m_position; }
//    inline ImoObj* get_added() { return m_added; }
//    inline ImoObj* get_removed() { return m_removed; }
//
//    //actions
//    virtual void undo(Document* pDoc)=0;
//    virtual void redo(Document* pDoc)=0;
//};
//
//
//class DocCommandInsert : public DocCommand
//{
//public:
//    DocCommandInsert(DocIterator& it, ImoObj* pObj);
//    ~DocCommandInsert();
//
//    void undo(Document* pDoc);
//    void redo(Document* pDoc);
//
//protected:
//    DocIterator m_itInserted;
//};


}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_H__

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
//---------------------------------------------------------------------------------------

#include "lomse_document.h"

#include <sstream>
#include "lomse_parser.h"
#include "lomse_analyser.h"
#include "lomse_compiler.h"
#include "lomse_injectors.h"
#include "lomse_id_assigner.h"
#include "lomse_internal_model.h"
#include "lomse_ldp_exporter.h"
#include "lomse_model_builder.h"
#include "lomse_im_factory.h"
#include "lomse_events.h"

using namespace std;

namespace lomse
{


//---------------------------------------------------------------------------------------
// Document implementation
//---------------------------------------------------------------------------------------
Document::Document(LibraryScope& libraryScope, ostream& reporter)
    : BlockLevelCreatorApi()
    , m_libraryScope(libraryScope)
    , m_reporter(reporter)
    , m_docScope(reporter)
    , m_pCompiler(NULL)
    , m_pIdAssigner(NULL)
    , m_pIModel(NULL)
    , m_pImoDoc(NULL)
    , m_idCounter(-1L)
{
}

//---------------------------------------------------------------------------------------
Document::~Document()
{
    delete m_pIModel;
    delete m_pCompiler;
    delete_observers();
}

//---------------------------------------------------------------------------------------
void Document::initialize()
{
    if (m_pImoDoc)
        throw std::runtime_error(
            "[Document::create] Attempting to create already created document");

    m_pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
    m_pIdAssigner = m_docScope.id_assigner();
}

//---------------------------------------------------------------------------------------
void Document::set_imo_doc(ImoDocument* pImoDoc)
{
    m_pImoDoc = pImoDoc;
    set_box_level_creator_api_parent( m_pImoDoc );
}

//---------------------------------------------------------------------------------------
int Document::from_file(const std::string& filename)
{
    initialize();
    m_pIModel = m_pCompiler->compile_file(filename);
    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
    return m_pCompiler->get_num_errors();
}

//---------------------------------------------------------------------------------------
int Document::from_string(const std::string& source)
{
    initialize();
    m_pIModel = m_pCompiler->compile_string(source);
    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
    return m_pCompiler->get_num_errors();
}

//---------------------------------------------------------------------------------------
int Document::from_input(LdpReader& reader)
{
    initialize();
    try
    {
        m_pIModel = m_pCompiler->compile_input(reader);
        m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
        return m_pCompiler->get_num_errors();
    }
    catch (...)
    {
        create_empty();
        return 0;
    }
}

//---------------------------------------------------------------------------------------
void Document::create_empty()
{
    initialize();
    m_pIModel = m_pCompiler->create_empty();
    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
}

//---------------------------------------------------------------------------------------
void Document::create_with_empty_score()
{
    initialize();
    m_pIModel = m_pCompiler->create_with_empty_score();
    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
}

//---------------------------------------------------------------------------------------
void Document::end_of_changes()
{
    ModelBuilder builder(m_docScope.default_reporter());
    builder.build_model(m_pIModel);
}

//---------------------------------------------------------------------------------------
std::string Document::to_string()
{
    //for tests
    LdpExporter exporter;
    return exporter.get_source(m_pImoDoc);
}


//ImDocument* Document::get_root()
//{
//    return dynamic_cast<ImDocument*>( m_pImoDoc->get_root() );
//}

//Document::iterator Document::content()
//{
//    iterator it = begin();
//    while (it != end() && !(*it)->is_type(k_content))
//        ++it;
//    return it;
//}
//
//Document::iterator Document::insert(iterator& it, LdpElement* node)
//{
//    // inserts element before position 'it', that is, as previous sibling
//    // of node pointed by 'it'
//
//    m_pIdAssigner->reassign_ids(node);
//    return (*it)->insert(it, node);
//}
//
//void Document::add_param(iterator& it, LdpElement* node)
//{
//    // adds a child to element referred by iterator 'it'.
//
//    m_pIdAssigner->reassign_ids(node);
//    (*it)->append_child(node);
//}
//
//LdpElement* Document::remove(iterator& it)
//{
//    // removes element pointed by 'it'. Returns removed element
//
//    LdpTree::depth_first_iterator itNode = it;
//    LdpElement* removed = *itNode;
//    m_pTree->erase(itNode);
//    return removed;
//}
//
//void Document::remove_last_param(iterator& it)
//{
//    // removes last param of element pointed by 'it'
//
//    Tree<LdpElement>::depth_first_iterator itParm( (*it)->get_last_child() );
//    m_pTree->erase(itParm);
//}


ImoScore* Document::get_score()
{
    //TODO: Item 0 is no longer the first score
    return dynamic_cast<ImoScore*>( m_pImoDoc->get_content_item(0) );
}

ImoObj* Document::create_object(const string& source)
{
    LdpParser parser(m_reporter, m_libraryScope.ldp_factory());
    SpLdpTree tree = parser.parse_text(source);
    Analyser a(m_reporter, m_libraryScope, this);
    ImoObj* pImo = a.analyse_tree_and_get_object(tree);
    delete tree->get_root();
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoStyle* Document::create_style(const string& name, const string& parent)
{
    return m_pImoDoc->create_style(name, parent);
}

//---------------------------------------------------------------------------------------
ImoStyle* Document::create_private_style(const string& parent)
{
    return m_pImoDoc->create_private_style(parent);
}

//---------------------------------------------------------------------------------------
ImoStyle* Document::get_default_style()
{
    return m_pImoDoc->get_default_style();
}

//---------------------------------------------------------------------------------------
ImoStyle* Document::find_style(const string& name)
{
    return m_pImoDoc->find_style(name);
}

//---------------------------------------------------------------------------------------
void Document::notify_that_document_has_been_modified()
{
    EventDoc* pEvent = new EventDoc(k_doc_modified_event, this);
    notify_observers(pEvent, NULL);
}

//---------------------------------------------------------------------------------------
// Observable 
//---------------------------------------------------------------------------------------
void Document::notify_observers(EventInfo* pEvent, ImoObj* pImo)
{
    std::list<Observer*>::iterator it;
    for (it = m_observers.begin(); it != m_observers.end(); ++it)
    {
        if ((*it)->target() == pImo)
        {
            (*it)->notify(pEvent);
            return;
        }
    }
}

//---------------------------------------------------------------------------------------
Observer* Document::add_observer(ImoObj* pImo)
{
    std::list<Observer*>::iterator it;
    for (it = m_observers.begin(); it != m_observers.end(); ++it)
    {
        if ((*it)->target() == pImo)
            return *it;
    }

    Observer* observer = new Observer(pImo);
    m_observers.push_back(observer);
    return observer;
}

//---------------------------------------------------------------------------------------
void Document::remove_observer(Observer* observer)
{
    m_observers.remove(observer);
    delete observer;
}

//---------------------------------------------------------------------------------------
void Document::delete_observers()
{
    std::list<Observer*>::iterator it;
    for (it = m_observers.begin(); it != m_observers.end(); ++it)
        delete *it;
    m_observers.clear();
}

//---------------------------------------------------------------------------------------
void Document::add_event_handler(ImoObj* pImo, int eventType, EventHandler* pHandler)
{
    Observer* observer = add_observer(pImo);
    observer->add_handler(eventType, pHandler);
}

////------------------------------------------------------------------
//// DocCommandInsert
////------------------------------------------------------------------
//
//DocCommandInsert::DocCommandInsert(DocIterator& it,ImoObj* pNewObj)
//    : DocCommand(it, pNewObj, NULL)
//{
//}
//
//DocCommandInsert::~DocCommandInsert()
//{
//    if (!m_applied)
//        delete m_added;
//}
//
//void DocCommandInsert::undo(Document* pDoc)
//{
//    (*m_itInserted)->reset_modified();
//    pDoc->remove(m_itInserted);
//    m_applied = false;
//}
//
//void DocCommandInsert::redo(Document* pDoc)
//{
//    m_itInserted = pDoc->insert(m_position, m_added);
//    (*m_itInserted)->set_modified();
//    m_applied = true;
//}


////------------------------------------------------------------------
//// DocCommandPushBack
////------------------------------------------------------------------
//
//DocCommandPushBack::DocCommandPushBack(Document::iterator& it, LdpElement* added)
//    : DocCommand(it, added, NULL)
//{
//}
//
//DocCommandPushBack::~DocCommandPushBack()
//{
//    if (!m_applied)
//        delete m_added;
//}
//
//void DocCommandPushBack::undo(Document* pDoc)
//{
//    (*m_position)->reset_modified();
//    pDoc->remove_last_param(m_position);
//    m_applied = false;
//}
//
//void DocCommandPushBack::redo(Document* pDoc)
//{
//    pDoc->add_param(m_position, m_added);
//    (*m_position)->set_modified();
//    m_applied = true;
//}
//
//
////------------------------------------------------------------------
//// DocCommandRemove
////------------------------------------------------------------------
//
//DocCommandRemove::DocCommandRemove(Document::iterator& it)
//    : DocCommand(it, NULL, *it)
//{
//    m_parent = (*it)->get_parent();
//    m_nextSibling = (*it)->get_next_sibling();
//}
//
//DocCommandRemove::~DocCommandRemove()
//{
//    if (m_applied)
//        delete m_removed;
//}
//
//void DocCommandRemove::undo(Document* pDoc)
//{
//    m_parent->reset_modified();
//    if (!m_nextSibling)
//    {
//        Document::iterator it(m_parent);
//        pDoc->add_param(it, m_removed);
//    }
//    else
//    {
//        Document::iterator it(m_nextSibling);
//        pDoc->insert(it, m_removed);
//    }
//    m_applied = false;
//}
//
//void DocCommandRemove::redo(Document* pDoc)
//{
//    pDoc->remove(m_position);
//    m_parent->set_modified();
//    m_applied = true;
//}


////------------------------------------------------------------------
//// DocCommandExecuter
////------------------------------------------------------------------
//
//DocCommandExecuter::DocCommandExecuter(Document* target)
//    : m_pDoc(target)
//{
//}
//
//void DocCommandExecuter::execute(DocCommand* pCmd)
//{
//    m_stack.push(pCmd);
//    pCmd->redo(m_pDoc);
//}
//
//void DocCommandExecuter::undo()
//{
//    DocCommand* cmd = m_stack.pop();
//    cmd->undo(m_pDoc);
//}
//
//void DocCommandExecuter::redo()
//{
//    DocCommand* cmd = m_stack.undo_pop();
//    cmd->redo(m_pDoc);
//}


}  //namespace lomse

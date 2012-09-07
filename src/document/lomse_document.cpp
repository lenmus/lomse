//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include "lomse_document.h"

#include <sstream>
#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_ldp_compiler.h"
#include "lomse_lmd_compiler.h"
#include "lomse_injectors.h"
#include "lomse_id_assigner.h"
#include "lomse_internal_model.h"
#include "lomse_ldp_exporter.h"
#include "lomse_model_builder.h"
#include "lomse_im_factory.h"
#include "lomse_events.h"
#include "lomse_ldp_elements.h"

using namespace std;

namespace lomse
{

//=======================================================================================
// Document implementation
//=======================================================================================
Document::Document(LibraryScope& libraryScope, ostream& reporter)
    : BlockLevelCreatorApi()
    , EventNotifier(libraryScope.get_events_dispatcher())
    , Observable()
    , m_libraryScope(libraryScope)
    , m_reporter(reporter)
    , m_docScope(reporter)
    //, m_pCompiler(NULL)
    //, m_pIdAssigner(NULL)
    , m_pIModel(NULL)
    , m_pImoDoc(NULL)
    , m_flags(k_dirty)
    , m_idCounter(-1L)
{
}

//---------------------------------------------------------------------------------------
Document::~Document()
{
    delete m_pIModel;
    //delete m_pCompiler;
    delete_observers();
}

//---------------------------------------------------------------------------------------
void Document::initialize()
{
    if (m_pImoDoc)
        throw std::runtime_error(
            "[Document::create] Attempting to create already created document");

    m_flags = k_dirty;
}

//---------------------------------------------------------------------------------------
void Document::set_imo_doc(ImoDocument* pImoDoc)
{
    m_pImoDoc = pImoDoc;
    set_box_level_creator_api_parent( m_pImoDoc );
}

//---------------------------------------------------------------------------------------
int Document::from_file(const std::string& filename, int format)
{
    initialize();
    Compiler* pCompiler = get_compiler_for_format(format);
    //m_pIdAssigner = m_docScope.id_assigner();
    m_pIModel = pCompiler->compile_file(filename);
    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
    int numErrors = pCompiler->get_num_errors();
    delete pCompiler;
    return numErrors;
}

//---------------------------------------------------------------------------------------
int Document::from_string(const std::string& source, int format)
{
    initialize();
    Compiler* pCompiler = get_compiler_for_format(format);
    //m_pIdAssigner = m_docScope.id_assigner();
    m_pIModel = pCompiler->compile_string(source);
    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
    int numErrors = pCompiler->get_num_errors();
    delete pCompiler;
    return numErrors;
}

//---------------------------------------------------------------------------------------
int Document::from_input(LdpReader& reader)
{
    initialize();
    try
    {
        LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
        //m_pIdAssigner = m_docScope.id_assigner();
        m_pIModel = pCompiler->compile_input(reader);
        m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
        int numErrors = pCompiler->get_num_errors();
        delete pCompiler;
        return numErrors;
    }
    catch (...)
    {
        //this avoids programs crashes when a document is malformed but
        //will produce memory lekeages
        m_pIModel = NULL;
        create_empty();
        return 0;
    }
}

//---------------------------------------------------------------------------------------
void Document::create_empty()
{
    initialize();
    LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
    //m_pIdAssigner = m_docScope.id_assigner();
    m_pIModel = pCompiler->create_empty();
    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
    delete pCompiler;
}

//---------------------------------------------------------------------------------------
void Document::create_with_empty_score()
{
    initialize();
    LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
    //m_pIdAssigner = m_docScope.id_assigner();
    m_pIModel = pCompiler->create_with_empty_score();
    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
    delete pCompiler;
}

//---------------------------------------------------------------------------------------
void Document::end_of_changes()
{
    ModelBuilder builder;
    builder.build_model(m_pIModel);
}

//---------------------------------------------------------------------------------------
std::string Document::to_string()
{
    //for tests
    LdpExporter exporter;
    return exporter.get_source(m_pImoDoc);
}

//---------------------------------------------------------------------------------------
Compiler* Document::get_compiler_for_format(int format)
{
    switch(format)
    {
        case k_format_ldp:
            return Injector::inject_LdpCompiler(m_libraryScope, this);

        case k_format_lmd:
            return Injector::inject_LmdCompiler(m_libraryScope, this);

        default:
            throw std::runtime_error(
                "[Document::from_file] Invalid identifier for file format");
    }
    return NULL;
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

//---------------------------------------------------------------------------------------
ImoObj* Document::create_object(const string& source)
{
    LdpParser parser(m_reporter, m_libraryScope.ldp_factory());
    parser.parse_text(source);
    LdpTree* tree = parser.get_ldp_tree();
    LdpAnalyser a(m_reporter, m_libraryScope, this);
    ImoObj* pImo = a.analyse_tree_and_get_object(tree);
    delete tree->get_root();
    return pImo;
}

//---------------------------------------------------------------------------------------
void Document::add_staff_objects(const string& source, ImoMusicData* pMD)
{
    string data = "(musicData " + source + ")";
    LdpParser parser(m_reporter, m_libraryScope.ldp_factory());
    parser.parse_text(data);
    LdpTree* tree = parser.get_ldp_tree();
    LdpAnalyser a(m_reporter, m_libraryScope, this);
    InternalModel* pIModel = a.analyse_tree(tree, "string:");
    ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
    ImoObj::children_iterator it = pMusic->begin();
    while (it != pMusic->end())
    {
        ImoObj* pImo = *it;
        pMusic->remove_child(pImo);
        pMD->append_child_imo(pImo);
        it = pMusic->begin();
    }
    delete tree->get_root();
    delete pIModel;
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
void Document::notify_if_document_modified()
{
    if (!is_dirty())
        return;

    clear_dirty();
    SpEventDoc pEvent( LOMSE_NEW EventDoc(k_doc_modified_event, this) );
    notify_observers(pEvent, this);
}


//---------------------------------------------------------------------------------------
ImoScore* Document::get_score(int i)
{
    //TODO: Content item 'i' is no longer the score 'i' as there are now other
    //content types, such as paragraphs.
    return dynamic_cast<ImoScore*>( m_pImoDoc->get_content_item(i) );
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

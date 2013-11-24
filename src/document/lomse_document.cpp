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

#include "lomse_document.h"
#include "lomse_build_options.h"

#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_ldp_compiler.h"
#include "lomse_lmd_analyser.h"
#include "lomse_lmd_parser.h"
#include "lomse_lmd_compiler.h"
#include "lomse_injectors.h"
#include "lomse_id_assigner.h"
#include "lomse_internal_model.h"
#include "lomse_ldp_exporter.h"
#include "lomse_lmd_exporter.h"
#include "lomse_model_builder.h"
#include "lomse_im_factory.h"
#include "lomse_events.h"
#include "lomse_ldp_elements.h"
#include "lomse_control.h"
#include "lomse_logger.h"

#include <sstream>
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
    , m_pIdAssigner( m_docScope.id_assigner() )
    , m_pIModel(NULL)
    , m_pImoDoc(NULL)
    , m_flags(k_dirty)
    , m_modified(0)
{
}

//---------------------------------------------------------------------------------------
Document::~Document()
{
    delete m_pIModel;
    delete_observers();
}

//---------------------------------------------------------------------------------------
void Document::initialize()
{
    if (m_pImoDoc)
    {
        LOMSE_LOG_ERROR("Aborting. Attempting to create already created document");
        throw std::runtime_error(
            "[Document::create] Attempting to create already created document");
    }

    m_flags = k_dirty;
    m_pImoDoc = NULL;
    m_modified = 0;
}

//---------------------------------------------------------------------------------------
void Document::set_imo_doc(ImoDocument* pImoDoc)
{
    m_pImoDoc = pImoDoc;
    set_box_level_creator_api_parent( m_pImoDoc );
}

//---------------------------------------------------------------------------------------
int Document::from_file(const string& filename, int format)
{
    initialize();
    Compiler* pCompiler = get_compiler_for_format(format);
    m_pIModel = pCompiler->compile_file(filename);
    int numErrors = pCompiler->get_num_errors();
    delete pCompiler;

    if (m_pIModel)
        m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
    else
        create_empty();

    return numErrors;
}

//---------------------------------------------------------------------------------------
int Document::from_string(const string& source, int format)
{
    initialize();
    Compiler* pCompiler = get_compiler_for_format(format);
    m_pIModel = pCompiler->compile_string(source);
    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
    int numErrors = pCompiler->get_num_errors();
    delete pCompiler;
    return numErrors;
}

//---------------------------------------------------------------------------------------
int Document::from_checkpoint(const string& data)
{
    //delete old internal model
    delete m_pIModel;
    m_pIModel = NULL;
    m_pImoDoc = NULL;
    m_flags = k_dirty;

    //reset IdAssigner
    m_pIdAssigner->reset();

    //observers are external to the Document. Therefore, they do not change by
    //modifying the Document. So, nothing to do about observers.
    //Nevertheless, in future, if undo data includes other actions (not only
    //those to modify the Document) such as for instance, creating a second View for
    //the Document, these actions could imply reviewing Document observers.

    //finally, load document from checkpoint source data
    return from_string(data, k_format_lmd);
}

//---------------------------------------------------------------------------------------
int Document::replace_object_from_checkpoint_data(ImoId id, const string& data)
{
    //object to replace
    ImoObj* pOldImo = get_pointer_to_imo(id);
    ImoObj* pParent = pOldImo->get_parent();

    //new object
    IdAssigner assigner;
    IdAssigner* pSave = m_pIdAssigner;
    m_pIdAssigner = &assigner;
    ImoObj* pNewImo = create_object_from_lmd(data);
    m_pIdAssigner = pSave;

    //replace old object
    //TODO: check that new and old have the same id?
    ImoObj::depth_first_iterator it(pOldImo);
    pParent->replace_node(it, pNewImo);
    delete pOldImo;

    //add new ids
    assigner.copy_ids_to(m_pIdAssigner);

    return 0;
}

//---------------------------------------------------------------------------------------
int Document::from_input(LdpReader& reader)
{
    initialize();
    try
    {
        LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
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
    m_pIModel = pCompiler->create_empty();
    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
    delete pCompiler;
}

//---------------------------------------------------------------------------------------
void Document::create_with_empty_score()
{
    initialize();
    LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
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
string Document::to_string(bool fWithIds)
{
    LdpExporter exporter;
    exporter.set_remove_newlines(true);
    exporter.set_add_id(fWithIds);
    return exporter.get_source(m_pImoDoc);
}

//---------------------------------------------------------------------------------------
string Document::get_checkpoint_data()
{
    return get_checkpoint_data_for( m_pImoDoc->get_id() );
}

//---------------------------------------------------------------------------------------
string Document::get_checkpoint_data_for(ImoId id)
{
    ImoObj* pImo = get_pointer_to_imo(id);
    //TODO: check that ImoObj is a terminal node?

    LmdExporter exporter(m_libraryScope);
    //exporter.set_remove_newlines(true);   //Commented out to facilitate debugging
    exporter.set_add_id(true);
    exporter.set_score_format(LmdExporter::k_format_ldp);
    return exporter.get_source(pImo);

    //code if Ldp format:
        //LdpExporter exporter;
        //exporter.set_remove_newlines(true);
        //exporter.set_add_id(true);
        //return exporter.get_source(m_pImoDoc);
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
        {
            LOMSE_LOG_ERROR("Aborting. Invalid identifier for file format.");
            throw std::runtime_error(
                "[Document::from_file] Invalid identifier for file format.");
        }
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
ImoObj* Document::create_object_from_ldp(const string& source)
{
    return create_object_from_ldp(source, m_reporter);
}

//---------------------------------------------------------------------------------------
ImoObj* Document::create_object_from_ldp(const string& source, ostream& reporter)
{
    LdpParser parser(reporter, m_libraryScope.ldp_factory());
    parser.parse_text(source);
    LdpTree* tree = parser.get_ldp_tree();
    if (tree)
    {
        LdpAnalyser a(reporter, m_libraryScope, this);
        a.set_score_version("2.0");
        try
        {
            ImoObj* pImo = a.analyse_tree_and_get_object(tree);
            delete tree->get_root();

            if (pImo)
            {
                ModelBuilder builder;
                builder.structurize(pImo);
            }
            return pImo;
        }
        catch (...)
        {
            return NULL;
        }
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
ImoTuplet* Document::add_tuplet(ImoNoteRest* pStartNR, ImoNoteRest* pEndNR,
                                const string& source, ostream& reporter)
{
    LdpParser parser(reporter, m_libraryScope.ldp_factory());
    try
    {
        parser.parse_text(source);
        LdpTree* tree = parser.get_ldp_tree();
        if (tree)
        {
            LdpAnalyser a(reporter, m_libraryScope, this);
            a.set_score_version("2.0");
            ImoTupletDto* pTupletStart = dynamic_cast<ImoTupletDto*>(
                                            a.analyse_tree_and_get_object(tree) );
            delete tree->get_root();

            if (pTupletStart)
            {
                //attach the tuplet to start note/rest
                pTupletStart->set_note_rest(pStartNR);
                a.add_relation_info(pTupletStart);
                pStartNR->set_dirty(true);

                //create end of tuplet and attach it to end note/rest
                ImoTupletDto* pTupletEnd = LOMSE_NEW ImoTupletDto();
                pTupletEnd->set_tuplet_type(ImoTupletDto::k_stop);
                pTupletEnd->set_note_rest(pEndNR);
                a.add_relation_info(pTupletEnd);

                //get the tuplet
                ImoTuplet* pTuplet = pStartNR->get_tuplet();
                return pTuplet;
            }
        }
    }
    catch (const std::exception& e)
    {
        reporter << e.what();
    }
    catch (...)
    {
        reporter << "exception caught";
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
ImoBeam* Document::add_beam(const list<ImoNoteRest*>& notes)
{
    LdpAnalyser a(m_reporter, m_libraryScope, this);
    return a.create_beam(notes);
}

//---------------------------------------------------------------------------------------
ImoTie* Document::tie_notes(ImoNote* pStart, ImoNote* pEnd, ostream& reporter)
{
    LdpAnalyser a(reporter, m_libraryScope, this);
    return a.create_tie(pStart, pEnd);
}

//---------------------------------------------------------------------------------------
void Document::delete_relation(ImoRelObj* pRO)
{
    //special treatment for ties
    if (pRO->is_tie())
    {
        ImoTie* pTie = static_cast<ImoTie*>(pRO);
        pTie->get_start_note()->set_tie_next(NULL);
        pTie->get_end_note()->set_tie_prev(NULL);
    }

    //procedure for all
    ImoNoteRest* pNR = static_cast<ImoNoteRest*>( pRO->get_start_object() );
    pRO->remove_all();
    delete pRO;
    pNR->set_dirty(true);
}

//---------------------------------------------------------------------------------------
void Document::delete_auxobj(ImoAuxObj* pAO)
{
    ImoObj* pParent = pAO->get_parent();
    if (pParent && pParent->is_attachments())       //MUST SUCCESS
    {
        ImoAttachments* pOwner = static_cast<ImoAttachments*>(pParent);
        pOwner->remove_child_imo(pAO);
        delete pAO;
    }
}

//---------------------------------------------------------------------------------------
ImoObj* Document::create_object_from_lmd(const string& source)
{
    LmdParser parser(m_reporter);
    parser.parse_text(source);
    LmdAnalyser a(m_reporter, m_libraryScope, this, &parser);
    XmlNode* tree = parser.get_tree_root();
    ImoObj* pImo = a.analyse_tree_and_get_object(tree);

    ModelBuilder builder;
    builder.structurize(pImo);
    return pImo;


//    Compiler* pCompiler = get_compiler_for_format(format);
//    m_pIModel = pCompiler->compile_string(source);
//    m_pImoDoc = dynamic_cast<ImoDocument*>(m_pIModel->get_root());
//    int numErrors = pCompiler->get_num_errors();
//    delete pCompiler;
//    return numErrors;
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
Observable* Document::get_observable_child(int childType, ImoId childId)
{
    if (childType == Observable::k_imo)
        return static_cast<ImoContentObj*>( get_pointer_to_imo(childId) );
    else if (childType == Observable::k_control)
        return get_pointer_to_control(childId);
    else
    {
        LOMSE_LOG_ERROR("Aborting. Invalid child type.");
        throw std::runtime_error(
            "[Document::get_observable_child] Invalid child type");
    }
}

//---------------------------------------------------------------------------------------
void Document::assign_id(ImoObj* pImo)
{
    m_pIdAssigner->assign_id(pImo);
}

//---------------------------------------------------------------------------------------
void Document::assign_id(Control* pControl)
{
    m_pIdAssigner->assign_id(pControl);
}

//---------------------------------------------------------------------------------------
void Document::removed_from_model(ImoObj* pImo)
{
    m_pIdAssigner->remove(pImo);
}

//---------------------------------------------------------------------------------------
ImoObj* Document::get_pointer_to_imo(ImoId id) const
{
    return m_pIdAssigner->get_pointer_to_imo(id);
}

//---------------------------------------------------------------------------------------
Control* Document::get_pointer_to_control(ImoId id) const
{
    return m_pIdAssigner->get_pointer_to_control(id);
}

//---------------------------------------------------------------------------------------
string Document::dump_ids() const
{
    return m_pIdAssigner->dump();
}


}  //namespace lomse

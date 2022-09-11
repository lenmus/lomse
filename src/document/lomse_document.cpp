//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "private/lomse_document_p.h"
#include "lomse_document.h"
#include "lomse_build_options.h"

#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_ldp_compiler.h"
#include "lomse_lmd_analyser.h"
#include "lomse_xml_parser.h"
#include "lomse_lmd_compiler.h"
#include "lomse_mxl_compiler.h"
#include "lomse_compressed_mxl_compiler.h"
#include "lomse_mnx_compiler.h"
#include "lomse_injectors.h"
#include "lomse_id_assigner.h"
#include "lomse_ldp_exporter.h"
#include "lomse_lmd_exporter.h"
#include "lomse_model_builder.h"
#include "lomse_im_factory.h"
#include "lomse_events.h"
#include "lomse_ldp_elements.h"
#include "lomse_control.h"
#include "lomse_logger.h"
#include "lomse_staffobjs_table.h"
#include "lomse_autoclef.h"
#include "lomse_relobj_cloner.h"

#include <sstream>
using namespace std;

///@cond INTERNALS
namespace lomse
{


//=======================================================================================
// DumpVisitor:  Helper class for traversing the document and printing a dump
//=======================================================================================
class DumpVisitor : public Visitor<ImoObj>
{
protected:
    ostream& m_reporter;
    int m_indent;
    int m_nodesIn;
    int m_nodesOut;
    int m_maxDepth;

public:
    DumpVisitor(ostream& reporter)
        : Visitor<ImoObj>()
        , m_reporter(reporter)
        , m_indent(0)
        , m_nodesIn(0)
        , m_nodesOut(0)
        , m_maxDepth(0)
    {
    }

    int num_in_nodes() { return m_nodesIn; }
    int num_out_nodes() { return m_nodesOut; }
    int max_depth() { return m_maxDepth; }

    void start_visit(ImoObj* pImo) override
    {
        int type = pImo->get_obj_type();
        const string& name = pImo->get_name();
        if (pImo->has_visitable_children())
        {
            m_reporter << indent() << "(" << name << " type " << type
                 << ", id=" << pImo->get_id() << endl;

        }
        else
        {
            m_reporter << indent() << "(" << name << " type " << type
                 << ", id=" << pImo->get_id() << ")" << endl;
        }

        m_indent++;
        m_nodesIn++;
        if (m_maxDepth < m_indent)
            m_maxDepth = m_indent;
    }

	void end_visit(ImoObj* pImo) override
    {
        m_indent--;
        m_nodesOut++;
        if (!pImo->has_visitable_children())
            return;
        m_reporter << indent() << ")" << endl;
    }


protected:

    string indent()
    {
        string spaces = "";
        for (int i=0; i < 3*m_indent; ++i)
            spaces += " ";
        return spaces;
    }

};


//=======================================================================================
// FixModelVisitor:  Helper class for traversing the document and setting the DocModel
// ptr in each ImoObj
    //add ptr to this DocModel in ImoObjs and instantiate ids in IdAssigner as the
    //tree is traversed
//=======================================================================================
class FixModelVisitor : public Visitor<ImoObj>
{
protected:
    DocModel* m_pModel = nullptr;
    IdAssigner* m_pIdAssigner = nullptr;
    ImoId m_maxId = 0L;

public:
    FixModelVisitor(DocModel* pModel, IdAssigner* pIdAssigner)
        : Visitor<ImoObj>(), m_pModel(pModel), m_pIdAssigner(pIdAssigner) {}

    ImoId max_id() { return m_maxId; }

    void start_visit(ImoObj* pImo) override
    {
        //fix DocModel pointer
        pImo->anchor_to_model(m_pModel);

        //collect id and transfer it to IdAssigner
        ImoId id = pImo->get_id();
        m_pIdAssigner->add_id(id, pImo);
        if (pImo->is_control())
        {
            Control* pControl = static_cast<ImoControl*>(pImo)->get_control();
            id = pControl->get_control_id();
            m_pIdAssigner->set_control_id(id, pControl);;
        }

        m_maxId = max(id, m_maxId);
    }
};


//=======================================================================================
// DocModel implementation
//=======================================================================================
DocModel::DocModel(Document* pDoc)
    : m_pDoc(pDoc)
    , m_pIdAssigner( LOMSE_NEW IdAssigner() )
    , m_pImoDoc(nullptr)
    , m_pRelObjCloner(nullptr)
    , m_flags(k_dirty)
    , m_imRef(-1L)
{
}

//---------------------------------------------------------------------------------------
DocModel::~DocModel()
{
    delete m_pImoDoc;
    delete m_pIdAssigner;
    delete m_pRelObjCloner;
}

//---------------------------------------------------------------------------------------
DocModel& DocModel::clone(const DocModel& a)
{
    //instantiate member variables
    m_pDoc = a.m_pDoc;
    m_pIdAssigner = LOMSE_NEW IdAssigner();
    m_pImoDoc = static_cast<ImoDocument*>( ImFactory::clone(a.m_pImoDoc) );
    m_flags = a.m_flags;

    //add ptr to this DocModel in ImoObjs and instantiate ids in IdAssigner as the
    //tree is traversed
    FixModelVisitor v(this, m_pIdAssigner);
    m_pImoDoc->accept_visitor(v);

    //copy xml strings from old IdAssigner
    m_pIdAssigner->copy_strings_from(a.m_pIdAssigner);

    //use the maximum found id to instantiate idCounter
    m_pIdAssigner->set_counter( v.max_id() );

    //build ColStaffObjs and ImMeasureTable
    //TODO: for speed, instead of running everything, only ColStaffObjs and ImMeasureTable
    //      builders are needed. Requires another facade method (simple to do, but not now)
    ModelBuilder builder;
    builder.fix_cloned_model(m_pImoDoc); //build_model(m_pImoDoc);

    add_unique_model_ref();

    delete m_pRelObjCloner;
    m_pRelObjCloner = nullptr;

    return *this;
}

//---------------------------------------------------------------------------------------
void DocModel::add_unique_model_ref()
{
    static long m_refsCounter = 0L;     //global counter to create unique id numbers

    m_imRef = ++m_refsCounter;
}

//---------------------------------------------------------------------------------------
RelObjCloner* DocModel::get_relobj_cloner()
{
    if (m_pRelObjCloner)
        return m_pRelObjCloner;
    else
        return m_pRelObjCloner = LOMSE_NEW RelObjCloner;
}

//---------------------------------------------------------------------------------------
void DocModel::assign_id(ImoObj* pImo)
{
    m_pIdAssigner->assign_id(pImo);
}

//---------------------------------------------------------------------------------------
ImoId DocModel::reserve_id(ImoId id)
{
    return m_pIdAssigner->reserve_id(id);
}

//---------------------------------------------------------------------------------------
void DocModel::assign_id(Control* pControl)
{
    m_pIdAssigner->assign_id(pControl);
}

//---------------------------------------------------------------------------------------
string DocModel::get_xml_id_for(ImoId id) const
{
    return m_pIdAssigner->get_xml_id_for(id);
}

//---------------------------------------------------------------------------------------
void DocModel::set_xml_id_for(ImoId id, const string& value)
{
    m_pIdAssigner->set_xml_id_for(id, value);
}

//---------------------------------------------------------------------------------------
size_t DocModel::id_assigner_size() const
{
    return m_pIdAssigner->size();
}

//---------------------------------------------------------------------------------------
string DocModel::dump_id_assigner() const
{
    return m_pIdAssigner->dump();
}

//---------------------------------------------------------------------------------------
ImoObj* DocModel::get_pointer_to_imo(const std::string& xmlId) const
{
    return m_pIdAssigner->get_pointer_to_imo(xmlId);
}

//---------------------------------------------------------------------------------------
ImoObj* DocModel::get_pointer_to_imo(ImoId id) const
{
    return m_pIdAssigner->get_pointer_to_imo(id);
}

//---------------------------------------------------------------------------------------
Control* DocModel::get_pointer_to_control(ImoId id) const
{
    return m_pIdAssigner->get_pointer_to_control(id);
}

//---------------------------------------------------------------------------------------
void DocModel::reset_id_assigner()
{
    m_pIdAssigner->reset();
}

//---------------------------------------------------------------------------------------
void DocModel::on_removed_from_model(ImoObj* pImo)
{
    m_pIdAssigner->remove(pImo);
}



//=======================================================================================
// Document implementation
//=======================================================================================
Document::Document(LibraryScope& libraryScope, ostream& reporter)
    : EventNotifier(libraryScope.get_events_dispatcher())
    , Observable()
    , m_libraryScope(libraryScope)
    , m_reporter(reporter)
    , m_docScope(reporter)
    , m_modified(0)
    , m_pModel( LOMSE_NEW DocModel(this) )
    , m_pModelCopy(nullptr)
{
}

//---------------------------------------------------------------------------------------
Document::~Document()
{
    //set pointers to nullptr to avoid callbacks (e.g. set_dirty()) while deleting them
    DocModel* pModel = m_pModel;
    DocModel* pModelCopy = m_pModelCopy;
    m_pModel = nullptr;
    m_pModelCopy = nullptr;

    //now it is safe to delete the models
    delete pModel;
    delete pModelCopy;

    delete_observers();
}

//---------------------------------------------------------------------------------------
void Document::initialize()
{
    if (m_pModel->m_pImoDoc)
    {
        LOMSE_LOG_ERROR("Aborting. Attempting to create already created document");
        throw std::runtime_error(
            "[Document::create] Attempting to create already created document");
    }

    m_pModel->set_dirty();
    m_pModel->m_pImoDoc = nullptr;
    m_modified = 0;
}

//---------------------------------------------------------------------------------------
void Document::set_imo_doc(ImoDocument* pImoDoc)
{
    delete m_pModel->m_pImoDoc;
    m_pModel->m_pImoDoc = pImoDoc;
}

//---------------------------------------------------------------------------------------
int Document::from_file(const string& filename, int format)
{
    initialize();
    int numErrors = 0;
    Compiler* pCompiler = get_compiler_for_format(format);
    if (pCompiler)
    {
        m_pModel->m_pImoDoc = pCompiler->compile_file(filename);
        numErrors = pCompiler->get_num_errors();
        delete pCompiler;
    }
    else
    {
        m_reporter << "File format not supported." << endl;
        numErrors = 1;
    }

    if (m_pModel->m_pImoDoc == nullptr)
        create_empty();

    if (m_pModel->m_pImoDoc && (format == Document::k_format_mxl || format == Document::k_format_mxl_compressed))
        fix_malformed_musicxml();


//    //DEBUG: Force to use a cloned copy
//    create_backup_copy();
//    restore_from_backup_copy();

    return numErrors;
}

//---------------------------------------------------------------------------------------
int Document::from_string(const string& source, int format)
{
    initialize();
    int numErrors = 0;
    Compiler* pCompiler = get_compiler_for_format(format);
    if (pCompiler)
    {
        m_pModel->m_pImoDoc = pCompiler->compile_string(source);
        numErrors = pCompiler->get_num_errors();
        delete pCompiler;
    }
    else
    {
        m_reporter << "File format not supported." << endl;
        numErrors = 1;
    }

    if (m_pModel->m_pImoDoc == nullptr)
        create_empty();

    if (m_pModel->m_pImoDoc && (format == Document::k_format_mxl || format == Document::k_format_mxl_compressed))
        fix_malformed_musicxml();

    return numErrors;
}

//---------------------------------------------------------------------------------------
int Document::from_input(LdpReader& reader)
{
    initialize();
    try
    {
        LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
        m_pModel->m_pImoDoc = pCompiler->compile_input(reader);
        int numErrors = pCompiler->get_num_errors();
        delete pCompiler;
        return numErrors;
    }
    catch (...)
    {
        //this avoids programs crashes when a document is malformed but
        //will produce memory lekeages
        m_pModel->m_pImoDoc = nullptr;
        create_empty();
        return 0;
    }
}

//---------------------------------------------------------------------------------------
void Document::create_empty()
{
    initialize();
    LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
    m_pModel->m_pImoDoc = pCompiler->create_empty();
    delete pCompiler;
}

//---------------------------------------------------------------------------------------
void Document::create_with_empty_score()
{
    initialize();
    LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
    m_pModel->m_pImoDoc = pCompiler->create_with_empty_score();
    delete pCompiler;
}

//---------------------------------------------------------------------------------------
void Document::end_of_changes()
{
    ModelBuilder builder;
    builder.build_model(m_pModel->m_pImoDoc);
    m_pModel->add_unique_model_ref();
}

//---------------------------------------------------------------------------------------
void Document::fix_malformed_musicxml()
{
    if (m_libraryScope.get_musicxml_options()->use_default_clefs())
    {
        ImoScore* pScore = dynamic_cast<ImoScore*>( m_pModel->m_pImoDoc->get_content_item(0) );
        if (pScore)
        {
            AutoClef ac(pScore);
            ac.do_autoclef();
        }
    }
}

//---------------------------------------------------------------------------------------
string Document::to_string(bool fWithIds)
{
    LdpExporter exporter;
    exporter.set_remove_newlines(true);
    exporter.set_add_id(fWithIds);
    return exporter.get_source(m_pModel->m_pImoDoc);
}

//---------------------------------------------------------------------------------------
void Document::create_backup_copy()
{
    delete m_pModelCopy;
    m_pModelCopy = LOMSE_NEW DocModel(*m_pModel);
}

//---------------------------------------------------------------------------------------
DocModel* Document::create_model_copy()
{
    return LOMSE_NEW DocModel(*m_pModel);
}

//---------------------------------------------------------------------------------------
void Document::replace_model(DocModel* pNewModel)
{
    delete m_pModel;
    m_pModel = pNewModel;

    delete m_pModelCopy;
    m_pModelCopy = nullptr;
}

//---------------------------------------------------------------------------------------
void Document::restore_from_backup_copy()
{
    if (m_pModelCopy)
    {
        delete m_pModel;
        m_pModel = m_pModelCopy;
        m_pModelCopy = nullptr;
    }
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

        case k_format_mxl:
            return Injector::inject_MxlCompiler(m_libraryScope, this);

#if (LOMSE_ENABLE_COMPRESSION == 1)
        case k_format_mxl_compressed:
            return Injector::inject_CompressedMxlCompiler(m_libraryScope, this);
#endif

        case k_format_mnx:
            return Injector::inject_MnxCompiler(m_libraryScope, this);

        default:
            return nullptr;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
bool Document::is_editable()
{
    //TODO: How to mark a document as 'not editable'?
    //For now, all documents are editable
    return true;
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
            return nullptr;
        }
    }
    return nullptr;
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

                //add the tuplet to intermediate notes
                int numTuplet = pTupletStart->get_item_number();
                ColStaffObjs* pCol = pStartNR->get_score()->get_staffobjs_table();
                ColStaffObjsIterator it = pCol->find(pStartNR);
                int line = (*it)->line();
                ++it;   //skip start note
                while ((*it)->imo_object() != pEndNR)
                {
                    if ((*it)->imo_object()->is_note_rest() && (*it)->line() == line)
                    {
                        ImoNoteRest* pNR = static_cast<ImoNoteRest*>((*it)->imo_object());
                        ImoTupletDto* pInfo = LOMSE_NEW ImoTupletDto();
                        pInfo->set_tuplet_type(ImoTupletDto::k_continue);
                        pInfo->set_note_rest(pNR);
                        pInfo->set_tuplet_number(numTuplet);
                        a.add_relation_info(pInfo);
                    }
                    ++it;
                }

                //create end of tuplet and attach it to end note/rest
                ImoTupletDto* pTupletEnd = LOMSE_NEW ImoTupletDto();
                pTupletEnd->set_tuplet_type(ImoTupletDto::k_stop);
                pTupletEnd->set_note_rest(pEndNR);
                pTupletEnd->set_tuplet_number(numTuplet);
                a.add_relation_info(pTupletEnd);

                //get the tuplet
                return a.get_last_created_tuplet();
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
    return nullptr;
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
        pTie->get_start_note()->set_tie_next(nullptr);
        pTie->get_end_note()->set_tie_prev(nullptr);
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
    XmlParser parser(m_reporter);
    parser.parse_text(source);
    LmdAnalyser a(m_reporter, m_libraryScope, this, &parser);
    XmlNode* tree = parser.get_tree_root();
    ImoObj* pImo = a.analyse_tree_and_get_object(tree);

    ModelBuilder builder;
    builder.structurize(pImo);
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
    ImoObj* pRoot = a.analyse_tree(tree, "string:");
    ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>(pRoot);
    ImoObj::children_iterator it = pMusic->begin();
    while (it != pMusic->end())
    {
        ImoObj* pImo = *it;
        pMusic->remove_child(pImo);
        pMD->append_child_imo(pImo);
        it = pMusic->begin();
    }
    delete tree->get_root();
    delete pRoot;
}

//---------------------------------------------------------------------------------------
ImoStyle* Document::create_style(const string& name, const string& parent)
{
    return m_pModel->m_pImoDoc->create_style(name, parent);
}

//---------------------------------------------------------------------------------------
ImoStyle* Document::create_private_style(const string& parent)
{
    return m_pModel->m_pImoDoc->create_private_style(parent);
}

//---------------------------------------------------------------------------------------
ImoStyle* Document::get_default_style()
{
    return m_pModel->m_pImoDoc->get_default_style();
}
//
////---------------------------------------------------------------------------------------
//ImoStyle* Document::find_style(const string& name)
//{
//    return m_pModel->m_pImoDoc->find_style(name);
//}

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
ImoObj* Document::get_pointer_to_imo(ImoId id) const
{
    return m_pModel->get_pointer_to_imo(id);
}

//---------------------------------------------------------------------------------------
ImoObj* Document::get_pointer_to_imo(const std::string& xmlId) const
{
    return m_pModel->get_pointer_to_imo(xmlId);
}

//---------------------------------------------------------------------------------------
Control* Document::get_pointer_to_control(ImoId id) const
{
    return m_pModel->get_pointer_to_control(id);
}

//---------------------------------------------------------------------------------------
string Document::dump_ids() const
{
    return m_pModel->dump_id_assigner();
}

//---------------------------------------------------------------------------------------
size_t Document::id_assigner_size() const
{
    return m_pModel->id_assigner_size();
}

//---------------------------------------------------------------------------------------
string Document::dump_tree() const
{
    stringstream data;

    DumpVisitor v(data);
    ImoDocument* pRoot = get_im_root();
    pRoot->accept_visitor(v);

    return data.str();
}

//---------------------------------------------------------------------------------------
ADocument Document::get_document_api()
{
    return ADocument(this);
}

//---------------------------------------------------------------------------------------
bool Document::is_valid_model(long imRef)
{
    return m_pModel->is_valid_model(imRef);
}

//---------------------------------------------------------------------------------------
long Document::get_model_ref()
{
    return m_pModel->get_model_ref();
}

///@endcond


EImoObjType object_type_to_imo_type(EDocObject type)
{
    switch(type)
    {
        case k_obj_anonymous_block:     return k_imo_anonymous_block;
        case k_obj_button:              return k_imo_button;
        case k_obj_content:             return k_imo_content;
        case k_obj_control:             return k_imo_control;
        case k_obj_dynamic:             return k_imo_dynamic;
        case k_obj_heading:             return k_imo_heading;
        case k_obj_image:               return k_imo_image;
        case k_obj_inline_wrapper:      return k_imo_inline_wrapper;
        case k_obj_instrument:          return k_imo_instrument;
        case k_obj_instr_group:         return k_imo_instr_group;
        case k_obj_link:                return k_imo_link;
        case k_obj_list:                return k_imo_list;
        case k_obj_list_item:           return k_imo_listitem;
        case k_obj_midi_info:           return k_imo_midi_info;
        case k_obj_multicolumn:         return k_imo_multicolumn;
        case k_obj_music_data:          return k_imo_music_data;
        case k_obj_paragraph:           return k_imo_para;
        case k_obj_score:               return k_imo_score;
        case k_obj_score_player:        return k_imo_score_player;
        case k_obj_sound_info:          return k_imo_sound_info;
        case k_obj_table:               return k_imo_table;
        case k_obj_table_cell:          return k_imo_table_cell;
        case k_obj_table_row:           return k_imo_table_row;
        case k_obj_text_item:           return k_imo_text_item;
        default:
            return k_imo_obj;
    }
}

//=======================================================================================
/** @class ADocument
    The %ADocument class is the API root object that contains, basically, the @IM,
    a model similar to the DOM in HTML. And the children of this root element represent
    the basic blocks for building a document: headers, paragraphs, music scores, lists,
    tables, images, etc.

    You can consider a lomse document similar to an HTML document but it can include also
    music scores. Another difference is that the document objects support styles, but
    not CSS. Therefore, you can consider the lomse document as a generic rich text
    document that also can
    contain full-fledged music scores. It is mainly oriented to display music scores
    and to have them inserted in an interactive text document, such as a music theory
    book with chapters, texts, music scores and interactive music exercises.

    The lomse document supports the most common objects for textual content, such as
    headings, paragraphs, lists, tables and images, plus specific objects for music
    scores.
    But for music scores the DOM model analogy is not truly feasible as many music notation
    markings (like a slur or beam) represent links between objects in the tree.
    Therefore, lomse or any other program have to maintain
    self-consistency when there are 'paired elements' representing the starts and ends
    of things like beams, crescendo markings, slurs, and so on. Therefore, the @IM
    contains additional structures for correctly representing a music score.

    Of course, the document can contain just one full-score, and this will be the case
    when importing music scores in other formats, such as a MusicXML score file.

    See @ref api-internal-model-adocument-content

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/

///@cond INTERNALS

ADocument::ADocument(Document* impl)
    : m_pImpl(impl)
{
}

///@endcond


//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns @TRUE if the object represents a valid document.
*/
bool ADocument::is_valid() const
{
    return m_pImpl != nullptr;
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the internal unique identifier (ID) for this document.
*/
ImoId ADocument::object_id() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
    return pRoot->get_id();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    For documents created from sources in LMD format this method will return
    the LMD version used in the source. For other document creation methods and
    formats it will return version "0.0".
*/
std::string& ADocument::lmd_version() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
    return pRoot->get_version();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Creates a new detached AObject of the type specified by parameter <i>type</i>.
    @param type     A value from enum #EDocObject that specifies the type of object
                    to be created.

    The created object is <i>detached</i>. This means that although it is part of the
    document, this object is not attached as content. It exists in memory and the document
    knows about it, but it is not part of the visible content. It is 'waiting' to be
    inserted at some place in the content. To attach to the document a detached object it
    is necessary to use specific methods, such as ADocument::append_child().
*/
AObject ADocument::create_object(EDocObject type)
{
    EImoObjType t = object_type_to_imo_type(type);
    if (t == k_imo_obj)
        return AObject();   //invalid conversion. Fix object_type_to_imo_type() function

    Document* pDoc = pimpl();
    ImoObj* pImo = ImFactory::inject(t, pDoc);
    long mref = pDoc->get_model_ref();
    switch(type)
    {
//        case k_obj_anonymous_block:     return IAnonymousBlock(pImo, pDoc, mref);
//        case k_obj_button:              return AScore(pImo, pDoc, mref);
//        case k_obj_content:             return IContent(pImo, pDoc, mref);
//        case k_obj_control:             return IControl(pImo, pDoc, mref);
        case k_obj_dynamic:             return ADynamic(pImo, pDoc, mref);
//        case k_obj_heading:             return IHeading(pImo, pDoc, mref);
//        case k_obj_image:               return IImage(pImo, pDoc, mref);
//        case k_obj_inline_wrapper:      return IInlineWrapper(pImo, pDoc, mref);
        case k_obj_instrument:          return AInstrument(pImo, pDoc, mref);
        case k_obj_instr_group:         return AInstrGroup(pImo, pDoc, mref);
        case k_obj_link:                return ALink(pImo, pDoc, mref);
        case k_obj_list:                return AList(pImo, pDoc, mref);
//        case k_obj_list_item:           return AListitem(pImo, pDoc, mref);
        case k_obj_midi_info:           return AMidiInfo(pImo, pDoc, mref);
//        case k_obj_multicolumn:         return IMulticolumn(pImo, pDoc, mref);
//        case k_obj_music_data:          return IMusicData(pImo, pDoc, mref);
        case k_obj_paragraph:           return AParagraph(pImo, pDoc, mref);
        case k_obj_score:               return AScore(pImo, pDoc, mref);
//        case k_obj_score_player:        return AScorePlayer(pImo, pDoc, mref);
        case k_obj_sound_info:          return ASoundInfo(pImo, pDoc, mref);
//        case k_obj_table:               return ITable(pImo, pDoc, mref);
//        case k_obj_table_cell:          return ITableCell(pImo, pDoc, mref);
//        case k_obj_table_row:           return ITableRow(pImo, pDoc, mref);
        case k_obj_text_item:           return ATextItem(pImo, pDoc, mref);
        default:
            return AObject();   //missing case. Fix this switch block
    }
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Appends a detached AObject to document content. The object to append must be a blocks
    contained object. Otherwise, nothing will be done. Returns @TRUE is the object has
    been successfully attached. Otherwise, the objects will continue existing as a
    detached object.
    @param detachedObj     The detached object to append to document content.

    @see ADocument::create_object()
*/
bool ADocument::append_child(const AObject& detachedObj)
{
    if (detachedObj.is_valid())
    {
        ImoObj* pImo = detachedObj.m_pImpl;
        if (pImo && pImo->is_block_level_obj())
        {
            pimpl()->append_content_item(static_cast<ImoBlockLevelObj*>(pImo));
            return true;
        }
    }
    return false;
}


/** @name Page size and margins

    Margings, in lomse, documents, are strips of white space around the edge of the
    paper. The wider the left and right margins, the narrower the page content. The
    wider the top and bottom margins, the shorter the page content.

    In most cases, it is not necessary to change the default page margins. However,
    you can change the margins with these methods.
*/
//@{

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the left margin in odd pages. The new margin value is specified in logical
    units (cents of a millimeter).
*/
void ADocument::set_page_left_margin_odd(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_left_margin_odd(value);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the right margin in odd pages. The new margin value is specified in logical
    units (cents of a millimeter).
*/
void ADocument::set_page_right_margin_odd(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_right_margin_odd(value);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the top margin in odd pages. The new margin value is specified in logical
    units (cents of a millimeter).
*/
void ADocument::set_page_top_margin_odd(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_top_margin_odd(value);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the bottom margin in odd pages. The new margin value is specified in logical
    units (cents of a millimeter).
*/
void ADocument::set_page_bottom_margin_odd(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_bottom_margin_odd(value);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the left margin in even pages. The new margin value is specified in logical
    units (cents of a millimeter).
*/
void ADocument::set_page_left_margin_even(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_left_margin_even(value);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the right margin in even pages. The new margin value is specified in logical
    units (cents of a millimeter).
*/
void ADocument::set_page_right_margin_even(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_right_margin_even(value);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the top margin in even pages. The new margin value is specified in logical
    units (cents of a millimeter).
*/
void ADocument::set_page_top_margin_even(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_top_margin_even(value);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the bottom margin in even pages. The new margin value is specified in logical
    units (cents of a millimeter).
*/
void ADocument::set_page_bottom_margin_even(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_bottom_margin_even(value);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the paper size intended for rendering this document. The size values
    are in logical units (cents of a millimeter).
*/
void ADocument::set_page_size(USize uPageSize)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_page_size(uPageSize);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the paper width intended for rendering this document. The width value
    is in logical units (cents of a millimeter).
*/
void ADocument::set_page_width(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->set_page_width(value);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Sets the paper height intended for rendering this document. The height value
    is in logical units (cents of a millimeter).
*/
void ADocument::set_page_height(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->set_page_height(value);
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the left margin for odd pages. The returned value is in logical
    units (cents of a millimeter).
*/
LUnits ADocument::page_left_margin_odd() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_left_margin_odd();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the right margin for odd pages. The returned value is in logical
    units (cents of a millimeter).
*/
LUnits ADocument::page_right_margin_odd() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_right_margin_odd();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the top margin for odd pages. The returned value is in logical
    units (cents of a millimeter).
*/
LUnits ADocument::page_top_margin_odd() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_top_margin_odd();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the bottom margin for odd pages. The returned value is in logical
    units (cents of a millimeter).
*/
LUnits ADocument::page_bottom_margin_odd() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_bottom_margin_odd();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the left margin for even pages. The returned value is in logical
    units (cents of a millimeter).
*/
LUnits ADocument::page_left_margin_even() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_left_margin_even();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the right margin for even pages. The returned value is in logical
    units (cents of a millimeter).
*/
LUnits ADocument::page_right_margin_even() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_right_margin_even();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the top margin for even pages. The returned value is in logical
    units (cents of a millimeter).
*/
LUnits ADocument::page_top_margin_even() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_top_margin_even();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the bottom margin for even pages. The returned value is in logical
    units (cents of a millimeter).
*/
LUnits ADocument::page_bottom_margin_even() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_bottom_margin_even();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the paper size intended for rendering this document. The returned value
    is in logical units (cents of a millimeter).
*/
USize ADocument::page_size() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_page_size();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the paper width intended for rendering this document. The returned value
    is in logical units (cents of a millimeter).
*/
LUnits ADocument::page_width() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_page_width();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the paper height intended for rendering this document. The returned value
    is in logical units (cents of a millimeter).
*/
LUnits ADocument::page_height() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_page_height();
}

//@}    //Page size and margins


/// @name Page content scale
//@{

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Return the scaling factor to apply to the content when rendered divided into
    pages of the size defined by the paper size. Normally this factor is 1.0.
*/
float ADocument::page_content_scale() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
    return pRoot->get_page_content_scale();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Set the scaling factor to apply to the content when rendered divided into
    pages of the size defined by the paper size. By default this factor is 1.0.
*/
void ADocument::set_page_content_scale(float scale)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
    pRoot->set_page_content_scale(scale);
}

//@}    //Page content scale


/// @name Content traversal
//@{

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the number of objects contained, at first level, in this document.
*/
int ADocument::num_children() const
{
    return const_cast<Document*>(pimpl())->get_num_content_items();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the specified child object.
    @param iItem    Is the index to the requested child (0 ... num.children - 1)
*/
AObject ADocument::child_at(int iItem) const
{
    Document* pDoc = const_cast<Document*>(pimpl());
    ImoDocument* pRoot = pDoc->get_im_root();
    ImoObj* pImo = pRoot->get_content_item(iItem);
    if (pImo)
        return AObject(pImo, pDoc, pDoc->get_model_ref()).downcast_to_content_obj();
    else
        return AObject();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the first child object.
*/
AObject ADocument::first_child() const
{
    Document* pDoc = const_cast<Document*>(pimpl());
    ImoDocument* pRoot = pDoc->get_im_root();
    ImoObj* pImo = pRoot->get_first_content_item();
    if (pImo)
        return AObject(pImo, pDoc, pDoc->get_model_ref()).downcast_to_content_obj();
    else
        return AObject();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the last child object.
*/
AObject ADocument::last_child() const
{
    Document* pDoc = const_cast<Document*>(pimpl());
    ImoDocument* pRoot = pDoc->get_im_root();
    ImoObj* pImo = pRoot->get_last_content_item();
    if (pImo)
        return AObject(pImo, pDoc, pDoc->get_model_ref()).downcast_to_content_obj();
    else
        return AObject();
}

//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Returns the first child object of type 'music score'.
*/
AScore ADocument::first_score() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
    ImoContent* pContent = pRoot->get_content();
    ImoObj::children_iterator it;
    for (it= pContent->begin(); it != pContent->end(); ++it)
    {
        if ((*it)->is_score())
        {
            ImoScore* pScore = static_cast<ImoScore*>(*it);
            Document* pDoc = const_cast<Document*>(pimpl());
            return AScore(pScore, pDoc, pDoc->get_model_ref());
        }
    }
    return AScore();
}

//@}    //Content traversal


//---------------------------------------------------------------------------------------
/** @memberof ADocument
    When you modify the content of a document it is necessary to update some
    structures associated to music scores.
    For this it is mandatory to invoke this method. Alternatively, you can
    invoke AScore::end_of_changes(), on the modified scores.
*/
void ADocument::end_of_changes()
{
    pimpl()->end_of_changes();
}


//---------------------------------------------------------------------------------------
/** @memberof ADocument
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method, open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
Document* ADocument::internal_object()
{
    return pimpl();
}




}  //namespace lomse

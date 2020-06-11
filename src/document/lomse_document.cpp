//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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
    LibraryScope& m_libraryScope;
    ostream& m_reporter;

    int m_indent;
    int m_nodesIn;
    int m_nodesOut;
    int m_maxDepth;

public:
    DumpVisitor(LibraryScope& libraryScope, ostream& reporter)
        : Visitor<ImoObj>()
        , m_libraryScope(libraryScope)
        , m_reporter(reporter)
        , m_indent(0)
        , m_nodesIn(0)
        , m_nodesOut(0)
        , m_maxDepth(0)
    {
    }

	virtual ~DumpVisitor() {}

    int num_in_nodes() { return m_nodesIn; }
    int num_out_nodes() { return m_nodesOut; }
    int max_depth() { return m_maxDepth; }

    void start_visit(ImoObj* pImo)
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

	void end_visit(ImoObj* pImo)
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
    , m_pImoDoc(nullptr)
    , m_flags(k_dirty)
    , m_modified(0)
    , m_imRef(0L)
{
}

//---------------------------------------------------------------------------------------
Document::~Document()
{
    delete m_pImoDoc;
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
    m_pImoDoc = nullptr;
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
    int numErrors = 0;
    Compiler* pCompiler = get_compiler_for_format(format);
    if (pCompiler)
    {
        m_pImoDoc = pCompiler->compile_file(filename);
        numErrors = pCompiler->get_num_errors();
        delete pCompiler;
    }
    else
    {
        m_reporter << "File format not supported." << endl;
        numErrors = 1;
    }

    if (m_pImoDoc == nullptr)
        create_empty();

    if (m_pImoDoc && format == Document::k_format_mxl)
        fix_malformed_musicxml();

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
        m_pImoDoc = pCompiler->compile_string(source);
        numErrors = pCompiler->get_num_errors();
        delete pCompiler;
    }
    else
    {
        m_reporter << "File format not supported." << endl;
        numErrors = 1;
    }

    if (m_pImoDoc == nullptr)
        create_empty();

    if (m_pImoDoc && format == Document::k_format_mxl)
        fix_malformed_musicxml();

    return numErrors;
}

//---------------------------------------------------------------------------------------
int Document::from_checkpoint(const string& data)
{
    //delete old internal model
    m_pImoDoc = nullptr;
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
    assigner.copy_ids_to(m_pIdAssigner, id);

    return 0;
}

//---------------------------------------------------------------------------------------
int Document::from_input(LdpReader& reader)
{
    initialize();
    try
    {
        LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
        m_pImoDoc = pCompiler->compile_input(reader);
        int numErrors = pCompiler->get_num_errors();
        delete pCompiler;
        return numErrors;
    }
    catch (...)
    {
        //this avoids programs crashes when a document is malformed but
        //will produce memory lekeages
        m_pImoDoc = nullptr;
        create_empty();
        return 0;
    }
}

//---------------------------------------------------------------------------------------
void Document::create_empty()
{
    initialize();
    LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
    m_pImoDoc = pCompiler->create_empty();
    delete pCompiler;
}

//---------------------------------------------------------------------------------------
void Document::create_with_empty_score()
{
    initialize();
    LdpCompiler* pCompiler  = Injector::inject_LdpCompiler(m_libraryScope, this);
    m_pImoDoc = pCompiler->create_with_empty_score();
    delete pCompiler;
}

//---------------------------------------------------------------------------------------
void Document::end_of_changes()
{
    ModelBuilder builder;
    builder.build_model(m_pImoDoc);
    ++m_imRef;
}

//---------------------------------------------------------------------------------------
void Document::fix_malformed_musicxml()
{
    if (m_libraryScope.get_musicxml_options()->use_default_clefs())
    {
        ImoScore* pScore = dynamic_cast<ImoScore*>( m_pImoDoc->get_content_item(0) );
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
    //exporter.set_remove_newlines(true);   //TODO: Commented out to facilitate debugging
    exporter.set_add_id(true);
    exporter.set_score_format(LmdExporter::k_format_ldp);
    return exporter.get_source(pImo);

    //code if Ldp format:
        //LdpExporter exporter;
        //exporter.set_remove_newlines(true);
        //exporter.set_add_id(true);
        //return exporter.get_source(m_pImo);
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
ImoId Document::reserve_id(ImoId id)
{
    return m_pIdAssigner->reserve_id(id);
}

//---------------------------------------------------------------------------------------
void Document::assign_id(Control* pControl)
{
    m_pIdAssigner->assign_id(pControl);
}

//---------------------------------------------------------------------------------------
void Document::on_removed_from_model(ImoObj* pImo)
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

//---------------------------------------------------------------------------------------
size_t Document::id_assigner_size() const
{
    return m_pIdAssigner->size();
}

//---------------------------------------------------------------------------------------
string Document::dump_tree() const
{
    stringstream data;

    DumpVisitor v(m_libraryScope, data);
    ImoDocument* pRoot = get_im_root();
    pRoot->accept_visitor(v);

    return data.str();
}

//---------------------------------------------------------------------------------------
unique_ptr<IDocument> Document::get_document_api()
{
    return unique_ptr<IDocument>(new IDocument(this));
}

//---------------------------------------------------------------------------------------
bool Document::is_valid_model(long imRef)
{
    return m_imRef == imRef;
}

//---------------------------------------------------------------------------------------
long Document::get_model_ref()
{
    return m_imRef;
}

///@endcond

//=======================================================================================
/** @class IDocument
    The %IDocument class is the API root object that contains, basically, the @IM,
    a model similar to the DOM in HTML.
    It represents the whole document and allows to access and modify the content
    of the document. By accessing and modifying this internal model you
    have full control over the document content and its properties.

    The %IDocument class also encapsulates all the internals, providing the basic API
    for creating, modifying and using a document.

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/

///@cond INTERNALS

IDocument::IDocument(Document* impl)
    : m_pImpl(impl)
{
}

///@endcond


//---------------------------------------------------------------------------------------
/** @memberof IDocument
    Returns the internal unique identifier (ID) for this document.
*/
ImoId IDocument::get_object_id() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
    return pRoot->get_id();
}

//---------------------------------------------------------------------------------------
/** @memberof IDocument
    For documents created from sources in LMD format this method will return
    the LMD version used in the source. For other document creation methods and
    formats it will return version "0.0".
*/
std::string& IDocument::get_lmd_version() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
    return pRoot->get_version();
}


/// @name Page size and margins
//@{

//---------------------------------------------------------------------------------------
void IDocument::set_page_left_margin(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_left_margin(value);
}

//---------------------------------------------------------------------------------------
void IDocument::set_page_right_margin(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_right_margin(value);
}

//---------------------------------------------------------------------------------------
void IDocument::set_page_top_margin(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_top_margin(value);
}

//---------------------------------------------------------------------------------------
void IDocument::set_page_bottom_margin(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_bottom_margin(value);
}

//---------------------------------------------------------------------------------------
void IDocument::set_page_binding_margin(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_binding_margin(value);
}

//---------------------------------------------------------------------------------------
void IDocument::set_page_size(USize uPageSize)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    pageInfo->set_page_size(uPageSize);
}

//---------------------------------------------------------------------------------------
void IDocument::set_page_width(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->set_page_width(value);
}

//---------------------------------------------------------------------------------------
void IDocument::set_page_height(LUnits value)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->set_page_height(value);
}

//---------------------------------------------------------------------------------------
LUnits IDocument::get_page_left_margin() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_left_margin();
}

//---------------------------------------------------------------------------------------
LUnits IDocument::get_page_right_margin() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_right_margin();
}

//---------------------------------------------------------------------------------------
LUnits IDocument::get_page_top_margin() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_top_margin();
}

//---------------------------------------------------------------------------------------
LUnits IDocument::get_page_bottom_margin() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_bottom_margin();
}

//---------------------------------------------------------------------------------------
LUnits IDocument::get_page_binding_margin() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_binding_margin();
}

//---------------------------------------------------------------------------------------
USize IDocument::get_page_size() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_page_size();
}

//---------------------------------------------------------------------------------------
/** @memberof IDocument
    Returns the paper width intended for rendering this document. The returned value
    is in logical units (cents of a millimeter).
*/
LUnits IDocument::get_page_width() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_page_width();
}

//---------------------------------------------------------------------------------------
/** @memberof IDocument
    Returns the paper height intended for rendering this document. The returned value
    is in logical units (cents of a millimeter).
*/
LUnits IDocument::get_page_height() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
	ImoPageInfo* pageInfo = pRoot->get_page_info();
    return pageInfo->get_page_height();
}

//@}    //Page size and margins


/// @name Page content scale
//@{

//---------------------------------------------------------------------------------------
/** @memberof IDocument
    Return the scaling factor to apply to the content when rendered divided into
    pages of the size defined by the paper size. Normally this factor is 1.0.
*/
float IDocument::get_page_content_scale() const
{
    ImoDocument* pRoot = pimpl()->get_im_root();
    return pRoot->get_page_content_scale();
}

//---------------------------------------------------------------------------------------
/** @memberof IDocument
    Set the scaling factor to apply to the content when rendered divided into
    pages of the size defined by the paper size. By default this factor is 1.0.
*/
void IDocument::set_page_content_scale(float scale)
{
    ImoDocument* pRoot = pimpl()->get_im_root();
    pRoot->set_page_content_scale(scale);
}

//@}    //Page content scale


/// @name Content traversal
//@{

//---------------------------------------------------------------------------------------
/** @memberof IDocument
    Returns the number of objects contained, at first level, in this document.
*/
int IDocument::get_num_children() const
{
    return const_cast<Document*>(pimpl())->get_num_content_items();
}

//---------------------------------------------------------------------------------------
std::unique_ptr<IObject> IDocument::get_child_at(int iItem) const
{
    Document* pDoc = const_cast<Document*>(pimpl());
    ImoDocument* pRoot = pDoc->get_im_root();
    ImoObj* pImo = pRoot->get_content_item(iItem);
    if (pImo)
        return IObject(pImo, pDoc, pDoc->get_model_ref()).downcast_to_content_obj();
    else
        return unique_ptr<IObject>();
}

//---------------------------------------------------------------------------------------
std::unique_ptr<IObject> IDocument::get_first_child() const
{
    Document* pDoc = const_cast<Document*>(pimpl());
    ImoDocument* pRoot = pDoc->get_im_root();
    ImoObj* pImo = pRoot->get_first_content_item();
    if (pImo)
        return IObject(pImo, pDoc, pDoc->get_model_ref()).downcast_to_content_obj();
    else
        return unique_ptr<IObject>();
}

//---------------------------------------------------------------------------------------
std::unique_ptr<IObject> IDocument::get_last_child() const
{
    Document* pDoc = const_cast<Document*>(pimpl());
    ImoDocument* pRoot = pDoc->get_im_root();
    ImoObj* pImo = pRoot->get_last_content_item();
    if (pImo)
        return IObject(pImo, pDoc, pDoc->get_model_ref()).downcast_to_content_obj();
    else
        return unique_ptr<IObject>();
}

//---------------------------------------------------------------------------------------
std::unique_ptr<IScore> IDocument::get_first_score() const
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
            return unique_ptr<IScore>( new IScore(pScore, pDoc, pDoc->get_model_ref()) );
        }
    }
    return unique_ptr<IScore>();
}

//@}    //Content traversal


//---------------------------------------------------------------------------------------
/** @memberof IDocument
    When you modify the content of a document it is necessary to update some
    structures associated to music scores.
    For this it is mandatory to invoke this method. Alternatively, you can
    invoke IScore::end_of_changes(), on the modified scores.
*/
void IDocument::end_of_changes()
{
    pimpl()->end_of_changes();
}


//---------------------------------------------------------------------------------------
/** @memberof IDocument
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method, open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
Document* IDocument::get_internal_object()
{
    return pimpl();
}




}  //namespace lomse

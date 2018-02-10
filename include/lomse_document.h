//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#ifndef __LOMSE_DOCUMENT_H__
#define __LOMSE_DOCUMENT_H__

#include "lomse_injectors.h"
#include "lomse_observable.h"
#include "lomse_ldp_elements.h"
#include "lomse_basic.h"
#include "lomse_internal_model.h"
#include "lomse_events.h"
#include "lomse_reader.h"

#include <sstream>
using namespace std;

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class DocCommand;
class DocCommandExecuter;
class Compiler;
class IdAssigner;
class Interactor;
class ImoDocument;
class ImoMusicData;
class ImoScore;
class ImoStyle;
class ImoObj;

class ImoButton;
class ImoParagraph;
class ImoTextItem;


//------------------------------------------------------------------------------------
/** Base class for any Lomse document.
    Encapsulates a document, providing the basic API for creating and using it, such
    as:
      - An iterator to traverse the document.
      - Support for visitors.
      - Loading from file or string and exporting to file or string.
      - Low level methods to modify the document (no undo/redo capabilities).
      - Methods to set/check a 'document modified' flag (but no logic to
        manage this flag, only reset when the document is created/loaded).
*/
class LOMSE_EXPORT Document : public BlockLevelCreatorApi
                            , public EventNotifier
                            , public Observable
                            , public EnableSharedFromThis<Document>
{
protected:
    LibraryScope&   m_libraryScope;
    ostream&        m_reporter;
    DocumentScope   m_docScope;
    IdAssigner*     m_pIdAssigner;
    ImoDocument*    m_pImoDoc;
    unsigned int    m_flags;
    int             m_modified;

public:
    /// Constructor
    Document(LibraryScope& libraryScope, ostream& reporter=cout);
    virtual ~Document();

    //flag values
    enum {
        k_dirty             = 0x0001,   //dirty: modified since last "clear_dirty()" ==> need to rebuild GModel
    };

    //supported file formats
    enum {
        k_format_ldp = 0,   //Lenguaje De Partituras (LDP, LISP like syntax)
        k_format_lmd,       //LenMus Document (LMD, XML syntax)
        k_format_mxl,       //MusicXML format
        k_format_mnx,       //W3C MNX format
        k_format_unknown,
    };

    //scope access
    inline DocumentScope& get_scope() { return m_docScope; }
    inline LibraryScope& get_library_scope() { return m_libraryScope; }

    /// @name Document creation
    //@{

    int from_file(const string& filename, int format=k_format_ldp);
    int from_string(const string& source, int format=k_format_ldp);
    int from_input(LdpReader& reader);
    void create_empty();
    void create_with_empty_score();
    inline SharedPtr<Document> get_shared_ptr_from_this() { return shared_from_this(); }

    //@}    //Document creation


    //properties
    bool is_editable();

    //dirty
    inline void clear_dirty() { m_flags &= ~k_dirty; }
    inline bool is_dirty() { return (m_flags & k_dirty) != 0; }


    /// @name Access to the internal model
    //@{

    /** Returns a pointer to the root object of the internal model. It is always an
        ImoDocument object. Access to this object is not always necessary as
        %Document provides facade methods for the most common operations. */
    inline ImoDocument* get_im_root() const { return m_pImoDoc; }

    /** Returns version of the internal model document. */
    inline std::string& get_version() { return m_pImoDoc->m_version; }

    /** Returns a pointer to the internal model object with the given ID. */
    ImoObj* get_pointer_to_imo(ImoId id) const;

    /** Returns a pointer to the ImoControl object with the given ID. */
    Control* get_pointer_to_control(ImoId id) const;
    inline string get_language() {
        return (m_pImoDoc != nullptr ? m_pImoDoc->get_language() : "en");
    }
    void removed_from_model(ImoObj* pImo);

    //@}    //Access to the internal model


    /// @name Low level edition API: ImoDocument related
    //@{

    /** Set the language used in this %Document. Parameter 'language' is a language code
    drawn from ISO 639, optionally extended with a country code drawn from ISO 3166, as
    'en-US'. It represents the default language for all texts in the document. */
    inline void set_language(const string& language) { m_pImoDoc->m_language = language; }

        //document intended paper size
    /** Append a ImoPageInfo node with default values. */
    void add_page_info(ImoPageInfo* pPI) { m_pImoDoc->add_page_info(pPI); }

    /** Return the ImoPageInfo node for this %Document. */
    inline ImoPageInfo* get_page_info() { return &(m_pImoDoc->m_pageInfo); }

    /** Return the paper width intended for rendering this %Document. The returned value
        is in logical units (cents of a millimeter). */
    inline LUnits get_paper_width() { return m_pImoDoc->m_pageInfo.get_page_width(); }

    /** Return the paper height intended for rendering this %Document. The returned value
        is in logical units (cents of a millimeter). */
    inline LUnits get_paper_height() { return m_pImoDoc->m_pageInfo.get_page_height(); }

    /** Append a new empty score (no instruments) at the end of the %Document. Returns a
        pointer to the created ImoScore object. */
    ImoScore* add_score(ImoStyle* pStyle=nullptr) { return m_pImoDoc->add_score(pStyle); }

    /** Insert a block level object (a node of class ImoBlockLevelObj, such as ImoScore or
        ImoParagraph) before the pointed node 'pAt'. If 'pAt' is nullptr
        then the block level object will be appended to the end of the %Document. */
    void insert_block_level_obj(ImoBlockLevelObj* pAt, ImoBlockLevelObj* pImoNew) { m_pImoDoc->insert_block_level_obj(pAt, pImoNew); }

    /** Remove from the internal model (and delete) the block level object referenced
        by pointer 'pAt'.  */
    void delete_block_level_obj(ImoBlockLevelObj* pAt) { m_pImoDoc->delete_block_level_obj(pAt); }

    //@}    //Low level edition API: ImoDocument related


    /// @name Low level edition API: objects creation/modification
    //@{

    /** When you modify the content of a %Document it is necessary to update some
        structures associated to music scores, such as the staffobjs collection.
        For this it is mandatory to invoke this method. Alternatively, you can
        invoke ImoScore::end_of_changes(), on the modified scores. */
    void end_of_changes();

    void add_staff_objects(const string& source, ImoMusicData* pMD);
    ImoObj* create_object_from_ldp(const string& source, ostream& reporter);
    ImoObj* create_object_from_ldp(const string& source);
    ImoObj* create_object_from_lmd(const string& source);
    void delete_relation(ImoRelObj* pRO);
    void delete_auxobj(ImoAuxObj* pAO);
    ImoTuplet* add_tuplet(ImoNoteRest* pStartNR, ImoNoteRest* pEndNR,
                          const string& source, ostream& reporter);
    ImoBeam* add_beam(const list<ImoNoteRest*>& notes);
    ImoTie* tie_notes(ImoNote* pStart, ImoNote* pEnd, ostream& reporter);


    /// @name Low level edition API: styles
    //@{

    ImoStyle* get_default_style();
    ImoStyle* create_style(const string& name, const string& parent="Default style");
    ImoStyle* create_private_style(const string& parent="Default style");
    ImoStyle* find_style(const string& name);
    ImoStyles* get_styles() { return m_pImoDoc->get_styles(); }
    void add_style(ImoStyle* pStyle) { m_pImoDoc->add_style(pStyle); }
    ImoStyle* get_style_or_default(const std::string& name) { return m_pImoDoc->get_style_or_default(name); }

    //@}    //Low level edition API: styles



    /// @name Low level edition API: traversing the document
    //@{

    int get_num_content_items() { return m_pImoDoc->get_num_content_items(); }
    ImoContent* get_content() { return m_pImoDoc->get_content(); }
    ImoContentObj* get_content_item(int iItem) { return m_pImoDoc->get_content_item(iItem); }
    ImoContentObj* get_first_content_item() { return m_pImoDoc->get_first_content_item(); }
    ImoContentObj* get_last_content_item() { return m_pImoDoc->get_last_content_item(); }
    void append_content_item(ImoContentObj* pItem) { return m_pImoDoc->append_content_item(pItem); }

    //@}    //Low level edition API: traversing the document



    /// @name Low level edition API: adding first level objects
    //@{

//    ImoTextItem* create_text_item(const string& text, ImoStyle* pStyle=nullptr);
//    ImoButton* create_button(const string& label, const USize& size,
//                             ImoStyle* pStyle=nullptr);
    ImoParagraph* add_paragraph(ImoStyle* pStyle=nullptr) { return m_pImoDoc->add_paragraph(pStyle); }
    ImoContent* add_content_wrapper(ImoStyle* pStyle=nullptr) { return m_pImoDoc->add_content_wrapper(pStyle); }
    ImoList* add_list(int type, ImoStyle* pStyle=nullptr) { return m_pImoDoc->add_list(type, pStyle); }
    ImoMultiColumn* add_multicolumn_wrapper(int numCols, ImoStyle* pStyle=nullptr) { return m_pImoDoc->add_multicolumn_wrapper(numCols, pStyle); }

    //@}    //Low level edition API: adding first level objects

    //mandatory overrides from Observable
    EventNotifier* get_event_notifier() { return this; }
    Observable* get_observable_child(int childType, ImoId childId);

    /** Send an EventDoc of type `k_doc_modified_event` if the document has been
        modified (if it is dirty).
        Notice that to have more control about when to update views, the document
        doesn't automatically notify observers when the document is modified, so it
        is responsibility of the code modifying the %Document to invoke this
        method after finishing the modifications.

        Edition commands and low level edition API methods do not invoke it.
    */
    void notify_if_document_modified();

    //TODO: public to be used by exercises (reconfigure buttons), To be changed to
    //protected as soon as buttons changed to controls
    inline void set_dirty() { m_flags |= k_dirty; }



///@cond INTERNALS
//methods excluded from documented public API. Only for internal use.

    //excluded from low level edition API


    //undo/redo support
    int from_checkpoint(const string& data);
    int replace_object_from_checkpoint_data(ImoId id, const string& data);
    string get_checkpoint_data();
    string get_checkpoint_data_for(ImoId id);

    //modified since last 'save to file' operation
    inline void clear_modified() { m_modified = 0; }
    inline bool is_modified() { return m_modified > 0; }
    inline void set_modified() { ++m_modified; }
    inline void reset_modified() { if (m_modified > 0) --m_modified; }

    //debug
    string dump_ids() const;
    size_t id_assigner_size() const;
    string dump_tree() const;
    string to_string(bool fWithIds = false);

    //inherited from LenMos 3.0. Not yet used
    void add_cursor_info(ImoCursorInfo* UNUSED(pCursor)) {};

///@endcond


protected:
    void initialize();
    Compiler* get_compiler_for_format(int format);
    void fix_malformed_musicxml();

    friend class ImFactory;
    void assign_id(ImoObj* pImo);
    ImoId reserve_id(ImoId id);

    friend class Control;
    void assign_id(Control* pControl);

    friend class LenmusdocAnalyser;
    friend class LenmusdocLmdAnalyser;
    friend class ScorePartwiseMxlAnalyser;
    friend class MnxMnxAnalyser;
    void set_imo_doc(ImoDocument* pImoDoc);

};

typedef SharedPtr<Document>  SpDocument;
typedef WeakPtr<Document>  WpDocument;


}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_H__

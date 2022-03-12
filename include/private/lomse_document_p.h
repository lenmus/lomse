//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_DOCUMENT_P_H__
#define __LOMSE_DOCUMENT_P_H__

    //***********************************************************************
    //*                                                                     *
    //*                         -------------                               *
    //*                         W A R N I N G                               *
    //*                         -------------                               *
    //*                                                                     *
    //*    This file is not part of lomse public API. It is here as a       *
    //*    convenience for lomse internal use, while the public API is      *
    //*    being defined and implemented. This header file may change       *
    //*    from version to version, or even be removed without notice.      *
    //*                                                                     *
    //*                        You've been warned!                          *
    //*                                                                     *
    //***********************************************************************

#include "lomse_injectors.h"
#include "lomse_observable.h"
#include "lomse_ldp_elements.h"
#include "lomse_basic.h"
#include "lomse_internal_model.h"
#include "lomse_events.h"
#include "lomse_reader.h"
#include "lomse_document.h"

#include <sstream>

///@cond INTERNALS
namespace lomse
{
///@endcond


///@cond INTERNALS


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


//---------------------------------------------------------------------------------------
// This enum describes document layout options for DocLayouter, so that it can build a
// @GM that satisfies the intended View needs. The options are set by the
// Interactor by using the values provided by the View.
enum EDocLayoutOptions
{
    k_use_paper_width       = 0x0001,   //Use paper width to determine constrains
    k_use_paper_height      = 0x0002,   //Use paper height to determine constrains
    k_infinite_width        = 0x0004,   //No width constrains
    k_infinite_height       = 0x0008,   //No height constrains
    k_use_viewport_width    = 0x0010,   //Use viewport width to determine constrains
};


//------------------------------------------------------------------------------------
/** The %Document class is a facade object that contains, basically, the @IM, a model
    similar to the DOM in HTML. By accessing and modifying this internal model you
    have full control over the document content.

    The %Document class also encapsulates all the internals, providing the basic API
    for creating, modifying and using a document.

    It is an observable class and, therefore, inherits methods for notifying observers
    (other registered objects) when changes occur to the document. For example, when the
    document is modified.

    In applications using the Lomse library, the main mechanism for creating a
    %Document is by requesting it to the %LomseDoorway object. See LomseDoorway class for
    details on using the library and creating documents.

    A %Document can be created:
    - From a file containing the document source code in any of the supported formats
        (see @ref page-file-formats).
    - From a string, also containing the source code in any of the supported formats.
    - Or, dynamically, by creating an empty %Document and injecting content by issuing
        edition commands or by direct creation and manipulation of the internal data
        structures that form the @IM.

    A %Document can also be created and modified by program. Edition commands operating
    on the %Document and direct manipulation of the @IM give full flexibility for this.

    When using the LDP format, an special LDP tag @<dynamic@> instructs Lomse to require
    from your application the content to be inserted at that point. This is something
    analogous to the HTML @<object@> tag.

*/
class LOMSE_EXPORT Document : public BlockLevelCreatorApi
                            , public EventNotifier
                            , public Observable
                            , public std::enable_shared_from_this<Document>
{
protected:
    LibraryScope&   m_libraryScope;
    ostream&        m_reporter;
    DocumentScope   m_docScope;
    IdAssigner*     m_pIdAssigner;
    ImoDocument*    m_pImoDoc;
    unsigned int    m_flags;
    int             m_modified;
    long            m_imRef;            //to validate the model

public:
    /// Constructor
    Document(LibraryScope& libraryScope, ostream& reporter=cout);
    ~Document() override;

    ///Values for flags
    enum EDocumentFlags {
        k_dirty             = 0x0001,   ///< dirty: modified since last "clear_dirty()" ==> need to rebuild GModel
    };

    ///Supported file formats
    enum EFileFormat {
        k_format_ldp = 0,   ///< Lenguaje De Partituras (LDP, LISP like syntax)
        k_format_lmd,       ///< LenMus %Document (LMD, XML syntax)
        k_format_mxl,       ///< MusicXML format
        k_format_mxl_compressed, ///< Compressed MusicXML format
        k_format_mnx,       ///< W3C MNX format
        k_format_unknown,
    };


    /// @name Document creation
    //@{

    /** Add content to an uninitialized %Document (a %Document created by just invoking
        the %Document constructor) by parsing the content of the specified file.
        @param filename   A string with the full file name (path and extension included).
        @param format   The expected format of the file content. Must be a value from
            enum EFileFormat.

        <b>Remarks</b>
        - Applications using the Lomse library do not have, normally, the need to use
            this method as all %Document creation is managed by LomseDoorway object.
        -  This method will do not attempt to discover the file format. It just assumes
            the format passed as parameter. Therefore, if the format is not correct,
            this method will report parsing errors and will return a valid empty
            document with a valid empty internal model.
        - Errors detected while parsing the file are reported to the reporter object
            defined in %Document constructor. By default, to `cout` stream.
    */
    int from_file(const std::string& filename, int format=k_format_ldp);

    /** Add content to an uninitialized %Document (a %Document created by just invoking
        the %Document constructor) by parsing the passed string.
        @param source   A string with the content to be parsed.
        @param format   The expected format of the source string. Must be a value from
            enum EFileFormat.

        <b>Remarks</b>
        - Applications using the Lomse library do not have, normally, the need to use
            this method as all %Document creation is managed by LomseDoorway object.
        -  This method will do not attempt to discover the string format. It just assumes
            the format passed as parameter. Therefore, if the format is not correct,
            this method will report parsing errors and will return a valid empty
            document with a valid empty internal model.
        - Errors detected while parsing the source string are reported to the reporter
            object defined in %Document constructor. By default, to `cout` stream.
    */
    int from_string(const std::string& source, int format=k_format_ldp);

    /** Add content to an uninitialized %Document (a %Document created by just invoking
        the %Document constructor) by parsing data from LdpReader object.
        <b>Remarks</b>
        - Applications using the Lomse library do not have, normally, the need to use
            this method as all %Document creation is managed by LomseDoorway object.
        - This method assumes LDP format.
        - Errors detected while parsing the source string are reported to the reporter
            object defined in %Document constructor. By default, to `cout` stream.
    */
    int from_input(LdpReader& reader);

    /** Initialize an uninitialized %Document (a %Document created by just invoking
        the %Document constructor) so that it will be a valid empty %Document with a
        valid empty internal model.

        <b>Remarks</b>
        - Applications using the Lomse library do not have, normally, the need to use
            this method as all %Document creation is managed by LomseDoorway object.
    */
    void create_empty();

    /** Initialize an uninitialized %Document (a %Document created by just invoking
        the %Document constructor) so that it will be a valid %Document containing
        a valid empty score (an score without instruments).

        <b>Remarks</b>
        - Applications using the Lomse library do not have, normally, the need to use
            this method as all %Document creation is managed by LomseDoorway object.
    */
    void create_with_empty_score();

    //@}    //Document creation


    /// @name Access to the internal model
    //@{

    /** Returns a pointer to the root object of the internal model. It is always an
        ImoDocument object. Access to this object is not always necessary as
        %Document provides facade methods for the most common operations. */
    inline ImoDocument* get_im_root() const { return m_pImoDoc; }

    /** Returns an ADocument object for interacting with the internal model.  */
    ADocument get_document_api();


    //LMD version used in the source or "0.0" in other cases
    inline std::string& get_version() { return m_pImoDoc->m_version; }

    // ptr to ImoDocument or nullptr if no object found with the given ID.
    ImoObj* get_pointer_to_imo(ImoId id) const;
    ImoObj* get_pointer_to_imo(const std::string& xmlId) const;

    //ptr. to ImoControl or value `k_no_imoid` (-1) if no ImoControl with the given ID
    Control* get_pointer_to_control(ImoId id) const;

    /** Returns the default language for all texts in the document. The returned value
        is always a language code drawn from ISO 639, optionally extended with a country
        code drawn from ISO 3166, as 'en-US'.

        For %Document objects created from sources in LMD format this method will return
        the language specified in the @<lenmusdoc@> tag.

        For %Document objects created from sources in other formats it will return
        'en' (English).
    */
    inline std::string get_language() {
        return (m_pImoDoc != nullptr ? m_pImoDoc->get_language() : "en");
    }

    inline float get_page_content_scale() {
        return (m_pImoDoc != nullptr ? m_pImoDoc->get_page_content_scale() : 1.0f);
    }

    //@}    //Access to the internal model


    /// @name Low level edition API: ImoDocument global settings
    //@{

    /** Set the language used in this %Document. Parameter 'language' is a language code
    drawn from ISO 639, optionally extended with a country code drawn from ISO 3166, as
    'en-US'. It represents the default language for all texts in the document. */
    inline void set_language(const std::string& language) { m_pImoDoc->m_language = language; }

    inline void set_page_content_scale(float scale) {
        if (m_pImoDoc)
            m_pImoDoc->set_page_content_scale(scale);
    }

    void end_of_changes();

    /** Return the ImoPageInfo node for this %Document. ImoPageInfo contains the
        document intended paper size. Example:
        @code
        //set page size to DIN A3 (297 x 420 mm)
        Document* pDoc = m_pPresenter->get_document_raw_ptr();
        ImoPageInfo* pPageInfo = pDoc->get_page_info();
        pPageInfo->set_page_height(42000.0f);   //logical units, cents of one millimeter
        pPageInfo->set_page_width(29700.0f);
        @endcode
    */
    inline ImoPageInfo* get_page_info() { return &(m_pImoDoc->m_pageInfo); }

    inline LUnits get_paper_width() { return m_pImoDoc->m_pageInfo.get_page_width(); }
    inline LUnits get_paper_height() { return m_pImoDoc->m_pageInfo.get_page_height(); }

    /** Append a ImoPageInfo node with default values.

        <b>Remarks</b>
        - There should be only one ImoPageInfo child in ImoDocument. This method
            should not be used in applications using Lomse, unless they know very well
            what they are doing.
    */
    void add_page_info(ImoPageInfo* pPI) { m_pImoDoc->add_page_info(pPI); }

    //@}    //Low level edition API: ImoDocument related



    /// @name Low level edition API: adding first level objects
    //@{

//    ImoTextItem* create_text_item(const std::string& text, ImoStyle* pStyle=nullptr);
//    ImoButton* create_button(const std::string& label, const USize& size,
//                             ImoStyle* pStyle=nullptr);

    /** Append a new empty score (no instruments) at the end of the %Document. Returns a
        pointer to the created ImoScore object. */
    ImoScore* add_score(ImoStyle* pStyle=nullptr) { return m_pImoDoc->add_score(pStyle); }

    /** Append a new empty paragraph at the end of the %Document. Returns a
        pointer to the created ImoParagraph object. */
    ImoParagraph* add_paragraph(ImoStyle* pStyle=nullptr) { return m_pImoDoc->add_paragraph(pStyle); }

    /** Append a new empty list at the end of the %Document. Returns a
        pointer to the created ImoList object. */
    ImoList* add_list(int type, ImoStyle* pStyle=nullptr) { return m_pImoDoc->add_list(type, pStyle); }

    /** Append a block level object (a node of class ImoBlockLevelObj, such as ImoScore or
        ImoParagraph) to the document. */
    void append_content_item(ImoBlockLevelObj* pItem) { return m_pImoDoc->append_content_item(pItem); }

    /** Insert a block level object (a node of class ImoBlockLevelObj, such as ImoScore or
        ImoParagraph) before the pointed node 'pAt'. If 'pAt' is nullptr
        then the block level object will be appended to the end of the %Document. */
    void insert_block_level_obj(ImoBlockLevelObj* pAt, ImoBlockLevelObj* pImoNew) { m_pImoDoc->insert_block_level_obj(pAt, pImoNew); }

    /** Remove from the internal model (and delete) the block level object referenced
        by pointer 'pAt'.  */
    void delete_block_level_obj(ImoBlockLevelObj* pAt) { m_pImoDoc->delete_block_level_obj(pAt); }

    //@}    //Low level edition API: adding first level objects


    /// @name Low level edition API: objects creation/modification
    //@{

    /** Append content to the ImoMusicData object passed as parameter. The content to
        add is created by parsing the source code passed in the 'source' string. This
        std::string must be in LDP format.

        Example. Append some notes to the first instrument in the score:
        @code
        ImoScore* pScore = ...

        //get the ImoMusicData child associated to the first instrument
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        //append two quarter notes and a barline
        pInstr->add_staff_objects("(n c4 q)(n e4 q)(barline simple)", pMD);
        @endcode
    */
    void add_staff_objects(const std::string& source, ImoMusicData* pMD);

    /** Removes a relation between staff objects.

        Example 1. Remove the beams in a beamed group, leaving the notes as single
        notes (not beamed):
        @code
        ImoNote* pNote = ...    //one of the notes in the beamed group
        ImoBeam* pBeam = pNote->get_beam();
        pDoc->delete_relation(pBeam);
        @endcode

        Example 2. Remove the tie between two notes:
        @code
        ImoNote* pNote = ...    //the first note (the note in which the tie starts)
        ImoTie* pTie = pNote->get_tie_next();
        pDoc->delete_relation(pTie);
        @endcode
    */
    void delete_relation(ImoRelObj* pRO);

    /** Removes an ImoAuxObj attached to an object.

        Example. Remove a fermata attached to a note.
        @code
        ImoNote* pNote = ...    //the note with the fermata
        ImoAttachments* pAuxObjs = pNote->get_attachments();
        ImoAuxObj* pAO = static_cast<ImoAuxObj*>( pAuxObjs->get_child_of_type(k_imo_fermata) );
        pDoc->delete_auxobj(pAO);
        @endcode
    */
    void delete_auxobj(ImoAuxObj* pAO);

    /** Join several notes to form a tuplet.
        @param pStartNR     The first ImoNoteRest to include in the tuplet.
        @param pEndNR       The last ImoNoteRest to include in the tuplet.
        @param source       An string, in LDP format, describing the tuplet to create.
        @param reporter     The ostream for receiving any error message that could occur.

        Example. Join three notes to form a triplet:
        @code
        std::stringstream msg;
        ImoNoteRest* pStart = ...   //the first note in the triplet
        ImoNoteRest* pEnd = ...     //the last note in the triplet
        std::string source = "";
        ImoTuplet* pTuplet = pDoc->add_tuplet(pStart, pEnd, "(t + 2 3)", msg);
        @endcode
    */
    ImoTuplet* add_tuplet(ImoNoteRest* pStartNR, ImoNoteRest* pEndNR,
                          const std::string& source, ostream& reporter);


    /** Join several notes/rests to form a beamed group. All notes/rests must be eighth
        notes or shorter.
        @param notes     A list of pointers to the notes/rest to join.

        Example. Join some notes to form a beamed group:
        @code
        //get the notes/rests to join
        list<ImoNoteRest*> notes;
        while (...)
            notes.push_back( ... );

        //create the beamed group
        ImoBeam* pBeam = pDoc->add_beam(notes);
        @endcode
    */
    ImoBeam* add_beam(const std::list<ImoNoteRest*>& notes);

    /** Tie two notes
        @param pStart       The first note
        @param pEnd         The last note
        @param reporter     Not used: the ostream for receiving any error message that
                            could occur.

        <b>Remarks</b>
        - The notes must be on the same score, instrument and staff, and must have
            the same pitch. This method does not perform any checks. Therefore,
            parameter 'reporter' is useless.

        Example. Tie two notes:
        @code
        std::stringstream msg;
        ImoNote* pStart = ...   //the first note
        ImoNote* pEnd = ...     //the last note
        ImoTie* pTie = pDoc->add_tie(pStart, pEnd, msg);
        @endcode
    */
    ImoTie* tie_notes(ImoNote* pStart, ImoNote* pEnd, ostream& reporter);



    // traversing the document
    ImoContent* get_content() { return m_pImoDoc->get_content(); }
    int get_num_content_items() { return m_pImoDoc->get_num_content_items(); }
    ImoBlockLevelObj* get_content_item(int iItem) {
        return dynamic_cast<ImoBlockLevelObj*>( m_pImoDoc->get_content_item(iItem) );
    }
    ImoBlockLevelObj* get_first_content_item() {
        return dynamic_cast<ImoBlockLevelObj*>( m_pImoDoc->get_first_content_item() );
    }
    ImoBlockLevelObj* get_last_content_item() {
        return dynamic_cast<ImoBlockLevelObj*>( m_pImoDoc->get_last_content_item() );
    }


    //styles

    /** Return the default style defined in this %Document. Must always exist. */
    ImoStyle* get_default_style();

    /** Define a new style. The created style will be public so that other styles can be
        derived from it.
        @param name     The name to assign to this new style.
        @param parent   The name of an existing style from which the new style will
            inherit all its values. By default, from "Default style"   */
    ImoStyle* create_style(const std::string& name, const std::string& parent="Default style");

    /** Define a new style. The created style will be private, that is, it does not have
        name and cannot be used to derive other styles from it.
        @param parent   The name of an existing style from which the new style will
            inherit all its values. By default, from "Default style"   */
    ImoStyle* create_private_style(const std::string& parent="Default style");

    /** Return the style whose name is passed as parameter. If the style name is not
        defined returns nullptr.
    */
    ImoStyle* find_style(const std::string& name);

    /** Return the style whose name is passed as parameter. If the style name is not
        defined returns the default style.
    */
    ImoStyle* get_style_or_default(const std::string& name) { return m_pImoDoc->get_style_or_default(name); }

    /** Return the ImoStyles object associated to this %Document. The returned object
        contains all the styles defined in this %Document.
    */
    ImoStyles* get_styles() { return m_pImoDoc->get_styles(); }

    /** Add the style passed as parameter to the sets of styles. */
    void add_style(ImoStyle* pStyle) { m_pImoDoc->add_style(pStyle); }



    /// @name Miscellaneous methods
    //@{

    /** Returns the scope object associated to this %Document.  */
    inline DocumentScope& get_scope() { return m_docScope; }

    /** Returns the scope object associated to the library.  */
    inline LibraryScope& get_library_scope() { return m_libraryScope; }

    /** Returns a shared pointer for this %Document. */
    inline std::shared_ptr<Document> get_shared_ptr_from_this() { return shared_from_this(); }

    //properties
    /** Returns @true if the %Document is editable.

        <b>Remarks</b>
        - This method is intended to add support, in future, for <i>write protected</i>
        documents. But for now all documents are editable and this method always
        returns @true.
    */
    bool is_editable();

    /** Return @true if the document has been modified (if it is dirty).
        This flag is automatically cleared when the graphic model for the
        document is created and when an EventDoc of type `k_doc_modified_event`
        is issued.
    */
    inline bool is_dirty() { return (m_flags & k_dirty) != 0; }

    /** Send an EventDoc of type `k_doc_modified_event` if the document has been
        modified (if it is dirty).
        Notice that to have more control about when to update views, the document
        doesn't automatically notify observers when the document is modified, so it
        is responsibility of the code modifying the %Document to invoke this
        method after finishing the modifications.

        Edition commands and low level edition API methods do not invoke it.
    */
    void notify_if_document_modified();

    //@}    //Miscellaneous methods




//methods excluded from documented public API. Only for internal use.

    //support for IM API methods
    bool is_valid_model(long imRef);
    long get_model_ref();

    //excluded from low level edition API
    ImoObj* create_object_from_ldp(const std::string& source, ostream& reporter);
    ImoObj* create_object_from_ldp(const std::string& source);
    ImoObj* create_object_from_lmd(const std::string& source);

    /** Add to the document a child object of type ImoContent. */
    ImoContent* add_content_wrapper(ImoStyle* pStyle=nullptr) { return m_pImoDoc->add_content_wrapper(pStyle); }

    /** Add to the document a child object of type ImoMultiColumn. */
    ImoMultiColumn* add_multicolumn_wrapper(int numCols, ImoStyle* pStyle=nullptr) { return m_pImoDoc->add_multicolumn_wrapper(numCols, pStyle); }

        //dirty

    //TODO: public to be used by exercises (reconfigure buttons), To be changed to
    //protected as soon as buttons changed to controls
    inline void set_dirty() { m_flags |= k_dirty; }

    inline void clear_dirty() { m_flags &= ~k_dirty; }

        //events
    /** Mandatory override from Observable. Returns the EventNotifier associated to
        this Observable object. For %Document class the EventNotifier is itself, so
        this method returns the <i>this</i> pointer.
    */
    EventNotifier* get_event_notifier() override { return this; }

    Observable* get_observable_child(int childType, ImoId childId) override;


    //internal model and ID management
    void on_removed_from_model(ImoObj* pImo);

    //undo/redo support
    int from_checkpoint(const std::string& data);
    int replace_object_from_checkpoint_data(ImoId id, const std::string& data);
    std::string get_checkpoint_data();
    std::string get_checkpoint_data_for(ImoId id);

    //modified since last 'save to file' operation
    inline void clear_modified() { m_modified = 0; }
    inline bool is_modified() { return m_modified > 0; }
    inline void set_modified() { ++m_modified; }
    inline void reset_modified() { if (m_modified > 0) --m_modified; }

    //debug
    std::string dump_ids() const;
    size_t id_assigner_size() const;
    std::string dump_tree() const;
    std::string to_string(bool fWithIds = false);

    //inherited from LenMos 3.0. Not yet used
    void add_cursor_info(ImoCursorInfo* UNUSED(pCursor)) {}



protected:
    void initialize();
    Compiler* get_compiler_for_format(int format);
    void fix_malformed_musicxml();

    friend class ImFactory;
    friend class InstrumentAnalyser;
    void assign_id(ImoObj* pImo);
    ImoId reserve_id(ImoId id);

    friend class ImoObj;
    std::string get_xml_id_for(ImoId id);
    void set_xml_id_for(ImoId id, const std::string& value);

    friend class Control;
    void assign_id(Control* pControl);


    //There is a design bug: ImoControl constructor needs to access ImoDocument for
    //setting the language. But when the control is created (in LdpAnalyser or
    //LmdAnalyser, the Document does not have yet the pointer to ImoDocument. To bypass
    //this design error, this method is provided.
    friend class LenmusdocAnalyser;
    friend class LenmusdocLmdAnalyser;
    void set_imo_doc(ImoDocument* pImoDoc);

};

typedef std::shared_ptr<Document>  SpDocument;
typedef std::weak_ptr<Document>  WpDocument;

///@endcond


}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_P_H__

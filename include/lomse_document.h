//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

namespace lomse
{

//forward declarations
class DocCommand;
class DocCommandExecuter;
class Compiler;
class IdAssigner;
class Interactor;
class InternalModel;
class ImoDocument;
class ImoMusicData;
class ImoScore;
class ImoStyle;
class ImoObj;

class ImoButton;
class ImoParagraph;
class ImoTextItem;


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
    InternalModel*  m_pIModel;
    ImoDocument*    m_pImoDoc;
    unsigned int    m_flags;
    int             m_modified;

protected:
    friend class LenmusdocAnalyser;
    friend class LenmusdocLmdAnalyser;
    friend class ScorePartwiseMxlAnalyser;
    void set_imo_doc(ImoDocument* pImoDoc);

public:
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
        k_format_mxl,       //MusicXML
        k_format_unknown,
    };

    //scope access
    inline DocumentScope& get_scope() { return m_docScope; }
    inline LibraryScope& get_library_scope() { return m_libraryScope; }

    //creation
    int from_file(const string& filename, int format=k_format_ldp);
    int from_string(const string& source, int format=k_format_ldp);
    int from_input(LdpReader& reader);
    int from_checkpoint(const string& data);
    int replace_object_from_checkpoint_data(ImoId id, const string& data);
    void create_empty();
    void create_with_empty_score();
    inline SharedPtr<Document> get_shared_ptr_from_this() { return shared_from_this(); }

    //properties
    bool is_editable();

    //dirty
    inline void clear_dirty() { m_flags &= ~k_dirty; }
    inline bool is_dirty() { return (m_flags & k_dirty) != 0; }

    //modified since last 'save to file' operation
    inline void clear_modified() { m_modified = 0; }
    inline bool is_modified() { return m_modified > 0; }
    inline void set_modified() { ++m_modified; }
    inline void reset_modified() { if (m_modified > 0) --m_modified; }

    //internal model
    inline ImoDocument* get_imodoc() const { return m_pImoDoc; }
    inline InternalModel* get_im_model() const { return m_pIModel; }
    ImoObj* get_pointer_to_imo(ImoId id) const;
    Control* get_pointer_to_control(ImoId id) const;
    string to_string(bool fWithIds = false);
    string get_checkpoint_data();
    string get_checkpoint_data_for(ImoId id);
    inline string get_language() {
        return (m_pImoDoc != NULL ? m_pImoDoc->get_language() : "en");
    }
    void removed_from_model(ImoObj* pImo);


    //API: objects creation/modification
    void end_of_changes();

    ImoObj* create_object_from_ldp(const string& source, ostream& reporter);
    ImoObj* create_object_from_ldp(const string& source);
    ImoObj* create_object_from_lmd(const string& source);
    void add_staff_objects(const string& source, ImoMusicData* pMD);
    void delete_relation(ImoRelObj* pRO);
    void delete_auxobj(ImoAuxObj* pAO);
    ImoTuplet* add_tuplet(ImoNoteRest* pStartNR, ImoNoteRest* pEndNR,
                          const string& source, ostream& reporter);
    ImoBeam* add_beam(const list<ImoNoteRest*>& notes);
    ImoTie* tie_notes(ImoNote* pStart, ImoNote* pEnd, ostream& reporter);


    //API: styles
    ImoStyle* get_default_style();
    ImoStyle* create_style(const string& name, const string& parent="Default style");
    ImoStyle* create_private_style(const string& parent="Default style");
    ImoStyle* find_style(const string& name);

    ////API: traversing the document
    //ImoContent* get_content();
    //void append_content_item(ImoContentObj* pItem);

    //API: adding first level onjects
    //These violate the Open/Close principle as it would require to modify the API
    //when a new first level item is created. Nevertheless it is an upwards
    //compatible change.
//    ImoParagraph* add_paragraph(ImoStyle* pStyle=NULL);
//    ImoTextItem* create_text_item(const string& text, ImoStyle* pStyle=NULL);
//    ImoButton* create_button(const string& label, const USize& size,
//                             ImoStyle* pStyle=NULL);

    //mandatory overrides from Observable
    EventNotifier* get_event_notifier() { return this; }
    Observable* get_observable_child(int childType, ImoId childId);

    /** Send doc-modified events
        To have more control about when to update views, the document doesn't
        automatically notify observers when the document is updated.
        Views observing the document will be notified about modifications only
        when the following method is invoked. Commands (atomic, DocCommands and
        UserCommands)don't invoke it. Invoking this method is a responsibility
        of the Interactor (or the user application if Interactor is not used)
        whenever a command or an event altering the document is processed.
    */
    void notify_if_document_modified();

    //TODO: public to be used by exercises (reconfigure buttons), To be changed to
    //protected as soon as buttons changed to controls
    inline void set_dirty() { m_flags |= k_dirty; }

    //debug
    string dump_ids() const;
    size_t id_assigner_size() const;
    string dump_tree() const;

protected:
    void initialize();
    Compiler* get_compiler_for_format(int format);
    void fix_malformed_musicxml();

    friend class ImFactory;
    void assign_id(ImoObj* pImo);
    ImoId reserve_id(ImoId id);

    friend class Control;
    void assign_id(Control* pControl);

};

typedef SharedPtr<Document>  SpDocument;
typedef WeakPtr<Document>  WpDocument;


}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_H__

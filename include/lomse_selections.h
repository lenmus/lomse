//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SELECTIONS_H__
#define __LOMSE_SELECTIONS_H__

#include "lomse_basic.h"
#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class ColStaffObjs;
class GraphicModel;
class GmoObj;
class ImoObj;
class ImoNote;
class ImoNoteRest;
class ImoStaffObj;
class SelectionSet;
class Document;
typedef std::shared_ptr<Document>     SpDocument;
typedef std::weak_ptr<Document>       WpDocument;


//---------------------------------------------------------------------------------------
// SelectionValidator
//  Helper class to validate a selection for a given operation
//  You can change behaviour by deriving from it and overriding desired methods
class SelectionValidator
{
protected:

public:
    SelectionValidator() {}
    virtual ~SelectionValidator() {}

    //validations
    virtual bool is_valid_to_add_tie(SelectionSet* pSelection,
                                   ImoNote** ppStartNote = nullptr,
                                   ImoNote** ppEndNote = nullptr);
    virtual bool is_valid_to_remove_tie(SelectionSet* pSelection);
    virtual bool is_valid_to_add_tuplet(SelectionSet* pSelection);
    virtual bool is_valid_to_remove_tuplet(SelectionSet* pSelection);
    virtual bool is_valid_for_join_beam(SelectionSet* pSelection);
    virtual bool is_valid_for_toggle_stem(SelectionSet* pSelection);

};

//---------------------------------------------------------------------------------------
// SelectionState: class for saving the state of a SelectionSet
class SelectionState
{
public:
    list<ImoId> m_ids;

public:
    SelectionState(const list<ImoId>& ids)
        : m_ids(ids)
    {
    }

};

//---------------------------------------------------------------------------------------
typedef list<ImoObj*>::iterator     SelectionSetIterator;

//---------------------------------------------------------------------------------------
class SelectionSet
{
protected:
    list<GmoObj*> m_gmos;
    list<ImoObj*> m_imos;
    list<ImoId> m_ids;
    SelectionValidator* m_pValidator;
    ColStaffObjs* m_pMasterCollection;
    ColStaffObjs* m_pCollection;
    bool m_fValid;
    GraphicModel* m_pGModel;
    Document* m_pDoc;

public:
    SelectionSet(Document* pDoc);
    virtual ~SelectionSet();

    ///change default validation rules
    void set_validator(SelectionValidator* pValidator);

    //operations
    void add(GmoObj* pGmo);
    void add(ImoId id, GraphicModel* pGM=nullptr);
    void remove(ImoId id);
    bool contains(ImoObj* pImo);
    bool contains(ImoId id);
    void clear();
    void graphic_model_changed(GraphicModel* pGModel);
    void restore_state(const SelectionState& state);

    ///true if no objects selected
    inline bool empty() { return m_ids.size() == 0; }
    inline int num_selected() { return static_cast<int>(m_ids.size()); }

    ///true if GmoObj and ImoObj are valid
    inline bool is_valid() { return m_fValid; }

    ///filter selection, only returning objects of requested types
    list<ImoId> filter(int type);
    list<ImoId> filter_notes_rests();
    list<ImoObj*> filter_deletable();
    void get_start_end_note_rests(ImoNoteRest** ppStart, ImoNoteRest** ppEndNote);

    ///validations on the selection. Use set_validator() to change validation rules
    bool is_valid_to_add_tie(ImoNote** ppStartNote = nullptr, ImoNote** ppEndNote = nullptr);
    bool is_valid_to_remove_tie();
    bool is_valid_to_add_tuplet();
    bool is_valid_to_remove_tuplet();
    bool is_valid_for_join_beam();
    bool is_valid_for_toggle_stem();

    //iterator and iterator related
	inline SelectionSetIterator begin() { ensure_set_is_valid(); return m_imos.begin(); }
	inline SelectionSetIterator end() { ensure_set_is_valid(); return m_imos.end(); }
    inline ImoObj* back() { ensure_set_is_valid(); return m_imos.back(); }
    inline ImoObj* front() { ensure_set_is_valid(); return m_imos.front(); }

    //accessors
    inline ColStaffObjs* get_staffobjs_collection() { ensure_set_is_valid(); return m_pCollection; }
    inline list<ImoObj*>& get_all_objects() { ensure_set_is_valid(); return m_imos; }
    inline list<GmoObj*>& get_all_gmo_objects() { ensure_set_is_valid(); return m_gmos; }
    inline SelectionState get_state() { return SelectionState(m_ids); }
    string dump_selection();

protected:
    void add_staffobj_to_collection(ImoStaffObj* pSO);
    void ensure_set_is_valid();             //virtual for unit tests
    void add_gmo(GmoObj* pGmo, bool fSaveImoId);

};


}   //namespace lomse

#endif      //__LOMSE_SELECTIONS_H__

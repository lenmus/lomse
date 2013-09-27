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
                                   ImoNote** ppStartNote = NULL,
                                   ImoNote** ppEndNote = NULL);
    virtual bool is_valid_to_remove_tie(SelectionSet* pSelection);
    virtual bool is_valid_to_add_tuplet(SelectionSet* pSelection);
    virtual bool is_valid_to_remove_tuplet(SelectionSet* pSelection);
    virtual bool is_valid_for_join_beam(SelectionSet* pSelection);
    virtual bool is_valid_for_toggle_stem(SelectionSet* pSelection);

};


//---------------------------------------------------------------------------------------
typedef list<ImoObj*>::iterator     SelectionSetIterator;

//---------------------------------------------------------------------------------------
class SelectionSet
{
protected:
    list<GmoObj*> m_gmos;
    list<ImoObj*> m_imos;
    SelectionValidator* m_pValidator;
    ColStaffObjs* m_pMasterCollection;
    ColStaffObjs* m_pCollection;

public:
    SelectionSet();
    ~SelectionSet();

    ///change default validation rules
    void set_validator(SelectionValidator* pValidator);

    //operations
    void add(GmoObj* pGmo, unsigned flags=0);
    bool contains(GmoObj* pGmo);
    void clear();

    ///true if no objects selected
    inline bool empty() { return m_imos.size() == 0; }

    ///filter selection, only returning objects of requested types
    list<ImoId> filter(int type);
    list<ImoId> filter_notes_rests();
    list<ImoObj*> filter_deletable();
    void get_start_end_note_rests(ImoNoteRest** ppStart, ImoNoteRest** ppEndNote);

    ///validations on the selection. Use set_validator() to change validation rules
    bool is_valid_to_add_tie(ImoNote** ppStartNote = NULL, ImoNote** ppEndNote = NULL);
    bool is_valid_to_remove_tie();
    bool is_valid_to_add_tuplet();
    bool is_valid_to_remove_tuplet();
    bool is_valid_for_join_beam();
    bool is_valid_for_toggle_stem();

    //iterator and iterator related
	inline SelectionSetIterator begin() { return m_imos.begin(); }
	inline SelectionSetIterator end() { return m_imos.end(); }
    inline ImoObj* back() { return m_imos.back(); }
    inline ImoObj* front() { return m_imos.front(); }

    //accessors
    inline ColStaffObjs* get_staffobjs_collection() { return m_pCollection; }
    inline list<ImoObj*>& get_all_objects() { return m_imos; }
    inline list<GmoObj*>& get_all_gmo_objects() { return m_gmos; }

    //only for unit tests
    void debug_add(ImoObj* pImo);

protected:
    void add_staffobj_to_collection(ImoStaffObj* pSO);

};


}   //namespace lomse

#endif      //__LOMSE_SELECTIONS_H__

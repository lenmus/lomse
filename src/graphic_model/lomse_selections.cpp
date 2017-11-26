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

#include "lomse_selections.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_table.h"
#include "lomse_document.h"
#include "lomse_graphical_model.h"

#include <sstream>
using namespace std;


namespace lomse
{

//=======================================================================================
// SelectionSet implementation
//=======================================================================================
SelectionSet::SelectionSet(Document* pDoc)
    : m_pValidator( LOMSE_NEW SelectionValidator() )
    , m_pMasterCollection(nullptr)
    , m_pCollection(nullptr)
    , m_fValid(true)
    , m_pGModel(nullptr)
    , m_pDoc(pDoc)
{
}

//---------------------------------------------------------------------------------------
SelectionSet::~SelectionSet()
{
    m_gmos.clear();
    m_imos.clear();
    m_ids.clear();
    delete m_pValidator;
    delete m_pCollection;
}

//---------------------------------------------------------------------------------------
void SelectionSet::set_validator(SelectionValidator* pValidator)
{
    delete m_pValidator;
    m_pValidator = pValidator;
}

//---------------------------------------------------------------------------------------
void SelectionSet::graphic_model_changed(GraphicModel* pGModel)
{
    m_pGModel = pGModel;
    m_fValid = false;
}

//---------------------------------------------------------------------------------------
void SelectionSet::ensure_set_is_valid()
{
    if (!m_fValid)
    {
        if (m_pDoc)
        {
            m_gmos.clear();
            m_imos.clear();
            delete m_pCollection;
            m_pCollection = nullptr;
            m_pMasterCollection = nullptr;
            m_fValid = true;

            list<ImoId>::iterator it = m_ids.begin();
            while (it != m_ids.end())
            {
                ImoObj* pImo = m_pDoc->get_pointer_to_imo(*it);
                if (pImo)
                {
                    m_imos.push_back(pImo);
                    if (pImo->is_staffobj())
                        add_staffobj_to_collection( static_cast<ImoStaffObj*>(pImo) );

                    if (m_pGModel)      //In some unit tests, there is no GModel
                    {
                        GmoObj* pGmo = m_pGModel->get_main_shape_for_imo( pImo->get_id() );
                        //TODO: When adding a GmoObj, its Shape Id should be saved so that
                        //      following method can be used:
                        // GmoShape* get_shape_for_imo(ImoId imoId, ShapeId shapeId);

                        m_gmos.push_back(pGmo);
                    }
                    ++it;
                }
                else
                    it = m_ids.erase(it);
            }
        }
        else
            clear();
    }
}

//---------------------------------------------------------------------------------------
void SelectionSet::add(GmoObj* pGmo)
{
    add_gmo(pGmo, true /*save ImoId*/);
}

//---------------------------------------------------------------------------------------
void SelectionSet::add(ImoId id, GraphicModel* pGM)
{
    list<ImoId>::iterator it = std::find(m_ids.begin(), m_ids.end(), id);
    if (it == m_ids.end())
    {
        if (pGM != nullptr && pGM == m_pGModel)
        {
            add( m_pGModel->get_main_shape_for_imo(id) );
        }
        else
        {
            m_fValid = false;
            if (pGM != nullptr)        //in some UnitTest there is no GM
                m_pGModel = pGM;
            m_ids.push_back(id);
        }
    }
}

//---------------------------------------------------------------------------------------
void SelectionSet::remove(ImoId id)
{
    m_fValid = false;
    m_ids.remove(id);
}

//---------------------------------------------------------------------------------------
void SelectionSet::add_gmo(GmoObj* pGmo, bool fSaveImoId)
{
    ensure_set_is_valid();

    m_gmos.push_back(pGmo);

    ImoObj* pImo = pGmo->get_creator_imo();
    if (pImo)
    {
        m_imos.push_back(pImo);
        if (fSaveImoId)
            m_ids.push_back( pImo->get_id() );

        if (pImo->is_staffobj())
            add_staffobj_to_collection( static_cast<ImoStaffObj*>(pImo) );
    }
}

//---------------------------------------------------------------------------------------
void SelectionSet::add_staffobj_to_collection(ImoStaffObj* pSO)
{
    //add it to staffobjs internal collection
    if (!m_pMasterCollection)
    {
        ImoScore* pScore = pSO->get_score();
        m_pMasterCollection = pScore->get_staffobjs_table();
    }

    //find entry in master collection
    ColStaffObjsIterator it = m_pMasterCollection->find(pSO);
    if (it == m_pMasterCollection->end())
    {
        //(impossible) error! not found!
        LOMSE_LOG_ERROR("Impossible error: staffobj not found when added to SelectionSet");
        return;
    }
    ColStaffObjsEntry* pEntry = *it;

    //add to local collection
    if (!m_pCollection)
        m_pCollection = LOMSE_NEW ColStaffObjs();

    m_pCollection->add_entry(pEntry->measure(),
                             pEntry->num_instrument(),
                             pEntry->line(),
                             pEntry->staff(),
                             pEntry->imo_object() );
}

//---------------------------------------------------------------------------------------
bool SelectionSet::contains(ImoObj* pImo)
{
    return contains( pImo->get_id() );
}

//---------------------------------------------------------------------------------------
bool SelectionSet::contains(ImoId id)
{
    list<ImoId>::iterator it = std::find(m_ids.begin(), m_ids.end(), id);
    return it != m_ids.end();
}

//---------------------------------------------------------------------------------------
void SelectionSet::clear()
{
    m_gmos.clear();
    m_imos.clear();
    m_ids.clear();
    delete m_pCollection;
    m_pCollection = nullptr;
    m_pMasterCollection = nullptr;
    m_fValid = true;
}

//---------------------------------------------------------------------------------------
void SelectionSet::restore_state(const SelectionState& state)
{
    clear();
    m_ids = state.m_ids;
    m_fValid = false;
}

//---------------------------------------------------------------------------------------
list<ImoId> SelectionSet::filter(int type)
{
    ensure_set_is_valid();

    list<ImoId> objects;
    list<ImoObj*>::iterator it;
    for (it = m_imos.begin(); it != m_imos.end(); ++it)
    {
        ImoObj* pImo = (*it);
        if (pImo->get_obj_type() == type)
            objects.push_back(pImo->get_id());
    }
    return objects;
}

//---------------------------------------------------------------------------------------
list<ImoId> SelectionSet::filter_notes_rests()
{
    //note/rests are returned in order

    ensure_set_is_valid();

    list<ImoId> notes;
    ColStaffObjs* pCollection = get_staffobjs_collection();
    if (pCollection != nullptr)
    {
        ColStaffObjsIterator it;
        for (it = pCollection->begin(); it != pCollection->end(); ++it)
        {
            ImoObj* pImo = (*it)->imo_object();
            if (pImo->is_note_rest())
                notes.push_back(pImo->get_id());
        }
    }
    return notes;
}

//---------------------------------------------------------------------------------------
list<ImoObj*> SelectionSet::filter_deletable()
{
    ensure_set_is_valid();

    list<ImoObj*> objects;

    //TODO: ?
//    list<GmoObj*>::iterator it;
//    for (it = m_gmos.begin(); it != m_gmos.end(); ++it)
//    {
//        //secondary and prolog shapes
//        //instrument, brace, bracket, staff lines
//        GmoObj* pGmo = *it;
//        if (!pGmo->is_shape())
//        ImoObj* pImo = (*it)->get_creator_imo();
//        if (pImo)
//        {
//            m_imos.push_back(pImo);
//
//            if (pImo->is_staffobj())
//                add_staffobj_to_collection( static_cast<ImoStaffObj*>(pImo) );
//        }
//    }
    return objects;
}

//---------------------------------------------------------------------------------------
bool SelectionSet::is_valid_to_add_tie(ImoNote** ppStartNote, ImoNote** ppEndNote)
{
    ensure_set_is_valid();

    return m_pValidator->is_valid_to_add_tie(this, ppStartNote, ppEndNote);
}

//---------------------------------------------------------------------------------------
bool SelectionSet::is_valid_to_remove_tie()
{
    ensure_set_is_valid();

    return m_pValidator->is_valid_to_remove_tie(this);
}

//---------------------------------------------------------------------------------------
bool SelectionSet::is_valid_to_add_tuplet()
{
    ensure_set_is_valid();

    return m_pValidator->is_valid_to_add_tuplet(this);
}

//---------------------------------------------------------------------------------------
bool SelectionSet::is_valid_to_remove_tuplet()
{
    ensure_set_is_valid();

    return m_pValidator->is_valid_to_remove_tuplet(this);
}

//---------------------------------------------------------------------------------------
bool SelectionSet::is_valid_for_join_beam()
{
    ensure_set_is_valid();

    return m_pValidator->is_valid_for_join_beam(this);
}

//---------------------------------------------------------------------------------------
bool SelectionSet::is_valid_for_toggle_stem()
{
    ensure_set_is_valid();

    return m_pValidator->is_valid_for_toggle_stem(this);
}

//---------------------------------------------------------------------------------------
void SelectionSet::get_start_end_note_rests(ImoNoteRest** ppStart,
                                            ImoNoteRest** ppEnd)
{
    ensure_set_is_valid();

    ImoNoteRest* pStart = nullptr;
    ImoNoteRest* pEnd = nullptr;

    ColStaffObjs* pCollection = get_staffobjs_collection();
    if (pCollection != nullptr)
    {
        ColStaffObjsIterator it;
        for (it = pCollection->begin(); it != pCollection->end(); ++it)
        {
            ImoObj* pImo = (*it)->imo_object();
            if (pImo->is_note_rest())
            {
                if (!pStart)
                    pStart = static_cast<ImoNoteRest*>(pImo);
                else
                    pEnd = static_cast<ImoNoteRest*>(pImo);
            }
        }
    }

    if (ppStart)
        *ppStart = pStart;
    if (ppEnd)
        *ppEnd = pEnd;
}

//---------------------------------------------------------------------------------------
string SelectionSet::dump_selection()
{
    if (this->empty())
        return "No objects selected.";

    ensure_set_is_valid();

    if (m_pDoc)
    {
        stringstream msg;
        list<ImoId>::iterator it;
        for (it = m_ids.begin(); it != m_ids.end(); ++it)
        {
            ImoObj* pImo = m_pDoc->get_pointer_to_imo(*it);
            msg << pImo->get_id() << ": " << pImo->get_name() << endl;
        }
        return msg.str();
    }
    return "Error: can't access to Document!";
}


//=======================================================================================
// SelectionValidator implementation
//=======================================================================================
bool SelectionValidator::is_valid_to_add_tie(SelectionSet* pSelection,
                                           ImoNote** ppStartNote,
                                           ImoNote** ppEndNote)
{
    //Returns TRUE if current selection is valid for adding a tie.
    //If valid, returns pointers to start and end notes, if not nullptr parameters received


    //Conditions to be valid:
    //   1. The first note found can be tied to next one
    //   2. If condition 1 is true, the next note must also be in the selection

    bool fValid = false;
    ImoNote* pStart = nullptr;
    ImoNote* pEnd = nullptr;

    ColStaffObjs* pCollection = pSelection->get_staffobjs_collection();
    if (pCollection == nullptr)
        return false;

    ColStaffObjsIterator it;
    for (it = pCollection->begin(); it != pCollection->end(); ++it)
    {
        ImoObj* pImo = (*it)->imo_object();
        if (pImo->is_note())
        {
            if (!pStart)
            {
                //first note found. Verify if it can be tied to next
                pStart = static_cast<ImoNote*>(pImo);
                if (!pStart->is_tied_next())
                {
                    ImoScore* pScore = pStart->get_score();
                    ColStaffObjs* pCol = pScore->get_staffobjs_table();
                    pEnd = ScoreAlgorithms::find_possible_end_of_tie(pCol, pStart);
                }
            }
            else
            {
                //Start note processed. verify if end note is also in the selection
                if (pEnd && pEnd->get_id() == pImo->get_id())
                {
                    fValid = true;      //ok. End note is in the selection
                    break;
                }
            }
        }
    }

    if (fValid)
    {
        if (ppStartNote)
            *ppStartNote = pStart;
        if (ppEndNote)
            *ppEndNote = pEnd;
        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------------------
bool SelectionValidator::is_valid_to_remove_tie(SelectionSet* pSelection)
{
    //Returns TRUE if current selection is valid for removing at least a tie.
    //Conditions to be valid: There is at least a tie in the selection

    list<ImoObj*> imos = pSelection->get_all_objects();
    list<ImoObj*>::iterator it;
    for (it = imos.begin(); it != imos.end(); ++it)
    {
        ImoObj* pImo = (*it);
        if (pImo->is_tie())
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
bool SelectionValidator::is_valid_to_add_tuplet(SelectionSet* pSelection)
{
    //Checks if current selection is valid for adding a tuplet.
    //Conditions to be valid:
    //   1. All notes/rest in the seleccion are not in a tuplet, are consecutive, and are
    //      in the same voice.

    bool fValid = true;
    ImoNoteRest* pStart = nullptr;
    int nNumNotes = 0;
    int nVoice;

    ColStaffObjs* pCollection = pSelection->get_staffobjs_collection();
    if (pCollection == nullptr)
        return false;

    ColStaffObjsIterator it;
    for (it = pCollection->begin(); fValid && it != pCollection->end(); ++it)
    {
        ImoObj* pImo = (*it)->imo_object();
        if (pImo->is_note_rest())
        {
            nNumNotes++;
            ImoNoteRest* pNote = static_cast<ImoNoteRest*>(pImo);
            if (pNote->is_in_tuplet())
                return false;

            if (!pStart)
            {
                //This is the first note/rest
                pStart = pNote;
                nVoice = pStart->get_voice();
            }
            else
            {
                fValid &= nVoice == pNote->get_voice();
            }
        }
    }

    //check that more than one note
    fValid &= (nNumNotes > 1);

    return fValid;
}

//---------------------------------------------------------------------------------------
bool SelectionValidator::is_valid_to_remove_tuplet(SelectionSet* pSelection)
{
    //Returns TRUE if current selection is valid for removing at least a tuplet.
    //Conditions to be valid: There is at least a tuplet in the selection

    list<ImoObj*> imos = pSelection->get_all_objects();
    list<ImoObj*>::iterator it;
    for (it = imos.begin(); it != imos.end(); ++it)
    {
        ImoObj* pImo = (*it);
        if (pImo->is_tuplet())
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
bool SelectionValidator::is_valid_for_join_beam(SelectionSet* pSelection)
{
    //Returns TRUE if current selection is valid either:
    // - to create a beamed group with the selected notes,
    // - to join two or more beamed groups
    // - or to add a note to a beamed group

    //Conditions to be valid:
    //   1. All notes/rest in the seleccion are consecutive, are in the same
    //      voice (unless in chord), and must be eighths or shorter ones.
    //   2. If not beamed, first note/rest must be a note
    //   3. If not beamed, last note/rest must be a note
    //   4. If beamed, all selected note/rest must not be in the same beam

    bool fValid = true;
    ImoNoteRest* pStart = nullptr;

    int nNumNotes = 0;
    int nVoice;
    ImoNoteRest* pLast = nullptr;
    bool fAllBeamed = true;     //assume that all are beamed in the same beam
    ImoBeam* pCurBeam = nullptr;

    ColStaffObjs* pCollection = pSelection->get_staffobjs_collection();
    if (pCollection == nullptr)
        return false;

    ColStaffObjsIterator it;
    for (it = pCollection->begin(); fValid && it != pCollection->end(); ++it)
    {
        ImoObj* pImo = (*it)->imo_object();
        if (pImo->is_note_rest())
        {
            nNumNotes++;
            if (!pStart)
            {
                //This is the first note/rest. If not beamed, it must be a note
                //shorter than quarter
                pStart = static_cast<ImoNoteRest*>(pImo);
                nVoice = pStart->get_voice();
                if (!pStart->is_beamed())
                {
                    fValid &= pStart->is_note();
                    fValid &= static_cast<ImoNote*>(pStart)->get_note_type() >= k_eighth;
                    fAllBeamed = false;
                }
                else
                    pCurBeam = pStart->get_beam();
            }
            else
            {
                // verify voice, and that it is an eighth or shorter
                pLast = static_cast<ImoNoteRest*>(pImo);
                fValid &= pLast->get_note_type() >= k_eighth;
                fValid &= nVoice == pLast->get_voice() ||
                          (pLast->is_note() && static_cast<ImoNote*>(pLast)->is_in_chord());

                //verify that if beamed, all selected note/rest must not be in the same beam
                fAllBeamed &= pLast->is_beamed();
                if (fValid && fAllBeamed)
                    fAllBeamed &= (pCurBeam == pLast->get_beam());
            }
        }
    }

    //verify last note/rest. If not beamed, it must be a note
    if (pLast && !pLast->is_beamed())
        fValid &= pLast->is_note();

    return fValid && !fAllBeamed && nNumNotes > 1;
}

//---------------------------------------------------------------------------------------
bool SelectionValidator::is_valid_for_toggle_stem(SelectionSet* pSelection)
{
    //Returns TRUE if current selection is valid to toggle stems.
    //It is valid if there is at least a note with stem

    ColStaffObjs* pCollection = pSelection->get_staffobjs_collection();
    if (pCollection == nullptr)
        return false;

    ColStaffObjsIterator it;
    for (it = pCollection->begin(); it != pCollection->end(); ++it)
    {
        ImoObj* pImo = (*it)->imo_object();
        if (pImo->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>(pImo);
            if (pNote->get_note_type() > k_whole && !pNote->is_in_chord()
                && pNote->get_stem_direction() != k_stem_none)
                return true;
        }
    }

    return false;
}


}  //namespace lomse

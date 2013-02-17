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

#include "lomse_score_modifier.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_table.h"

namespace lomse
{


//=======================================================================================
// ScoreModifier implementation
//=======================================================================================
ScoreModifier::ScoreModifier(ImoScore* pScore)
    : m_pScore(pScore)
    , m_pColStaffObjs( pScore->get_staffobjs_table() )
{
}

//---------------------------------------------------------------------------------------
void ScoreModifier::insert_staffobj(ImoStaffObj* pPos, ImoStaffObj* pSO)
{
    //insert pImo before pPos

    ImoInstrument* pInstr = pSO->get_instrument();
    ImoMusicData* pMD = pInstr->get_musicdata();
    TreeNode<ImoObj>::iterator it(pPos);
    pMD->insert(it, pSO);
    pInstr->set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ScoreModifier::delete_staffobj(ImoStaffObj* pSO)
{
    remove_object_from_all_relationships(pSO);

    if (is_necessary_to_shift_time_back_from(pSO))
        shift_objects_back_in_time_from(pSO);

    if (pSO->is_barline())
        decrement_measure_number_from(pSO);

    if (pSO->is_key_signature())
    {
    }
    //Key Signature: ask user. If user agrees, repitch all affected notes until a
    //  new applicable key signature found.
    //
    //        Notes after the key will be affected by this action.
    //        Would you like to keep notes' pitch and, therefore, to add/remove accidentals to the affected notes?
    //            1. Keep pitch by adding/removing accidentals when necessary.
    //            2. Do nothing. Notes' pitch will be affected by the change in key signature. ==> repitch
    //            3. Cancel. The insert, delete or change key command will be cancelled.

    else if (pSO->is_clef())
    {
    //Deleting a clef doesn't imply re-pitching notes. But if notes are not re-pitched they will be repainted in a different position on staff (the position implied by previous clef) and, if no previous clef exist G4 clef will be assumed. Therefore it is convenient to ask user about what to do:
    //    //  nAction:  0=Cancel operation, 1=keep pitch, 2=keep position
    //
    //    ask user. If user agrees, repitch all affected notes until a new applicable clef found
    }

    else if (pSO->is_time_signature())
    {
    //ask user. If user agrees, delete all barlines until a new time signature found (the preceeding barline must be kept)
    }

    remove_object_from_staffobjs_table(pSO);

    remove_object_from_imo_tree(pSO);
}

//---------------------------------------------------------------------------------------
void ScoreModifier::remove_object_from_staffobjs_table(ImoStaffObj* pSO)
{
    m_pColStaffObjs->delete_entry_for(pSO);
}

//---------------------------------------------------------------------------------------
bool ScoreModifier::is_necessary_to_shift_time_back_from(ImoStaffObj* pSO)
{
    //StaffObj pSO is goin to be deleted. As a consequence, the time position of all
    //staff objects (in this instrument and staff) after pSO could be affected.
    //This method checks if it is necessary to shift times back

    if (pSO->get_duration() > 0.0f)
    {
        bool fShiftTime = true;     //assume it
        if (pSO->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>( pSO );
            fShiftTime = !pNote->is_in_chord();
        }
        return fShiftTime;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void ScoreModifier::shift_objects_back_in_time_from(ImoStaffObj* pSO)
{
    //StaffObj pSO is goin to be deleted. Proceed to shift back the time position
    //of all staffobjs in this instr & voice (line). When a barline is found determine time
    //implied by each staff and choose greater one. If barline time doesn't change stop shifting times.
    //OPTIMIZATION: barlines should store time implied by each staff.

    float timeShift = pSO->get_duration();
    ColStaffObjsIterator it = m_pColStaffObjs->find(pSO);
    ColStaffObjsEntry* pEntry = *it;
    int instr = pEntry->num_instrument();
    int line = pEntry->line();
    ++it;   //skip object to be deleted
    while(it != m_pColStaffObjs->end())
    {
        if ((*it)->line() == line && (*it)->num_instrument() == instr)
        {
            (*it)->decrement_time(timeShift);
        }
        ++it;
    }
    m_pColStaffObjs->sort_table();
}

//---------------------------------------------------------------------------------------
void ScoreModifier::remove_object_from_all_relationships(ImoStaffObj* pSO)
{
    //remove from all relations. Could imply deleting the relation.
    //TODO
}

//---------------------------------------------------------------------------------------
void ScoreModifier::remove_object_from_imo_tree(ImoStaffObj* pSO)
{
    ImoInstrument* pInstr = pSO->get_instrument();
    ImoMusicData* pMusicData = pInstr->get_musicdata();
    pMusicData->remove_child(pSO);
    delete pSO;
    pInstr->set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ScoreModifier::decrement_measure_number_from(ImoStaffObj* pSO)
{
    //received StaffObj is the barline to be deleted. As a consequence, all
    //measures in this instrument after this barline must be renumbered.

    //TODO
}


}  //namespace lomse

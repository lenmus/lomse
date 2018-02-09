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

#include "lomse_im_algorithms.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_table.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"

namespace lomse
{


//=======================================================================================
// ImoTreeAlgoritms implementation
//=======================================================================================
ImoTreeAlgoritms::ImoTreeAlgoritms()    //ImoScore* pScore)
//    : m_pScore(pScore)
//    , m_pColStaffObjs( pScore->get_staffobjs_table() )
{
}

//---------------------------------------------------------------------------------------
void ImoTreeAlgoritms::remove_staffobj(Document* pDoc, ImoStaffObj* pSO)
{
    //get and save relations
    vector<ImoId> relIds;
    ImoRelations* pRels = pSO->get_relations();
    if (pRels)
    {
        list<ImoRelObj*>& relations = pRels->get_relations();
        if (relations.size() > 0)
        {
            list<ImoRelObj*>::iterator it;
            for (it = relations.begin(); it != relations.end(); ++it)
            {
                relIds.push_back( (*it)->get_id() );
            }
        }
    }

    //delete object
    ImoInstrument* pInstr = pSO->get_instrument();
    pInstr->delete_staffobj(pSO);

    //ask relations to reorganize themselves
    if (relIds.size() > 0)
    {
        vector<ImoId>::iterator itV;
        for (itV = relIds.begin(); itV != relIds.end(); ++itV)
        {
            ImoRelObj* pRO = static_cast<ImoRelObj*>( pDoc->get_pointer_to_imo(*itV) );
            if (pRO)
                pRO->reorganize_after_object_deletion();
        }
    }
}

//---------------------------------------------------------------------------------------
void ImoTreeAlgoritms::replace_staffobj(Document* pDoc, ImoStaffObj* pSO,
                                        ImoStaffObj* pAt, const string& ldpsource)
{
    ImoInstrument* pInstr = pSO->get_instrument();
    remove_staffobj(pDoc, pSO);
    insert_staffobjs(pInstr, pAt, ldpsource);
}

//---------------------------------------------------------------------------------------
void ImoTreeAlgoritms::change_noterest_duration(ImoNoteRest* pNR, TimeUnits duration)
{
    NoteTypeAndDots figdots = duration_to_note_type_and_dots(duration);
    pNR->set_note_type_and_dots(figdots.noteType, figdots.dots);
}

//---------------------------------------------------------------------------------------
list<ImoStaffObj*> ImoTreeAlgoritms::insert_staffobjs(ImoInstrument* pInstr,
                                                      ImoStaffObj* pAt,
                                                      const string& ldpsource)
{
    ImoScore* pScore = pInstr->get_score();
    stringstream errormsg;
    list<ImoStaffObj*> objects
                = pInstr->insert_staff_objects_at(pAt, ldpsource, errormsg);
    if (objects.size() > 0)
        pScore->end_of_changes();        //update ColStaffObjs table

    return objects;
}

//---------------------------------------------------------------------------------------
void ImoTreeAlgoritms::add_note_to_chord(ImoNote* pBaseNote, ImoNote* pNewNote,
                                         Document* pDoc)
{
    //AWARE: new note must have been added to Imo tree
    //This method just "joins" the notes into a chord

    //create/update the chord
    ImoChord* pChord;
    if (pBaseNote->is_in_chord())
    {
        //chord already created. Get it
        pChord = pBaseNote->get_chord();
    }
    else
    {
        //chord didn't exist. Create it
        pChord = static_cast<ImoChord*>(ImFactory::inject(k_imo_chord, pDoc));
        pBaseNote->include_in_relation(pDoc, pChord);
    }
    pNewNote->include_in_relation(pDoc, pChord);
}

//---------------------------------------------------------------------------------------
NoteTypeAndDots duration_to_note_type_and_dots(TimeUnits duration)
{
    //determine type
    int type = k_256th;
    if (is_lower_time(duration, 2.0))
        type = k_256th;
    else if (is_lower_time(duration, 4.0))
        type = k_128th;
    else if (is_lower_time(duration, 8.0))
        type = k_64th;
    else if (is_lower_time(duration, 16.0))
        type = k_32nd;
    else if (is_lower_time(duration, 32.0))
        type = k_16th;
    else if (is_lower_time(duration, 64.0))
        type = k_eighth;
    else if (is_lower_time(duration, 128.0))
        type = k_quarter;
    else if (is_lower_time(duration, 256.0))
        type = k_half;
    else if (is_lower_time(duration, 512.0))
        type = k_whole;
    else if (is_lower_time(duration, 1024.0))
        type = k_breve;
    else
        type = k_longa;

    //compute dots
    TimeUnits base = to_duration(type, 0);
    duration -= to_duration(type, 0);
    double factor = 1.0 + duration / base;
    if (factor < 1.25)
        return NoteTypeAndDots(type, 0);
    if (factor < 1.625)
        return NoteTypeAndDots(type, 1);
    if (factor < 1.8125)
        return NoteTypeAndDots(type, 2);
    if (factor < 1.90625)
        return NoteTypeAndDots(type, 3);
    if (factor < 1.953125)
        return NoteTypeAndDots(type, 4);
    if (factor < 1.9765625)
        return NoteTypeAndDots(type, 5);
    if (factor < 1.98828125)
        return NoteTypeAndDots(type, 6);
    if (factor < 1.99414063)
        return NoteTypeAndDots(type, 7);
    if (factor < 1.99707031)
        return NoteTypeAndDots(type, 8);
    else
        return NoteTypeAndDots(type, 9);
}















////---------------------------------------------------------------------------------------
//void ImoTreeAlgoritms::insert_staffobj(ImoStaffObj* pPos, ImoStaffObj* pSO)
//{
//    //insert pImo before pPos
//
//    ImoInstrument* pInstr = pSO->get_instrument();
//    ImoMusicData* pMD = pInstr->get_musicdata();
//    TreeNode<ImoObj>::iterator it(pPos);
//    pMD->insert(it, pSO);
//    pInstr->set_dirty(true);
//}
//
////---------------------------------------------------------------------------------------
//void ImoTreeAlgoritms::delete_staffobj(ImoStaffObj* pSO)
//{
//    remove_object_from_all_relationships(pSO);
//
//    if (is_necessary_to_shift_time_back_from(pSO))
//        shift_objects_back_in_time_from(pSO);
//
//    if (pSO->is_barline())
//        decrement_measure_number_from(pSO);
//
//    if (pSO->is_key_signature())
//    {
//    }
//    //Key Signature: ask user. If user agrees, repitch all affected notes until a
//    //  new applicable key signature found.
//    //
//    //        Notes after the key will be affected by this action.
//    //        Would you like to keep notes' pitch and, therefore, to add/remove accidentals to the affected notes?
//    //            1. Keep pitch by adding/removing accidentals when necessary.
//    //            2. Do nothing. Notes' pitch will be affected by the change in key signature. ==> repitch
//    //            3. Cancel. The insert, delete or change key command will be cancelled.
//
//    else if (pSO->is_clef())
//    {
//    //Deleting a clef doesn't imply re-pitching notes. But if notes are not re-pitched they will be repainted in a different position on staff (the position implied by previous clef) and, if no previous clef exist G4 clef will be assumed. Therefore it is convenient to ask user about what to do:
//    //    //  nAction:  0=Cancel operation, 1=keep pitch, 2=keep position
//    //
//    //    ask user. If user agrees, repitch all affected notes until a new applicable clef found
//    }
//
//    else if (pSO->is_time_signature())
//    {
//    //ask user. If user agrees, delete all barlines until a new time signature found (the preceeding barline must be kept)
//    }
//
//    remove_object_from_staffobjs_table(pSO);
//
//    remove_object_from_imo_tree(pSO);
//}
//
////---------------------------------------------------------------------------------------
//void ImoTreeAlgoritms::remove_object_from_staffobjs_table(ImoStaffObj* pSO)
//{
//    m_pColStaffObjs->delete_entry_for(pSO);
//}
//
////---------------------------------------------------------------------------------------
//bool ImoTreeAlgoritms::is_necessary_to_shift_time_back_from(ImoStaffObj* pSO)
//{
//    //StaffObj pSO is goin to be deleted. As a consequence, the time position of all
//    //staff objects (in this instrument and staff) after pSO could be affected.
//    //This method checks if it is necessary to shift times back
//
//    if (pSO->get_duration() > 0.0f)
//    {
//        bool fShiftTime = true;     //assume it
//        if (pSO->is_note())
//        {
//            ImoNote* pNote = static_cast<ImoNote*>( pSO );
//            fShiftTime = !pNote->is_in_chord();
//        }
//        return fShiftTime;
//    }
//    return false;
//}
//
////---------------------------------------------------------------------------------------
//void ImoTreeAlgoritms::shift_objects_back_in_time_from(ImoStaffObj* pSO)
//{
//    //StaffObj pSO is goin to be deleted. Proceed to shift back the time position
//    //of all staffobjs in this instr & voice (line). When a barline is found determine time
//    //implied by each staff and choose greater one. If barline time doesn't change stop shifting times.
//    //OPTIMIZATION: barlines should store time implied by each staff.
//
//    float timeShift = pSO->get_duration();
//    ColStaffObjsIterator it = m_pColStaffObjs->find(pSO);
//    ColStaffObjsEntry* pEntry = *it;
//    int instr = pEntry->num_instrument();
//    int line = pEntry->line();
//    ++it;   //skip object to be deleted
//    while(it != m_pColStaffObjs->end())
//    {
//        if ((*it)->line() == line && (*it)->num_instrument() == instr)
//        {
//            (*it)->decrement_time(timeShift);
//        }
//        ++it;
//    }
//    m_pColStaffObjs->sort_table();
//}
//
////---------------------------------------------------------------------------------------
//void ImoTreeAlgoritms::remove_object_from_all_relationships(ImoStaffObj* pSO)
//{
//    //remove from all relations. Could imply deleting the relation.
//    //TODO
//}
//
////---------------------------------------------------------------------------------------
//void ImoTreeAlgoritms::remove_object_from_imo_tree(ImoStaffObj* pSO)
//{
//    ImoInstrument* pInstr = pSO->get_instrument();
//    ImoMusicData* pMusicData = pInstr->get_musicdata();
//    pMusicData->remove_child(pSO);
//    delete pSO;
//    pInstr->set_dirty(true);
//}
//
////---------------------------------------------------------------------------------------
//void ImoTreeAlgoritms::decrement_measure_number_from(ImoStaffObj* pSO)
//{
//    //received StaffObj is the barline to be deleted. As a consequence, all
//    //measures in this instrument after this barline must be renumbered.
//
//    //TODO
//}


}  //namespace lomse

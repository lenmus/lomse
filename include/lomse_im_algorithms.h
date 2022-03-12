//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_IM_ALGORITHMS_H__        //to avoid nested includes
#define __LOMSE_IM_ALGORITHMS_H__

#include "lomse_internal_model.h"       //struct NoteTypeAndDots
//#include "lomse_basic.h"


#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class ColStaffObjs;
class ImoScore;
class ImoStaffObj;
class Document;
class ImoNoteRest;


//---------------------------------------------------------------------------------------
// ImoTreeAlgoritms:  Encloses knowledge for performing common operations on ImoTree.
//                    Mainly oriented to support complex commands.
class ImoTreeAlgoritms
{
protected:
//    ImoScore* m_pScore;
//    ColStaffObjs* m_pColStaffObjs;

public:
    ImoTreeAlgoritms();
    virtual ~ImoTreeAlgoritms() {}


    static void replace_staffobj(Document* pDoc, ImoStaffObj* pSO,
                                 ImoStaffObj* pAt, const string& ldpsource);
    static void remove_staffobj(Document* pDoc, ImoStaffObj* pSO);
    static void change_noterest_duration(ImoNoteRest* pNR, TimeUnits duration);
    static list<ImoStaffObj*> insert_staffobjs(ImoInstrument* pInstr, ImoStaffObj* pAt,
                                               const string& ldpsource);
    static void add_note_to_chord(ImoNote* pBaseNote, ImoNote* pNewNote,
                                  Document* pDoc);



//    //score edition API
//    void delete_staffobj(ImoStaffObj* pImo);
//    void insert_staffobj(ImoStaffObj* pPos, ImoStaffObj* pSO);

protected:
//    void remove_object_from_all_relationships(ImoStaffObj* pSO);
//    bool is_necessary_to_shift_time_back_from(ImoStaffObj* pSO);
//    void shift_objects_back_in_time_from(ImoStaffObj* pSO);
//    void remove_object_from_imo_tree(ImoStaffObj* pSO);
//    void remove_object_from_staffobjs_table(ImoStaffObj* pSO);
//    void decrement_measure_number_from(ImoStaffObj* pSO);

};

extern NoteTypeAndDots duration_to_note_type_and_dots(TimeUnits duration);


}   //namespace lomse

#endif    // __LOMSE_IM_ALGORITHMS_H__


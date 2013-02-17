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

#ifndef __LOMSE_SCORE_MODIFIER_H__        //to avoid nested includes
#define __LOMSE_SCORE_MODIFIER_H__

//#include "lomse_build_options.h"

using namespace std;

namespace lomse
{

//forward declarations
class ColStaffObjs;
class ImoScore;
class ImoStaffObj;

//---------------------------------------------------------------------------------------
class ScoreModifier
{
protected:
    ImoScore* m_pScore;
    ColStaffObjs* m_pColStaffObjs;


public:
    ScoreModifier(ImoScore* pScore);
    virtual ~ScoreModifier() {}

    //score edition API
    void delete_staffobj(ImoStaffObj* pImo);
    void insert_staffobj(ImoStaffObj* pPos, ImoStaffObj* pSO);

protected:
    void remove_object_from_all_relationships(ImoStaffObj* pSO);
    bool is_necessary_to_shift_time_back_from(ImoStaffObj* pSO);
    void shift_objects_back_in_time_from(ImoStaffObj* pSO);
    void remove_object_from_imo_tree(ImoStaffObj* pSO);
    void remove_object_from_staffobjs_table(ImoStaffObj* pSO);
    void decrement_measure_number_from(ImoStaffObj* pSO);

};


}   //namespace lomse

#endif    // __LOMSE_SCORE_MODIFIER_H__


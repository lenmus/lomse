//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include "lomse_model_builder.h"

#include "lomse_document.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_table.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_score_utilities.h"

using namespace std;

namespace lomse
{

//=======================================================================================
// ModelBuilder implementation
//=======================================================================================
ImoDocument* ModelBuilder::build_model(InternalModel* IModel)
{
    ImoDocument* pDoc = static_cast<ImoDocument*>( IModel->get_root() );
    int numContent = pDoc->get_num_content_items();
    for (int i = 0; i < numContent; i++)
        structurize( pDoc->get_content_item(i) );
    return pDoc;
}


////---------------------------------------------------------------------------------------
//void ModelBuilder::update_model(InternalModel* IModel)
//{
//    m_pTree = pTree;
//    DocIterator it(m_pTree);
//    for (it.start_of_content(); *it != NULL; ++it)
//    {
//        //Factory method ?
//        if ((*it)->is_modified())
//        {
//            if((*it)->is_type(k_score))
//            {
//                ImoScore* pScore = dynamic_cast<ImoScore*>( (*it)->get_imobj() );
//                ColStaffObjsBuilder builder(m_pTree);
//                builder.update(pScore);
//            }
//        }
//    }
//}

//---------------------------------------------------------------------------------------
void ModelBuilder::structurize(ImoObj* pImo)
{
    //in future this should invoke a factory object

    if (pImo->is_score())
    {
        ImoScore* pScore = static_cast<ImoScore*>(pImo);
        ColStaffObjsBuilder builder;
        builder.build(pScore);
        PitchAssigner tuner;
        tuner.assign_pitch(pScore);
    }
}


//=======================================================================================
// ModelBuilder implementation
//=======================================================================================
PitchAssigner::PitchAssigner()
{
}

//---------------------------------------------------------------------------------------
void PitchAssigner::assign_pitch(ImoScore* pScore)
{
    StaffObjsCursor cursor(pScore);

    ImoKeySignature* pKey = NULL;
    reset_accidentals(pKey);

    while(!cursor.is_end())
    {
        ImoStaffObj* pSO = cursor.get_staffobj();
        if (pSO->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>(pSO);
            compute_pitch(pNote);
        }
        else if (pSO->is_barline())
        {
            reset_accidentals(pKey);
        }
        else if (pSO->is_key_signature())
        {
            pKey = static_cast<ImoKeySignature*>( pSO );
            reset_accidentals(pKey);
        }

        cursor.move_next();
    }
}

//---------------------------------------------------------------------------------------
void PitchAssigner::compute_pitch(ImoNote* pNote)
{
    float alter = pNote->get_actual_accidentals();
    int step = pNote->get_step();
    if (alter == k_acc_not_computed && step != k_no_pitch)
    {
        update_context_accidentals(pNote);
        pNote->set_actual_accidentals( float(m_accidentals[step]) );
    }
}

//---------------------------------------------------------------------------------------
void PitchAssigner::reset_accidentals(ImoKeySignature* pKey)
{
    if (pKey)
    {
        int keyType = pKey->get_key_type();
        get_accidentals_for_key(keyType, m_accidentals);
    }
    else
    {
        for (int iStep=0; iStep < 7; ++iStep)
            m_accidentals[iStep] = 0;
    }
}

//---------------------------------------------------------------------------------------
void PitchAssigner::update_context_accidentals(ImoNote* pNote)
{
    int step = pNote->get_step();
    EAccidentals acc = pNote->get_notated_accidentals();
    switch (acc)
    {
        case k_no_accidentals:
            //do not modify context
            break;
        case k_natural:
            //force 'natural' (=no accidentals)
            m_accidentals[step] = 0;
            break;
        case k_flat:
            //Force one flat
            m_accidentals[step] = -1;
            break;
        case k_natural_flat:
            //Force one flat
            m_accidentals[step] = -1;
            break;
        case k_sharp:
            //force one sharp
            m_accidentals[step] = 1;
            break;
        case k_natural_sharp:
            //force one sharp
            m_accidentals[step] = 1;
            break;
        case k_flat_flat:
            //Force two flats
            m_accidentals[step] = -2;
            break;
        case k_sharp_sharp:
        case k_double_sharp:
            //force two sharps
            m_accidentals[step] = 2;
            break;
        default:
            ;
    }
}


}  //namespace lomse

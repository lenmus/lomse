//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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
#include "lomse_logger.h"
#include "lomse_im_factory.h"
#include "lomse_measures_table.h"

#include <algorithm>
using namespace std;

namespace lomse
{


//=======================================================================================
// helper class for accessing scores
// AWARE: if in future there are more structurizable elements, modify this class as
//        shown in commented sentences.
//=======================================================================================
class VisitorForStructurizables : public Visitor<ImoScore>
//                                , public Visitor<ImoOtherStructurizable>
{
protected:
    ModelBuilder* m_builder;

public:
    VisitorForStructurizables(ModelBuilder* builder)
        : Visitor<ImoScore>()
        //, Visitor<ImoOtherStructurizable>()
        , m_builder(builder)
    {
    }
	virtual ~VisitorForStructurizables() {}

    void start_visit(ImoScore* pImo) { m_builder->structurize(pImo); }
    //void start_visit(ImoOtherStructurizable* pImo) { m_builder->structurize(pImo); }

	void end_visit(ImoScore* UNUSED(pImo)) {}
    //void end_visit(ImoOtherStructurizable* pImo) {}

};


//=======================================================================================
// ModelBuilder implementation
//=======================================================================================
ImoDocument* ModelBuilder::build_model(ImoDocument* pImoDoc)
{
    if (pImoDoc)
    {
        VisitorForStructurizables v(this);
        pImoDoc->accept_visitor(v);
    }
    return pImoDoc;
}

//---------------------------------------------------------------------------------------
void ModelBuilder::structurize(ImoObj* pImo)
{
    //in future this should invoke a factory object

    if (pImo && pImo->is_score())
    {
        ImoScore* pScore = static_cast<ImoScore*>(pImo);

        ColStaffObjsBuilder builder;
        builder.build(pScore);

        MeasuresTableBuilder measures;
        measures.build(pScore);

        MidiAssigner assigner;
        assigner.assign_midi_data(pScore);

        PitchAssigner tuner;
        tuner.assign_pitch(pScore);
    }
}


//=======================================================================================
// PitchAssigner implementation
//=======================================================================================
void PitchAssigner::assign_pitch(ImoScore* pScore)
{
    StaffObjsCursor cursor(pScore);

    ImoKeySignature* pKey = nullptr;
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


//=======================================================================================
// MidiAssigner implementation
//=======================================================================================
MidiAssigner::MidiAssigner()
{
}

//---------------------------------------------------------------------------------------
MidiAssigner::~MidiAssigner()
{
}

//---------------------------------------------------------------------------------------
void MidiAssigner::assign_midi_data(ImoScore* pScore)
{
    collect_sounds_info(pScore);
    assign_score_instr_id();
    assign_port_and_channel();
}

//---------------------------------------------------------------------------------------
void MidiAssigner::collect_sounds_info(ImoScore* pScore)
{
    int instruments = pScore->get_num_instruments();
    for (int iInstr=0; iInstr < instruments; ++iInstr)
    {
        ImoInstrument* pInstr = pScore->get_instrument(iInstr);
        ImoSounds* sounds = pInstr->get_sounds();
        if (sounds)
        {
            ImoObj::children_iterator it;
            for (it= sounds->begin(); it != sounds->end(); ++it)
            {
                ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>(*it);
                m_sounds.push_back(pInfo);
                if (!pInfo->get_score_instr_id().empty())
                    m_ids.push_back(pInfo->get_score_instr_id());
            }
        }
        else
        {
            Document* pDoc = pInstr->get_the_document();
            ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>(
                                        ImFactory::inject(k_imo_sound_info, pDoc) );
            pInstr->add_sound_info(pInfo);
            m_sounds.push_back(pInfo);
        }
    }
}

//---------------------------------------------------------------------------------------
void MidiAssigner::assign_score_instr_id()
{
    int idx = 0;
    list<ImoSoundInfo*>::iterator it;
    for (it=m_sounds.begin(); it != m_sounds.end(); ++it)
    {
        if ((*it)->get_score_instr_id().empty())
        {
            while (true)
            {
                ++idx;
                stringstream id;
                id << "SOUND-" << idx;
                if (find(m_ids.begin(), m_ids.end(), id.str()) == m_ids.end())
                {
                    (*it)->set_score_instr_id(id.str());
                    break;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void MidiAssigner::assign_port_and_channel()
{
    //Algorithm:
    //- each port (p) has 16 channels (c), assume infinite number of ports
    //- mapping index: p*16+c
    //- initial default index: idx=0
    // Rules:
    //- p,c specified:
    //      nothing to do. Assign it
    //- only c specified:
    //      find first port in which channel c is free, assign it
    //- only p specified:
    //      find first free channel in port p, and assign it.
    //      Exclude channel 9 (MIDI channel 10, percussion).
    //	    If no channels available, ignore p and assume p&c not specified (next rule)
    //- p,c not specified:
    //      iterate idx++ and assign the first free idx:

	map<int, ImoSoundInfo*> m_assigned;
    int idx = 0;
    list<ImoSoundInfo*>::iterator it;
    for (it=m_sounds.begin(); it != m_sounds.end(); ++it)
    {
        ImoMidiInfo* pMidi = (*it)->get_midi_info();

        //assign port and channel
        int p = pMidi->get_midi_port();
        int c = pMidi->get_midi_channel();

        // p,c specified
        if (p != -1 && c != -1)
            m_assigned[p*16+c] = *it;

        // only c specified
        else if (p == -1)
        {
            int i = c;
            while(true)
            {
                if (m_assigned[i] == nullptr)
                {
                    pMidi->set_midi_port(i / 16);
                    m_assigned[i] = *it;
                    break;
                }
                i += 16;
            }
        }

        // only p specified
        else if (c == -1)
        {
            bool fAssigned = false;
            for (int ch=0; ch < 16; ++ch)
            {
                if (ch==9) ++ch;

                int i = p*16 + ch;
                if (m_assigned[i] == nullptr)
                {
                    pMidi->set_midi_channel(i % 16);
                    m_assigned[i] = *it;
                    fAssigned = true;
                    break;
                }
            }
            if (!fAssigned)
                p = -1;
        }

        // none specified
        if (p == -1 && c == -1)
        {
            while(true)
            {
                if (m_assigned[idx] == nullptr)
                {
                    pMidi->set_midi_port(idx / 16);
                    pMidi->set_midi_channel(idx % 16);
                    m_assigned[idx] = *it;
                    ++idx;
                    break;
                }
                ++idx;
            }
        }
    }
}


//=======================================================================================
// MeasuresTableBuilder implementation
//=======================================================================================
MeasuresTableBuilder::MeasuresTableBuilder()
{
}

//---------------------------------------------------------------------------------------
MeasuresTableBuilder::~MeasuresTableBuilder()
{
}

//---------------------------------------------------------------------------------------
void MeasuresTableBuilder::build(ImoScore* pScore)
{
    ColStaffObjs* pCSO = pScore->get_staffobjs_table();
    if (pCSO->num_entries() == 0)
        return;

    int numInstrs = pScore->get_num_instruments();
    m_instruments.assign(numInstrs, nullptr);
    m_measures.assign(numInstrs, nullptr);

    ColStaffObjsIterator it = pCSO->begin();
    while (it != pCSO->end())
    {
        ColStaffObjsEntry* pCsoEntry = *it;
        int iInstr = pCsoEntry->num_instrument();
        ImoStaffObj* pSO = pCsoEntry->imo_object();

        //if first entry for the instrument create measures table and first measure
        if (m_instruments[iInstr] == nullptr)
        {
            ImoInstrument* pInstr = pScore->get_instrument(iInstr);
            start_measures_table_for(iInstr, pInstr, pCsoEntry);
        }

        //start new measure if no current measure
        if (m_measures[iInstr] == nullptr)
            start_new_measure(iInstr, pCsoEntry);

        //if Time Signature update beat duration
        if (pSO->is_time_signature())
        {
            ImoTimeSignature* pTS = static_cast<ImoTimeSignature*>(pSO);
            m_measures[iInstr]->set_beat_duration( pTS->get_beat_duration() );
        }

        //if not intermediate barline finish current measure
        if (pSO->is_barline())
        {
            ImoBarline* pBL = static_cast<ImoBarline*>(pSO);
            if (!pBL->is_middle())
                finish_current_measure(iInstr);
        }

        //advance to next entry
        ++it;
    }
}

//---------------------------------------------------------------------------------------
void MeasuresTableBuilder::start_measures_table_for(int iInstr, ImoInstrument* pInstr,
                                                    ColStaffObjsEntry* pCsoEntry)
{
    m_instruments[iInstr] = pInstr;

    //create measures table
    ImMeasuresTable* pTable = LOMSE_NEW ImMeasuresTable();
    pInstr->set_measures_table(pTable);

    //add first measure
    m_measures[iInstr] = pTable->add_entry(pCsoEntry);
}

//---------------------------------------------------------------------------------------
void MeasuresTableBuilder::finish_current_measure(int iInstr)
{
    m_measures[iInstr] = nullptr;
}

//---------------------------------------------------------------------------------------
void MeasuresTableBuilder::start_new_measure(int iInstr, ColStaffObjsEntry* pCsoEntry)
{
    ImoInstrument* pInstr = m_instruments[iInstr];
    ImMeasuresTable* pTable = pInstr->get_measures_table();
    ImMeasuresTableEntry* prevMeasure = pTable->back();
    m_measures[iInstr] = pTable->add_entry(pCsoEntry);

    if (prevMeasure != nullptr)
        m_measures[iInstr]->set_beat_duration( prevMeasure->get_beat_duration() );
}


}  //namespace lomse

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_model_builder.h"

#include "private/lomse_document_p.h"
#include "lomse_internal_model.h"
#include "private/lomse_internal_model_p.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_table.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_score_utilities.h"
#include "lomse_logger.h"
#include "lomse_im_factory.h"
#include "lomse_im_measures_table.h"

#include <math.h>       //round

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

    void start_visit(ImoScore* pImo) override { m_builder->structurize(pImo); }
    //void start_visit(ImoOtherStructurizable* pImo) { m_builder->structurize(pImo); }

	void end_visit(ImoScore* UNUSED(pImo)) override {}
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

        PartIdAssigner parts;
        parts.assign_parts_id(pScore);

        GroupBarlinesFixer fixer;
        fixer.set_barline_layout_in_instruments(pScore);
    }
}


//=======================================================================================
// PitchAssigner implementation
//=======================================================================================
void PitchAssigner::assign_pitch(ImoScore* pScore)
{
    if (pScore->get_accidentals_model() == ImoScore::k_pitch_and_notation_provided)
        return;

    StaffObjsCursor cursor(pScore);

    int staves = cursor.get_num_staves();
    m_context.assign(staves, {{0,0,0,0,0,0,0}} );       //alterations, per staff index

    int numInstrs = cursor.get_num_instruments();
    vector<ImoKeySignature*> keys;                      //key, per instrument
    keys.assign(numInstrs, nullptr);

    vector<int> numStaves;                              //num. staves, per instrument
    for (int i=0; i< numInstrs; ++i)
    {
        numStaves.push_back( pScore->get_instrument(i)->get_num_staves() );
    }

    while(!cursor.is_end())
    {
        ImoStaffObj* pSO = cursor.get_staffobj();
        if (pSO->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>(pSO);
            int idx = cursor.staff_index();
            compute_pitch(pNote, idx);
        }
        else if (pSO->is_barline())
        {
            int iInstr = cursor.num_instrument();
            for (int iStaff=0; iStaff < numStaves[iInstr]; ++iStaff)
            {
                int idx = cursor.staff_index_for(iInstr, iStaff);
                reset_accidentals(keys[iInstr], idx);
            }
        }
        else if (pSO->is_key_signature())
        {
            int iInstr = cursor.num_instrument();
            keys[iInstr] = static_cast<ImoKeySignature*>( pSO );
            for (int iStaff=0; iStaff < numStaves[iInstr]; ++iStaff)
            {
                int idx = cursor.staff_index_for(iInstr, iStaff);
                reset_accidentals(keys[iInstr], idx);
            }
        }

        cursor.move_next();
    }
}

//---------------------------------------------------------------------------------------
void PitchAssigner::compute_notated_accidentals(ImoNote* pNote, int context)
{
    int required = int(round( pNote->get_actual_accidentals() ));
    EAccidentals acc = k_no_accidentals;

    //determine notated accidentals, as required by music notation rules
    switch(context)
    {
        case 0:
        {
            switch(required)
            {
                case -2:    acc = k_flat_flat;            break;
                case -1:    acc = k_flat;                 break;
                case 0:     acc = k_no_accidentals;       break;
                case 1:     acc = k_sharp;                break;
                case 2:     acc = k_double_sharp;         break;
                default:    acc = k_invalid_accidentals;  break;
            }
            break;
        }

        case 1:
        {
            switch(required)
            {
                case -2:    acc = k_flat_flat;            break;
                case -1:    acc = k_flat;                 break;
                case 0:     acc = k_natural;              break;
                case 1:     acc = k_no_accidentals;       break;
                case 2:     acc = k_double_sharp;         break;
                default:    acc = k_invalid_accidentals;  break;
            }
            break;
        }

        case 2:
        {
            switch(required)
            {
                case -2:    acc = k_flat_flat;            break;
                case -1:    acc = k_flat;                 break;
                case 0:     acc = k_natural;              break;
                case 1:     acc = k_natural_sharp;        break;
                case 2:     acc = k_no_accidentals;       break;
                default:    acc = k_invalid_accidentals;  break;
            }
            break;
        }

        case -1:
        {
            switch(required)
            {
                case -2:    acc = k_flat_flat;            break;
                case -1:    acc = k_no_accidentals;       break;
                case 0:     acc = k_natural;              break;
                case 1:     acc = k_sharp;                break;
                case 2:     acc = k_double_sharp;         break;
                default:    acc = k_invalid_accidentals;  break;
            }
            break;
        }

        case -2:
        {
            switch(required)
            {
                case -2:    acc = k_no_accidentals;       break;
                case -1:    acc = k_natural_flat;         break;
                case 0:     acc = k_natural;              break;
                case 1:     acc = k_sharp;                break;
                case 2:     acc = k_double_sharp;         break;
                default:    acc = k_invalid_accidentals;  break;
            }
            break;
        }

        default:
            acc = k_invalid_accidentals;
    }

    //decide what accidentals to display. It must be always the computed accidentals but
    //there are two exceptions:
    //  1. when the user has set the accidentals and they are not inconsistent
    //  2. when courtesy accidentals are requested
    if (pNote->is_display_accidentals_forced())     //user has set accidentals
    {
        //acceptable inconsistencies: cases in which the user has provided alternative
        //notations with the same meaning (e.g.two sharps instead of double sharp)
        if (pNote->get_notated_accidentals() == k_natural_flat && acc == k_flat)
            acc = k_natural_flat;
        else if (pNote->get_notated_accidentals() == k_flat && acc == k_natural_flat)
            acc = k_flat;
        else if (pNote->get_notated_accidentals() == k_natural_sharp && acc == k_sharp)
            acc = k_natural_sharp;
        else if (pNote->get_notated_accidentals() == k_sharp_sharp && acc == k_double_sharp)
            acc = k_sharp_sharp;
        else if (pNote->get_notated_accidentals() == k_sharp && acc == k_natural_sharp)
            acc = k_sharp;

        //no accidental is required. Deal with cases the user has requested to display
        //the accidentals (courtesy accidentals)
        else if (acc == k_no_accidentals)
        {
            if (pNote->is_display_naturals_forced())
                acc = k_natural;

            else if (pNote->get_notated_accidentals() == k_natural)
            {
                acc = k_natural;
                //AWARE: notated_accidentals_never_computed() is "true" the first time the
                //score is loaded, when importing or re-loading the file. Score source
                //format is irrelevant.
                if (pNote->notated_accidentals_never_computed())
                    pNote->force_to_display_naturals();
            }
            else
            {
                //context == required. No accidental is required but user has set
                //the accidental (courtesy acc.)
                switch(context)
                {
                    case -2:    acc = k_flat_flat;            break;
                    case -1:    acc = k_flat;                 break;
                    case 0:     acc = k_no_accidentals;       break;
                    case 1:     acc = k_sharp;                break;
                    case 2:     acc = k_double_sharp;         break;
                    default:    acc = k_invalid_accidentals;  break;
                }
            }
        }
    }

    pNote->mark_notated_accidentals_as_computed();
    pNote->set_notated_accidentals(acc);
}

//---------------------------------------------------------------------------------------
void PitchAssigner::compute_pitch(ImoNote* pNote, int idx)
{
    float alter = pNote->get_actual_accidentals();
    int step = pNote->get_step();
    if (step == k_no_pitch)
    {
        pNote->set_actual_accidentals(k_acc_not_computed);
        pNote->set_notated_accidentals(k_no_accidentals);
    }
    else
    {
        int context = m_context[idx][step];
        bool fContextUpdated = false;
        if (alter == k_acc_not_computed)
        {
            update_context_accidentals(pNote, idx);
            fContextUpdated = true;
            pNote->set_actual_accidentals( float(m_context[idx][step]) );
        }

        compute_notated_accidentals(pNote, context);

        if (!fContextUpdated)
            m_context[idx][step] = int( round(alter) );
    }
}

//---------------------------------------------------------------------------------------
void PitchAssigner::reset_accidentals(ImoKeySignature* pKey, int idx)
{
    if (!pKey)
    {
        for (int i=0; i < 7; ++i)
            m_context[idx][i] = 0;
    }
    else
    {
        if(pKey->is_standard())
        {
            int keyType = pKey->get_key_type();
            int accidentals[7];
            KeyUtilities::get_accidentals_for_key(keyType, accidentals);
            for (int i=0; i < 7; ++i)
                m_context[idx][i] = accidentals[i];
        }
        else
        {
            for (int i=0; i < 7; ++i)
            {
                // Each element of the array refers to one step: 0=C, 1=D, 2=E, ...
                // & its value can be one of:
                //     0  = no accidental
                //    -1  = a flat
                //     1  = a sharp
                KeyAccidental& acc = pKey->get_accidental(i);
                if (is_equal_float(acc.alter, 0.0f) || is_equal_float(acc.alter, 1.0f)
                    || is_equal_float(acc.alter, -1.0f))
                {
                    m_context[idx][acc.step] = acc.alter;
                }
                else
                    m_context[idx][acc.step] = 0;
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void PitchAssigner::update_context_accidentals(ImoNote* pNote, int idx)
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
            m_context[idx][step] = 0;
            break;
        case k_flat:
            //Force one flat
            m_context[idx][step] = -1;
            break;
        case k_natural_flat:
            //Force one flat
            m_context[idx][step] = -1;
            break;
        case k_sharp:
            //force one sharp
            m_context[idx][step] = 1;
            break;
        case k_natural_sharp:
            //force one sharp
            m_context[idx][step] = 1;
            break;
        case k_flat_flat:
            //Force two flats
            m_context[idx][step] = -2;
            break;
        case k_sharp_sharp:
        case k_double_sharp:
            //force two sharps
            m_context[idx][step] = 2;
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
                    pMidi->init_midi_port(i / 16);
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
                    pMidi->init_midi_channel(i % 16);
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
                    pMidi->init_midi_port(idx / 16);
                    pMidi->init_midi_channel(idx % 16);
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
// PartIdAssigner implementation
//=======================================================================================
PartIdAssigner::PartIdAssigner()
{
}

//---------------------------------------------------------------------------------------
PartIdAssigner::~PartIdAssigner()
{
}

//---------------------------------------------------------------------------------------
void PartIdAssigner::assign_parts_id(ImoScore* pScore)
{
    list<long> ids;
    list<ImoInstrument*> instrs;    //instruments without partID
    ImoInstrument* pI = pScore->get_instrument(0);
    while (pI)
    {
        string partID = pI->get_instr_id();
        if (!partID.empty())
        {
            //check if partID is like "P###" and extract the number "###"
            if (partID.front() == 'P' && partID.size() > 1)
            {
                string number = partID.substr(1, partID.size() - 1);
                long nNumber;
                std::istringstream iss(number);
                if (!(iss >> std::dec >> nNumber).fail())
                    ids.push_back(nNumber);
            }
        }
        else
        {
            instrs.push_back(pI);
        }
        pI = static_cast<ImoInstrument*>(pI->get_next_sibling());
    }

    long number = 1L;
    list<ImoInstrument*>::iterator it;
    for (it=instrs.begin(); it != instrs.end(); ++it)
    {
        bool found = (std::find(ids.begin(), ids.end(), number) != ids.end());
        while (found)
        {
            ++number;
            found = (std::find(ids.begin(), ids.end(), number) != ids.end());
        }
        stringstream ss;
        ss << "P" << number++;
        (*it)->set_instr_id(ss.str());
    }
}


//=======================================================================================
// GroupBarlinesFixer implementation
//=======================================================================================
GroupBarlinesFixer::GroupBarlinesFixer()
{
}

//---------------------------------------------------------------------------------------
GroupBarlinesFixer::~GroupBarlinesFixer()
{
}

//---------------------------------------------------------------------------------------
void GroupBarlinesFixer::set_barline_layout_in_instruments(ImoScore* pScore)
{
    //restore default barlines in instruments
    ImoInstrument* pInstr = pScore->get_instrument(0);
    while (pInstr)
    {
        pInstr->set_barline_layout(ImoInstrument::k_isolated);
        pInstr = static_cast<ImoInstrument*>(pInstr->get_next_sibling());
    }

    //compute barlines layout for groups
    ImoInstrGroups* pGroups = pScore->get_instrument_groups();
    if (pGroups)
    {
        ImoObj::children_iterator itG;
        for (itG= pGroups->begin(); itG != pGroups->end(); ++itG)
        {
            ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(*itG);
            set_barlines_layout_for(pGrp);
        }
    }
}

//---------------------------------------------------------------------------------------
void GroupBarlinesFixer::set_barlines_layout_for(ImoInstrGroup* pGrp)
{
    if (pGrp->join_barlines() == EJoinBarlines::k_non_joined_barlines)
        return;

    int layout = (pGrp->join_barlines() == EJoinBarlines::k_joined_barlines
                    ? ImoInstrument::k_joined
                    : ImoInstrument::k_mensurstrich);

    int iFirst = pGrp->get_index_to_first_instrument();
    int iLast = pGrp->get_index_to_last_instrument();
    for (int i=iFirst; i <= iLast; ++i)
    {
        ImoInstrument* pInstr = pGrp->get_instrument(i - iFirst);
        if (i != iLast)
            pInstr->set_barline_layout(layout);
        else if (layout == ImoInstrument::k_mensurstrich)
            pInstr->set_barline_layout(ImoInstrument::k_nothing);
    }
}


//=======================================================================================
// MeasuresTableBuilder implementation
//=======================================================================================
void MeasuresTableBuilder::build(ImoScore* pScore)
{
    ColStaffObjs* pCSO = pScore->get_staffobjs_table();
    if (pCSO->num_entries() == 0)
        return;

    int numInstrs = pScore->get_num_instruments();
    m_tables.assign(numInstrs, nullptr);
    m_curMeasure.assign(numInstrs, nullptr);

    ColStaffObjsIterator it = pCSO->begin();
    while (it != pCSO->end())
    {
        ColStaffObjsEntry* pCsoEntry = *it;
        int iInstr = pCsoEntry->num_instrument();
        ImoStaffObj* pSO = pCsoEntry->imo_object();

        //if first entry for the instrument create measures table and first measure
        if (m_tables[iInstr] == nullptr)
        {
            ImoInstrument* pInstr = pScore->get_instrument(iInstr);
            start_measures_table_for(iInstr, pInstr, pCsoEntry);
        }

        //start new measure if no current measure or current object is for next measure
        if (m_curMeasure[iInstr] == nullptr
            || pCsoEntry->measure() > m_curMeasure[iInstr]->get_start_entry()->measure())
        {
            start_new_measure(iInstr, pCsoEntry);
        }

        //if Time Signature update beat duration
        if (pSO->is_time_signature())
        {
            ImoTimeSignature* pTS = static_cast<ImoTimeSignature*>(pSO);
            m_curMeasure[iInstr]->set_implied_beat_duration( pTS->get_beat_duration() );
            m_curMeasure[iInstr]->set_bottom_ts_beat_duration( pTS->get_ref_note_duration() );
        }

        //if not intermediate barline finish current measure
        if (pSO->is_barline()
            && pCsoEntry->measure() == m_curMeasure[iInstr]->get_start_entry()->measure())
        {
            ImoBarline* pBL = static_cast<ImoBarline*>(pSO);
            if (!pBL->is_middle())
                finish_current_measure(iInstr, pCsoEntry);
        }

        //advance to next entry
        ++it;
    }
}

//---------------------------------------------------------------------------------------
void MeasuresTableBuilder::start_measures_table_for(int iInstr, ImoInstrument* pInstr,
                                                    ColStaffObjsEntry* pCsoEntry)
{
    //create measures table
    m_tables[iInstr] = LOMSE_NEW ImMeasuresTable();
    pInstr->set_measures_table(m_tables[iInstr]);

    //add first measure
    m_curMeasure[iInstr] = m_tables[iInstr]->add_entry(pCsoEntry);
}

//---------------------------------------------------------------------------------------
void MeasuresTableBuilder::finish_current_measure(int iInstr, ColStaffObjsEntry* pEndEntry)
{
    m_curMeasure[iInstr]->set_end_entry(pEndEntry);

    //the next measure could already exits when it starts with clef, key or time
    //signature, as these objects are moved to previous measure and, in these cases,
    //they have been already processed.
    ImMeasuresTableEntry* pMeasure = m_tables[iInstr]->back();
    if (pMeasure->get_table_index() > m_curMeasure[iInstr]->get_table_index())
        m_curMeasure[iInstr] = pMeasure;
    else
        m_curMeasure[iInstr] = nullptr;
}

//---------------------------------------------------------------------------------------
void MeasuresTableBuilder::start_new_measure(int iInstr, ColStaffObjsEntry* pStartEntry)
{
    ImMeasuresTableEntry* prevMeasure = m_tables[iInstr]->back();
    ImMeasuresTableEntry* pMeasure = m_tables[iInstr]->add_entry(pStartEntry);

    if (prevMeasure != nullptr)
    {
        pMeasure->set_implied_beat_duration( prevMeasure->get_implied_beat_duration() );
        pMeasure->set_bottom_ts_beat_duration( prevMeasure->get_bottom_ts_beat_duration() );
    }

    if (m_curMeasure[iInstr] == nullptr)
        m_curMeasure[iInstr] = pMeasure;
}


}  //namespace lomse

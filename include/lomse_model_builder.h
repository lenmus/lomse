//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_MODEL_BUILDER_H__
#define __LOMSE_MODEL_BUILDER_H__

//lomse
#include "lomse_staffobjs_table.h"

//std library
#include <ostream>
#include <map>
#include <list>
#include <vector>
#include <array>

namespace lomse
{

//forward declarations
class ImoBarline;
class ImoDocument;
class ImoInstrument;
class ImoInstrGroup;
class ImoKeySignature;
class ImoNote;
class ImoObj;
class ImoScore;
class ImoSoundInfo;
class ImMeasuresTable;
class ImMeasuresTableEntry;


//---------------------------------------------------------------------------------------
// ModelBuilder. Implements the final step of LDP compiler: code generation.
// Traverses the parse tree and creates the internal model
class ModelBuilder
{
public:
    ModelBuilder() {}
    virtual ~ModelBuilder() {}

    ImoDocument* build_model(ImoDocument* pImoDoc);
    void structurize(ImoObj* pImo);

};

//---------------------------------------------------------------------------------------
/** PitchAssigner. Implements the algorithm to traverse the score and assign pitch to
    notes, based on notated pitch, and taking into account key signature and notated
    accidentals introduced by previous notes on the same measure.
*/
class PitchAssigner
{
protected:
    std::vector<std::array<int, 7> > m_context;

public:
    PitchAssigner() {}
    virtual ~PitchAssigner() {}

    void assign_pitch(ImoScore* pScore);

protected:
    void reset_accidentals(ImoKeySignature* pKey, int idx);
    void update_context_accidentals(ImoNote* pNote, int idx);
    void compute_pitch(ImoNote* pNote, int idx);
    void compute_notated_accidentals(ImoNote* pNote, int context);

};

//---------------------------------------------------------------------------------------
// MidiAssigner. Implements the algorithm to traverse the score instruments and assign
// midi channel and midi port pitch to the <sound-instruments>. It also ensures that
// all ImoInstruments have at least one ImoSoundInfo, creating one if not.
class MidiAssigner
{
protected:
    std::list<ImoSoundInfo*> m_sounds;
	std::list<std::string> m_ids;

public:
    MidiAssigner();
    virtual ~MidiAssigner();

	void assign_midi_data(ImoScore* pScore);

protected:
    void collect_sounds_info(ImoScore* pScore);
    void assign_score_instr_id();
    void assign_port_and_channel();
};

//---------------------------------------------------------------------------------------
// PartIdAssigner. Implements the algorithm to traverse the score instruments and assign
// a unique partID to any instrument not having partID
class PartIdAssigner
{
protected:

public:
    PartIdAssigner();
    virtual ~PartIdAssigner();

	void assign_parts_id(ImoScore* pScore);

protected:
};

//---------------------------------------------------------------------------------------
// GroupBarlinesFixer. Implements the algorithm for ensuring that barlines shared between
// all instruments in a group are are identified and marked as such.
class GroupBarlinesFixer
{
protected:

public:
    GroupBarlinesFixer();
    virtual ~GroupBarlinesFixer();

	void set_barline_layout_in_instruments(ImoScore* pScore);

protected:
    void set_barlines_layout_for(ImoInstrGroup* pGrp);
};


//---------------------------------------------------------------------------------------
// MeasuresTableBuilder. Implements the algorithm to traverse the ColStaffObjs table
// for creating the ImMeasuresTable for each instrument.
class MeasuresTableBuilder
{
protected:
    std::vector<ImMeasuresTable*> m_tables;             //table for each instrument
    std::vector<ImMeasuresTableEntry*> m_curMeasure;    //current measure in process, for each instrument

public:
    MeasuresTableBuilder() {}
    virtual ~MeasuresTableBuilder() {}

	void build(ImoScore* pScore);

protected:

    void start_measures_table_for(int iInstr, ImoInstrument* pInstr,
                                  ColStaffObjsEntry* pStartEntry);
    void finish_current_measure(int iInstr, ColStaffObjsEntry* pEndEntry=nullptr);
    void start_new_measure(int iInstr, ColStaffObjsEntry* pStartEntry);
};


}   //namespace lomse

#endif      //__LOMSE_MODEL_BUILDER_H__

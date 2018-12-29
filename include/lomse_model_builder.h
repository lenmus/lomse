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

#ifndef __LOMSE_MODEL_BUILDER_H__
#define __LOMSE_MODEL_BUILDER_H__

#include <ostream>

#include <map>
#include <list>
#include <vector>
#include <array>
using namespace std;

namespace lomse
{

//forward declarations
class ImoBarline;
class ImoDocument;
class ImoInstrument;
class ImoKeySignature;
class ImoNote;
class ImoObj;
class ImoScore;
class ImoSoundInfo;
class ColStaffObjsEntry;
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
    vector<std::array<int, 7> > m_context;

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
    list<ImoSoundInfo*> m_sounds;
	list<string> m_ids;

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
// MeasuresTableBuilder. Implements the algorithm to traverse the ColStaffObjs table
// and create the ImMeasuresTable for each instrument. If the measure entries already
// exist (they could have been created by importers, e.g. MusicXML, MNX) in these cases
// the algorithm just updates them to ensure they have valid content.
class MeasuresTableBuilder
{
protected:
    vector<ImoInstrument*> m_instruments;
    vector<ImMeasuresTableEntry*> m_measures;   //current open measures

public:
    MeasuresTableBuilder();
    virtual ~MeasuresTableBuilder();

	void build(ImoScore* pScore);

protected:
    void start_measures_table_for(int iInstr, ImoInstrument* pInstr,
                                  ColStaffObjsEntry* pCsoEntry);
    void finish_current_measure(int iInstr);
    void start_new_measure(int iInstr, ColStaffObjsEntry* pCsoEntry);
};


}   //namespace lomse

#endif      //__LOMSE_MODEL_BUILDER_H__

//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//  
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE__IM_NOTE_H__        //to avoid nested includes
#define __LOMSE__IM_NOTE_H__

#include "lomse_internal_model.h"

using namespace std;

namespace lomse
{

class DtoNoteRest;
class DtoNote;
class DtoRest;


//----------------------------------------------------------------------------------
class ImoNoteRest : public ImoStaffObj
{
protected:
    int     m_nNoteType;
    int     m_nDots;
    float   m_rDuration;
    int     m_nVoice;
    ImoBeamInfo m_beamInfo;
    ImoBeam* m_pBeam;
    ImoTuplet* m_pTuplet;

public:
    ImoNoteRest(int objtype) : ImoStaffObj(objtype) {}
    ImoNoteRest(int objtype, DtoNoteRest& dto);
    ImoNoteRest(long id, int objtype, int nNoteType, float rDuration, int nDots, int nStaff,
               int nVoice, bool fVisible, ImoBeamInfo* pBeamInfo);
    virtual ~ImoNoteRest() {}

    enum    { k_unknown=-1, k_longa=0, k_breve=1, k_whole=2, k_half=3, k_quarter=4,
              k_eighth=5, k_16th=6, k_32th=7, k_64th=8, k_128th=9, k_256th=10, };

    //getters
    inline int get_note_type() { return m_nNoteType; }
    inline float get_duration() { return m_rDuration; }
    inline int get_dots() { return m_nDots; }
    inline int get_voice() { return m_nVoice; }

    //beam
    inline void set_beam(ImoBeam* pBeam) { m_pBeam = pBeam; }
    inline bool is_beamed() { return m_pBeam != NULL; }
    void set_beam_type(int level, int type);
    inline ImoBeamInfo* get_beam_info() { return &m_beamInfo; }
    int get_beam_type(int level);

    //tuplet
    inline void set_tuplet(ImoTuplet* pTuplet) { m_pTuplet = pTuplet; }
    inline bool is_in_tuplet() { return m_pTuplet != NULL; }

    //setters
    //inline void set_note_type(int noteType) { m_nNoteType = noteType; }
    //inline void set_duration(float duration) { m_rDuration = duration; }
    //inline void get_dots(int dots) { m_nDots = dots; }
    //inline void set_voice(int voice) { m_nVoice = voice; }
    //void set_note_type_and_dots(int noteType, int dots);

};

//----------------------------------------------------------------------------------
class ImoRest : public ImoNoteRest
{
protected:

public:
    ImoRest();
    ImoRest(DtoRest& dto);
    ImoRest(long id, int nNoteType, float rDuration, int nDots, int nStaff,
           int nVoice, bool fVisible = true, bool fBeamed = false,
           ImoBeamInfo* pBeamInfo = NULL);

    virtual ~ImoRest() {}

};

//----------------------------------------------------------------------------------
class ImoNote : public ImoNoteRest
{
protected:
    int     m_step;
    int     m_octave;
    int     m_accidentals;
    int     m_stemDirection;
    bool    m_inChord;
    ImoTie* m_pTieNext;
    ImoTie* m_pTiePrev;


public:
    ImoNote();
    ImoNote(DtoNote& dto);
    ImoNote(long id, int nNoteType, float rDuration, int nDots, int nStaff, int nVoice,
           bool fVisible, bool fBeamed, ImoBeamInfo* pBeamInfo);
    ~ImoNote();

    enum    { C=0, D=1, E=2, F=3, G=4, A=5, B=6, last=B, NoPitch=-1, };     //steps
    enum    { k_no_accidentals=0, k_sharp, k_sharp_sharp, k_double_sharp, k_natural_sharp,
              k_flap, k_flat_flat, k_natural_flat, k_natural, };
    enum    { k_default=0, k_up, k_down, };

    //pitch
    inline int get_step() { return m_step; }
    inline int get_octave() { return m_octave; }
    inline int get_accidentals() { return m_accidentals; }
//    inline void set_step(int step) { m_step = step; }
//    inline void set_octave(int octave) { m_octave = octave; }
//    inline void set_accidentals(int accidentals) { m_accidentals = accidentals; }
//    inline void set_pitch(int step, int octave, int accidentals) {
//        m_step = step;
//        m_octave = octave;
//        m_accidentals = accidentals;
//    }

    //ties
    inline bool is_tied_next() { return m_pTieNext != NULL; }
    inline bool is_tied_prev() { return m_pTiePrev != NULL; }
    inline void set_tie_next(ImoTie* pStartTie) { m_pTieNext = pStartTie; }
    inline void set_tie_prev(ImoTie* pEndTie) { m_pTiePrev = pEndTie; }
    //inline ImoTie* get_tie_next() { return m_pTieNext; }
    //inline ImoTie* get_tie_prev() { return m_pTiePrev; }
    void remove_tie(ImoTie* pTie);

    //stem
//    inline void set_stem_direction(int value) { m_stemDirection = value; }
    inline int get_stem_direction() { return m_stemDirection; }
    inline bool is_stem_up() { return m_stemDirection == k_up; }
    inline bool is_stem_down() { return m_stemDirection == k_down; }
    inline bool is_stem_default() { return m_stemDirection == k_default; }

    //in chord
    inline void set_in_chord(bool value) { m_inChord = value; }
    inline bool is_in_chord() { return m_inChord; }


};


}   //namespace lomse

#endif    // __LOMSE__IM_NOTE_H__


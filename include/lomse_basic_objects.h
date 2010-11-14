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
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_BASIC_OBJECTS_H__        //to avoid nested includes
#define __LOMSE_BASIC_OBJECTS_H__

#include <string>
#include <list>
#include <vector>
#include "lomse_visitor.h"
#include "lomse_basic.h"

using namespace std;


namespace lomse
{

//----------------------------------------------------------------------------------
class DtoObj
{
protected:
    long m_id;
    int m_objtype;

public:
    DtoObj() : m_id(-1L), m_objtype(0) {}
    DtoObj(long id, int objtype) : m_id(id), m_objtype(objtype) {}
    virtual ~DtoObj() {}


    //getters
    inline long get_id() { return m_id; }

    //setters
    inline void set_id(long id) { m_id = id; }

};

//----------------------------------------------------------------------------------
class DtoDocObj : public DtoObj
{
protected:
    Tenths m_txUserLocation;
    Tenths m_tyUserLocation;

public:
    DtoDocObj() : DtoObj(), m_txUserLocation(0.0f), m_tyUserLocation(0.0f) {}
    virtual ~DtoDocObj() {}

    //getters
    inline Tenths get_user_location_x() { return m_txUserLocation; }
    inline Tenths get_user_location_y() { return m_tyUserLocation; }

    //setters
    inline void set_user_location_x(Tenths tx) { m_txUserLocation = tx; }
    inline void set_user_location_y(Tenths ty) { m_tyUserLocation = ty; }
};

//----------------------------------------------------------------------------------
class DtoComponentObj : public DtoDocObj
{
protected:
    bool m_fVisible;
    Color m_color;

public:
    DtoComponentObj() : DtoDocObj(), m_fVisible(true), m_color(0,0,0,255) {}
    virtual ~DtoComponentObj() {}

    //getters
    inline bool is_visible() { return m_fVisible; }
    inline Color& get_color() { return m_color; }

    //setters
    inline void set_visible(bool visible) { m_fVisible = visible; }
    void set_color(Color color);
};

//----------------------------------------------------------------------------------
class DtoStaffObj : public DtoComponentObj
{
protected:
    int m_staff;

public:
    DtoStaffObj() : DtoComponentObj(), m_staff(0) {}
    virtual ~DtoStaffObj() {}

    //getters
    inline int get_staff() { return m_staff; }

    //setters
    virtual void set_staff(int staff) { m_staff = staff; }
};

//----------------------------------------------------------------------------------
class DtoAuxObj : public DtoComponentObj
{
public:
    DtoAuxObj() : DtoComponentObj() {}
    virtual ~DtoAuxObj() {}

};

//----------------------------------------------------------------------------------
class DtoBarline : public DtoStaffObj
{
protected:
    int m_barlineType;

public:
    DtoBarline(int barlineType);
    ~DtoBarline() {}

    //getters and setters
    inline int get_barline_type() { return m_barlineType; }
    inline void set_barline_type(int type) { m_barlineType = type; }

    //overrides: barlines always in staff 0
    void set_staff(int staff) { m_staff = 0; }

};

//----------------------------------------------------------------------------------
class DtoClef : public DtoStaffObj
{
protected:
    int m_clefType;

public:
    DtoClef(int type) : DtoStaffObj(), m_clefType(type) {}
    ~DtoClef() {}

    //getters and setters
    inline int get_clef_type() { return m_clefType; }
    inline void set_clef_type(int type) { m_clefType = type; }

};

//----------------------------------------------------------------------------------
class DtoFermata : public DtoAuxObj
{
protected:
    int m_placement;
    int m_symbol;

public:
    DtoFermata();
    ~DtoFermata() {}

    //getters
    inline int get_placement() { return m_placement; }
    inline int get_symbol() { return m_symbol; }

    //setters
    inline void set_placement(int placement) { m_placement = placement; }
    inline void set_symbol(int symbol) { m_symbol = symbol; }

};

//----------------------------------------------------------------------------------
class DtoGoBackFwd : public DtoStaffObj
{
protected:
    bool    m_fFwd;
    float   m_rTimeShift;

    const float SHIFT_START_END;     //any too big value

public:
    DtoGoBackFwd(bool fFwd) : DtoStaffObj(), m_fFwd(fFwd), m_rTimeShift(0.0f),
                             SHIFT_START_END(100000000.0f) {}
    ~DtoGoBackFwd() {}

    //getters and setters
    inline bool is_forward() { return m_fFwd; }
    inline bool is_to_start() { return !m_fFwd && (m_rTimeShift == -SHIFT_START_END); }
    inline bool is_to_end() { return m_fFwd && (m_rTimeShift == SHIFT_START_END); }
    inline float get_time_shift() { return m_rTimeShift; }
    inline void set_to_start() { set_time_shift(SHIFT_START_END); }
    inline void set_to_end() { set_time_shift(SHIFT_START_END); }
    inline void set_time_shift(float rTime) { m_rTimeShift = (m_fFwd ? rTime : -rTime); }
};

//----------------------------------------------------------------------------------
class DtoKeySignature : public DtoStaffObj
{
protected:
    int m_keyType;

public:
    DtoKeySignature(int type) : DtoStaffObj() , m_keyType(type) {}
    ~DtoKeySignature() {}

    //getters and setters
    inline int get_key_type() { return m_keyType; }
    inline void set_key_type(int type) { m_keyType = type; }

    //overrides: key signatures always in staff 0
    void set_staff(int staff) { m_staff = 0; }

};

//----------------------------------------------------------------------------------
class DtoMetronomeMark : public DtoStaffObj
{
protected:
    int     m_markType;
    int     m_ticksPerMinute;
    int     m_leftNoteType;
    int     m_leftDots;
    int     m_rightNoteType;
    int     m_rightDots;
    bool    m_fParenthesis;

public:
    DtoMetronomeMark();
    ~DtoMetronomeMark() {}

    //getters
    inline int get_left_note_type() { return m_leftNoteType; }
    inline int get_right_note_type() { return m_rightNoteType; }
    inline int get_left_dots() { return m_leftDots; }
    inline int get_right_dots() { return m_rightDots; }
    inline int get_ticks_per_minute() { return m_ticksPerMinute; }
    inline int get_mark_type() { return m_markType; }
    inline bool has_parenthesis() { return m_fParenthesis; }

    //setters
    inline void set_left_note_type(int noteType) { m_leftNoteType = noteType; }
    inline void set_right_note_type(int noteType) { m_rightNoteType = noteType; }
    inline void set_left_dots(int dots) { m_leftDots = dots; }
    inline void set_right_dots(int dots) { m_rightDots = dots; }
    inline void set_ticks_per_minute(int ticks) { m_ticksPerMinute = ticks; }
    inline void set_mark_type(int type) { m_markType = type; }
    inline void set_parenthesis(bool fValue) { m_fParenthesis = fValue; }

};

//----------------------------------------------------------------------------------
class DtoNoteRest : public DtoStaffObj
{
protected:
    int     m_nNoteType;
    int     m_nDots;
    int     m_nVoice;

public:
    DtoNoteRest();
    virtual ~DtoNoteRest() {}

    //getters
    inline int get_note_type() { return m_nNoteType; }
    inline int get_dots() { return m_nDots; }
    inline int get_voice() { return m_nVoice; }

    //setters
    inline void set_note_type(int noteType) { m_nNoteType = noteType; }
    inline void get_dots(int dots) { m_nDots = dots; }
    inline void set_voice(int voice) { m_nVoice = voice; }
    void set_note_type_and_dots(int noteType, int dots);

};

//----------------------------------------------------------------------------------
class DtoRest : public DtoNoteRest
{
protected:

public:
    DtoRest() : DtoNoteRest() {}
    ~DtoRest() {}

};

//----------------------------------------------------------------------------------
class DtoNote : public DtoNoteRest
{
protected:
    int     m_step;
    int     m_octave;
    int     m_accidentals;
    int     m_stemDirection;
    bool    m_inChord;


public:
    DtoNote();
    ~DtoNote() {}

    //pitch
    inline int get_step() { return m_step; }
    inline int get_octave() { return m_octave; }
    inline int get_accidentals() { return m_accidentals; }
    inline void set_step(int step) { m_step = step; }
    inline void set_octave(int octave) { m_octave = octave; }
    inline void set_accidentals(int accidentals) { m_accidentals = accidentals; }
    inline void set_pitch(int step, int octave, int accidentals) {
        m_step = step;
        m_octave = octave;
        m_accidentals = accidentals;
    }

    //stem
    inline void set_stem_direction(int value) { m_stemDirection = value; }
    inline int get_stem_direction() { return m_stemDirection; }

    //in chord
    inline void set_in_chord(bool value) { m_inChord = value; }
    inline bool is_in_chord() { return m_inChord; }


};

//----------------------------------------------------------------------------------
class DtoSpacer : public DtoStaffObj
{
protected:
    Tenths  m_space;

public:
    DtoSpacer(Tenths space=0.0f) : DtoStaffObj(), m_space(space) {}
    ~DtoSpacer() {}

    //getters
    inline Tenths get_width() { return m_space; }

    //setters
    inline void set_width(Tenths space) { m_space = space; }

};

//----------------------------------------------------------------------------------
class DtoTimeSignature : public DtoStaffObj
{
protected:
    int     m_beats;
    int     m_beatType;

public:
    DtoTimeSignature() : DtoStaffObj() , m_beats(2) , m_beatType(4) {}
    ~DtoTimeSignature() {}

    //getters and setters
    inline int get_beats() { return m_beats; }
    inline void set_beats(int beats) { m_beats = beats; }
    inline int get_beat_type() { return m_beatType; }
    inline void set_beat_type(int beatType) { m_beatType = beatType; }

    //overrides: time signatures always in staff 0
    void set_staff(int staff) { m_staff = 0; }

};


}   //namespace lomse

#endif    // __LOMSE_BASIC_OBJECTS_H__


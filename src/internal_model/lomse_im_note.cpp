//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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
//---------------------------------------------------------------------------------------

#include "lomse_im_note.h"

#include <math.h>               //pow
#include "lomse_basic_objects.h"

namespace lomse
{


//=======================================================================================
// ImoNoteRest implementation
//=======================================================================================
ImoNoteRest::ImoNoteRest(int objtype, DtoNoteRest& dto)
    : ImoStaffObj(objtype, dto)
    , m_nNoteType( dto.get_note_type() )
    , m_nDots( dto.get_dots() )
    , m_nVoice( dto.get_voice() )
{
}

//---------------------------------------------------------------------------------------
int ImoNoteRest::get_beam_type(int level)
{
    ImoBeam* pBeam = static_cast<ImoBeam*>( find_attachment(ImoObj::k_beam) );
    if (pBeam)
    {
        ImoBeamData* pData = dynamic_cast<ImoBeamData*>( pBeam->get_data_for(this) );
        return pData->get_beam_type(level);
    }
    else
        return ImoBeam::k_none;
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_beam_type(int level, int type)
{
    ImoBeamData* pData = static_cast<ImoBeamData*>( find_reldataobj(ImoObj::k_beam_data) );
    pData->set_beam_type(level, type);
}

//---------------------------------------------------------------------------------------
ImoBeam* ImoNoteRest::get_beam()
{
    return static_cast<ImoBeam*>( find_attachment(ImoObj::k_beam) );
}

//---------------------------------------------------------------------------------------
bool ImoNoteRest::is_end_of_beam()
{
    ImoBeam* pBeam = static_cast<ImoBeam*>( find_attachment(ImoObj::k_beam) );
    if (pBeam)
    {
        ImoBeamData* pData = dynamic_cast<ImoBeamData*>( pBeam->get_data_for(this) );
        return pData->is_end_of_beam();
    }
    else
        return ImoBeam::k_none;
}

//---------------------------------------------------------------------------------------
bool ImoNoteRest::is_in_tuplet()
{
    return find_attachment(ImoObj::k_tuplet) != NULL;
}

//---------------------------------------------------------------------------------------
ImoTuplet* ImoNoteRest::get_tuplet()
{
    return static_cast<ImoTuplet*>( find_attachment(ImoObj::k_tuplet) );
}

//---------------------------------------------------------------------------------------
float ImoNoteRest::get_duration()
{
    float rTime = to_duration(m_nNoteType, m_nDots);
    ImoTuplet* pTuplet = get_tuplet();
    if (pTuplet)
    {
        rTime *= float(pTuplet->get_normal_number());
        rTime /= float(pTuplet->get_actual_number());
    }
    return rTime;
}


//=======================================================================================
// ImoRest implementation
//=======================================================================================
ImoRest::ImoRest()
    : ImoNoteRest(ImoObj::k_rest)
{
}

//---------------------------------------------------------------------------------------
ImoRest::ImoRest(DtoRest& dto)
    : ImoNoteRest(ImoObj::k_rest, dto)
{
}


//=======================================================================================
// ImoNote implementation
//=======================================================================================
ImoNote::ImoNote()
    : ImoNoteRest(ImoObj::k_note)
    , m_step(k_no_pitch)
    , m_octave(4)
    , m_accidentals(k_no_accidentals)
    , m_stemDirection(k_stem_default)
    , m_pTieNext(NULL)
    , m_pTiePrev(NULL)
    , m_pChord(NULL)
{
}

//---------------------------------------------------------------------------------------
ImoNote::ImoNote(DtoNote& dto)
    : ImoNoteRest(ImoObj::k_note, dto)
    , m_step( dto.get_step() )
    , m_octave( dto.get_octave() )
    , m_accidentals( dto.get_accidentals() )
    , m_stemDirection( dto.get_stem_direction() )
    , m_pTieNext(NULL)
    , m_pTiePrev(NULL)
    , m_pChord(NULL)
{
}

//---------------------------------------------------------------------------------------
ImoNote::ImoNote(int step, int octave, int noteType, int accidentals, int dots,
                 int staff, int voice, int stem)
    : ImoNoteRest(ImoObj::k_note)
    , m_step(step)
    , m_octave(octave)
    , m_accidentals(accidentals)
    , m_stemDirection(stem)
    , m_pTieNext(NULL)
    , m_pTiePrev(NULL)
    , m_pChord(NULL)
{
    m_nNoteType = noteType;
    m_nDots = dots;
    m_nVoice = voice;
    m_staff = staff;
}

//---------------------------------------------------------------------------------------
ImoNote::~ImoNote()
{
}

//---------------------------------------------------------------------------------------
bool ImoNote::is_in_chord()
{
    return get_chord() != NULL;
}

//---------------------------------------------------------------------------------------
ImoChord* ImoNote::get_chord()
{
    return static_cast<ImoChord*>( find_attachment(ImoObj::k_chord) );
}

//---------------------------------------------------------------------------------------
bool ImoNote::is_start_of_chord()
{
    ImoChord* pChord = get_chord();
    return pChord && dynamic_cast<ImoNote*>(pChord->get_start_object()) == this;
}

//---------------------------------------------------------------------------------------
bool ImoNote::is_end_of_chord()
{
    ImoChord* pChord = get_chord();
    return pChord && pChord->get_end_object() == this;
}



}  //namespace lomse

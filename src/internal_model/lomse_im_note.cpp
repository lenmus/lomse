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
    , m_rDuration( to_duration(m_nNoteType, m_nDots) )
    , m_nVoice( dto.get_voice() )
    , m_pBeam(NULL)
    , m_pTuplet(NULL)
{
}

//---------------------------------------------------------------------------------------
ImoNoteRest::ImoNoteRest(long id, int objtype, int nNoteType, float rDuration, int nDots,
               int nStaff, int nVoice, bool fVisible, ImoBeamInfo* pBeamInfo)
    : ImoStaffObj(id, objtype)
    , m_nNoteType(nNoteType)
    , m_nDots(nDots)
    , m_rDuration(rDuration)
    , m_nVoice(nVoice)
    , m_beamInfo(*pBeamInfo)
    , m_pBeam(NULL)
    , m_pTuplet(NULL)
{
    set_staff(nStaff);
}

//---------------------------------------------------------------------------------------
int ImoNoteRest::get_beam_type(int level)
{
    return m_beamInfo.get_beam_type(level);
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_beam_type(int level, int type)
{
    m_beamInfo.set_beam_type(level, type);
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

//---------------------------------------------------------------------------------------
ImoRest::ImoRest(long id, int nNoteType, float rDuration, int nDots, int nStaff,
               int nVoice, bool fVisible, bool fBeamed, ImoBeamInfo* pBeamInfo)
    : ImoNoteRest(id, ImoObj::k_rest, nNoteType, rDuration, nDots,
                 nStaff, nVoice, fVisible, pBeamInfo)
{
}


//=======================================================================================
// ImoNote implementation
//=======================================================================================
ImoNote::ImoNote()
    : ImoNoteRest(ImoObj::k_note)
    , m_step(k_no_pitch)
    , m_octave(4)
    , m_accidentals(ImoNote::k_no_accidentals)
    , m_stemDirection(ImoNote::k_stem_default)
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
ImoNote::ImoNote(long id, int nNoteType, float rDuration, int nDots, int nStaff,
                 int nVoice, bool fVisible, bool fBeamed, ImoBeamInfo* pBeamInfo)
    : ImoNoteRest(id, ImoObj::k_note, nNoteType, rDuration, nDots, nStaff, nVoice,
                 fVisible, pBeamInfo)
    , m_step(k_step_C)
    , m_octave(4)
    , m_accidentals(ImoNote::k_no_accidentals)
    , m_stemDirection(ImoNote::k_stem_default)
    , m_pTieNext(NULL)
    , m_pTiePrev(NULL)
    , m_pChord(NULL)
{
}

//---------------------------------------------------------------------------------------
ImoNote::~ImoNote()
{
    if (m_pTiePrev)
        remove_tie(m_pTiePrev);
    if (m_pTieNext)
        remove_tie(m_pTieNext);
}

//---------------------------------------------------------------------------------------
void ImoNote::remove_tie(ImoTie* pTie)
{
    ImoNote* pStartNote = pTie->get_start_note();
    ImoNote* pEndNote = pTie->get_end_note();
    pStartNote->set_tie_next( NULL );
    pEndNote->set_tie_prev( NULL );

    //tie is included as an attached object. Remove and delete it
    pStartNote->remove_attachment(pTie);
    pEndNote->remove_attachment(pTie);
    //delete pTie;      //tie is deleted when deleting attached objects
}

//---------------------------------------------------------------------------------------
bool ImoNote::is_start_of_chord()
{
    return m_pChord && dynamic_cast<ImoNote*>(m_pChord->get_start_object()) == this;
}

//---------------------------------------------------------------------------------------
bool ImoNote::is_end_of_chord()
{
    return m_pChord && m_pChord->get_end_object() == this;
}



}  //namespace lomse

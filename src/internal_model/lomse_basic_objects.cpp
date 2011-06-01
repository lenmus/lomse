//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include "lomse_basic_objects.h"

#include <algorithm>
#include "lomse_internal_model.h"

using namespace std;

namespace lomse
{


//-------------------------------------------------------------------------------------
// DtoScoreObj implementation
//-------------------------------------------------------------------------------------

void DtoScoreObj::set_color(Color color)
{
    m_color = color;
}


//-------------------------------------------------------------------------------------
// DtoBarline implementation
//-------------------------------------------------------------------------------------

DtoBarline::DtoBarline(int barlineType)
    : DtoStaffObj()
    , m_barlineType(barlineType)
{
}

//-------------------------------------------------------------------------------------
// DtoFermata implementation
//-------------------------------------------------------------------------------------

DtoFermata::DtoFermata()
    : DtoAuxObj()
    , m_placement(k_placement_default)
    , m_symbol(ImoFermata::k_normal)
{
}

//-------------------------------------------------------------------------------------
// DtoMetronomeMark implementation
//-------------------------------------------------------------------------------------

DtoMetronomeMark::DtoMetronomeMark()
    : DtoStaffObj()
    , m_markType(ImoMetronomeMark::k_value)
    , m_ticksPerMinute(60)
    , m_leftNoteType(0)
    , m_leftDots(0)
    , m_rightNoteType(0)
    , m_rightDots(0)
    , m_fParenthesis(false)
{
}


//-------------------------------------------------------------------------------------
// DtoNoteRest implementation
//-------------------------------------------------------------------------------------

DtoNoteRest::DtoNoteRest()
    : DtoStaffObj()
    , m_nDots(0)
    , m_nVoice(1)
{
}

void DtoNoteRest::set_note_type_and_dots(int noteType, int dots)
{
    m_nNoteType = noteType;
    m_nDots = dots;
}


//-------------------------------------------------------------------------------------
// DtoNote implementation
//-------------------------------------------------------------------------------------

DtoNote::DtoNote()
    : DtoNoteRest()
    , m_step(-1)        //-1 == k_no_pitch
    , m_octave(4)
    , m_accidentals(0)
    , m_stemDirection(0)
    , m_inChord(false)
{
}


}  //namespace lomse

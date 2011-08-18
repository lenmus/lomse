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

#ifndef __LOMSE_SORE_UTILITIES_H__
#define __LOMSE_SORE_UTILITIES_H__

#include "lomse_gm_basic.h"


//---------------------------------------------------------------------------------------
// This module contains a collection of global methods for auxiliary computations
// related to scores
//---------------------------------------------------------------------------------------

namespace lomse
{

//forward declarations
class ImoTimeSignature;


//---------------------------------------------------------------------------------------
// Determines if a timePos is in on-beat, off-beat position
enum { k_off_beat = -1, };
extern int get_beat_position(float timePos, ImoTimeSignature* pTS);

//---------------------------------------------------------------------------------------
// Key signature related functions
extern void get_accidentals_for_key(int keyType, int nAccidentals[]);


}   //namespace lomse

#endif      //__LOMSE_SORE_UTILITIES_H__

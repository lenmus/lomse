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

#ifndef __LOMSE_BARLINE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_BARLINE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoBarline;
class GmoBoxSliceInstr;
class GmoShape;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class BarlineEngraver : public Engraver
{
protected:
    int m_nBarlineType;

public:
    BarlineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~BarlineEngraver() {}

    GmoShape* create_shape(ImoBarline* pBarline, int iInstr, LUnits xPos, LUnits yTop,
                           LUnits yBottom);
};


}   //namespace lomse

#endif    // __LOMSE_BARLINE_ENGRAVER_H__


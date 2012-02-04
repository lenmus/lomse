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

#ifndef __LOMSE_LINE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_LINE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoScoreLine;
class GmoShape;
class ScoreMeter;
class GmoShapeLine;

//---------------------------------------------------------------------------------------
class LineEngraver : public Engraver
{
protected:
    int m_iInstr;
    int m_iStaff;
    ImoScoreLine* m_pLine;
    GmoShape* m_pParentShape;
    GmoShapeLine* m_pLineShape;

public:
    LineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~LineEngraver() {}

    GmoShapeLine* create_shape(ImoScoreLine* pLine, int iInstr, int iStaff,
                               UPoint pos, GmoShape* pParentShape=NULL);

protected:
    LUnits tenths_to_logical(Tenths tenths);
    UPoint compute_location(UPoint pos);

};


}   //namespace lomse

#endif    // __LOMSE_LINE_ENGRAVER_H__


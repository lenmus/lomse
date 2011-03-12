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

#ifndef __LOMSE_CLEF_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_CLEF_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_score_enums.h"

namespace lomse
{

//forward declarations
class ImoClef;
class GmoBoxSliceInstr;
class GmoShape;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class ClefEngraver : public Engraver
{
protected:
    int m_iInstr;
    int m_iStaff;
    int m_nClefType;
    int m_symbolSize;
    int m_iGlyph;

public:
    ClefEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~ClefEngraver() {}

    GmoShape* create_shape(ImoObj* pCreatorImo, int iInstr, int iStaff, UPoint uPos,
                           int clefType, int m_symbolSize=k_size_full);

protected:
    int find_glyph();
    double determine_font_size();
    Tenths get_glyph_offset();
};


}   //namespace lomse

#endif    // __LOMSE_CLEF_ENGRAVER_H__


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

#ifndef __LOMSE_TIME_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_TIME_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class GmoShapeTimeSignature;
class ImoObj;
class GmoShape;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class TimeEngraver : public Engraver
{
protected:
    GmoShapeTimeSignature* m_pTimeShape;
    int m_iInstr;
    int m_iStaff;
    UPoint m_uPos;
    LUnits m_uTopWidth;
    LUnits m_uBottomWidth;
    GmoShape* m_pShapesTop[2];
    GmoShape* m_pShapesBottom[2];
    double m_fontSize;
    ImoObj* m_pCreatorImo;

public:
    TimeEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~TimeEngraver() {}

    GmoShape* create_shape_normal(ImoObj* pCreatorImo, int iInstr, int iStaff,
                                  UPoint uPos, int beats, int beat_type);
    GmoShape* create_shape_common(ImoObj* pCreatorImo, int iInstr, int iStaff, UPoint uPos);
    GmoShape* create_shape_cut(ImoObj* pCreatorImo, int iInstr, int iStaff, UPoint uPos);

protected:
    void create_main_container_shape(UPoint uPos);
    void create_top_digits(UPoint uPos, int beats);
    void create_bottom_digits(UPoint uPos, int beat_type);
    void center_numbers();
    void add_all_shapes_to_container();
    void create_digits(int digits, GmoShape* pShape[]);
    GmoShape* create_digit(int digit);
    double determine_font_size();

};


}   //namespace lomse

#endif    // __LOMSE_TIME_ENGRAVER_H__


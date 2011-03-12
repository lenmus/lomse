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

#ifndef __LOMSE_ACCIDENTALS_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_ACCIDENTALS_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class GmoShapeAccidentals;
class GmoShape;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class AccidentalsEngraver : public Engraver
{
protected:
    int m_accidentals;
    bool m_fCautionary;
    int m_iInstr;
    int m_iStaff;
    GmoShapeAccidentals* m_pContainer;
    double m_fontSize;
    ImoObj* m_pCreatorImo;

public:
    AccidentalsEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~AccidentalsEngraver() {}

    GmoShapeAccidentals* create_shape(ImoObj* pCreatorImo, int iInstr, int iStaff,
                                      UPoint uPos, int accidentals, 
                                      bool fCautionary=false);

protected:
    void find_glyphs();
    void create_container_shape(ImoObj* pCreatorImo, UPoint pos);
    void add_glyphs_to_container_shape(UPoint pos);
    LUnits glyph_offset(int iGlyph);
    double determine_font_size();

    int m_glyphs[4];
    int m_numGlyphs;
};


}   //namespace lomse

#endif    // __LOMSE_ACCIDENTALS_ENGRAVER_H__


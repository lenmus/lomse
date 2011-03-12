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

#ifndef __LOMSE_KEY_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_KEY_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoKeySignature;
class GmoShape;
class GmoShapeKeySignature;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class KeyEngraver : public Engraver
{
protected:
    GmoShapeKeySignature* m_pKeyShape;
    int m_nKeyType;
    int m_iInstr;
    int m_iStaff;
    Tenths m_tPos[8];           //sharps/flats positions, in order of appearance
    double m_fontSize;
    ImoObj* m_pCreatorImo;

public:
    KeyEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~KeyEngraver() {}

    GmoShape* create_shape(ImoKeySignature* pKey, int iInstr, int iStaff,
                           int clefType, UPoint uPos);

protected:
    void compute_positions_for_flats(int clefType);
    void compute_positions_for_sharps(int clefType);
    void add_accidentals(int numAccidentals, int iGlyph, UPoint uPos);
    int get_num_fifths(int keyType);
    double determine_font_size();

};


}   //namespace lomse

#endif    // __LOMSE_KEY_ENGRAVER_H__

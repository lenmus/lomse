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

#include "lomse_key_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_glyphs.h"
#include "lomse_shapes.h"
#include "lomse_score_meter.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// KeyEngraver implementation
//---------------------------------------------------------------------------------------
KeyEngraver::KeyEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : Engraver(libraryScope, pScoreMeter)
    , m_pKeyShape(NULL)
{
}

//---------------------------------------------------------------------------------------
GmoShape* KeyEngraver::create_shape(ImoKeySignature* pKey, int iInstr, int iStaff,
                                    int clefType, UPoint uPos)
{
    m_pCreatorImo = pKey;
    m_nKeyType = pKey->get_key_type();
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_fontSize = determine_font_size();

    //create the container shape object
    int idx = 0;
    m_pKeyShape = new GmoShapeKeySignature(pKey, idx, uPos, Color(0,0,0), m_libraryScope);
    //m_pKeyShape->SetShapeLevel(lm_eMainShape);

    int numAccidentals = get_num_fifths(m_nKeyType);
    if (numAccidentals == 0)
        return m_pKeyShape;

    else if (numAccidentals > 0)
    {
        compute_positions_for_sharps(clefType);
        add_accidentals(numAccidentals, k_glyph_sharp_accidental, uPos);
    }
    else
    {
        compute_positions_for_flats(clefType);
        add_accidentals(-numAccidentals, k_glyph_flat_accidental, uPos);
    }
    return m_pKeyShape;
}

//---------------------------------------------------------------------------------------
void KeyEngraver::add_accidentals(int numAccidentals, int iGlyph, UPoint uPos)
{
    //LUnits width = 0;
    LUnits x = uPos.x;
    for (int i=1; i <= numAccidentals; i++)
    {
        Tenths yOffset = glyphs_lmbasic2[iGlyph].GlyphOffset + m_tPos[i] + 40.0f;
        LUnits y = uPos.y + m_pMeter->tenths_to_logical(yOffset, m_iInstr, m_iStaff);
        GmoShape* pSA = new GmoShapeAccidental(m_pCreatorImo, 0, iGlyph, UPoint(x, y),
                                               Color(0,0,0), m_libraryScope, m_fontSize);
        m_pKeyShape->add(pSA);
        x += pSA->get_width();
        //width += pSA->get_width();
    }
}

//---------------------------------------------------------------------------------------
void KeyEngraver::compute_positions_for_sharps(int clefType)
{
    switch(clefType)
    {
        case ImoClef::k_G2:
        case ImoClef::k_8_G2:
        case ImoClef::k_G2_8:
        case ImoClef::k_15_G2:
        case ImoClef::k_G2_15:
            m_tPos[1] = - 50.0f;        //line 5 (Fa)
            m_tPos[2] = - 35.0f;        //space between lines 3 & 4 (Do)
            m_tPos[3] = - 55.0f;        //space above line 5 (Sol)
            m_tPos[4] = - 40.0f;        //line 4 (Re)
            m_tPos[5] = - 25.0f;        //space between lines 2 & 3 (La)
            m_tPos[6] = - 45.0f;        //space between lines 4 & 5 (Mi)
            m_tPos[7] = - 30.0f;        //line 3 (Si)
            break;

        case ImoClef::k_F4:
        case ImoClef::k_8_F4:
        case ImoClef::k_F4_8:
        case ImoClef::k_15_F4:
        case ImoClef::k_F4_15:
            m_tPos[1] = - 40.0f;        //line 4 (Fa)
            m_tPos[2] = - 25.0f;        //space between lines 2 & 3 (Do)
            m_tPos[3] = - 45.0f;        //space between lines 4 & 5 (Sol)
            m_tPos[4] = - 30.0f;        //line 3 (Re)
            m_tPos[5] = - 15.0f;        //line 5 (La)
            m_tPos[6] = - 35.0f;        //space between lines 3 & 4 (Mi)
            m_tPos[7] = - 20.0f;        //space aboveline 5 (Si)
            break;

        case ImoClef::k_F3:
            m_tPos[1] = - 30.0f;		//line 3 (Fa)
            m_tPos[2] = - 50.0f;      //line 5 (Do)
            m_tPos[3] = - 35.0f;      //space between lines 3 & 4 (Sol)
            m_tPos[4] = - 20.0f;      //line 2 (Re)
            m_tPos[5] = - 40.0f;      //line 4 (La)
            m_tPos[6] = - 25.0f;      //space between lines 2 & 3 (Mi)
            m_tPos[7] = - 45.0f;      //space between lines 4 & 5 (Si)
            break;

        case ImoClef::k_C1:
            m_tPos[1] = - 25.0f;        //space between lines 2 & 3 (Fa)
            m_tPos[2] = - 10.0f;        //line 1 (Do)
            m_tPos[3] = - 30.0f;        //line 3 (Sol)
            m_tPos[4] = - 15.0f;        //space between lines 1 & 2 (Re)
            m_tPos[5] = - 35.0f;        //space between lines 3 & 4 (La)
            m_tPos[6] = - 20.0f;        //line 2 (Mi)
            m_tPos[7] = - 40.0f;        //linea 4 (Si)
            break;

        case ImoClef::k_C2:
            m_tPos[1] = - 35.0f;		//space between lines 3 & 4 (Fa)
            m_tPos[2] = - 20.0f;        //line 2 (Do)
            m_tPos[3] = - 40.0f;        //line 4 (Sol)
            m_tPos[4] = - 25.0f;        //space between lines 2 & 3 (Re)
            m_tPos[5] = - 10.0f;        //line 1 (La)
            m_tPos[6] = - 30.0f;        //line 3 (Mi)
            m_tPos[7] = - 15.0f;        //space between lines 1 & 2 (Si)
            break;

        case ImoClef::k_C3:
            m_tPos[1] = - 45.0f;		//space between lines 4 & 5 (Fa)
            m_tPos[2] = - 30.0f;        //line 3 (Do)
            m_tPos[3] = - 50.0f;        //line 5 (Sol)
            m_tPos[4] = - 35.0f;        //space between lines 3 & 4 (Re)
            m_tPos[5] = - 20.0f;        //line 2 (La)
            m_tPos[6] = - 40.0f;        //line 4 (Mi)
            m_tPos[7] = - 25.0f;        //space between lines 2 & 3 (Si)
            break;

        case ImoClef::k_C4:
            m_tPos[1] = - 20.0f;		//line 2 (Fa)
            m_tPos[2] = - 40.0f;      //line 4 (Do)
            m_tPos[3] = - 25.0f;      //space between lines 2 & 3 (Sol)
            m_tPos[4] = - 45.0f;      //space between lines 4 & 5 (Re)
            m_tPos[5] = - 30.0f;      //line 3 (La)
            m_tPos[6] = - 50.0f;      //line 5 (Mi)
            m_tPos[7] = - 35.0f;      //space between lines 3 & 4 (Si)
            break;

        case ImoClef::k_C5:
            m_tPos[1] = - 30.0f;        //line 3 (Fa)
            m_tPos[2] = - 50.0f;        //line 5 (Do)
            m_tPos[3] = - 35.0f;        //space between lines 3 & 4 (Sol)
            m_tPos[4] = - 20.0f;        //line 2 (Re)
            m_tPos[5] = - 40.0f;        //line 4 (La)
            m_tPos[6] = - 25.0f;        //space between lines 2 & 3 (Mi)
            m_tPos[7] = - 45.0f;        //space between lines 4 & 5 (Si)
            break;

        case ImoClef::k_F5:
            m_tPos[1] = - 50.0f;        //line 5 (Fa)
            m_tPos[2] = - 35.0f;        //space between lines 3 & 4 (Do)
            m_tPos[3] = - 20.0f;        //line 2 (Sol)
            m_tPos[4] = - 40.0f;        //line 4 (Re)
            m_tPos[5] = - 25.0f;        //space between lines 2 & 3 (La)
            m_tPos[6] = - 45.0f;        //space between lines 4 & 5 (Mi)
            m_tPos[7] = - 30.0f;        //line 3 (Si)
            break;

        case ImoClef::k_G1:
            m_tPos[1] = - 40.0f;        //line 4 (Fa)
            m_tPos[2] = - 25.0f;        //space between lines 2 & 3 (Do)
            m_tPos[3] = - 10.0f;        //line 1 (Sol)
            m_tPos[4] = - 30.0f;        //line 3 (Re)
            m_tPos[5] = - 15.0f;        //space between lines 1 & 2 (La)
            m_tPos[6] = - 35.0f;        //space between lines 3 & 4 (Mi)
            m_tPos[7] = - 20.0f;        //line 2 (Si)
            break;

        case ImoClef::k_percussion:
        case ImoClef::k_undefined:
            m_nKeyType = ImoKeySignature::k_C;    //force not to draw any accidentals
            break;

        default:
            //LogMessage();
            m_nKeyType = ImoKeySignature::k_C;    //force not to draw any accidentals
    }
}

//---------------------------------------------------------------------------------------
void KeyEngraver::compute_positions_for_flats(int clefType)
{
    switch(clefType)
    {
        case ImoClef::k_G2:
        case ImoClef::k_8_G2:
        case ImoClef::k_G2_8:
        case ImoClef::k_15_G2:
        case ImoClef::k_G2_15:
            m_tPos[1] = - 30.0f;		//line 3 (Si)
            m_tPos[2] = - 45.0f;       //space between lines 4 & 5 (Mi)
            m_tPos[3] = - 25.0f;       //space between lines 2 & 3 (La)
            m_tPos[4] = - 40.0f;       //line 4 (Re)
            m_tPos[5] = - 20.0f;       //line 2 (Sol)
            m_tPos[6] = - 35.0f;       //space between lines 3 & 4 (Do)
            m_tPos[7] = - 15.0f;       //space between lines 1 & 2 (Fa)
            break;

        case ImoClef::k_F4:
        case ImoClef::k_8_F4:
        case ImoClef::k_F4_8:
        case ImoClef::k_15_F4:
        case ImoClef::k_F4_15:
            m_tPos[1] = - 20.0f;		//line 2 (Si)
            m_tPos[2] = - 35.0f;       //space between lines 3 & 4 (Mi)
            m_tPos[3] = - 15.0f;       //space between lines 1 & 2 (La)
            m_tPos[4] = - 30.0f;       //line 3 (Re)
            m_tPos[5] = - 10.0f;      //line 1 (Sol)
            m_tPos[6] = - 25.0f;       //space between lines 2 & 3 (Do)
            m_tPos[7] = - 40.0f;       //linea 4 (Fa)
            break;

        case ImoClef::k_F3:
            m_tPos[1] = - 45.0f;		//space between lines 4 & 5 (Si)
            m_tPos[2] = - 25.0f;       //space between lines 2 & 3 (Mi)
            m_tPos[3] = - 40.0f;       //line 4 (La)
            m_tPos[4] = - 20.0f;       //line 2 (Re)
            m_tPos[5] = - 35.0f;       //space between lines 3 & 4 (Sol)
            m_tPos[6] = - 50.0f;       //line 5 (Do)
            m_tPos[7] = - 30.0f;       //line 3 (Fa)
            break;

        case ImoClef::k_C1:
            m_tPos[1] = - 40.0f;		//line 4 (Si)
            m_tPos[2] = - 20.0f;       //line 2 (Mi)
            m_tPos[3] = - 35.0f;       //space between lines 3 & 4 (La)
            m_tPos[4] = - 15.0f;       //space between lines 1 & 2 (Re)
            m_tPos[5] = - 30.0f;       //line 3 (Sol)
            m_tPos[6] = - 10.0f;      //line 1 (Do)
            m_tPos[7] = - 25.0f;       //space between lines 2 & 3 (Fa)
            break;

        case ImoClef::k_C2:
            m_tPos[1] = - 15.0f;       //space between lines 1 & 2 (Si)
            m_tPos[2] = - 30.0f;       //line 3 (Mi)
            m_tPos[3] = - 10.0f;      //line 1 (La)
            m_tPos[4] = - 25.0f;       //space between lines 2 & 3 (Re)
            m_tPos[5] = - 40.0f;       //line 4 (Sol)
            m_tPos[6] = - 20.0f;       //line 2 (Do)
            m_tPos[7] = - 35.0f;       //space between lines 3 & 4 (Fa)
            break;

        case ImoClef::k_C3:
            m_tPos[1] = - 25.0f;       //space between lines 2 & 3 (Si)
            m_tPos[2] = - 40.0f;       //line 4 (Mi)
            m_tPos[3] = - 20.0f;       //line 2 (La)
            m_tPos[4] = - 35.0f;       //space between lines 3 & 4 (Re)
            m_tPos[5] = - 50.0f;       //line 5 (Sol)
            m_tPos[6] = - 30.0f;       //line 3 (Do)
            m_tPos[7] = - 45.0f;       //space between lines 4 & 5 (Fa)
            break;

        case ImoClef::k_C4:
            m_tPos[1] = - 35.0f;       //space between lines 3 & 4 (Si)
            m_tPos[2] = - 50.0f;       //line 5 (Mi)
            m_tPos[3] = - 30.0f;       //line 3 (La)
            m_tPos[4] = - 45.0f;       //space between lines 4 & 5 (Re)
            m_tPos[5] = - 25.0f;       //space between lines 2 & 3 (Sol)
            m_tPos[6] = - 40.0f;       //line 4 (Do)
            m_tPos[7] = - 20.0f;       //line 2 (Fa)
            break;

        case ImoClef::k_C5:
            m_tPos[1] = - 45.0f;        //space between lines 4 & 5 (Si)
            m_tPos[2] = - 25.0f;        //space between lines 2 & 3 (Mi)
            m_tPos[3] = - 40.0f;        //line 4 (La)
            m_tPos[4] = - 20.0f;        //line 2 (Re)
            m_tPos[5] = - 35.0f;        //space between lines 3 & 4 (Sol)
            m_tPos[6] = - 50.0f;        //line 5 (Do)
            m_tPos[7] = - 30.0f;        //line 3 (Fa)
            break;

        case ImoClef::k_F5:
            m_tPos[1] = - 30.0f;        //line 3 (Si)
            m_tPos[2] = - 45.0f;        //space between lines 4 & 5 (Mi)
            m_tPos[3] = - 25.0f;        //space between lines 2 & 3 (La)
            m_tPos[4] = - 40.0f;        //line 4 (Re)
            m_tPos[5] = - 20.0f;        //line 2 (Sol)
            m_tPos[6] = - 35.0f;        //space between lines 3 & 4 (Do)
            m_tPos[7] = - 50.0f;        //linea 5 (Fa)
            break;

        case ImoClef::k_G1:
            m_tPos[1] = - 20.0f;		//line 2 (Si)
            m_tPos[2] = - 35.0f;        //space between lines 3 & 4 (Mi)
            m_tPos[3] = - 15.0f;        //space between lines 1 & 2 (La)
            m_tPos[4] = - 30.0f;        //line 3 (Re)
            m_tPos[5] = - 10.0f;        //line 1 (Sol)
            m_tPos[6] = - 25.0f;        //space between lines 2 & 3 (Do)
            m_tPos[7] = - 40.0f;        //line 4 (Fa)
            break;

        case ImoClef::k_percussion:
        case ImoClef::k_undefined:
            m_nKeyType = ImoKeySignature::k_C;    //force not to draw any accidentals
            break;

        default:
            //LogMessage();
            m_nKeyType = ImoKeySignature::k_C;    //force not to draw any accidentals
    }
}

//---------------------------------------------------------------------------------------
int KeyEngraver::get_num_fifths(int keyType)
{
    switch(keyType)
    {
        case ImoKeySignature::k_C:
        case ImoKeySignature::k_a:
            return 0;

        //Sharps ---------------------------------------
        case ImoKeySignature::k_G:
        case ImoKeySignature::k_e:
            return 1;

        case ImoKeySignature::k_D:
        case ImoKeySignature::k_b:
            return 2;

        case ImoKeySignature::k_A:
        case ImoKeySignature::k_fs:
            return 3;

        case ImoKeySignature::k_E:
        case ImoKeySignature::k_cs:
            return 4;

        case ImoKeySignature::k_B:
        case ImoKeySignature::k_gs:
            return 5;

        case ImoKeySignature::k_Fs:
        case ImoKeySignature::k_ds:
            return 6;

        case ImoKeySignature::k_Cs:
        case ImoKeySignature::k_as:
            return 7;


        //Flats -------------------------------------------
        case ImoKeySignature::k_F:
        case ImoKeySignature::k_d:
            return -1;

        case ImoKeySignature::k_Bf:
        case ImoKeySignature::k_g:
            return -2;

        case ImoKeySignature::k_Ef:
        case ImoKeySignature::k_c:
            return -3;

        case ImoKeySignature::k_Af:
        case ImoKeySignature::k_f:
            return -4;

        case ImoKeySignature::k_Df:
        case ImoKeySignature::k_bf:
            return -5;

        case ImoKeySignature::k_Gf:
        case ImoKeySignature::k_ef:
            return -6;

        case ImoKeySignature::k_Cf:
        case ImoKeySignature::k_af:
            return -7;

        default:
            return -7;
            //LogMessage();
    }
}

//---------------------------------------------------------------------------------------
double KeyEngraver::determine_font_size()
{
    //TODO
    return 21.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;
}


}  //namespace lomse

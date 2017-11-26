//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
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
KeyEngraver::KeyEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                         int iInstr, int iStaff)
    : Engraver(libraryScope, pScoreMeter, iInstr, iStaff)
    , m_pKeyShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
GmoShape* KeyEngraver::create_shape(ImoKeySignature* pKey, int clefType, UPoint uPos,
                                    Color color)
{
    m_pCreatorImo = pKey;
    m_nKeyType = pKey->get_key_type();
    m_fontSize = determine_font_size();
    m_color = color;

    //create the container shape object
    ShapeId idx = 0;
    m_pKeyShape = LOMSE_NEW GmoShapeKeySignature(pKey, idx, uPos, color, m_libraryScope);
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
    LUnits x = uPos.x;
    for (int i=1; i <= numAccidentals; i++)
    {
        Tenths yOffset = m_libraryScope.get_glyphs_table()->glyph_offset(iGlyph)
                         + m_tPos[i] + 50.0f;
        LUnits y = uPos.y + m_pMeter->tenths_to_logical(yOffset, m_iInstr, m_iStaff);
        GmoShape* pSA = LOMSE_NEW GmoShapeAccidental(m_pCreatorImo, 0, iGlyph, UPoint(x, y),
                                               m_color, m_libraryScope, m_fontSize);
        m_pKeyShape->add(pSA);
        x += pSA->get_width();
    }
}

//---------------------------------------------------------------------------------------
void KeyEngraver::compute_positions_for_sharps(int clefType)
{
    switch(clefType)
    {
        case k_clef_G2:
        case k_clef_8_G2:
        case k_clef_G2_8:
        case k_clef_15_G2:
        case k_clef_G2_15:
            m_tPos[1] = - 50.0f;        //line 5 (Fa)
            m_tPos[2] = - 35.0f;        //space between lines 3 & 4 (Do)
            m_tPos[3] = - 55.0f;        //space above line 5 (Sol)
            m_tPos[4] = - 40.0f;        //line 4 (Re)
            m_tPos[5] = - 25.0f;        //space between lines 2 & 3 (La)
            m_tPos[6] = - 45.0f;        //space between lines 4 & 5 (Mi)
            m_tPos[7] = - 30.0f;        //line 3 (Si)
            break;

        case k_clef_F4:
        case k_clef_8_F4:
        case k_clef_F4_8:
        case k_clef_15_F4:
        case k_clef_F4_15:
            m_tPos[1] = - 40.0f;        //line 4 (Fa)
            m_tPos[2] = - 25.0f;        //space between lines 2 & 3 (Do)
            m_tPos[3] = - 45.0f;        //space between lines 4 & 5 (Sol)
            m_tPos[4] = - 30.0f;        //line 3 (Re)
            m_tPos[5] = - 15.0f;        //line 5 (La)
            m_tPos[6] = - 35.0f;        //space between lines 3 & 4 (Mi)
            m_tPos[7] = - 20.0f;        //space aboveline 5 (Si)
            break;

        case k_clef_F3:
            m_tPos[1] = - 30.0f;		//line 3 (Fa)
            m_tPos[2] = - 50.0f;      //line 5 (Do)
            m_tPos[3] = - 35.0f;      //space between lines 3 & 4 (Sol)
            m_tPos[4] = - 20.0f;      //line 2 (Re)
            m_tPos[5] = - 40.0f;      //line 4 (La)
            m_tPos[6] = - 25.0f;      //space between lines 2 & 3 (Mi)
            m_tPos[7] = - 45.0f;      //space between lines 4 & 5 (Si)
            break;

        case k_clef_C1:
            m_tPos[1] = - 25.0f;        //space between lines 2 & 3 (Fa)
            m_tPos[2] = - 10.0f;        //line 1 (Do)
            m_tPos[3] = - 30.0f;        //line 3 (Sol)
            m_tPos[4] = - 15.0f;        //space between lines 1 & 2 (Re)
            m_tPos[5] = - 35.0f;        //space between lines 3 & 4 (La)
            m_tPos[6] = - 20.0f;        //line 2 (Mi)
            m_tPos[7] = - 40.0f;        //linea 4 (Si)
            break;

        case k_clef_C2:
            m_tPos[1] = - 35.0f;		//space between lines 3 & 4 (Fa)
            m_tPos[2] = - 20.0f;        //line 2 (Do)
            m_tPos[3] = - 40.0f;        //line 4 (Sol)
            m_tPos[4] = - 25.0f;        //space between lines 2 & 3 (Re)
            m_tPos[5] = - 10.0f;        //line 1 (La)
            m_tPos[6] = - 30.0f;        //line 3 (Mi)
            m_tPos[7] = - 15.0f;        //space between lines 1 & 2 (Si)
            break;

        case k_clef_C3:
            m_tPos[1] = - 45.0f;		//space between lines 4 & 5 (Fa)
            m_tPos[2] = - 30.0f;        //line 3 (Do)
            m_tPos[3] = - 50.0f;        //line 5 (Sol)
            m_tPos[4] = - 35.0f;        //space between lines 3 & 4 (Re)
            m_tPos[5] = - 20.0f;        //line 2 (La)
            m_tPos[6] = - 40.0f;        //line 4 (Mi)
            m_tPos[7] = - 25.0f;        //space between lines 2 & 3 (Si)
            break;

        case k_clef_C4:
            m_tPos[1] = - 20.0f;		//line 2 (Fa)
            m_tPos[2] = - 40.0f;      //line 4 (Do)
            m_tPos[3] = - 25.0f;      //space between lines 2 & 3 (Sol)
            m_tPos[4] = - 45.0f;      //space between lines 4 & 5 (Re)
            m_tPos[5] = - 30.0f;      //line 3 (La)
            m_tPos[6] = - 50.0f;      //line 5 (Mi)
            m_tPos[7] = - 35.0f;      //space between lines 3 & 4 (Si)
            break;

        case k_clef_C5:
            m_tPos[1] = - 30.0f;        //line 3 (Fa)
            m_tPos[2] = - 50.0f;        //line 5 (Do)
            m_tPos[3] = - 35.0f;        //space between lines 3 & 4 (Sol)
            m_tPos[4] = - 20.0f;        //line 2 (Re)
            m_tPos[5] = - 40.0f;        //line 4 (La)
            m_tPos[6] = - 25.0f;        //space between lines 2 & 3 (Mi)
            m_tPos[7] = - 45.0f;        //space between lines 4 & 5 (Si)
            break;

        case k_clef_F5:
            m_tPos[1] = - 50.0f;        //line 5 (Fa)
            m_tPos[2] = - 35.0f;        //space between lines 3 & 4 (Do)
            m_tPos[3] = - 20.0f;        //line 2 (Sol)
            m_tPos[4] = - 40.0f;        //line 4 (Re)
            m_tPos[5] = - 25.0f;        //space between lines 2 & 3 (La)
            m_tPos[6] = - 45.0f;        //space between lines 4 & 5 (Mi)
            m_tPos[7] = - 30.0f;        //line 3 (Si)
            break;

        case k_clef_G1:
            m_tPos[1] = - 40.0f;        //line 4 (Fa)
            m_tPos[2] = - 25.0f;        //space between lines 2 & 3 (Do)
            m_tPos[3] = - 10.0f;        //line 1 (Sol)
            m_tPos[4] = - 30.0f;        //line 3 (Re)
            m_tPos[5] = - 15.0f;        //space between lines 1 & 2 (La)
            m_tPos[6] = - 35.0f;        //space between lines 3 & 4 (Mi)
            m_tPos[7] = - 20.0f;        //line 2 (Si)
            break;

        case k_clef_percussion:
        case k_clef_undefined:
            m_nKeyType = k_key_C;    //force not to draw any accidentals
            break;

        default:
            //LogMessage();
            m_nKeyType = k_key_C;    //force not to draw any accidentals
    }
}

//---------------------------------------------------------------------------------------
void KeyEngraver::compute_positions_for_flats(int clefType)
{
    switch(clefType)
    {
        case k_clef_G2:
        case k_clef_8_G2:
        case k_clef_G2_8:
        case k_clef_15_G2:
        case k_clef_G2_15:
            m_tPos[1] = - 30.0f;		//line 3 (Si)
            m_tPos[2] = - 45.0f;       //space between lines 4 & 5 (Mi)
            m_tPos[3] = - 25.0f;       //space between lines 2 & 3 (La)
            m_tPos[4] = - 40.0f;       //line 4 (Re)
            m_tPos[5] = - 20.0f;       //line 2 (Sol)
            m_tPos[6] = - 35.0f;       //space between lines 3 & 4 (Do)
            m_tPos[7] = - 15.0f;       //space between lines 1 & 2 (Fa)
            break;

        case k_clef_F4:
        case k_clef_8_F4:
        case k_clef_F4_8:
        case k_clef_15_F4:
        case k_clef_F4_15:
            m_tPos[1] = - 20.0f;		//line 2 (Si)
            m_tPos[2] = - 35.0f;       //space between lines 3 & 4 (Mi)
            m_tPos[3] = - 15.0f;       //space between lines 1 & 2 (La)
            m_tPos[4] = - 30.0f;       //line 3 (Re)
            m_tPos[5] = - 10.0f;      //line 1 (Sol)
            m_tPos[6] = - 25.0f;       //space between lines 2 & 3 (Do)
            m_tPos[7] = - 40.0f;       //linea 4 (Fa)
            break;

        case k_clef_F3:
            m_tPos[1] = - 45.0f;		//space between lines 4 & 5 (Si)
            m_tPos[2] = - 25.0f;       //space between lines 2 & 3 (Mi)
            m_tPos[3] = - 40.0f;       //line 4 (La)
            m_tPos[4] = - 20.0f;       //line 2 (Re)
            m_tPos[5] = - 35.0f;       //space between lines 3 & 4 (Sol)
            m_tPos[6] = - 50.0f;       //line 5 (Do)
            m_tPos[7] = - 30.0f;       //line 3 (Fa)
            break;

        case k_clef_C1:
            m_tPos[1] = - 40.0f;		//line 4 (Si)
            m_tPos[2] = - 20.0f;       //line 2 (Mi)
            m_tPos[3] = - 35.0f;       //space between lines 3 & 4 (La)
            m_tPos[4] = - 15.0f;       //space between lines 1 & 2 (Re)
            m_tPos[5] = - 30.0f;       //line 3 (Sol)
            m_tPos[6] = - 10.0f;      //line 1 (Do)
            m_tPos[7] = - 25.0f;       //space between lines 2 & 3 (Fa)
            break;

        case k_clef_C2:
            m_tPos[1] = - 15.0f;       //space between lines 1 & 2 (Si)
            m_tPos[2] = - 30.0f;       //line 3 (Mi)
            m_tPos[3] = - 10.0f;      //line 1 (La)
            m_tPos[4] = - 25.0f;       //space between lines 2 & 3 (Re)
            m_tPos[5] = - 40.0f;       //line 4 (Sol)
            m_tPos[6] = - 20.0f;       //line 2 (Do)
            m_tPos[7] = - 35.0f;       //space between lines 3 & 4 (Fa)
            break;

        case k_clef_C3:
            m_tPos[1] = - 25.0f;       //space between lines 2 & 3 (Si)
            m_tPos[2] = - 40.0f;       //line 4 (Mi)
            m_tPos[3] = - 20.0f;       //line 2 (La)
            m_tPos[4] = - 35.0f;       //space between lines 3 & 4 (Re)
            m_tPos[5] = - 50.0f;       //line 5 (Sol)
            m_tPos[6] = - 30.0f;       //line 3 (Do)
            m_tPos[7] = - 45.0f;       //space between lines 4 & 5 (Fa)
            break;

        case k_clef_C4:
            m_tPos[1] = - 35.0f;       //space between lines 3 & 4 (Si)
            m_tPos[2] = - 50.0f;       //line 5 (Mi)
            m_tPos[3] = - 30.0f;       //line 3 (La)
            m_tPos[4] = - 45.0f;       //space between lines 4 & 5 (Re)
            m_tPos[5] = - 25.0f;       //space between lines 2 & 3 (Sol)
            m_tPos[6] = - 40.0f;       //line 4 (Do)
            m_tPos[7] = - 20.0f;       //line 2 (Fa)
            break;

        case k_clef_C5:
            m_tPos[1] = - 45.0f;        //space between lines 4 & 5 (Si)
            m_tPos[2] = - 25.0f;        //space between lines 2 & 3 (Mi)
            m_tPos[3] = - 40.0f;        //line 4 (La)
            m_tPos[4] = - 20.0f;        //line 2 (Re)
            m_tPos[5] = - 35.0f;        //space between lines 3 & 4 (Sol)
            m_tPos[6] = - 50.0f;        //line 5 (Do)
            m_tPos[7] = - 30.0f;        //line 3 (Fa)
            break;

        case k_clef_F5:
            m_tPos[1] = - 30.0f;        //line 3 (Si)
            m_tPos[2] = - 45.0f;        //space between lines 4 & 5 (Mi)
            m_tPos[3] = - 25.0f;        //space between lines 2 & 3 (La)
            m_tPos[4] = - 40.0f;        //line 4 (Re)
            m_tPos[5] = - 20.0f;        //line 2 (Sol)
            m_tPos[6] = - 35.0f;        //space between lines 3 & 4 (Do)
            m_tPos[7] = - 50.0f;        //linea 5 (Fa)
            break;

        case k_clef_G1:
            m_tPos[1] = - 20.0f;		//line 2 (Si)
            m_tPos[2] = - 35.0f;        //space between lines 3 & 4 (Mi)
            m_tPos[3] = - 15.0f;        //space between lines 1 & 2 (La)
            m_tPos[4] = - 30.0f;        //line 3 (Re)
            m_tPos[5] = - 10.0f;        //line 1 (Sol)
            m_tPos[6] = - 25.0f;        //space between lines 2 & 3 (Do)
            m_tPos[7] = - 40.0f;        //line 4 (Fa)
            break;

        case k_clef_percussion:
        case k_clef_undefined:
            m_nKeyType = k_key_C;    //force not to draw any accidentals
            break;

        default:
            //LogMessage();
            m_nKeyType = k_key_C;    //force not to draw any accidentals
    }
}

//---------------------------------------------------------------------------------------
int KeyEngraver::get_num_fifths(int keyType)
{
    switch(keyType)
    {
        case k_key_C:
        case k_key_a:
            return 0;

        //Sharps ---------------------------------------
        case k_key_G:
        case k_key_e:
            return 1;

        case k_key_D:
        case k_key_b:
            return 2;

        case k_key_A:
        case k_key_fs:
            return 3;

        case k_key_E:
        case k_key_cs:
            return 4;

        case k_key_B:
        case k_key_gs:
            return 5;

        case k_key_Fs:
        case k_key_ds:
            return 6;

        case k_key_Cs:
        case k_key_as:
            return 7;


        //Flats -------------------------------------------
        case k_key_F:
        case k_key_d:
            return -1;

        case k_key_Bf:
        case k_key_g:
            return -2;

        case k_key_Ef:
        case k_key_c:
            return -3;

        case k_key_Af:
        case k_key_f:
            return -4;

        case k_key_Df:
        case k_key_bf:
            return -5;

        case k_key_Gf:
        case k_key_ef:
            return -6;

        case k_key_Cf:
        case k_key_af:
            return -7;

        default:
            return -7;
            //LogMessage();
    }
}


}  //namespace lomse

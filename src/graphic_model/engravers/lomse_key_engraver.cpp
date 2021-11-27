//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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
#include "lomse_staffobjs_cursor.h"
#include "lomse_accidentals_engraver.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// KeyEngraver implementation
//---------------------------------------------------------------------------------------
KeyEngraver::KeyEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                         int iInstr, int iStaff)
    : StaffObjEngraver(libraryScope, pScoreMeter, iInstr, iStaff)
    , m_pKeyShape(nullptr)
    , m_nKeyType(k_clef_undefined)
    , m_fontSize(0.0)
    , m_pCreatorImo(nullptr)
{
}

//---------------------------------------------------------------------------------------
GmoShape* KeyEngraver::create_shape(ImoKeySignature* pKey, ImoClef* pClef, UPoint uPos,
                                    StaffObjsCursor* pCursor, Color color)
{
    m_pCreatorImo = pKey;
    m_nKeyType = pKey->get_key_type();
    m_fontSize = determine_font_size();
    m_color = color;

    if (pKey->is_standard())
        return create_shape_for_standard_key(pKey, pClef, uPos, pCursor);
    else
        return create_shape_for_non_standard_key(pKey, pClef, uPos, pCursor);
}

//---------------------------------------------------------------------------------------
GmoShape* KeyEngraver::create_shape_for_standard_key(ImoKeySignature* pKey,
                                                     ImoClef* pClef, UPoint uPos,
                                                     StaffObjsCursor* pCursor)
{
    //create the container shape object
    ShapeId idx = 0;
    m_pKeyShape = LOMSE_NEW GmoShapeKeySignature(pKey, idx, uPos, m_color, m_libraryScope);

    if (!pClef || !pClef->supports_accidentals())
        return m_pKeyShape;

    //add accidentals to the shape
    int numAccidentals = pKey->get_fifths();
    if (numAccidentals == 0)
    {
        //cancel_previous_key
        ImoKeySignature* pPrevKey = pCursor ? pCursor->get_key_for_instr_staff(m_iInstr, m_iStaff) : nullptr;
        if (pPrevKey && pPrevKey->has_accidentals())
            cancel_key(pPrevKey, pClef, uPos);
    }
    else if (numAccidentals > 0)
    {
        if (numAccidentals <= 7)
            add_sharps(1, numAccidentals, uPos, k_glyph_sharp_accidental, pClef, pKey);
        else
        {
            int iStart = numAccidentals - 6;
            uPos = add_sharps(iStart, 7, uPos, k_glyph_sharp_accidental, pClef, pKey);
            add_sharps(1, iStart-1, uPos, k_glyph_double_sharp_accidental, pClef, pKey);
        }
    }
    else
    {
        if (numAccidentals >= -7)
            add_flats(1, -numAccidentals, uPos, k_glyph_flat_accidental, pClef, pKey);
        else
        {
            int iStart = -numAccidentals - 6;
            uPos = add_flats(iStart, 7, uPos, k_glyph_flat_accidental, pClef, pKey);
            add_flats(1, iStart-1, uPos, k_glyph_double_flat_accidental, pClef, pKey);
        }
    }
    return m_pKeyShape;
}

//---------------------------------------------------------------------------------------
GmoShape* KeyEngraver::create_shape_for_non_standard_key(ImoKeySignature* pKey,
                                                         ImoClef* pClef, UPoint uPos,
                                                         StaffObjsCursor* pCursor)
{
    //create the container shape object
    ShapeId idx = 0;
    m_pKeyShape = LOMSE_NEW GmoShapeKeySignature(pKey, idx, uPos, m_color, m_libraryScope);

    if (!pClef || !pClef->supports_accidentals())
        return m_pKeyShape;

    //space between accidentals
    const LUnits space = tenths_to_logical(LOMSE_SPACE_BETWEEN_KEY_ACCIDENTALS);

    //cancel_previous_key
    //TODO: cancel only when new key signature is 0 accidentals?
    //  This behaviour is not coherent with that of standard key signatures. An option at
    //  document level (overriding a global option for default behaviour) is needed. And
    //  the behaviour should be the same for standard and non-standard.
    ImoKeySignature* pPrevKey = pCursor ? pCursor->get_key_for_instr_staff(m_iInstr, m_iStaff) : nullptr;
    if (pPrevKey && pPrevKey->has_accidentals())
        uPos = cancel_key(pPrevKey, pClef, uPos);

    //add accidentals to the shape
    for (int i=0; i < 7; ++i)
    {
        KeyAccidental& acc = pKey->get_accidental(i);
        if (acc.step != k_step_undefined && acc.accidental != k_no_accidentals)
        {
            //accidental position is not standardized. Use positions for sharps
            Tenths yShift = compute_accidental_shift(acc.step, pKey->get_octave(i), pClef, true);
            uPos = add_accidental(acc.accidental, uPos, yShift);
            uPos.x += space;
        }
    }
    return m_pKeyShape;
}

//---------------------------------------------------------------------------------------
UPoint KeyEngraver::cancel_key(ImoKeySignature* pKey, ImoClef* pClef, UPoint uPos)
{
    if (pKey->is_standard())
    {
        const int numAccidentals = pKey->get_fifths();

        //display naturals to let a performer know about the key change
        if (numAccidentals > 0)
        {
            int iMax = (numAccidentals <= 7 ? numAccidentals : 7);
            uPos = add_sharps(1, iMax, uPos, k_glyph_natural_accidental, pClef, pKey);
        }
        else if (numAccidentals < 0)
        {
            int iMax = (-numAccidentals <= 7 ? -numAccidentals : 7);
            uPos = add_flats(1, iMax, uPos, k_glyph_natural_accidental, pClef, pKey);
        }
    }
    else
    {
        const LUnits space = tenths_to_logical(LOMSE_SPACE_BETWEEN_KEY_ACCIDENTALS);

        for (int i=0; i < 7; ++i)
        {
            KeyAccidental& acc = pKey->get_accidental(i);
            if (acc.step != k_step_undefined && acc.accidental != k_no_accidentals)
            {
                //accidental position is not standardized. Use positions for sharps
                Tenths yShift = compute_accidental_shift(acc.step, pKey->get_octave(i), pClef, true);
                uPos = add_accidental(k_glyph_natural_accidental, uPos, yShift);
                uPos.x += space;
            }
        }
    }

    return uPos;
}

//---------------------------------------------------------------------------------------
UPoint KeyEngraver::add_accidental(int acc, UPoint uPos, Tenths yShift)
{
    int iGlyph = AccidentalsEngraver::get_glyph_for(acc);
    LUnits y = uPos.y + tenths_to_logical(yShift);

    AccidentalsEngraver engrv(m_libraryScope, m_pMeter, m_iInstr, m_iStaff);
    GmoShape* pSA = engrv.create_shape_for_glyph(iGlyph, UPoint(uPos.x, y), m_color,
                                                 m_fontSize, m_pCreatorImo, 0);
    m_pKeyShape->add(pSA);
    uPos.x += pSA->get_width();

    return uPos;
}

//---------------------------------------------------------------------------------------
UPoint KeyEngraver::add_flats(int iStart, int iEnd, UPoint uPos, int iGlyph,
                              ImoClef* pClef, ImoKeySignature* pKey)
{
    return add_accidentals(iStart, iEnd, uPos, iGlyph, pClef, pKey, false);
}

//---------------------------------------------------------------------------------------
UPoint KeyEngraver::add_sharps(int iStart, int iEnd, UPoint uPos, int iGlyph,
                               ImoClef* pClef, ImoKeySignature* pKey)
{
    return add_accidentals(iStart, iEnd, uPos, iGlyph, pClef, pKey, true);
}

//---------------------------------------------------------------------------------------
UPoint KeyEngraver::add_accidentals(int iStart, int iEnd, UPoint uPos, int iGlyph,
                                    ImoClef* pClef, ImoKeySignature* pKey,
                                    bool fAtSharpPosition)
{
    const LUnits space = tenths_to_logical(LOMSE_SPACE_BETWEEN_KEY_ACCIDENTALS);
    const int sharps[7] = { k_step_F, k_step_C, k_step_G, k_step_D, k_step_A, k_step_E, k_step_B };
    const int flats[7] = { k_step_B, k_step_E, k_step_A, k_step_D, k_step_G, k_step_C, k_step_F };

    LUnits x = uPos.x;
    AccidentalsEngraver engrv(m_libraryScope, m_pMeter, m_iInstr, m_iStaff);

    for (int i=iStart-1; i < iEnd; i++)
    {
        int j = (i < 7 ? i : i - 8);

        int accOctave = pKey->get_octave(j);
        int step = (fAtSharpPosition ? sharps[j] : flats[j]);
        Tenths shift = compute_accidental_shift(step, accOctave, pClef, fAtSharpPosition);
        LUnits y = uPos.y + tenths_to_logical(shift);

        GmoShape* pSA = engrv.create_shape_for_glyph(iGlyph, UPoint(x, y), m_color,
                                                     m_fontSize, m_pCreatorImo, 0);
        m_pKeyShape->add(pSA);
        x += pSA->get_width() + space;
    }
    return UPoint(x, uPos.y);
}

//---------------------------------------------------------------------------------------
Tenths KeyEngraver::compute_accidental_shift(int accStep, int accOctave, ImoClef* pClef,
                                             bool fAtSharpPosition)
{
    // Returns the shift (Tenths) referred to the first ledger line of the staff (bellow)

    DiatonicPitch clefPitch = pClef->get_first_ledger_line_pitch();

    //compute pitch for accidental line/space
    if (accOctave == -1)
    {
        accOctave = pClef->get_octave_change();
        if (fAtSharpPosition)
            accOctave += get_sharp_default_octave(accStep, pClef->get_sign(), pClef->get_line());
        else
            accOctave += get_flat_default_octave(accStep, pClef->get_sign(), pClef->get_line());
    }
    DiatonicPitch accPitch(accStep, accOctave);

    //compute shift
    return Tenths((accPitch - clefPitch) * -5);
}

//---------------------------------------------------------------------------------------
int KeyEngraver::get_sharp_default_octave(int step, int clefSign, int clefLine)
{
    if (clefSign == k_clef_sign_G)
    {
        if (step == k_step_A || step == k_step_B)
            return 4;
        else
            return 5;
    }

    if (clefSign == k_clef_sign_F)
    {
        if (clefLine == 4 || clefLine == 5)
        {
            if (step == k_step_A || step == k_step_B)
                return 2;
            else
                return 3;
        }
        else if (clefLine == 3)
            return 3;
    }

    if (clefSign == k_clef_sign_C)
    {
        if (clefLine == 1)  //C1
        {
            if (step == k_step_C || step == k_step_D || step == k_step_E)
                return 5;
            else
                return 4;
        }

        else if (clefLine == 2) //C2
            return 4;

        else if (clefLine == 3) //C3
        {
            if (step == k_step_A || step == k_step_B)
                return 3;
            else
                return 4;
        }

        else if (clefLine == 4) //C4
        {
            if (step == k_step_C || step == k_step_D || step == k_step_E)
                return 4;
            else
                return 3;
        }

        else if (clefLine == 5) //C5
        {
            if (step == k_step_C || step == k_step_D)
                return 4;
            else
                return 3;
        }
    }

    return 4;
}

//---------------------------------------------------------------------------------------
int KeyEngraver::get_flat_default_octave(int step, int clefSign, int clefLine)
{
    if (clefSign == k_clef_sign_G)
    {
        if (clefLine == 1)  //G1
        {
            if (step == k_step_C)
                return 5;
            else
                return 4;
        }
        else if (clefLine == 2)  //G2
        {
            if (step == k_step_C || step == k_step_D || step == k_step_E)
                return 5;
            else
                return 4;
        }
    }

    if (clefSign == k_clef_sign_F)
    {
        if (clefLine == 4 || clefLine == 5)
        {
            if (step == k_step_E || step == k_step_D || step == k_step_C)
                return 3;
            else
                return 2;
        }
        else if (clefLine == 3)
            return 3;
    }

    if (clefSign == k_clef_sign_C)
    {
        if (clefLine == 1 || clefLine == 2)  //C1, C2
            return 4;

        else if (clefLine == 3 || clefLine == 4) //C3, C4
        {
            if (step == k_step_C || step == k_step_D || step == k_step_E)
                return 4;
            else
                return 3;
        }

        else if (clefLine == 5) //C5
            return 3;
    }

    return 4;
}



}  //namespace lomse

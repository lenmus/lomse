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

#ifndef __LOMSE_KEY_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_KEY_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_pitch.h"

namespace lomse
{

//forward declarations
class ImoClef;
class ImoKeySignature;
class GmoShape;
class GmoShapeKeySignature;
class ScoreMeter;
class StaffObjsCursor;

//---------------------------------------------------------------------------------------
class KeyEngraver : public StaffObjEngraver
{
protected:
    GmoShapeKeySignature* m_pKeyShape;
    int m_nKeyType;
    Tenths m_tPos[8];           //sharps/flats positions, in order of appearance
    double m_fontSize;
    ImoObj* m_pCreatorImo;
    Color m_color;

public:
    KeyEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter, int iInstr,
                int iStaff);
    ~KeyEngraver() {}

    GmoShape* create_shape(ImoKeySignature* pKey, ImoClef* pClef, UPoint uPos,
                           StaffObjsCursor* pCursor, Color color=Color(0,0,0));

protected:
    UPoint cancel_key(ImoKeySignature* pKey, ImoClef* pClef, UPoint uPos);

    Tenths compute_accidental_shift(int accStep, int accOctave, ImoClef* pClef,
                                    bool fAtSharpPosition);

    int get_sharp_default_octave(int step, int clefSign, int clefLine);
    int get_flat_default_octave(int step, int clefSign, int clefLine);

    UPoint add_accidental(int iGlyph, UPoint uPos, LUnits yShift);
    UPoint add_accidentals(int iStart, int iEnd, UPoint uPos, int iGlyph, ImoClef* pClef,
                           ImoKeySignature* pKey, bool fAtSharpPosition);
    UPoint add_flats(int iStart, int iEnd, UPoint uPos, int iGlyph, ImoClef* pClef,
                     ImoKeySignature* pKey);
    UPoint add_sharps(int iStart, int iEnd, UPoint uPos, int iGlyph, ImoClef* pClef,
                      ImoKeySignature* pKey);

    GmoShape* create_shape_for_standard_key(ImoKeySignature* pKey, ImoClef* pClef,
                                            UPoint uPos, StaffObjsCursor* pCursor);
    GmoShape* create_shape_for_non_standard_key(ImoKeySignature* pKey, ImoClef* pClef,
                                                UPoint uPos, StaffObjsCursor* pCursor);

};


}   //namespace lomse

#endif    // __LOMSE_KEY_ENGRAVER_H__

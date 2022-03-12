//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

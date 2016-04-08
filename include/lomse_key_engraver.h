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
    Tenths m_tPos[8];           //sharps/flats positions, in order of appearance
    double m_fontSize;
    ImoObj* m_pCreatorImo;
    Color m_color;

public:
    KeyEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter, int iInstr,
                int iStaff);
    ~KeyEngraver() {}

    GmoShape* create_shape(ImoKeySignature* pKey, int clefType, UPoint uPos,
                           Color color=Color(0,0,0));

protected:
    void compute_positions_for_flats(int clefType);
    void compute_positions_for_sharps(int clefType);
    void add_accidentals(int numAccidentals, int iGlyph, UPoint uPos);
    int get_num_fifths(int keyType);

};


}   //namespace lomse

#endif    // __LOMSE_KEY_ENGRAVER_H__

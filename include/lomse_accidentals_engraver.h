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

#ifndef __LOMSE_ACCIDENTALS_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_ACCIDENTALS_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_pitch.h"

namespace lomse
{

//forward declarations
class GmoShapeAccidentals;
class GmoShapeAccidental;
class GmoShape;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class AccidentalsEngraver : public StaffSymbolEngraver
{
protected:
    EAccidentals m_accidentals;
    bool m_fCautionary;
    GmoShapeAccidentals* m_pContainer;
    double m_fontSize;
    ImoNote* m_pNote;
    Color m_color;

public:
    AccidentalsEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                        int iInstr, int iStaff);
    ~AccidentalsEngraver() {}

    GmoShapeAccidentals* create_shape(ImoNote* pNote, UPoint uPos,
                                      EAccidentals accidentals, double fontSize,
                                      bool fCautionary=false, Color color=Color(0,0,0));

    //helper, for KeyEngraver or other
    GmoShapeAccidental* create_shape_for_glyph(int iGlyph, UPoint pos, Color color,
                                               double fontSize, ImoObj* pCreatorImo,
                                               ShapeId idx);
    static int get_glyph_for(int accidental);


protected:
    void find_glyphs();
    void create_container_shape(UPoint pos);
    void add_glyphs_to_container_shape(UPoint pos);
    void add_voice(VoiceRelatedShape* pVRS);
    LUnits glyph_offset(int iGlyph);

    int m_glyphs[4];
    int m_numGlyphs;
};


}   //namespace lomse

#endif    // __LOMSE_ACCIDENTALS_ENGRAVER_H__


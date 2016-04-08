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

#ifndef __LOMSE_METRONOME_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_METRONOME_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class GmoShapeMetronomeMark;
class ImoObj;
class GmoShape;
class ScoreMeter;
class ImoMetronomeMark;

//---------------------------------------------------------------------------------------
class MetronomeMarkEngraver : public Engraver
{
protected:
    GmoShapeMetronomeMark* m_pMainShape;
    UPoint m_uPos;
    double m_fontSize;
    ImoMetronomeMark* m_pCreatorImo;
    Color m_color;

public:
    MetronomeMarkEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                          int iInstr, int iStaff);
    ~MetronomeMarkEngraver() {}

    GmoShape* create_shape(ImoMetronomeMark* pImo, UPoint uPos, Color color=Color(0,0,0));

protected:
    void create_main_container_shape();
    GmoShape* create_shape_mm_value();
    GmoShape* create_shape_note_note();
    GmoShape* create_shape_note_value();
    void create_text_shape(const string& text);
    void create_symbol_shape(int noteType, int dots);
    int select_glyph(int noteType);

};


}   //namespace lomse

#endif    // __LOMSE_METRONOME_ENGRAVER_H__


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

#ifndef __LOMSE_REST_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_REST_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoRest;
class GmoShapeRest;
class GmoShapeRestGlyph;
class ScoreMeter;
class EngraversMap;
class GmoShapeBeam;
class VoiceRelatedShape;

//---------------------------------------------------------------------------------------
class RestEngraver : public Engraver
{
protected:
    int m_restType;
    int m_numDots;
    ImoRest* m_pRest;
    Color m_color;
    double m_fontSize;

public:
    RestEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 EngraversMap* pEngravers, int iInstr, int iStaff);
    ~RestEngraver() {}

    GmoShapeRest* create_shape(ImoRest* pRest, UPoint uPos, Color color=Color(0,0,0));
    GmoShape* create_tool_dragged_shape(int restType, int dots);
    UPoint get_drag_offset();

protected:
    void determine_position();
    void create_main_shape();
    void add_shapes_for_dots_if_required();

    void add_voice(VoiceRelatedShape* pVRS);
    int find_glyph();
    LUnits get_glyph_offset(int iGlyph);
    LUnits add_dot_shape(LUnits x, LUnits y, Color color);
    Tenths get_offset_for_dot();

    LUnits m_uxLeft, m_uyTop;       //current position
    int m_iGlyph;
    GmoShapeRest* m_pRestShape;
    GmoShapeRestGlyph* m_pRestGlyphShape;
};


}   //namespace lomse

#endif    // __LOMSE_REST_ENGRAVER_H__


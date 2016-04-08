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

#ifndef __LOMSE_TIME_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_TIME_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class GmoShapeTimeSignature;
class ImoObj;
class GmoShape;
class ScoreMeter;
class ImoTimeSignature;

//---------------------------------------------------------------------------------------
class TimeEngraver : public Engraver
{
protected:
    GmoShapeTimeSignature* m_pTimeShape;
    UPoint m_uPos;
    LUnits m_uTopWidth;
    LUnits m_uBottomWidth;
    GmoShape* m_pShapesTop[2];
    GmoShape* m_pShapesBottom[2];
    double m_fontSize;
    ImoTimeSignature* m_pCreatorImo;
    Color m_color;

public:
    TimeEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 int iInstr, int iStaff);
    ~TimeEngraver() {}

    GmoShape* create_shape(ImoTimeSignature* pCreatorImo, UPoint uPos,
                           Color color=Color(0,0,0));

protected:
    GmoShape* create_symbol_shape(int iGlyph, ShapeId idx);
    GmoShape* create_shape_normal(UPoint uPos, int beats, int beat_type);

    void create_main_container_shape(UPoint uPos);
    void create_top_digits(UPoint uPos, int beats);
    void create_bottom_digits(UPoint uPos, int beat_type);
    void center_numbers();
    void add_all_shapes_to_container();
    void create_digits(int digits, GmoShape* pShape[]);
    GmoShape* create_digit(int digit);

};


}   //namespace lomse

#endif    // __LOMSE_TIME_ENGRAVER_H__


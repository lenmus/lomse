//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2017. All rights reserved.
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

#ifndef __LOMSE_VOLTA_BRACKET_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_VOLTA_BRACKET_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoVoltaBracket;
class ImoBarline;
class GmoShapeVoltaBracket;
class GmoShapeBarline;
class ScoreMeter;
class InstrumentEngraver;
class ImoStyle;

//---------------------------------------------------------------------------------------
class VoltaBracketEngraver : public RelObjEngraver
{
protected:
    int m_numShapes;
    ImoVoltaBracket* m_pVolta;
    LUnits m_uStaffLeft;
    LUnits m_uStaffRight;
    ImoStyle* m_pStyle;

    ImoBarline* m_pStartBarline;
    ImoBarline* m_pStopBarline;
    GmoShapeBarline* m_pStartBarlineShape;
    GmoShapeBarline* m_pStopBarlineShape;

    bool m_fTwoBrackets;
    ShapeBoxInfo m_shapesInfo[2];

public:
    VoltaBracketEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                         LUnits uStaffLeft, LUnits uStaffRight);
    ~VoltaBracketEngraver() {}

    void set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol,
                            LUnits xRight, LUnits xLeft, LUnits yTop);
    void set_end_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol,
                          LUnits xRight, LUnits xLeft, LUnits yTop);
    int create_shapes(Color color=Color(0,0,0));
    int get_num_shapes();
    ShapeBoxInfo* get_shape_box_info(int i);

    inline void set_prolog_width(LUnits width) { m_uStaffLeft += width; }

protected:
    void decide_if_one_or_two_brackets();
    inline bool two_brackets_needed() { return m_fTwoBrackets; }
    void create_two_shapes();
    void create_one_shape();

    void set_shape_details(GmoShapeVoltaBracket* pShape, bool fFirstShape);
};


}   //namespace lomse

#endif    // __LOMSE_VOLTA_BRACKET_ENGRAVER_H__


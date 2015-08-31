//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2015 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_ENGRAVER_H__

#include "lomse_injectors.h"
#include "lomse_basic.h"

namespace lomse
{

//forward declarations
class ScoreMeter;
class GmoShape;
class ImoRelObj;
class ImoStaffObj;

//---------------------------------------------------------------------------------------
// base class for all engravers
class Engraver
{
protected:
    LibraryScope& m_libraryScope;
    ScoreMeter* m_pMeter;
    int m_iInstr;
    int m_iStaff;

public:
    Engraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter, int iInstr=0,
             int iStaff=0)
        : m_libraryScope(libraryScope)
        , m_pMeter(pScoreMeter)
        , m_iInstr(iInstr)
        , m_iStaff(iStaff)
    {
    }

    virtual ~Engraver() {}

protected:
    LUnits tenths_to_logical(Tenths value);
    virtual double determine_font_size();
    virtual void add_user_shift(ImoContentObj* pImo, UPoint* pos);
};


//---------------------------------------------------------------------------------------
// helper struct to save a shape and its box info (instrument, system, column)
struct ShapeBoxInfo
{
    GmoShape* pShape;
    int iSystem;
    int iCol;
    int iInstr;

    ShapeBoxInfo(GmoShape* shape, int system, int col, int instr)
        : pShape(shape), iSystem(system), iCol(col), iInstr(instr)
    {
    }
    ShapeBoxInfo() : pShape(NULL), iSystem(-1), iCol(-1), iInstr(-1) {}
};


//---------------------------------------------------------------------------------------
// base class for all realtion auxiliary objects' engravers
class RelAuxObjEngraver : public Engraver
{
protected:
    GmoShape* m_pShape;
    Color m_color;

public:
    RelAuxObjEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
        : Engraver(libraryScope, pScoreMeter)
        , m_pShape(NULL)
        , m_color( Color(0,0,0) )
    {
    }
    virtual ~RelAuxObjEngraver() {}

    virtual void set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                    GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                    int iSystem, int iCol) = 0;
    virtual void set_middle_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                     GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                     int iSystem, int iCol) {}
    virtual void set_end_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                  GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                  int iSystem, int iCol) = 0;
    virtual int create_shapes(Color color=Color(0,0,0)) = 0;
    virtual int get_num_shapes() = 0;
    virtual ShapeBoxInfo* get_shape_box_info(int i) = 0;
    virtual void set_prolog_width(LUnits width) {}

    virtual GmoShape* get_shape() { return m_pShape; }
};


}   //namespace lomse

#endif    // __LOMSE_ENGRAVER_H__


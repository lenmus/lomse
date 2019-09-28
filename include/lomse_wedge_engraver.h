//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#ifndef __LOMSE_WEDGE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_WEDGE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoWedgeData;
class ImoWedge;
class ImoDirection;
class GmoShapeWedge;
class GmoShapeInvisible;
class ScoreMeter;
class InstrumentEngraver;

//---------------------------------------------------------------------------------------
class WedgeEngraver : public RelObjEngraver
{
protected:
    InstrumentEngraver* m_pInstrEngrv;
    LUnits m_uStaffTopStart;    //top line of start system
    LUnits m_uStaffTopEnd;      //top line of end system

    int m_numShapes;
    ImoWedge* m_pWedge;
    bool m_fWedgeAbove;
    LUnits m_uPrologWidth;

    ImoDirection* m_pStartDirection;
    ImoDirection* m_pEndDirection;
    GmoShapeInvisible* m_pStartDirectionShape;
    GmoShapeInvisible* m_pEndDirectionShape;

    bool m_fTwoWedges;
    UPoint m_points1[4];    //points for first wedge
    UPoint m_points2[4];    //points for second wedge, when fisrt one is split
    ShapeBoxInfo m_shapesInfo[2];

public:
    WedgeEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 InstrumentEngraver* pInstrEngrv);
    ~WedgeEngraver() {}

    void set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol,
                            LUnits xRight, LUnits xLeft, LUnits yTop);
    void set_end_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol,
                          LUnits xRight, LUnits xLeft, LUnits yTop);
    int create_shapes(Color color);
    int create_shapes();
    int get_num_shapes();
    ShapeBoxInfo* get_shape_box_info(int i);

    void set_prolog_width(LUnits width);

protected:
    void decide_placement();
    void decide_if_one_or_two_wedges();
    inline bool two_wedges_needed() { return m_fTwoWedges; }
    void create_two_shapes();
    void create_one_shape();

    void compute_first_shape_position();
    void compute_second_shape_position();
    //void add_user_displacements(int iWedge, UPoint* points);

};


}   //namespace lomse

#endif    // __LOMSE_WEDGE_ENGRAVER_H__

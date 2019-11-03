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
class VerticalProfile;

//---------------------------------------------------------------------------------------
class WedgeEngraver : public RelObjEngraver
{
protected:
    InstrumentEngraver* m_pInstrEngrv;
    LUnits m_uStaffTop;         //top line of current system

    int m_numShapes;
    ImoWedge* m_pWedge;
    bool m_fWedgeAbove;
    LUnits m_uPrologWidth;

    ImoDirection* m_pStartDirection;
    ImoDirection* m_pEndDirection;
    GmoShapeInvisible* m_pStartDirectionShape;
    GmoShapeInvisible* m_pEndDirectionShape;

    UPoint m_points[4];    //points for wedge

public:
    WedgeEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 InstrumentEngraver* pInstrEngrv);
    ~WedgeEngraver() {}

    void set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol,
                            LUnits xStaffLeft, LUnits xStaffRight, LUnits yTop,
                            int idxStaff, VerticalProfile* pVProfile) override;
    void set_end_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol,
                          LUnits xStaffLeft, LUnits xStaffRight, LUnits yTop,
                          int idxStaff, VerticalProfile* pVProfile) override;

    //RelObjEngraver mandatory overrides
    void set_prolog_width(LUnits width) override;
    GmoShape* create_first_or_intermediate_shape(Color color=Color(0,0,0)) override;
    GmoShape* create_last_shape(Color color=Color(0,0,0)) override;

protected:
    void decide_placement();
    GmoShape* create_first_shape();
    GmoShape* create_intermediate_shape();
    GmoShape* create_final_shape();

    void compute_first_shape_position();
    void compute_last_shape_position();
    void compute_intermediate_shape_position();

    LUnits determine_center_line_of_shape(LUnits startSpread, LUnits endSpread);
    //void add_user_displacements(int iWedge, UPoint* points);

};


}   //namespace lomse

#endif    // __LOMSE_WEDGE_ENGRAVER_H__


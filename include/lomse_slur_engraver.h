//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_SLUR_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_SLUR_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoSlurData;
class ImoSlur;
class ImoNote;
class GmoShapeSlur;
class GmoShapeNote;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class SlurEngraver : public RelAuxObjEngraver
{
protected:
    LUnits m_uStaffLeft;
    LUnits m_uStaffRight;
    int m_numShapes;
    ImoSlur* m_pSlur;
    int m_iInstr;
    int m_iStaff;
    UPoint m_pos;
    bool m_fSlurBelow;
    std::vector< pair<ImoNote*, GmoShapeNote*> > m_notes;

    //bool m_fTwoArches;
    UPoint m_points1[4];    //bezier points for first arch
    //UPoint m_points2[4];    //bezier points for second arch
    LUnits m_thickness;
    ShapeBoxInfo m_shapesInfo[2];

public:
    SlurEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                LUnits uStaffLeft, LUnits uStaffRight);
    ~SlurEngraver() {}

    void set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol, UPoint pos);
    void set_end_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol);
    int create_shapes();
    int get_num_shapes();
    ShapeBoxInfo* get_shape_box_info(int i);

    inline void set_prolog_width(LUnits width) { m_uStaffLeft += width; }


protected:
    void decide_placement();
    //void decide_if_one_or_two_arches();
    //inline bool two_arches_needed() { return m_fTwoArches; }
    //void create_two_shapes();
    void create_one_shape();

    void compute_ref_point(GmoShapeNote* pNoteShape, UPoint* point);
    //void compute_start_of_staff_point();
    //void compute_end_of_staff_point();
    void compute_default_control_points(UPoint* points);
    //void add_user_displacements(int iTie, UPoint* points);
    LUnits tenths_to_logical(Tenths tenths);

};


}   //namespace lomse

#endif    // __LOMSE_SLUR_ENGRAVER_H__


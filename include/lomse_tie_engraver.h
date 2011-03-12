//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_TIE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_TIE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoTie;
class ImoNote;
class GmoShapeTie;
class GmoShapeNote;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class TieEngraver : public RelAuxObjEngraver
{
protected:
    LUnits m_uStaffLeft;
    LUnits m_uStaffRight;
    ImoTie* m_pTie;
    int m_iInstr;
    int m_iStaff;
    ShapeBoxInfo m_shapesInfo[2];
    int m_numShapes;
    ImoNote* m_pStartNote;
    ImoNote* m_pEndNote;
    GmoShapeNote* m_pStartNoteShape;
    GmoShapeNote* m_pEndNoteShape;
    UPoint m_pos;
    UPoint m_points1[4];    //bezier points for first arch
    UPoint m_points2[4];    //bezier points for second arch
    LUnits m_thickness;
    bool m_fTieBelowNote;
    bool m_fTwoArches;

public:
    TieEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                LUnits uStaffLeft, LUnits uStaffRight);
    ~TieEngraver() {}

    void set_start_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol, UPoint pos);
    void set_end_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol);
    int create_shapes();
    int get_num_shapes();
    ShapeBoxInfo* get_shape_box_info(int i);

    inline void set_prolog_width(LUnits width) { m_uStaffLeft += width; }


protected:
    void decide_placement();
    void decide_if_one_or_two_arches();
    inline bool two_arches_needed() { return m_fTwoArches; }
    void create_two_shapes();
    void create_one_shape();

    void compute_start_point();
    void compute_end_point(UPoint* point);
    void compute_start_of_staff_point();
    void compute_end_of_staff_point();
    void compute_default_control_points(UPoint* points);
    void add_user_displacements(int iTie, UPoint* points);
    LUnits tenths_to_logical(Tenths tenths);

};


}   //namespace lomse

#endif    // __LOMSE_TIE_ENGRAVER_H__


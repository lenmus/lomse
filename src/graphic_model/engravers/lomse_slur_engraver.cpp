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

#include "lomse_slur_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_shape_tie.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"


namespace lomse
{

//some macros to improve code reading
#define START       ImoBezierInfo::k_start
#define END         ImoBezierInfo::k_end
#define CTROL1      ImoBezierInfo::k_ctrol1
#define CTROL2      ImoBezierInfo::k_ctrol2

//---------------------------------------------------------------------------------------
// SlurEngraver implementation
//---------------------------------------------------------------------------------------
SlurEngraver::SlurEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           LUnits uStaffLeft, LUnits uStaffRight)
    : RelAuxObjEngraver(libraryScope, pScoreMeter)
    , m_uStaffLeft(uStaffLeft)
    , m_uStaffRight(uStaffRight)
    , m_numShapes(0)
{
}

//---------------------------------------------------------------------------------------
void SlurEngraver::set_start_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                      GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                      int iSystem, int iCol, UPoint pos)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pos = pos;
    m_pSlur = dynamic_cast<ImoSlur*>(pAO);
    //int numNotes = m_pSlur->get_num_objects();
    //m_notes.reserve(numNotes);

    ImoNote* pNote = dynamic_cast<ImoNote*>(pSO);
    GmoShapeNote* pShape = dynamic_cast<GmoShapeNote*>(pStaffObjShape);
    m_notes.push_back( make_pair(pNote, pShape) );

    m_shapesInfo[0].iCol = iCol;
    m_shapesInfo[0].iInstr = iInstr;
    m_shapesInfo[0].iSystem = iSystem;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::set_end_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                   GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                   int iSystem, int iCol)
{
    ImoNote* pNote = dynamic_cast<ImoNote*>(pSO);
    GmoShapeNote* pShape = dynamic_cast<GmoShapeNote*>(pStaffObjShape);
    m_notes.push_back( make_pair(pNote, pShape) );

    m_shapesInfo[1].iCol = iCol;
    m_shapesInfo[1].iInstr = iInstr;
    m_shapesInfo[1].iSystem = iSystem;
}

//---------------------------------------------------------------------------------------
int SlurEngraver::create_shapes()
{
    decide_placement();
    //decide_if_one_or_two_arches();
    //if (two_arches_needed())
    //    create_two_shapes();
    //else
        create_one_shape();
    //return m_numShapes;
    return 1;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::create_one_shape()
{
    m_numShapes = 1;

    compute_ref_point(m_notes[0].second, &m_points1[START]);
    compute_ref_point(m_notes.back().second, &m_points1[END]);
//    compute_start_point();
//    compute_end_point(&m_points1[END]);
    compute_default_control_points(&m_points1[0]);
    //add_user_displacements(0, &m_points1[0]);
    m_shapesInfo[0].pShape = new GmoShapeSlur(m_pSlur, 0, &m_points1[0], m_thickness);

    m_shapesInfo[1].pShape = NULL;
}

////---------------------------------------------------------------------------------------
//void SlurEngraver::create_two_shapes()
//{
//    //m_numShapes = 2;
//
//    ////create first shape
//    //compute_start_point();
//    //compute_end_of_staff_point();
//    //compute_default_control_points(&m_points1[0]);
//    //add_user_displacements(0, &m_points1[0]);
//    //m_shapesInfo[0].pShape = new GmoShapeSlur(m_pSlur, 0, &m_points1[0], m_thickness);
//
//    ////create second shape
//    //compute_end_point(&m_points2[END]);
//    //compute_start_of_staff_point();
//    //compute_default_control_points(&m_points2[0]);
//    //add_user_displacements(1, &m_points2[0]);
//    //m_shapesInfo[1].pShape = new GmoShapeSlur(m_pSlur, 0, &m_points2[0], m_thickness);
//}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_default_control_points(UPoint* points)
{
    LUnits D = (points+END)->x - (points+START)->x;
    LUnits d = D / 5.8f;
    m_thickness = tenths_to_logical(LOMSE_TIE_MAX_THICKNESS);
    LUnits hc = m_thickness * 3.88f;
    (points+CTROL1)->x = (points+START)->x + d;
    (points+CTROL1)->y = (points+START)->y + (m_fSlurBelow ? hc : -hc);

    (points+CTROL2)->x = (points+END)->x - d;
    (points+CTROL2)->y = (points+END)->y + (m_fSlurBelow ? hc : -hc);
}

////---------------------------------------------------------------------------------------
//void SlurEngraver::compute_start_of_staff_point()
//{
//    //m_points2[START].x = m_uStaffLeft;
//    //m_points2[START].y = m_points2[END].y;
//}
//
////---------------------------------------------------------------------------------------
//void SlurEngraver::compute_end_of_staff_point()
//{
//    //m_points1[END].x = m_uStaffRight;
//    //m_points1[END].y = m_points1[START].y;
//}

//---------------------------------------------------------------------------------------
void SlurEngraver::decide_placement()
{
    //[Read, p.266] placement: at notehead side when all notes in same direction.
    //When mixed directions, preferred above

    bool fMixed = false;
    int numNotes = int(m_notes.size());
    bool fPrevIsUp = m_notes[0].second->is_up();
    for (int i=1; i < numNotes; ++i)
    {
        bool fIsUp = m_notes[i].second->is_up();
        if (fIsUp != fPrevIsUp)
        {
            fMixed = true;
            break;
        }
    }

    if (fMixed)
        m_fSlurBelow = false;
    else
        m_fSlurBelow = fPrevIsUp ? true : false;
}

////---------------------------------------------------------------------------------------
//void SlurEngraver::decide_if_one_or_two_arches()
//{
//    //m_fTwoArches = (m_shapesInfo[0].iSystem != m_shapesInfo[1].iSystem);
//}
//

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_ref_point(GmoShapeNote* pNoteShape, UPoint* point)
{
    bool fOnNoteHead = (m_fSlurBelow && pNoteShape->is_up()) ||
                       (!m_fSlurBelow && !pNoteShape->is_up());

    if (fOnNoteHead)
    {
        //x pos: center on notehead
        point->x = (pNoteShape->get_notehead_right() +
                    pNoteShape->get_notehead_left()) / 2.0f;
    }
    else
    {
        //x. pos: center of stem
        point->x = pNoteShape->get_stem_left() + pNoteShape->get_stem_width() / 2.0f;
    }


    //space = 5 tenths appart from y reference point
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);

    if (fOnNoteHead)
    {
        // y pos: notehead border
        point->y = (m_fSlurBelow ?
                        pNoteShape->get_notehead_bottom() + space
                        : pNoteShape->get_notehead_top() - space );
    }
    else
    {
        //y pos: top/bottom of note shape
        point->y = (pNoteShape->is_up() ?
                        pNoteShape->get_top() - space
                        : pNoteShape->get_bottom() + space );
    }
}

////---------------------------------------------------------------------------------------
//void SlurEngraver::add_user_displacements(int iTie, UPoint* points)
//{
//    //ImoBezierInfo* pBezier = (iTie == 0 ? m_pSlur->get_start_bezier()
//    //                                    : m_pSlur->get_stop_bezier() );
//    //if (pBezier)
//    //{
//    //    for (int i=0; i < 4; i++)
//    //    {
//    //        (points+i)->x += tenths_to_logical(pBezier->get_point(i).x);
//    //        (points+i)->y += tenths_to_logical(pBezier->get_point(i).y);
//    //    }
//    //}
//}

//---------------------------------------------------------------------------------------
LUnits SlurEngraver::tenths_to_logical(Tenths tenths)
{
    return m_pMeter->tenths_to_logical(tenths, m_iInstr, m_iStaff);
}

//---------------------------------------------------------------------------------------
int SlurEngraver::get_num_shapes()
{
    return m_numShapes;
}

//---------------------------------------------------------------------------------------
ShapeBoxInfo* SlurEngraver::get_shape_box_info(int i)
{
    return &m_shapesInfo[i];
}


}  //namespace lomse

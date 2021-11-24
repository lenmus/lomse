//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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
class InstrumentEngraver;

//---------------------------------------------------------------------------------------
class SlurEngraver : public RelObjEngraver
{
protected:
    InstrumentEngraver* m_pInstrEngrv;
    LUnits m_uStaffTop = 0.0f;         //top line of current system

    int m_numShapes = 0;
    ImoSlur* m_pSlur = nullptr;
    bool m_fSlurBelow = false;
    bool m_fSlurForGraces = false;
    LUnits m_uPrologWidth = 0.0f;

    ImoNote* m_pStartNote = nullptr;
    ImoNote* m_pEndNote = nullptr;
    GmoShapeNote* m_pStartNoteShape = nullptr;
    GmoShapeNote* m_pEndNoteShape = nullptr;
    int m_idxStaffStart = 0;
    int m_idxStaffEnd = 0;


    bool m_fTwoArches = false;
    UPoint m_points[4];    //bezier points for current shape
    LUnits m_thickness = 0.0f;

    std::vector<UPoint> m_dataPoints;   //intermediate ref. points
    Color m_dbgColor = Color(255,0,0);  //debug: color por bezier control points
    UPoint m_dbgPeak;                   //debug: bezier point at peak point

public:
    SlurEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 InstrumentEngraver* pInstrEngrv);
    ~SlurEngraver() {}

    void set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;
    void set_end_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;

    //RelObjEngraver mandatory overrides
    GmoShape* create_first_or_intermediate_shape(const RelObjEngravingContext& ctx) override;
    GmoShape* create_last_shape(const RelObjEngravingContext& ctx) override;

protected:
    void decide_placement();
    inline bool is_end_point_set() { return m_pEndNote != nullptr; }
    GmoShape* create_shape(int type);

    enum { k_system_start=0, k_system_end, k_system_full, k_single_shape, };
    void compute_start_end_points(int type);
    void apply_rules_for_start_end_points(int type);
    void compute_control_points(int type);
    //void add_user_displacements(int iSlur, UPoint* points);
    void find_contour_reference_points();
    void compute_baseline();

    void compute_start_point();
    void compute_end_point();
    void compute_end_point_for_graces();
    void compute_start_of_staff_point();
    void compute_end_of_staff_point();
    GmoShapeNote* get_relevant_note_for_chords(GmoShapeNote* pNoteShape);

    void compute_two_points_slur(int type);
    void compute_many_points_slur(int type);
    void improve_low_slur(LUnits dist);

    //different methods to compute beziers
    void method_for_two_points();
    void compute_default_slur();
    void method_asymmetrical_slur(LUnits height);

    //helper, for adjusting the bezier
    void shift_bezier_up_down(LUnits dist);
    UPoint determine_peak_point();
    LUnits check_if_slur_exceeds_all_reference_points();
    void compute_approximate_arc();

//    //helper
//    LUnits determine_beam_height(ImoNote* pNote);


    //auxiliary, for intermediate computations
    bool m_fStartOverStem = false;      //slur attached to stem at start point
    bool m_fEndOverStem = false;        //slur attached to stem at end point

    //slur baseline data, for intermediate computations
    LUnits uD = 0.0f;         // length of slur baseline, in logical units
    LUnits D = 0.0f;          // length of slur baseline, in tenths
    LUnits height = 0.0f;     // height of slur in logical units (h)
    LUnits cosb = 1.0f;       // cos(baseline angle)
    LUnits sinb = 1.0f;       // sin(baseline angle))

    //slur control points, during intermediate computations, to simplify code writting
    LUnits x0, y0;      //start point
    LUnits x1, y1;      //first ctrol point, near start point
    LUnits x2, y2;      //second ctrol point, near end point
    LUnits x3, y3;      //end point

    //slur middle point and approx. arc, for intermediate computations
    LUnits xm, ym;      //bezier middle point (t=0.5)
    LUnits xc, yc;      //center of circle
    LUnits r = 0.0f;    //radius

    //UNUSED: Letft here until slur layout is more advanced, just in case some of
    //        these methods could still be needed. Some of them took time to write
//    void increment_bezier_height(LUnits dist);
//    float compute_bezier(float y0, float y1, float y2, float y3, float t);
//    void method_double_max_angles(int type);
//    void method_old(int type);
//    void method_symmetrical_flat(int type);
//    static float solve_cubic(float a, float b, float c, float d);
//    void find_peak_point_and_shift();

};


}   //namespace lomse

#endif    // __LOMSE_SLUR_ENGRAVER_H__


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

#ifndef __LOMSE_BEAM_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_BEAM_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_vertical_profile.h"

#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class GmoShapeBeam;
class ScoreMeter;
class ImoBeam;
class ImoNote;
class GmoShapeNote;
class ImoNoteRest;
class GmoShape;
class VerticalProfile;


//---------------------------------------------------------------------------------------
class BeamEngraver : public RelObjEngraver
{
protected:
    GmoShapeBeam* m_pBeamShape;
    ImoBeam* m_pBeam;
    std::list< pair<ImoNoteRest*, GmoShape*> > m_noteRests;

    std::list<LUnits> m_segments;
    UPoint m_origin;
    USize m_size;
    LUnits m_uBeamThickness;
    bool m_fBeamAbove;
	UPoint m_outerLeftPoint;
    UPoint m_outerRightPoint;

public:
    BeamEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~BeamEngraver();

    //implementation of virtual methods from RelObjEngraver
    void set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol,
                            LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                            int idxStaff, VerticalProfile* pVProfile) override;
    void set_middle_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                             GmoShape* pStaffObjShape, int iInstr, int iStaff,
                             int iSystem, int iCol,
                             LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                             int idxStaff, VerticalProfile* pVProfile) override;
    void set_end_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol,
                          LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                          int idxStaff, VerticalProfile* pVProfile) override;

    //RelObjEngraver mandatory overrides
    void set_prolog_width(LUnits UNUSED(width)) override {}
    GmoShape* create_first_or_intermediate_shape(Color color=Color(0,0,0)) override;
    GmoShape* create_last_shape(Color color=Color(0,0,0)) override;

    //During layout, when more space is added between two staves, all shapes are shifted
    //down to account for the added extra space. If a beam is cross-staff, it is
    //necessary to increment the lenght of all stems conected to this beam
    void increment_cross_staff_stems(LUnits yIncrement);

protected:
    void create_shape();
    void add_shape_to_noterests();
    void reposition_rests();
    void decide_on_stems_direction();
    void decide_beam_position();
    void change_stems_direction();
    void adjust_stems_lengths();
    void compute_beam_segments();
	void add_segment(LUnits uxStart, LUnits uyStart, LUnits uxEnd, LUnits uyEnd);
    void update_bounds(LUnits uxStart, LUnits uyStart, LUnits uxEnd, LUnits uyEnd);
    void make_segments_relative();

    bool m_fStemForced;     //at least one stem forced
    bool m_fStemsMixed;     //not all stems in the same direction
    bool m_fStemsDown;      //stems direction down
    bool m_fCrossStaff;     //the beamed group is cross-staff (= has notes on several staves)
    int m_numStemsDown;     //number of noteheads with stem down
    int m_numNotes;         //total number of notes
    int m_averagePosOnStaff;
    int m_maxStaff;         //for cross-staff beams, the highest staff. For normal beams, just the staff
};


}   //namespace lomse

#endif    // __LOMSE_BEAM_ENGRAVER_H__


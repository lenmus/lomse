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

#ifndef __LOMSE_BEAM_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_BEAM_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
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


//---------------------------------------------------------------------------------------
class BeamEngraver : public RelAuxObjEngraver
{
protected:
    int m_iInstr;
    int m_iStaff;
    GmoShapeBeam* m_pBeamShape;
    ImoBeam* m_pBeam;
    std::list< pair<ImoNoteRest*, GmoShape*> > m_noteRests;

    ShapeBoxInfo m_shapesInfo[2];
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

    //implementation of virtual methods from RelAuxObjEngraver
    void set_start_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol, UPoint pos);
    void set_middle_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                             GmoShape* pStaffObjShape, int iInstr, int iStaff,
                             int iSystem, int iCol);
    void set_end_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol);
    int create_shapes();
    int get_num_shapes() { return 1; }
    ShapeBoxInfo* get_shape_box_info(int i) { return &m_shapesInfo[0]; }


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

    bool m_fStemForced;     //at least one stem forced
    bool m_fStemMixed;      //not all stems in the same direction
    bool m_fStemsDown;      //stems direction down
    int m_numStemsDown;     //number of noteheads with stem down
    int m_numNotes;         //total number of notes
    int m_averagePosOnStaff;
};


}   //namespace lomse

#endif    // __LOMSE_BEAM_ENGRAVER_H__


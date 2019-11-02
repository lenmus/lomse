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

#ifndef __LOMSE_BOX_SYSTEM_H__
#define __LOMSE_BOX_SYSTEM_H__

#include "lomse_basic.h"
#include "lomse_gm_basic.h"

//using namespace std;

namespace lomse
{

//forward declarations
class GmoBoxScorePage;
class TimeGridTable;
class GmMeasuresTable;
class GmoBoxSliceStaff;
class SystemLayouter;

//---------------------------------------------------------------------------------------
/** GmoBoxSystem represents a line of music in the printed score.
*/
class GmoBoxSystem : public GmoBox
{
protected:
	vector<GmoShapeStaff*> m_staffShapes;
	vector<int> m_firstStaff;       //index to first staff for each instrument
    TimeGridTable* m_pGridTable;
    int m_iPage;            //number of score page (0..n-1) in which this system is contained

	vector<int> m_iFirstMeasure;    //index to first measure, per instrument
    vector<int> m_nMeasures;        //number of measures in this system, per instrument
    LUnits m_dxFirstMeasure;        //shift from box left (virtual end barline of previous measure)

public:
    GmoBoxSystem(ImoScore* pScore);
    ~GmoBoxSystem();

    //helpers for layout
    /**  Move boxes and shapes to theirs final 'y' positions. */
    void reposition_slices_and_shapes(const std::vector<LUnits>& yOrgShifts,
                                      std::vector<LUnits>& heights,
                                      vector<LUnits>& barlinesHeight,
                                      SystemLayouter* pSysLayouter);

    //slices
	inline int get_num_slices() const { return (int)m_childBoxes.size(); }
    inline GmoBoxSlice* get_slice(int i) const { return (GmoBoxSlice*)m_childBoxes[i]; }
    GmoBoxSliceInstr* get_first_instr_slice(int iInstr);
    GmoBoxSliceStaff* get_first_slice_staff_for(int iInstr, int iStaff);

    //grid table: xPositions/timepos
    inline void set_time_grid_table(TimeGridTable* pGridTable) { m_pGridTable = pGridTable; }
    inline TimeGridTable* get_time_grid_table() { return m_pGridTable; }
    TimeUnits start_time();
    TimeUnits end_time();
    LUnits get_x_for_note_rest_at_time(TimeUnits timepos);
    LUnits get_x_for_barline_at_time(TimeUnits timepos);

	//miscellaneous info
    GmoShapeStaff* get_staff_shape(int absStaff);
    GmoShapeStaff* get_staff_shape(int iInstr, int iStaff);
    int instr_number_for_staff(int absStaff);
    int staff_number_for(int absStaff, int iInstr);
    inline void set_page_number(int iPage) { m_iPage = iPage; }
	inline int get_page_number() { return m_iPage; }

	//measures info
	/** Returns the index (0..n-1) for the measure that starts in this system, or
	    -1 if no measures in this instrument
    */
	int get_first_measure(int iInstr);

	/** Lomse considers two positions for barlines at the end of a system:
	    - the real position at end of previous system, and
	    - the virtual position at start of next system (the first
          available position after prolog).

        This method returns the virtual start position for the mesaure starting
        the system.
	*/
    inline LUnits get_start_measure_xpos() { return m_origin.x + m_dxFirstMeasure; }

    //Staff shapes
    GmoShapeStaff* add_staff_shape(GmoShapeStaff* pShape);
    void add_num_staves_for_instrument(int staves);
    inline vector<GmoShapeStaff*>& get_staff_shapes() { return m_staffShapes; }

    //hit tests related
    int nearest_staff_to_point(LUnits y);

    //helper. User API related
    int get_num_instruments();
    LUnits tenths_to_logical(Tenths value, int iInstr=0, int iStaff=0);

    //debug
    string dump_timegrid_table();
    string dump_measures_info();

protected:
    //building measures table
    friend class TimeSlice;
    void add_barline_info(int iMeasure, int iInstr);

    friend class SystemLayouter;
    inline void add_shift_to_start_measure(LUnits width) { m_dxFirstMeasure += width; }


};



}   //namespace lomse

#endif      //__LOMSE_BOX_SYSTEM_H__

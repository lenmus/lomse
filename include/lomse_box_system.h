//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_BOX_SYSTEM_H__
#define __LOMSE_BOX_SYSTEM_H__

#include "lomse_basic.h"
#include "lomse_gm_basic.h"

//using namespace std;

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class GmoBoxScorePage;
class TimeGridTable;
class GmMeasuresTable;
class GmoBoxSliceStaff;
class SystemLayouter;

//---------------------------------------------------------------------------------------
/** %GmoBoxSystem is a container for all boxes and shapes that represents a line of music
    in the printed score. Its bounding box encloses all the notation for the system.

    %GmoBoxSystem is also responsible for storing and managing a table with
    the relation timepos --> position for all occupied timepos in the score.

    The recorded positions are for the center of note heads or rests. The last position
    is for the barline (if exists).

    This object is responsible for supplying all valid timepos and their positions
    so that other objects could, for instance:
        a) Determine the timepos to assign to a mouse click in a certain position.
        b) Draw a grid of valid timepos
        c) To determine the position for a beat.

    Important: There can exist many entries for a given timepos, the first ones are the
    x position for the non-timed staffobjs, and the last one is
    the x position for the notes/rests at that timepos. For example,
    a barline and the next note do have the same timepos, but they are placed at
    different positions. This also happens when there exist non-timed staffobjs, such as
    clefs, key  signatures and time signatures.
*/
class GmoBoxSystem : public GmoBox
{
protected:
	vector<GmoShapeStaff*> m_staffShapes;
	vector<int> m_firstStaff;       //index to first staff for each instrument
    TimeGridTable* m_pGridTable;
    int m_iPage;        //index of score page (0..n-1) in which this system is contained
    int m_iSystem;      //index of this system in the score (0..n-1)

	vector<int> m_iFirstMeasure;    //index to first measure, per instrument
    vector<int> m_nMeasures;        //number of measures in this system, per instrument
    LUnits m_dxFirstMeasure;        //shift from box left (virtual end barline of previous measure)

    //free vertical space at top and bottom
    LUnits m_uFreeAtTop = 0.0f;
    LUnits m_uFreeAtBottom = 0.0f;

public:
    ///@cond INTERNALS
    //excluded from public API. Only for internal use.
    GmoBoxSystem(ImoScore* pScore);
    ~GmoBoxSystem();
    ///@endcond


	//measures info
    /// @name Information about measures
    //@{

	/** Returns the index (0..n-1) for the measure that starts in this system, or
	    -1 if no measures in this instrument.
	    This index has nothing to do with the displayed measure number. First measure
	    has always index 0 regardless of whether the first measure is measure number 1
	    or is an incomplete measure (the score begins on pickup).

	    @param iInstr The index (0..n-1), in current layout, for instrument to which this
	        request refer to. Take into account that in polymetric music (music in which
	        not all instruments have the same time signature) the measure number is not
	        a common value for all instruments and, thus measure numbers in a system
	        will depend on which instrument you refer to. Thus, it is necessary to
	        specify an instrument.
    */
	int get_first_measure(int iInstr);

	/** Returns the number of measures in this system for an instrument.

	    @param iInstr The index (0..n-1), in current layout, for instrument to which this
	        request refer to. Take into account that in polymetric music (music in which
	        not all instruments have the same time signature) the number of measures is
	        not a common value for all instruments and, thus the number of measures in
	        a system will depend on which instrument you refer to. Thus, it is necessary
	        to specify an instrument.
    */
	int get_num_measures(int iInstr);

	/** Lomse considers two positions for barlines when they are at the end of a system:
	    - the real position at end of a system, and
	    - the virtual position at start of next system (the first
          available position after prolog).

        This method returns the virtual start position for the measure starting
        the system.
	*/
    inline LUnits get_start_measure_xpos() { return m_origin.x + m_dxFirstMeasure; }

    //@}    //measures info


	//staves
    /// @name Information about staves
    //@{

    /** Returns the shape object representing an staff.
        @param absStaff Absolute index (0..n-1) to the desired staff. The top staff
            in a system has index 0, the next one below it has index 1, and so on.
    */
    GmoShapeStaff* get_staff_shape(int absStaff);

    /** Returns the shape object representing an staff.
	    @param iInstr The index (0..n-1), in current layout, for the instrument to which
	        this request refer to.
        @param iStaff The index (0..n-1) to the desired staff referred to the instrument
            iInstr. The top staff in an instrument has index 0, the next one below it has
            index 1, and so on.
    */
    GmoShapeStaff* get_staff_shape(int iInstr, int iStaff);

    /** Returns the instrument index (0..n-1), in current layout, to which a staff
        belongs.
        @param absStaff Absolute index (0..n-1) of the staff to which this request refer
            to. The top staff in a system has index 0, the next one below it has index
            1, and so on.
    */
    int instr_number_for_staff(int absStaff);

    /** Helper method to convert absolute staff index into a relative one.
        Once you know the instrument to which a staff belongs (by using method
        instr_number_for_staff() ) this method allows to determine the relative staff
        index (0..n-1) referred to the instrument.
    */
    int staff_number_for(int absStaff, int iInstr);

    //@}    //staves


	//timepos
    /// @name Information about time positions and coordiantes
    //@{

    /** Returns the timepos at start of this system. */
    TimeUnits start_time();

    /** Returns the timepos at end of this system. */
    TimeUnits end_time();

    /** Returns the x position for the given timepos. This method only takes notes and
        rests into account, ignoring other staff objects that could exist
        at the same timepos in different locations, such as a barline or a clef before
        the note/rest. The returned value is the x position at which notes/rests are
        vertically aligned with notes/rests at the same timepos in other staves.
        If there are no notes/rests at the requested timepos, this method provides an
        approximated interpolated value.

        The returned value is in logical units, relative to GmoDocPage origin.

        @param timepos Absolute time units for the requested position.

        See get_x_for_barline_at_time()
    */
    LUnits get_x_for_note_rest_at_time(TimeUnits timepos);

    /** Returns the x position for the given timepos. This method takes only barlines
        into account, ignoring other staff objects that could exist at the same timepos
        (e.g. any staffobj after the barline). Therefore, the returned value is the x
        position of the first barline found at the provided @c timepos.

        The returned value is in logical units, relative to GmoDocPage origin.

        @param timepos Absolute time units for the requested position.

        See get_x_for_note_rest_at_time()
    */
    LUnits get_x_for_barline_at_time(TimeUnits timepos);

    //@}    //timepos


///@cond INTERNALS
//excluded from public API. Only for internal use.

    //helpers for layout
    inline void set_top_limit(LUnits minLimit) { m_uFreeAtTop = minLimit - get_top(); }
    inline void set_bottom_limit(LUnits maxLimit) { m_uFreeAtBottom = get_bottom() - maxLimit; }
    inline LUnits get_free_space_at_top() { return m_uFreeAtTop; }
    inline LUnits get_free_space_at_bottom() { return m_uFreeAtBottom; }
    inline void set_free_space_at_top(LUnits space) { m_uFreeAtTop = space; }
    inline void set_free_space_at_bottom(LUnits space) { m_uFreeAtBottom = space; }

    /**  Move boxes and shapes to theirs final 'y' positions. */
    void reposition_slices_and_shapes(const std::vector<LUnits>& yOrgShifts,
                                      const std::vector<LUnits>& heights,
                                      const std::vector<LUnits>& barlinesHeight,
                                      const std::vector<std::vector<LUnits>>& relStaffTopPositions,
                                      LUnits bottomMarginIncr,
                                      SystemLayouter* pSysLayouter);

    //slices
	inline int get_num_slices() const { return (int)m_childBoxes.size(); }
    inline GmoBoxSlice* get_slice(int i) const { return (GmoBoxSlice*)m_childBoxes[i]; }
    GmoBoxSliceInstr* get_first_instr_slice(int iInstr);
    GmoBoxSliceInstr* find_instr_slice_at(LUnits x, LUnits y);
    GmoBoxSliceStaff* get_first_slice_staff_for(int iInstr, int iStaff);
    void reposition_slices(USize shift);
    void remove_free_space_at_bottom_and_adjust_slices();

    //grid table: xPositions/timepos
    inline void set_time_grid_table(TimeGridTable* pGridTable) { m_pGridTable = pGridTable; }
    inline TimeGridTable* get_time_grid_table() { return m_pGridTable; }

	//miscellaneous info
	inline int get_system_number() { return m_iSystem; }
    inline void set_page_number(int iPage) { m_iPage = iPage; }
	inline int get_page_number() { return m_iPage; }

    //Staff shapes
    GmoShapeStaff* add_staff_shape(GmoShapeStaff* pShape);
    void add_num_staves_for_instrument(int staves);
    inline vector<GmoShapeStaff*>& get_staff_shapes() { return m_staffShapes; }

    //helper. User API related
    int get_num_instruments();
    LUnits tenths_to_logical(Tenths value, int iInstr=0, int iStaff=0);

    //hit tests related
    int staff_at(LUnits y);

    //debug
    string dump_timegrid_table();
    string dump_measures_info();

///@endcond

protected:
    //building measures table
    friend class TimeSlice;
    void add_barline_info(int iMeasure, int iInstr);

    friend class SystemLayouter;
    inline void add_shift_to_start_measure(LUnits width) { m_dxFirstMeasure += width; }

    friend class GmoBoxScorePage;
    inline void set_system_number(int iSystem) { m_iSystem = iSystem; }

    //overrides
    void draw_box_bounds(Drawer* pDrawer, double xorg, double yorg, Color& color) override;

};



}   //namespace lomse

#endif      //__LOMSE_BOX_SYSTEM_H__

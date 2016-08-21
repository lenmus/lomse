//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#ifndef __LOMSE_SPACING_ALGORITHM_GOURLAY_H__        //to avoid nested includes
#define __LOMSE_SPACING_ALGORITHM_GOURLAY_H__

#include "lomse_basic.h"
#include "lomse_logger.h"
#include "lomse_spacing_algorithm.h"
#include "lomse_time.h"
#include "lomse_timegrid_table.h"

//For performance measurements (timing)
#include <ctime>   //clock
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;


namespace lomse
{

//forward declarations
class ImoScore;
class ScoreMeter;
class LibraryScope;
class ScoreLayouter;
class ShapesStorage;
class ShapesCreator;
class PartsEngraver;
class GmoBoxSlice;
class ColStaffObjsEntry;
class TimeGridTable;
class GmoBoxSliceInstr;

class ColumnDataGourlay;
class TimeSlice;
class StaffObjData;

//---------------------------------------------------------------------------------------
// SpAlgGourlay
// Spacing algorithm based on Gourlay's method
//
class SpAlgGourlay : public SpAlgColumn
{
protected:
    LibraryScope&   m_libraryScope;
    ScoreMeter*     m_pScoreMeter;
    ScoreLayouter*  m_pScoreLyt;
    ImoScore*       m_pScore;
    ShapesStorage&  m_shapesStorage;
    ShapesCreator*  m_pShapesCreator;
    PartsEngraver*  m_pPartsEngraver;

    list<TimeSlice*> m_slices;              //list of TimeSlices
    vector<ColumnDataGourlay*> m_columns;   //columns
    vector<StaffObjData*> m_data;           //data associated to each staff object

    //auxiliary temporal variables used while collecting columns' data
    TimeSlice*   m_pCurSlice;
    ColStaffObjsEntry*  m_pLastEntry;
    int                 m_prevType;
    TimeUnits           m_prevTime;
    int                 m_numEntries;
    ColumnDataGourlay*  m_pCurColumn;
    int                 m_numSlices;
    int                 m_iPrevColumn;

    //data collected for each slice
    TimeUnits   m_maxNoteDur;
    TimeUnits   m_minNoteDur;


public:
    SpAlgGourlay(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                       ScoreLayouter* pScoreLyt, ImoScore* pScore,
                       ShapesStorage& shapesStorage, ShapesCreator* pShapesCreator,
                       PartsEngraver* pPartsEngraver);
    virtual ~SpAlgGourlay();


    //spacing algorithm main actions
    void do_spacing(int iCol, bool fTrace=false, int level=k_trace_off);
    void justify_system(int iFirstCol, int iLastCol, LUnits uSpaceIncrement);

    //for lines break algorithm
    float determine_penalty_for_line(int iSystem, int i, int j);
    bool is_better_option(float prevPenalty, float newPenalty, float nextPenalty,
                          int i, int j);

    //information about a column
    bool is_empty_column(int iCol);
    LUnits get_column_width(int iCol);
    bool column_has_barline_at_end(int iCol);

    //methods to compute results
    TimeGridTable* create_time_grid_table_for_column(int iCol);

    //debug
    void dump_column_data(int iCol, ostream& outStream);

    //column creation: collecting content
    void start_column_measurements(int iCol);
    void include_object(ColStaffObjsEntry* pCurEntry, int iCol, int iLine, int iInstr,
                        ImoStaffObj* pSO, TimeUnits rTime, int nStaff, GmoShape* pShape,
                        bool fInProlog=false);
    void finish_column_measurements(int iCol);

    //auxiliary: shapes and boxes
    void add_shapes_to_box(int iCol, GmoBoxSliceInstr* pSliceInstrBox, int iInstr);
    void delete_shapes(int iCol);
    void reposition_slices_and_staffobjs(int iFirstCol, int iLastCol,
            LUnits yShift,
            LUnits* yMin, LUnits* yMax);

protected:
    void new_column(TimeSlice* pSlice);
    void new_slice(ColStaffObjsEntry* pEntry, int entryType, int iColumn, int iData,
                   bool fInProlog);
    void finish_slice(ColStaffObjsEntry* pLastEntry, int numEntries);
    void compute_springs();
    void order_slices_in_columns();
    void apply_force(float F);

};

//---------------------------------------------------------------------------------------
// ColumnDataGourlay
// A column is the set of slices between two feasible break points.
// This objects contains data associated to each column.
//
class ColumnDataGourlay
{
public:
    TimeSlice* m_pFirstSlice;            //first slice in natural order
    vector<TimeSlice*> m_orderedSlices;  //slices ordered by pre-stretching force fi

    float   m_slope;            //slope of aproximated sff() for this column
    LUnits  m_xFixed;           //fixed spacing for this column
    LUnits  m_colWidth;         //current col. width after having applying force
    LUnits  m_colMinWidth;      //minimum width (force 0)
    bool    m_fBarlineAtEnd;    //true if last slice is barline


    ColumnDataGourlay(TimeSlice* pSlice);
    ~ColumnDataGourlay();

    //creation
    void set_num_entries(int numSlices);
    void order_slices();
    void determine_minimum_width();

    //spacing
    LUnits determine_extent_for(float force);
    float determine_force_for(LUnits width);
    void determine_approx_sff_for(float force);
    void apply_force(float F);

    //access to position and spacing data
    inline LUnits get_column_width() { return m_colWidth; }
    inline LUnits get_minimum_width() { return m_colMinWidth; }

    //other information
    inline int num_slices() { return int(m_orderedSlices.size()); }
    bool is_empty_column();
    inline bool has_barline_at_end() { return m_fBarlineAtEnd; }

    //managing shapes
    void add_shapes_to_box(GmoBoxSliceInstr* pSliceInstrBox, int iInstr,
                           vector<StaffObjData*>& data);
    void delete_shapes(vector<StaffObjData*>& data);
    void move_shapes_to_final_positions(vector<StaffObjData*>& data, LUnits xPos,
                                        LUnits yPos, LUnits* yMin, LUnits* yMax);

    //debug
    void dump(ostream& outStream, bool fOrdered=false);

protected:

};


//---------------------------------------------------------------------------------------
// StaffObjData
// Spacing data associated to an staff object.
class StaffObjData
{
public:
    GmoShape*   m_pShape;       //shape for this staff object
    LUnits m_xUserShift;
    LUnits m_yUserShift;

public:
    StaffObjData();
    virtual ~StaffObjData();

    inline GmoShape* get_shape() { return m_pShape; }
};


//---------------------------------------------------------------------------------------
// TimeSlice
// An slice is the set of staff objects that must be aligned at the same timepos.
// This object contains data associated to an slice.
class TimeSlice
{
protected:
    friend class SpAlgGourlay;
    friend class ColumnDataGourlay;
    friend class TimeSliceNonTimed;
    ColStaffObjsEntry*  m_firstEntry;
    ColStaffObjsEntry*  m_lastEntry;
    int         m_iFirstData;   //index to first StaffObjData element for this slice
    int         m_numEntries;   //num staffobjs in this slice
    int         m_type;         //type of slice. Value from enum ESliceType
    int         m_iColumn;      //Column in which this slice is included
    bool        m_fInProlog;    //this slice is part of the score prolog

    //list
    TimeSlice* m_next;
    TimeSlice* m_prev;

    //data for spacing
    LUnits  m_xLi;          //width of left rod
    LUnits  m_xRi;          //width of right rod
    float   m_fi;           //pre-stretching force
    float   m_ci;           //spring constant
    LUnits  m_width;        //final extent after applying force
    LUnits  m_xLeft;        //fixed space at start (a rod in the springs join)

    //auxiliary
    TimeUnits   m_ds;       //spring duration (= timepos(next_slice) - timepos(this_slice))
    TimeUnits   m_di;       //shortest duration in this segment or still sounding in this segment
    TimeUnits   m_minNote;      //min note/rest duration in this segment
    TimeUnits   m_minNoteNext;  //min note/rest duration still sounding in next segment

public:
    TimeSlice(ColStaffObjsEntry* pEntry, int entryType, int column,
                     int iData, bool fInProlog);
    virtual ~TimeSlice();

    enum ESliceType {
        k_undefined = -1,
        k_prolog = 0,
        k_non_timed,
        k_noterest,
        k_barline,
    };

    //creation
    void set_final_data(ColStaffObjsEntry* pLastEntry, int numEntries,
                        TimeUnits maxNextTime, TimeUnits minNote);

    //list creation
    inline TimeSlice* next() { return m_next; }
    inline void set_next(TimeSlice* pSlice) { m_next = pSlice; }

    //spacing
    void compute_spring_data(LUnits uSmin, float alpha, float log2dmin, TimeUnits dmin,
                             bool fProportional, LUnits dsFixed);
    virtual void assign_spacing_values(vector<StaffObjData*>& data, ScoreMeter* pMeter);
    void apply_force(float F);
    inline void increment_xRi(LUnits value) { m_xRi += value; }
    inline void set_minimum_xi(LUnits value) {
        if (get_xi() < value)
            m_xRi = value - m_xLi;
    }

    //basic information
    inline int get_type() { return m_type; }

    //access to information
    inline LUnits get_xi() { return m_xLi + m_xRi; }
    inline LUnits get_minimum_extent() { return m_xLi + m_xRi + m_xLeft; }
    TimeUnits get_timepos();
    inline int get_num_entries() { return m_numEntries; }
    inline ColStaffObjsEntry* get_first_entry() { return m_firstEntry; }
    inline ColStaffObjsEntry* get_last_entry() { return m_lastEntry; }

    //operations
    void add_shapes_to_box(GmoBoxSliceInstr* pSliceInstrBox, int iInstr,
                           vector<StaffObjData*>& data);
    void delete_shapes(vector<StaffObjData*>& data);
    virtual void move_shapes_to_final_positions(vector<StaffObjData*>& data, LUnits xPos,
                                                LUnits yPos, LUnits* yMin, LUnits* yMax);
    //access to information
    inline LUnits get_width() { return m_width; }

    //debug
    void dump(ostream& ss);
    static void dump_header(ostream& ss);

protected:
    void compute_smallest_duration_di(TimeUnits minNotePrev);
    void find_smallest_note_soundig_at(TimeUnits nextTime);
    void compute_spring_constant(LUnits uSmin, float alpha, float log2dmin,
                                 TimeUnits dmin, bool fProportional, LUnits dsFixed);
    void compute_pre_stretching_force();
    LUnits spacing_function(LUnits uSmin, float alpha, float log2dmin, TimeUnits dmin);
    inline TimeUnits get_min_note_still_sounding() { return m_minNoteNext; }
    bool is_last_slice_in_prolog();

};


//---------------------------------------------------------------------------------------
// TimeSliceProlog
// An slice for the prolog objects
class TimeSliceProlog : public TimeSlice
{
protected:
    LUnits m_clefsWidth;
    LUnits m_timesWidth;
    LUnits m_keysWidth;
    LUnits m_spaceBefore;

public:
    TimeSliceProlog(ColStaffObjsEntry* pEntry, int entryType, int column, int iData);
    virtual ~TimeSliceProlog();

    //overrides
    void assign_spacing_values(vector<StaffObjData*>& data, ScoreMeter* pMeter);
    void move_shapes_to_final_positions(vector<StaffObjData*>& data, LUnits xPos,
                                        LUnits yPos, LUnits* yMin, LUnits* yMax);

};


//---------------------------------------------------------------------------------------
// TimeSliceNonTimed
// An slice for non-timed objects not at prolog
class TimeSliceNonTimed : public TimeSlice
{
protected:
    int m_numStaves;
    LUnits m_realWidth;
    LUnits m_interSpace;
    vector<LUnits> m_widths;    //total width for objects on each staff

public:
    TimeSliceNonTimed(ColStaffObjsEntry* pEntry, int entryType, int column, int iData);
    virtual ~TimeSliceNonTimed();

    //overrides
    void assign_spacing_values(vector<StaffObjData*>& data, ScoreMeter* pMeter);
    void move_shapes_to_final_positions(vector<StaffObjData*>& data, LUnits xPos,
                                        LUnits yPos, LUnits* yMin, LUnits* yMax);

};


}   //namespace lomse

#endif    // __LOMSE_SPACING_ALGORITHM_GOURLAY_H__


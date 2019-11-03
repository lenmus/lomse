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

#ifndef __LOMSE_SPACING_ALGORITHM_GOURLAY_H__        //to avoid nested includes
#define __LOMSE_SPACING_ALGORITHM_GOURLAY_H__

#include "lomse_basic.h"
#include "lomse_logger.h"
#include "lomse_spacing_algorithm.h"
#include "lomse_time.h"
#include "lomse_timegrid_table.h"

namespace lomse
{

//forward declarations
class ImoScore;
class ScoreMeter;
class LibraryScope;
class ScoreLayouter;
class EngraversMap;
class ShapesCreator;
class PartsEngraver;
class GmoBoxSlice;
class ColStaffObjsEntry;
class TimeGridTable;
class GmoBoxSliceInstr;
class ImoLyric;
class ColumnDataGourlay;
class TimeSlice;
class StaffObjData;
class FullMeasureRestData;
class TextMeter;
class ImoStyle;
class GmMeasuresTable;
class GraphicModel;


//---------------------------------------------------------------------------------------
// SpAlgGourlay
// Spacing algorithm based on Gourlay's method
//
class SpAlgGourlay : public SpAlgColumn
{
protected:
    list<TimeSlice*> m_slices;              //list of TimeSlices
    vector<ColumnDataGourlay*> m_columns;   //columns
    vector<StaffObjData*> m_data;           //data associated to each staff object

    //auxiliary temporal variables used while collecting columns' data
    TimeSlice*          m_pCurSlice;
    ColStaffObjsEntry*  m_pLastEntry;
    int                 m_prevType;
    TimeUnits           m_prevTime;
    int                 m_numEntries;
    ColumnDataGourlay*  m_pCurColumn;
    int                 m_numSlices;
    int                 m_iPrevColumn;

    //auxiliary, for controlling objects to include in a prolog slice
    TimeUnits    m_lastPrologTime;
    vector<bool> m_prologClefs;
    vector<bool> m_prologKeys;
    vector<bool> m_prologTimes;

    //data collected for each slice
    TimeUnits   m_maxNoteDur;
    TimeUnits   m_minNoteDur;

    //spacing parameters
	LUnits m_uSmin;     //minimun space between notes
    float  m_alpha;     //alpha parameter for Gourlay's formula
    float  m_dmin;      //min note duration for which fixed spacing will be used
    float  m_log2dmin;  //precomputed value for log2(dmin)
    float  m_Fopt;      //Optimum force (user defined and dependent on personal taste)

public:
    SpAlgGourlay(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 ScoreLayouter* pScoreLyt, ImoScore* pScore,
                 EngraversMap& engravers, ShapesCreator* pShapesCreator,
                 PartsEngraver* pPartsEngraver);
    virtual ~SpAlgGourlay();


    //spacing algorithm main actions
    void do_spacing(int iCol, bool fTrace=false);
    void justify_system(int iFirstCol, int iLastCol, LUnits uSpaceIncrement);

    //for lines break algorithm
    float determine_penalty_for_line(int iSystem, int i, int j);
    bool is_better_option(float prevPenalty, float newPenalty, float nextPenalty,
                          int i, int j);

    //information about a column
    bool is_empty_column(int iCol);
    LUnits get_column_width(int iCol);
    virtual int get_column_barlines_information(int iCol);

    //methods to compute results
    TimeGridTable* create_time_grid_table_for_column(int iCol);

    //debug
    void dump_column_data(int iCol, ostream& outStream);

    //column creation: collecting content
    void start_column_measurements(int iCol);
    void include_object(ColStaffObjsEntry* pCurEntry, int iCol, int iInstr, int iStaff,
                        ImoStaffObj* pSO, GmoShape* pShape, bool fInProlog=false);
    void include_full_measure_rest(GmoShape* pRestShape, ColStaffObjsEntry* pCurEntry,
                                   GmoShape* pNonTimedShape);
    void finish_column_measurements(int iCol);

    //auxiliary: shapes and boxes
    void add_shapes_to_box(int iCol, GmoBoxSliceInstr* pSliceInstrBox, int iInstr);
    void delete_shapes(int iCol);
    void reposition_slices_and_staffobjs(int iFirstCol, int iLastCol,
                                         LUnits yShift, LUnits* yMin, LUnits* yMax);
    void reposition_full_measure_rests(int iFirstCol, int iLastCol, GmoBoxSystem* pBox);

protected:
    void new_column(TimeSlice* pSlice);
    void new_slice(ColStaffObjsEntry* pEntry, int entryType, int iColumn, int iData);
    void finish_slice(ColStaffObjsEntry* pLastEntry, int numEntries);
    void compute_springs();
    void order_slices_in_columns();
    void apply_force(float F);
    void determine_spacing_parameters();
    bool accept_for_prolog_slice(ColStaffObjsEntry* pEntry);

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
    list<FullMeasureRestData*> m_rests;  //data for full-measure rests in this column

    float   m_slope;            //slope of approximated sff() for this column
    float   m_minFi;            //minimum force at which this column reacts
    LUnits  m_xFixed;           //fixed spacing for this column
    LUnits  m_colWidth;         //current col. width after having applying force
    LUnits  m_colMinWidth;      //minimum width (force 0)
    int     m_barlinesInfo;     //information about barlines in last slice of this column


    //for creating TimeGridTable
    LUnits  m_xPos;             //position for this column


    ColumnDataGourlay(TimeSlice* pSlice);
    ~ColumnDataGourlay();

    //creation
    void set_num_entries(int numSlices);
    void order_slices();
    void determine_minimum_width();
    void include_full_measure_rest(GmoShape* pShape, ColStaffObjsEntry* pEntry,
                                   GmoShape* pNonTimedShape);

    //spacing
    LUnits determine_extent_for(float force);
    float determine_force_for(LUnits width);
    void determine_approx_sff_for(float force);
    void apply_force(float F);

    //for TimeGridTable
    TimeGridTable* create_time_grid_table();
    LUnits get_position() { return m_xPos; }

    //access to position and spacing data
    inline LUnits get_column_width() { return m_colWidth; }
    inline LUnits get_minimum_width() { return m_colMinWidth; }

    //other information
    inline int num_slices() { return int(m_orderedSlices.size()); }
    bool is_empty_column();
    inline int get_barlines_information() { return m_barlinesInfo; }
    inline bool all_instr_have_barline() {
        return m_barlinesInfo & k_all_instr_have_barline;
    }
    inline bool some_instr_have_barline() {
        return (m_barlinesInfo & k_some_instr_have_barline) != 0;
    }
    inline bool all_instr_have_final_barline() {
        return (m_barlinesInfo & k_all_instr_have_final_barline) != 0;
    }
    void collect_barlines_information(int numInstruments);

    //managing shapes
    void add_shapes_to_box(GmoBoxSliceInstr* pSliceInstrBox, int iInstr,
                           vector<StaffObjData*>& data);
    void delete_shapes(vector<StaffObjData*>& data);
    void move_shapes_to_final_positions(vector<StaffObjData*>& data, LUnits xPos,
                                        LUnits yPos, LUnits* yMin, LUnits* yMax,
                                        ScoreMeter* pMeter,
                                        VerticalProfile* pVProfile);
    void reposition_full_measure_rests(GmoBoxSystem* pBox, GmMeasuresTable* pMeasures);

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
    int         m_idxStaff;     //staff index 0..n-1, relative to system
    int         m_iStaff;       //staff index 0..n-1, relative to instrument
    LUnits      m_xUserShift;
    LUnits      m_yUserShift;

public:
    StaffObjData();
    virtual ~StaffObjData();

    inline GmoShape* get_shape() { return m_pShape; }
};



//---------------------------------------------------------------------------------------
// FullMeasureRestData
// Data associated to a full-measure rests
class FullMeasureRestData
{
public:
    GmoShape* m_pRestShape;         //shape for this rest
    ColStaffObjsEntry* m_pEntry;    //the rest entry in ColStaffObjs
    GmoShape* m_pNonTimedShape;     //the last non-timed before the rest, if any

public:
    FullMeasureRestData(GmoShape* pRestShape, ColStaffObjsEntry* pEntry,
                        GmoShape* pNonTimedShape)
        : m_pRestShape(pRestShape)
        , m_pEntry(pEntry)
        , m_pNonTimedShape(pNonTimedShape)
    {
    }

    ~FullMeasureRestData() {}

    inline GmoShape* get_rest_shape() { return m_pRestShape; }
    inline ColStaffObjsEntry* get_rest_entry() { return m_pEntry; }
    inline GmoShape* get_non_timed_shape() { return m_pNonTimedShape; }
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
    friend class TimeSliceBarline;
    friend class TimeSliceNoterest;
    ColStaffObjsEntry*  m_firstEntry;
    ColStaffObjsEntry*  m_lastEntry;
    int         m_iFirstData;   //index to first StaffObjData element for this slice
    int         m_numEntries;   //num staffobjs in this slice
    int         m_type;         //type of slice. Value from enum ESliceType
    int         m_iColumn;      //Column in which this slice is included

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

    TimeSlice(ColStaffObjsEntry* pEntry, int entryType, int column, int iData);

public:
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
                        TimeUnits maxNextTime, TimeUnits minNote, ScoreMeter* pMeter);

    //list creation
    inline TimeSlice* next() { return m_next; }
    inline void set_next(TimeSlice* pSlice) { m_next = pSlice; }

    //spacing
    void compute_spring_data(LUnits uSmin, float alpha, float log2dmin, TimeUnits dmin,
                             bool fProportional, LUnits dsFixed);
    virtual void assign_spacing_values(vector<StaffObjData*>& data, ScoreMeter* pMeter,
                                       TextMeter& textMeter) = 0;
    void apply_force(float F);
    inline void increment_fixed_extent(LUnits value) { m_xLeft += value; }
    virtual void increment_xRi(LUnits value) { m_xRi += value; }
    virtual void merge_with_xRi(LUnits value) { m_xRi = max(m_xRi, value); }
    virtual void set_minimum_xi(LUnits value) {
        if (get_xi() < value)
            m_xRi = value - m_xLi;
    }

    //basic information
    inline int get_type() { return m_type; }

    //access to information
    inline LUnits get_xi() { return m_xLi + m_xRi; }
    inline LUnits get_minimum_extent() { return m_xLi + m_xRi + m_xLeft; }
    inline LUnits get_left_rod() { return m_xLi; }
    inline LUnits get_right_rod() { return m_xRi; }
    inline LUnits get_fixed_extent() { return m_xLeft; }
    inline float get_pre_stretching_force() { return m_fi; }
    inline TimeUnits get_spring_duration() { return m_ds; }
    inline TimeUnits get_shortest_duration() { return m_di; }
    TimeUnits get_timepos();
    inline int get_num_entries() { return m_numEntries; }
    inline ColStaffObjsEntry* get_first_entry() { return m_firstEntry; }
    inline ColStaffObjsEntry* get_last_entry() { return m_lastEntry; }

    //operations
    void add_shapes_to_box(GmoBoxSliceInstr* pSliceInstrBox, int iInstr,
                           vector<StaffObjData*>& data);
    void delete_shapes(vector<StaffObjData*>& data);
    virtual void move_shapes_to_final_positions(vector<StaffObjData*>& data, LUnits xPos,
                                                LUnits yPos, LUnits* yMin, LUnits* yMax,
                                                ScoreMeter* pMeter,
                                                VerticalProfile* pVProfile);

    //access to information
    inline LUnits get_width() { return m_width; }

    //other information (barline slice)
    int collect_barlines_information(int numInstruments);

    //debug
    void dump(ostream& ss);
    static void dump_header(ostream& ss);
    inline int dbg_get_first_data() { return m_iFirstData; }


protected:
    void compute_smallest_duration_di(TimeUnits minNotePrev);
    void find_smallest_note_soundig_at(TimeUnits nextTime);
    void compute_spring_constant(LUnits uSmin, float alpha, float log2dmin,
                                 TimeUnits dmin, bool fProportional, LUnits dsFixed);
    void compute_pre_stretching_force();
    LUnits spacing_function(LUnits uSmin, float alpha, float log2dmin, TimeUnits dmin);
    inline TimeUnits get_min_note_still_sounding() { return m_minNoteNext; }

    LUnits measure_text(const string& text, ImoStyle* pStyle,
                        const string& language, TextMeter& meter);

};


//---------------------------------------------------------------------------------------
// TimeSliceProlog
// An slice for the prolog objects.
// At maximum one clef, one key and one time signature per staff
class TimeSliceProlog : public TimeSlice
{
protected:
    LUnits m_clefsWidth;
    LUnits m_timesWidth;
    LUnits m_keysWidth;
    LUnits m_spaceBefore;

public:
    TimeSliceProlog(ColStaffObjsEntry* pEntry, int column, int iData);
    virtual ~TimeSliceProlog();

    //overrides
    void assign_spacing_values(vector<StaffObjData*>& data, ScoreMeter* pMeter,
                               TextMeter& textMeter) override;
    void move_shapes_to_final_positions(vector<StaffObjData*>& data, LUnits xPos,
                                        LUnits yPos, LUnits* yMin, LUnits* yMax,
                                        ScoreMeter* pMeter,
                                        VerticalProfile* pVProfile) override;

    //specific methods
    void remove_after_space_if_not_full(ScoreMeter* pMeter, int SOtype);
    void remove_after_space(ScoreMeter* pMeter);
};


//---------------------------------------------------------------------------------------
// TimeSliceNonTimed
// An slice for non-timed objects not at prolog
class TimeSliceNonTimed : public TimeSlice
{
protected:
    int m_numStaves;
    LUnits m_interSpace;
    vector<LUnits> m_widths;        //total width for objects, by staff
    vector<bool> m_fHasObjects;     //there are objects, by staff
    bool m_fHasWidth;               //true if at least one shape has width
    bool m_fSomeVisible;            //true if at least one shape is visible

public:
    TimeSliceNonTimed(ColStaffObjsEntry* pEntry, int column, int iData);
    virtual ~TimeSliceNonTimed();

    //overrides
    void assign_spacing_values(vector<StaffObjData*>& data, ScoreMeter* pMeter,
                               TextMeter& textMeter) override;
    void move_shapes_to_final_positions(vector<StaffObjData*>& data, LUnits xPos,
                                        LUnits yPos, LUnits* yMin, LUnits* yMax,
                                        ScoreMeter* pMeter,
                                        VerticalProfile* pVProfile) override;

    inline bool has_width() { return m_fHasWidth; }
    inline bool some_objects_visible() { return m_fSomeVisible; }
    inline LUnits get_width_for_staff(int iStaff) { return m_widths[iStaff]; }
    inline bool is_empty_staff(int iStaff) { return !m_fHasObjects[iStaff]; }
};


//---------------------------------------------------------------------------------------
// TimeSliceBarline
// An slice for barlines
class TimeSliceBarline : public TimeSlice
{
protected:

public:
    TimeSliceBarline(ColStaffObjsEntry* pEntry, int column, int iData);
    virtual ~TimeSliceBarline();

    //overrides
    void assign_spacing_values(vector<StaffObjData*>& data, ScoreMeter* pMeter,
                               TextMeter& textMeter) override;

};


//---------------------------------------------------------------------------------------
// TimeSliceNoterest
// An slice for noterests
class TimeSliceNoterest : public TimeSlice
{
protected:
    //lyrics (ptr to lyrics, index to staff)
    vector< pair<ImoLyric*, int> > m_lyrics;
    LUnits m_xRiLyrics;     //part of xRi due to lyrics
    LUnits m_xRiMerged;     //part of xRi due to merged from non-timed
    vector<LUnits> m_xLy;   //rods for lyrics

public:
    TimeSliceNoterest(ColStaffObjsEntry* pEntry, int column, int iData);
    virtual ~TimeSliceNoterest();

    //overrides
    void assign_spacing_values(vector<StaffObjData*>& data, ScoreMeter* pMeter,
                               TextMeter& textMeter) override;
    void increment_xRi(LUnits value) override;
    void merge_with_xRi(LUnits value) override;
    void set_minimum_xi(LUnits value) override;

    //specific to deal with lyrics
    void add_lyrics(ScoreMeter* pMeter);
    LUnits measure_lyric(ImoLyric* pLyric, ScoreMeter* pMeter, TextMeter& textMeter);
    inline LUnits get_lyrics_rod() { return m_xRiLyrics; }

};


}   //namespace lomse

#endif    // __LOMSE_SPACING_ALGORITHM_GOURLAY_H__


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
class ShapeData;
class FullMeasureRestData;
class TextMeter;
class ImoStyle;
class GmMeasuresTable;
class GraphicModel;
class TimeSliceNoterest;
class ImoNoteRest;
class GmoShapeNote;


//---------------------------------------------------------------------------------------
// SpAlgGourlay
// Spacing algorithm based on Gourlay's method
//
class SpAlgGourlay : public SpAlgColumn
{
protected:
    std::list<TimeSlice*> m_slices;              //list of TimeSlices
    std::vector<ColumnDataGourlay*> m_columns;   //columns
    std::vector<ShapeData*> m_shapes;            //data associated to each staff object

    //auxiliary temporal variables used while collecting columns' data
    TimeSlice*          m_pCurSlice;
    ColStaffObjsEntry*  m_pLastEntry;
    int                 m_prevType;
    TimeUnits           m_prevTime;
    TimeUnits           m_prevAlignTime;    //for grace notes
    int                 m_numEntries;
    ColumnDataGourlay*  m_pCurColumn;
    int                 m_numSlices;
    int                 m_iPrevColumn;

    //auxiliary temporal data to determine note sequences and minimum rods
    std::vector<int> m_lastSequence;                //last sequence value for each line
    std::vector<TimeSliceNoterest*> m_lastSlice;    //last slice for each line

    //auxiliary, for controlling objects to include in a prolog slice
    TimeUnits    m_lastPrologTime;
    std::vector<bool> m_prologClefs;
    std::vector<bool> m_prologKeys;
    std::vector<bool> m_prologTimes;

    //data collected for each slice
    TimeUnits   m_maxNoteDur;
    TimeUnits   m_minNoteDur;

    //spacing parameters
	LUnits m_uSmin;     //minimun space between notes
    float  m_alpha;     //alpha parameter for Gourlay's formula
    float  m_dmin;      //min note duration for which fixed spacing will be used
    float  m_Fopt;      //Optimum force (user defined and dependent on personal taste)

public:
    SpAlgGourlay(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 ScoreLayouter* pScoreLyt, ImoScore* pScore,
                 EngraversMap& engravers, ShapesCreator* pShapesCreator,
                 PartsEngraver* pPartsEngraver);
    virtual ~SpAlgGourlay();


    //spacing algorithm main actions
    void do_spacing(int iColumnToTrace) override;
    void justify_system(int iFirstCol, int iLastCol, LUnits uSpaceIncrement) override;

    //for lines break algorithm
    float determine_penalty_for_line(int iSystem, int i, int j) override;
    bool is_better_option(float prevPenalty, float newPenalty, float nextPenalty,
                          int i, int j) override;

    //information about a column
    bool is_empty_column(int iCol) override;
    LUnits get_column_width(int iCol) override;
    int get_column_barlines_information(int iCol) override;

    //methods to compute results
    TimeGridTable* create_time_grid_table_for_column(int iCol) override;

    //debug
    void dump_column_data(int iCol, std::ostream& outStream) override;
    std::string dump_spacing_parameters();

    //column creation: collecting content
    void start_column_measurements(int iCol) override;
    void include_object(ColStaffObjsEntry* pCurEntry, int iCol, int iInstr, int iStaff,
                        ImoStaffObj* pSO, GmoShape* pShape, bool fInProlog=false) override;
    void include_full_measure_rest(GmoShape* pRestShape, ColStaffObjsEntry* pCurEntry,
                                   GmoShape* pNonTimedShape) override;
    void finish_column_measurements(int iCol) override;

    //auxiliary: shapes and boxes
    void add_shapes_to_box(int iCol, GmoBoxSliceInstr* pSliceInstrBox, int iInstr) override;
    void delete_shapes(int iCol) override;
    void reposition_slices_and_staffobjs(int iFirstCol, int iLastCol,
                                         LUnits yShift, LUnits* yMin, LUnits* yMax) override;
    void reposition_full_measure_rests(int iFirstCol, int iLastCol, GmoBoxSystem* pBox) override;

protected:
    void new_column(TimeSlice* pSlice);
    void new_slice(ColStaffObjsEntry* pEntry, int entryType, int iColumn, int iShape);
    void finish_slice(ColStaffObjsEntry* pLastEntry, int numEntries);
    void finish_sequences();
    void compute_rods_ds_and_di();
    void fix_neighborhood_spacing_problems(int iColumnToTrace);
    void compute_springs();
    void determine_spacing_parameters();
    bool accept_for_prolog_slice(ColStaffObjsEntry* pEntry);
    int determine_required_slice_type(ImoStaffObj* pSO, bool fInProlog);
    ShapeData* save_info_for_shape(GmoShape* pShape, int iInstr, int iStaff);
    bool determine_if_new_slice_needed(ColStaffObjsEntry* pCurEntry, TimeUnits curTime,
                                       int curType, ImoStaffObj* pSO);

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
    std::vector<TimeSlice*> m_orderedSlices;  //slices ordered by pre-stretching force fi
    std::list<FullMeasureRestData*> m_rests;  //data for full-measure rests in this column

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
    void fix_neighborhood_spacing_problems(bool fTrace);

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
    inline bool all_instr_have_barline_TS_or_KS() {
        return m_barlinesInfo & k_all_instr_have_barline_TS_or_KS;
    }
    inline bool some_instr_have_barline_TS_or_KS() {
        return (m_barlinesInfo & k_some_instr_have_barline_TS_or_KS) != 0;
    }
    void collect_barlines_information(int numInstruments);

    //managing shapes
    void add_shapes_to_box(GmoBoxSliceInstr* pSliceInstrBox, int iInstr,
                           std::vector<ShapeData*>& shapes);
    void delete_shapes(std::vector<ShapeData*>& shapes);
    void move_shapes_to_final_positions(std::vector<ShapeData*>& shapes, LUnits xPos,
                                        LUnits yPos, LUnits* yMin, LUnits* yMax,
                                        ScoreMeter* pMeter,
                                        VerticalProfile* pVProfile);
    void reposition_full_measure_rests(GmoBoxSystem* pBox, GmMeasuresTable* pMeasures);

    //debug
    void dump(std::ostream& outStream, bool fOrdered=false);

protected:

};



//---------------------------------------------------------------------------------------
// ShapeData
// Shape and related data associated to an staff object.
class ShapeData
{
public:
    GmoShape*   m_pShape;       //shape for this staff object
    int         m_idxStaff;     //staff index 0..n-1, relative to system
    int         m_iStaff;       //staff index 0..n-1, relative to instrument

public:
    ShapeData() : m_pShape(nullptr) {}

    inline GmoShape* get_shape() { return m_pShape; }
};



//---------------------------------------------------------------------------------------
// SeqData
// information for each noterest included in an slice, to compute sequences
class SeqData
{
public:
    ColStaffObjsEntry* m_pEntry;    //the note/rest entry in ColStaffObjs
    int m_sequence;         //type of sequence. Value from enum ESeqType

public:
    SeqData(ColStaffObjsEntry* pEntry, int sequence);

    enum ESeqType {
        k_seq_isolated = 0,     //0-isolated note
        k_seq_start,            //1-start of sequence
        k_seq_end,              //2-end of sequence
        k_seq_end_start,        //3-end of a sequence and and start of a new one
        k_seq_continue,         //4-in sequence
    };

    inline bool is_in_sequence() { return m_sequence == k_seq_continue; }
    void update_info(ColStaffObjsEntry* pEntry, GmoShape* pShape);
    inline void update_sequence(int seq) { m_sequence = seq; }
    inline void finish_sequence_if_in_sequence() {
        if (is_in_sequence())
            m_sequence = k_seq_end;
    }
    TimeUnits get_duration();
    int get_sequence() { return m_sequence; }

    //debug
    void dump(int iLine, std::ostream& ss);

};

//---------------------------------------------------------------------------------------
// RodsData
// Rods information for each noterest included in an slice
class RodsData
{
public:
    LUnits  m_dxB;          //width of left rod (before)
    LUnits  m_dxNH;         //width of notehead
    LUnits  m_dxA;          //width of right rod (after)

public:
    RodsData(LUnits dxB, LUnits dxNH, LUnits dxA)
        : m_dxB(dxB)
        , m_dxNH(dxNH)
        , m_dxA(dxA)
    {
    }

    void update_rods(LUnits dxB, LUnits dxNH, LUnits dxA);

    //debug
    void dump(int iStaff, std::ostream& ss);

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
    friend class TimeSliceGraces;
    ColStaffObjsEntry*  m_firstEntry;
    ColStaffObjsEntry*  m_lastEntry;
    int         m_iFirstShape;  //index to first ShapeData element for this slice
    int         m_numEntries;   //num staffobjs in this slice
    int         m_type;         //type of slice. Value from enum ESliceType
    int         m_iColumn;      //Column in which this slice is included

    //list
    TimeSlice* m_next;
    TimeSlice* m_prev;

    //data for spacing
    LUnits  m_dxLeft;       //fixed space at start (a rod in the springs join)
    LUnits  m_dxL;          //width of left rod
    LUnits  m_dxR;          //width of right rod
    float   m_fi;           //pre-stretching force
    float   m_c;            //spring constant c
    LUnits  m_width;        //final extent after applying force

    //auxiliary
    TimeUnits   m_ds;       //spring duration (= timepos(next_slice) - timepos(this_slice))
    TimeUnits   m_di;       //shortest duration in this segment or still sounding in this segment
    TimeUnits   m_minNote;      //min note/rest duration in this segment
    TimeUnits   m_minNoteNext;  //min note/rest duration still sounding in next segment

    TimeSlice(ColStaffObjsEntry* pEntry, int entryType, int column, int iShape);

public:
    virtual ~TimeSlice();

    enum ESliceType {
        k_undefined = -1,
        k_prolog = 0,
        k_non_timed,
        k_noterest,
        k_barline,
        k_graces,
    };

    //creation
    void set_final_data(ColStaffObjsEntry* pLastEntry, int numEntries,
                        TimeUnits maxNextTime, TimeUnits minNote, ScoreMeter* pMeter);

    //list creation
    inline TimeSlice* next() { return m_next; }
    inline void set_next(TimeSlice* pSlice) { m_next = pSlice; }

    //spacing
    void compute_ds_and_di();
    void compute_spring_data(LUnits uSmin, float alpha, TimeUnits dmin,
                             bool fProportional, LUnits dsFixed);
    virtual void assign_spacing_values(std::vector<ShapeData*>& shapes, ScoreMeter* pMeter,
                                       TextMeter& textMeter) = 0;
    void apply_force(float F);
    inline void increment_fixed_extent(LUnits value) { m_dxLeft += value; }
    virtual void increment_dxR(LUnits value) { m_dxR += value; }
    virtual void merge_with_dxR(LUnits value) { m_dxR = max(m_dxR, value); }
    virtual void set_minimum_space(LUnits value) {
        if (get_total_rods() < value)
            m_dxR = value - m_dxL;
    }

    //basic information
    inline int get_type() { return m_type; }

    //access to information
    inline LUnits get_left_space() { return m_dxLeft; }
    inline LUnits get_left_rod() { return m_dxL; }
    inline LUnits get_right_rod() { return m_dxR; }
    virtual LUnits get_total_rods() { return m_dxL + m_dxR; }
    LUnits get_minimum_extent() { return get_total_rods() + m_dxLeft; }
    inline float get_pre_stretching_force() { return m_fi; }
    inline TimeUnits get_spring_duration() { return m_ds; }
    inline TimeUnits get_shortest_duration() { return m_di; }
    TimeUnits get_timepos();
    inline int get_num_entries() { return m_numEntries; }
    inline ColStaffObjsEntry* get_first_entry() { return m_firstEntry; }
    inline ColStaffObjsEntry* get_last_entry() { return m_lastEntry; }
    inline LUnits get_width() { return m_width; }

    //operations
    void add_shapes_to_box(GmoBoxSliceInstr* pSliceInstrBox, int iInstr,
                           std::vector<ShapeData*>& shapes);
    void delete_shapes(std::vector<ShapeData*>& shapes);
    virtual void move_shapes_to_final_positions(std::vector<ShapeData*>& shapes, LUnits xPos,
                                                LUnits yPos, LUnits* yMin, LUnits* yMax,
                                                ScoreMeter* pMeter,
                                                VerticalProfile* pVProfile);

    //settings
    inline void set_shortest_duration(TimeUnits di) { m_di = di; }

    //other information (barline slice)
    int collect_barlines_information(int numInstruments);

    //debug
    void dump(std::ostream& ss);
    virtual void dump_lines(std::ostream& UNUSED(ss)) {}
    virtual void dump_rods(std::ostream& UNUSED(ss)) {}
    virtual void dump_neighborhood(std::ostream& ss) { ss << "    "; }
    static void dump_header(std::ostream& ss);
    inline int dbg_get_first_data() { return m_iFirstShape; }
    static std::string slice_type_to_string(int sliceType);


protected:
    void compute_smallest_duration_di(TimeUnits minNotePrev);
    void find_smallest_note_soundig_at(TimeUnits nextTime);
    void compute_spring_constant(LUnits uSmin, float alpha, TimeUnits dmin,
                                 bool fProportional, LUnits dsFixed);
    void compute_pre_stretching_force();
    static LUnits spacing_function(TimeUnits d, LUnits uSmin, float alpha, TimeUnits dmin);
    inline TimeUnits get_min_note_still_sounding() { return m_minNoteNext; }

    LUnits measure_text(const std::string& text, ImoStyle* pStyle,
                        const std::string& language, TextMeter& meter);

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
    TimeSliceProlog(ColStaffObjsEntry* pEntry, int column, int iShape);
    virtual ~TimeSliceProlog();

    //overrides
    void assign_spacing_values(std::vector<ShapeData*>& shapes, ScoreMeter* pMeter,
                               TextMeter& textMeter) override;
    void move_shapes_to_final_positions(std::vector<ShapeData*>& shapes, LUnits xPos,
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
    std::vector<LUnits> m_widths;        //total width for objects, by staff
    std::vector<bool> m_fHasObjects;     //there are objects, by staff
    bool m_fHasWidth;               //true if at least one shape has width
    bool m_fSomeVisible;            //true if at least one shape is visible

public:
    TimeSliceNonTimed(ColStaffObjsEntry* pEntry, int column, int iShape);
    virtual ~TimeSliceNonTimed();

    //overrides
    void assign_spacing_values(std::vector<ShapeData*>& shapes, ScoreMeter* pMeter,
                               TextMeter& textMeter) override;
    void move_shapes_to_final_positions(std::vector<ShapeData*>& shapes, LUnits xPos,
                                        LUnits yPos, LUnits* yMin, LUnits* yMax,
                                        ScoreMeter* pMeter,
                                        VerticalProfile* pVProfile) override;
    void join_with_previous(ScoreMeter* pMeter);

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
    TimeSliceBarline(ColStaffObjsEntry* pEntry, int column, int iShape);
    virtual ~TimeSliceBarline();

    //overrides
    void assign_spacing_values(std::vector<ShapeData*>& shapes, ScoreMeter* pMeter,
                               TextMeter& textMeter) override;
    void move_shapes_to_final_positions(std::vector<ShapeData*>& shapes, LUnits xPos,
                                        LUnits yPos, LUnits* yMin, LUnits* yMax,
                                        ScoreMeter* pMeter,
                                        VerticalProfile* pVProfile) override;
    void join_with_previous();

};


//---------------------------------------------------------------------------------------
// TimeSliceGraces
// An slice for grace notes
class TimeSliceGraces : public TimeSlice
{
protected:

public:
    TimeSliceGraces(ColStaffObjsEntry* pEntry, int column, int iShape);
    virtual ~TimeSliceGraces();

    //overrides
    void assign_spacing_values(std::vector<ShapeData*>& shapes, ScoreMeter* pMeter,
                               TextMeter& textMeter) override;
    void join_with_previous(ScoreMeter* pMeter, std::vector<LUnits>& xAcc, LUnits xPrev);

};


//---------------------------------------------------------------------------------------
// TimeSliceNoterest
// An slice for noterests
class TimeSliceNoterest : public TimeSlice
{
protected:
    //lyrics (ptr to lyrics, index to staff)
    std::vector< std::pair<ImoLyric*, int> > m_lyrics;
    LUnits m_dxRLyrics;     //part of dxR due to lyrics
    LUnits m_dxRMerged;     //part of dxR due to merged from non-timed
//    std::vector<LUnits> m_xLy;          //rods for lyrics
    std::vector<SeqData*> m_lines;     //info for each noterest, for sequences
    std::vector<RodsData*> m_rods;     //rods info for each noterest, by staff
    int m_neighborhood;     //type of neighborhood: none, start, continue, end, end-start

public:
    TimeSliceNoterest(ColStaffObjsEntry* pEntry, int column, int iShape, int numLines,
                      int numStaves);
    virtual ~TimeSliceNoterest();

    //overrides
    void assign_spacing_values(std::vector<ShapeData*>& shapes, ScoreMeter* pMeter,
                               TextMeter& textMeter) override;
    void join_with_previous(ScoreMeter* pMeter, LUnits xLyrics, LUnits xPrev);
    void increment_dxR(LUnits value) override;
    void merge_with_dxR(LUnits value) override;
    void set_minimum_space(LUnits value) override;
    LUnits get_total_rods() override { return m_dxL + m_dxR + max(m_dxRLyrics - m_dxL, m_dxRMerged); };
    void dump_lines(std::ostream& ss) override;
    void dump_rods(std::ostream& ss) override;
    void dump_neighborhood(std::ostream& ss) override;

    //specific, for computing note sequences and measuring rods by line
    void save_seq_data(ColStaffObjsEntry* pEntry, int sequence);
    void update_sequence(int iLine, int sequence);
    void finish_sequences(int iLine);
    TimeUnits get_duration(int iLine);
    int get_sequence(int iLine);
    inline int get_neighborhood() { return m_neighborhood; }
    int compute_neighborhood(int prevNeighborhood, int numOpenSeqs);

    //specific to deal with lyrics
    void add_lyrics(ScoreMeter* pMeter);
    LUnits measure_lyric(ImoLyric* pLyric, ScoreMeter* pMeter, TextMeter& textMeter);
    inline LUnits get_lyrics_rod() { return m_dxRLyrics; }

protected:
    SeqData* find_prev_rodsdata_for_line(int iLine);
    SeqData* get_rodsdata(int iLine) { return m_lines[iLine]; }
    void fix_spacing_issues(std::vector<ShapeData*>& shapes, ScoreMeter* pMeter);
    LUnits compute_rods(std::vector<ShapeData*>& shapes, ScoreMeter* pMeter);
    void merge_rods_with(std::vector<RodsData*>& nextRods, LUnits minSpace);

};


}   //namespace lomse

#endif    // __LOMSE_SPACING_ALGORITHM_GOURLAY_H__


//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_SYSTEM_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_SYSTEM_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_time.h"
#include "lomse_score_enums.h"
#include "lomse_logger.h"
#include "lomse_injectors.h"
#include <list>
#include <vector>
using namespace std;

namespace lomse
{

//forward declarations
class ImoInstrument;
class ImoScore;
class ImoStaffObj;
class ImoAuxObj;
class ImoStaff;
class GmoShape;
class GmoBoxSystem;
class GmoBoxSliceInstr;
class GmoBoxSlice;
class ScoreMeter;
class ShapesStorage;
class ColStaffObjsEntry;
class InstrumentEngraver;
class ShapesCreator;

class ScoreLayouter;
class SystemLayouter;
class ColumnLayouter;
class ColumnStorage;
class MusicLine;
class LineEntry;
class ColumnResizer;
class LineResizer;
class LineSpacer;
class TimeGridLineExplorer;

//some helper definitions
#define LineEntryIterator		  std::vector<LineEntry*>::iterator
#define LinesIterator             std::vector<MusicLine*>::iterator
#define LineSpacersIterator       std::vector<LineSpacer*>::iterator


//---------------------------------------------------------------------------------------
//LineEntry: an entry in MusicLine
//---------------------------------------------------------------------------------------
class LineEntry
{
public:
    // constructor and destructor
    LineEntry(ImoStaffObj* pSO, GmoShape* pShape, bool fProlog, TimeUnits rTime,
              LUnits xUserShift, LUnits yUserShift);
    ~LineEntry() {}

    //constructor for unit tests
    LineEntry(bool fIsBarlineEntry, bool fProlog, TimeUnits rTime, LUnits xAnchor,
              LUnits xLeft, LUnits uSize, LUnits uFixedSpace, LUnits uVarSpace,
              LUnits xFinal);


    void reposition_at(LUnits uxNewXLeft);
	void assign_fixed_and_variable_space(ColumnLayouter* pTT, TimeUnits rFactor);
    void move_shape(UPoint sliceOrg);
    void add_shape_info();

    //access to entry data
    inline bool is_barline_entry() { return m_fIsBarlineEntry; }
    inline ImoStaffObj* get_staffobj() { return m_pSO; }
    inline GmoShape*  get_shape() { return m_pShape; }
	inline bool is_prolog_object() { return m_fProlog; }
    inline TimeUnits get_timepos() { return m_rTimePos; }
    inline LUnits get_position() { return m_xLeft; }
    inline LUnits get_anchor() { return m_uxAnchor; }
    inline LUnits get_x_final() { return m_xFinal; }
    inline LUnits get_shape_size() { return m_uSize; }
    LUnits get_fixed_space() { return m_uFixedSpace; }
    inline LUnits get_variable_space() { return m_uVariableSpace; }

    //setters and getters
    inline LUnits get_total_size() { return m_uSize + m_uFixedSpace + m_uVariableSpace; }
    inline void set_variable_space(LUnits space) { m_uVariableSpace = space; }
    inline void set_fixed_space(LUnits space) { m_uFixedSpace = space; }
    inline void set_size(LUnits width) { m_uSize = width; }
    inline TimeUnits get_duration() { return 0.0; } //TODO m_pSO->GetTimePosIncrement(); }
    inline void set_position(LUnits uPos) { m_xLeft = uPos; }
    inline void mark_as_barline_entry() { m_fIsBarlineEntry = true; }
    LUnits get_shift_to_noterest_center();
    bool is_note_rest();
    bool has_barline();
    bool has_visible_barline();
    void update_x_final() { m_xFinal = m_xLeft + get_total_size(); };

    //debug
    void dump(int iEntry, ostream& outStream);
    static void dump_header(ostream& outStream);


protected:
    //member variables (one entry of the table)
    //----------------------------------------------------------------------------
    bool            m_fIsBarlineEntry;  //is last entry. Contains barline or nothing
    ImoStaffObj*    m_pSO;              //ptr to the StaffObj
    GmoShape*       m_pShape;           //ptr to the shape
	bool			m_fProlog;          //this shape is a prolog object (clef, KS, TS at start of system)
    TimeUnits       m_rTimePos;         //timepos for this pSO or -1 if not anchored in time
    LUnits          m_xLeft;            //current position of the left border of the object
    LUnits          m_uxAnchor;         //offset to anchor line
    LUnits          m_xFinal;           //next position (right border position + trailing space)
    //to redistribute objects we need to know:
    LUnits          m_uSize;            //size of the shape (notehead, etc.)
    LUnits          m_uFixedSpace;      //fixed space added after shape
    LUnits          m_uVariableSpace;   //any variable added space we can adjust
    LUnits          m_xUserShift;       //location shift specified by user
    LUnits          m_yUserShift;       //location shift specified by user

    //debug
    bool m_fShapeInfoLoaded;

};


//---------------------------------------------------------------------------------------
// MusicLine: an object to encapsulate positioning data for a line and its objects
//---------------------------------------------------------------------------------------
class MusicLine
{
protected:
	std::vector<LineEntry*>	m_LineEntries;	    //the staffobjs that form this table

    //identification data
	int     m_line;		        //line number (0..n-1)
	int     m_nInstr;		    //instrument (1..n)
	int     m_nVoice;		    //voice (1..n) [ = m_line + 1 ]

    //global spacing data
    LUnits m_uxLineStart;               //xStart -initial position
    LUnits m_uStartFixedSpace;          //SFS - fixed space at start
    LUnits m_uxFirstSymbol;             //xs - start of first symbol
    LUnits m_uxFirstAnchor;             //xa - first anchor position
    LUnits m_uxRightEdge;               //xr - right most position
    LUnits m_uxStartOfEndVarSpace;      //xv - start of final var space
    TimeUnits m_rFirstTime;             //time for first timed entry or -1

    //to control if ends in barline
    enum {
        k_barline_unknown = 0,
        k_no_barline,
        k_visible_barline,
        k_no_visible_barline,
    };
    int m_barlineType;

public:
    MusicLine(int line, int nInstr, LUnits uxStart, LUnits fixedSpace);
    virtual ~MusicLine();

    //building the line
    inline void clear() { m_LineEntries.clear(); }
    inline void push_back(LineEntry* pEntry) { m_LineEntries.push_back(pEntry); }
	LineEntry* add_entry(ImoStaffObj* pSO, GmoShape* pShape, TimeUnits rTime,
                         bool fInProlog, LUnits xUserShift, LUnits yUserShift);
    void do_measurements();

    //access to an item
    inline LineEntry* front() { return m_LineEntries.front(); }
    inline LineEntry* back() { return m_LineEntries.back(); }
    inline LineEntry* item(int i) { return m_LineEntries[i]; }
    inline LineEntry* get_last_entry() { return m_LineEntries.back(); }

    //iterator to an item
    inline LineEntryIterator begin() { return m_LineEntries.begin(); }
    inline LineEntryIterator end() { return m_LineEntries.end(); }

    //measurements to build and justify systems
    inline LUnits get_line_start_position() { return m_uxLineStart; }           //xStart
    inline LUnits get_fixed_space_at_start() { return m_uStartFixedSpace; }     //SFS
    inline LUnits get_right_most_edge() { return m_uxRightEdge; }               //xr
    inline LUnits get_start_of_final_var_space() { return m_uxStartOfEndVarSpace; }  //xv
    inline LUnits get_first_anchor_position() { return m_uxFirstAnchor; }       //xa
    inline LUnits get_start_of_first_symbol() { return m_uxFirstSymbol; }       //xs
    inline TimeUnits get_first_time() { return m_rFirstTime; }

    LUnits get_final_pos();             //xf
    LUnits get_end_hook_width();
    LUnits get_fixed_space_at_end();
    LUnits get_line_width();

    //
    inline int get_num_objects_in_line() { return (int)m_LineEntries.size(); }
    inline size_t size() { return m_LineEntries.size(); }
    inline bool is_for_line(int line) { return m_line == line; }
    inline bool is_line_for_instrument(int nInstr) { return m_nInstr == nInstr; }
    inline bool is_line_for_voice(int nVoice) { return m_nVoice == 0 || m_nVoice == nVoice; }
    inline bool is_voiced_defined() { return m_nVoice != 0; }
    inline void set_voice(int nVoice) { m_nVoice = nVoice; }
    inline int get_instrument() { return m_nInstr; }
    inline int get_voice() { return m_nVoice; }
    bool contains_barline();
    bool contains_visible_barline();

    //other
    void add_shapes(GmoBoxSliceInstr* pSliceInstrBox);
    void delete_shapes();

    //Debug and Unit Tests
    void dump_music_line(ostream& outStream);

protected:
    void determine_barline_type();

};


//---------------------------------------------------------------------------------------
//ColumnStorage: encapsulates the information for a column
//---------------------------------------------------------------------------------------
class ColumnStorage
{
protected:
	std::vector<MusicLine*> m_Lines;	    //lines that form this column

    //column measurements
    LUnits m_uTrimmedSize;
    LUnits m_uJustifiedSize;
    LUnits m_uFixedSpaceForNextColumn;
    LUnits m_uEndHookWidth;              //variable space at end of column

    LUnits m_uxStart;                   //X0
    LUnits m_uStartFixedSpace;          //SFS
    LUnits m_uxFirstSymbol;             //XS
    LUnits m_uxFirstAnchor;             //XA
    LUnits m_uxRightEdge;               //XR - right most position
    LUnits m_uxStartOfEndVarSpace;      //XV - start of final var space
    LUnits m_uxFinal;                   //XF - end of column
    bool m_fVisibleBarline;             //ends in visible barline

public:
    ColumnStorage();
    virtual ~ColumnStorage();

    void determine_sizes();

    //lines creation
    inline void set_start_position(LUnits uxStart) { m_uxStart = uxStart; }
    inline void set_initial_space(LUnits fixedSpace) { m_uStartFixedSpace = fixedSpace; }

    //methods to build the lines
    bool include_object(int iLine, int iInstr, ImoStaffObj* pSO, TimeUnits rTime,
                        int nStaff, GmoShape* pShape, bool fInProlog,
                        LUnits xUserShift, LUnits yUserShift);
    void finish_column_measurements(LUnits xStart);

    //access to an item
    MusicLine* front() { return m_Lines.front(); }

    //iterator
    inline LinesIterator begin() { return m_Lines.begin(); }
    inline LinesIterator end() { return m_Lines.end(); }
    inline LinesIterator get_last_line() {
        if (m_Lines.size() == 0)
            return m_Lines.end();
	    LinesIterator it = m_Lines.end();
	    return --it;
    }
    LinesIterator find_line(int line);

    //storage manipulation
    MusicLine* open_new_line(int line, int instr, LUnits uxStart, LUnits fixedSpace);
    void close_all_lines(LUnits xStart);

    //properties
    inline size_t size() { return m_Lines.size(); }
    inline bool is_end_of_table(LinesIterator it) { return it == m_Lines.end(); }

    //column width manipulation
    inline void set_trimmed_width(LUnits size) {
        m_uTrimmedSize = size;
        m_uJustifiedSize = size;
    }
    inline void increment_justified_width(LUnits uIncr) { m_uJustifiedSize += uIncr; }
    inline void set_justified_width(LUnits size) { m_uJustifiedSize = size; }

    //access to column measurements
    inline LUnits get_end_hook_width() { return m_uEndHookWidth; }     //+
    LUnits get_start_hook_width();    //+
    inline LUnits get_trimmed_width() { return m_uTrimmedSize; }
    inline LUnits get_justified_width() { return m_uJustifiedSize; }
    inline LUnits get_fixed_space_at_start() { return m_uStartFixedSpace; }     //+
    inline LUnits get_fixed_space_for_next_column() { return m_uFixedSpaceForNextColumn; } //+

    inline LUnits get_start_of_first_symbol() { return m_uxFirstSymbol; }   //XS
    inline LUnits get_first_anchor_position() { return m_uxFirstAnchor; }   //XA
    inline LUnits get_right_most_edge() { return m_uxRightEdge; }           //XR
    inline LUnits get_start_of_final_var_space() { return m_uxStartOfEndVarSpace; }  //XV
    inline LUnits get_start_of_column() { return m_uxStart; }               //X0
    inline LUnits get_end_of_column() { return m_uxFinal; }                 //XF
    LUnits get_main_width();                                                //main width

    //adding shapes to graphical model
    void add_shapes(GmoBoxSliceInstr* pSliceInstrBox, int iInstr);
    void delete_shapes();

    //debug
    void dump_column_storage(ostream& outStream);

    ////Public methods coded only for Unit Tests
    //inline int get_num_objects_in_line(int iLine) { return (int)m_Lines[iLine]->size(); }

protected:
    void delete_lines();
    LinesIterator start_line(int line, int instr);
    void determine_first_anchor_line();
};


//---------------------------------------------------------------------------------------
//ColumnLayouter: column layout algorithm
//  - explores all lines, by time pos, aligning objects
//---------------------------------------------------------------------------------------
class ColumnLayouter
{
protected:
    LibraryScope& m_libraryScope;
    ColumnStorage* m_pColStorage;       //music lines for this column
    ScoreMeter* m_pScoreMeter;
    bool m_fHasSystemBreak;
    GmoBoxSlice* m_pBoxSlice;           //box for this column
    float m_penalty;                    //penalty for ending the system with this column
    std::vector<LineSpacer*> m_LineSpacers;     //one spacer for each line
    std::vector<GmoBoxSliceInstr*> m_sliceInstrBoxes;   //instr.boxes for this column

    //applicable prolog at start of this column. On entry per staff
    std::vector<ColStaffObjsEntry*> m_prologClefs;
    std::vector<ColStaffObjsEntry*> m_prologKeys;

    //final measurements, when shapes are added to graphical model
    bool m_fHasShapes;
    LUnits m_yMin;
    LUnits m_yMax;


public:
    ColumnLayouter(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                   ColumnStorage* pStorage);
    virtual ~ColumnLayouter();

    //column creation
    void start_column_measurements(LUnits uxStart, LUnits fixedSpace);
    void include_object(int iLine, int iInstr, ImoStaffObj* pSO, TimeUnits rTime,
                        int nStaff, GmoShape* pShape, bool fInProlog);
    void finish_column_measurements(LUnits xStart);
    void save_context(int iInstr, int iStaff, ColStaffObjsEntry* pClefEntry,
                      ColStaffObjsEntry* pKeyEntry);

    inline void initialize() {}
    inline void set_slice_box(GmoBoxSlice* pBoxSlice) { m_pBoxSlice = pBoxSlice; };
    inline GmoBoxSlice* get_slice_box() { return m_pBoxSlice; };

    //methods to compute results
    void do_spacing(bool fTrace = false);
    inline void set_trimmed_width(LUnits size) { m_pColStorage->set_trimmed_width(size); }
    inline void set_justified_width(LUnits size) { m_pColStorage->set_justified_width(size); }
    inline void increment_justified_width(LUnits uIncr) { m_pColStorage->increment_justified_width(uIncr); }
    LUnits redistribute_space(LUnits uNewStart, LUnits uNewWidth, UPoint org);

    //access to info
    bool column_has_barline();
    bool column_has_visible_barline();
    inline LUnits get_main_width() { return m_pColStorage->get_main_width(); }
    inline LUnits get_trimmed_width() { return m_pColStorage->get_trimmed_width(); }
    inline LUnits get_justified_width() { return m_pColStorage->get_justified_width(); }
    inline bool has_system_break() { return m_fHasSystemBreak; }
    inline void set_system_break(bool value) { m_fHasSystemBreak = value; }
    LUnits get_start_of_column();
    bool is_empty_column();
    inline ColStaffObjsEntry* get_prolog_clef(int idx) { return m_prologClefs[idx]; }
    inline ColStaffObjsEntry* get_prolog_key(int idx) { return m_prologKeys[idx]; }
    inline LUnits get_end_hook_width() { return m_pColStorage->get_end_hook_width(); }
    inline LUnits get_start_hook_width() { return m_pColStorage->get_start_hook_width(); }
    inline LUnits get_fixed_space_for_next_column() {
        return m_pColStorage->get_fixed_space_for_next_column();
    }
    inline float get_penalty_factor() { return m_penalty; }

    //boxes and shapes
    void add_shapes_to_boxes(ShapesStorage* pStorage);
    GmoBoxSliceInstr* create_slice_instr(ImoInstrument* pInstr, LUnits yTop);
    inline GmoBoxSliceInstr* get_slice_instr(int iInstr) { return m_sliceInstrBoxes[iInstr]; }
    void set_slice_width(LUnits width);
    void set_slice_final_position(LUnits left, LUnits top);

    //values only valid after having invoked add_shapes_to_boxes() method
    inline bool has_shapes() { return m_fHasShapes; }
    inline LUnits get_y_min() { return m_yMin; }
    inline LUnits get_y_max() { return m_yMax; }

    //support for debug and unit tests
    inline int get_num_lines() { return int(m_pColStorage->size()); }
    void dump_column_data(ostream& outStream);
    void delete_box_and_shapes(ShapesStorage* pStorage);

protected:
    void reserve_space_for_prolog_clefs_keys(int numStaves);
    void compute_spacing();
    void compute_penalty_factor();
    void delete_line_spacers();
    inline bool there_are_objects() { return m_fThereAreObjects; }
    inline bool there_are_lines() { return m_pColStorage->size() > 0; }

    //variables and methods for column traversal ---------------------------------
    bool    m_fThereAreObjects;
    TimeUnits   m_rCurrentTime;     //current tiempos being aligned
    LUnits  m_uCurrentPos;          //xPos to start placing objects

    void create_line_spacers();
    void process_non_timed_at_prolog();
    void process_timed_at_current_timepos();
    void process_non_timed_at_current_timepos();
    //----------------------------------------------------------------------------

};


//---------------------------------------------------------------------------------------
// SystemLayouter: algorithm to layout a system
//---------------------------------------------------------------------------------------
class SystemLayouter
{
protected:
    ScoreLayouter*  m_pScoreLyt;
    LibraryScope&   m_libraryScope;
    ScoreMeter*     m_pScoreMeter;
    ImoScore*       m_pScore;
    ShapesStorage&  m_shapesStorage;
    ShapesCreator*  m_pShapesCreator;
    std::vector<ColumnLayouter*>& m_ColLayouters;
    std::vector<InstrumentEngraver*>& m_instrEngravers;

    LUnits m_uPrologWidth;
    GmoBoxSystem* m_pBoxSystem;
    LUnits m_yMin;
    LUnits m_yMax;

    int m_iSystem;
    int m_iFirstCol;
    int m_iLastCol;
    LUnits m_uFreeSpace;    //free space available on current system
    UPoint m_pagePos;
    bool m_fFirstColumnInSystem;

public:
    SystemLayouter(ScoreLayouter* pScoreLyt, LibraryScope& libraryScope,
                   ScoreMeter* pScoreMeter, ImoScore* pScore,
                   ShapesStorage& shapesStorage,
                   ShapesCreator* pShapesCreator,
                   std::vector<ColumnLayouter*>& colLayouters,
                   std::vector<InstrumentEngraver*>& instrEngravers);
    ~SystemLayouter();

    GmoBoxSystem* create_system_box(LUnits left, LUnits top, LUnits width, LUnits height);
    void engrave_system(LUnits indent, int iFirstCol, int iLastCol, UPoint pos);

    //    void AddTimeGridToBoxSlice(int iCol, GmoBoxSlice* pBSlice);

        //Access to information
    inline void set_prolog_width(LUnits width) { m_uPrologWidth = width; }
    inline LUnits get_prolog_width() { return m_uPrologWidth; }
    inline GmoBoxSystem* get_box_system() { return m_pBoxSystem; }
    inline LUnits get_y_min() { return m_yMin; }
    inline LUnits get_y_max() { return m_yMax; }

protected:
    void reposition_staves(LUnits indent);
    void fill_current_system_with_columns();
    void justify_current_system();
    void engrave_instrument_details();
    void truncate_current_system(LUnits indent);

    void add_column_to_system(int iCol);
    void add_shapes_for_column(int iCol, ShapesStorage* pStorage);
    bool system_must_be_justified();
    void add_initial_line_joining_all_staves_in_system();
    void reposition_slices_and_staffobjs();
    LUnits redistribute_space(int iCol, LUnits uNewStart);
    void redistribute_free_space();
    void engrave_system_details(int iSystem);

    void add_system_prolog_if_necessary();
    LUnits engrave_prolog(int iInstr);
    LUnits determine_column_start_position(int iCol);
    LUnits determine_column_size(int iCol);
    void reposition_and_add_slice_boxes(int iCol, LUnits pos, LUnits size);

    void engrave_attached_objects(ImoStaffObj* pSO, GmoShape* pShape,
                                  int iInstr, int iStaff, int iSystem,
                                  int iCol, int iLine,
                                  ImoInstrument* pInstr);

    void add_auxobjs_shapes_to_model(ImoObj* pAO, GmoShape* pStaffObjShape, int layer);
    void add_auxobj_shape_to_model(GmoShape* pShape, int layer, int iSystem, int iCol,
                                   int iInstr);

    //helpers
    inline bool is_first_column_in_system() { return m_fFirstColumnInSystem; }
};


//---------------------------------------------------------------------------------------
//LineResizer: encapsulates the methods to recompute shapes positions so that the line
//will have the desired width, and to move the shapes to those positions
//---------------------------------------------------------------------------------------
class LineResizer
{
protected:
    MusicLine*  m_pLine;           //table for the line to resize
    LUnits      m_uOldWidth;
    LUnits      m_uNewWidth;
    LUnits      m_uNewStart;
    UPoint      m_sliceOrg;
    LineEntryIterator m_itCurrent;

    //final measurements, when shapes are repositioned
    bool m_fHasShapes;
    LUnits m_yMin;
    LUnits m_yMax;

public:
    LineResizer(MusicLine* pLine, LUnits uOldBarSize, LUnits uNewBarSize,
                LUnits uNewStart, UPoint sliceOrg);

    TimeUnits move_prolog_shapes();
    void reasign_position_to_all_other_objects(LUnits uFizedSizeAtStart);
    LUnits get_time_line_position_for_time(TimeUnits rFirstTime);

    //values only valid after having resized the line
    inline bool has_shapes() { return m_fHasShapes; }
    inline LUnits get_y_min() { return m_yMin; }
    inline LUnits get_y_max() { return m_yMax; }

protected:
//    void InformAttachedObjs();

};


//---------------------------------------------------------------------------------------
//LineSpacer:
//  encapsulates the algorithm to assign spaces and positions to a single line
//---------------------------------------------------------------------------------------
class LineSpacer
{
protected:
    MusicLine*          m_pLine;           //the line to assign space
    float               m_rFactor;          //spacing factor
    ScoreMeter*         m_pMeter;           //for tenths/logical conversion
    LineEntryIterator   m_itCur;            //current entry
    TimeUnits           m_rCurTime;         //current time
	LUnits              m_uxCurPos;         //current xPos at start of current time
    LUnits              m_uxRemovable;      //space that can be removed if required
    LineEntryIterator   m_itNonTimedAtCurPos;
    LUnits              m_uxNotTimedFinalPos;

public:
    LineSpacer(MusicLine* pLineTable, ScoreMeter* pMeter);
    virtual ~LineSpacer() {}

    void process_non_timed_at_prolog(LUnits uSpaceAfterProlog);
    void process_non_timed_at_current_timepos(LUnits uxPos);
    void process_timed_at_current_timepos(LUnits uxPos);
    LUnits determine_next_feasible_position_after(LUnits uxPos);
	inline bool current_time_is(TimeUnits rTime) { return is_equal_time(m_rCurTime, rTime); }
    inline bool are_there_timed_objs() {
        return m_itCur != m_pLine->end()
               && is_equal_time((*m_itCur)->get_timepos(), m_rCurTime);
    }
    inline bool are_there_more_objects() { return (m_itCur != m_pLine->end()); }
    TimeUnits get_next_available_time();
    LUnits get_next_position();

protected:
    void add_shapes_info_to_table();
    void prepare_for_traversing();
    LUnits compute_shift_to_avoid_overlap_with_previous();
    void drag_any_previous_clef_to_place_it_near_this_one();

    inline bool is_non_timed_object(LineEntryIterator it) {
        return (it != m_pLine->end() && (*it)->get_timepos() < 0.0f);
    }
    inline bool is_timed_object(LineEntryIterator it) {
        return (it != m_pLine->end() && (*it)->get_timepos() >= 0.0f);
    }
    inline bool is_current_object_non_timed() { return is_non_timed_object(m_itCur); }

    virtual void assign_fixed_and_variable_space(LineEntry* pEntry);
    void set_note_rest_space(LineEntry* pEntry);
    LUnits compute_ideal_distance(LineEntry* pEntry);
    LUnits compute_ideal_distance_fixed(LineEntry* pEntry);
    LUnits compute_ideal_distance_proportional(LineEntry* pEntry);
    void assign_variable_space(LineEntry* pEntry, LUnits uIdeal);
    void assign_no_space(LineEntry* pEntry);
    void assign_minimum_fixed_space(LineEntry* pEntry);


    //variables and methods used only to position non-timed objects ------
    // create helper object?
    LUnits m_uxMaxOcuppiedSpace;
    LUnits m_uxMinOcuppiedSpace;

    void compute_max_and_min_occupied_space();
    void position_non_timed();
    void shift_non_timed(LUnits uxShift);

    void position_using_max_space_with_shift(LUnits uShift);
    void position_using_min_space_with_shift(LUnits uShift);

};


//---------------------------------------------------------------------------------------
//TimeGridTable:
//  A table with occupied times and durations, and connecting time with position
//---------------------------------------------------------------------------------------

//an item in the positions and times table
typedef struct
{
    TimeUnits rTimepos;
    TimeUnits rDuration;
    LUnits uxPos;
}
PosTimeItem;

//the table
class TimeGridTable
{
protected:
    ColumnStorage* m_pColStorage;
    std::vector<PosTimeItem> m_PosTimes;         //the table

public:
    TimeGridTable(ColumnStorage* pColStorage);
    ~TimeGridTable();

    inline int get_size() { return (int)m_PosTimes.size(); }

    //access to an entry values
    inline TimeUnits get_timepos(int iItem) { return m_PosTimes[iItem].rTimepos; }
    inline TimeUnits get_duration(int iItem) { return m_PosTimes[iItem].rDuration; }
    inline LUnits get_x_pos(int iItem) { return m_PosTimes[iItem].uxPos; }

    //access by position
    TimeUnits get_time_for_position(LUnits uxPos);

//    //debug
//    void dump();

protected:
    //variables and methods for column traversal
    std::vector<TimeGridLineExplorer*> m_LineExplorers;
    TimeUnits m_rCurrentTime;
    TimeUnits m_rMinDuration;
    LUnits m_uCurPos;
    bool m_fTimedObjectsFound;

    inline bool timed_objects_found() { return m_fTimedObjectsFound; }
    bool there_are_objects();
    void create_line_explorers();
    void delete_line_explorers();
    void skip_non_timed_at_current_timepos();
    void find_shortest_noterest_at_current_timepos();
    void create_table_entry();
    void get_current_time();
    void interpolate_missing_times();

};

//---------------------------------------------------------------------------------------
// helper class to interpolate missing entries
//---------------------------------------------------------------------------------------
class TimeInserter
{
protected:
    std::vector<PosTimeItem>& m_PosTimes;

    std::vector<PosTimeItem>::iterator  m_itInsertionPoint;
    TimeUnits m_rTimeBeforeInsertionPoint;
    LUnits m_uPositionBeforeInsertionPoint;

public:
    TimeInserter(std::vector<PosTimeItem>& oPosTimes);
    void interpolate_missing_times();

protected:
    bool is_time_in_table(TimeUnits rTimepos);
    void find_insertion_point(TimeUnits rTimepos);
    void insert_time_interpolating_position(TimeUnits rTimepos);

};


//---------------------------------------------------------------------------------------
// helper class to encapsulate the line traversal algorithm
// for creating the time-pos table
//---------------------------------------------------------------------------------------
class TimeGridLineExplorer
{
private:
    MusicLine* m_pLine;           //the line to assign space
    LineEntryIterator m_itCur;            //current entry
    TimeUnits m_rCurTime;
	LUnits m_uCurPos;
    LUnits m_uShiftToNoteRestCenter;
    TimeUnits m_rMinDuration;

public:
    TimeGridLineExplorer(MusicLine* pLineTable);
    ~TimeGridLineExplorer();

    bool skip_non_timed_at_current_timepos();
    bool find_shortest_noterest_at_current_timepos();
    inline bool there_are_objects() { return (m_itCur != m_pLine->end()); }
    TimeUnits get_current_time();
    TimeUnits get_duration_for_found_entry();
    LUnits get_position_for_found_entry();

protected:
    inline bool is_current_object_non_timed() {
        return (m_itCur != m_pLine->end() && (*m_itCur)->get_timepos() < 0.0f);
    }

    inline bool current_object_is_timed() {
        return (m_itCur != m_pLine->end() && (*m_itCur)->get_timepos() >= 0.0f);
    }
};



//---------------------------------------------------------------------------------------
//ColumnResizer: encapsulates the methods to recompute shapes positions so that the
//column will have the desired width, and to move the shapes to those positions
//---------------------------------------------------------------------------------------
class ColumnResizer
{
protected:
    ColumnStorage* m_pColStorage;      //column to resize
    LUnits m_uNewWidth;
    LUnits m_uOldWidth;
    LUnits m_uNewStart;
    UPoint m_sliceOrg;
    TimeUnits m_rFirstTime;
    LUnits m_uFixedPart;
    std::vector<LineResizer*> m_LineResizers;

    //final measurements, when shapes are moved to final positions
    bool m_fHasShapes;
    LUnits m_yMin;
    LUnits m_yMax;

public:
    ColumnResizer(ColumnStorage* pColStorage, LUnits uNewBarSize);
    LUnits reposition_shapes(LUnits uNewStart, LUnits uNewWidth, UPoint org);

    //values only valid after having invoked determine_vertical_limits() method
    inline bool has_shapes() { return m_fHasShapes; }
    inline LUnits get_y_min() { return m_yMin; }
    inline LUnits get_y_max() { return m_yMax; }

protected:
    void create_line_resizers();
    void move_prolog_shapes_and_get_initial_time();
    void reposition_all_other_shapes();
    void determine_fixed_size_at_start_of_column();
    void determine_vertical_limits();
    void delete_line_resizers();

};


}   //namespace lomse

#endif    // __LOMSE_SYSTEM_LAYOUTER_H__


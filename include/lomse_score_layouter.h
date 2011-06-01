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

#ifndef __LOMSE_SCORE_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_SCORE_LAYOUTER_H__

#include "lomse_content_layouter.h"
#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_score_enums.h"
#include "lomse_logger.h"
#include "lomse_shapes_storage.h"
#include <vector>
using namespace std;

namespace lomse
{

//forward declarations
class InternalModel;
class FontStorage;
class GraphicModel;
class ImoContentObj;
class ImoScore;
class ImoStaffObj;
class ImoAuxObj;
class ImoInstrument;
class ImoRelObj;
class ImoTimeSignature;
class GmoBoxScorePage;
class GmoBoxSlice;
class GmoBoxSystem;
class GmoBoxSliceInstr;
class GmoStubScore;
class InstrumentEngraver;
class SystemLayouter;
class SystemCursor;
class GmoShape;
class ScoreMeter;
class ColumnLayouter;
class ColumnStorage;
class ColumnsBuilder;
class ShapesCreator;


//---------------------------------------------------------------------------------------
// helper struct to store data about aux objs to be engraved when the system is ready
struct PendingAuxObjs
{
    ImoStaffObj* m_pSO;
    GmoShape* m_pMainShape;
    ImoInstrument* m_pInstr;
    int m_iInstr;
    int m_iStaff;
    int m_iCol;
    int m_iLine;

    PendingAuxObjs(ImoStaffObj* pSO, GmoShape* pMainShape, int iInstr, int iStaff,
                   int iCol, int iLine,
                   ImoInstrument* pInstr)
        : m_pSO(pSO)
        , m_pMainShape(pMainShape)
        , m_pInstr(pInstr)
        , m_iInstr(iInstr)
        , m_iStaff(iStaff)
        , m_iCol(iCol)
        , m_iLine(iLine)
    {
    }

};


//---------------------------------------------------------------------------------------
class ScoreLayouter : public ContentLayouter
{
protected:
    LibraryScope&   m_libraryScope;
    UPoint          m_pageCursor;
    ImoScore*       m_pScore;
    ScoreMeter*     m_pScoreMeter;
    ColumnsBuilder* m_pColsBuilder;
    ShapesStorage   m_shapesStorage;
    ShapesCreator*  m_pShapesCreator;

    std::vector<InstrumentEngraver*> m_instrEngravers;
    std::vector<ColumnLayouter*> m_ColLayouters;
    std::vector<SystemLayouter*> m_sysLayouters;
    std::vector<int> m_breaks;

    //temposry data about current page being layouted
    int m_iCurPage;     //[0..n-1] current page. (-1 if no page yet created!)

    //temporary data about current syustem being layouted
    int m_iCurSystem;   //[0..n-1] Current system (-1 if no system yet created!)
    SystemLayouter* m_pCurSysLyt;

    int m_iCurColumn;   //[0..n-1] current column. (-1 if no column yet created!)

    //renderization options and parameters
    bool                m_fStopStaffLinesAtFinalBarline;
    bool                m_fJustifyFinalBarline;

    //space values to use
    LUnits              m_uFirstSystemIndent;
    LUnits              m_uOtherSystemIndent;

    //current boxes being layouted
    GmoStubScore*       m_pStubScore;
    GmoBoxScorePage*    m_pCurBoxPage;
    GmoBoxSystem*       m_pCurBoxSystem;

    //support for debug and unit test
    int                 m_iColumnToTrace;

public:
    ScoreLayouter(ImoContentObj* pImo, GraphicModel* pGModel, LibraryScope& libraryScope);
    virtual ~ScoreLayouter();

    void prepare_to_start_layout();
    void layout_in_page(GmoBox* pContainerBox);
    GmoBox* create_main_box();

    //info
    virtual int get_num_columns();
    SystemLayouter* get_system_layouter(int iSys) { return m_sysLayouters[iSys]; }

    //support for helper classes
    virtual LUnits get_target_size_for_system(int iSystem);
    virtual LUnits get_main_width(int iCol);
    virtual LUnits get_trimmed_width(int iCol);
    virtual bool column_has_system_break(int iCol);
    virtual float get_column_penalty(int iCol);

    //support for debuggin and unit tests
    void dump_column_data(int iCol, ostream& outStream=dbgLogger);
    void delete_not_used_objects();
    inline void trace_column(int iCol) { m_iColumnToTrace = iCol; }

protected:
    friend class ColumnsBuilder;
    friend class SystemLayouter;

    //helpers
    inline bool is_first_page() { return m_iCurPage == 0; }
    inline LUnits get_system_indent() {
        return (m_iCurSystem == 0 ? m_uFirstSystemIndent : m_uOtherSystemIndent);
    }
    inline int get_num_systems() { return int(m_breaks.size()); }
    inline bool is_last_system() { return m_iCurSystem == get_num_systems() - 1; }
    inline bool is_first_system_in_score() { return m_iCurSystem == 0; }

    //---------------------------------------------------------------
    void create_stub_for_score();
    void create_instrument_engravers();
    void decide_systems_indentation();
    void add_score_titles();
    bool enough_space_in_page();
    void create_system();
    void add_system_to_page();
    void decide_line_breaks();
    void page_initializations(GmoBox* pContainerBox);
    void decide_line_sizes();
    void fill_page_with_empty_systems_if_required();

    void delete_instrument_engravers();
    void delete_system_layouters();
    void get_score_renderization_options();

    bool m_fFirstSystemInPage;
    inline void is_first_system_in_page(bool value) { m_fFirstSystemInPage = value; }
    inline bool is_first_system_in_page() { return m_fFirstSystemInPage; }

    //---------------------------------------------------------------
    void move_cursor_to_top_left_corner();
    void move_cursor_after_headers();
    LUnits remaining_height();
    void create_system_layouter();
    void create_system_box();
    void engrave_system();
    void create_empty_system();
    void engrave_empty_system();

    void advance_paper_cursor_to_bottom_of_added_system();
    LUnits get_first_system_staves_size();
    LUnits get_other_systems_staves_size();

    //---------------------------------------------------------------
    LUnits determine_system_top_margin();
    LUnits determine_top_space(int nInstr, bool fFirstSystemInScore=false,
                               bool fFirstSystemInPage=false);

    void determine_staff_lines_horizontal_position(int iInstr);
    LUnits space_used_by_prolog(int iSystem);
    LUnits distance_to_top_of_system(int iSystem, bool fFirstInPage);

    std::list<PendingAuxObjs*> m_pendingAuxObjs;


    //---------------------------------------------------------------
    int get_system_containing_column(int iCol);

    void delete_column_layouters();

    bool is_system_empty(int iSystem);

};


//---------------------------------------------------------------------------------------
// ColumnsBuilder: algorithm to build the columns for one score
class ColumnsBuilder
{
protected:
    LibraryScope&   m_libraryScope;
    ScoreMeter*     m_pScoreMeter;
    ScoreLayouter*  m_pScoreLyt;
    ImoScore*       m_pScore;
    ShapesStorage&  m_shapesStorage;
    ShapesCreator*  m_pShapesCreator;
    std::vector<ColumnLayouter*>& m_ColLayouters;    //layouter for each column
    std::vector<InstrumentEngraver*>& m_instrEngravers;
    SystemCursor* m_pSysCursor;                     //cursor for traversing the score
    std::vector<LUnits> m_SliceInstrHeights;
    LUnits m_stavesHeight;      //system height without top and bottom margins
    UPoint m_pagePos;           //to track current position

    int m_iColumn;   //[0..n-1] current column. (-1 if no column yet created!)

    int m_iColumnToTrace;   //support for debug and unit test

public:
    ColumnsBuilder(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                   ScoreLayouter* pScoreLyt, ImoScore* pScore,
                   ShapesStorage& shapesStorage,
                   ShapesCreator* pShapesCreator,
                   std::vector<ColumnLayouter*>& colLayouters,
                   std::vector<InstrumentEngraver*>& instrEngravers);
    ~ColumnsBuilder();


    void create_columns();
    inline LUnits get_staves_height() { return m_stavesHeight; }

    //support for debuggin and unit tests
    inline void trace_column(int iCol) { m_iColumnToTrace = iCol; }

protected:
    void determine_staves_vertical_position();
    void create_column();

    void create_column_layouter();
    void create_column_boxes();
    void collect_content_for_this_column();
    void layout_column();
    void assign_width_to_column();

    GmoBoxSlice* create_slice_box();
    void find_and_save_context_info_for_this_column();

    void delete_column_storage();
    LUnits determine_initial_fixed_space();

    void store_info_about_attached_objects(ImoStaffObj* pSO, GmoShape* pShape,
                                  int iInstr, int iStaff, int iCol, int iLine,
                                  ImoInstrument* pInstr);

    //column creation
    void start_column_measurements(int iCol, LUnits uxStart, LUnits fixedSpace);
    void include_object(int iCol, int iLine, int iInstr, ImoStaffObj* pSO,
                        float rTime, int nStaff, GmoShape* pShape, bool fInProlog=false);
    void finish_column_measurements(int iCol, LUnits xStart);
    bool determine_if_is_in_prolog(float rTime);

    //helpers
    inline bool is_first_column() { return m_iColumn == 0; }

};


//---------------------------------------------------------------------------------------
// ColumnBreaker: algorithm to decide when to finish a column
class ColumnBreaker
{
protected:
    int m_numInstruments;
    bool m_fBarlineFound;
    float m_targetBreakTime;
    int m_numLines;
    std::vector<ImoStaffObj*> m_staffObjs;
    std::vector<bool> m_beamed;

public:
    ColumnBreaker(int numInstruments, SystemCursor* pSysCursor);
    ~ColumnBreaker() {}

    bool column_should_be_finished(ImoStaffObj* pSO, float rTime, int iLine);
};


//---------------------------------------------------------------------------------------
// ShapesCreator: helper fcatory to create staffobjs shapes
class ShapesCreator
{
protected:
    LibraryScope& m_libraryScope;
    ScoreMeter* m_pScoreMeter;
    ShapesStorage& m_shapesStorage;
    std::vector<InstrumentEngraver*>& m_instrEngravers;

public:
    ShapesCreator(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                  ShapesStorage& shapesStorage,
                  std::vector<InstrumentEngraver*>& instrEngravers);
    ~ShapesCreator() {}

    enum {k_flag_small_clef=1, };

    GmoShape* create_staffobj_shape(ImoStaffObj* pSO, int iInstr, int iStaff,
                                    UPoint pos, int clefType=0, unsigned flags=0);
    GmoShape* create_auxobj_shape(ImoAuxObj* pAO, int iInstr, int iStaff,
                                  GmoShape* pParentShape, UPoint pos);


    void start_engraving_relobj(ImoAuxObj* pAO, ImoRelObj* pRO, ImoStaffObj* pSO,
                                GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                int iSystem, int iCol, int iLine, ImoInstrument* pInstr,
                                UPoint pos);
    void continue_engraving_relobj(ImoAuxObj* pAO, ImoRelObj* pRO, ImoStaffObj* pSO,
                                   GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                   int iSystem, int iCol, int iLine,
                                   ImoInstrument* pInstr);
    void finish_engraving_relobj(ImoAuxObj* pAO, ImoRelObj* pRO, ImoStaffObj* pSO,
                                 GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                 int iSystem, int iCol, int iLine, LUnits prologWidth,
                                 ImoInstrument* pInstr);

protected:

};


//---------------------------------------------------------------------------------------
// LinesBreaker: base class for any lines break algorithm
class LinesBreaker
{
protected:
    ScoreLayouter* m_pScoreLyt;
    std::vector<int>& m_breaks;

public:
    LinesBreaker(ScoreLayouter* pScoreLyt, std::vector<int>& breaks)
        : m_pScoreLyt(pScoreLyt)
        , m_breaks(breaks)
    {
    }
    virtual ~LinesBreaker() {}

    virtual void decide_line_breaks() = 0;
};


//---------------------------------------------------------------------------------------
// Simple lines break algorithm: just fill systems with columns
class LinesBreakerSimple : public LinesBreaker
{
public:
    LinesBreakerSimple(ScoreLayouter* pScoreLyt, std::vector<int>& breaks);
    virtual ~LinesBreakerSimple() {}

    void decide_line_breaks();
};


//---------------------------------------------------------------------------------------
// Optimal lines break algorithm, based on Knuths' algorithm for text processors
class LinesBreakerOptimal : public LinesBreaker
{
public:
    LinesBreakerOptimal(ScoreLayouter* pScoreLyt, std::vector<int>& breaks);
    virtual ~LinesBreakerOptimal() {}

    void decide_line_breaks();

    //support for debug and tests
    void dump_entries(ostream& outStream=dbgLogger);

protected:
    struct Entry
    {
        float penalty;          //total penalty for the score
        int predecessor;        //previous break point
        int system;             //system [0..n] started by this entry
        float product;          //total product of not used space
    };
    std::vector<Entry> m_entries;
    int m_numCols;
    bool m_fJustifyLastLine;

    void initialize_entries_table();
    void compute_optimal_break_sequence();
    void retrieve_breaks_sequence();
    float determine_penalty_for_line(int iSystem, int i, int j);
    bool is_better_option(float totalPenalty, float curPenalty, int i, int j);

};



}   //namespace lomse

#endif    // __LOMSE_SCORE_LAYOUTER_H__


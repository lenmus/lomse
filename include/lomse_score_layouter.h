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
#include "lomse_shapes_storage.h"
#include "lomse_logger.h"
#include <vector>
using namespace std;

namespace lomse
{

//forward declarations
class InternalModel;
class FontStorage;
class GraphicModel;
class ImoDocObj;
class ImoScore;
class ImoStaffObj;
class ImoAuxObj;
class ImoInstrument;
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
class LinesBuilder;
//class ColumnsBuilder;


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
    SystemCursor*   m_pSysCursor;
    ScoreMeter*     m_pScoreMeter;
//    ColumnsBuilder* m_pColsBuilder;

    //auxiliary data for computing and justifying systems
    int m_iCurSystem;   //[0..n-1] Current system (-1 if no system yet created!)
    int m_iCurColumn;   //[0..n-1] current column. (-1 if no column yet created!)
    int m_iCurPage;     //[0..n-1] current page. (-1 if no page yet created!)
    bool m_fFirstColumnInSystem;


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
    GmoBoxSlice*        m_pCurSlice;

    std::vector<InstrumentEngraver*> m_instrEngravers;

public:
    ScoreLayouter(ImoDocObj* pImo, GraphicModel* pGModel, LibraryScope& libraryScope);
    virtual ~ScoreLayouter();

    void prepare_to_start_layout();
    void layout_in_page(GmoBox* pContainerBox);
    GmoBox* create_pagebox(GmoBox* pParentBox);

    //only for unit tests
    SystemLayouter* get_system_layouter(int iSys) { return m_sysLayouters[iSys]; }
    void dump_column_data(int iCol, ostream& outStream=dbgLogger);
    int get_num_columns();
    int get_num_lines_in_column(int iCol);
    inline std::vector<int>& get_line_breaks() { return m_breaks; }


protected:

    //helpers
    ImoScore* get_imo_score();
    inline bool is_first_column_in_system() { return m_fFirstColumnInSystem; }
    inline bool is_first_page() { return m_iCurPage == 0; }
    inline LUnits get_system_indent() {
        return (m_iCurSystem == 0 ? m_uFirstSystemIndent : m_uOtherSystemIndent);
    }
    InstrumentEngraver* get_instrument_engraver(int iInstr);
    inline int get_num_systems() { return int(m_breaks.size()); }
    inline bool is_last_system() { return m_iCurSystem == get_num_systems() - 1; }
    inline bool is_first_system_in_score() { return m_iCurSystem == 0; }


    //level 1: invoked from public methods
    //---------------------------------------------------------------
    void initialize();
    void create_stub_for_score();
    void create_instrument_engravers();
    void decide_systems_indentation();
    void add_score_titles();
    bool enough_space_in_page();
    void create_system();
    void add_system_to_page();
    void decide_line_breaks();
    void create_columns();
    void page_initializations(GmoBox* pContainerBox);
    void decide_line_sizes();

    void delete_instrument_engravers();
    void delete_system_layouters();
    void get_score_renderization_options();
    void create_system_cursor();
    void delete_system_cursor();
    void add_initial_line_joining_all_staves_in_system();

    bool m_fFirstSystemInPage;
    inline void is_first_system_in_page(bool value) { m_fFirstSystemInPage = value; }
    inline bool is_first_system_in_page() { return m_fFirstSystemInPage; }

    //level 2: invoked from level 1 methods
    //---------------------------------------------------------------
    void move_cursor_to_top_left_corner();
    void move_cursor_after_headers();
    LUnits remaining_height();
    void create_system_box(SystemLayouter* pSysLyt);
    void determine_staves_vertical_position();
    void advance_paper_cursor_to_bottom_of_added_system();
    void fill_current_system_with_columns();
    void justify_current_system();
    void truncate_current_system();
    void engrave_instrument_details();
    LUnits get_first_system_staves_size();
    LUnits get_other_systems_staves_size();
    SystemLayouter* create_system_layouter();

    //SystemCursor* m_pSysCursor;
    std::vector<SystemLayouter*> m_sysLayouters;
    int m_nColumnsInSystem;     //the number of columns in current system
    LUnits m_stavesHeight;      //system height without top and bottom margins

    //level 3: invoked from level 2 methods
    //---------------------------------------------------------------
    void create_column();
    bool enough_space_in_system();
    inline bool must_finish_system() { return m_fTerminateSystem; }
    inline void must_finish_system(bool value) { m_fTerminateSystem = value; }
    bool system_must_be_justified();
    void redistribute_free_space();

    bool m_fTerminateSystem;


    //level 4: invoked from level 3 methods
    //---------------------------------------------------------------
    void create_column_boxes();
    void collect_content_for_this_column();
    void measure_this_column();
    void add_column_to_system();
    LUnits get_available_space_in_system(int iSystem);
    void engrave_system_details();
    void set_slice_box_size(int iCol, LUnits width);
    void reposition_slices_and_staffobjs();

    LUnits m_uFreeSpace;    //free space available on current system


    //level 5: invoked from level 4 methods
    //---------------------------------------------------------------
    void add_system_prolog();
    void create_slice_box();
    LUnits determine_top_space(int nInstr);
    void determine_staff_lines_horizontal_position(int iInstr);
    void add_shapes_for_column(int iCol, ShapesStorage* pStorage);
    LUnits determine_initial_space();
    LUnits space_used_by_prolog(int iSystem);
    LUnits distance_to_top_of_system(int iSystem, bool fFirstInPage);

    GmoBoxSliceInstr* m_pCurBSI;


    //level 6: invoked from level 5 methods
    //---------------------------------------------------------------
    GmoShape* create_staffobj_shape(ImoStaffObj* pSO, int iInstr, int iStaff,
                                    unsigned flags=0);
    GmoShape* create_auxobj_shape(ImoAuxObj* pAO, int iInstr, int iStaff,
                                  GmoShape* pParentShape);
    void store_info_about_attached_objects(ImoStaffObj* pSO, GmoShape* pShape,
                                  int iInstr, int iStaff, int iCol, int iLine,
                                  ImoInstrument* pInstr);
    void engrave_attached_objects(ImoStaffObj* pSO, GmoShape* pShape,
                                  int iInstr, int iStaff, int iSystem,
                                  int iCol, int iLine,
                                  ImoInstrument* pInstr);
    LUnits engrave_prolog(int iInstr);

    enum {k_flag_small_clef=1, };
    ShapesStorage m_shapesStorage;
    std::list<PendingAuxObjs*> m_pendingAuxObjs;


    //level 7: invoked from level 6 methods
    //---------------------------------------------------------------
    void start_engraving_auxobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                int iSystem, int iCol, int iLine, ImoInstrument* pInstr);
    void continue_engraving_auxobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                   GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                   int iSystem, int iCol, int iLine,
                                   ImoInstrument* pInstr);
   void finish_engraving_auxobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                int iSystem, int iCol, int iLine,
                                ImoInstrument* pInstr);
    void add_auxobjs_shapes_to_model(ImoAuxObj* pAO, GmoShape* pStaffObjShape, int layer);
    void add_auxobj_shape_to_model(GmoShape* pShape, int layer, int iSystem, int iCol,
                                   int iInstr);
    int get_system_including_col(int iCol);

    //column creation --------------------------------------------------------------
    void delete_column_layouters();

    //caller informs that a new collumn is going to be layouted
    void prepare_for_new_column(GmoBoxSlice* pBoxSlice);

    //caller ask to prepare for receiving data about column iCol [0..n-1] for
    //the given instrument
    void start_bar_measurements(int iCol, LUnits uxStart, LUnits uSpace);

    //caller sends data about one staffobj in column iCol [0..n-1]
    void include_object(int iCol, int iLine, int iInstr, ImoInstrument* pInstr,
                        ImoStaffObj* pSO, float rTime, int nStaff, GmoShape* pShape);

    //caller sends lasts object to store in column iCol [0..n-1].
    void include_barline_and_finish_bar_measurements(int iCol, int iLine,
                                                     ImoStaffObj* pSO, GmoShape* pShape,
                                                     LUnits xStart, float rTime);

    //caller informs that there are no barline and no more objects in column iCol [0..n-1].
    //void finish_bar_measurements_without_barline(int iCol, LUnits xStart, float rTime);
    void finish_bar_measurements(int iCol, LUnits xStart);

    //caller request to ignore measurements for last column
    void discard_data_for_current_column(ShapesStorage* pStorage);

        // Processing
    void do_column_spacing(int iCol, bool fTrace = false);
    LUnits redistribute_space(int iCol, LUnits uNewStart);

        //Operations
    void increment_column_size(int iCol, LUnits uIncr);
    void add_shapes_to_column(int iCol, ShapesStorage* pStorage);

        //Access to information
    LUnits get_start_position_for_column(int iCol);
    bool is_system_empty(int iSystem);
    LUnits get_minimum_size(int iCol);
//    //bool get_optimum_break_point(int iCol, LUnits uAvailable, float* prTime,
//    //                          LUnits* puWidth);
    bool column_has_barline(int iCol);
    GmoBoxSliceInstr* get_slice_instr_box_for(int iCol, int iInstr);
    GmoBoxSlice* get_slice_box_for_column(int iCol);

    //columns for this score
    std::vector<ColumnLayouter*> m_ColLayouters;    //layouter for each column
    std::vector<ColumnStorage*> m_ColStorage;       //data for each column
    std::vector<LinesBuilder*> m_LinesBuilder;      //lines builder for each column
    std::vector<int> m_breaks;                      //abs. column (1..m) for system start
    std::vector<LUnits> m_SliceInstrHeights;


    //------------------------------------------------------------------------------

};


////---------------------------------------------------------------------------------------
//// ColumnsBuilder: algorithm to build the columns for one score
//class ColumnsBuilder
//{
//protected:
//    ScoreLayouter* m_pScoreLyt;
//    std::vector<ColumnLayouter*>& m_ColLayouters;    //layouter for each column
//    std::vector<ColumnStorage*>& m_ColStorage;       //data for each column
//    std::vector<LinesBuilder*>& m_LinesBuilder;      //lines builder for each column
//    std::vector<InstrumentEngraver*>& m_instrEngravers;
//    SystemCursor* m_pSysCursor;                     //cursor for traversing the score
//
//
//    //std::vector<int> m_breaks;                      //abs. column (1..m) for system start
//
//public:
//    ColumnsBuilder(ScoreLayouter* pScoreLyt, std::vector<ColumnLayouter*>& colLayouters,
//                   std::vector<ColumnStorage*>& colStorage,
//                   std::vector<LinesBuilder*>& linesBuilder,
//                   std::vector<InstrumentEngraver*>& instrEngravers)
//        : m_pScoreLyt(pScoreLyt)
//        , m_ColLayouters(colLayouters)
//        , m_ColStorage(colStorage)
//        , m_LinesBuilder(linesBuilder)
//        , m_instrEngravers(instrEngravers)
//        , m_pSysCursor(NULL)
//    {
//    }
//    ~ColumnsBuilder() {
//        delete_system_cursor();
//    }
//
//    void create_columns();
//
//protected:
//	//determine_staves_vertical_position();
// //       create_column();
//    //delete_system_cursor();
//    //create_system_cursor();
//
//    //create_column_boxes();
//    //collect_content_for_this_column();
//    //measure_this_column();
//
////    //column creation --------------------------------------------------------------
////
////    GmoBoxSliceInstr* create_slice_instr(int iCol, ImoInstrument* pInstr, LUnits yTop);
////
////    //caller informs that a new collumn is going to be layouted
////    void prepare_for_new_column(GmoBoxSlice* pBoxSlice);
////
////    //caller ask to prepare for receiving data about column iCol [0..n-1] for
////    //the given instrument
////    void start_bar_measurements(int iCol, LUnits uxStart, LUnits uSpace);
////
////    //caller sends data about one staffobj in column iCol [0..n-1]
////    void include_object(int iCol, int iLine, int iInstr, ImoInstrument* pInstr,
////                        ImoStaffObj* pSO, float rTime, bool fProlog, int nStaff,
////                        GmoShape* pShape);
////
////    //caller sends lasts object to store in column iCol [0..n-1].
////    void include_barline_and_finish_bar_measurements(int iCol, int iLine,
////                                                     ImoStaffObj* pSO, GmoShape* pShape,
////                                                     LUnits xStart, float rTime);
////
////    //caller informs that there are no barline and no more objects in column iCol [0..n-1].
////    //void finish_bar_measurements_without_barline(int iCol, LUnits xStart, float rTime);
////    void finish_bar_measurements(int iCol, LUnits xStart);
////
////    //caller request to ignore measurements for last column
////    void discard_data_for_current_column(ShapesStorage* pStorage);
////
////        // Processing
////    void do_column_spacing(int iCol, bool fTrace = false);
////    LUnits redistribute_space(int iCol, LUnits uNewStart);
////
////        //Operations
////    void increment_column_size(int iCol, LUnits uIncr);
////    void add_shapes_to_column(int iCol, ShapesStorage* pStorage);
////
////        //Access to information
////    LUnits get_start_position_for_column(int iCol);
////    bool is_system_empty(int iSystem);
////    LUnits get_minimum_size(int iCol);
//////    //bool get_optimum_break_point(int iCol, LUnits uAvailable, float* prTime,
//////    //                          LUnits* puWidth);
////    bool column_has_barline(int iCol);
////    GmoBoxSliceInstr* get_slice_instr_box_for(int iCol, int iInstr);
////    GmoBoxSlice* get_slice_box_for_column(int iCol);
//
//};


}   //namespace lomse

#endif    // __LOMSE_SCORE_LAYOUTER_H__


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


//---------------------------------------------------------------------------------------
class ScoreLayouter : public ContentLayouter
{
protected:
    LibraryScope&   m_libraryScope;
    UPoint          m_pageCursor;
    SystemCursor*   m_pSysCursor;
    ScoreMeter*     m_pScoreMeter;

    //auxiliary data for computing and justifying systems
    int m_nCurSystem;   //[0..n-1] Current system number
    int m_nAbsColumn;   //[0..n-1] number of column in process, absolute
    int m_nRelColumn;   //[0..n-1] number of column in process, relative to current system

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
    int                 m_nCurrentPageNumber;       //1..n. if 0 no page yet created!
    LUnits              m_uLastSystemHeight;
//

public:
    ScoreLayouter(ImoDocObj* pImo, GraphicModel* pGModel, LibraryScope& libraryScope);
    virtual ~ScoreLayouter();

    void layout_in_page(GmoBox* pContainerBox);
    GmoBox* create_pagebox(GmoBox* pParentBox);
    void prepare_to_start_layout();


protected:

    //helpers
    inline bool is_first_column_in_system() { return m_nRelColumn == 0; }
    inline bool is_first_page() { return m_nCurrentPageNumber == 0; }
    inline LUnits get_system_indent() {
        return (m_nCurSystem == 1 ? m_uFirstSystemIndent : m_uOtherSystemIndent);
    }
    InstrumentEngraver* get_instrument_engraver(int iInstr);
    inline LUnits tenths_to_logical(Tenths tenths, LUnits lineSpacing) {
        return (tenths * lineSpacing) / 10.0f;
    }


    //level 1: invoked from public methods
    //---------------------------------------------------------------
    void page_initializations(GmoBox* pContainerBox);
    ImoScore* get_imo_score();
    void create_stub_for_score();
    void create_instrument_engravers();
    void decide_systems_indentation();
    void add_titles_if_first_page();
    bool enough_space_in_page();
    void add_next_system();
    void delete_instrument_engravers();
    void delete_system_layouters();
    void get_score_renderization_options();
    void create_system_cursor();
    void delete_system_cursor();
    void add_initial_line_joining_all_staves_in_system();

    std::vector<InstrumentEngraver*> m_instrEngravers;
    bool m_fFirstSystemInPage;
    inline void is_first_system_in_page(bool value) { m_fFirstSystemInPage = value; }
    inline bool is_first_system_in_page() { return m_fFirstSystemInPage; }
    bool m_fThereAreMoreSystems;
    inline void more_systems_to_add(bool value) { m_fThereAreMoreSystems = value; }
    inline bool more_systems_to_add() { return m_fThereAreMoreSystems; }


    //level 2: invoked from level 1 methods
    //---------------------------------------------------------------
    void add_score_titles();
    void move_cursor_to_top_left_corner();
    void move_cursor_after_headers();
    LUnits remaining_height();
    void create_system_layouter();
    void create_system_box();
    void set_system_height_and_advance_paper_cursor();
    void fill_current_system_with_columns();
    void justify_current_system();
    void truncate_current_system();
    void update_box_slices_sizes();

    //SystemCursor* m_pSysCursor;
    std::vector<SystemLayouter*> m_sysLayouters;
    int m_nColumnsInSystem;         //the number of columns in current system


    //level 3: invoked from level 2 methods
    //---------------------------------------------------------------
    void create_column_and_add_it_to_current_system();
    inline bool must_terminate_system() { return m_fTerminateSystem; }
    inline void must_terminate_system(bool value) { m_fTerminateSystem = value; }
    bool system_must_be_justified();
    void redistribute_free_space();
    void reposition_staffobjs();

    bool m_fTerminateSystem;


    //level 4: invoked from level 3 methods
    //---------------------------------------------------------------
    void create_column_boxes();
    void collect_content_for_this_bar();
    void measure_this_bar();
    void add_column_to_system();
    LUnits get_available_space_in_system();

    LUnits m_uFreeSpace;    //free space available on current system


    //level 5: invoked from level 4 methods
    //---------------------------------------------------------------
    void add_slice_box();
    LUnits determine_top_space(int nInstr, ImoInstrument* pInstr);
    void start_slice_instr(ImoInstrument* pInstr, int iInstr, LUnits uTopMargin);
    void terminate_slice_instr(int iInstr, LUnits uBottomMargin);
    void add_staff_lines_name_and_bracket(int iInstr, LUnits uTopMargin);
    void add_shapes_for_score_objs(int iCol);
    LUnits determine_initial_space();
    LUnits space_used_by_prolog();

    GmoBoxSliceInstr* m_pCurBSI;


    //level 6: invoked from level 5 methods
    //---------------------------------------------------------------
    GmoShape* create_staffobj_shape(ImoStaffObj* pSO, LUnits lineSpacing, int iInstr);
    void add_key_signature(ImoStaffObj* pSO, LUnits lineSpacing,
                           int iInstr, bool fProlog);
    void add_time_signature(ImoStaffObj* pSO, LUnits lineSpacing,
                            int iInstr, bool fProlog);
    void add_prolog(int iInstr);


};


}   //namespace lomse

#endif    // __LOMSE_SCORE_LAYOUTER_H__


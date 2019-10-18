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

#ifndef __LOMSE_SCORE_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_SCORE_LAYOUTER_H__

#include "lomse_layouter.h"
#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_score_enums.h"
#include "lomse_logger.h"
#include "lomse_engravers_map.h"
#include "lomse_spacing_algorithm.h"

#include <vector>
using namespace std;

namespace lomse
{

//forward declarations
class FontStorage;
class GraphicModel;
class ImoContentObj;
class ImoScore;
class ImoStaffObj;
class ImoAuxObj;
class ImoInstrument;
class ImoRelObj;
class ImoAuxRelObj;
class ImoTimeSignature;
class GmoBoxScorePage;
class GmoBoxSlice;
class GmoBoxSystem;
class GmoBoxSliceInstr;
class InstrumentEngraver;
class SystemLayouter;
class StaffObjsCursor;
class GmoShape;
class ScoreMeter;
class ColumnStorage;
class ColumnsBuilder;
class ShapesCreator;
class ScoreStub;
class ColumnBreaker;
class PartsEngraver;
class LyricEngraver;
class GmoShapeNote;

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
    int m_idxStaff;

    PendingAuxObjs(ImoStaffObj* pSO, GmoShape* pMainShape, int iInstr, int iStaff,
                   int iCol, int iLine, ImoInstrument* pInstr, int idxStaff)
        : m_pSO(pSO)
        , m_pMainShape(pMainShape)
        , m_pInstr(pInstr)
        , m_iInstr(iInstr)
        , m_iStaff(iStaff)
        , m_iCol(iCol)
        , m_iLine(iLine)
        , m_idxStaff(idxStaff)
    {
    }

};

//---------------------------------------------------------------------------------------
// helper struct to store data about lyric shapes pending to be completed with details
struct PendingLyrics
{
    ImoStaffObj* m_pSO;
    GmoShape* m_pMainShape;
    ImoInstrument* m_pInstr;
    int m_iInstr;
    int m_iStaff;
    int m_iCol;
    int m_iLine;

    PendingLyrics(ImoStaffObj* pSO, GmoShape* pMainShape, int iInstr, int iStaff,
                  int iCol, int iLine, ImoInstrument* pInstr)
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
class ScoreLayouter : public Layouter
{
protected:
    LibraryScope&   m_libraryScope;
    ImoScore*       m_pScore;
    ScoreMeter*     m_pScoreMeter;
    SpacingAlgorithm* m_pSpAlgorithm;
    EngraversMap    m_engravers;
    ShapesCreator*  m_pShapesCreator;
    PartsEngraver*  m_pPartsEngraver;
    UPoint          m_cursor;
    LUnits          m_startTop;

    std::vector<SystemLayouter*> m_sysLayouters;
    std::vector<int> m_breaks;


    //temporary data about current page being laid out
    int m_iCurPage;     //[0..n-1] current page. (-1 if no page yet created!)

    //temporary data about current system being laid out
    int m_iCurSystem;   //[0..n-1] Current system (-1 if no system yet created!)
    SystemLayouter* m_pCurSysLyt;
    int m_iSysPage;     //value of m_iCurPage when system was engraved
    UPoint m_sysCursor; //value of m_cursor when system was engraved

    int m_iCurColumn;   //[0..n-1] current column. (-1 if no column yet created!)

    //renderization options and parameters
    long    m_truncateStaffLines;
    long    m_justifyLastSystem;

    //space values to use
    LUnits              m_uFirstSystemIndent;
    LUnits              m_uOtherSystemIndent;

    //score stub and current boxes being laid out
    ScoreStub*          m_pStub;
    GmoBoxScorePage*    m_pCurBoxPage;
    GmoBoxSystem*       m_pCurBoxSystem;

    //support for debug and unit test
    int                 m_iColumnToTrace;
    int                 m_nTraceLevel;

public:
    ScoreLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                  LibraryScope& libraryScope);
    virtual ~ScoreLayouter();

    void prepare_to_start_layout();
    void layout_in_box();
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height);

    //info
    virtual int get_num_columns();
    SystemLayouter* get_system_layouter(int iSys) { return m_sysLayouters[iSys]; }
    virtual TypeMeasureInfo* get_measure_info_for_column(int iCol);
    virtual GmoShapeBarline* get_start_barline_shape_for_column(int iCol);

    //support for helper classes
    virtual LUnits get_target_size_for_system(int iSystem);
    virtual LUnits get_column_width(int iCol);
    virtual bool column_has_system_break(int iCol);

    //support for building the GmMeasuresTable
        //invoked when a non-middle barline is found
    void finish_measure(int iInstr, GmoShapeBarline* pBarlineShape);

    //support for debugging and unit tests
    void dump_column_data(int iCol, ostream& outStream=dbgLogger);
    void delete_not_used_objects();
    void trace_column(int iCol, int level);
    ColumnData* get_column(int i);

protected:
    void add_error_message(const string& msg);

    friend class ColumnsBuilder;
    friend class SystemLayouter;
    friend class PartsEngraver;

    //helpers
    inline bool is_first_page() { return m_iCurPage == 0; }
    inline LUnits get_system_indent() {
        return (m_iCurSystem == 0 ? m_uFirstSystemIndent : m_uOtherSystemIndent);
    }
    inline int get_num_systems() { return int(m_breaks.size()); }
    inline bool is_last_system() { return m_iCurSystem == get_num_systems() - 1; }
    inline bool is_first_system_in_score() { return m_iCurSystem == 0; }

    //---------------------------------------------------------------
    void initialice_score_layouter();
    void create_parts_engraver();
    void decide_systems_indentation();
    void create_stub();
    void add_score_titles();
    bool enough_space_for_empty_system();
    void create_system();
    void add_system_to_page();
    void decide_line_breaks();
    void page_initializations(GmoBox* pContainerBox);
    void decide_line_sizes();
    void fill_page_with_empty_systems_if_required();
    bool score_page_is_the_only_content_of_parent_box();
    void remove_unused_space();

    void delete_system_layouters();
    void get_score_renderization_options();

    bool m_fFirstSystemInPage;
    inline void is_first_system_in_page(bool value) { m_fFirstSystemInPage = value; }
    inline bool is_first_system_in_page() { return m_fFirstSystemInPage; }

    bool system_created();
    bool enough_space_in_page_for_system();
    void delete_system();

    //---------------------------------------------------------------
    void move_cursor_to_top_left_corner();
    LUnits remaining_height();
    void create_system_layouter();
    void create_system_box();
    void engrave_system();
    void create_empty_system();
    void engrave_empty_system();

    void reposition_system_if_page_has_changed();
    void move_paper_cursor_to_bottom_of_added_system();
    LUnits get_first_system_staves_size();
    LUnits get_other_systems_staves_size();

    //---------------------------------------------------------------
    LUnits determine_system_top_margin();
    LUnits determine_top_space(int nInstr, bool fFirstSystemInScore=false,
                               bool fFirstSystemInPage=false);

    LUnits space_used_by_prolog(int iSystem);
    LUnits distance_to_top_of_system(int iSystem, bool fFirstInPage);

    std::list<PendingAuxObjs*> m_pendingAuxObjs;


    //---------------------------------------------------------------
    int get_system_containing_column(int iCol);

    bool is_system_empty(int iSystem);

};


//---------------------------------------------------------------------------------------
// ColumnBreaker: algorithm to decide when to finish a column
class ColumnBreaker
{
protected:
    int m_breakMode;
    int m_numInstruments;
    int m_consecutiveBarlines;
    int m_numInstrWithTS;
    bool m_fWasInBarlinesMode;
    TimeUnits m_targetBreakTime;
    TimeUnits m_lastBarlineTime;
    TimeUnits m_maxMeasureDuration;
    TimeUnits m_lastBreakTime;
    TimeUnits m_measureMeanTime;

    int m_numLines;
    std::vector<TimeUnits> m_measures;
    std::vector<bool> m_beamed;
    std::vector<bool> m_tied;

public:
    ColumnBreaker(int numInstruments, StaffObjsCursor* pSysCursor);
    virtual ~ColumnBreaker() {}

    bool feasible_break_before_this_obj(ImoStaffObj* pSO, TimeUnits rTime,
                                        int iInstr, int iLine);

protected:

    enum EBreakModes {
        k_undefined = -1,
        k_barlines = 0,         //at common clear barlines
        k_clear_cuts,           //at common clear cuts
    };

    bool is_suitable_note_rest(ImoStaffObj* pSO, TimeUnits rTime);
    void determine_initial_break_mode(StaffObjsCursor* pSysCursor);
    void determine_measure_mean_time(StaffObjsCursor* pSysCursor);

};


//---------------------------------------------------------------------------------------
// ShapesCreator: helper factory to create staffobjs shapes
class ShapesCreator
{
protected:
    LibraryScope& m_libraryScope;
    ScoreMeter* m_pScoreMeter;
    EngraversMap& m_engravers;
    PartsEngraver* m_pPartsEngraver;
    map<string, LyricEngraver*> m_lyricEngravers;

public:
    ShapesCreator(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                  EngraversMap& engravers, PartsEngraver* pPartsEngraver);
    ~ShapesCreator();


    enum {k_flag_small_clef=0x01, };

    //StaffObj shapes
    GmoShape* create_staffobj_shape(ImoStaffObj* pSO, int iInstr, int iStaff,
                                    UPoint pos, int clefType=0, int octaveShift=0,
                                    unsigned flags=0);
    GmoShape* create_auxobj_shape(ImoAuxObj* pAO, int iInstr, int iStaff,
                                  int idxStaff, VerticalProfile* pVProfile,
                                  GmoShape* pParentShape);
    GmoShape* create_invisible_shape(ImoObj* pSO, int iInstr, int iStaff,
                                     UPoint uPos, LUnits width);

    //RelObj shapes
    void start_engraving_relobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                int iSystem, int iCol, int iLine, ImoInstrument* pInstr,
                                int idxStaff, VerticalProfile* pVProfile);
    void continue_engraving_relobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                   GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                   int iSystem, int iCol, int iLine,
                                   ImoInstrument* pInstr, int idxStaff,
                                   VerticalProfile* pVProfile);
    void finish_engraving_relobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                 GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                 int iSystem, int iCol, int iLine, LUnits prologWidth,
                                 ImoInstrument* pInstr, int idxStaff,
                                 VerticalProfile* pVProfile);
    GmoShape* create_first_or_intermediate_shape(ImoRelObj* pRO);
    GmoShape* create_last_shape(ImoRelObj* pRO);

    //AuxRelObj shapes
    void start_engraving_auxrelobj(ImoAuxRelObj* pARO, ImoStaffObj* pSO, const string& tag,
                                   GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                   int iSystem, int iCol, int iLine, ImoInstrument* pInstr,
                                   int idxStaff, VerticalProfile* pVProfile);
    void continue_engraving_auxrelobj(ImoAuxRelObj* pARO, ImoStaffObj* pSO, const string& tag,
                                   GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                   int iSystem, int iCol, int iLine,
                                   ImoInstrument* pInstr, int idxStaff,
                                   VerticalProfile* pVProfile);
    void finish_engraving_auxrelobj(ImoAuxRelObj* pARO, ImoStaffObj* pSO, const string& tag,
                                    GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                    int iSystem, int iCol, int iLine, LUnits prologWidth,
                                    ImoInstrument* pInstr, int idxStaff,
                                    VerticalProfile* pVProfile);

    //other shapes
    GmoShape* create_measure_number_shape(ImoObj* pCreator, const string& number,
                                          LUnits xPos, LUnits yPos);

protected:

};


//---------------------------------------------------------------------------------------
// LinesBreaker: base class for any lines break algorithm

#define LOMSE_INFINITE_PENALTY      10000000000.0f

class LinesBreaker
{
protected:
    ScoreLayouter* m_pScoreLyt;
    LibraryScope& m_libraryScope;
    SpacingAlgorithm* m_pSpAlgorithm;
    std::vector<int>& m_breaks;

public:
    LinesBreaker(ScoreLayouter* pScoreLyt, LibraryScope& libScope,
                 SpacingAlgorithm* pSpAlgorithm, std::vector<int>& breaks)
        : m_pScoreLyt(pScoreLyt)
        , m_libraryScope(libScope)
        , m_pSpAlgorithm(pSpAlgorithm)
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
    LinesBreakerSimple(ScoreLayouter* pScoreLyt, LibraryScope& libScope,
                       SpacingAlgorithm* pSpAlgorithm, std::vector<int>& breaks);
    virtual ~LinesBreakerSimple() {}

    void decide_line_breaks();
};


//---------------------------------------------------------------------------------------
// Optimal lines break algorithm, based on Knuths' algorithm for text processors
class LinesBreakerOptimal : public LinesBreaker
{
public:
    LinesBreakerOptimal(ScoreLayouter* pScoreLyt, LibraryScope& libScope,
                        SpacingAlgorithm* pSpAlgorithm, std::vector<int>& breaks);
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
    };
    std::vector<Entry> m_entries;
    int m_numCols;
    bool m_fJustifyLastLine;

    void initialize_entries_table();
    void compute_optimal_break_sequence();
    void retrieve_breaks_sequence();

};



}   //namespace lomse

#endif    // __LOMSE_SCORE_LAYOUTER_H__


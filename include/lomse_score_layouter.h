//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SCORE_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_SCORE_LAYOUTER_H__

#include "lomse_layouter.h"
#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_score_enums.h"
#include "lomse_logger.h"
#include "lomse_engraver.h"
#include "lomse_engravers_map.h"
#include "lomse_spacing_algorithm.h"

#include <vector>
using namespace std;

namespace lomse
{

//forward declarations
class ColumnBreaker;
class ColumnsBuilder;
class ColumnStorage;
class FontStorage;
class GmoBoxScorePage;
class GmoBoxSlice;
class GmoBoxSliceInstr;
class GmoBoxSystem;
class GmoShape;
class GmoShapeNote;
class GraphicModel;
class ImoAuxObj;
class ImoAuxRelObj;
class ImoClef;
class ImoContentObj;
class ImoInstrument;
class ImoRelObj;
class ImoScore;
class ImoStaffObj;
class ImoTimeSignature;
class InstrumentEngraver;
class LyricEngraver;
class PartsEngraver;
class ScoreMeter;
class ScoreStub;
class ShapesCreator;
class StaffObjsCursor;
class SystemLayouter;
class SystemLayoutScope;

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

// some helper typedefs
typedef std::pair<ImoRelObj*, AuxObjContext*> PendingRelObj;
typedef std::pair<std::string, AuxObjContext*> PendingLyricsObj;


//=======================================================================================
// Helper class to store global information whose scope is the score layout process.
// It facilitates access to this global information and simplifies the list of parameters
// to pass to all classes and methods related to layout
//
class ScoreLayoutScope
{
protected:
    ScoreLayouter*  m_pScoreLyt = nullptr;
    LibraryScope&   m_libraryScope;
    GraphicModel*   m_pGModel = nullptr;
    ImoScore*       m_pScore = nullptr;
    ScoreMeter*     m_pScoreMeter = nullptr;
    SpacingAlgorithm* m_pSpAlgorithm = nullptr;
    EngraversMap*   m_pEngraversMap = nullptr;
    ShapesCreator*  m_pShapesCreator = nullptr;
    PartsEngraver*  m_pPartsEngraver = nullptr;
//    ScoreLayoutOptions* m_pOptions = nullptr;

public:
    explicit ScoreLayoutScope(ScoreLayouter* pParent, LibraryScope& libraryScope,
                              GraphicModel* pGModel);
    ~ScoreLayoutScope();


    inline ScoreLayouter* get_score_layouter() { return m_pScoreLyt; }
    inline LibraryScope& get_library_scope() { return m_libraryScope; }
    inline GraphicModel* get_graphic_model() { return m_pGModel; }
    inline ImoScore* get_score() { return m_pScore; }
    inline ScoreMeter* get_score_meter() { return m_pScoreMeter; }
    inline SpacingAlgorithm* get_spacing_algorithm() { return m_pSpAlgorithm; }
    inline EngraversMap& get_engravers_map() { return *m_pEngraversMap; }
    inline ShapesCreator* get_shapes_creator() { return m_pShapesCreator; }
    inline PartsEngraver* get_parts_engraver() { return m_pPartsEngraver; }


protected:
    //instantiation
    friend class ScoreLayouter;
    void initialice(ImoScore* pScore, EngraversMap* pEngraversMap);

};

//=======================================================================================
// Algorithm to layout an score
//
class ScoreLayouter : public Layouter
{
protected:
    ScoreLayoutScope  m_scoreLayoutScope;
    ImoScore*         m_pScore;

    //variables stored in ScoreLayoutScope
    ScoreMeter*       m_pScoreMeter = nullptr;
    SpacingAlgorithm* m_pSpAlgorithm = nullptr;
    EngraversMap      m_engravers;
    ShapesCreator*    m_pShapesCreator = nullptr;
    PartsEngraver*    m_pPartsEngraver = nullptr;

        //member variables

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
    GmoBoxSystem*       m_pPrevBoxSystem = nullptr;

    //support for debug and unit test
    int                 m_iColumnToTrace;
    int                 m_nTraceLevel;

public:
    ScoreLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                  LibraryScope& libraryScope);
    virtual ~ScoreLayouter();

    void prepare_to_start_layout() override;
    void layout_in_box() override;
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height) override;

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
    void dump_column_data(int iCol, ostream& outStream=glogger.get_stream());
    void delete_not_used_objects();
    void delete_pendig_aux_objects();
    void delete_system_boxes();
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
    void final_touches();
    bool score_page_is_the_only_content_of_parent_box();

    void fill_page_with_empty_systems_if_required();
    void remove_unused_space();
    void center_score_if_requested();
    void delete_system_layouters();
    void get_score_renderization_options();
    void auto_scale();

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

    //AuxObjs and RelObjs pending to be engraved
    std::list<AuxObjContext*> m_pendingAuxObjs;

    //RelObjs that continue in next system
    std::list<PendingRelObj> m_notFinishedRelObj;

    //Lyrics that continue in next system
    std::list<PendingLyricsObj> m_notFinishedLyrics;


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

    bool feasible_break_before_this_obj(ImoStaffObj* pSO, ImoStaffObj* pPrevSO,
                                        TimeUnits rTime, int iInstr, int iLine);

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
                                    UPoint pos, ImoClef* pClef=nullptr, int octaveShift=0,
                                    unsigned flags=0, StaffObjsCursor* pCursor=nullptr);
    GmoShape* create_auxobj_shape(ImoAuxObj* pAO, const AuxObjContext& aoc,
                                  const SystemLayoutScope& systemScope);
    GmoShape* create_invisible_shape(ImoObj* pSO, int iInstr, int iStaff,
                                     UPoint uPos, LUnits width);

    //RelObj shapes
    void start_engraving_relobj(ImoRelObj* pRO, const AuxObjContext& aoc);
    void continue_engraving_relobj(ImoRelObj* pRO, const AuxObjContext& aoc);
    void finish_engraving_relobj(ImoRelObj* pRO, const AuxObjContext& aoc);

    GmoShape* create_first_or_intermediate_shape(ImoRelObj* pRO, RelObjEngravingContext& ctx);
    GmoShape* create_last_shape(ImoRelObj* pRO, RelObjEngravingContext& ctx);

    //AuxRelObj shapes
    void start_engraving_auxrelobj(ImoAuxRelObj* pARO, const AuxObjContext& aoc, const string& tag);
    void continue_engraving_auxrelobj(ImoAuxRelObj* pARO, const AuxObjContext& aoc, const string& tag);
    void finish_engraving_auxrelobj(ImoAuxRelObj* pARO, const AuxObjContext& aoc, const string& tag);

    //other shapes
    GmoShape* create_measure_number_shape(ImoObj* pCreator, const string& number,
                                          ShapeId idx, LUnits xPos, LUnits yPos);

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

    void decide_line_breaks() override;
};


//---------------------------------------------------------------------------------------
// Optimal lines break algorithm, based on Knuths' algorithm for text processors
class LinesBreakerOptimal : public LinesBreaker
{
public:
    LinesBreakerOptimal(ScoreLayouter* pScoreLyt, LibraryScope& libScope,
                        SpacingAlgorithm* pSpAlgorithm, std::vector<int>& breaks);
    virtual ~LinesBreakerOptimal() {}

    void decide_line_breaks() override;

    //support for debug and tests
    void dump_entries(ostream& outStream=glogger.get_stream());

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


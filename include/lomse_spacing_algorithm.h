//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SPACING_ALGORITHM_H__        //to avoid nested includes
#define __LOMSE_SPACING_ALGORITHM_H__

#include "lomse_basic.h"
#include "lomse_logger.h"

//std
#include <list>

namespace lomse
{

//forward declarations
class ColStaffObjsEntry;
class ColumnBreaker;
class ColumnData;
class ColumnsBuilder;
class GmoBoxSlice;
class GmoBoxSliceInstr;
class GmoShape;
class GmoShapeBarline;
class ImoInstrument;
class ImoScore;
class ImoStaffObj;
class LibraryScope;
class PartsEngraver;
class ScoreLayouter;
class ScoreMeter;
class ShapesCreator;
class EngraversMap;
class SpAlgColumn;
class StaffObjsCursor;
class TimeGridTable;
class TypeMeasureInfo;
class GmoBoxSystem;
class VerticalProfile;

//---------------------------------------------------------------------------------------
// Barlines at the end of a column
enum EColumnBarlinesInfo
{
    k_all_instr_have_barline            = 0x01,
    k_some_instr_have_barline           = 0x02,
    k_all_instr_have_final_barline      = 0x04,
    k_all_instr_have_barline_TS_or_KS   = 0x08,
    k_some_instr_have_barline_TS_or_KS  = 0x10,
};


//---------------------------------------------------------------------------------------
/** %SpacingAlgorithm
    Abstract class providing the public interface for any spacing algorithm.
    The idea is to facilitate testing different algorithms without having to
    rewrite other parts of the code.
*/
class SpacingAlgorithm
{
protected:
    LibraryScope&   m_libraryScope;
    ScoreMeter*     m_pScoreMeter;
    ScoreLayouter*  m_pScoreLyt;
    ImoScore*       m_pScore;
    EngraversMap&  m_engravers;
    ShapesCreator*  m_pShapesCreator;
    PartsEngraver*  m_pPartsEngraver;
    VerticalProfile* m_pVProfile;

public:
    SpacingAlgorithm(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                     ScoreLayouter* pScoreLyt, ImoScore* pScore,
                     EngraversMap& engravers, ShapesCreator* pShapesCreator,
                     PartsEngraver* pPartsEngraver);
    virtual ~SpacingAlgorithm();

    //spacing algorithm ---------

    ///This is the first method to be invoked. Your implementation must:
    ///
    ///- collect score content and organize it as necessary for the algorithm.
    ///
    ///- split the content into columns (e.g. measures). A column must end in a point
    ///  were it must be possible to break the lines. Splitting the content in measures
    ///  is the most simple approach, but smaller chunks could be possible (and
    ///  to deal with scores without barlines, with multimetrics scores and with
    ///  long measures.
    ///
    virtual void split_content_in_columns() = 0;

    ///Next, this method will be invoked. Your implementation must apply the
    /// spacing algorithm for determining the minimum size for each column.
    virtual void do_spacing_algorithm() = 0;

    ///Methods for line break will then be invoked
    virtual float determine_penalty_for_line(int iSystem, int i, int j) = 0;
    virtual bool is_better_option(float prevPenalty, float newPenalty, float nextPenalty,
                                  int i, int j) = 0;

    ///Finally, if justification is required this method will be invoked
    virtual void justify_system(int iFirstCol, int iLastCol, LUnits uSpaceIncrement) = 0;


    //provide information -----------

    ///Return the number of columns in which the content has been split
    virtual int get_num_columns() = 0;

    virtual LUnits get_staves_height() = 0;

    //information about a column
    virtual bool is_empty_column(int iCol) = 0;
    virtual LUnits get_column_width(int iCol) = 0;
    virtual bool has_system_break(int iCol) = 0;
    virtual int get_column_barlines_information(int iCol) = 0;
    virtual TypeMeasureInfo* get_measure_info_for_column(int iCol) = 0;
    virtual GmoShapeBarline* get_start_barline_shape_for_column(int iCol) = 0;

    //boxes and shapes management
    virtual void reposition_slices_and_staffobjs(int iFirstCol, int iLastCol,
                                        LUnits yShift, LUnits* yMin, LUnits* yMax) = 0;
    virtual void reposition_full_measure_rests(int iFirstCol, int iLastCol,
                                               GmoBoxSystem* pBox) = 0;
    virtual void add_shapes_to_boxes(int iCol, VerticalProfile* pVProfile) = 0;
    virtual void delete_shapes(int iCol) = 0;
    virtual GmoBoxSliceInstr* get_slice_instr(int iCol, int iInstr) = 0;
    virtual void set_slice_final_position(int iCol, LUnits left, LUnits top) = 0;
    virtual void create_boxes_for_column(int iCol, LUnits left, LUnits top)=0;
    virtual void delete_box_and_shapes(int iCol) = 0;
    ///store slice box for column iCol and access it
    virtual void use_this_slice_box(int iCol, GmoBoxSlice* pBoxSlice) = 0;
    virtual GmoBoxSlice* get_slice_box(int iCol) = 0;

    //methods to compute results
    virtual TimeGridTable* create_time_grid_table_for_column(int iCol) = 0;

    //access to info
    virtual ColStaffObjsEntry* get_prolog_clef(int iCol, ShapeId idx) = 0;
    virtual ColStaffObjsEntry* get_prolog_key(int iCol, ShapeId idx) = 0;
    virtual ColStaffObjsEntry* get_prolog_time(int iCol, ShapeId idx) = 0;

    //debug and support for unit tests
    virtual void dump_column_data(int iCol, ostream& outStream) = 0;
    virtual void set_trace_level(int iCol, int nTraceLevel) = 0;
    virtual ColumnData* get_column(int i) = 0;

};


//---------------------------------------------------------------------------------------
//ColumnData: helper class used by SpAlgColumn for storing data for columns
class ColumnData
{
protected:
    ScoreMeter* m_pScoreMeter;
    bool m_fHasSystemBreak;
    GmoBoxSlice* m_pBoxSlice;           //box for this column
    SpAlgColumn* m_pSpAlgorithm;
    bool m_fMeasureStart;               //a measure starts in this column
    TypeMeasureInfo* m_pMeasureInfo;    //info for measure that starts or nullptr

    GmoShapeBarline* m_pShapeBarline;   //shape for barline at which the measure starts
                                        //(the previous barline) or nullptr if measure
                                        //does not starts in this column or it is the
                                        //first measure.

    int m_nTraceLevel;                  //for debugging

    std::vector<GmoBoxSliceInstr*> m_sliceInstrBoxes;   //instr.boxes for this column

    //applicable prolog at start of this column. On entry per staff
    std::vector<ColStaffObjsEntry*> m_prologClefs;
    std::vector<ColStaffObjsEntry*> m_prologKeys;
    std::vector<ColStaffObjsEntry*> m_prologTimes;


public:
    ColumnData(ScoreMeter* pScoreMeter, SpAlgColumn* pSpAlgorithm);
    virtual ~ColumnData();


    inline void use_this_slice_box(GmoBoxSlice* pBoxSlice) { m_pBoxSlice = pBoxSlice; };
    inline GmoBoxSlice* get_slice_box() { return m_pBoxSlice; };
    void save_context(int iInstr, int iStaff, ColStaffObjsEntry* pClefEntry,
                      ColStaffObjsEntry* pKeyEntry, ColStaffObjsEntry* pTimeEntry);

    //access to info
    inline bool has_system_break() { return m_fHasSystemBreak; }
    inline void set_system_break(bool value) { m_fHasSystemBreak = value; }
    inline ColStaffObjsEntry* get_prolog_clef(ShapeId idx) { return m_prologClefs[idx]; }
    inline ColStaffObjsEntry* get_prolog_key(ShapeId idx) { return m_prologKeys[idx]; }
    inline ColStaffObjsEntry* get_prolog_time(ShapeId idx) { return m_prologTimes[idx]; }

    //boxes and shapes
    void add_shapes_to_boxes(int iCol);
    GmoBoxSliceInstr* create_slice_instr(ImoInstrument* pInstr, int idxStaff, LUnits yTop);
    inline GmoBoxSliceInstr* get_slice_instr(int iInstr) { return m_sliceInstrBoxes[iInstr]; }
    void set_slice_width(LUnits width);
    void set_slice_final_position(LUnits left, LUnits top);

    //adding shapes to graphical model
    void add_shapes(GmoBoxSliceInstr* pSliceInstrBox, int iInstr);
    void delete_shapes(int iCol);

    //support to lay out measure attributes
    inline void mark_as_start_of_measure(TypeMeasureInfo* pInfo, GmoShapeBarline* pShape)
    {
        m_pMeasureInfo = pInfo;
        m_fMeasureStart = true;
        m_pShapeBarline = pShape;
    }
    inline bool is_start_of_measure() { return m_fMeasureStart; }
    inline TypeMeasureInfo* get_measure_info() { return m_pMeasureInfo; }
    inline GmoShapeBarline* get_shape_for_start_barline() { return m_pShapeBarline; }

    //support for debug and unit tests
    void delete_box_and_shapes(int iCol);
    inline void set_trace_level(int level) { m_nTraceLevel = level; }

protected:
    void reserve_space_for_prolog_clefs_keys(int numStaves);


};


//---------------------------------------------------------------------------------------
// SpAlgColumn
// Abstract class for spacing algorithms based on using ColumnBuilder object for
// organizing the content in columns and managing the columns information.
//
class SpAlgColumn: public SpacingAlgorithm
{
protected:
    ColumnsBuilder* m_pColsBuilder;
    std::vector<ColumnData*> m_colsData;


public:
    SpAlgColumn(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                ScoreLayouter* pScoreLyt, ImoScore* pScore,
                EngraversMap& engravers, ShapesCreator* pShapesCreator,
                PartsEngraver* pPartsEngraver);
    virtual ~SpAlgColumn();


    //overrides of base class SpacingAlgorithm. Normally, not need to override them
    //------------------------------------------------------------------------------

    //collect content
    void split_content_in_columns() override;
    //spacing algorithm
    void do_spacing_algorithm() override;
    //boxes and shapes
    void add_shapes_to_boxes(int iCol, VerticalProfile* pVProfile) override;
    GmoBoxSliceInstr* get_slice_instr(int iCol, int iInstr) override;
    void set_slice_final_position(int iCol, LUnits left, LUnits top) override;
    void create_boxes_for_column(int iCol, LUnits left, LUnits top) override;
    LUnits get_staves_height() override;
    ///store slice box for column iCol and access it
    void use_this_slice_box(int iCol, GmoBoxSlice* pBoxSlice) override;
    GmoBoxSlice* get_slice_box(int iCol) override;
    bool has_system_break(int iCol) override;
    void delete_box_and_shapes(int iCol) override;
    //other
    TypeMeasureInfo* get_measure_info_for_column(int iCol) override;
    GmoShapeBarline* get_start_barline_shape_for_column(int iCol) override;


    //methods in base class SpacingAlgorithm that still need to be created
    //------------------------------------------------------------------------

//    virtual void reposition_slices_and_staffobjs(int iFirstCol, int iLastCol,
//            LUnits yShift,
//            LUnits* yMin, LUnits* yMax) = 0;
//    virtual void justify_system(int iFirstCol, int iLastCol, LUnits uSpaceIncrement) = 0;
//
//    //for line break algorithm
//    virtual bool is_empty_column(int iCol) = 0;
//
//    //information about a column
//    virtual LUnits get_column_width(int iCol) = 0;
//    virtual int get_column_barlines_information(int iCol) = 0;
//
//    //methods to compute results
//    virtual TimeGridTable* create_time_grid_table_for_column(int iCol) = 0;
//
//    //methods for line break
//    virtual float determine_penalty_for_line(int iSystem, int i, int j) = 0;
//    virtual bool is_better_option(float prevPenalty, float newPenalty, float nextPenalty,
//                                  int i, int j) = 0;
//
//    //debug
//    virtual void dump_column_data(int iCol, ostream& outStream) = 0;
    ColumnData* get_column(int i) override;


    //new methods to be implemented by derived classes (apart from previous methods)
    //---------------------------------------------------------------------------------

    //column creation: collecting content
    ///prepare for receiving information for a new column
    virtual void start_column_measurements(int iCol) = 0;
    ///save information for staff object in current column
    virtual void include_object(ColStaffObjsEntry* pCurEntry, int iCol, int iInstr,
                                int  iStaff, ImoStaffObj* pSO, GmoShape* pShape,
                                bool fInProlog=false) = 0;
    ///save info for a full-measure rest
    virtual void include_full_measure_rest(GmoShape* pRestShape, ColStaffObjsEntry* pCurEntry,
                                           GmoShape* pNonTimedShape) = 0;
    ///terminate current column
    virtual void finish_column_measurements(int iCol) = 0;

    //spacing algorithm main actions
    ///apply spacing algorithm to all columns
    virtual void do_spacing(int iColumnToTrace) = 0;

    //auxiliary: shapes and boxes
    ///add shapes for staff objects to graphical model
    virtual void add_shapes_to_box(int iCol, GmoBoxSliceInstr* pSliceInstrBox,
                                   int iInstr) = 0;

//    virtual void delete_shapes(int iCol) = 0;


    //new methods for this class, normally no need to override
    //-------------------------------------------------------------------------

    ///Returns the number of columns in which the content has been split
    int get_num_columns() override;

    ///save context information (clef, key, time) for iCol, and access it
    virtual void save_context(int iCol, int iInstr, int iStaff,
                              ColStaffObjsEntry* pClefEntry,
                              ColStaffObjsEntry* pKeyEntry,
                              ColStaffObjsEntry* pTimeEntry);
    ColStaffObjsEntry* get_prolog_clef(int iCol, ShapeId idx) override;
    ColStaffObjsEntry* get_prolog_key(int iCol, ShapeId idx) override;
    ColStaffObjsEntry* get_prolog_time(int iCol, ShapeId idx) override;

    ///system break found while collecting content for iCol
    virtual void set_system_break(int iCol, bool value);

    ///a new column is going to be created (do whatever your spacing algorithm requires:
    ///allocating memory for column data, etc.)
    virtual void prepare_for_new_column(int UNUSED(iCol)) {}

    ///create slice instr box for column iCol and access it
    virtual GmoBoxSliceInstr* create_slice_instr(int iCol, ImoInstrument* pInstr,
                                                 int idxStaff, LUnits yTop);
    ///set width of slice box for column iCol
    virtual void set_slice_width(int iCol, LUnits width);

    ///activate trace for iCol at level traceLevel
    void set_trace_level(int iCol, int nTraceLevel) override;


};


//---------------------------------------------------------------------------------------
// ColumnsBuilder: algorithm to build the columns for one score
class ColumnsBuilder
{
protected:
    ScoreMeter*     m_pScoreMeter;
    ScoreLayouter*  m_pScoreLyt;
    ImoScore*       m_pScore;
    EngraversMap&  m_engravers;
    ShapesCreator*  m_pShapesCreator;
    PartsEngraver*  m_pPartsEngraver;
    StaffObjsCursor* m_pSysCursor;  //cursor for traversing the score
    ColumnBreaker*  m_pBreaker;
    std::vector<LUnits> m_SliceInstrHeights;
    LUnits m_stavesHeight;      //system height without top and bottom margins

    int m_iColumn;   //[0..n-1] current column. (-1 if no column yet created!)
    int m_iColStartMeasure;     //to store index at which next measure starts
    GmoShapeBarline* m_pStartBarlineShape;  //to store ptr to the barline shape for the
                                            //the measure that finishes current measure.
                                            //It is going to be the barline that starts
                                            //next measure (in column m_iColStartMeasure)

    int m_iColumnToTrace;   //support for debug and unit test
    int m_nTraceLevel;

    SpAlgColumn* m_pSpAlgorithm;
    int m_maxColumn;
    std::vector<ColumnData*>& m_colsData;

public:
    ColumnsBuilder(ScoreMeter* pScoreMeter, std::vector<ColumnData*>& colsData,
                   ScoreLayouter* pScoreLyt, ImoScore* pScore,
                   EngraversMap& engravers,
                   ShapesCreator* pShapesCreator,
                   PartsEngraver* pPartsEngraver,
                   SpAlgColumn* pSpAlgorithm);
    ~ColumnsBuilder();


    void create_columns();
    void do_spacing_algorithm();
    inline LUnits get_staves_height()
    {
        return m_stavesHeight;
    }

    //support for debugging and unit tests
    inline void set_debug_options(int iCol, int level)
    {
        m_iColumnToTrace = iCol;
        m_nTraceLevel = level;
    }

    //managing shapes
    void add_shapes_to_boxes(int iCol);
    void delete_shapes(int iCol);
    void create_boxes_for_column(int iCol, LUnits xLeft, LUnits yTop);

protected:
    void determine_staves_vertical_position();

    void prepare_for_new_column();
    void collect_content_for_this_column();

    GmoBoxSlice* create_slice_box();
    void find_and_save_context_info_for_this_column();

    void store_info_about_attached_objects(ImoStaffObj* pSO, GmoShape* pShape,
                                           int iInstr, int iStaff, int iCol, int iLine,
                                           ImoInstrument* pInstr, int idxStaff);

    bool determine_if_is_in_prolog(ImoStaffObj* pSO, TimeUnits rTime, int iInstr,
                                   int idx);
    std::vector<bool> m_fClefFound;     //for each instrument
    std::vector<bool> m_fSignatures;    //key or time signature found, for each instrument
    std::vector<bool> m_fOther;         //other objects found, for each instrument

    inline bool is_first_column()
    {
        return m_iColumn == 0;
    }

};


}   //namespace lomse

#endif    // __LOMSE_SPACING_ALGORITHM_H__


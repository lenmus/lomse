//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SYSTEM_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_SYSTEM_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_time.h"
#include "lomse_score_enums.h"
#include "lomse_logger.h"
#include "lomse_injectors.h"
#include "lomse_spacing_algorithm.h"
#include "lomse_aux_shapes_aligner.h"

#include <list>
#include <memory>
#include <tuple>

namespace lomse
{

//forward declarations
class ColumnStorage;
class EngraversMap;
class GmoBoxSlice;
class GmoBoxSliceInstr;
class GmoShape;
class GmoBoxSystem;
class GmoShapeBeam;
class GmoShapeNote;
class ImoAuxObj;
class ImoRelObj;
class ImoAuxRelObj;
class ImoInstrument;
class ImoScore;
class ImoStaff;
class ImoStaffObj;
class InstrumentEngraver;
class PartsEngraver;
class ScoreLayouter;
class ScoreLayoutScope;
class ScoreMeter;
class ShapesCreator;
class SpacingAlgorithm;
class SystemLayouter;
class VerticalProfile;
struct TypeMeasureInfo;

struct AuxObjContext;
enum EAuxShapesAlignmentScope : int;


//=======================================================================================
// Helper class to store global information whose scope is the layout of one system.
// It facilitates access to this global information and simplifies the list of parameters
// to pass to all classes and methods related to layout
//
class SystemLayoutScope
{
protected:
    SystemLayouter*     m_pSystemLayouter = nullptr;
    VerticalProfile*    m_pVProfile = nullptr;
    AuxShapesAlignersSystem* m_pCurrentAuxShapesAligner = nullptr;
//    SystemLayoutOptions* m_pOptions = nullptr;

public:
    explicit SystemLayoutScope(SystemLayouter* pParent);

    inline SystemLayouter* get_system_layouter() const { return m_pSystemLayouter; }
    inline VerticalProfile* get_vertical_profile() const { return m_pVProfile; }
    inline AuxShapesAlignersSystem* get_aux_shapes_aligner() const { return m_pCurrentAuxShapesAligner; }


protected:
    //instantiation
    friend class SystemLayouter;
    void set_current_aux_shapes_aligner(AuxShapesAlignersSystem* p) { m_pCurrentAuxShapesAligner = p; }
    void set_vertical_profile(VerticalProfile* p) { m_pVProfile = p; }

};


//=======================================================================================
// SystemLayouter: algorithm to layout a system
//
class SystemLayouter
{
protected:
    ScoreLayoutScope& m_scoreLayoutScope;
    SystemLayoutScope m_systemLayoutScope;

    //variables stored in ScoreLayoutScope
    ScoreLayouter*  m_pScoreLyt;
    LibraryScope&   m_libraryScope;
    ScoreMeter*     m_pScoreMeter;
    ImoScore*       m_pScore;
    EngraversMap&   m_engravers;
    ShapesCreator*  m_pShapesCreator;
    PartsEngraver*  m_pPartsEngraver;
    SpacingAlgorithm* m_pSpAlgorithm;

    //variables used in SystemLayoutScope but owned by this SystemLayouter
    std::unique_ptr<VerticalProfile> m_pVProfile;
    std::unique_ptr<AuxShapesAlignersSystem> m_curAuxShapesAligner;


    GmoBoxSystem* m_pBoxSystem = nullptr;

    LUnits m_uPrologWidth = 0.0f;
    LUnits m_yMin = 0.0f;
    LUnits m_yMax = 0.0f;
    LUnits m_uFreeSpace = 0.0f;    //free space available on current system

    int m_iSystem = 0;
    int m_iFirstCol = 0;
    int m_iLastCol = 0;
    int m_barlinesInfo = 0;     //info about barlines at end of this system
    int m_constrains = 0;
    UPoint m_pagePos;
    bool m_fFirstColumnInSystem = true;

    //prolog shapes waiting to be added to slice staff box
    std::list< std::tuple<GmoShape*, int, int> > m_prologShapes;


public:
    explicit SystemLayouter(ScoreLayoutScope& scoreLayoutScope);

    GmoBoxSystem* create_system_box(LUnits left, LUnits top, LUnits width, LUnits height);
    void engrave_system(LUnits indent, int iFirstCol, int iLastCol, UPoint pos,
                        GmoBoxSystem* pPrevBoxSystem);
    void on_origin_shift(LUnits yShift);
    inline void set_constrains(int constrains) { m_constrains = constrains; }

        //Access to information
    inline void set_prolog_width(LUnits width) { m_uPrologWidth = width; }
    inline LUnits get_prolog_width() { return m_uPrologWidth; }
    inline GmoBoxSystem* get_box_system() { return m_pBoxSystem; }
    inline LUnits get_y_min() { return m_yMin; }
    inline LUnits get_y_max() { return m_yMax; }
    inline bool all_instr_have_barline() {
        return m_barlinesInfo & k_all_instr_have_barline;
    }
    inline bool some_instr_have_barline() {
        return (m_barlinesInfo & k_some_instr_have_barline) != 0;
    }
    inline bool all_instr_have_final_barline() {
        return (m_barlinesInfo & k_all_instr_have_final_barline) != 0;
    }
    bool system_must_be_truncated();

protected:
    void set_position_and_width_for_staves(LUnits indent);
    void create_vertical_profile();
    void fill_current_system_with_columns();
    void collect_last_column_information();
    void justify_current_system();
    void build_system_timegrid();
    void reposition_full_measure_rests();
    void engrave_instrument_details();
    void truncate_current_system(LUnits indent);
    void add_column_to_system(int iCol);
    void add_shapes_for_column(int iCol);
    bool system_must_be_justified();
    void add_initial_line_joining_all_staves_in_system();
    void reposition_slices_and_staffobjs();
    void redistribute_free_space();
    void engrave_measure_numbers();
    void engrave_system_details(int iSystem);
    void setup_aux_shapes_aligner(EAuxShapesAlignmentScope scope, Tenths maxAlignDistance = 0.0f);
    void add_instruments_info();
    void move_staves_to_avoid_collisions(GmoBoxSystem* pPrevBoxSystem);
    void reposition_staves_in_engravers(const std::vector<LUnits>& yOrgShifts);
    void reposition_slice_boxes_and_shapes(const vector<LUnits>& yOrgShifts,
                                           vector<LUnits>& heights,
                                           LUnits bottomMarginIncr);

    void add_prolog_shapes_to_boxes();
    void add_system_prolog_if_necessary();
    LUnits engrave_prolog(int iInstr);
    LUnits determine_column_start_position(int iCol);
    LUnits determine_column_size(int iCol);
    void create_boxes_for_column(int iCol, LUnits pos, LUnits size);
    bool measure_number_must_be_displayed(int policy, TypeMeasureInfo* pInfo,
                                          bool fFirstNumberInSystem);

    void engrave_attached_object(ImoObj* pAR, const AuxObjContext& aoc, int iSystem);
    void engrave_not_finished_relobj(ImoRelObj* pRO, const AuxObjContext& aoc);
    void engrave_not_finished_lyrics(const std::string& tag, const AuxObjContext& aoc);

    void add_last_rel_shape_to_model(GmoShape* pShape, ImoRelObj* pRO, int layer,
                                     int iCol, int iInstr, int iStaff, int idxStaff);
    void delete_rel_obj_engraver(ImoRelObj* pRO);
    void add_lyrics_shapes_to_model(const std::string& tag, int layer, bool fLast,
                                    int iInstr, int iStaff);
    void add_aux_shape_to_model(GmoShape* pShape, int layer, int iCol, int iInstr,
                                int iStaff, int idxStaff);

    //helpers
    inline bool is_first_column_in_system() { return m_fFirstColumnInSystem; }

    //debug
    void dbg_add_vertical_profile_shape();

    //final layout
    friend class GmoBoxSliceStaff;
    void increment_cross_staff_stems(GmoShapeBeam* pShapeBeam, LUnits yIncrement);

};



}   //namespace lomse

#endif    // __LOMSE_SYSTEM_LAYOUTER_H__


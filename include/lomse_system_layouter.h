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

#ifndef __LOMSE_SYSTEM_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_SYSTEM_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_time.h"
#include "lomse_score_enums.h"
#include "lomse_logger.h"
#include "lomse_injectors.h"
#include "lomse_spacing_algorithm.h"

#include <list>
#include <tuple>

namespace lomse
{

//forward declarations
class ColumnStorage;
class GmoBoxSlice;
class GmoBoxSliceInstr;
class GmoShape;
class GmoBoxSystem;
class GmoShapeBeam;
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
class ScoreMeter;
class ShapesCreator;
class EngraversMap;
class SpacingAlgorithm;
class SystemLayouter;
class TypeMeasureInfo;
class VerticalProfile;
struct PendingAuxObjs;

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
    EngraversMap&   m_engravers;
    ShapesCreator*  m_pShapesCreator;
    PartsEngraver*  m_pPartsEngraver;
    VerticalProfile* m_pVProfile;

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
    int m_barlinesInfo;     //info about barlines at end of this system

    //RelObjs that continue in next system
    typedef std::pair<ImoRelObj*, PendingAuxObjs*> PendingRelObj;
    std::list<PendingRelObj> m_notFinishedRelObj;

    //Lyrics that continue in next system
    typedef std::pair<std::string, PendingAuxObjs*> PendingLyrics;
    std::list<PendingLyrics> m_notFinishedLyrics;

    //prolog shapes waiting to be added to slice staff box
    std::list< std::tuple<GmoShape*, int, int> > m_prologShapes;

    SpacingAlgorithm* m_pSpAlgorithm;
    int m_constrains;

public:
    SystemLayouter(ScoreLayouter* pScoreLyt, LibraryScope& libraryScope,
                   ScoreMeter* pScoreMeter, ImoScore* pScore,
                   EngraversMap& engravers,
                   ShapesCreator* pShapesCreator,
                   PartsEngraver* pPartsEngraver,
                   SpacingAlgorithm* pSpAlgorithm);
    ~SystemLayouter();

    GmoBoxSystem* create_system_box(LUnits left, LUnits top, LUnits width, LUnits height);
    void engrave_system(LUnits indent, int iFirstCol, int iLastCol, UPoint pos);
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
    void add_instruments_info();
    void move_staves_to_avoid_collisions();
    void reposition_staves_in_engravers(const std::vector<LUnits>& yOrgShifts);
    void reposition_slice_boxes_and_shapes(const vector<LUnits>& yOrgShifts,
                                           vector<LUnits>& heights);

    void add_prolog_shapes_to_boxes();
    void add_system_prolog_if_necessary();
    LUnits engrave_prolog(int iInstr);
    LUnits determine_column_start_position(int iCol);
    LUnits determine_column_size(int iCol);
    void create_boxes_for_column(int iCol, LUnits pos, LUnits size);
    bool measure_number_must_be_displayed(int policy, TypeMeasureInfo* pInfo,
                                          bool fFirstNumberInSystem);

    void engrave_attached_object(ImoObj* pAR, PendingAuxObjs* pPAO, int iSystem);
    void engrave_not_finished_relobj(ImoRelObj* pRO, PendingAuxObjs* pPAO, int iSystem);
    void engrave_not_finished_lyrics(const std::string& tag, PendingAuxObjs* pPAO, int iSystem);

    void add_last_rel_shape_to_model(GmoShape* pShape, ImoRelObj* pRO, int layer,
                                     int iCol, int iInstr, int iStaff, int idxStaff);
    void add_lyrics_shapes_to_model(const std::string& tag, int layer, bool fLast,
                                    int iStaff, int idxStaff);
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


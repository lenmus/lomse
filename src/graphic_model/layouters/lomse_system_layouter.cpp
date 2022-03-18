//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_system_layouter.h"

#include "lomse_box_system.h"
#include "lomse_box_slice.h"
#include "lomse_box_slice_instr.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_engravers_map.h"
#include "lomse_logger.h"
#include "lomse_shape_barline.h"
#include "lomse_score_layouter.h"
#include "lomse_staffobjs_table.h"
#include "lomse_engraving_options.h"
#include "lomse_barline_engraver.h"
#include "lomse_instrument_engraver.h"
#include "lomse_spacing_algorithm.h"
#include "lomse_timegrid_table.h"
#include "lomse_vertical_profile.h"
#include "lomse_tree.h"
#include "lomse_shape_beam.h"
#include "lomse_beam_engraver.h"
#include "lomse_chord_engraver.h"
#include "lomse_aux_shapes_aligner.h"
#include "lomse_lyric_engraver.h"

#include <sstream>
#include <algorithm>
#include <bitset>
#include <math.h>
using namespace std;


namespace lomse
{

#define LOMSE_NO_POSITION   100000000000000.0f    //any impossible high value


//=======================================================================================
// Some variables to control engraving ordering for AuxObjs/RelObjs
//=======================================================================================

typedef std::pair<ImoObj*, AuxObjContext*> PendingPair;

enum EAuxShapesAlignmentScope : int
{
    k_alignment_scope_none,
    k_alignment_scope_begin,
    k_alignment_scope_continue,
};

struct EngravingOrderInfo
{
    int type;
    EAuxShapesAlignmentScope alignmentScope;
    Tenths maxAlignDistance;

    EngravingOrderInfo(int type,
                       EAuxShapesAlignmentScope alignmentScope = k_alignment_scope_none,
                       Tenths maxAlignDistance = 0.0f)
        : type(type)
        , alignmentScope(alignmentScope)
        , maxAlignDistance(maxAlignDistance)
    {
    }
};

static const std::initializer_list<EngravingOrderInfo> m_order
{
    { k_imo_beam },
    { k_imo_articulation_symbol },

    { k_imo_tuplet },
    { k_imo_slur },
    { k_imo_tie },

    { k_imo_fermata },
    { k_imo_metronome_mark },
    { k_imo_ornament },
    { k_imo_symbol_repetition_mark },
    { k_imo_technical },
    { k_imo_fingering },
    { k_imo_articulation_line },
    { k_imo_text_repetition_mark },
    { k_imo_octave_shift },
    { k_imo_volta_bracket,          k_alignment_scope_begin,    LOMSE_INFINITE_LENGTH   },
    { k_imo_dynamics_mark,          k_alignment_scope_begin,    50.0f                   },
    { k_imo_wedge,                  k_alignment_scope_continue                          },
    { k_imo_score_text },
    { k_imo_lyric },
    { k_imo_score_title },
    { k_imo_line },
    { k_imo_score_line },
    { k_imo_text_box },
    { k_imo_pedal_mark,             k_alignment_scope_begin,    LOMSE_INFINITE_LENGTH   },
    { k_imo_pedal_line,             k_alignment_scope_continue                          },
};


//=======================================================================================
// SystemLayoutScope implementation
//=======================================================================================
SystemLayoutScope::SystemLayoutScope(SystemLayouter* pParent)
    : m_pSystemLayouter(pParent)
{
}


//=======================================================================================
// SystemLayouter implementation
//=======================================================================================
SystemLayouter::SystemLayouter(ScoreLayoutScope& scoreLayoutScope)
    : m_scoreLayoutScope(scoreLayoutScope)
    , m_systemLayoutScope(this)
    , m_pScoreLyt( scoreLayoutScope.get_score_layouter() )
    , m_libraryScope( scoreLayoutScope.get_library_scope() )
    , m_pScoreMeter( scoreLayoutScope.get_score_meter() )
    , m_pScore( scoreLayoutScope.get_score() )
    , m_engravers( scoreLayoutScope.get_engravers_map() )
    , m_pShapesCreator( scoreLayoutScope.get_shapes_creator() )
    , m_pPartsEngraver( scoreLayoutScope.get_parts_engraver() )
    , m_pSpAlgorithm( scoreLayoutScope.get_spacing_algorithm() )
{
}

//---------------------------------------------------------------------------------------
GmoBoxSystem* SystemLayouter::create_system_box(LUnits left, LUnits top, LUnits width,
                                                LUnits height)
{
    m_pBoxSystem = LOMSE_NEW GmoBoxSystem(m_pScore);
    m_pBoxSystem->set_origin(left, top);

    LUnits leftMargin = 0.0f; //TODO-LOG: m_pScoreLyt->get_system_left_space(iSystem);
    m_pBoxSystem->set_left_margin(leftMargin);

    m_pBoxSystem->set_width(width);
    m_pBoxSystem->set_height(height);
    m_yMin = top;
    m_yMax = top + height;

    return m_pBoxSystem;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_system(LUnits indent, int iFirstCol, int iLastCol,
                                    UPoint pos, GmoBoxSystem* pPrevBoxSystem)
{
    m_iSystem = m_pScoreLyt->m_iCurSystem;
    m_iFirstCol = iFirstCol;
    m_iLastCol = iLastCol;
    m_pagePos = pos;

    set_position_and_width_for_staves(indent);
    create_vertical_profile();
    fill_current_system_with_columns();
    collect_last_column_information();
    justify_current_system();
    truncate_current_system(indent);
    build_system_timegrid();
    reposition_full_measure_rests();
    engrave_system_details(m_iSystem);

    if (m_libraryScope.draw_vertical_profile())
        dbg_add_vertical_profile_shape();

    engrave_measure_numbers();

    if (!m_libraryScope.draw_vertical_profile())
        move_staves_to_avoid_collisions(pPrevBoxSystem);

    engrave_instrument_details();
    add_instruments_info();

    add_initial_line_joining_all_staves_in_system();
}

//---------------------------------------------------------------------------------------
void SystemLayouter::set_position_and_width_for_staves(LUnits indent)
{
    //For engraving staffobjs it is necessary to know the staves position.
    //In this method, once the system box is created, instrument engravers compute
    //staves position, width and vertical distance between staves. The
    //vertical distance is standard, based only on staves margins.

    UPoint org = m_pBoxSystem->get_origin();
    org.y += m_pScoreLyt->determine_top_space(0);
    org.x = 0.0f;

    m_pBoxSystem->add_shift_to_start_measure(indent);

    m_pPartsEngraver->set_position_and_width_for_staves(indent, org, m_pBoxSystem);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::on_origin_shift(LUnits yShift)
{
    m_yMax += yShift;
    m_yMin += yShift;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::create_vertical_profile()
{
    LUnits xStart = m_pBoxSystem->get_origin().x;
    LUnits xEnd = xStart + m_pBoxSystem->get_width();
    int numStaves = m_pScoreMeter->num_staves();

    m_pVProfile.reset( LOMSE_NEW VerticalProfile(xStart, xEnd, numStaves) );
    m_systemLayoutScope.set_vertical_profile(m_pVProfile.get());

    //initialize the profile with the position of each staff
    int numInstrs = m_pScore->get_num_instruments();
    for (int iInstr = 0; iInstr < numInstrs; iInstr++)
    {
        InstrumentEngraver* pInstrEngraver = m_pPartsEngraver->get_engraver_for(iInstr);
        for (int iStaff=0; iStaff < pInstrEngraver->get_num_staves(); iStaff++)
        {
            LUnits yTop = pInstrEngraver->get_top_line_of_staff(iStaff);
            LUnits yBottom = pInstrEngraver->get_bottom_line_of_staff(iStaff);
            int idxStaff = m_pScoreMeter->staff_index(iInstr, iStaff);
            m_pVProfile->initialize(idxStaff, yTop, yBottom);
        }
    }
}

//---------------------------------------------------------------------------------------
void SystemLayouter::fill_current_system_with_columns()
{
    m_pScoreLyt->m_iCurColumn = 0;
    if (m_pScoreLyt->get_num_systems() == 0)
        return;

    InstrumentEngraver* pEngrv = m_pPartsEngraver->get_engraver_for(0);
    m_pagePos.x = pEngrv->get_staves_left();

    m_uFreeSpace = (m_iSystem == 0 ? m_pScoreLyt->get_first_system_staves_size()
                                   : m_pScoreLyt->get_other_systems_staves_size() );

    m_fFirstColumnInSystem = true;
    for (int iCol = m_iFirstCol; iCol < m_iLastCol; ++iCol)
    {
        m_pScoreLyt->m_iCurColumn = iCol;
        add_system_prolog_if_necessary();
        add_column_to_system(iCol);
        m_fFirstColumnInSystem = false;
    }
    m_pScoreLyt->m_iCurColumn = m_iLastCol;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::collect_last_column_information()
{
    if (m_iLastCol > 0)
        m_barlinesInfo = m_pSpAlgorithm->get_column_barlines_information(m_iLastCol - 1);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::justify_current_system()
{
    if (m_pScoreLyt->is_system_empty(m_iSystem))
        return;

    if (system_must_be_justified())
        redistribute_free_space();

    reposition_slices_and_staffobjs();
}

//---------------------------------------------------------------------------------------
void SystemLayouter::truncate_current_system(LUnits indent)
{
    if (system_must_be_truncated())
    {
        GmoBoxSlice* pSlice = m_pSpAlgorithm->get_slice_box(m_iLastCol-1);
        LUnits width = pSlice->get_right() - m_pBoxSystem->get_left();
        m_pBoxSystem->set_width(width);
        width -= indent;
        m_pPartsEngraver->set_staves_width( width );
    }
    else if (m_constrains & k_infinite_width)
    {
        if (m_iLastCol > 0)
        {
            GmoBoxSlice* pSlice = m_pSpAlgorithm->get_slice_box(m_iLastCol-1);
            LUnits width = pSlice->get_right() - m_pBoxSystem->get_left()
                            + pSlice->get_width();
            m_pBoxSystem->set_width(width);
            width -= indent;
            m_pPartsEngraver->set_staves_width( width );
        }
        else
            m_pPartsEngraver->set_staves_width( 5000.0f );
    }
}

//---------------------------------------------------------------------------------------
bool SystemLayouter::system_must_be_truncated()
{
    //Specifications (do not change):
    //
    //Staff lines truncation only can occur when system is not justified.
    //Staff lines always run until right margin unless requesting truncation.
    //Option "StaffLines.Truncate" defines the behaviour:
    //    k_truncate_never = 0,			    Never truncate. Staff lines will always run to right margin.
    //    k_truncate_barline_final = 1,	    Truncate only if last object is a barline of type final
    //    k_truncate_barline_any = 2,		Truncate only if last object is a barline of any type
    //    k_truncate_always = 3,			Truncate always, in any case, after last object
    //
    //Option 1 is the default behaviour and it can be useful for score
    //editors: staff lines will run always to right margin until a barline
    //of type final is entered.
    //
    //Option 3 truncates staff lines after last staff object. It can be
    //useful for creating score samples (e.g., for ebooks).


    //never truncate empty scores
    if (m_pScoreMeter->is_empty_score())
        return false;

    //never truncate justified systems, crazy!
    if (system_must_be_justified())
        return false;

    //only last system can be truncated
    if (!m_pScoreLyt->is_last_system())
        return false;

    //last system must be truncated only in the following cases:

    //Opt 1: truncate only if last object is barline of type final
    if (m_pScoreLyt->m_truncateStaffLines == k_truncate_barline_final)
        return all_instr_have_final_barline();

    //Opt 2: truncate only if last object is barline (any type)
    if (m_pScoreLyt->m_truncateStaffLines == k_truncate_barline_any)
        return all_instr_have_barline();

    //Opt 3: truncate always after last object
    if (m_pScoreLyt->m_truncateStaffLines == k_truncate_always)
        return !m_pScoreLyt->is_system_empty(m_iSystem);

    //in other cases (k_truncate_never) do not truncate
    return false;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_column_to_system(int iCol)
{
    m_pagePos.x = determine_column_start_position(iCol);
    LUnits size = determine_column_size(iCol);

    create_boxes_for_column(iCol, m_pagePos.x, size);

    if (iCol != 0 && is_first_column_in_system())
        add_prolog_shapes_to_boxes();

    add_shapes_for_column(iCol);

    m_uFreeSpace -= size;
    m_pagePos.x += size;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_prolog_shapes_to_boxes()
{
    //prolog shapes were created for measuring the prolog, and saved in list
    //m_prologShapes. Now, once the first system slice box has been created, the
    //prolog shapes are added to the boxes.

    list< tuple<GmoShape*, int, int> >::iterator it;
    for (it = m_prologShapes.begin(); it != m_prologShapes.end(); ++it)
    {
        GmoShape* pShape = get<0>(*it);
        int iInstr = get<1>(*it);
        int iStaff = get<2>(*it);
        GmoBoxSliceInstr* pBox = m_pBoxSystem->get_first_instr_slice(iInstr);
        pBox->add_shape(pShape, GmoShape::k_layer_notes, iStaff);
    }
    m_prologShapes.clear();
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_system_prolog_if_necessary()
{
    if (m_pScoreLyt->m_iCurColumn > 0 && is_first_column_in_system())
	{
	    LUnits uPrologWidth = 0.0f;

	    int numInstruments = m_pScoreMeter->num_instruments();
	    for (int iInstr=0; iInstr < numInstruments; ++iInstr)
	    {
            LUnits width = engrave_prolog(iInstr);
	        uPrologWidth = max(uPrologWidth, width);
	    }

        m_pagePos.x += uPrologWidth;
        m_uFreeSpace -= uPrologWidth;

        m_pBoxSystem->add_shift_to_start_measure(uPrologWidth);
	}
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::determine_column_start_position(int UNUSED(iCol))
{
    return m_pagePos.x;
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::determine_column_size(int iCol)
{
      return m_pSpAlgorithm->get_column_width(iCol);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::create_boxes_for_column(int iCol, LUnits pos, LUnits UNUSED(size))
{
    LUnits ySystem = m_pBoxSystem->get_top();
    m_pSpAlgorithm->create_boxes_for_column(iCol, pos, ySystem);

    GmoBoxSlice* pSlice = m_pSpAlgorithm->get_slice_box(iCol);
    m_pBoxSystem->add_child_box(pSlice);
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::engrave_prolog(int iInstr)
{
    //Prolog shapes are created, for measuring the space taken by the prolog. But
    //prolog shapes cannot be added to the model because first system slice box and
    //related are not yet created. Therefore, once created, the shapes are saved in list
    //m_prologShapes so that later, when appropiate, the shapes will be added to
    //the boxes.

    LUnits uPrologWidth = 0.0f;

    //AWARE when this method is invoked the paper position is at the left marging,
    //at the start of the new system.
    LUnits xStartPos = m_pagePos.x;      //Save x to align all clefs

    //iterate over the collection of staff objects to draw current clef and key signature
    ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);

    int numStaves = pInstr->get_num_staves();
    InstrumentEngraver* pInstrEngrv = m_pPartsEngraver->get_engraver_for(iInstr);
    for (int iStaff=0; iStaff < numStaves; ++iStaff)
    {
        LUnits xPos = xStartPos;
        m_pagePos.y = pInstrEngrv->get_top_line_of_staff(iStaff);
        int idxStaff = m_pScoreMeter->staff_index(iInstr, iStaff);

        ColStaffObjsEntry* pClefEntry =
            m_pSpAlgorithm->get_prolog_clef(m_iFirstCol, idxStaff);
        ImoClef* pClef = pClefEntry ? static_cast<ImoClef*>(pClefEntry->imo_object())
                                    : nullptr;

        ColStaffObjsEntry* pKeyEntry =
            m_pSpAlgorithm->get_prolog_key(m_iFirstCol, idxStaff);

        ColStaffObjsEntry* pTimeEntry =
            m_pSpAlgorithm->get_prolog_time(m_iFirstCol, idxStaff);

        //add clef shape
        if (pClefEntry)
        {
            if (pClef && pClef->is_visible())
            {
                xPos += m_pScoreMeter->tenths_to_logical(LOMSE_SPACE_BEFORE_PROLOG, iInstr, iStaff);
                m_pagePos.x = xPos;
                GmoShape* pShape =
                    m_pShapesCreator->create_staffobj_shape(pClef, iInstr, iStaff,
                                                            m_pagePos, pClef);
                pShape->assign_id_as_prolog_shape(m_iSystem, iStaff, numStaves);
                m_prologShapes.push_back( make_tuple(pShape, iInstr, iStaff));
                xPos += pShape->get_width();
            }
        }

        //add key signature shape
        if (pKeyEntry)
        {
            ImoKeySignature* pKey = static_cast<ImoKeySignature*>( pKeyEntry->imo_object() );
            if (pKey && pKey->is_visible())
            {
                xPos += m_pScoreMeter->tenths_to_logical(LOMSE_PROLOG_GAP_BEFORE_KEY, iInstr, iStaff);
                m_pagePos.x = xPos;
                if (pKey->get_fifths() != 0)
                {
                    GmoShape* pShape =
                        m_pShapesCreator->create_staffobj_shape(pKey, iInstr, iStaff,
                                                                m_pagePos, pClef);
                    pShape->assign_id_as_prolog_shape(m_iSystem, iStaff, numStaves);
                    m_prologShapes.push_back( make_tuple(pShape, iInstr, iStaff));
                    xPos += pShape->get_width();
                }
            }
        }

        //add time signature shape if it changed at end of previous system
        if (pTimeEntry)
        {
            ImoTimeSignature* pTime = static_cast<ImoTimeSignature*>( pTimeEntry->imo_object() );
            if (pTime && pTime->is_visible())
            {
                xPos += m_pScoreMeter->tenths_to_logical(LOMSE_PROLOG_GAP_BEFORE_TIME, iInstr, iStaff);
                m_pagePos.x = xPos;
                GmoShape* pShape =
                    m_pShapesCreator->create_staffobj_shape(pTime, iInstr, iStaff,
                                                            m_pagePos, pClef);
                pShape->assign_id_as_prolog_shape(m_iSystem, iStaff, numStaves);
                m_prologShapes.push_back( make_tuple(pShape, iInstr, iStaff));
                xPos += pShape->get_width();
            }
        }

        xPos += m_pScoreMeter->tenths_to_logical(LOMSE_SPACE_AFTER_PROLOG, iInstr, iStaff);
        uPrologWidth = max(uPrologWidth, xPos - xStartPos);
    }

    m_pagePos.x = xStartPos;     //restore cursor
    set_prolog_width(uPrologWidth);

    return uPrologWidth;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_shapes_for_column(int iCol)
{
    m_pSpAlgorithm->add_shapes_to_boxes(iCol, m_pVProfile.get());
}

//---------------------------------------------------------------------------------------
bool SystemLayouter::system_must_be_justified()
{
    //Specifications (do not change):
    //
    //Justification of last system is controlled by option "Score.JustifyLastSystem",
    //accepting the following values:
    //    k_justify_never = 0,			    Never justify last system
    //    k_justify_barline_final = 1,	    Justify it only if ends in barline of type final
    //    k_justify_barline_any = 2,		Justify it only if ends in barline of any type
    //    k_justify_always = 3,			Justify it in any case
    //
    //Option 1 is the default value, and is convenient for score editors as
    //never justifies the last system as it is being written and emulates
    //the behaviour of writing scores with pen and paper. Once the score
    //is finished, the user should change to option 3.


    //if justification suppressed, for debugging, do not justify
    if (!m_libraryScope.justify_systems())
        return false;

    //if layout on infinite width, do not justify
    if (m_constrains & k_infinite_width)
        return false;

    //if not last system or free space is negative, force justification
    if (m_uFreeSpace < 0.0f || !m_pScoreLyt->is_last_system())
        return true;

    //Otherwise, the decision for final system depends on the justification option:

    //Opt. 0: never justify last system
    if (m_pScoreLyt->m_justifyLastSystem == k_justify_never)
        return false;

    //Opt. 1: justify it only if ends in barline of type final
    if (m_pScoreLyt->m_justifyLastSystem == k_justify_barline_final)
        return all_instr_have_final_barline();

    //Opt. 2: justify it only if ends in barline of any type
    if (m_pScoreLyt->m_justifyLastSystem == k_justify_barline_any)
        return all_instr_have_barline();

    //Opt. 3: justify it in any case
    return true;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::reposition_slices_and_staffobjs()
{
    LUnits yShift = m_pScoreLyt->determine_top_space(0);
    m_pSpAlgorithm->reposition_slices_and_staffobjs(m_iFirstCol, m_iLastCol, yShift,
                                                    &m_yMin, &m_yMax);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::reposition_staves_in_engravers(const vector<LUnits>& yOrgShifts)
{
    m_pPartsEngraver->reposition_staves_in_engravers(yOrgShifts);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::reposition_slice_boxes_and_shapes(const vector<LUnits>& yOrgShifts,
                                                       vector<LUnits>& heights,
                                                       LUnits bottomMarginIncr)
{
    int numInstrs = m_pScore->get_num_instruments();
    vector<LUnits> barlinesHeight(numInstrs);
    vector<vector<LUnits>> relStaffTopPositions(numInstrs);

    for (int iInstr = 0; iInstr < numInstrs; iInstr++)
    {
        InstrumentEngraver* pInstrEngrv = m_pPartsEngraver->get_engraver_for(iInstr);

        const LUnits yTop = pInstrEngrv->get_barline_top();
        barlinesHeight[iInstr] = pInstrEngrv->get_barline_bottom() - yTop;

        const int numStaves = pInstrEngrv->get_num_staves();
        relStaffTopPositions[iInstr].reserve(numStaves);

        for (int iStaff = 0; iStaff < numStaves; ++iStaff)
        {
            const LUnits relStaffTop = pInstrEngrv->get_top_line_of_staff(iStaff) - yTop;
            relStaffTopPositions[iInstr].push_back(relStaffTop);
        }
    }

    GmoBoxSystem* pSystem = get_box_system();
    pSystem->reposition_slices_and_shapes(yOrgShifts, heights, barlinesHeight,
                                          relStaffTopPositions, bottomMarginIncr, this);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::reposition_full_measure_rests()
{
    GmoBoxSystem* pBox = get_box_system();
    m_pSpAlgorithm->reposition_full_measure_rests(m_iFirstCol, m_iLastCol, pBox);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::redistribute_free_space()
{
    m_pSpAlgorithm->justify_system(m_iFirstCol, m_iLastCol, m_uFreeSpace);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_initial_line_joining_all_staves_in_system()
{
    //do not draw if 'hide staff lines' option enabled
    ImoOptionInfo* pOpt = m_pScore->get_option("StaffLines.Hide");
    bool fDrawStafflines = (pOpt == nullptr || pOpt->get_bool_value() == false);
    if (!fDrawStafflines)
        return;

    //do not draw if empty score with one instrument with one staff
    if (m_pScore->get_num_instruments() == 1)
    {
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        if (pInstr->get_num_staves() == 1 && m_pScoreMeter->is_empty_score())
            return;
    }

    //systemic barline is optional. do not draw if so asked
	if (m_pScoreMeter->must_draw_systemic_barline())
	{
        InstrumentEngraver* pInstrEngrv = m_pPartsEngraver->get_engraver_for(0);
        ImoObj* pCreator = m_pScore;
        LUnits xPos = pInstrEngrv->get_staves_left();
        LUnits yTop = pInstrEngrv->get_staves_top_line();
        int iInstr = m_pScoreMeter->num_instruments() - 1;
        pInstrEngrv = m_pPartsEngraver->get_engraver_for(iInstr);
        LUnits yBottom = pInstrEngrv->get_staves_bottom_line();
        BarlineEngraver engrv(m_libraryScope, m_pScoreMeter);
        Color color = Color(0,0,0); //TODO staff lines color?
        GmoShape* pLine =
            engrv.create_systemic_barline_shape(pCreator, m_iSystem+1, xPos, yTop, yBottom, color);
        m_pBoxSystem->add_shape(pLine, GmoShape::k_layer_staff);
	}
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_instrument_details()
{
    ImoOptionInfo* pOpt = m_pScore->get_option("StaffLines.Hide");
    bool fDrawStafflines = (pOpt == nullptr || pOpt->get_bool_value() == false);

    m_pPartsEngraver->engrave_names_and_brackets(fDrawStafflines, m_pBoxSystem,
                                                 m_iSystem);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::move_staves_to_avoid_collisions(GmoBoxSystem* pPrevBoxSystem)
{
    int numStaves = m_pScoreMeter->num_staves();
    vector<LUnits> yOrgShifts(numStaves, 0.0f); //accumulated vertical pos. increment for each staff
    vector<LUnits> heights(numStaves, 0.0f);    //height increment for each staff. To set InstrSlice boxes height

    //determine shapes limits to know free space at system top an bottom
    LUnits maxLimit = m_pVProfile->get_max_limit(numStaves-1);
    LUnits  minLimit = m_pVProfile->get_min_limit(0);
    m_pBoxSystem->set_top_limit(minLimit);      //AWARE: can be negative if shapes above
    m_pBoxSystem->set_bottom_limit(maxLimit);   //AWARE: can be negative if shapes below

    //existing separation from previous system
    LUnits minSysDistance = m_pScoreMeter->tenths_to_logical(LOMSE_MIN_SPACING_SYSTEMS);
    LUnits freeSpace = max(m_pBoxSystem->get_free_space_at_top(), 0.0f);
    freeSpace += (pPrevBoxSystem ? pPrevBoxSystem->get_free_space_at_bottom()
                                 : minSysDistance);

    //top margin increment if there are shapes above system top
    LUnits topMarginIncr = max(m_pBoxSystem->get_top() - minLimit, 0.0f);
    if (topMarginIncr > 0.0f)
        m_pBoxSystem->set_free_space_at_top(0.0f);

    //bottom margin increment if there are shapes below system bottom
    LUnits bottomMarginIncr = max(maxLimit - m_pBoxSystem->get_bottom(), 0.0f);
    if (bottomMarginIncr > 0.0f)
    {
        heights[numStaves-1] += bottomMarginIncr;
        m_pBoxSystem->set_free_space_at_bottom(0.0f);
    }

    //add space for required top margin incr or/and for minimum space between systems
    LUnits spaceIncr = max(minSysDistance - freeSpace, 0.0f);
    if (spaceIncr > 0.0f)
    {
        m_pBoxSystem->set_free_space_at_top(spaceIncr);
        topMarginIncr += spaceIncr;
    }

    //if it is necessary to increment top margin, do it
    if (topMarginIncr > 0.0f)
    {
        heights[0] += topMarginIncr;
        for (int j=0; j < numStaves; ++j)
            yOrgShifts[j] += topMarginIncr;
    }

    //determine shifts to apply to the staves to avoid overlaps between their notations
    //and height increments
    LUnits minDistance = m_pScoreMeter->tenths_to_logical(LOMSE_MIN_SPACING_STAVES);
    for (int i=1; i < numStaves; ++i)
    {
        LUnits distance = m_pVProfile->get_staves_distance(i);
        if (distance < minDistance)
        {
            distance -= minDistance;
            heights[i] += -distance;
            for (int j=i; j < numStaves; ++j)
                yOrgShifts[j] -= distance;
        }
    }

    //shift staves
    //- reposition staves in each instrument and group engraver
    //- reposition instrSlice boxes and recompute its height
    //- shift all shapes inside staffSlice boxes
    reposition_staves_in_engravers(yOrgShifts);

    if (yOrgShifts[numStaves - 1] > 0.0f || bottomMarginIncr > 0.0f)
        reposition_slice_boxes_and_shapes(yOrgShifts, heights, bottomMarginIncr);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::increment_cross_staff_stems(GmoShapeBeam* pShapeBeam, LUnits yIncrement)

{
    ImoBeam* pBeam = static_cast<ImoBeam*>(pShapeBeam->get_creator_imo());
    if (pBeam)
    {
        BeamEngraver* pEngrv
            = dynamic_cast<BeamEngraver*>(m_engravers.get_engraver(pBeam));
        if (pEngrv == nullptr)
        {
            LOMSE_LOG_ERROR("Engraver not found or is not BeamEngraver");
            return;
        }

        pEngrv->increment_cross_staff_stems(yIncrement);

        m_engravers.remove_engraver(pBeam);
        delete pEngrv;
    }
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_system_details(int iSystem)
{
    //list of AuxObjs/RelObjs for system iSystem
    std::list<PendingPair> systemAuxObjs;
    std::bitset<k_imo_last> used;
    used.reset();

    std::list<AuxObjContext*>::iterator it;
    for (it = m_pScoreLyt->m_pendingAuxObjs.begin(); it != m_pScoreLyt->m_pendingAuxObjs.end(); ++it)
    {
        int iCol = (*it)->iCol;
        int objSystem = m_pScoreLyt->get_system_containing_column(iCol);
        if (objSystem > iSystem)
            break;
        if (objSystem == iSystem)
        {
            AuxObjContext* pParent = *it;

            //add its AuxObjs/RelObjs to the system list
            ImoStaffObj* pSO = (*it)->pSO;
            if (pSO->get_num_relations() > 0)
            {
                //rel objs
                ImoRelations* pRelObjs = pSO->get_relations();
                list<ImoRelObj*>& relObjs = pRelObjs->get_relations();
                list<ImoRelObj*>::iterator itR;
                for(itR = relObjs.begin(); itR != relObjs.end(); ++itR)
                {
                    systemAuxObjs.push_back( make_pair(*itR, pParent) );
                    used.set( (*itR)->get_obj_type() );
                }
            }
            if (pSO->get_num_attachments() > 0)
            {
                //aux objs
                ImoAttachments* pAuxObjs = pSO->get_attachments();
                TreeNode<ImoObj>::children_iterator itA(pAuxObjs);
                for (itA=pAuxObjs->begin(); itA != pAuxObjs->end(); ++itA)
                {
                    systemAuxObjs.push_back( make_pair(*itA, pParent) );
                    used.set( (*itA)->get_obj_type() );
                }
            }
        }
    }

    //engrave the AuxObjs/RelObjs in this system
    //systemAuxObjs is traversed several times to engrave objects by priority order
    for (const EngravingOrderInfo& order : m_order)
    {
        setup_aux_shapes_aligner(order.alignmentScope, order.maxAlignDistance);

        const int type = order.type;
        if (used.test(type))
        {
            std::list<PendingPair>::iterator it = systemAuxObjs.begin();
            while (it != systemAuxObjs.end())
            {
                if (((*it).first)->get_obj_type() == type)
                {
                    engrave_attached_object((*it).first, *((*it).second), iSystem);
                    it = systemAuxObjs.erase(it);
                }
                else
                    ++it;
            }
        }

        if (k_imo_relobj < type && type < k_imo_relobj_last)
        {
            //engrave RelObjs that continue in next system
            for (PendingRelObj& obj : m_pScoreLyt->m_notFinishedRelObj)
            {
                if (obj.first->get_obj_type() == type)
                {
                    engrave_not_finished_relobj(obj.first, *(obj.second));
                }
            }
        }
        else if (type == k_imo_lyric)
        {
            //engrave Lyrics that continue in next system
            for (PendingLyricsObj& obj : m_pScoreLyt->m_notFinishedLyrics)
            {
                engrave_not_finished_lyrics(obj.first, *(obj.second));
            }
        }
    }

    //finish the last alignment scope
    setup_aux_shapes_aligner(k_alignment_scope_none);

    //delete engraved staffobjs
    for (it = m_pScoreLyt->m_pendingAuxObjs.begin(); it != m_pScoreLyt->m_pendingAuxObjs.end();)
    {
        int iCol = (*it)->iCol;
        int objSystem = m_pScoreLyt->get_system_containing_column(iCol);
        if (objSystem > iSystem)
            break;
        if (objSystem == iSystem)
        {
            AuxObjContext* pAOC = *it;
		    it = m_pScoreLyt->m_pendingAuxObjs.erase(it);
            delete pAOC;
        }
        else
            ++it;
    }
}

//---------------------------------------------------------------------------------------
void SystemLayouter::setup_aux_shapes_aligner(EAuxShapesAlignmentScope scope, Tenths maxAlignDistanceTenths)
{
    if (scope == k_alignment_scope_continue)
        return;

    if (m_curAuxShapesAligner)
    {
        m_curAuxShapesAligner->align_shape_base_lines(m_pVProfile.get());
        m_curAuxShapesAligner.reset();
    }

    if (scope == k_alignment_scope_begin)
    {
        const LUnits maxAlignDistance = m_pScoreMeter->tenths_to_logical_max(maxAlignDistanceTenths);
        m_curAuxShapesAligner.reset(LOMSE_NEW AuxShapesAlignersSystem(m_pScoreMeter->num_staves(),
                                                                   m_pBoxSystem->get_left(), m_pBoxSystem->get_right(),
                                                                   maxAlignDistance));
    }

    m_systemLayoutScope.set_current_aux_shapes_aligner(m_curAuxShapesAligner.get());
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_attached_object(ImoObj* pAR, const AuxObjContext& aoc,
                                             int iSystem)
{
    ImoStaffObj* pSO = aoc.pSO;
    int iInstr = aoc.iInstr;
    int iStaff = aoc.iStaff;
    int iCol = aoc.iCol;
    int idxStaff = aoc.idxStaff;

    //create engraving context
    InstrumentEngraver* pInstrEngrv = m_pPartsEngraver->get_engraver_for(iInstr);
    RelObjEngravingContext ctx;
    ctx.iSystem = iSystem;
    ctx.pVProfile = m_pVProfile.get();
    ctx.pAuxShapesAligner = m_curAuxShapesAligner.get();
    ctx.pInstrEngrv = pInstrEngrv;
    ctx.xStaffRight = pInstrEngrv->get_staves_right();
    ctx.xStaffLeft = pInstrEngrv->get_staves_left();
    ctx.yStaffTop = pInstrEngrv->get_top_line_of_staff(iStaff);
    ctx.prologWidth = m_uPrologWidth;

    //RelObjs. Exclude chords and grace notes RelObjs as they do no generate shapes
    if (pAR->is_relobj() && !pAR->is_chord() && !pAR->is_grace_relobj())
    {
        ImoRelObj* pRO = static_cast<ImoRelObj*>(pAR);
        ctx.color = pRO->get_color();

        //special case: start and end in the same object
        if (pSO == pRO->get_start_object() && pSO == pRO->get_end_object())
        {
            m_pShapesCreator->start_engraving_relobj(pRO, aoc);
            m_pShapesCreator->finish_engraving_relobj(pRO, aoc);

            GmoShape* pAuxShape = m_pShapesCreator->create_last_shape(pRO, ctx);
            add_last_rel_shape_to_model(pAuxShape, pRO, GmoShape::k_layer_aux_objs,
                                        iCol, iInstr, iStaff, idxStaff);
        }

        //normal cases: start and end in different objects
        else if (pSO == pRO->get_start_object())
        {
            m_pShapesCreator->start_engraving_relobj(pRO, aoc);
            AuxObjContext* pData = LOMSE_NEW AuxObjContext(aoc);
            m_pScoreLyt->m_notFinishedRelObj.push_back(make_pair(pRO, pData) );
        }
        else if (pSO == pRO->get_end_object())
        {
            m_pShapesCreator->finish_engraving_relobj(pRO, aoc);
            GmoShape* pAuxShape = m_pShapesCreator->create_last_shape(pRO, ctx);
            add_last_rel_shape_to_model(pAuxShape, pRO, GmoShape::k_layer_aux_objs,
                                        iCol, iInstr, iStaff, idxStaff);

            //remove from pending RelObjs
            list<PendingRelObj>::iterator itR;
            for (itR = m_pScoreLyt->m_notFinishedRelObj.begin();
                 itR != m_pScoreLyt->m_notFinishedRelObj.end(); ++itR)
            {
                if ((*itR).first == pRO)
                {
                    delete (*itR).second;
                    m_pScoreLyt->m_notFinishedRelObj.erase(itR);
                    break;
                }
            }
        }
        else
        {
            m_pShapesCreator->continue_engraving_relobj(pRO, aoc);
        }
    }

    //AuxObjs && AuxRelObjs
    else
    {
        ImoAuxObj* pAO = static_cast<ImoAuxObj*>(pAR);

        //AuxRelObjs
        if (pAO->is_lyric())
        {
            if (pSO->is_note())
            {
                ImoLyric* pLyric = static_cast<ImoLyric*>(pAO);
                ImoNote* pNote = static_cast<ImoNote*>(pSO);

                //build hash code from instrument, number & voice.
                stringstream tag;
                tag << iInstr << "-" << pLyric->get_number()
                    << "-" << pNote->get_voice();

                if (pLyric->is_start_of_relation())
                {
                    m_pShapesCreator->start_engraving_auxrelobj(pLyric, aoc, tag.str());
                    AuxObjContext* pData = LOMSE_NEW AuxObjContext(aoc);
                    m_pScoreLyt->m_notFinishedLyrics.push_back(make_pair(tag.str(), pData));
                }
                else if (pLyric->is_end_of_relation())
                {
                    m_pShapesCreator->finish_engraving_auxrelobj(pLyric, aoc, tag.str());
                    add_lyrics_shapes_to_model(tag.str(), GmoShape::k_layer_aux_objs,
                                               true, iInstr, iStaff);

                    //remove from pending AuxRelObjs
                    list<PendingLyricsObj>::iterator itAR;
                    for (itAR = m_pScoreLyt->m_notFinishedLyrics.begin();
                         itAR != m_pScoreLyt->m_notFinishedLyrics.end(); ++itAR)
                    {
                        if ((*itAR).first == tag.str())
                        {
                            delete (*itAR).second;
                            m_pScoreLyt->m_notFinishedLyrics.erase(itAR);
                            break;
                        }
                    }
                }
                else
                {
                    m_pShapesCreator->continue_engraving_auxrelobj(pLyric, aoc, tag.str());

                    //when the start of the relation was in a previous system, it is
                    //necessary to add the auxrelobj to the list of pending auxobjs if
                    //not yet included.
                    list<PendingLyricsObj>::iterator it;
                    for (it = m_pScoreLyt->m_notFinishedLyrics.begin();
                         it != m_pScoreLyt->m_notFinishedLyrics.end(); ++it)
                    {
                        if ((*it).first == tag.str())
                            break;
                    }
                    if (it == m_pScoreLyt->m_notFinishedLyrics.end())
                    {
                        AuxObjContext* pData = LOMSE_NEW AuxObjContext(aoc);
                        m_pScoreLyt->m_notFinishedLyrics.push_back(make_pair(tag.str(), pData));
                    }
                }
            }
        }

        //AuxObjs
        else
        {
            GmoShape* pAuxShape =
                m_pShapesCreator->create_auxobj_shape(pAO, aoc, m_systemLayoutScope);

            add_aux_shape_to_model(pAuxShape, GmoShape::k_layer_aux_objs,
                                   iCol, iInstr, iStaff, idxStaff);
        }
    }

}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_not_finished_relobj(ImoRelObj* pRO, const AuxObjContext& aoc)
{
    //create engraving context
    InstrumentEngraver* pInstrEngrv = m_pPartsEngraver->get_engraver_for(aoc.iInstr);
    RelObjEngravingContext ctx;
    ctx.iSystem = m_iSystem;
    ctx.pVProfile = m_pVProfile.get();
    ctx.pAuxShapesAligner = m_curAuxShapesAligner.get();
    ctx.pInstrEngrv = pInstrEngrv;
    ctx.xStaffRight = pInstrEngrv->get_staves_right();
    ctx.xStaffLeft = pInstrEngrv->get_staves_left();
    ctx.yStaffTop = pInstrEngrv->get_top_line_of_staff(aoc.iStaff);
    ctx.prologWidth = m_uPrologWidth;

    GmoShape* pAuxShape =
        m_pShapesCreator->create_first_or_intermediate_shape(pRO, ctx);

    if (pAuxShape)
    {
        //pRO may span for several systems, ensure that the new
        //shape is bound to a column from this system
        int iCol = max(aoc.iCol, m_iFirstCol);
        add_aux_shape_to_model(pAuxShape, GmoShape::k_layer_aux_objs, iCol, aoc.iInstr,
                               aoc.iStaff, aoc.idxStaff);
    }
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_not_finished_lyrics(const string& tag, const AuxObjContext& aoc)
{
    add_lyrics_shapes_to_model(tag, GmoShape::k_layer_aux_objs, false, aoc.iInstr, aoc.iStaff);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_measure_numbers()
{
    for (int iCol = m_iFirstCol; iCol < m_iLastCol; ++iCol)
    {
        bool fFirstNumberInSystem = (iCol == m_iFirstCol);

        //get the number for the measure
        TypeMeasureInfo* pInfo = m_pScoreLyt->get_measure_info_for_column(iCol);
        if (pInfo == nullptr)
            continue;   //nothing to render in this measure

        string number = pInfo->number;
        ShapeId idx = ShapeId(pInfo->count);

        if (number.empty())
            continue;   //nothing to render in this measure

        //TODO: Loop for all instruments. For MusicXML is needed as, in theory, each
        //  part could have a different policy. But I have not found any sample
        //  requiring it. So, for now, I will save time and do it only for 1st instr.
        //  Notice that TypeMeasureInfo in iCol is from first instrument. If the
        //  instruments loop is finally required, the columns must include info
        //  for all instruments.
        int iInstr = 0;
        ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
        int policy = pInstr->get_measures_numbering();

        if (measure_number_must_be_displayed(policy, pInfo, fFirstNumberInSystem))
        {
            int iStaff = 0;
            LUnits xPos = 0.0f;
            LUnits yPos = 0.0f;
            ImoObj* pCreator = m_pScore->get_instrument(iInstr);
            InstrumentEngraver* pInstrEngrv = m_pPartsEngraver->get_engraver_for(iInstr);

            if (fFirstNumberInSystem)
            {
                //engrave_measure_number_at_start_of_system();
                xPos = pInstrEngrv->get_staves_left();
            }
            else
            {
                //engrave_measure_number_at_barline();
                GmoShapeBarline* pShapeBarline =
                    m_pScoreLyt->get_start_barline_shape_for_column(iCol);
                xPos = pShapeBarline->get_right();
            }
            yPos = pInstrEngrv->get_staves_top_line()
                   - m_pScoreMeter->tenths_to_logical(20.0f, iInstr, iStaff);

            GmoShape* pShape = m_pShapesCreator->create_measure_number_shape(
                                        pCreator, number, idx, xPos, yPos);

            m_pBoxSystem->add_shape(pShape, GmoShape::k_layer_staff);

            m_yMax = max(m_yMax, pShape->get_bottom());
        }
    }
}

//---------------------------------------------------------------------------------------
bool SystemLayouter::measure_number_must_be_displayed(int policy, TypeMeasureInfo* pInfo,
                                                      bool fFirstNumberInSystem)
{
    bool fPrintNumber;
    if (policy == ImoInstrument::k_system)
        fPrintNumber = fFirstNumberInSystem;
    else if (policy == ImoInstrument::k_all)
        fPrintNumber = true;
    else
        fPrintNumber = false;

    if (fPrintNumber)
        fPrintNumber = !pInfo->fHideNumber;

    return fPrintNumber;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_last_rel_shape_to_model(GmoShape* pShape, ImoRelObj* pRO,
                                                 int layer, int iCol, int iInstr,
                                                 int iStaff, int idxStaff)
{
    if (!pShape)
    {
        delete_rel_obj_engraver(pRO);
        return;
    }

    //in case of cross-staff beams (beams with flag stem segments in two or more staves),
    //the beam must be placed on bottom staff when below or double-stemmed or in top
    //staff when above.
    //For beams in a single staff the beam shape must be placed in that staff
    bool fDeleteEngraver = true;
    if (pRO->is_beam())
    {
        int staff = static_cast<ImoBeam*>(pRO)->get_min_staff();
        GmoShapeBeam* pSB = static_cast<GmoShapeBeam*>(pShape);
        if (pSB->is_cross_staff() && !pSB->is_beam_above())
        {
            staff = static_cast<ImoBeam*>(pRO)->get_max_staff();
            fDeleteEngraver = false;
        }
        idxStaff -= (iStaff - staff);
        iStaff = staff;
    }

    add_aux_shape_to_model(pShape, layer, iCol, iInstr, iStaff, idxStaff);

    if (fDeleteEngraver)
        delete_rel_obj_engraver(pRO);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::delete_rel_obj_engraver(ImoRelObj* pRO)
{
    RelObjEngraver* pEngrv
        = dynamic_cast<RelObjEngraver*>(m_engravers.get_engraver(pRO));
    if (pEngrv == nullptr)
    {
        LOMSE_LOG_ERROR("Engraver is not RelObjEngraver");
        return;
    }
    m_engravers.remove_engraver(pRO);
    delete pEngrv;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_lyrics_shapes_to_model(const string& tag, int layer, bool fLast,
                                                int iInstr, int iStaff)
{
    LyricEngraver* pEngrv
        = dynamic_cast<LyricEngraver*>(m_engravers.get_engraver(tag));
    if (pEngrv == nullptr)
    {
        LOMSE_LOG_ERROR("Fatal: Engraver is not LyricEngraver!");
        return;
    }

    //create the shapes
    InstrumentEngraver* pInstrEngrv = m_pPartsEngraver->get_engraver_for(iInstr);
    RelObjEngravingContext ctx;
    ctx.pVProfile = m_pVProfile.get();
    ctx.xStaffRight = pInstrEngrv->get_staves_right();
    ctx.xStaffLeft = pInstrEngrv->get_staves_left();
    ctx.yStaffTop = pInstrEngrv->get_top_line_of_staff(iStaff);

    int numShapes = pEngrv->create_shapes(ctx);

    //add shapes to model
    for (int i=0; i < numShapes; ++i)
    {
        ShapeBoxInfo* pInfo = pEngrv->get_shape_box_info(i);
        GmoShape* pAuxShape = pInfo->pShape;
        if (pAuxShape)
            add_aux_shape_to_model(pAuxShape, layer, pInfo->iCol, pInfo->iInstr,
                                   pInfo->iStaff, pInfo->idxStaff);
    }

    if (fLast)
    {
        m_engravers.remove_engraver(tag);
        delete pEngrv;
    }
    else
        pEngrv->prepare_for_next_system();
}


//---------------------------------------------------------------------------------------
void SystemLayouter::add_aux_shape_to_model(GmoShape* pShape, int layer, int iCol,
                                            int iInstr, int iStaff, int idxStaff)
{
    pShape->set_layer(layer);
    GmoBoxSliceInstr* pBox = m_pSpAlgorithm->get_slice_instr(iCol, iInstr);
    pBox->add_shape(pShape, layer, iStaff);
    m_yMax = max(m_yMax, pShape->get_bottom());
    if (idxStaff >= 0)
        m_pVProfile->update(pShape, idxStaff);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::build_system_timegrid()
{
    TimeGridTable* pTable = LOMSE_NEW TimeGridTable();

    for (int iCol = m_iFirstCol; iCol < m_iLastCol; ++iCol)
    {
        TimeGridTable* pColTable =
            m_pSpAlgorithm->create_time_grid_table_for_column(iCol);

        //accumulate data
        if (pColTable)
            pTable->add_entries( pColTable->get_entries() );

        delete pColTable;
    }

    m_pBoxSystem->set_time_grid_table(pTable);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_instruments_info()
{
    int maxInstr = m_pScore->get_num_instruments() - 1;
    for (int i = 0; i <= maxInstr; i++)
    {
        ImoInstrument* pInstr = m_pScore->get_instrument(i);
        m_pBoxSystem->add_num_staves_for_instrument(pInstr->get_num_staves());
    }
}

//---------------------------------------------------------------------------------------
void SystemLayouter::dbg_add_vertical_profile_shape()
{
    m_pVProfile->dbg_add_vertical_profile_shapes(m_pBoxSystem);
}


}  //namespace lomse

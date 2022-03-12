//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_aux_shapes_aligner.h"
#include "lomse_score_meter.h"
#include "lomse_vertical_profile.h"


namespace lomse
{

//=======================================================================================
// Engraver implementation
//=======================================================================================
LUnits Engraver::tenths_to_logical(Tenths value) const
{
    return m_pMeter->tenths_to_logical(value);
}

//---------------------------------------------------------------------------------------
void Engraver::add_user_shift(ImoContentObj* pImo, UPoint* pos)
{
    (*pos).x += tenths_to_logical(pImo->get_user_location_x());
    (*pos).y += tenths_to_logical(pImo->get_user_location_y());
}


//=======================================================================================
// StaffSymbolEngraver implementation
//=======================================================================================
LUnits StaffSymbolEngraver::tenths_to_logical(Tenths value) const
{
    return m_pMeter->tenths_to_logical(value, m_iInstr, m_iStaff);
}

//---------------------------------------------------------------------------------------
double StaffSymbolEngraver::determine_font_size()
{
    return 21.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;
}

//=======================================================================================
// AuxObjEngraver implementation
//=======================================================================================
void AuxObjEngraver::add_to_aux_shapes_aligner(GmoShape* pShape, bool fAboveStaff) const
{
    AuxShapesAligner* pAligner = get_aux_shapes_aligner(m_idxStaff, fAboveStaff);
    if (pAligner)
        pAligner->add_shape(pShape);
}

//---------------------------------------------------------------------------------------
AuxShapesAligner* AuxObjEngraver::get_aux_shapes_aligner(int idxStaff, bool fAbove) const
{
    return m_pAuxShapesAligner ? &m_pAuxShapesAligner->get_aligner(idxStaff, fAbove) : nullptr;
}


//=======================================================================================
// RelObjEngraver implementation
//=======================================================================================
void RelObjEngraver::save_context_parameters(const RelObjEngravingContext& ctx)
{
    m_color = ctx.color;
    m_uStaffLeft = ctx.xStaffLeft;
    m_uStaffRight = ctx.xStaffRight;
    m_uStaffTop = ctx.yStaffTop;
    m_pVProfile = ctx.pVProfile;
    m_pAuxShapesAligner = ctx.pAuxShapesAligner;
    m_pInstrEngrv = ctx.pInstrEngrv;
    m_uPrologWidth = ctx.prologWidth;
}

//---------------------------------------------------------------------------------------
void RelObjEngraver::add_to_aux_shapes_aligner(GmoShape* pShape, bool fAboveStaff) const
{
    //AuxShapesAligner* pAligner = m_pVProfile->get_current_aux_shapes_aligner(m_idxStaff, fAboveStaff);
    AuxShapesAligner* pAligner = get_aux_shapes_aligner(m_idxStaff, fAboveStaff);
    if (pAligner)
        pAligner->add_shape(pShape);
}

//---------------------------------------------------------------------------------------
AuxShapesAligner* RelObjEngraver::get_aux_shapes_aligner(int idxStaff, bool fAbove) const
{
    return m_pAuxShapesAligner ? &m_pAuxShapesAligner->get_aligner(idxStaff, fAbove) : nullptr;
}


}  //namespace lomse

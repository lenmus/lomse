//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

////---------------------------------------------------------------------------------------
//double Engraver::determine_font_size()
//{
//    return 21.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;
//}

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

////---------------------------------------------------------------------------------------
//void StaffSymbolEngraver::add_user_shift(ImoContentObj* pImo, UPoint* pos)
//{
//    (*pos).x += tenths_to_logical(pImo->get_user_location_x());
//    (*pos).y += tenths_to_logical(pImo->get_user_location_y());
//}

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

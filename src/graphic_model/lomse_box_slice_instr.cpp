//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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

#include "lomse_box_slice_instr.h"

#include "lomse_internal_model.h"
#include "lomse_box_slice.h"
#include "lomse_shape_barline.h"
#include "lomse_shape_note.h"
#include "lomse_shape_beam.h"
#include "lomse_system_layouter.h"
#include "lomse_im_note.h"


namespace lomse
{

//=======================================================================================
// GmoBoxSliceInstr implementation
//=======================================================================================
GmoBoxSliceInstr::GmoBoxSliceInstr(ImoInstrument* pInstr, int idxStaff)
    : GmoBox(GmoObj::k_box_slice_instr, pInstr)
    , m_idxStaff(idxStaff)
{
    int numStaves = pInstr->get_num_staves();
    for (int iStaff=0; iStaff < numStaves; ++iStaff)
    {
        GmoBoxSliceStaff* pBox = LOMSE_NEW GmoBoxSliceStaff(pInstr, idxStaff++);
        add_child_box(pBox);
    }
}

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr::~GmoBoxSliceInstr()
{
}

//---------------------------------------------------------------------------------------
GmoBoxSystem* GmoBoxSliceInstr::get_system_box()
{
    GmoBoxSlice* pSlice = dynamic_cast<GmoBoxSlice*>(m_pParentBox);
    return (pSlice ? pSlice->get_system_box() : nullptr);
}

//---------------------------------------------------------------------------------------
void GmoBoxSliceInstr::add_shape(GmoShape* pShape, int layer, int iStaff)
{
    GmoBoxSliceStaff* pBox = static_cast<GmoBoxSliceStaff*>(m_childBoxes[iStaff]);
    pBox->add_shape(pShape, layer);
}

//---------------------------------------------------------------------------------------
void GmoBoxSliceInstr::reposition_slices_and_shapes(const vector<LUnits>& yOrgShifts,
                                                    const vector<LUnits>& heights,
                                                    LUnits barlinesHeight,
                                                    const std::vector<LUnits>& relStaffTopPositions,
                                                    SystemLayouter* pSysLayouter)

{
    vector<GmoBox*>::iterator it;
    int idxStaff = m_idxStaff;
    int staff = 0;
    for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it, ++staff)
    {
        GmoBoxSliceStaff* pSlice = static_cast<GmoBoxSliceStaff*>(*it);
        pSlice->reposition_shapes(yOrgShifts, barlinesHeight, relStaffTopPositions, pSysLayouter, staff);

        m_size.height += heights[idxStaff];
    }

    //shift origin
    m_origin.y += yOrgShifts[m_idxStaff];
}

//---------------------------------------------------------------------------------------
GmoBoxSliceStaff* GmoBoxSliceInstr::get_slice_staff_for(int iStaff)
{
    return static_cast<GmoBoxSliceStaff*>(m_childBoxes[iStaff]);
}


//=======================================================================================
// GmoBoxSliceStaff implementation
//=======================================================================================
GmoBoxSliceStaff::GmoBoxSliceStaff(ImoInstrument* pInstr, int idxStaff)
    : GmoBox(GmoObj::k_box_slice_staff, pInstr)
    , m_idxStaff(idxStaff)
{
}

//---------------------------------------------------------------------------------------
GmoBoxSliceStaff::~GmoBoxSliceStaff()
{
}

//---------------------------------------------------------------------------------------
void GmoBoxSliceStaff::reposition_shapes(const vector<LUnits>& yShifts,
                                         LUnits barlinesHeight,
                                         const std::vector<LUnits>& relStaffTopPositions,
                                         SystemLayouter* pSysLayouter, int UNUSED(staff))

{
    LUnits yShift = yShifts[m_idxStaff];

    if (yShift == 0.0f)
    {
        //deal only with barlines height and cross-staff stems
        list<GmoShape*>::iterator it;
        for (it=m_shapes.begin(); it != m_shapes.end(); ++it)
        {
            if ((*it)->is_shape_barline())
            {
                GmoShapeBarline* pBarlineShape = static_cast<GmoShapeBarline*>(*it);
                pBarlineShape->set_height(barlinesHeight);
                pBarlineShape->set_relative_staff_top_positions(relStaffTopPositions);
            }
        }
    }
    else
    {
        //shift shapes and do all other changes
        list<GmoShape*>::iterator it;
        for (it=m_shapes.begin(); it != m_shapes.end(); ++it)
        {
            if ((*it)->is_shape_beam())
            {
                GmoShapeBeam* pShapeBeam = static_cast<GmoShapeBeam*>(*it);
                if (pShapeBeam->is_cross_staff())
                {
//                    if (!pShapeBeam->has_chords())
//                    {
                        LUnits down = (yShift + yShifts[m_idxStaff-1]) / 2.0f;
                        LUnits increment = (yShift - yShifts[m_idxStaff-1]) / 2.0f;
                        pSysLayouter->increment_cross_staff_stems(pShapeBeam, increment);
                        (*it)->reposition_shape(down);
//                    }
//                    else //if (pShapeBeam->get_staff() == staff)
//                        (*it)->reposition_shape(yShift);
                }
                else
                    (*it)->reposition_shape(yShift);
            }
            else if ((*it)->is_shape_note())
            {
                GmoShapeNote* pShapeNote = static_cast<GmoShapeNote*>(*it);
                LUnits increment = (yShift + yShifts[m_idxStaff-1]);
                pShapeNote->reposition_shape(yShift);

                if (pShapeNote->is_cross_staff_chord())
                {
                    if (pShapeNote->is_chord_start_note())
                    {
                        pShapeNote->increment_stem_length(increment);
                    }
                    else if (pShapeNote->is_chord_flag_note() && !pShapeNote->is_up())
                    {
                        GmoShapeChordBaseNote* pBase = pShapeNote->get_base_note_shape();
                        pShapeNote = pBase->get_start_note();
                        pShapeNote->increment_stem_length(increment);
                    }
                }
//                ImoNote* pNote = static_cast<ImoNote*>(pShapeNote->get_creator_imo());
//                if (pNote->is_beamed() && pShapeNote->is_up())
//                {
//                    ImoBeam* pBeam = pNote->get_beam();
//                    if (pBeam->is_cross_staff())
//                        pShapeNote->increment_stem_length(yShift);
//                }
            }
            else if ((*it)->is_shape_barline())
            {
                GmoShapeBarline* pBarlineShape = static_cast<GmoShapeBarline*>(*it);
                pBarlineShape->reposition_shape(yShift);
                pBarlineShape->set_height(barlinesHeight);
                pBarlineShape->set_relative_staff_top_positions(relStaffTopPositions);
            }
            else
            {
                (*it)->reposition_shape(yShift);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void GmoBoxSliceStaff::dump(ostream& outStream, int level)
{
    std::ios_base::fmtflags f( outStream.flags() );  //save formating options

    outStream << setw(level*3) << level << " [" << setw(3) << m_objtype << "] "
              << get_name(m_objtype)
              << ", idxStaff=" << m_idxStaff << endl;

    outStream.flags( f );  //restore formating options
}


}  //namespace lomse

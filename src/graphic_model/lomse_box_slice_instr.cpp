//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
GmoShape* GmoBoxSliceInstr::find_staffobj_shape_before(LUnits x)
{
    vector<GmoBox*>::iterator it;
    GmoShape* pShape = nullptr;
    for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it)
    {
        GmoBoxSliceStaff* pSlice = static_cast<GmoBoxSliceStaff*>(*it);
        GmoShape* pS = pSlice->find_staffobj_shape_before(x);
        if (pS)
        {
            if (!pShape)
                pShape = pS;
            else if (pShape->get_right() < pS->get_right())
                pShape = pS;
        }
    }
    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* GmoBoxSliceInstr::find_staffobj_shape_after(LUnits x)
{
    vector<GmoBox*>::iterator it;
    GmoShape* pShape = nullptr;
    for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it)
    {
        GmoBoxSliceStaff* pSlice = static_cast<GmoBoxSliceStaff*>(*it);
        GmoShape* pS = pSlice->find_staffobj_shape_after(x);
        if (pS)
        {
            if (!pShape)
                pShape = pS;
            else if (pShape->get_left() > pS->get_left())
                pShape = pS;
        }
    }
    return pShape;
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

        m_size.height += heights[idxStaff+staff];
    }

    //shift origin
    if (m_idxStaff > 0)
        m_origin.y += yOrgShifts[m_idxStaff-1];
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
    LUnits yPrevShift = (m_idxStaff > 0 ? yShifts[m_idxStaff-1] : 0.0f);

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
                        LUnits down = (yShift + yPrevShift) / 2.0f;
                        LUnits increment = (yShift - yPrevShift) / 2.0f;
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
                LUnits increment = (yShift - yPrevShift);
                pShapeNote->reposition_shape(yShift);

                if (pShapeNote->is_cross_staff_chord())
                {
                    if (pShapeNote->is_chord_start_note())
                    {
                        pShapeNote->increment_stem_length(increment);

                        GmoShapeArpeggio* pArpeggio = pShapeNote->get_base_note_shape()->get_arpeggio();

                        if (pArpeggio)
                            pArpeggio->increase_length_up(increment);
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
GmoShape* GmoBoxSliceStaff::find_staffobj_shape_before(LUnits x)
{
    list<GmoShape*>::reverse_iterator it;
    for (it=m_shapes.rbegin(); it != m_shapes.rend(); ++it)
    {
        ImoObj* pImo = (*it)->get_creator_imo();
        if (pImo && pImo->is_staffobj())
        {
            if ((*it)->get_right() <= x)
                return *it;
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoShape* GmoBoxSliceStaff::find_staffobj_shape_after(LUnits x)
{
    list<GmoShape*>::iterator it;
    for (it=m_shapes.begin(); it != m_shapes.end(); ++it)
    {
        ImoObj* pImo = (*it)->get_creator_imo();
        if (pImo && pImo->is_staffobj())
        {
            if ((*it)->get_left() >= x)
                return *it;
        }
    }
    return nullptr;
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

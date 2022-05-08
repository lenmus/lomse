//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_graphical_model.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_im_measures_table.h"
#include "lomse_drawer.h"
#include "lomse_selections.h"
#include "lomse_time.h"
#include "lomse_control.h"
#include "lomse_box_system.h"
#include "lomse_logger.h"
#include "lomse_shape_staff.h"
#include "lomse_box_slice_instr.h"
#include "lomse_box_system.h"
#include "lomse_box_slice.h"
#include "lomse_timegrid_table.h"
#include "lomse_score_algorithms.h"
#include "lomse_logger.h"

#include <cstdlib>      //abs
#include <iomanip>


namespace lomse
{


//=======================================================================================
// Graphic model implementation
//=======================================================================================
static long m_idCounter = 0L;

//---------------------------------------------------------------------------------------
GraphicModel::GraphicModel(ImoDocument* pCreator)
    : m_modified(true)
{
    m_root = LOMSE_NEW GmoBoxDocument(this, pCreator);
    m_modelId = ++m_idCounter;
}

//---------------------------------------------------------------------------------------
GraphicModel::~GraphicModel()
{
    delete m_root;

    //delete stubs
    map<ImoId, ScoreStub*>::const_iterator it;
    for (it = m_scores.begin(); it != m_scores.end(); ++it)
        delete it->second;

    m_scores.clear();
}

//---------------------------------------------------------------------------------------
int GraphicModel::get_num_pages()
{
    return m_root->get_num_pages();
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GraphicModel::get_page(int i)
{
    return m_root->get_page(i);
}

//---------------------------------------------------------------------------------------
void GraphicModel::draw_page(int iPage, UPoint& origin, Drawer* pDrawer,
                             RenderOptions& opt)
{
    pDrawer->set_shift(-origin.x, -origin.y);
    GmoBoxDocPage* pPage = get_page(iPage);
    if (pPage)
    {
        pPage->on_draw(pDrawer, opt);
        pDrawer->render();
        pDrawer->remove_shift();
    }
    else
    {
        stringstream msg;
        msg << "Page " << iPage << " does not exists!";
        LOMSE_LOG_ERROR(msg.str());
    }
}

//---------------------------------------------------------------------------------------
void GraphicModel::dump_page(int iPage, ostream& outStream)
{
    outStream << "                    org.x        org.y     size.x      size.y" << endl;
    outStream << "-------------------------------------------------------------" << endl;
    get_page(iPage)->dump_boxes_shapes(outStream, 0);
}

//---------------------------------------------------------------------------------------
GmoObj* GraphicModel::hit_test(int iPage, LUnits x, LUnits y)
{
    return get_page(iPage)->hit_test(x, y);
}

//---------------------------------------------------------------------------------------
GmoShape* GraphicModel::find_shape_at(int iPage, LUnits x, LUnits y)
{
    return get_page(iPage)->find_shape_at(x, y);
}

//---------------------------------------------------------------------------------------
GmoBox* GraphicModel::find_inner_box_at(int iPage, LUnits x, LUnits y)
{
    return get_page(iPage)->find_inner_box_at(x, y);
}

//---------------------------------------------------------------------------------------
void GraphicModel::select_objects_in_rectangle(int iPage, SelectionSet* selection,
                                               const URect& selRect, unsigned flags)
{
    selection->clear();
    get_page(iPage)->select_objects_in_rectangle(selection, selRect, flags);
}

//---------------------------------------------------------------------------------------
GmoShape* GraphicModel::find_shape_for_object(ImoStaffObj* pSO)
{
    int numPages = get_num_pages();
    for (int i = 0; i < numPages; ++i)
    {
        GmoShape* pShape = get_page(i)->find_shape_for_object(pSO);
        if (pShape)
            return pShape;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoShape* GraphicModel::get_shape_for_noterest(ImoNoteRest* pNR)
{
    return get_main_shape_for_imo(pNR->get_id());
}

//---------------------------------------------------------------------------------------
void GraphicModel::store_in_map_imo_shape(ImoObj* pImo, GmoShape* pShape)
{
    ImoId id = pImo->get_id();
    ShapeId idx = pShape->get_shape_id();
    if (idx > 0)
        m_imoToSecondaryShape[ make_pair(id, idx) ] = pShape;
    else
        m_imoToMainShape[id] = pShape;
}

//---------------------------------------------------------------------------------------
void GraphicModel::add_to_map_imo_to_box(GmoBox* pBox)
{
    ImoObj* pImo = pBox->get_creator_imo();
    if (pImo)
    {
        ImoId id = pImo->get_id();
        //DBG ------------------------------------------------------------
        map<ImoId, GmoBox*>::const_iterator it = m_imoToBox.find(id);
        if (it != m_imoToBox.end())
        {
            LOMSE_LOG_ERROR(
                "Duplicated Imo id %d. Existing Gmo: %s. Adding Gmo: %s",
                id, (it->second)->get_name().c_str(), pBox->get_name().c_str() );
            //TO_INVESTIGATE: This is not an error for DocPage and DocPageContent
            //boxes, as they can create more boxes when the content
            //is split in two or more physical pages. Maybe the
            //error is in not having considered the implications for these
            //detected cases.
        }
        //END_DBG --------------------------------------------------------
        m_imoToBox[id] = pBox;
    }
}

//---------------------------------------------------------------------------------------
void GraphicModel::add_to_map_ref_to_box(GmoBox* pBox)
{
    GmoRef gref = pBox->get_ref();
    if (gref != k_no_gmo_ref)
    {
        LOMSE_LOG_TRACE(Logger::k_gmodel, "Added (%d, %d) %s",
            gref.first, gref.second, pBox->get_name().c_str() );
        m_ctrolToPtr[gref] = pBox;
    }
}

//---------------------------------------------------------------------------------------
GmoShape* GraphicModel::get_shape_for_imo(ImoId id, ShapeId shapeId)
{
    if (shapeId == 0)
        return get_main_shape_for_imo(id);
    else
    {
        map< pair<ImoId, ShapeId>, GmoShape*>::const_iterator it
            = m_imoToSecondaryShape.find( make_pair(id, shapeId) );
        if (it != m_imoToSecondaryShape.end())
            return it->second;
        else
            return nullptr;
    }
}

//---------------------------------------------------------------------------------------
GmoShape* GraphicModel::get_main_shape_for_imo(ImoId id)
{
    map<ImoId, GmoShape*>::const_iterator it = m_imoToMainShape.find(id);
    if (it != m_imoToMainShape.end())
        return it->second;
    else
    {
        LOMSE_LOG_INFO("No shape found for Imo id: %d", id );
        return nullptr;
    }
}

//---------------------------------------------------------------------------------------
GmoObj* GraphicModel::get_box_for_control(GmoRef gref)
{
	map<GmoRef, GmoObj*>::const_iterator it = m_ctrolToPtr.find(gref);
	if (it != m_ctrolToPtr.end())
		return it->second;
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
GmoBox* GraphicModel::get_box_for_imo(ImoId id)
{
	map<ImoId, GmoBox*>::const_iterator it = m_imoToBox.find(id);
	if (it != m_imoToBox.end())
		return it->second;
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void GraphicModel::build_main_boxes_table()
{
    if (m_root)
    {
        vector<GmoBox*>& pageBoxes = m_root->get_child_boxes();
        vector<GmoBox*>::iterator itP;
        for (itP=pageBoxes.begin(); itP != pageBoxes.end(); ++itP)
        {
            vector<GmoBox*>& contentBoxes = (*itP)->get_child_boxes();
            vector<GmoBox*>::iterator itC;
            for (itC=contentBoxes.begin(); itC != contentBoxes.end(); ++itC)
            {
                (*itC)->add_boxes_to_controls_map(this);

                (*itC)->add_boxes_to_map_imo_to_box(this);

//                vector<GmoBox*>& childBoxes = (*itC)->get_child_boxes();
//                vector<GmoBox*>::iterator it;
//                for (it=childBoxes.begin(); it != childBoxes.end(); ++it)
//                    add_to_map_imo_to_box(*it);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
GmoShapeStaff* GraphicModel::get_shape_for_first_staff_in_first_system(ImoId scoreId)
{
    GmoBoxScorePage* pBSP = static_cast<GmoBoxScorePage*>( get_box_for_imo(scoreId) );
    GmoBoxSystem* pSystem = dynamic_cast<GmoBoxSystem*>(pBSP->get_child_box(0));
    if (pSystem)
        return pSystem->get_staff_shape(0);
    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoBoxSystem* GraphicModel::get_system_for(ImoId scoreId, TimeUnits timepos)
{
    //if not found returns nullptr

    ScoreStub* pStub = get_stub_for(scoreId);
    GmoBoxScorePage* pPage = pStub->get_page_for(timepos);
    if (pPage)
    {
        //find system in this page
        GmoBoxSystem* pSystem = nullptr;
        int i = pPage->get_num_first_system();
        int maxSystem = pPage->get_num_systems() + i;
        LOMSE_LOG_DEBUG(Logger::k_events, "get_system_for(%f), i=%d, maxSystem=%d",
                        timepos, i, maxSystem);
        for (; i < maxSystem; ++i)
        {
            pSystem = pPage->get_system(i);
            LOMSE_LOG_DEBUG(Logger::k_events, "system %d. End time = %f",
                            i, pSystem->end_time());
            if (is_lower_time(timepos, pSystem->end_time()))
                break;
            else if(is_equal_time(timepos, pSystem->end_time()))
            {
                //look in next system
                int iNext = i + 1;
                if (iNext < maxSystem)
                {
                    GmoBoxSystem* pNextSystem = pPage->get_system(iNext);
                    if (is_equal_time(timepos, pNextSystem->start_time()))
                    {
                        i = iNext;
                        pSystem = pNextSystem;
                    }
                }
                break;
            }
        }

        if (i < maxSystem)
            return pSystem;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoBoxSystem* GraphicModel::get_system_for_staffobj(ImoId id)
{
    GmoShape* pShape = get_main_shape_for_imo(id);
    if (!pShape)
        return nullptr;

    GmoBoxSliceInstr* pBSI = dynamic_cast<GmoBoxSliceInstr*>( pShape->get_owner_box() );
    if (!pBSI)
        return nullptr;

    GmoBoxSlice* pBS = static_cast<GmoBoxSlice*>( pBSI->get_parent_box() );
    return static_cast<GmoBoxSystem*>( pBS->get_parent_box() );
}

//---------------------------------------------------------------------------------------
GmoBoxSystem* GraphicModel::get_system_box(int UNUSED(iSystem))
{
    //TODO
    return nullptr;
}

//---------------------------------------------------------------------------------------
ScoreStub* GraphicModel::add_stub_for(ImoScore* pScore)
{
    ScoreStub* pStub = LOMSE_NEW ScoreStub(pScore);
    m_scores[pScore->get_id()] = pStub;
    return pStub;
}

//---------------------------------------------------------------------------------------
ScoreStub* GraphicModel::get_stub_for(ImoId scoreId)
{
	map<ImoId, ScoreStub*>::const_iterator it = m_scores.find( scoreId );
	if (it != m_scores.end())
		return it->second;
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
GmMeasuresTable* GraphicModel::get_measures_table(ImoId scoreId)
{
	ScoreStub* pStub = get_stub_for(scoreId);
	return (pStub ? pStub->get_measures_table() : nullptr);
}

//---------------------------------------------------------------------------------------
int GraphicModel::get_page_number_containing(GmoObj* pGmo)
{
    GmoBoxDocPage* pBoxPage = pGmo->get_page_box();
    return m_root->get_page_number(pBoxPage);
}

//---------------------------------------------------------------------------------------
AreaInfo* GraphicModel::get_info_for_point(int iPage, LUnits x, LUnits y)
{
    if (x != m_areaInfo.x || y != m_areaInfo.y || m_areaInfo.areaType == k_point_unknown)
    {
        m_areaInfo.clear(x, y);

        //check if pointing to a shape (not to a box)
        m_areaInfo.pGmo = find_shape_at(iPage, x, y);
        if (m_areaInfo.pGmo)
        {
            //pointing to a shape. Get pointed area type
            if (m_areaInfo.pGmo->is_shape_staff())
            {
                m_areaInfo.areaType = k_point_on_staff;
                m_areaInfo.pShapeStaff = static_cast<GmoShapeStaff*>(m_areaInfo.pGmo);
            }
            else if (m_areaInfo.pGmo->is_shape_note() || m_areaInfo.pGmo->is_shape_rest())
            {
                m_areaInfo.areaType = k_point_on_note_or_rest;
            }
            else
                m_areaInfo.areaType = k_point_on_other_shape;

            //get the SliceInstr.
            GmoObj* pBox = find_inner_box_at(iPage, x, y);
            //AWARE: nullptr is returned if point is only in GmoBoxDocPage but no other box.
            if (pBox)
            {
                //as point is in a GmoShapeStaff only two possibilities:
                if (pBox->is_box_slice_instr())
                    m_areaInfo.pBSI = static_cast<GmoBoxSliceInstr*>(pBox);
                else if (pBox->is_box_system())
                {
                    //empty part of score. No GmoBoxSliceInstr.
                    //for now assume it is pointing to first staff.
                    //TODO: check if point is over a shape staff and select it
                    //TODO: next commented sentence
                    //m_areaInfo.pShapeStaff = ((lmBoxSystem*)m_areaInfo.pGmo)->GetStaffShape(1);
                }
                else
                {
                    stringstream msg;
                    msg << "Unknown case " << m_areaInfo.pGmo->get_name();
                    LOMSE_LOG_ERROR(msg.str());
                }
            }

            //get the ShapeStaff
            if (m_areaInfo.pBSI && !m_areaInfo.pShapeStaff)
            {
                GmoBoxSystem* pBSYS =
                    GModelAlgorithms::get_box_system_for(m_areaInfo.pBSI, y);
                int absStaff = pBSYS->staff_at(y);
                m_areaInfo.pShapeStaff = pBSYS->get_staff_shape(absStaff);
            }
        }

        //not pointing to a shape
        else
        {
            //check if pointing to a box
            m_areaInfo.pGmo = find_inner_box_at(iPage, x, y);
            //AWARE: nullptr is returned if point is only in GmoBoxDocPage but no other box.
            if (m_areaInfo.pGmo)
            {
                if (m_areaInfo.pGmo->is_box_slice_instr())
                {
                    m_areaInfo.pBSI = static_cast<GmoBoxSliceInstr*>(m_areaInfo.pGmo);
                    //determine staff
                    GmoBoxSystem* pBSYS =
                        GModelAlgorithms::get_box_system_for(m_areaInfo.pBSI, y);
                    int absStaff = pBSYS->staff_at(y);
                    m_areaInfo.pShapeStaff = pBSYS->get_staff_shape(absStaff);
////                    if (m_pLastBSI != m_areaInfo.pBSI)
////                    {
////                        //first time on this BoxInstrSlice, between two staves
////                        m_areaInfo.pShapeStaff = m_areaInfo.pBSI->GetNearestStaff(m_uMousePagePos);
////                    }
////                    else
////                    {
////                        //continue in this BoxInstrSlice, in same inter-staves area
////                        m_areaInfo.pShapeStaff = m_pLastShapeStaff;
////                    }
                    //determine position (above/below) relative to staff
                    if (y > m_areaInfo.pShapeStaff->get_bounds().bottom())
                        m_areaInfo.areaType = k_point_below_staff;
                    else
                        m_areaInfo.areaType = k_point_above_staff;
                }
                else
                    m_areaInfo.areaType = k_point_on_other_box;
            }
            else
                m_areaInfo.areaType = k_point_on_other;
        }
//
//        //determine timepos at mouse point, by using time grid
//        if (m_areaInfo.pBSI)
//        {
//            lmBoxSlice* pBSlice = (lmBoxSlice*)m_areaInfo.pBSI->GetParentBox();
//            m_rCurGridTime = pBSlice->GetGridTimeForPosition(m_uMousePagePos.x);
//            GetMainFrame()->SetStatusBarMouseData(m_nNumPage, m_rCurGridTime,
//                                                  pBSlice->GetNumMeasure(),
//                                                  m_uMousePagePos);
//        }
//        ////DBG --------------------------------------
//        //wxString sSO = (m_areaInfo.pGmo ? m_areaInfo.pGmo->GetName() : _T("No object"));
//        //wxLogMessage(_T("[GraphicModel::get_info_for_point] LastBSI=0x%x, CurBSI=0x%x, LastStaff=0x%x, CurStaff=0x%x, Area=%d, Object=%s"),
//        //             m_pLastBSI, m_areaInfo.pBSI, m_pLastShapeStaff, m_areaInfo.pShapeStaff,
//        //             m_areaInfo.areaType, sSO.wx_str() );
//        ////END DBG ----------------------------------
//
//        }
    }
    return &m_areaInfo;
}


//=======================================================================================
// GModelAlgorithms implementation
//=======================================================================================
GmoBoxSystem* GModelAlgorithms::get_box_system_for(GmoObj* pGmo, LUnits y)
{
    //mouse point is over inner box pGmo. Find box system

    if (pGmo)
    {
        if (pGmo->is_shape_staff())     //pImo is instrument
        {
            //click on a staff
            GmoShapeStaff* pSS = static_cast<GmoShapeStaff*>(pGmo);
            return static_cast<GmoBoxSystem*>( pSS->get_owner_box() );
        }
        else if (pGmo->is_box_slice_instr())    //pImo is instrument
        {
            //click on the score, in empty space above/below staves
            GmoBoxSliceInstr* pBSI = static_cast<GmoBoxSliceInstr*>(pGmo);
            GmoBoxSlice* pBS = static_cast<GmoBoxSlice*>( pBSI->get_parent_box() );
            return static_cast<GmoBoxSystem*>( pBS->get_parent_box() );
        }
        else if (pGmo->is_box_system())     // pImo is score
        {
            //click on the score, after last staffobj
            return static_cast<GmoBoxSystem*>(pGmo);
        }
        else if (pGmo->is_box_score_page())
        {
            //click on the score, after final barline
            GmoBoxScorePage* pPage = static_cast<GmoBoxScorePage*>(pGmo);
            int iSystem = pPage->nearest_system_to_point(y);
            return pPage->get_system(iSystem);
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
ClickPointData GModelAlgorithms::find_info_for_point(LUnits x, LUnits y, GmoObj* pGmo)
{
//    //Debug: to collect info for unit tests
//    {
//        stringstream ss;
//        ss << endl << "        LUnits x = " << x << ";" << endl;
//        ss << "        LUnits y = " << y << ";" << endl;
//        LOMSE_LOG_INFO(ss.str());
//    }

    //get clicked IM object
    if (!pGmo)
        return ClickPointData();

    //get associated IM object
    ImoObj* pImo = pGmo->get_creator_imo();
    if (!pImo)
    {
        stringstream ss;
        ss << "Invalid case? Click on a Gmo but there is no creator Imo.";
        LOMSE_LOG_ERROR(ss.str());
        return ClickPointData();
    }

    ClickPointData data;
    data.pImo = pImo;

    if (pImo->is_scoreobj())
    {
        //click point is a StaffObj, an AuxObj or a RelObj

        ImoStaffObj* pSO = nullptr;
        bool fPrologShape = false;

        if (pImo->is_staffobj())
        {
            //click on a staff object
            pSO = static_cast<ImoStaffObj*>(pImo);

            fPrologShape = (pSO->is_clef() || pSO->is_time_signature())
                           && static_cast<GmoShape*>(pGmo)->get_shape_id() != 0;

        }
        else if (pImo->is_auxobj())
        {
            //click on AuxObj. Get parent StaffObj
            ImoAuxObj* pAO = static_cast<ImoAuxObj*>(pImo);
            pSO = pAO->get_parent_staffobj();
        }
        else if (pImo->is_relobj())
        {
            //click on RelObj. Get parent StaffObj
            ImoRelObj* pRO = static_cast<ImoRelObj*>(pImo);
            pSO = pRO->get_start_object();
        }
        else
        {
            stringstream ss;
            ss << "Invalid case. Click on ScoreObj but it is not StaffObj, AuxObj or "
                << "RelObj. It is '" << pImo->get_name() << "'. Lomse maintenance error?";
            LOMSE_LOG_ERROR(ss.str());
            return data;
        }

        if (!pSO)
        {
            stringstream ss;
            ss << "Invalid case? Click on ScoreObj of type '"
                << pImo->get_name() << "', but no associated StaffObj.";
            LOMSE_LOG_ERROR(ss.str());
            return data;
        }

        //get staff and timepos
        int iStaff = pSO->get_staff();
        TimeUnits timepos = pSO->get_time();

        //get instrument number
        ImoInstrument* pInstr = pSO->get_instrument();
        ImoScore* pScore = pInstr->get_score();
        int iInstr = pScore->get_instr_number_for(pInstr);

        //get measure number
        if (!fPrologShape)
        {
            data.ml = ScoreAlgorithms::get_locator_for(pScore, timepos, iInstr);
            data.iStaff = iStaff;
        }
        else
        {
            //prolog gosht shape. Deduce measure and time from box system
            GmoBoxSliceStaff* pSliceStaff = dynamic_cast<GmoBoxSliceStaff*>(pGmo->get_owner_box());
            if (!pSliceStaff)
            {
                stringstream ss;
                ss << "Invalid GM. No parent BoxSliceStaff for prolog shape";
                LOMSE_LOG_ERROR(ss.str());
                return data;
            }
            GmoBoxSliceInstr* pSliceInstr = dynamic_cast<GmoBoxSliceInstr*>(pSliceStaff->get_owner_box());
            if (!pSliceInstr)
            {
                stringstream ss;
                ss << "Invalid GM. No parent BoxSliceInstr for BoxSliceStaff";
                LOMSE_LOG_ERROR(ss.str());
                return data;
            }
            GmoBoxSlice* pSlice = dynamic_cast<GmoBoxSlice*>(pSliceInstr->get_owner_box());
            if (!pSlice)
            {
                stringstream ss;
                ss << "Invalid GM. No parent BoxSlice for BoxSliceInstr";
                LOMSE_LOG_ERROR(ss.str());
                return data;
            }
            GmoBoxSystem* pBSYS = dynamic_cast<GmoBoxSystem*>(pSlice->get_owner_box());
            if (!pBSYS)
            {
                stringstream ss;
                ss << "Invalid GM. No parent BoxSystem for BoxSlice";
                LOMSE_LOG_ERROR(ss.str());
                return data;
            }

            //get start time and measure number
            TimeUnits time = pBSYS->start_time();
            data.ml = ScoreAlgorithms::get_locator_for(pScore, time, iInstr);
            data.iStaff = iStaff;
        }

        //TODO: (?) Fix staff for barlines
        //if clicked object is a barline staff will be always 0. To avoid this,
        //staff must be deduced from y position

        return data;
    }

    else if (pImo->is_instrument())
    {
        //click on a staff, bracket/brace, instr. name/abbrev
        if (!pGmo->is_shape_staff())
        {
            //out of score or on empty space out of staff (e.g. just inmediatelly
            //above/below an staff, on space before system, bracket/brace, instrument or
            //group name/abbrev)
            return data;
        }

        //click on a staff
        GmoShapeStaff* pStaff = static_cast<GmoShapeStaff*>(pGmo);
        int iStaff = pStaff->get_num_staff();

        //determine time and instrument number
        GmoBoxSystem* pBSYS = GModelAlgorithms::get_box_system_for(pGmo, y);
        if (!pBSYS)  //Must always exist!
        {
            stringstream ss;
            ss << "Invalid case? Click on a Staff but no parent BoxSystem.";
            LOMSE_LOG_ERROR(ss.str());
            return data;
        }

        //determine time
        TimeGridTable* pTimeGrid = pBSYS->get_time_grid_table();
        TimeUnits timepos = pTimeGrid->get_time_for_position(x);
        //LOMSE_LOG_INFO( pTimeGrid->dump() );

        //determine instrument
        int absStaff = pBSYS->staff_at(y);
        int iInstr = pBSYS->instr_number_for_staff(absStaff);

        //find nearest previous object in this staff
        GmoBoxSliceInstr* pSlice = pBSYS->find_instr_slice_at(x, y);
        if (!pSlice)
        {
            //click on staff after final barline

            //get measure number
            ImoScore* pScore = static_cast<ImoScore*>(pBSYS->get_creator_imo());
            data.ml = ScoreAlgorithms::get_locator_for(pScore, timepos, iInstr);
            data.iStaff = iStaff;

            //TODO: (?) Fix measure number when click after last measure
            //When click point is after last barline but on a staff, returned measure
            //is always the last measure and the locator points to the last barline.
            //Question: Should instead return measure+1 as point is after the barline?
            //But then the returned measure number will be grater than the number of
            //measures and could cause problems (?)

            return data;
        }

        //click on staff, on a slice containing stafobjs
        GmoShape* pShape = pSlice->find_staffobj_shape_before(x);
        if (!pShape)
        {
            //click on a BoxSliceInstrument, on the staff, but there are no StaffObjs
            //before click point. Must be just after measure start barline
            pShape = pSlice->find_staffobj_shape_after(x);
            if (!pShape)
            {
                stringstream ss;
                ss << "Invalid case? Click on a BoxSliceInstrument, on the staff, "
                    << "but there are no StaffObjs on it.";
                LOMSE_LOG_ERROR(ss.str());
                return data;
            }
        }

        ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pShape->get_creator_imo());
        //LOMSE_LOG_INFO(pSO->to_string());

        LUnits xPos = pShape->get_left();
        TimeUnits prevTimepos = pTimeGrid->get_time_for_position(xPos);

        //get instrument number
        ImoInstrument* pInstr = pSO->get_instrument();
        ImoScore* pScore = pInstr->get_score();

        //get locator
        data.ml = ScoreAlgorithms::get_locator_for(pScore, prevTimepos, iInstr);
        data.ml.location += (timepos - prevTimepos);
        data.iStaff = iStaff;

        return data;
    }

    else
    {
        //other cases:
        //- after last barline, empty space between staves: Click on box-system, Imo is score
        //- after last barline, empty space inmediately above/below the staff: Click on box-system, Imo is score
        //- empty space above/below the score: Click on box-doc-page, Imo is lenmusdoc
        return data;
    }
}


}  //namespace lomse

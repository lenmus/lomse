//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//  
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#include "lomse_score_layouter.h"

#include "lomse_basic_model.h"
#include "lomse_gm_basic.h"
//#include <iostream>
//#include <iomanip>
//#include "lomse_internal_model.h"
//#include "lomse_im_note.h"


namespace lomse
{

//-------------------------------------------------------------------------------------
// ScoreLayouter implementation
//-------------------------------------------------------------------------------------

ScoreLayouter::ScoreLayouter(ImoDocObj* pImo)
    : ContentLayouter(pImo)
{
}

ScoreLayouter::~ScoreLayouter()
{
}

void ScoreLayouter::do_layout(GmoBox* pContainerBox)
{
    initializations();
}

void ScoreLayouter::initializations()
{
    //m_pGModel = new GraphicModel();

 //   GetScoreRenderizationOptions();
 //   PrepareFontsThatMatchesStavesSizes();
 //   DecideSystemsIndentation();
 //   DecideSpaceBeforeProlog();
 //   CreateSystemCursor();

	//m_nCurrentPageNumber = 0;
 //   m_nCurSystem = 0;
 //   m_uLastSystemHeight = 0.0f;
 //   m_nAbsColumn = 1;

 //   m_pBoxScore = new lmBoxScore(m_pScore);
}

//lmBoxScore* ScoreLayouter::LayoutScore(lmScore* pScore)
//{
//    //Build the graphical model for a score justifying measures, so that they fit exactly
//    //in page width.
//    //This method encapsulates the line breaking algorithm and the spacing algorithm.
//    //Page filling is not yet implemented.
//    //This method is only invoked from lmGraphicManager::Layout()
//
//    wxASSERT(pScore);
//	//pScore->Dump(_T("dump.txt"));
//
//    //prepare things
//    m_pScore = pScore;
//    Initializations();
//
//    do
//    {
//        //Each loop cycle computes and justifies one system
//
//        //Form and add columns until no more space in current system
//        bool fFirstSystemInPage = AddNewPageIfRequired();
//        CreateSystemBox(fFirstSystemInPage);
//        bool fThisIsLastSystem = FillCurrentSystemWithColumns();
//
//        //Justify system (distribute space across all columns)
//        ComputeMeasuresSizesToJustifyCurrentSystem(fThisIsLastSystem);
//        RepositionStaffObjs();
//
//        //Final details for this system
//        SetCurrentSystemLenght(fThisIsLastSystem);
//        AddInitialLineJoiningAllStavesInSystem();
//        UpdateBoxSlicesSizes();
//        GetSystemHeightAndAdvancePaperCursor();
//
//    } while (m_pSysCursor->ThereAreObjects());
//
//
//    if (RequestedToFillScoreWithEmptyStaves())
//        FillPageWithEmptyStaves();
//
//    m_pBoxScore->PopulateLayers();      //reorganize shapes in layers for renderization
//
//    return m_pBoxScore;
//}



}  //namespace lomse

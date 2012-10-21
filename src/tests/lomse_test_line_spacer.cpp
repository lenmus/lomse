//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_score_layouter.h"
#include "lomse_system_layouter.h"
//#include "lomse_injectors.h"
//#include "lomse_document.h"
//#include "lomse_gm_basic.h"
//#include "lomse_internal_model.h"
//#include "lomse_box_system.h"
//#include "lomse_calligrapher.h"
//#include "lomse_instrument_engraver.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LineSpacerTestFixture
{
public:
    LibraryScope m_libraryScope;

    LineSpacerTestFixture()   // setUp()
        : m_libraryScope(cout)
    {
    }

    ~LineSpacerTestFixture()  // tearDown()
    {
        delete_test_data();
    }

    void delete_test_data()
    {
    }
};

SUITE(LineSpacerTest)
{

//    TEST_FIXTURE(LineSpacerTestFixture, SpaceBeforeProlog)
//    {
//        LineTable table(int line, int nInstr, LUnits uxStart, LUnits fixedSpace, LUnits varSpace);
//        //constructor for unit testing
//        ScoreMeter meter(int numInstruments, int numStaves, LUnits lineSpacing,
//                    float rSpacingFactor=0.547f,
//                    ESpacingMethod nSpacingMethod=k_spacing_proportional,
//                    Tenths rSpacingValue=15.0f,
//                    bool fDrawLeftBarline=true);
//        table.add_entry(ImoStaffObj* pSO, GmoShape* pShape, float rTime);
//        table.add_final_entry();

        //ColumnLayouter
//        //create_column()
//        GmoBoxSlice* pSlice = new GmoBoxSlice(iColumn);
//        pSlice->set_left(0.0f);
//        pSlice->set_top(0.0f);
//        ColumnLayouter* pColLyt = new ColumnLayouter(m_pScoreMeter);
//        pColLyt->set_slice_box(pBoxSlice);

//    pColLyt->start_column_measurements(uxStart, fixedSpace, variableSpace);
//    //loop to include objects
//    pColLyt->include_object(iLine, iInstr, pInstr, pSO, rTime,
//                                         nStaff, pShape);
//    //finish
//        pColLyt->finish_column_measurements(xStart);
//    //do layout column
//    bool fTrace = false;
//    pColLyt->do_spacing(fTrace);

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_EnoughtSpaceInPage_1st_true)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns

        scoreLyt.my_delete_all();
   }


}



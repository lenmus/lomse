//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_ldp_factory.h"
#include "lomse_document.h"
#include "lomse_staffobjs_table.h"
#include "lomse_internal_model.h"
#include "lomse_score_layouter.h"

#include <vector>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// helper, mock class to contain tests data for columns
class MyScoreLayouter2 : public ScoreLayouter
{
private:
    LUnits m_firstSystemSize;
    LUnits m_otherSystemsSize;
    std::vector<LUnits> m_sizes;
    std::vector<bool> m_hasSystemBreak;

public:
    MyScoreLayouter2(ImoContentObj* pImo, GraphicModel* pGModel,
                     LibraryScope& libraryScope)
        : ScoreLayouter(pImo, NULL, pGModel, libraryScope)
    {
    }
    virtual ~MyScoreLayouter2() {}

    void my_create_column(LUnits size) {
        m_sizes.push_back(size);
        m_hasSystemBreak.push_back(false);
    }
    void my_set_has_break(int iCol) { m_hasSystemBreak[iCol] = true; }
    void my_set_systems_sizes(LUnits first, LUnits others) {
        m_firstSystemSize = first;
        m_otherSystemsSize = others;
    }

    //overrides for tests
    int get_num_columns() { return int(m_sizes.size()); }
    LUnits get_target_size_for_system(int iSystem) {
        return iSystem == 0 ? m_firstSystemSize : m_otherSystemsSize;
    }
    bool column_has_system_break(int iCol) { return m_hasSystemBreak[iCol]; }
    float get_column_penalty(int iCol) { return 1.0f; }
    LUnits get_main_width(int iCol) { return m_sizes[iCol]; }
    LUnits get_trimmed_width(int iCol) { return m_sizes[iCol]; }
};


//---------------------------------------------------------------------------------------
// helper class to access protected members
class MyLinesBreaker : public LinesBreakerOptimal
{
public:
    MyLinesBreaker(ScoreLayouter* pScoreLyt, std::vector<int>& breaks)
        : LinesBreakerOptimal(pScoreLyt, breaks)
    {
    }
    ~MyLinesBreaker() {}

    void my_initialize_entries_table() { initialize_entries_table(); }
    void my_compute_optimal_break_sequence() { compute_optimal_break_sequence(); }
    void my_retrieve_breaks_sequence() { retrieve_breaks_sequence(); }
    float my_determine_penalty_for_line(int iSystem, int i, int j) {
        return determine_penalty_for_line(iSystem, i, j);
    }
    std::vector<Entry>& my_get_entries() { return m_entries; }
    void my_dump_entries() { dump_entries(cout); }
};


//---------------------------------------------------------------------------------------
class LinesBreakerTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;
    Document* m_pDoc;
    GraphicModel* m_pGModel;
    ImoScore* m_pScore;
    MyScoreLayouter2* m_pLyt;


    LinesBreakerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pDoc(NULL)
        , m_pGModel(NULL)
        , m_pLyt(NULL)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~LinesBreakerTestFixture()    //TearDown fixture
    {
        delete_test_data();
    }

    void create_score_layouter()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope, cout);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData ))) ))" );
        m_pGModel = LOMSE_NEW GraphicModel();
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        m_pLyt = LOMSE_NEW MyScoreLayouter2(m_pScore, m_pGModel, m_libraryScope);
        m_pLyt->my_set_systems_sizes(15000.0f, 18000.0f);
    }

    void delete_test_data()
    {
        delete m_pDoc;
        m_pDoc = NULL;
        delete m_pGModel;
        m_pGModel = NULL;
        delete m_pLyt;
        m_pLyt = NULL;
    }
};

SUITE(LinesBreakerTest)
{

    //===================================================================================
    // tests for LinesBreakerSimple
    //===================================================================================

    TEST_FIXTURE(LinesBreakerTestFixture, FillColumns)
    {
        create_score_layouter();
        for (int i=0; i < 30; i++)
            m_pLyt->my_create_column(1450.0f);
        std::vector<int> breaks;
        LinesBreakerSimple breaker(m_pLyt, breaks);

        breaker.decide_line_breaks();

        //cout << "num.breaks = " << breaks.size() << endl;
        //cout << "breaks = " << breaks[0] << ", " << breaks[1]
             //<< ", " << breaks[2] << endl;
        CHECK( breaks.size() == 3 );
        CHECK( breaks[0] == 0 );        //system 0: 0-9
        CHECK( breaks[1] == 10 );       //system 1: 10-21
        CHECK( breaks[2] == 22 );       //system 1: 22-29

        delete_test_data();
    }

    TEST_FIXTURE(LinesBreakerTestFixture, SystemBreak)
    {
        create_score_layouter();
        for (int i=0; i < 20; i++)
            m_pLyt->my_create_column(1450.0f);
        m_pLyt->my_set_has_break(15);
        std::vector<int> breaks;
        LinesBreakerSimple breaker(m_pLyt, breaks);

        breaker.decide_line_breaks();

        CHECK( breaks.size() == 3 );
        CHECK( breaks[0] == 0 );        //system 0: 0-9
        CHECK( breaks[1] == 10 );       //system 1: 10-14
        CHECK( breaks[2] == 15 );       //system 1: 15-19

        delete_test_data();
    }


    //===================================================================================
    // tests for LinesBreakerOptimal
    //===================================================================================

    TEST_FIXTURE(LinesBreakerTestFixture, Optimal_Penalty)
    {
        create_score_layouter();
        for (int i=0; i < 30; i++)
            m_pLyt->my_create_column(1500.0f);
        std::vector<int> breaks;
        MyLinesBreaker breaker(m_pLyt, breaks);

        float penalty = breaker.my_determine_penalty_for_line(0, 0, 9);

        //cout << "penalty = " << penalty << endl;
        CHECK( penalty == 0.0f );

        delete_test_data();
    }

    TEST_FIXTURE(LinesBreakerTestFixture, Optimal_PenaltyForSystemBreak)
    {
        create_score_layouter();
        for (int i=0; i < 10; i++)
            m_pLyt->my_create_column(1450.0f);
        m_pLyt->my_set_has_break(5);
        std::vector<int> breaks;
        MyLinesBreaker breaker(m_pLyt, breaks);

        float penalty = breaker.my_determine_penalty_for_line(0, 0, 6);

        //cout << "penalty = " << penalty << endl;
        CHECK( penalty > 100000.0f );

        delete_test_data();
    }

    TEST_FIXTURE(LinesBreakerTestFixture, Optimal_SystemBreak)
    {
        create_score_layouter();
        for (int i=0; i < 20; i++)
            m_pLyt->my_create_column(1450.0f);
        m_pLyt->my_set_has_break(15);
        std::vector<int> breaks;
        MyLinesBreaker breaker(m_pLyt, breaks);

        breaker.decide_line_breaks();

        //cout << "num.breaks = " << breaks.size() << endl;
        //cout << "breaks = " << breaks[0] << ", " << breaks[1]
        //     << ", " << breaks[2] << endl;
        CHECK( breaks.size() == 3 );
        CHECK( breaks[0] == 0 );        //system 0: 0-9
        CHECK( breaks[1] == 10 );       //system 1: 10-15
        CHECK( breaks[2] == 16 );       //system 1: 16-19

        delete_test_data();
    }

    TEST_FIXTURE(LinesBreakerTestFixture, Optimal_DecideBreaks)
    {
        create_score_layouter();
        for (int i=0; i < 30; i++)
            m_pLyt->my_create_column(1450.0f);
        std::vector<int> breaks;
        MyLinesBreaker breaker(m_pLyt, breaks);

        breaker.decide_line_breaks();

//        cout << "num.breaks = " << breaks.size() << endl;
//        cout << "breaks = " << breaks[0] << ", " << breaks[1]
//             << ", " << breaks[2] << endl;
        CHECK( breaks.size() == 3 );
        CHECK( breaks[0] == 0 );        //0..9 (10):    size=14500, space = 50
        CHECK( breaks[1] == 10 );       //10..21 (12)   size=17400, space = 600
        CHECK( breaks[2] == 22 );       //22..29 (8)    size=11600, space = 6400

        delete_test_data();
    }

};



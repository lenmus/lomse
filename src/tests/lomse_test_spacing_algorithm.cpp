//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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
#include "lomse_im_note.h"
#include "lomse_spacing_algorithm.h"
#include "lomse_pitch.h"
#include "lomse_score_layouter.h"
#include "lomse_graphical_model.h"
#include "lomse_gm_basic.h"
#include "lomse_spacing_algorithm_gourlay.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
// helper, for accessing protected members
class MyScoreLayouter3 : public ScoreLayouter
{
public:
    MyScoreLayouter3(ImoContentObj* pImo, GraphicModel* pGModel,
                    LibraryScope& libraryScope)
        : ScoreLayouter(pImo, nullptr, pGModel, libraryScope)
    {
    }
    virtual ~MyScoreLayouter3() {}

    SpacingAlgorithm* get_spacing_algorithm() { return m_pSpAlgorithm; }
    void my_delete_all() { delete_not_used_objects(); }
};

//---------------------------------------------------------------------------------------
// helper, for accessing protected members
class MySpAlgGourlay : public SpAlgGourlay
{
public:
    MySpAlgGourlay(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                   ScoreLayouter* pScoreLyt, ImoScore* pScore,
                   ShapesStorage& shapesStorage, ShapesCreator* pShapesCreator,
                   PartsEngraver* pPartsEngraver)
        : SpAlgGourlay(libraryScope, pScoreMeter, pScoreLyt, pScore, shapesStorage,
                       pShapesCreator, pPartsEngraver)
    {
    }
    virtual ~MySpAlgGourlay() {}

    ColumnDataGourlay* my_get_column(int i) { return m_columns[i]; }
    list<TimeSlice*>& my_get_slices() { return m_slices; }
    vector<StaffObjData*>& my_get_data_vector() { return m_data; }
};

//---------------------------------------------------------------------------------------
// helper, for accessing protected members
class MyTimeSlice : public TimeSlice
{
public:
    MyTimeSlice(ColStaffObjsEntry* pEntry, int entryType, int column, int iData)
        : TimeSlice(pEntry, entryType, column, iData)
    {
    }
    virtual ~MyTimeSlice() {}

    inline int my_get_iData() { return m_iFirstData; }
    inline int my_get_num_entries() { return m_numEntries; }

};

//---------------------------------------------------------------------------------------
// helper, for accessing protected members
class MyColumnDataGourlay : public ColumnDataGourlay
{
public:
    MyColumnDataGourlay(TimeSlice* pSlice)
        : ColumnDataGourlay(pSlice)
    {
    }
    virtual ~MyColumnDataGourlay() {}

    inline MyTimeSlice* my_get_ordered_slice(int i)
    {
        return static_cast<MyTimeSlice*>( m_orderedSlices[i] );
    }

    inline MyTimeSlice* my_get_first_slice() {
        return static_cast<MyTimeSlice*>(m_pFirstSlice);
    }
};


//---------------------------------------------------------------------------------------
class SpAlgGourlayTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    SpAlgGourlayTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~SpAlgGourlayTestFixture()    //TearDown fixture
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    void dump_columns(MySpAlgGourlay* pAlg, ostream& ss)
    {
        int nCols = pAlg->get_num_columns();
        for (int i=0; i < nCols; ++i)
        {
            pAlg->dump_column_data(i, ss);
        }
    }

    void dump_columns_ordered_segments(MySpAlgGourlay* pAlg, ostream& ss)
    {
        int nCols = pAlg->get_num_columns();
        for (int i=0; i < nCols; ++i)
        {
            ColumnDataGourlay* pCol = pAlg->my_get_column(i);
            pCol->dump(ss, true);
        }
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }


};

SUITE(SpAlgGourlayTest)
{
    TEST_FIXTURE(SpAlgGourlayTestFixture, SpAlgGourlay_01)
    {
        //@ 01. create columns for one line
        //@     Check that columns, slices and staffobj data are created

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 e g+)(n e4 e g-)(n d4 q)"
            ")) )))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        MyScoreLayouter3 scoreLyt(pImoScore, &gmodel, m_libraryScope);

        scoreLyt.prepare_to_start_layout();     //this creates columns and do spacing
        MySpAlgGourlay* pAlg = static_cast<MySpAlgGourlay*>(scoreLyt.get_spacing_algorithm());
        CHECK( pAlg != nullptr );

        //columns & slices
        CHECK( pAlg->get_num_columns() == 2 );
        ColumnDataGourlay* pCol;
        pCol = pAlg->my_get_column(0);
        CHECK( pCol->m_orderedSlices.size() == 3 );
        pCol = pAlg->my_get_column(1);
        CHECK( pCol->m_orderedSlices.size() == 1 );

        //staffobj data
        list<TimeSlice*>& slices = pAlg->my_get_slices();
        CHECK( slices.size() == 4 );
        vector<StaffObjData*>& data = pAlg->my_get_data_vector();
        CHECK( data.size() == 4 );
        list<TimeSlice*>::iterator it = slices.begin();
        MyTimeSlice* pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( pSlice->my_get_iData() == 0 );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( pSlice->my_get_iData() == 1 );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( pSlice->my_get_iData() == 2 );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( pSlice->my_get_iData() == 3 );

//        cout << test_name() << endl;
//        ColStaffObjs* pSOCol = pImoScore->get_staffobjs_table();
//        cout << pSOCol->dump() << endl;
//        dump_columns(pAlg, cout);

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(SpAlgGourlayTestFixture, SpAlgGourlay_02)
    {
        //@ 02. create columns for several lines
        //@     Check that columns, slices and staffobj data are created, and
        //@     check that slices contains data for pre-stretch extend xi

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(n c4 e p1 g+)(n +d4 e p1 g-)(n g4 q)"
            "(n c3 q p2 v2)(n d3 q)"
            ")) )))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        MyScoreLayouter3 scoreLyt(pImoScore, &gmodel, m_libraryScope);

        scoreLyt.prepare_to_start_layout();     //this creates columns and do spacing
        MySpAlgGourlay* pAlg = static_cast<MySpAlgGourlay*>(scoreLyt.get_spacing_algorithm());
        CHECK( pAlg != nullptr );
        CHECK( pAlg->get_num_columns() == 2 );
        ColumnDataGourlay* pCol;
        pCol = pAlg->my_get_column(0);
        CHECK( pCol->m_orderedSlices.size() == 3 );
        pCol = pAlg->my_get_column(1);
        CHECK( pCol->m_orderedSlices.size() == 1 );

        //check xi computation & iData
        list<TimeSlice*>& slices = pAlg->my_get_slices();
        CHECK( slices.size() == 4 );
        list<TimeSlice*>::iterator it = slices.begin();
        MyTimeSlice* pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_left_rod(), 0.0f) );
        CHECK( is_equal(pSlice->get_right_rod(), 0.0f) );
        CHECK( is_equal(pSlice->get_fixed_extent(), 1096.0f) );
        CHECK( pSlice->my_get_iData() == 0 );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_left_rod(), 219.0f) );
        CHECK( is_equal(pSlice->get_right_rod(), 212.0f) );
        CHECK( is_equal(pSlice->get_fixed_extent(), 45.0f) );
        CHECK( pSlice->my_get_iData() == 2 );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_left_rod(), 219.0f) );
        CHECK( is_equal(pSlice->get_right_rod(), 0.0f) );
        CHECK( is_equal(pSlice->get_fixed_extent(), 45.0f) );
        CHECK( pSlice->my_get_iData() == 4 );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_left_rod(), 219.0f) );
        CHECK( is_equal(pSlice->get_right_rod(), 0.0f) );
        CHECK( is_equal(pSlice->get_fixed_extent(), 45.0f) );
        CHECK( pSlice->my_get_iData() == 5 );

//        cout << test_name() << endl;
//        ColStaffObjs* pSOCol = pImoScore->get_staffobjs_table();
//        cout << pSOCol->dump() << endl;
//        dump_columns(pAlg, cout);

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(SpAlgGourlayTestFixture, SpAlgGourlay_03)
    {
        //@ 03. compute di, ds, ci & fi for each slice
        //@     This checks TimeSlice::compute_spring_data()

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(n a4 e p1 g+ (tm 2 3)(t 1 + 3 2))"
            "(n a4 e (tm 2 3))(n d4 e g- (tm 2 3)(t 1 -))(n g4 q)"
            "(n c3 s p2 g+ v2)(n d3 e. g-)(n e3 q)"
            ")) )))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        MyScoreLayouter3 scoreLyt(pImoScore, &gmodel, m_libraryScope);

        scoreLyt.prepare_to_start_layout();     //this creates columns and do spacing
        MySpAlgGourlay* pAlg = static_cast<MySpAlgGourlay*>(scoreLyt.get_spacing_algorithm());
        CHECK( pAlg != nullptr );
        CHECK( pAlg->get_num_columns() == 2 );
        ColumnDataGourlay* pCol;
        pCol = pAlg->my_get_column(0);
        CHECK( pCol->m_orderedSlices.size() == 5 );
        pCol = pAlg->my_get_column(1);
        CHECK( pCol->m_orderedSlices.size() == 1 );
        list<TimeSlice*>& slices = pAlg->my_get_slices();
        CHECK( slices.size() == 6 );

        //check di computation
        list<TimeSlice*>::iterator it = slices.begin();
        MyTimeSlice* pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_shortest_duration(), 0.0f) );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_shortest_duration(), 16.0f) );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_shortest_duration(), 21.333f) );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_shortest_duration(), 21.333f) );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_shortest_duration(), 21.333f) );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_shortest_duration(), 64.0f) );

        //check ds computation
        it = slices.begin();
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_spring_duration(), 0.0f) );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_spring_duration(), 16.0f) );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_spring_duration(), 5.333f) );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_spring_duration(), 21.333f) );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_spring_duration(), 21.333f) );
        ++it;
        pSlice = static_cast<MyTimeSlice*>(*it);
        CHECK( is_equal(pSlice->get_spring_duration(), 64.0f) );

//        cout << test_name() << endl;
//        ColStaffObjs* pSOCol = pImoScore->get_staffobjs_table();
//        cout << pSOCol->dump() << endl;
//        dump_columns(pAlg, cout);

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(SpAlgGourlayTestFixture, SpAlgGourlay_04)
    {
        //@ 04. Build column vector of ordered slices.
        //@     This checks ColumnDataGourlay::order_slices()

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(n a4 e p1 g+ (tm 2 3)(t 1 + 3 2))"
            "(n a4 e (tm 2 3))(n d4 e g- (tm 2 3)(t 1 -))(n g4 q)"
            "(n c3 s p2 g+ v2)(n d3 e. g-)(n e3 q)"
            ")) )))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        MyScoreLayouter3 scoreLyt(pImoScore, &gmodel, m_libraryScope);

        scoreLyt.prepare_to_start_layout();     //this creates columns and do spacing
        MySpAlgGourlay* pAlg = static_cast<MySpAlgGourlay*>(scoreLyt.get_spacing_algorithm());
        CHECK( pAlg != nullptr );
        CHECK( pAlg->get_num_columns() == 2 );

        MyColumnDataGourlay* pCol = static_cast<MyColumnDataGourlay*>(pAlg->my_get_column(0));
        CHECK( pCol->m_orderedSlices.size() == 5 );

        MyTimeSlice* pTS1 = pCol->my_get_ordered_slice(0);
        MyTimeSlice* pTS2 = pCol->my_get_ordered_slice(1);
        for (int i=3; i < 5; ++i)
        {
            CHECK( pTS1->get_pre_stretching_force() <= pTS2->get_pre_stretching_force() );
            pTS1 = pTS2;
            pTS2 = pCol->my_get_ordered_slice(i);
        }

//        cout << test_name() << endl;
//        ColStaffObjs* pSOCol = pImoScore->get_staffobjs_table();
//        cout << pSOCol->dump() << endl;
//        dump_columns_ordered_segments(pAlg, cout);

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(SpAlgGourlayTestFixture, SpAlgGourlay_05)
    {
        //@ 05. Build prolog slices.

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData "
            "(clef G)(key a)(time 2 4)"
            "(n e4 e g+)(n g4 e g-)(n d4 e g+)(n g4 e g-)"
            "(barline)))"
            "(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)"
            "(key a)(time 2 4)"
            "(n g2 s g+ p2)(n d3 s)(n g3 s)(n b3 s g-)"
            "(n d4 s g+ p1)(n g4 s)(n d4 s)(n b3 s g-)"
            "(barline)"
            ")) )))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        MyScoreLayouter3 scoreLyt(pImoScore, &gmodel, m_libraryScope);

        scoreLyt.prepare_to_start_layout();     //this creates columns and do spacing
        MySpAlgGourlay* pAlg = static_cast<MySpAlgGourlay*>(scoreLyt.get_spacing_algorithm());
        CHECK( pAlg != nullptr );
        CHECK( pAlg->get_num_columns() == 1 );

        MyColumnDataGourlay* pCol = static_cast<MyColumnDataGourlay*>(pAlg->my_get_column(0));
        CHECK( pCol->m_orderedSlices.size() == 10 );

        MyTimeSlice* pTS1 = pCol->my_get_first_slice();
        CHECK( pTS1->my_get_num_entries() == 9 );

//        cout << test_name() << endl;
//        ColStaffObjs* pSOCol = pImoScore->get_staffobjs_table();
//        cout << pSOCol->dump() << endl;
//        dump_columns(pAlg, cout);

        scoreLyt.my_delete_all();
    }

};

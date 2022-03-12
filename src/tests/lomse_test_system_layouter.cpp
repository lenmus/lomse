//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_system_layouter.h"
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_score_layouter.h"
#include "lomse_score_meter.h"

#include <cmath>

using namespace UnitTest;
using namespace std;
using namespace lomse;

//
////---------------------------------------------------------------------------------------
//// helper, mock class
//class MyColumnStorage1 : public ColumnStorage
//{
//private:
//
//public:
//    MyColumnStorage1(LUnits startHook, LUnits UNUSED(bodyWidth), LUnits endHook)
//        : ColumnStorage()
//    {
//        m_uEndHookWidth = endHook;
//        m_uxFirstAnchor = startHook;
//    }
//    MyColumnStorage1(LUnits xStart, LUnits fixedSpace, LUnits xs, LUnits xa, LUnits xr,
//                     LUnits xv, LUnits xf, bool fEndsInBarline=false)
//        : ColumnStorage()
//    {
//        m_uxStart = xStart;
//        m_uStartFixedSpace = fixedSpace;
//        m_uxFirstSymbol = xs;
//        m_uxFirstAnchor = xa;
//        m_uxRightEdge = xr;
//        m_uxStartOfEndVarSpace = xv;
//        m_uxFinal = xf;
//        m_uEndHookWidth = (fEndsInBarline ? xf - xr : xf - xv);
//        m_fVisibleBarline = fEndsInBarline;
//    }
//    virtual ~MyColumnStorage1() {}
//
//    inline bool my_are_there_lines() { return get_last_line() != end(); }
//    inline LUnits my_get_initial_fixed_space() { return m_uStartFixedSpace; }
//};
//
//
////---------------------------------------------------------------------------------------
//// helper, mock class
//class MyColumnLayouter : public ColumnLayouter
//{
//private:
//
//public:
//    MyColumnLayouter(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
//                     ColumnStorage* pStorage)
//        : ColumnLayouter(libraryScope, pScoreMeter, pStorage)
//    {
//    }
//    virtual ~MyColumnLayouter() {}
//
//};
//
////---------------------------------------------------------------------------------------
//// helper, mock class
//class MySystemLayouter : public SystemLayouter
//{
//private:
//
//public:
//    MySystemLayouter(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
//                     ShapesStorage& shapesStorage,
//                     std::vector<ColumnLayouter*>& colLayouters,
//                     PartsEngraver* pPartsEngravers,
//                     SlicesSet& slices)
//        : SystemLayouter(NULL, //ScoreLayouter* pScoreLyt,
//                         libraryScope, pScoreMeter,
//                         NULL, //ImoScore* pScore,
//                         shapesStorage,
//                         NULL, //ShapesCreator* pShapesCreator,
//                         colLayouters, pPartsEngravers,
//                         slices)
//    {
//    }
//    virtual ~MySystemLayouter() {}
//
//    void my_set_is_first_column_in_system(bool value) { m_fFirstColumnInSystem = value; }
//    void my_set_page_cursor(LUnits x, LUnits y) {
//        m_pagePos.x = x;
//        m_pagePos.y = y;
//    }
//    LUnits my_determine_column_start_position(int iCol) {
//        return determine_column_start_position(iCol);
//    }
//    LUnits my_determine_column_size(int iCol) { return determine_column_size(iCol); }
//    void my_redistribute_free_space() { redistribute_free_space(); }
//    void my_set_free_space(LUnits space) { m_uFreeSpace = space; }
//    void my_set_columns_range(int first, int last) { m_iLastCol = last; m_iFirstCol = first; }
//    LUnits my_get_trimmed_width(int iCol) { return m_ColLayouters[iCol]->get_trimmed_width(); }
//    LUnits my_get_justified_width(int iCol) { return m_ColLayouters[iCol]->get_justified_width(); }
//
//};
//
//
////---------------------------------------------------------------------------------------
//class SystemLayouterTestFixture
//{
//public:
//    LibraryScope m_libraryScope;
//    std::string m_scores_path;
//
//    SystemLayouterTestFixture()   // setUp()
//        : m_libraryScope(cout)
//        , m_scores_path(TESTLIB_SCORES_PATH)
//    {
//        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
//    }
//
//    ~SystemLayouterTestFixture()  // tearDown()
//    {
//    }
//
//    bool is_equal(float x, float y)
//    {
//        return (fabs(x - y) < 0.1f);
//    }
//
////    SystemLayouter* create_system_layouter(ColumnStorage* pStorage0,
////                                           ColumnStorage* pStorage1)
////    {
////        ScoreMeter meter(1, 1, 150.0f);
////        MyColumnLayouter colLyt0(m_libraryScope, &meter, pStorage0);
////        MyColumnLayouter colLyt1(m_libraryScope, &meter, pStorage1);
////        std::vector<ColumnLayouter*> colLayouters;
////        colLayouters.push_back(&colLyt0);
////        colLayouters.push_back(&colLyt1);
////        std::vector<InstrumentEngraver*> instrEngravers;
////        ShapesStorage shapesStorage;
////
////        MySystemLayouter sysLyt(m_libraryScope, &meter, shapesStorage,
////                                colLayouters, instrEngravers);
////        sysLyt.my_set_page_cursor(1500.0f, 2000.0f);
////        sysLyt.my_set_columns_range(0, 2);
////    }
//
//};
//
//
//SUITE(SystemLayouterTest)
//{
//
//    // determine_column_start_position() ------------------------------------------------
//
//    TEST_FIXTURE(SystemLayouterTestFixture, ColumnStartPosition_FirstCol)
//    {
//        ScoreMeter meter(1, 1, 150.0f);
//        MyColumnStorage1* pStorage0 = LOMSE_NEW MyColumnStorage1(0.0f, 1800.0f, 500.0f);
//        MyColumnLayouter colLyt0(m_libraryScope, &meter, pStorage0);
//        MyColumnStorage1* pStorage1 = LOMSE_NEW MyColumnStorage1(200, 1200.0f, 400.0f);
//        MyColumnLayouter colLyt1(m_libraryScope, &meter, pStorage1);
//        std::vector<ColumnLayouter*> colLayouters;
//        colLayouters.push_back(&colLyt0);
//        colLayouters.push_back(&colLyt1);
//        ShapesStorage shapesStorage;
//
//        MySystemLayouter sysLyt(m_libraryScope, &meter, shapesStorage,
//                                colLayouters, NULL);
//        sysLyt.my_set_page_cursor(1500.0f, 2000.0f);
//        sysLyt.my_set_is_first_column_in_system(true);
//
//        LUnits pos = sysLyt.my_determine_column_start_position(0);
//
//        //cout << "column start pos = " << pos << endl;
//        CHECK( is_equal(pos, 1500.0f) );
//    }
//
//    TEST_FIXTURE(SystemLayouterTestFixture, ColumnStartPosition_OtherCol_Case1)
//    {
//        //second column has start hook, but lower than first column end hook.
//
//        ScoreMeter meter(1, 1, 150.0f);
//        MyColumnStorage1* pStorage0 = LOMSE_NEW MyColumnStorage1(0.0f, 1800.0f, 500.0f);
//        MyColumnLayouter colLyt0(m_libraryScope, &meter, pStorage0);
//        MyColumnStorage1* pStorage1 = LOMSE_NEW MyColumnStorage1(200.0f, 1200.0f, 400.0f);
//        MyColumnLayouter colLyt1(m_libraryScope, &meter, pStorage1);
//        std::vector<ColumnLayouter*> colLayouters;
//        colLayouters.push_back(&colLyt0);
//        colLayouters.push_back(&colLyt1);
//        ShapesStorage shapesStorage;
//
//        MySystemLayouter sysLyt(m_libraryScope, &meter, shapesStorage,
//                                colLayouters, NULL);
//        sysLyt.my_set_page_cursor(1500.0f, 2000.0f);
//        sysLyt.my_set_is_first_column_in_system(false);
//
//        LUnits pos = sysLyt.my_determine_column_start_position(1);
//
//        //cout << "column start pos = " << pos << endl;
//        CHECK( is_equal(pos, 1800.0f) );    //1500 + (500-200)
//    }
//
//    TEST_FIXTURE(SystemLayouterTestFixture, ColumnStartPosition_OtherCol_Case2)
//    {
//        //second column has start hook, greater than first column end hook.
//
//        ScoreMeter meter(1, 1, 150.0f);
//        MyColumnStorage1* pStorage0 = LOMSE_NEW MyColumnStorage1(0.0f, 1800.0f, 500.0f);
//        MyColumnLayouter colLyt0(m_libraryScope, &meter, pStorage0);
//        MyColumnStorage1* pStorage1 = LOMSE_NEW MyColumnStorage1(850.0f, 1200.0f, 400.0f);
//        MyColumnLayouter colLyt1(m_libraryScope, &meter, pStorage1);
//        std::vector<ColumnLayouter*> colLayouters;
//        colLayouters.push_back(&colLyt0);
//        colLayouters.push_back(&colLyt1);
//        ShapesStorage shapesStorage;
//
//        MySystemLayouter sysLyt(m_libraryScope, &meter, shapesStorage,
//                                colLayouters, NULL);
//        sysLyt.my_set_page_cursor(1500.0f, 2000.0f);
//        sysLyt.my_set_is_first_column_in_system(false);
//
//        LUnits pos = sysLyt.my_determine_column_start_position(1);
//
//        //cout << "column start pos = " << pos << endl;
//        CHECK( is_equal(pos, 1500.0f) );    //1500
//    }
//
//    // determine_column_size() ----------------------------------------------------------
//
//    TEST_FIXTURE(SystemLayouterTestFixture, ColumnSize_FirstCol)
//    {
//        //First column size = column width - end hook
//
//        ScoreMeter meter(1, 1, 150.0f);
//        MyColumnStorage1* pStorage0
//            //                     x0    sfs    xs    xa     xr       xv       xf       barline
//            = LOMSE_NEW MyColumnStorage1(0.0f, 20.0f, 20.0, 20.0f, 1800.0f, 1800.0f, 2300.0f, false);
//        MyColumnLayouter colLyt0(m_libraryScope, &meter, pStorage0);
//        MyColumnStorage1* pStorage1
//            //                     x0    sfs   xs     xa      xr       xv       xf       barline
//            = LOMSE_NEW MyColumnStorage1(0.0f, 0.0f, 0.0f,  200.0f, 1200.0f, 1300.0f, 1600.0f, false);
//        MyColumnLayouter colLyt1(m_libraryScope, &meter, pStorage1);
//        std::vector<ColumnLayouter*> colLayouters;
//        colLayouters.push_back(&colLyt0);
//        colLayouters.push_back(&colLyt1);
//        ShapesStorage shapesStorage;
//
//        MySystemLayouter sysLyt(m_libraryScope, &meter, shapesStorage,
//                                colLayouters, NULL);
//        sysLyt.my_set_page_cursor(1500.0f, 2000.0f);
//        sysLyt.my_set_is_first_column_in_system(true);
//
//        LUnits size = sysLyt.my_determine_column_size(0);
//
//        //cout << "column size = " << size << endl;
//        CHECK( is_equal(size, 1800.0f) );
//    }
//
//    TEST_FIXTURE(SystemLayouterTestFixture, ColumnSize_OtherCol_Case1)
//    {
//        //second column has start hook, but lower than first column end hook.
//        //size of second column increased by dif EndHook(i) - StartHook(i+1)
//
//        ScoreMeter meter(1, 1, 150.0f);
//        MyColumnStorage1* pStorage0
//            //                     x0    sfs    xs    xa     xr       xv       xf       barline
//            = LOMSE_NEW MyColumnStorage1(0.0f, 20.0f, 20.0, 20.0f, 1800.0f, 1800.0f, 2300.0f, false);
//        MyColumnLayouter colLyt0(m_libraryScope, &meter, pStorage0);
//        MyColumnStorage1* pStorage1
//            //                     x0    sfs   xs     xa      xr       xv       xf       barline
//            = LOMSE_NEW MyColumnStorage1(0.0f, 0.0f, 0.0f,  200.0f, 1200.0f, 1300.0f, 1600.0f, false);
//
//        MyColumnLayouter colLyt1(m_libraryScope, &meter, pStorage1);
//        std::vector<ColumnLayouter*> colLayouters;
//        colLayouters.push_back(&colLyt0);
//        colLayouters.push_back(&colLyt1);
//        ShapesStorage shapesStorage;
//
//        MySystemLayouter sysLyt(m_libraryScope, &meter, shapesStorage,
//                                colLayouters, NULL);
//        sysLyt.my_set_page_cursor(1500.0f, 2000.0f);
//        sysLyt.my_set_is_first_column_in_system(false);
//
//        LUnits size = sysLyt.my_determine_column_size(1);
//
//        //cout << "column size = " << size << endl;
//        //cout << "start hook width = " << pStorage1->get_start_hook_width() << endl;
//        CHECK( is_equal(size, 1600.0f) );       //1300 + (500-200)
//    }
//
//    TEST_FIXTURE(SystemLayouterTestFixture, ColumnSize_OtherCol_Case2)
//    {
//        //second column has start hook, greater than first column end hook.
//        //size of second column increased by StartHook
//
//        ScoreMeter meter(1, 1, 150.0f);
//        MyColumnStorage1* pStorage0
//            //                     x0    sfs    xs    xa     xr       xv       xf       barline
//            = LOMSE_NEW MyColumnStorage1(0.0f, 20.0f, 20.0, 20.0f, 1800.0f, 1800.0f, 2000.0f, false);
//        MyColumnLayouter colLyt0(m_libraryScope, &meter, pStorage0);
//        MyColumnStorage1* pStorage1
//            //                     x0    sfs   xs     xa      xr       xv       xf       barline
//            = LOMSE_NEW MyColumnStorage1(0.0f, 0.0f, 0.0f,  300.0f, 1200.0f, 1300.0f, 1600.0f, false);
//        MyColumnLayouter colLyt1(m_libraryScope, &meter, pStorage1);
//        std::vector<ColumnLayouter*> colLayouters;
//        colLayouters.push_back(&colLyt0);
//        colLayouters.push_back(&colLyt1);
//        ShapesStorage shapesStorage;
//
//        MySystemLayouter sysLyt(m_libraryScope, &meter, shapesStorage,
//                                colLayouters, NULL);
//        sysLyt.my_set_page_cursor(1500.0f, 2000.0f);
//        sysLyt.my_set_is_first_column_in_system(false);
//
//        LUnits size = sysLyt.my_determine_column_size(1);
//
//        //cout << "column size = " << size << endl;
//        //cout << "prev end hook width = " << pStorage0->get_end_hook_width() << endl;
//        //cout << "next start hook width = " << pStorage1->get_start_hook_width() << endl;
//        //cout << "next main width = " << pStorage1->get_main_width() << endl;
//
//        CHECK( is_equal(size, 1300.0f) );
//    }
//
//    // system justification -------------------------------------------------------------
//
//    TEST_FIXTURE(SystemLayouterTestFixture, RedistributeSpace_OccupyAllWhenFreeSpace)
//    {
//        ScoreMeter meter(1, 1, 150.0f);
//        MyColumnStorage1* pStorage0 = LOMSE_NEW MyColumnStorage1(0.0f, 1800.0f, 500.0f);
//        MyColumnLayouter colLyt0(m_libraryScope, &meter, pStorage0);
//        MyColumnStorage1* pStorage1 = LOMSE_NEW MyColumnStorage1(850.0f, 1550.0f, 400.0f);
//        MyColumnLayouter colLyt1(m_libraryScope, &meter, pStorage1);
//        std::vector<ColumnLayouter*> colLayouters;
//        colLayouters.push_back(&colLyt0);
//        colLayouters.push_back(&colLyt1);
//        ShapesStorage shapesStorage;
//
//        MySystemLayouter sysLyt(m_libraryScope, &meter, shapesStorage,
//                                colLayouters, NULL);
//        sysLyt.my_set_page_cursor(1500.0f, 2000.0f);
//        sysLyt.my_set_columns_range(0, 2);
//
//        pStorage0->set_trimmed_width(1800.0f);
//        pStorage1->set_trimmed_width(2400.0f);
//        sysLyt.my_set_free_space(420.0f);
//
//        sysLyt.my_redistribute_free_space();
//
//        //cout << "new size 0 = " << sysLyt.my_get_justified_width(0) << endl;
//        //cout << "new size 1 = " << sysLyt.my_get_justified_width(1) << endl;
//        CHECK( is_equal(sysLyt.my_get_justified_width(0), 1980.0f) );
//        CHECK( is_equal(sysLyt.my_get_justified_width(1), 2640.0f) );
//    }
//
//    TEST_FIXTURE(SystemLayouterTestFixture, RedistributeSpace_DoesNothingWhenNoSpace)
//    {
//        ScoreMeter meter(1, 1, 150.0f);
//        MyColumnStorage1* pStorage0 = LOMSE_NEW MyColumnStorage1(0.0f, 1800.0f, 500.0f);
//        MyColumnLayouter colLyt0(m_libraryScope, &meter, pStorage0);
//        MyColumnStorage1* pStorage1 = LOMSE_NEW MyColumnStorage1(850.0f, 1550.0f, 400.0f);
//        MyColumnLayouter colLyt1(m_libraryScope, &meter, pStorage1);
//        std::vector<ColumnLayouter*> colLayouters;
//        colLayouters.push_back(&colLyt0);
//        colLayouters.push_back(&colLyt1);
//        ShapesStorage shapesStorage;
//
//        MySystemLayouter sysLyt(m_libraryScope, &meter, shapesStorage,
//                                colLayouters, NULL);
//        sysLyt.my_set_page_cursor(1500.0f, 2000.0f);
//        sysLyt.my_set_columns_range(0, 2);
//
//        pStorage0->set_trimmed_width(1800.0f);
//        pStorage1->set_trimmed_width(2400.0f);
//        sysLyt.my_set_free_space(0.0f);
//
//        sysLyt.my_redistribute_free_space();
//
////        cout << "new size 0 = " << sysLyt.my_get_trimmed_width(0) << endl;
////        cout << "new size 1 = " << sysLyt.my_get_trimmed_width(1) << endl;
//        CHECK( is_equal(sysLyt.my_get_trimmed_width(0), 1800.0f) );
//        CHECK( is_equal(sysLyt.my_get_trimmed_width(1), 2400.0f) );
//    }
//
//};
//
//

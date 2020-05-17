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
#include "lomse_right_aligner.h"
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "private/lomse_document_p.h"
#include "lomse_im_factory.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// helper, access to protected members
class MyRightAligner : public RightAligner
{
public:
    MyRightAligner()
        : RightAligner()
    {
    }
    virtual ~MyRightAligner() {}

    //access to protected members
    LUnits my_get_width() { return m_width; }
    URect my_get_box(int iBox) { return get_box(iBox); }
    LUnits my_get_touch_x_pos(LUnits y0, LUnits y1) { return get_touch_x_pos(y0, y1); }
    void my_add_border_segment(LUnits y0, LUnits y1, LUnits x) {
        add_border_segment(y0, y1, x);
    }
    list<UPoint>& my_get_border_points() { return m_border; }
    void my_set_width(LUnits w) { m_width = w; }
};


//---------------------------------------------------------------------------------------
class RightAlignerTestFixture
{
public:
    RightAlignerTestFixture()   // setUp()
    {
    }

    ~RightAlignerTestFixture()  // tearDown()
    {
    }

    void cout_rectangle(URect rect)
    {
        cout << UnitTest::CurrentTest::Details()->testName << ": x=" << rect.get_x()
             << ", y=" << rect.get_y() << ", width=" << rect.get_width()
             << ", height=" << rect.get_height() << endl;
    }

    void dump_border(list<UPoint>& border)
    {
        cout << UnitTest::CurrentTest::Details()->testName
             << ": border points (" << border.size() << "):";
        list<UPoint>::iterator it;
        for (it=border.begin(); it!=border.end(); ++it)
            cout << ", (" << (*it).x << "," << (*it).y << ")";
        cout << endl;
    }


};


SUITE(RightAlignerTest)
{
    //-- Building the border function ---------------------------------------------------

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_001)
    {
        //001. boder function is empty
        MyRightAligner a;

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 2 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        CHECK( a.get_total_height() == 0.0f );
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_002)
    {
        //002. add boder segment. full overlap
        MyRightAligner a;

        a.my_add_border_segment(20.0f, 50.0f, 15.0f);

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 4 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        CHECK( a.get_total_height() == 50.0f );
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_003)
    {
        //003. add boder segment. partial overlap top
        MyRightAligner a;
        a.my_add_border_segment(20.0f, 50.0f, 15.0f);

        a.my_add_border_segment(10.0f, 30.0f, 12.0f);

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 5 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(12.0f, 10.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 30.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border( a.my_get_border_points() );
        CHECK( a.get_total_height() == 50.0f );
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_004)
    {
        //004. add boder segment. partial overlap bottom
        MyRightAligner a;
        a.my_add_border_segment(20.0f, 50.0f, 15.0f);

        a.my_add_border_segment(25.0f, 70.0f, 12.0f);

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 5 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(12.0f, 25.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 70.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        CHECK( a.get_total_height() == 70.0f );
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_005)
    {
        //005. add boder segment. one point exists. top
        MyRightAligner a;
        a.my_add_border_segment(20.0f, 50.0f, 15.0f);

        a.my_add_border_segment(20.0f, 30.0f, 12.0f);

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 5 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(12.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 30.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border( a.my_get_border_points() );
        CHECK( a.get_total_height() == 50.0f );
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_006)
    {
        //006. add boder segment. one point exists. bottom
        MyRightAligner a;
        a.my_add_border_segment(20.0f, 50.0f, 15.0f);

        a.my_add_border_segment(25.0f, 50.0f, 12.0f);

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 5 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(12.0f, 25.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border( a.my_get_border_points() );
        CHECK( a.get_total_height() == 50.0f );
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_007)
    {
        //007. add boder segment. both points exist.
        MyRightAligner a;
        a.my_add_border_segment(20.0f, 50.0f, 15.0f);

        a.my_add_border_segment(20.0f, 50.0f, 12.0f);

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 4 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(12.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);
        CHECK( a.get_total_height() == 50.0f );
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_008)
    {
        //008. add boder segment. empty place.
        MyRightAligner a;
        a.my_add_border_segment(20.0f, 50.0f, 15.0f);

        a.my_add_border_segment(60.0f, 80.0f, 45.0f);

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 6 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(45.0f, 60.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 80.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);
        CHECK( a.get_total_height() == 80.0f );
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_009)
    {
        //009. add boder segment. replace several points.
        MyRightAligner a;
        a.my_add_border_segment(10.0f, 20.0f, 25.0f);
        a.my_add_border_segment(20.0f, 30.0f, 35.0f);
        a.my_add_border_segment(30.0f, 50.0f, 15.0f);
        a.my_add_border_segment(60.0f, 70.0f, 35.0f);
        a.my_add_border_segment(80.0f, 90.0f, 15.0f);
        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 10 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(25.0f, 10.0f) );
        ++it;
        CHECK( *it == UPoint(35.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 30.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(35.0f, 60.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 70.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 80.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 90.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);
        CHECK( a.get_total_height() == 90.0f );

        a.my_add_border_segment(12.0f, 85.0f, 5.0f);

        border = a.my_get_border_points();
        CHECK( border.size() == 6 );
        it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(25.0f, 10.0f) );
        ++it;
        CHECK( *it == UPoint(5.0f, 12.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 85.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 90.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);
        CHECK( a.get_total_height() == 90.0f );
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_010)
    {
        //010. add boder segment. replace several points. increase compound height
        MyRightAligner a;
        a.my_add_border_segment(10.0f, 20.0f, 25.0f);
        a.my_add_border_segment(20.0f, 30.0f, 35.0f);
        a.my_add_border_segment(30.0f, 50.0f, 15.0f);
        a.my_add_border_segment(60.0f, 70.0f, 35.0f);
        a.my_add_border_segment(80.0f, 90.0f, 15.0f);
        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 10 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(25.0f, 10.0f) );
        ++it;
        CHECK( *it == UPoint(35.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 30.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(35.0f, 60.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 70.0f) );
        ++it;
        CHECK( *it == UPoint(15.0f, 80.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 90.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);

        a.my_add_border_segment(12.0f, 95.0f, 5.0f);

        border = a.my_get_border_points();
        CHECK( border.size() == 5 );
        it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(25.0f, 10.0f) );
        ++it;
        CHECK( *it == UPoint(5.0f, 12.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 95.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);
        CHECK( a.get_total_height() == 95.0f );
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_011)
    {
        //011. add boder segment. failure in test 042
        MyRightAligner a;
        a.my_add_border_segment(0.0f, 50.0f, 45.0f);
        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 3 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(45.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);

        a.my_add_border_segment(20.0f, 30.0f, 0.0f);

        border = a.my_get_border_points();
        CHECK( border.size() == 5 );
        //dump_border(border);
        it = border.begin();
        CHECK( *it == UPoint(45.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(0.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(45.0f, 30.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        CHECK( a.get_total_height() == 50.0f );
    }

    //-- Using border function to find x for border -------------------------------------

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_021)
    {
        //021. boder function is empty. get right border.
        MyRightAligner a;
        CHECK( is_equal_pos(a.my_get_width(), 0.0f) );
        CHECK( is_equal_pos(a.get_total_height(), 0.0f) );
        a.my_set_width(77.0f);

        LUnits x = a.my_get_touch_x_pos(20.0f, 30.0f);

        CHECK( is_equal_pos(x, 77.0f) );
        //cout << "x for border = " << x << endl;
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_022)
    {
        //022. find x for border. full overlap
        MyRightAligner a;
        a.my_set_width(77.0f);
        a.my_add_border_segment(20.0f, 40.0f, 15.0f);
        a.my_add_border_segment(40.0f, 70.0f, 45.0f);
        a.my_add_border_segment(100.0f, 130.0f, 55.0f);
        //dump_border( a.my_get_border_points() );

        LUnits x = a.my_get_touch_x_pos(50.0f, 60.0f);

        CHECK( is_equal_pos(x, 45.0f) );
        //cout << "x for border = " << x << endl;
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_023)
    {
        //023. find x for border. full overlap. empty place at bottom.
        MyRightAligner a;
        a.my_set_width(77.0f);
        a.my_add_border_segment(20.0f, 40.0f, 15.0f);
        a.my_add_border_segment(40.0f, 70.0f, 45.0f);
        a.my_add_border_segment(100.0f, 130.0f, 55.0f);
        //dump_border( a.my_get_border_points() );

        LUnits x = a.my_get_touch_x_pos(140.0f, 150.0f);

        CHECK( is_equal_pos(x, 77.0f) );
        //cout << "x for border = " << x << endl;
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_024)
    {
        //024. find x for border. partial overlap. one intermediate point
        MyRightAligner a;
        a.my_set_width(77.0f);
        a.my_add_border_segment(20.0f, 40.0f, 15.0f);
        a.my_add_border_segment(40.0f, 70.0f, 45.0f);
        a.my_add_border_segment(100.0f, 130.0f, 55.0f);
        //dump_border( a.my_get_border_points() );

        LUnits x = a.my_get_touch_x_pos(30.0f, 60.0f);

        CHECK( is_equal_pos(x, 15.0f) );
        //cout << "x for border = " << x << endl;
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_025)
    {
        //025. find x for border. partial overlap. many intermediate points
        MyRightAligner a;
        a.my_set_width(77.0f);
        a.my_add_border_segment(20.0f, 40.0f, 15.0f);
        a.my_add_border_segment(40.0f, 70.0f, 45.0f);
        a.my_add_border_segment(100.0f, 130.0f, 55.0f);
        //dump_border( a.my_get_border_points() );

        LUnits x = a.my_get_touch_x_pos(10.0f, 140.0f);

        CHECK( is_equal_pos(x, 15.0f) );
        //cout << "x for border = " << x << endl;
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_026)
    {
        //026. find x for border. full overlap. empty place at top.
        MyRightAligner a;
        a.my_set_width(77.0f);
        a.my_add_border_segment(20.0f, 40.0f, 15.0f);
        a.my_add_border_segment(40.0f, 70.0f, 45.0f);
        a.my_add_border_segment(100.0f, 130.0f, 55.0f);
        //dump_border( a.my_get_border_points() );

        LUnits x = a.my_get_touch_x_pos(10.0f, 15.0f);

        CHECK( is_equal_pos(x, 77.0f) );
        //cout << "x for border = " << x << endl;
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_027)
    {
        //027. find x for border. partial overlap. empty place at top.
        MyRightAligner a;
        a.my_set_width(77.0f);
        a.my_add_border_segment(20.0f, 40.0f, 15.0f);
        a.my_add_border_segment(40.0f, 70.0f, 45.0f);
        a.my_add_border_segment(100.0f, 130.0f, 55.0f);
        //dump_border( a.my_get_border_points() );

        LUnits x = a.my_get_touch_x_pos(80.0f, 120.0f);

        CHECK( is_equal_pos(x, 55.0f) );
        //cout << "x for border = " << x << endl;
    }

    //-- The aligner works as expected --------------------------------------------------

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_041)
    {
        //041. first box determines initial composite box (= Case 3)
        MyRightAligner a;
        CHECK( is_equal_pos(a.my_get_width(), 0.0f) );
        CHECK( is_equal_pos(a.get_total_height(), 0.0f) );
        URect box1(10.0f, 0.0f, 10.0f, 50.0f);

        int iBox = a.add_box(box1);

        CHECK( iBox == 0 );
        CHECK( a.my_get_box(0) == URect(0.0f, 0.0f, 10.0f, 50.0f) );
        //cout_rectangle(a.my_get_box(0));
        CHECK( is_equal_pos(a.my_get_width(), 10.0f) );
        CHECK( is_equal_pos(a.get_total_height(), 50.0f) );
        //cout << "RightAligner_041" << ": h=" << a.get_total_height()
        //     << ": w=" << a.my_get_width() << endl;

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 3 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(0.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_042)
    {
        //042. case 1: move existing boxes to the right
        MyRightAligner a;
        CHECK( is_equal_pos(a.my_get_width(), 0.0f) );
        CHECK( is_equal_pos(a.get_total_height(), 0.0f) );
        URect box1(0.0f, 0.0f, 10.0f, 50.0f);
        a.add_box(box1);
        //dump_border(a.my_get_border_points());

        URect box2(0.0f, 20.0f, 45.0f, 10.0f);
        int iBox = a.add_box(box2);

        CHECK( iBox == 1 );
        CHECK( a.my_get_box(0) == URect(45.0f, 0.0f, 10.0f, 50.0f) );
        CHECK( a.my_get_box(1) == URect(0.0f, 20.0f, 45.0f, 10.0f) );
        //cout_rectangle(a.my_get_box(0));
        CHECK( is_equal_pos(a.my_get_width(), 55.0f) );
        CHECK( is_equal_pos(a.get_total_height(), 50.0f) );

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 5 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(45.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(0.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(45.0f, 30.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_043)
    {
        //043. case 2: move new one to the right to touch existing boxes.
        MyRightAligner a;
        URect box1(0.0f, 0.0f, 10.0f, 50.0f);
        a.add_box(box1);
        URect box2(0.0f, 20.0f, 45.0f, 10.0f);
        a.add_box(box2);

        URect box3(0.0f, 40.0f, 22.0f, 10.0f);
        int iBox = a.add_box(box3);

        CHECK( iBox == 2 );
        CHECK( a.my_get_box(0) == URect(45.0f, 0.0f, 10.0f, 50.0f) );
        CHECK( a.my_get_box(1) == URect(0.0f, 20.0f, 45.0f, 10.0f) );
        CHECK( a.my_get_box(2) == URect(23.0f, 40.0f, 22.0f, 10.0f) );
        //cout_rectangle(a.my_get_box(2));
        CHECK( is_equal_pos(a.my_get_width(), 55.0f) );
        CHECK( is_equal_pos(a.get_total_height(), 50.0f) );

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 6 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(45.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(0.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(45.0f, 30.0f) );
        ++it;
        CHECK( *it == UPoint(23.0f, 40.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_044)
    {
        //044. case 3: new in empty place. shift existing boxes to the right.
        MyRightAligner a;
        URect box1(0.0f, 30.0f, 10.0f, 50.0f);
        a.add_box(box1);
        URect box2(0.0f, 50.0f, 45.0f, 10.0f);
        a.add_box(box2);

        URect box3(0.0f, 10.0f, 65.0f, 10.0f);
        int iBox = a.add_box(box3);

        CHECK( iBox == 2 );
        CHECK( a.my_get_box(0) == URect(55.0f, 30.0f, 10.0f, 50.0f) );
        CHECK( a.my_get_box(1) == URect(10.0f, 50.0f, 45.0f, 10.0f) );
        CHECK( a.my_get_box(2) == URect(0.0f, 10.0f, 65.0f, 10.0f) );
        //cout_rectangle(a.my_get_box(2));
        CHECK( is_equal_pos(a.my_get_width(), 65.0f) );
        CHECK( is_equal_pos(a.get_total_height(), 80.0f) );

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 8 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(0.0f, 10.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(55.0f, 30.0f) );
        ++it;
        CHECK( *it == UPoint(10.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(55.0f, 60.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 80.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);
    }

    TEST_FIXTURE(RightAlignerTestFixture, RightAligner_045)
    {
        //045. case 4: new in empty place. shift it to right margin.
        MyRightAligner a;
        URect box1(0.0f, 30.0f, 10.0f, 50.0f);
        a.add_box(box1);
        URect box2(0.0f, 50.0f, 45.0f, 10.0f);
        a.add_box(box2);

        URect box3(0.0f, 10.0f, 32.0f, 10.0f);
        int iBox = a.add_box(box3);

        CHECK( iBox == 2 );
        CHECK( a.my_get_box(0) == URect(45.0f, 30.0f, 10.0f, 50.0f) );
        CHECK( a.my_get_box(1) == URect(0.0f, 50.0f, 45.0f, 10.0f) );
        CHECK( a.my_get_box(2) == URect(23.0f, 10.0f, 32.0f, 10.0f) );
        //cout_rectangle(a.my_get_box(2));
        CHECK( is_equal_pos(a.my_get_width(), 55.0f) );
        CHECK( is_equal_pos(a.get_total_height(), 80.0f) );

        list<UPoint>& border = a.my_get_border_points();
        CHECK( border.size() == 8 );
        list<UPoint>::iterator it = border.begin();
        CHECK( *it == UPoint(-1.0f, 0.0f) );
        ++it;
        CHECK( *it == UPoint(23.0f, 10.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 20.0f) );
        ++it;
        CHECK( *it == UPoint(45.0f, 30.0f) );
        ++it;
        CHECK( *it == UPoint(0.0f, 50.0f) );
        ++it;
        CHECK( *it == UPoint(45.0f, 60.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, 80.0f) );
        ++it;
        CHECK( *it == UPoint(-1.0f, -1.0f) );
        //dump_border(border);
    }

};

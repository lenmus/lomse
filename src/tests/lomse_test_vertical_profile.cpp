//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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
#include "lomse_vertical_profile.h"
#include "lomse_shape_note.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
// helper, access to protected members
class MyVerticalProfile : public VerticalProfile
{
public:
    MyVerticalProfile(LUnits xStart, LUnits xEnd, int numStaves)
        : VerticalProfile(xStart, xEnd, numStaves)
    {
    }

    virtual ~MyVerticalProfile() {}

    inline int my_get_num_staves() { return m_numStaves; }

    inline size_t my_x_min_size(int idxStaff) { return m_xMin[idxStaff]->size(); }
    inline size_t my_x_max_size(int idxStaff) { return m_xMax[idxStaff]->size(); }
    inline list<VProfilePoint>* my_xMin(int idxStaff) { return m_xMin[idxStaff]; }
    inline list<VProfilePoint>* my_xMax(int idxStaff) { return m_xMax[idxStaff]; }
    VProfilePoint my_xMin(int idxStaff, int i)
    {
        list<VProfilePoint>::iterator it = m_xMin[idxStaff]->begin();
        std::advance(it, i);
        return *it;
    }
    inline VProfilePoint my_xMax(int idxStaff, int i)
    {
        list<VProfilePoint>::iterator it = m_xMax[idxStaff]->begin();
        std::advance(it, i);
        return *it;
    }

    string dump_points(list<VProfilePoint>* pPoints, int idxStaff)
    {
        stringstream msg;
        msg << "size = " << pPoints->size() << endl;
        PointsIterator it= pPoints->begin();
        int i = 0;
        for (; it != pPoints->end(); ++it, ++i)
        {
            msg << "point(" << idxStaff << ", " << i << ") = {" << (*it).x << ", "
                << (*it).y << ", " << (void*)((*it).shape) << "}" << endl;
        }
        return msg.str();
    }
};


//=======================================================================================
class VerticalProfileTestFixture
{
public:
    LibraryScope m_libraryScope;
    string m_scores_path;

    VerticalProfileTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~VerticalProfileTestFixture()    //TearDown fixture
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    void dump_point(const string& prefix, VProfilePoint pt)
    {
        cout << test_name() << ". point" << prefix << " = {" << pt.x << ", "
             << pt.y << ", " << (void*)pt.shape << "}" << endl;
    }

};

SUITE(VerticalProfileTest)
{

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_001)
    {
        //@001 Vertical profile creation
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);

        CHECK( vp.my_get_num_staves() == 2 );

        LUnits yTop = 3000.0f;
        LUnits yBottom = 3400.0f;
        vp.initialize(0, yTop, yBottom);
//        vp.initialize(1, yTop, yBottom);

        CHECK( vp.get_min_limit(0) == LOMSE_PAPER_UPPER_LIMIT );
        CHECK( vp.get_max_limit(0) == LOMSE_PAPER_LOWER_LIMIT );
//        CHECK( vp.get_min_limit(1) == LOMSE_PAPER_LOWER_LIMIT );
//        CHECK( vp.get_max_limit(1) == LOMSE_PAPER_LOWER_LIMIT );

        CHECK( vp.my_x_min_size(0) == 2 );
        CHECK( vp.my_x_max_size(0) == 2 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_010)
    {
        //@010 get_max_for(): empty profile
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        LUnits yTop = 3000.0f;
        LUnits yBottom = 3400.0f;
        vp.initialize(0, yTop, yBottom);
        vp.initialize(1, yTop, yBottom);

        CHECK( vp.get_max_for(2000.0f, 3000.0f, 0) == LOMSE_PAPER_LOWER_LIMIT );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_020)
    {
        //@020 get_min_for(): empty profile
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        LUnits yTop = 3000.0f;
        LUnits yBottom = 3400.0f;
        vp.initialize(0, yTop, yBottom);
        vp.initialize(1, yTop, yBottom);

        CHECK( vp.get_min_for(2000.0f, 3000.0f, 0) == LOMSE_PAPER_UPPER_LIMIT );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_030)
    {
        //@030 update(): empty profile
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        vp.initialize(0, 3000.0f, 3400.0f);
        vp.initialize(1, 3000.0f, 3400.0f);

        GmoShapeRectangle shape(nullptr);
        shape.set_origin(3400.0f, 1500.0f);
        shape.set_height(2500.0f);
        shape.set_width(5000.0f);
        vp.update(&shape, 0);

//        cout << test_name() << ".  Dump of xMin:" << endl;
//        cout << vp.dump_points(vp.my_xMin(0), 0) << endl;
        CHECK( vp.my_x_min_size(0) == 4 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(3400.0f, 1500.0f, &shape) );
        CHECK( vp.my_xMin(0, 2) == VProfilePoint(8400.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 3) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );

//        cout << test_name() << ".  Dump of xMax:" << endl;
//        cout << vp.dump_points(vp.my_xMax(0), 0) << endl;
        CHECK( vp.my_x_max_size(0) == 4 );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(3400.0f, 4000.0f, &shape) );
        CHECK( vp.my_xMax(0, 2) == VProfilePoint(8400.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 3) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );

        CHECK( vp.get_min_limit(0) == 1500.0f );
        CHECK( vp.get_max_limit(0) == 4000.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_031)
    {
        //@031 update(): partial overlap of existing shape
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        vp.initialize(0, 3000.0f, 3400.0f);
        vp.initialize(1, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(9000.0f, 1500.0f);
        shape1.set_height(2500.0f);
        shape1.set_width(1000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(8500.0f, 4000.0f);
        shape2.set_height(2500.0f);
        shape2.set_width(1000.0f);
        vp.update(&shape2, 0);

//        cout << test_name() << ".  Dump of xMin:" << endl;
//        cout << vp.dump_points(vp.my_xMin(0), 0) << endl;
        CHECK( vp.my_x_min_size(0) == 5 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(8500.0f, 4000.0f, &shape2) );
        CHECK( vp.my_xMin(0, 2) == VProfilePoint(9000.0f, 1500.0f, &shape1) );
        CHECK( vp.my_xMin(0, 3) == VProfilePoint(10000.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 4) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );

//        cout << test_name() << ".  Dump of xMax:" << endl;
//        cout << vp.dump_points(vp.my_xMax(0), 0) << endl;
        CHECK( vp.my_x_max_size(0) == 5 );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(8500.0f, 6500.0f, &shape2) );
        CHECK( vp.my_xMax(0, 2) == VProfilePoint(9500.0f, 4000.0f, &shape1) );
        CHECK( vp.my_xMax(0, 3) == VProfilePoint(10000.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 4) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );

        CHECK( vp.get_min_limit(0) == 1500.0f );
        CHECK( vp.get_max_limit(0) == 6500.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_032)
    {
        //@032 update(): full overlap of existing shape
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        vp.initialize(0, 3000.0f, 3400.0f);
        vp.initialize(1, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(9000.0f, 1500.0f);
        shape1.set_height(2500.0f);
        shape1.set_width(1000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(8500.0f, 4000.0f);
        shape2.set_height(2500.0f);
        shape2.set_width(2000.0f);
        vp.update(&shape2, 0);

//        cout << test_name() << ".  Dump of xMin:" << endl;
//        cout << vp.dump_points(vp.my_xMin(0), 0) << endl;
        CHECK( vp.my_x_min_size(0) == 6 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(8500.0f, 4000.0f, &shape2) );
        CHECK( vp.my_xMin(0, 2) == VProfilePoint(9000.0f, 1500.0f, &shape1) );
        CHECK( vp.my_xMin(0, 3) == VProfilePoint(10000.0f, 4000.0f, &shape2) );
        CHECK( vp.my_xMin(0, 4) == VProfilePoint(10500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 5) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );

//        cout << test_name() << ".  Dump of xMax:" << endl;
//        cout << vp.dump_points(vp.my_xMax(0), 0) << endl;
        CHECK( vp.my_x_max_size(0) == 4 );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(8500.0f, 6500.0f, &shape2) );
        CHECK( vp.my_xMax(0, 2) == VProfilePoint(10500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 3) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );

        CHECK( vp.get_min_limit(0) == 1500.0f );
        CHECK( vp.get_max_limit(0) == 6500.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_033)
    {
        //@033 update(): full overlap of existing shape. Opposite side
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        vp.initialize(0, 3000.0f, 3400.0f);
        vp.initialize(1, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(9000.0f, 5000.0f);
        shape1.set_height(2000.0f);
        shape1.set_width(1000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(8500.0f, 1500.0f);
        shape2.set_height(2500.0f);
        shape2.set_width(3500.0f);
        vp.update(&shape2, 0);

//        cout << test_name() << ".  Dump of xMin:" << endl;
//        cout << vp.dump_points(vp.my_xMin(0), 0) << endl;
        CHECK( vp.my_x_min_size(0) == 4 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(8500.0f, 1500.0f, &shape2) );
        CHECK( vp.my_xMin(0, 2) == VProfilePoint(12000.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 3) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );

//        cout << test_name() << ".  Dump of xMax:" << endl;
//        cout << vp.dump_points(vp.my_xMax(0), 0) << endl;
        CHECK( vp.my_x_max_size(0) == 6 );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(8500.0f, 4000.0f, &shape2) );
        CHECK( vp.my_xMax(0, 2) == VProfilePoint(9000.0f, 7000.0f, &shape1) );
        CHECK( vp.my_xMax(0, 3) == VProfilePoint(10000.0f, 4000.0f, &shape2) );
        CHECK( vp.my_xMax(0, 4) == VProfilePoint(12000.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 5) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );

        CHECK( vp.get_min_limit(0) == 1500.0f );
        CHECK( vp.get_max_limit(0) == 7000.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_034)
    {
        //@034 update(): partial overlap of existing shape. Opposite side
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        vp.initialize(0, 3000.0f, 3400.0f);
        vp.initialize(1, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(9000.0f, 4000.0f);
        shape1.set_height(1000.0f);
        shape1.set_width(3000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(8000.0f, 2000.0f);
        shape2.set_height(1000.0f);
        shape2.set_width(2000.0f);
        vp.update(&shape2, 0);

//        cout << test_name() << ".  Dump of xMin:" << endl;
//        cout << vp.dump_points(vp.my_xMin(0), 0) << endl;
        CHECK( vp.my_x_min_size(0) == 5 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(8000.0f, 2000.0f, &shape2) );
        CHECK( vp.my_xMin(0, 2) == VProfilePoint(10000.0f, 4000.0f, &shape1) );
        CHECK( vp.my_xMin(0, 3) == VProfilePoint(12000.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 4) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );

//        cout << test_name() << ".  Dump of xMax:" << endl;
//        cout << vp.dump_points(vp.my_xMax(0), 0) << endl;
        CHECK( vp.my_x_max_size(0) == 5 );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(8000.0f, 3000.0f, &shape2) );
        CHECK( vp.my_xMax(0, 2) == VProfilePoint(9000.0f, 5000.0f, &shape1) );
        CHECK( vp.my_xMax(0, 3) == VProfilePoint(12000.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 4) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );

        CHECK( vp.get_min_limit(0) == 2000.0f );
        CHECK( vp.get_max_limit(0) == 5000.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_035)
    {
        //@035 update(): partial overlap of existing shape.
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        vp.initialize(0, 3000.0f, 3400.0f);
        vp.initialize(1, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(8000.0f, 2000.0f);
        shape1.set_height(1000.0f);
        shape1.set_width(2000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(9000.0f, 4000.0f);
        shape2.set_height(1000.0f);
        shape2.set_width(3000.0f);
        vp.update(&shape2, 0);

//        cout << test_name() << ".  Dump of xMin:" << endl;
//        cout << vp.dump_points(vp.my_xMin(0), 0) << endl;
        CHECK( vp.my_x_min_size(0) == 5 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(8000.0f, 2000.0f, &shape1) );
        CHECK( vp.my_xMin(0, 2) == VProfilePoint(10000.0f, 4000.0f, &shape2) );
        CHECK( vp.my_xMin(0, 3) == VProfilePoint(12000.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 4) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );

//        cout << test_name() << ".  Dump of xMax:" << endl;
//        cout << vp.dump_points(vp.my_xMax(0), 0) << endl;
        CHECK( vp.my_x_max_size(0) == 5 );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(8000.0f, 3000.0f, &shape1) );
        CHECK( vp.my_xMax(0, 2) == VProfilePoint(9000.0f, 5000.0f, &shape2) );
        CHECK( vp.my_xMax(0, 3) == VProfilePoint(12000.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 4) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );

        CHECK( vp.get_min_limit(0) == 2000.0f );
        CHECK( vp.get_max_limit(0) == 5000.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_036)
    {
        //@036 update(): intermediate point, case 2.1.1
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        vp.initialize(0, 3000.0f, 3400.0f);
        vp.initialize(1, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(8000.0f, 2000.0f);
        shape1.set_height(1000.0f);
        shape1.set_width(2000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(9000.0f, 4000.0f);
        shape2.set_height(1000.0f);
        shape2.set_width(3000.0f);
        vp.update(&shape2, 0);

        GmoShapeRectangle shape3(nullptr);
        shape3.set_origin(8500.0f, 6000.0f);
        shape3.set_height(1000.0f);
        shape3.set_width(5500.0f);
        vp.update(&shape3, 0);

//        cout << test_name() << ".  Dump of xMin:" << endl;
//        cout << vp.dump_points(vp.my_xMin(0), 0) << endl;
        CHECK( vp.my_x_min_size(0) == 6 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(8000.0f, 2000.0f, &shape1) );
        CHECK( vp.my_xMin(0, 2) == VProfilePoint(10000.0f, 4000.0f, &shape2) );
        CHECK( vp.my_xMin(0, 3) == VProfilePoint(12000.0f, 6000.0f, &shape3) );
        CHECK( vp.my_xMin(0, 4) == VProfilePoint(14000.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 5) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );

//        cout << test_name() << ".  Dump of xMax:" << endl;
//        cout << vp.dump_points(vp.my_xMax(0), 0) << endl;
        CHECK( vp.my_x_max_size(0) == 5 );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(8000.0f, 3000.0f, &shape1) );
        CHECK( vp.my_xMax(0, 2) == VProfilePoint(8500.0f, 7000.0f, &shape3) );
        CHECK( vp.my_xMax(0, 3) == VProfilePoint(14000.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 4) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );

        CHECK( vp.get_min_limit(0) == 2000.0f );
        CHECK( vp.get_max_limit(0) == 7000.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_037)
    {
        //@037 update(): intermediate point, case 2.1.2
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        vp.initialize(0, 3000.0f, 3400.0f);
        vp.initialize(1, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(8000.0f, 2000.0f);
        shape1.set_height(1000.0f);
        shape1.set_width(2000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(9000.0f, 6000.0f);
        shape2.set_height(1000.0f);
        shape2.set_width(3000.0f);
        vp.update(&shape2, 0);

        GmoShapeRectangle shape3(nullptr);
        shape3.set_origin(8500.0f, 4000.0f);
        shape3.set_height(1000.0f);
        shape3.set_width(5500.0f);
        vp.update(&shape3, 0);

//        cout << test_name() << ".  Dump of xMin:" << endl;
//        cout << vp.dump_points(vp.my_xMin(0), 0) << endl;
        CHECK( vp.my_x_min_size(0) == 5 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(8000.0f, 2000.0f, &shape1) );
        CHECK( vp.my_xMin(0, 2) == VProfilePoint(10000.0f, 4000.0f, &shape3) );
        CHECK( vp.my_xMin(0, 3) == VProfilePoint(14000.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 4) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );

//        cout << test_name() << ".  Dump of xMax:" << endl;
//        cout << vp.dump_points(vp.my_xMax(0), 0) << endl;
        CHECK( vp.my_x_max_size(0) == 7 );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(8000.0f, 3000.0f, &shape1) );
        CHECK( vp.my_xMax(0, 2) == VProfilePoint(8500.0f, 5000.0f, &shape3) );
        CHECK( vp.my_xMax(0, 3) == VProfilePoint(9000.0f, 7000.0f, &shape2) );
        CHECK( vp.my_xMax(0, 4) == VProfilePoint(12000.0f, 5000.0f, &shape3) );
        CHECK( vp.my_xMax(0, 5) == VProfilePoint(14000.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 6) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );

        CHECK( vp.get_min_limit(0) == 2000.0f );
        CHECK( vp.get_max_limit(0) == 7000.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_038)
    {
        //@038 update(): exact overlap
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        vp.initialize(0, 3000.0f, 3400.0f);
        vp.initialize(1, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(9000.0f, 4000.0f);
        shape1.set_height(1000.0f);
        shape1.set_width(3000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(9000.0f, 2000.0f);
        shape2.set_height(1000.0f);
        shape2.set_width(3000.0f);
        vp.update(&shape2, 0);

//        cout << test_name() << ".  Dump of xMin:" << endl;
//        cout << vp.dump_points(vp.my_xMin(0), 0) << endl;
        CHECK( vp.my_x_min_size(0) == 4 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(9000.0f, 2000.0f, &shape2) );
        CHECK( vp.my_xMin(0, 2) == VProfilePoint(12000.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 3) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );

//        cout << test_name() << ".  Dump of xMax:" << endl;
//        cout << vp.dump_points(vp.my_xMax(0), 0) << endl;
        CHECK( vp.my_x_max_size(0) == 4 );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(9000.0f, 5000.0f, &shape1) );
        CHECK( vp.my_xMax(0, 2) == VProfilePoint(12000.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 3) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );

        CHECK( vp.get_min_limit(0) == 2000.0f );
        CHECK( vp.get_max_limit(0) == 5000.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_039)
    {
        //@039 update(): shape out of range do not update the profile
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 2);
        vp.initialize(0, 3000.0f, 3400.0f);
        vp.initialize(1, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(9000.0f, 4000.0f);
        shape1.set_height(1000.0f);
        shape1.set_width(3000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(0.0f, 0.0f);
        shape2.set_height(1000.0f);
        shape2.set_width(3000.0f);
        vp.update(&shape2, 0);

//        cout << test_name() << ".  Dump of xMin:" << endl;
//        cout << vp.dump_points(vp.my_xMin(0), 0) << endl;
        CHECK( vp.my_x_min_size(0) == 4 );
        CHECK( vp.my_xMin(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 1) == VProfilePoint(9000.0f, 4000.0f, &shape1) );
        CHECK( vp.my_xMin(0, 2) == VProfilePoint(12000.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );
        CHECK( vp.my_xMin(0, 3) == VProfilePoint(19500.0f, LOMSE_PAPER_UPPER_LIMIT, nullptr) );

//        cout << test_name() << ".  Dump of xMax:" << endl;
//        cout << vp.dump_points(vp.my_xMax(0), 0) << endl;
        CHECK( vp.my_x_max_size(0) == 4 );
        CHECK( vp.my_xMax(0, 0) == VProfilePoint(1500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 1) == VProfilePoint(9000.0f, 5000.0f, &shape1) );
        CHECK( vp.my_xMax(0, 2) == VProfilePoint(12000.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );
        CHECK( vp.my_xMax(0, 3) == VProfilePoint(19500.0f, LOMSE_PAPER_LOWER_LIMIT, nullptr) );

        CHECK( vp.get_min_limit(0) == 4000.0f );
        CHECK( vp.get_max_limit(0) == 5000.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_100)
    {
        //@100 get_max_for()
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 1);
        vp.initialize(0, 3000.0f, 3400.0f);

        GmoShapeRectangle shape(nullptr);
        shape.set_origin(3400.0f, 1500.0f);
        shape.set_height(2500.0f);
        shape.set_width(5000.0f);
        vp.update(&shape, 0);

        CHECK( vp.get_max_for(3800.0f, 6300.0f, 0) == 4000.0f );
        CHECK( vp.get_max_for(2500.0f, 3100.0f, 0) == LOMSE_PAPER_LOWER_LIMIT );
//        cout << "get_max_for(3800.0f, 6300.0f) = " << vp.get_max_for(3800.0f, 6300.0f, 0)
//             << ", get_max_for(2500.0f, 3100.0f) = " << vp.get_max_for(2500.0f, 3100.0f, 0) << endl;

    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_101)
    {
        //@101 get_max_for(). Interval exactly equal than shape
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 1);
        vp.initialize(0, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(2000.0f, 1500.0f);
        shape1.set_height(1000.0f);
        shape1.set_width(7000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(3400.0f, 3500.0f);
        shape2.set_height(2500.0f);
        shape2.set_width(5000.0f);
        vp.update(&shape2, 0);

        CHECK( vp.get_max_for(3400.0f, 8400.0f, 0) == 6000.0f );

    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_102)
    {
        //@102 get_min_for(). Interval exactly equal than shape
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        MyVerticalProfile vp(xStart, xEnd, 1);
        vp.initialize(0, 3000.0f, 3400.0f);

        GmoShapeRectangle shape1(nullptr);
        shape1.set_origin(3400.0f, 1500.0f);
        shape1.set_height(1000.0f);
        shape1.set_width(5000.0f);
        vp.update(&shape1, 0);

        GmoShapeRectangle shape2(nullptr);
        shape2.set_origin(2000.0f, 3500.0f);
        shape2.set_height(2500.0f);
        shape2.set_width(7000.0f);
        vp.update(&shape2, 0);

        CHECK( vp.get_min_for(3400.0f, 8400.0f, 0) == 1500.0f );
    }


};



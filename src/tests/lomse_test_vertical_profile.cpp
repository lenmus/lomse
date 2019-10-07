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
    MyVerticalProfile(LUnits xStart, LUnits xEnd, LUnits cellWidth, int numStaves)
        : VerticalProfile(xStart, xEnd, cellWidth, numStaves)
    {
    }

    virtual ~MyVerticalProfile() {}

    inline int my_get_num_staves() { return m_numStaves; }
    inline int my_get_num_cells() { return m_numCells; }
//    LUnits m_xStart;        //sytem left side
//    LUnits m_xEnd;          //system right side

	inline vector<CellsRow*>& my_get_vector_max() { return m_yMax; }
	inline vector<CellsRow*>& my_get_vector_min() { return m_yMin; }
    inline int my_cell_index(LUnits xPos) { return cell_index(xPos); }
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
};

SUITE(VerticalProfileTest)
{

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_01)
    {
        //@01 Vertical profile creation
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        LUnits cellWidth = 200.0f;      //2 mm
        MyVerticalProfile vp(xStart, xEnd, cellWidth, 2);

        CHECK( vp.my_get_num_staves() == 2 );
        CHECK( vp.my_get_num_cells() == int((xEnd - xStart) / cellWidth) );     //90
        CHECK( vp.my_get_num_cells() == 90 );

        LUnits yTop = 3000.0f;
        LUnits yBottom = 3400.0f;
        vp.initialize(0, yTop, yBottom);

        bool fOkMax = true;
        bool fOkMin = true;
        LUnits yCenter = (yTop + yBottom) / 2.0f;
        for (size_t j=0; j < 90; ++j)
        {
            fOkMax &= (vp.get_max_cell(j, 0) == yCenter);
            fOkMin &= (vp.get_min_cell(j, 0) == yCenter);
//            cout << test_name() << ", j=" << j << ", max="
//                 << vp.get_max_cell(j, 0) << ", min="
//                 << vp.get_min_cell(j, 0) << endl;
        }
        CHECK (fOkMax == true );
        CHECK (fOkMin == true );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_02)
    {
        //@02 cell index: xPos in third cell (iCell==2)
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        LUnits cellWidth = 200.0f;      //2 mm
        MyVerticalProfile vp(xStart, xEnd, cellWidth, 2);

        LUnits xPos = xStart + 3.5f * cellWidth;
        CHECK( vp.my_cell_index(xPos) == 3 );
//        cout << test_name() << ", index=" << vp.my_cell_index(xPos)
//             << ", numCells=" << vp.my_get_num_cells() << endl;
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_03)
    {
        //@03 cell index: xPos at system left
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        LUnits cellWidth = 200.0f;      //2 mm
        MyVerticalProfile vp(xStart, xEnd, cellWidth, 2);

        LUnits xPos = xStart + 0.05f;
        CHECK( vp.my_cell_index(xPos) == 0 );
//        cout << test_name() << ", index=" << vp.my_cell_index(xPos)
//             << ", numCells=" << vp.my_get_num_cells() << endl;
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_04)
    {
        //@04 cell index: xPos lower than system left
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        LUnits cellWidth = 200.0f;      //2 mm
        MyVerticalProfile vp(xStart, xEnd, cellWidth, 2);

        LUnits xPos = xStart - 0.05f;
        CHECK( vp.my_cell_index(xPos) == 0 );
//        cout << test_name() << ", index=" << vp.my_cell_index(xPos)
//             << ", numCells=" << vp.my_get_num_cells() << endl;
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_05)
    {
        //@05 cell index: xPos at system right
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        LUnits cellWidth = 200.0f;      //2 mm
        MyVerticalProfile vp(xStart, xEnd, cellWidth, 2);

        LUnits xPos = xEnd - 0.05f;
        CHECK( vp.my_cell_index(xPos) == vp.my_get_num_cells() - 1 );
//        cout << test_name() << ", index=" << vp.my_cell_index(xPos)
//             << ", numCells=" << vp.my_get_num_cells() << endl;
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_06)
    {
        //@06 cell index: xPos greater than system right
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        LUnits cellWidth = 200.0f;      //2 mm
        MyVerticalProfile vp(xStart, xEnd, cellWidth, 2);

        LUnits xPos = 22000.0f;
        CHECK( vp.my_cell_index(xPos) == vp.my_get_num_cells() - 1 );
//        cout << test_name() << ", index=" << vp.my_cell_index(xPos)
//             << ", numCells=" << vp.my_get_num_cells() << endl;
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_07)
    {
        //@07 cell index: xPos in center of last cell
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        LUnits cellWidth = 200.0f;      //2 mm
        MyVerticalProfile vp(xStart, xEnd, cellWidth, 2);

        LUnits xPos = xEnd - cellWidth/2.0f;
        CHECK( vp.my_cell_index(xPos) == vp.my_get_num_cells() - 1 );
//        cout << test_name() << ", index=" << vp.my_cell_index(xPos)
//             << ", numCells=" << vp.my_get_num_cells() << endl;
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_08)
    {
        //@08 update ok
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        LUnits cellWidth = 200.0f;      //2 mm
        MyVerticalProfile vp(xStart, xEnd, cellWidth, 2);
        vp.initialize(0, 3000.0f, 3400.0f);

        GmoShapeRectangle shape(nullptr);
        shape.set_origin(3400.0f, 1500.0f);
        shape.set_height(2500.0f);
        shape.set_width(5000.0f);
        vp.update(&shape, 0);

        bool fOkMax = true;
        bool fOkMin = true;
        for (size_t j=0; j < 9; ++j)
        {
            fOkMax &= (vp.get_max_cell(j, 0) == 3200.0f);
            fOkMin &= (vp.get_min_cell(j, 0) == 3200.0f);
//            cout << test_name() << ", j=" << j << ", min=" << vp.get_min_cell(j, 0)
//                 << ", max=" << vp.get_max_cell(j, 0) << endl;
        }
//        cout << test_name() << "--------------------------------------------" << endl;
        CHECK( fOkMax == true );
        CHECK( fOkMin == true );

        fOkMax = true;
        fOkMin = true;
        for (size_t j=9; j < 35; ++j)
        {
            fOkMax &= (vp.get_max_cell(j, 0) == 4000.0f);
            fOkMin &= (vp.get_min_cell(j, 0) == 1500.0f);
//            cout << test_name() << ", j=" << j << ", min=" << vp.get_min_cell(j, 0)
//                 << ", max=" << vp.get_max_cell(j, 0) << endl;
        }
//        cout << test_name() << "--------------------------------------------" << endl;
        CHECK( fOkMax == true );
        CHECK( fOkMin == true );

        fOkMax = true;
        fOkMin = true;
        for (size_t j=35; j < 90; ++j)
        {
            fOkMax &= (vp.get_max_cell(j, 0) == 3200.0f);
            fOkMin &= (vp.get_min_cell(j, 0) == 3200.0f);
//            cout << test_name() << ", j=" << j << ", min=" << vp.get_min_cell(j, 0)
//                 << ", max=" << vp.get_max_cell(j, 0) << endl;
        }
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_09)
    {
        //@09 get_max_for()
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        LUnits cellWidth = 200.0f;      //2 mm
        MyVerticalProfile vp(xStart, xEnd, cellWidth, 2);
        vp.initialize(0, 3000.0f, 3400.0f);

        GmoShapeRectangle shape(nullptr);
        shape.set_origin(3400.0f, 1500.0f);
        shape.set_height(2500.0f);
        shape.set_width(5000.0f);
        vp.update(&shape, 0);

        CHECK( vp.get_max_for(3800.0f, 6300.0f, 0) == 4000.0f );
        CHECK( vp.get_max_for(2500.0f, 3100.0f, 0) == 3200.0f );
    }

    TEST_FIXTURE(VerticalProfileTestFixture, vertical_profile_10)
    {
        //@10 get_min_for()
        LUnits xStart = 1500.0f;
        LUnits xEnd = 19500.0f;
        LUnits cellWidth = 200.0f;      //2 mm
        MyVerticalProfile vp(xStart, xEnd, cellWidth, 2);
        vp.initialize(0, 3000.0f, 3400.0f);

        GmoShapeRectangle shape(nullptr);
        shape.set_origin(3400.0f, 1500.0f);
        shape.set_height(2500.0f);
        shape.set_width(5000.0f);
        vp.update(&shape, 0);

        CHECK( vp.get_min_for(3800.0f, 6300.0f, 0) == 1500.0f );
        CHECK( vp.get_min_for(2500.0f, 3100.0f, 0) == 3200.0f );
    }

};



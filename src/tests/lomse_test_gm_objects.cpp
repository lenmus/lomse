//---------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_box_system.h"
#include "lomse_box_slice.h"
#include "lomse_internal_model.h"
#include "lomse_shape_staff.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class GmoTestFixture
{
public:

    GmoTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~GmoTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

//---------------------------------------------------------------------------------------
SUITE(GmoTest)
{

    // GmoBoxDocument ---------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, GmoBoxDocument_InitiallyEmpty)
    {
        GmoBoxDocument boxDoc;
        CHECK( boxDoc.get_num_pages() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, GmoBoxDocument_AddPage)
    {
        GmoBoxDocument boxDoc;
        boxDoc.add_new_page();
        CHECK( boxDoc.get_num_pages() == 1 );
        GmoBoxDocPage* pPage = boxDoc.get_page(0);
        CHECK( pPage != NULL );
        CHECK( pPage->get_number() == 1 );
    }

    TEST_FIXTURE(GmoTestFixture, GmoBoxDocument_AddPageIncrementsPagenum)
    {
        GmoBoxDocument boxDoc;
        boxDoc.add_new_page();
        boxDoc.add_new_page();
        CHECK( boxDoc.get_num_pages() == 2 );
        GmoBoxDocPage* pPage = boxDoc.get_page(1);
        CHECK( pPage != NULL );
        CHECK( pPage->get_number() == 2 );
    }

    // GmoStubScore -----------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, StubScore_InitiallyEmpty)
    {
        GmoStubScore stub(NULL);
        CHECK( stub.get_num_pages() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, StubScore_HasPages)
    {
        GmoStubScore stub(NULL);
        GmoBoxScorePage box(&stub, NULL);
        CHECK( stub.get_num_pages() == 1 );
        CHECK( stub.get_num_systems() == 0 );
    }

    // GmoBoxScorePage ---------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxScorePage_InitiallyEmpty)
    {
        GmoStubScore stub(NULL);
        GmoBoxScorePage box(&stub, NULL);
        CHECK( box.get_num_systems() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, BoxScorePage_AddSystem)
    {
        GmoStubScore stub(NULL);
        GmoBoxScorePage box(&stub, NULL);
        GmoBoxSystem* pSys = box.add_system(0);
        CHECK( box.get_num_systems() == 1 );
        CHECK( pSys = box.get_system(0) );
    }

    // GmoBoxSystem ---------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxSystem_InitiallyEmpty)
    {
        GmoBoxSystem box(NULL);
        CHECK( box.get_num_slices() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_AddSlice)
    {
        GmoBoxSystem box(NULL);
        GmoBoxSlice* pSlice = box.add_slice(0);
        CHECK( box.get_num_slices() == 1 );
        CHECK( box.get_slice(0) == pSlice );
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_StoreShapes)
    {
        GmoBoxSystem box(NULL);
        ImoStaffInfo staff;
        GmoShapeStaff* pShape = new GmoShapeStaff(&box, &staff, 0, 0.0f);
        box.add_staff_shape(pShape);
        CHECK( box.get_staff_shape(0) == pShape );
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_ShapesHaveLayer)
    {
        GmoBoxSystem box(NULL);
        ImoStaffInfo staff;
        GmoShapeStaff* pShape = new GmoShapeStaff(&box, &staff, 0, 0.0f);
        box.add_staff_shape(pShape);
        CHECK( pShape->get_layer() == GmoShape::k_layer_staff );
    }

    TEST_FIXTURE(GmoTestFixture, Shape_SetOrigin)
    {
        GmoBoxSystem box(NULL);
        ImoStaffInfo staff;
        GmoShapeStaff* pShape = new GmoShapeStaff(&box, &staff, 0, 0.0f);
        box.add_staff_shape(pShape);
        pShape->set_origin(2000.0f, 3000.0f);
        CHECK( pShape->get_left() == 2000.0f );
        CHECK( pShape->get_top() == 3000.0f );
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_AddShapeUpdatesLayerPointers)
    {
        GmoBoxSystem box(NULL);
        CHECK( box.get_first_shape_for_layer(GmoShape::k_layer_staff) == NULL );
        ImoStaffInfo staff;
        GmoShapeStaff* pShape = new GmoShapeStaff(&box, &staff, 0, 0.0f);
        box.add_staff_shape(pShape);
        CHECK( box.get_first_shape_for_layer(GmoShape::k_layer_staff) == pShape );
    }

    // GmoBoxSlice ---------------------------------------------------------------

    //TEST_FIXTURE(GmoTestFixture, BoxSlice_InitiallyEmpty)
    //{
    //    GmoBoxSlice box;
    //    CHECK( box.get_num_?() == 0 );
    //}


}



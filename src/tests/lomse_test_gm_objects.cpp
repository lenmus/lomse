//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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
#include <list>
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
// for accessing protected members
class MyGmoBoxDocPage : public GmoBoxDocPage
{
public:
    MyGmoBoxDocPage() : GmoBoxDocPage() {}
    ~MyGmoBoxDocPage() {}

    inline std::list<GmoShape*>& get_all_shapes() { return m_allShapes; }
};


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
        GmoBoxScorePage box(&stub);
        CHECK( stub.get_num_pages() == 1 );
        CHECK( stub.get_num_systems() == 0 );
    }

    // GmoBoxScorePage ---------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxScorePage_InitiallyEmpty)
    {
        GmoStubScore stub(NULL);
        GmoBoxScorePage box(&stub);
        CHECK( box.get_num_systems() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, BoxScorePage_AddSystem)
    {
        GmoStubScore stub(NULL);
        GmoBoxScorePage box(&stub);
        GmoBoxSystem* pSys = box.add_system(0);
        CHECK( box.get_num_systems() == 1 );
        CHECK( pSys = box.get_system(0) );
    }

    // GmoBoxSystem ---------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxSystem_InitiallyEmpty)
    {
        GmoBoxSystem box;
        CHECK( box.get_num_slices() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_AddSlice)
    {
        GmoBoxSystem box;
        GmoBoxSlice* pSlice = box.add_slice(0);
        CHECK( box.get_num_slices() == 1 );
        CHECK( box.get_slice(0) == pSlice );
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_AddShapeUpdatesLayerPointers)
    {
        GmoBoxDocPage page;
        GmoBoxDocPageContent* pDPC = new GmoBoxDocPageContent();
        page.add_child_box(pDPC);
        GmoStubScore stub(NULL);
        GmoBoxScorePage* pScorePage = new GmoBoxScorePage(&stub);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = new GmoBoxSystem();
        pScorePage->add_child_box(pBox);
        CHECK( page.get_first_shape_for_layer(GmoShape::k_layer_staff) == NULL );
        ImoStaffInfo staff;
        GmoShapeStaff* pShape = new GmoShapeStaff(0, &staff, 0, 20.0f, Color(0,0,0));
        pBox->add_staff_shape(pShape);
        CHECK( page.get_first_shape_for_layer(GmoShape::k_layer_staff) == pShape );
    }

    // GmoShape -------------------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxSystem_StoreShapes)
    {
        GmoBoxDocPage page;
        GmoBoxDocPageContent* pDPC = new GmoBoxDocPageContent();
        page.add_child_box(pDPC);
        GmoStubScore stub(NULL);
        GmoBoxScorePage* pScorePage = new GmoBoxScorePage(&stub);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = new GmoBoxSystem();
        pScorePage->add_child_box(pBox);
        ImoStaffInfo staff;
        GmoShapeStaff* pShape = new GmoShapeStaff(0, &staff, 0, 20.0f, Color(0,0,0));
        pBox->add_staff_shape(pShape);
        CHECK( pBox->get_staff_shape(0) == pShape );
    }

    TEST_FIXTURE(GmoTestFixture, Box_StoreShapes)
    {
        GmoBoxDocPage page;
        GmoBoxDocPageContent* pDPC = new GmoBoxDocPageContent();
        page.add_child_box(pDPC);
        GmoStubScore stub(NULL);
        GmoBoxScorePage* pScorePage = new GmoBoxScorePage(&stub);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = new GmoBoxSystem();
        pScorePage->add_child_box(pBox);
        ImoStaffInfo staff;
        GmoShapeStaff* pShape = new GmoShapeStaff(0, &staff, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape, GmoShape::k_layer_staff);
        CHECK( pBox->get_shape(0) == pShape );
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_ShapesHaveLayer)
    {
        GmoBoxDocPage page;
        GmoBoxDocPageContent* pDPC = new GmoBoxDocPageContent();
        page.add_child_box(pDPC);
        GmoStubScore stub(NULL);
        GmoBoxScorePage* pScorePage = new GmoBoxScorePage(&stub);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = new GmoBoxSystem();
        pScorePage->add_child_box(pBox);
        ImoStaffInfo staff;
        GmoShapeStaff* pShape = new GmoShapeStaff(0, &staff, 0, 20.0f, Color(0,0,0));
        pBox->add_staff_shape(pShape);
        CHECK( pShape->get_layer() == GmoShape::k_layer_staff );
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_ShapesOrderedByLayer)
    {
        MyGmoBoxDocPage page;
        GmoBoxDocPageContent* pDPC = new GmoBoxDocPageContent();
        page.add_child_box(pDPC);
        GmoStubScore stub(NULL);
        GmoBoxScorePage* pScorePage = new GmoBoxScorePage(&stub);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = new GmoBoxSystem();
        pScorePage->add_child_box(pBox);
        ImoStaffInfo staff;
        GmoShapeStaff* pShape0 = new GmoShapeStaff(0, &staff, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape0, 2);
        GmoShapeStaff* pShape1 = new GmoShapeStaff(1, &staff, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape1, 1);
        GmoShapeStaff* pShape2 = new GmoShapeStaff(2, &staff, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape2, 3);
        GmoShapeStaff* pShape3 = new GmoShapeStaff(3, &staff, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape3, 1);
        GmoShapeStaff* pShape4 = new GmoShapeStaff(4, &staff, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape4, 0);
        GmoShapeStaff* pShape5 = new GmoShapeStaff(5, &staff, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape5, 1);

        std::list<GmoShape*>& shapes = page.get_all_shapes();
        std::list<GmoShape*>::iterator it = shapes.begin();

        //cout << (*it)->get_layer() << endl;
        CHECK( (*it) == pShape4 );
        CHECK( (*it)->get_layer() == 0 );
        ++it;

        CHECK( (*it) == pShape1 );
        CHECK( (*it)->get_layer() == 1 );
        ++it;

        CHECK( (*it) == pShape3 );
        CHECK( (*it)->get_layer() == 1 );
        ++it;

        CHECK( (*it) == pShape5 );
        CHECK( (*it)->get_layer() == 1 );
        ++it;

        CHECK( (*it) == pShape0 );
        CHECK( (*it)->get_layer() == 2 );
        ++it;

        CHECK( (*it) == pShape2 );
        CHECK( (*it)->get_layer() == 3 );
        ++it;

        CHECK( it == shapes.end() );
    }

    TEST_FIXTURE(GmoTestFixture, Shape_SetOrigin)
    {
        GmoBoxDocPage page;
        GmoBoxDocPageContent* pDPC = new GmoBoxDocPageContent();
        page.add_child_box(pDPC);
        GmoStubScore stub(NULL);
        GmoBoxScorePage* pScorePage = new GmoBoxScorePage(&stub);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = new GmoBoxSystem();
        pScorePage->add_child_box(pBox);
        ImoStaffInfo staff;
        GmoShapeStaff* pShape = new GmoShapeStaff(0, &staff, 0, 0.0f, Color(0,0,0));
        pBox->add_shape(pShape, 0);
        pShape->set_origin(2000.0f, 3000.0f);
        CHECK( pShape->get_left() == 2000.0f );
        CHECK( pShape->get_top() == 3000.0f );
    }

    // GmoBox ---------------------------------------------------------------------------

    //TEST_FIXTURE(GmoTestFixture, Box_xxxxxxxxx)
    //{
    //    GmoBoxSlice box;
    //    CHECK( box.get_num_?() == 0 );
    //}

}



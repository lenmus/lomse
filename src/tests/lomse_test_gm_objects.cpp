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
#include <list>
#include "lomse_injectors.h"
#include "lomse_graphical_model.h"
#include "lomse_gm_basic.h"
#include "lomse_box_system.h"
#include "lomse_box_slice.h"
#include "lomse_internal_model.h"
#include "lomse_shape_staff.h"
#include "lomse_im_factory.h"
#include "private/lomse_document_p.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
// for accessing protected members
class MyGmoBoxDocPage : public GmoBoxDocPage
{
public:
    MyGmoBoxDocPage(ImoObj* pCreatorImo) : GmoBoxDocPage(pCreatorImo) {}
    ~MyGmoBoxDocPage() {}

    inline std::list<GmoShape*>& get_all_shapes() { return m_allShapes; }
};


//---------------------------------------------------------------------------------------
class GmoTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    GmoTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~GmoTestFixture()    //TearDown fixture
    {
    }
};

//---------------------------------------------------------------------------------------
SUITE(GmoTest)
{

    // GmoBoxDocument ---------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, GmoBoxDocument_InitiallyEmpty)
    {
        GmoBoxDocument boxDoc(nullptr, nullptr);
        CHECK( boxDoc.get_num_pages() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, GmoBoxDocument_AddPage)
    {
        GmoBoxDocument boxDoc(nullptr, nullptr);
        boxDoc.add_new_page();
        CHECK( boxDoc.get_num_pages() == 1 );
        GmoBoxDocPage* pPage = boxDoc.get_page(0);
        CHECK( pPage != nullptr );
        CHECK( pPage->get_number() == 1 );
    }

    TEST_FIXTURE(GmoTestFixture, GmoBoxDocument_AddPageIncrementsPagenum)
    {
        GmoBoxDocument boxDoc(nullptr, nullptr);
        boxDoc.add_new_page();
        boxDoc.add_new_page();
        CHECK( boxDoc.get_num_pages() == 2 );
        GmoBoxDocPage* pPage = boxDoc.get_page(1);
        CHECK( pPage != nullptr );
        CHECK( pPage->get_number() == 2 );
    }

    // GmoBoxScorePage ---------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxScorePage_InitiallyEmpty)
    {
        GmoBoxScorePage box(nullptr);
        CHECK( box.get_num_systems() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, BoxScorePage_AddSystem)
    {
        GmoBoxScorePage box(nullptr);
        GmoBoxSystem* pSys = LOMSE_NEW GmoBoxSystem(nullptr);
        box.add_system(pSys, 0);
        CHECK( box.get_num_systems() == 1 );
        CHECK( pSys = box.get_system(0) );
    }

    // GmoBoxSystem ---------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxSystem_InitiallyEmpty)
    {
        GmoBoxSystem box(nullptr);
        CHECK( box.get_num_slices() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_AddShapeUpdatesLayerPointers)
    {
        Document doc(m_libraryScope);
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(nullptr);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(nullptr);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(nullptr);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff* pShape = LOMSE_NEW GmoShapeStaff(pInfo, 0, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_staff_shape(pShape);

        CHECK( page.get_first_shape_for_layer(GmoShape::k_layer_staff) == nullptr );

        pScorePage->add_system(pBox, 0);
        pBox->add_shapes_to_tables();

        CHECK( page.get_first_shape_for_layer(GmoShape::k_layer_staff) == pShape );
        delete pInfo;
    }

    // GmoShape -------------------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxSystem_StoreShapes)
    {
        Document doc(m_libraryScope);
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(nullptr);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(nullptr);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(nullptr);
        pScorePage->add_child_box(pBox);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff* pShape = LOMSE_NEW GmoShapeStaff(pInfo, 0, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_staff_shape(pShape);
        CHECK( pBox->get_staff_shape(0) == pShape );
        delete pInfo;
    }

    TEST_FIXTURE(GmoTestFixture, Box_StoreShapes)
    {
        Document doc(m_libraryScope);
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(nullptr);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(nullptr);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(nullptr);
        pScorePage->add_child_box(pBox);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff* pShape = LOMSE_NEW GmoShapeStaff(pInfo, 0, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape, GmoShape::k_layer_staff);
        CHECK( pBox->get_shape(0) == pShape );
        delete pInfo;
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_ShapesHaveLayer)
    {
        Document doc(m_libraryScope);
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(nullptr);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(nullptr);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(nullptr);
        pScorePage->add_child_box(pBox);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff* pShape = LOMSE_NEW GmoShapeStaff(pInfo, 0, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_staff_shape(pShape);
        CHECK( pShape->get_layer() == GmoShape::k_layer_staff );
        delete pInfo;
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_ShapesOrderedByLayer)
    {
        Document doc(m_libraryScope);
        MyGmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(nullptr);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(nullptr);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(nullptr);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff* pShape0 = LOMSE_NEW GmoShapeStaff(pInfo, 0, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape0, 2);
        GmoShapeStaff* pShape1 = LOMSE_NEW GmoShapeStaff(pInfo, 1, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape1, 1);
        GmoShapeStaff* pShape2 = LOMSE_NEW GmoShapeStaff(pInfo, 2, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape2, 3);
        GmoShapeStaff* pShape3 = LOMSE_NEW GmoShapeStaff(pInfo, 3, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape3, 1);
        GmoShapeStaff* pShape4 = LOMSE_NEW GmoShapeStaff(pInfo, 4, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape4, 0);
        GmoShapeStaff* pShape5 = LOMSE_NEW GmoShapeStaff(pInfo, 5, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_shape(pShape5, 1);

        pScorePage->add_system(pBox, 0);
        pBox->add_shapes_to_tables();

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
        delete pInfo;
    }

    TEST_FIXTURE(GmoTestFixture, Shape_SetOrigin)
    {
        Document doc(m_libraryScope);
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(nullptr);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(nullptr);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(nullptr);
        pScorePage->add_child_box(pBox);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff* pShape = LOMSE_NEW GmoShapeStaff(pInfo, 0, pInfo, 0, 0.0f, Color(0,0,0));
        pBox->add_shape(pShape, 0);
        pShape->set_origin(2000.0f, 3000.0f);
        CHECK( pShape->get_left() == 2000.0f );
        CHECK( pShape->get_top() == 3000.0f );
        delete pInfo;
    }

    // GmoBox ---------------------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, Box_GetGraphicModel)
    {
        GraphicModel gm(nullptr);
        GmoBoxDocument* pDoc = gm.get_root();
        GmoBoxDocPage* pDP = LOMSE_NEW GmoBoxDocPage(nullptr);
        pDoc->add_child_box(pDP);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(nullptr);
        pDP->add_child_box(pDPC);

        CHECK( pDP->get_graphic_model() == &gm );
    }

};



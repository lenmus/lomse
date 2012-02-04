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
#include "lomse_build_options.h"

//classes related to these tests
#include <list>
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_box_system.h"
#include "lomse_box_slice.h"
#include "lomse_internal_model.h"
#include "lomse_shape_staff.h"
#include "lomse_im_factory.h"
#include "lomse_document.h"

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
        m_scores_path = LOMSE_TEST_SCORES_PATH;
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
        GmoBoxDocument boxDoc(NULL, NULL);
        CHECK( boxDoc.get_num_pages() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, GmoBoxDocument_AddPage)
    {
        GmoBoxDocument boxDoc(NULL, NULL);
        boxDoc.add_new_page();
        CHECK( boxDoc.get_num_pages() == 1 );
        GmoBoxDocPage* pPage = boxDoc.get_page(0);
        CHECK( pPage != NULL );
        CHECK( pPage->get_number() == 1 );
    }

    TEST_FIXTURE(GmoTestFixture, GmoBoxDocument_AddPageIncrementsPagenum)
    {
        GmoBoxDocument boxDoc(NULL, NULL);
        boxDoc.add_new_page();
        boxDoc.add_new_page();
        CHECK( boxDoc.get_num_pages() == 2 );
        GmoBoxDocPage* pPage = boxDoc.get_page(1);
        CHECK( pPage != NULL );
        CHECK( pPage->get_number() == 2 );
    }

    // GmoBoxScorePage ---------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxScorePage_InitiallyEmpty)
    {
        GmoBoxScorePage box(NULL);
        CHECK( box.get_num_systems() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, BoxScorePage_AddSystem)
    {
        GmoBoxScorePage box(NULL);
        GmoBoxSystem* pSys = LOMSE_NEW GmoBoxSystem(NULL);
        box.add_system(pSys, 0);
        CHECK( box.get_num_systems() == 1 );
        CHECK( pSys = box.get_system(0) );
    }

    // GmoBoxSystem ---------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxSystem_InitiallyEmpty)
    {
        GmoBoxSystem box(NULL);
        CHECK( box.get_num_slices() == 0 );
    }

    TEST_FIXTURE(GmoTestFixture, BoxSystem_AddShapeUpdatesLayerPointers)
    {
        Document doc(m_libraryScope);
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(NULL);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(NULL);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(NULL);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff* pShape = LOMSE_NEW GmoShapeStaff(pInfo, 0, pInfo, 0, 20.0f, Color(0,0,0));
        pBox->add_staff_shape(pShape);

        CHECK( page.get_first_shape_for_layer(GmoShape::k_layer_staff) == NULL );

        pScorePage->add_system(pBox, 0);
        pBox->add_shapes_to_tables();

        CHECK( page.get_first_shape_for_layer(GmoShape::k_layer_staff) == pShape );
        delete pInfo;
    }

    // GmoShape -------------------------------------------------------------------------

    TEST_FIXTURE(GmoTestFixture, BoxSystem_StoreShapes)
    {
        Document doc(m_libraryScope);
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(NULL);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(NULL);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(NULL);
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
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(NULL);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(NULL);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(NULL);
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
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(NULL);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(NULL);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(NULL);
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
        MyGmoBoxDocPage page(NULL);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(NULL);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(NULL);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(NULL);
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
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(NULL);
        page.add_child_box(pDPC);
        GmoBoxScorePage* pScorePage = LOMSE_NEW GmoBoxScorePage(NULL);
        pDPC->add_child_box(pScorePage);
        GmoBoxSystem* pBox = LOMSE_NEW GmoBoxSystem(NULL);
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
        GraphicModel gm;
        GmoBoxDocument* pDoc = gm.get_root();
        GmoBoxDocPage* pDP = LOMSE_NEW GmoBoxDocPage(NULL);
        pDoc->add_child_box(pDP);
        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(NULL);
        pDP->add_child_box(pDPC);

        CHECK( pDP->get_graphic_model() == &gm );
    }

};



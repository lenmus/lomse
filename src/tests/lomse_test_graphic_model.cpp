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
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_box_system.h"
#include "lomse_shape_staff.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_document.h"
#include "lomse_interactor.h"
#include "lomse_graphic_view.h"
#include "lomse_doorway.h"
#include "lomse_screen_drawer.h"
#include "lomse_analyser.h"
#include "lomse_model_builder.h"
#include "lomse_im_factory.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;






//---------------------------------------------------------------------------------------
class MyDoorway : public LomseDoorway
{
protected:
    bool m_fUpdateWindowInvoked;
    bool m_fSetWindowTitleInvoked;
    std::string m_title;
    RenderingBuffer m_buffer;

public:
    MyDoorway()
        : LomseDoorway()
    {
        init_library(k_pix_format_rgba32, 96, false);
    }
    virtual ~MyDoorway() {}

    void update_window() { m_fUpdateWindowInvoked = true; }
    void set_window_title(const std::string& title) {
        m_fSetWindowTitleInvoked = true;
        m_title = title;
    }
    void force_redraw() {}
    RenderingBuffer& get_window_buffer() { return m_buffer; }

    bool set_window_title_invoked() { return m_fSetWindowTitleInvoked; }
    bool update_window_invoked() { return m_fUpdateWindowInvoked; }
    const std::string& get_title() { return m_title; }
    double get_screen_ppi() const { return 96.0; }

};


//---------------------------------------------------------------------------------------
class GraphicModelTestFixture
{
public:
    std::string m_scores_path;

    GraphicModelTestFixture()     //SetUp fixture
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~GraphicModelTestFixture()    //TearDown fixture
    {
    }
};

SUITE(GraphicModelTest)
{

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_RootIsDocBox)
    {
        GraphicModel* pGModel = new GraphicModel();
        GmoObj* pRoot = pGModel->get_root();
        CHECK( pRoot != NULL );
        delete pGModel;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_BoxFound)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        Document doc(libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, ViewFactory::k_view_vertical_book, &doc) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        GraphicModel* pModel = pIntor->get_graphic_model();

        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        LUnits x = pBSliceInstr->get_left() + 1.0f;
        LUnits y = pBSliceInstr->get_top() + 1.0f;

        //cout << "DocPage: " << pPage->get_left() << ", " << pPage->get_top() << endl;
        //cout << "DocPageContent: " << pBDPC->get_left() << ", " << pBDPC->get_top() << endl;
        //cout << "ScorePage: " << pBSP->get_left() << ", " << pBSP->get_top() << endl;
        //cout << "System: " << pBSys->get_left() << ", " << pBSys->get_top() << endl;
        //cout << "Slice: " << pBSlice->get_left() << ", " << pBSlice->get_top() << endl;
        //cout << "SliceInsr: " << pBSliceInstr->get_left() << ", " << pBSliceInstr->get_top() << endl;
        //cout << "Finding: " << x << ", " << y << endl;

        GmoBox* pHit = pPage->find_inner_box_at(x, y);

        CHECK ( pHit != NULL );
        CHECK ( pHit == pBSliceInstr );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_NoBox)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        Document doc(libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (staves 2)(musicData )))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, ViewFactory::k_view_vertical_book, &doc) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);
        LUnits x = pPage->get_left();
        LUnits y = pPage->get_top();

        GmoObj* pHit = pPage->find_inner_box_at(x - 1.0f, y - 1.0f);

        CHECK ( pHit == NULL );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_ClefAt)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        Document doc(libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, ViewFactory::k_view_vertical_book, &doc) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        GraphicModel* pModel = pIntor->get_graphic_model();

        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoShape* pShape = pBSliceInstr->get_shape(0);      //Clef
        LUnits x = pShape->get_left() + 1.0f;
        LUnits y = pShape->get_top() + 1.0f;

        //cout << "DocPage: " << pPage->get_left() << ", " << pPage->get_top() << endl;
        //cout << "DocPageContent: " << pBDPC->get_left() << ", " << pBDPC->get_top() << endl;
        //cout << "ScorePage: " << pBSP->get_left() << ", " << pBSP->get_top() << endl;
        //cout << "System: " << pBSys->get_left() << ", " << pBSys->get_top() << endl;
        //cout << "Slice: " << pBSlice->get_left() << ", " << pBSlice->get_top() << endl;
        //cout << "SliceInsr: " << pBSliceInstr->get_left() << ", " << pBSliceInstr->get_top() << endl;
        //cout << "Shape: " << pShape->get_left() << ", " << pShape->get_top() << endl;
        //cout << "Finding: " << x << ", " << y << endl;

        GmoObj* pHit = pPage->find_shape_at(x, y);

        CHECK ( pHit != NULL );
        CHECK ( pHit == pShape );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_StaffAt)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        Document doc(libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, ViewFactory::k_view_vertical_book, &doc) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        GraphicModel* pModel = pIntor->get_graphic_model();

        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>(pBSP->get_child_box(0));   //System
        GmoShapeStaff* pShape = pBSys->get_staff_shape(0);   //Staff
        LUnits x = pShape->get_left() + 10000.0f;
        LUnits y = pShape->get_top() + 300.0f;

        //cout << "DocPage: " << pPage->get_left() << ", " << pPage->get_top() << endl;
        //cout << "DocPageContent: " << pBDPC->get_left() << ", " << pBDPC->get_top() << endl;
        //cout << "ScorePage: " << pBSP->get_left() << ", " << pBSP->get_top() << endl;
        //cout << "System: " << pBSys->get_left() << ", " << pBSys->get_top() << endl;
        //cout << "Staff: " << pShape->get_left() << ", " << pShape->get_top() << endl;
        //cout << "Finding: " << x << ", " << y << endl;

        GmoObj* pHit = pPage->find_shape_at(x, y);

        CHECK ( pHit != NULL );
        CHECK ( pHit == pShape );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_HitTestBoxFound)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        Document doc(libraryScope);
        //doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
        //    "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (staves 2)(musicData )))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, ViewFactory::k_view_vertical_book, &doc) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        GraphicModel* pModel = pIntor->get_graphic_model();

        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        LUnits x = pBSys->get_left() + 800.0f;
        LUnits y = pBSys->get_top() + 800.0f;

        //cout << "DocPage: " << pPage->get_left() << ", " << pPage->get_top() << endl;
        //cout << "DocPageContent: " << pBDPC->get_left() << ", " << pBDPC->get_top() << endl;
        //cout << "ScorePage: " << pBSP->get_left() << ", " << pBSP->get_top() << endl;
        //cout << "System: " << pBSys->get_left() << ", " << pBSys->get_top() << endl;
        //cout << "Finding: " << x << ", " << y << endl;

        GmoObj* pHit = pPage->hit_test(x, y);

        CHECK ( pHit != NULL );
        CHECK ( pHit == pBSys );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_HitTestNoBox)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        Document doc(libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (staves 2)(musicData )))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, ViewFactory::k_view_vertical_book, &doc) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);
        LUnits x = pPage->get_left() - 1.0f;
        LUnits y = pPage->get_top() - 1.0f;

        GmoObj* pHit = pPage->hit_test(x, y);

        CHECK ( pHit == NULL );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_HitTestShape)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        Document doc(libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, ViewFactory::k_view_vertical_book, &doc) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        GraphicModel* pModel = pIntor->get_graphic_model();

        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoShape* pShape = pBSliceInstr->get_shape(0);      //Clef
        LUnits x = pShape->get_left() + 1.0f;
        LUnits y = pShape->get_top() + 1.0f;

        //cout << "DocPage: " << pPage->get_left() << ", " << pPage->get_top() << endl;
        //cout << "DocPageContent: " << pBDPC->get_left() << ", " << pBDPC->get_top() << endl;
        //cout << "ScorePage: " << pBSP->get_left() << ", " << pBSP->get_top() << endl;
        //cout << "System: " << pBSys->get_left() << ", " << pBSys->get_top() << endl;
        //cout << "Slice: " << pBSlice->get_left() << ", " << pBSlice->get_top() << endl;
        //cout << "SliceInsr: " << pBSliceInstr->get_left() << ", " << pBSliceInstr->get_top() << endl;
        //cout << "Shape: " << pShape->get_left() << ", " << pShape->get_top() << endl;
        //cout << "Finding: " << x << ", " << y << endl;

        GmoObj* pHit = pPage->hit_test(x, y);

        CHECK ( pHit != NULL );
        CHECK ( pHit == pShape );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_ModelHitTestShape)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        Document doc(libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, ViewFactory::k_view_vertical_book, &doc) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        GraphicModel* pModel = pIntor->get_graphic_model();

        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoShape* pShape = pBSliceInstr->get_shape(0);      //Clef
        LUnits x = pShape->get_left() + 1.0f;
        LUnits y = pShape->get_top() + 1.0f;

        //cout << "DocPage: " << pPage->get_left() << ", " << pPage->get_top() << endl;
        //cout << "DocPageContent: " << pBDPC->get_left() << ", " << pBDPC->get_top() << endl;
        //cout << "ScorePage: " << pBSP->get_left() << ", " << pBSP->get_top() << endl;
        //cout << "System: " << pBSys->get_left() << ", " << pBSys->get_top() << endl;
        //cout << "Slice: " << pBSlice->get_left() << ", " << pBSlice->get_top() << endl;
        //cout << "SliceInsr: " << pBSliceInstr->get_left() << ", " << pBSliceInstr->get_top() << endl;
        //cout << "Shape: " << pShape->get_left() << ", " << pShape->get_top() << endl;
        //cout << "Finding: " << x << ", " << y << endl;

        GmoObj* pHit = pModel->hit_test(0, x, y);

        CHECK ( pHit != NULL );
        CHECK ( pHit == pShape );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, NoterestAddedToShapesMap)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);

        Document doc(libraryScope);
        doc.create_empty();

        ImoDocument* pDoc = doc.get_imodoc();
        ImoScore* pScore = static_cast<ImoScore*>( 
                                ImFactory::inject(k_imo_score, &doc) );
        pDoc->append_content_item(pScore);
        ImoInstrument* pInstr = static_cast<ImoInstrument*>(
                                    ImFactory::inject(k_imo_instrument, &doc));
        pScore->add_instrument(pInstr);
        ImoMusicData* pMD = static_cast<ImoMusicData*>(
                                ImFactory::inject(k_imo_music_data, &doc) );
        pInstr->append_child(pMD);
        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
        pClef->set_clef_type(k_clef_G2);
        pMD->append_child(pClef);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(&doc, "(n c4 q)"));
        pMD->append_child(pNote);

        doc.end_of_changes();

        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, ViewFactory::k_view_vertical_book, &doc) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        GraphicModel* pModel = pIntor->get_graphic_model();

        GmoShape* pShape = pModel->get_shape_for_noterest(pNote);
        CHECK ( pShape != NULL );
        CHECK ( pShape->is_shape_note() == true );

        delete pIntor;
    }

};



//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#define LOMSE_INTERNAL_API
#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_graphical_model.h"
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
#include "lomse_ldp_analyser.h"
#include "lomse_model_builder.h"
#include "lomse_im_factory.h"
#include "lomse_timegrid_table.h"

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

    //void request_window_update() { m_fUpdateWindowInvoked = true; }
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
//helper, for accessing private members
class MyGraphicModel : public GraphicModel
{
public:
    MyGraphicModel() : GraphicModel() {}
    ~MyGraphicModel() {}

//    std::map<ImoObj*, RefToGmo*>& my_get_map() { return m_imoToGmo; }
};


//---------------------------------------------------------------------------------------
class GraphicModelTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    GraphicModelTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~GraphicModelTestFixture()    //TearDown fixture
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }
};

SUITE(GraphicModelTest)
{

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_RootIsDocBox)
    {
        GraphicModel* pGModel = LOMSE_NEW GraphicModel();
        GmoObj* pRoot = pGModel->get_root();
        CHECK( pRoot != nullptr );
        delete pGModel;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_BoxFound)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
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

        CHECK ( pHit != nullptr );
        CHECK ( pHit == pBSliceInstr );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_NoBox)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 2.0) "
            "(instrument (staves 2)(musicData)))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);
        LUnits x = pPage->get_left();
        LUnits y = pPage->get_top();

        GmoObj* pHit = pPage->find_inner_box_at(x - 1.0f, y - 1.0f);

        CHECK ( pHit == nullptr );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_ClefAt)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
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

        CHECK ( pHit != nullptr );
        CHECK ( pHit == pShape );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_StaffAt)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pModel = pIntor->get_graphic_model();

        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>(pBSP->get_child_box(0));   //System
        GmoShapeStaff* pShape = pBSys->get_staff_shape(0);   //Staff
        LUnits x = pShape->get_left() + 10000.0f;
        LUnits y = pShape->get_top() + 300.0f;

//        cout << "Test GraphicModel_StaffAt" << endl;
//        cout << "DocPage: " << pPage->get_left() << ", " << pPage->get_top()
//             << ", width= " << pPage->get_width() << endl;
//        cout << "DocPageContent: " << pBDPC->get_left() << ", " << pBDPC->get_top() << endl;
//        cout << "ScorePage: " << pBSP->get_left() << ", " << pBSP->get_top() << endl;
//        cout << "System: " << pBSys->get_left() << ", " << pBSys->get_top()
//             << ", width= " << pBSys->get_width() << ", height= " << pBSys->get_height()
//             << endl;
//        cout << "Staff: " << pShape->get_left() << ", " << pShape->get_top()
//             << ", width= " << pShape->get_width() << ", height= " << pShape->get_height()
//             << endl;
//        cout << "Finding at: " << x << ", " << y << endl;

        GmoObj* pHit = pPage->find_shape_at(x, y);

        CHECK ( pHit != nullptr );
        CHECK ( pHit == pShape );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_HitTestBoxFound)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        //spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
        //    "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        spDoc->from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 2.0) "
            "(instrument (staves 2)(musicData)))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
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

        CHECK ( pHit != nullptr );
        CHECK ( pHit == pBSys );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_HitTestNoBox)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 2.0) "
            "(instrument (staves 2)(musicData)))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);
        LUnits x = pPage->get_left() - 1.0f;
        LUnits y = pPage->get_top() - 1.0f;

        GmoObj* pHit = pPage->hit_test(x, y);

        CHECK ( pHit == nullptr );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_HitTestShape)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
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

        CHECK ( pHit != nullptr );
        CHECK ( pHit == pShape );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, GraphicModel_ModelHitTestShape)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
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

        CHECK ( pHit != nullptr );
        CHECK ( pHit == pShape );

        delete pIntor;
    }

    // map imo -> gmo -------------------------------------------------------------------

    TEST_FIXTURE(GraphicModelTestFixture, NoterestAddedToShapesMap)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);

        //build document, in steps, for easier access to ImObjs
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->create_empty();
        ImoScore* pScore = spDoc->add_score();
        ImoInstrument* pInstr = pScore->add_instrument();
        pInstr->add_clef(k_clef_G2);
        ImoNote* pNote = static_cast<ImoNote*>( pInstr->add_object("(n c4 q)") );
        spDoc->end_of_changes();

        //build gmodel
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pModel = pIntor->get_graphic_model();

        GmoShape* pShape = pModel->get_shape_for_noterest(pNote);

        CHECK ( pShape != nullptr );
        CHECK ( pShape->is_shape_note() == true );

        delete pIntor;
    }

    // dirty bits -----------------------------------------------------------------------

    TEST_FIXTURE(GraphicModelTestFixture, dirty_at_creation)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoShape* pShape = pBSliceInstr->get_shape(0);      //Clef

        CHECK ( pPage->is_dirty() == true );
        CHECK ( pBDPC->is_dirty() == true );
        CHECK ( pBSP->is_dirty() == true );
        CHECK ( pBSys->is_dirty() == true );
        CHECK ( pBSlice->is_dirty() == true );
        CHECK ( pBSliceInstr->is_dirty() == true );
        CHECK ( pShape->is_dirty() == true );

        CHECK ( pPage->are_children_dirty() == false );
        CHECK ( pBDPC->are_children_dirty() == false );
        CHECK ( pBSP->are_children_dirty() == false );
        CHECK ( pBSys->are_children_dirty() == false );
        CHECK ( pBSlice->are_children_dirty() == false );
        CHECK ( pBSliceInstr->are_children_dirty() == false );
        CHECK ( pShape->are_children_dirty() == false );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, set_dirty_propagates)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoShape* pShape = pBSliceInstr->get_shape(0);      //Clef

        pPage->set_dirty(false);
        pBDPC->set_dirty(false);
        pBSP->set_dirty(false);
        pBSys->set_dirty(false);
        pBSlice->set_dirty(false);
        pBSliceInstr->set_dirty(false);
        pShape->set_dirty(false);

        CHECK ( pPage->is_dirty() == false );
        CHECK ( pBDPC->is_dirty() == false );
        CHECK ( pBSP->is_dirty() == false );
        CHECK ( pBSys->is_dirty() == false );
        CHECK ( pBSlice->is_dirty() == false );
        CHECK ( pBSliceInstr->is_dirty() == false );
        CHECK ( pShape->is_dirty() == false );

        CHECK ( pPage->are_children_dirty() == false );
        CHECK ( pBDPC->are_children_dirty() == false );
        CHECK ( pBSP->are_children_dirty() == false );
        CHECK ( pBSys->are_children_dirty() == false );
        CHECK ( pBSlice->are_children_dirty() == false );
        CHECK ( pBSliceInstr->are_children_dirty() == false );
        CHECK ( pShape->are_children_dirty() == false );


        pShape->set_dirty(true);


        CHECK ( pPage->is_dirty() == false );
        CHECK ( pBDPC->is_dirty() == false );
        CHECK ( pBSP->is_dirty() == false );
        CHECK ( pBSys->is_dirty() == false );
        CHECK ( pBSlice->is_dirty() == false );
        CHECK ( pBSliceInstr->is_dirty() == false );
        CHECK ( pShape->is_dirty() == true );

        CHECK ( pPage->are_children_dirty() == true );
        CHECK ( pBDPC->are_children_dirty() == true );
        CHECK ( pBSP->are_children_dirty() == true );
        CHECK ( pBSys->are_children_dirty() == true );
        CHECK ( pBSlice->are_children_dirty() == true );
        CHECK ( pBSliceInstr->are_children_dirty() == true );
        CHECK ( pShape->are_children_dirty() == false );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, set_dirty_propagates_to_gmodel)
    {
        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoShape* pShape = pBSliceInstr->get_shape(0);      //Clef

        pPage->set_dirty(false);
        pBDPC->set_dirty(false);
        pBSP->set_dirty(false);
        pBSys->set_dirty(false);
        pBSlice->set_dirty(false);
        pBSliceInstr->set_dirty(false);
        pShape->set_dirty(false);

        CHECK ( pModel->is_modified() == true );
        pModel->set_modified(false);
        CHECK ( pModel->is_modified() == false );

        pShape->set_dirty(true);
        CHECK ( pModel->is_modified() == true );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, all_gmos_have_name)
    {
        bool fOk = true;
        for (int i=0; i < GmoObj::k_max; ++i)
        {
            if (GmoObj::get_name(i) == "unknown" )
            {
                fOk = false;
                cout << "Name for GmoObj type " << i << " is not defined" << endl;
            }
        }
        CHECK (fOk);
    }

//    // map Imo-> Gmo --------------------------------------------------------------------
//
//    TEST_FIXTURE(GraphicModelTestFixture, map_empty)
//    {
//        MyGraphicModel gm;
//        GmoBoxDocument* pBD = gm.get_root();
//        GmoBoxDocPage* pBDP = LOMSE_NEW GmoBoxDocPage(nullptr);
//        pBD->add_child_box(pBDP);
//
//        std::map<ImoObj*, RefToGmo*>& rmap = gm.my_get_map();
//
////        GmoBoxDocPageContent* pDPC = LOMSE_NEW GmoBoxDocPageContent(nullptr);
////        pDP->add_child_box(pDPC);
//
//        CHECK( rmap.size() == 0 );
//    }
//
//    TEST_FIXTURE(GraphicModelTestFixture, add_to_map)
//    {
//        MyGraphicModel gm;
//        std::map<ImoObj*, RefToGmo*>& rmap = gm.my_get_map();
//        Document doc(m_libraryScope);
//        spDoc->create_empty();
//        ImoScore* pScore = spDoc->add_score();
//        GmoBoxScorePage* pBox = LOMSE_NEW GmoBoxScorePage(pScore);
//
//        gm.add_to_map_imo_gmo(pBox);
//
//        CHECK( rmap.size() == 1 );
//
//        delete pBox;
//    }
//
//    TEST_FIXTURE(GraphicModelTestFixture, add_to_map_ignored_if_no_imo)
//    {
//        MyGraphicModel gm;
//        std::map<ImoObj*, RefToGmo*>& rmap = gm.my_get_map();
//        GmoBoxScorePage* pBox = LOMSE_NEW GmoBoxScorePage(nullptr);
//
//        gm.add_to_map_imo_gmo(pBox);
//
//        CHECK( rmap.size() == 0 );
//
//        delete pBox;
//    }
//
//    TEST_FIXTURE(GraphicModelTestFixture, map_get_box_simple)
//    {
//        MyGraphicModel gm;
//        Document doc(m_libraryScope);
//        spDoc->create_empty();
//        ImoScore* pScore = spDoc->add_score();
//        GmoBoxScorePage* pBox = LOMSE_NEW GmoBoxScorePage(pScore);
//
//        gm.build_main_boxes_table();
////        gm.add_to_map_imo_gmo(pBox);
//        ImoId id = pScore->get_id();
//        GmoBox* pSavedBox = gm.get_box_for_imo(id);
//        CHECK( pSavedBox == pBox );
//
//        delete pBox;
//    }
//
//    TEST_FIXTURE(GraphicModelTestFixture, add_to_map_twice)
//    {
//        MyGraphicModel gm;
//        std::map<ImoObj*, RefToGmo*>& rmap = gm.my_get_map();
//        Document doc(m_libraryScope);
//        spDoc->create_empty();
//        ImoScore* pScore = spDoc->add_score();
//        GmoBoxScorePage* pBox = LOMSE_NEW GmoBoxScorePage(pScore);
//        gm.add_to_map_imo_gmo(pBox);
//
//        gm.add_to_map_imo_gmo(pBox);
//
//        CHECK( rmap.size() == 1 );
//        CHECK( gm.get_box_for_imo(pScore->get_id()) == pBox );
//
//        delete pBox;
//    }
//
//    TEST_FIXTURE(GraphicModelTestFixture, add_to_map_update)
//    {
//        MyGraphicModel gm;
//        std::map<ImoObj*, RefToGmo*>& rmap = gm.my_get_map();
//        Document doc(m_libraryScope);
//        spDoc->create_empty();
//        ImoScore* pScore = spDoc->add_score();
//        GmoBoxScorePage* pBox = LOMSE_NEW GmoBoxScorePage(pScore);
//        gm.add_to_map_imo_gmo(pBox);
//        delete pBox;
//        pBox = LOMSE_NEW GmoBoxScorePage(pScore);
//
//        gm.add_to_map_imo_gmo(pBox);
//
//        CHECK( rmap.size() == 1 );
//        CHECK( gm.get_box_for_imo(pScore->get_id()) == pBox );
//
//        delete pBox;
//    }
//
////    TEST_FIXTURE(GraphicModelTestFixture, map_add_box)
////    {
////        MyGraphicModel gm;
////        GmoBoxDocument* pBD = gm.get_root();
////        GmoBoxDocPage* pBDP = LOMSE_NEW GmoBoxDocPage(nullptr);
////        pBD->add_child_box(pBDP);
////        std::map<ImoObj*, RefToGmo*>& rmap = gm.my_get_map();
////
////        Document doc(m_libraryScope);
////        spDoc->create_empty();
////        ImoScore* pScore = spDoc->add_score();
//////        ImoInstrument* pInstr = pScore->add_instrument();
//////        ImoClef* pClef = pInstr->add_clef(k_clef_G2);
////
////        ImoDocument* pDoc = spDoc->get_im_root();
////        ImoContent* pContent = static_cast<ImoContent*>(
////                                    pDoc->get_child_of_type(k_content) );
////        GmoBoxDocPageContent* pBDPC = LOMSE_NEW GmoBoxDocPageContent(pContent);
////        CHECK( rmap.size() == 0 );
////
////        pBDP->add_child_box(pBDPC);
////
////        CHECK( rmap.size() == 1 );
////    }

    TEST_FIXTURE(GraphicModelTestFixture, get_system_for_001)
    {
        //@001. get_system_for() returns system

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();

        GmoBoxDocPage* pPage = pGModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBoxSystem* pBSys = static_cast<GmoBoxSystem*>(pBSP->get_child_box(0));

        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBoxSystem = pGModel->get_system_for(scoreId, 64.0);

        CHECK( pBoxSystem == pBSys );

        delete pIntor;
    }


    //@1xx. TimeGridTable::get_x_for_note_rest_at_time() --------------------------------

    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_100)
    {
        //@100. Returns 0 if timepos lower than first entry

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();
        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBSys = pGModel->get_system_for(scoreId, 64.0);
        TimeGridTable* pGrid = pBSys->get_time_grid_table();

        CHECK( pGrid->get_x_for_note_rest_at_time(-1.0) == 0.0 );

//        cout << test_name() << endl;
//        cout << pGrid->dump();
//        cout << "x(t=-1.0) = " << pGrid->get_x_for_note_rest_at_time(-1.0) << endl;

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_101)
    {
        //@101. Get note position when no more staffobjs at that timepos.

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(time 2 4)"
            "(n c4 q)(r q)(barline simple)"
            "(n e4 q)(n e4 e)(n g4 e)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();
        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBSys = pGModel->get_system_for(scoreId, 128.0);
        TimeGridTable* pGrid = pBSys->get_time_grid_table();

        CHECK( is_equal_pos(pGrid->get_x_for_note_rest_at_time(192.0), 6439.38477f) );
//        cout << test_name() << endl;
//        cout << pGrid->dump();
//        cout << "x(t=192.0) = " << setw(14) << setprecision(5) << pGrid->get_x_for_note_rest_at_time(192.0) << endl;

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_102)
    {
        //@102. Get note position when two staffobjs at that timepos

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(time 2 4)"
            "(n c4 q)(r q)(barline simple)"
            "(n e4 q)(n e4 e)(n g4 e)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();
        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBSys = pGModel->get_system_for(scoreId, 128.0);
        TimeGridTable* pGrid = pBSys->get_time_grid_table();

        CHECK( is_equal_pos(pGrid->get_x_for_note_rest_at_time(128.0), 5381.58984f) );
//        cout << test_name() << endl;
//        cout << pGrid->dump();
//        cout << "x(t=128.0) = " << setw(14) << setprecision(5) << pGrid->get_x_for_note_rest_at_time(128.0) << endl;

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_103)
    {
        //@103. Get note position when many staffobjs at that timepos

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(time 2 4)"
            "(n c4 q)(r q)(barline simple)"
            "(clef F4)(n e4 q)(n e4 e)(n g4 e)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();
        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBSys = pGModel->get_system_for(scoreId, 128.0);
        TimeGridTable* pGrid = pBSys->get_time_grid_table();

        CHECK( is_equal_pos(pGrid->get_x_for_note_rest_at_time(128.0), 6154.58984f) );
//        cout << test_name() << endl;
//        cout << pGrid->dump();
//        cout << "x(t=128.0) = " << setw(14) << setprecision(5) << pGrid->get_x_for_note_rest_at_time(128.0) << endl;

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_104)
    {
        //@104. Interpolated value for timepos between two table values

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(time 2 4)"
            "(n c4 q)(r q)(barline simple)"
            "(n e4 q)(n e4 e)(n g4 e)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();
        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBSys = pGModel->get_system_for(scoreId, 128.0);
        TimeGridTable* pGrid = pBSys->get_time_grid_table();

        CHECK( is_equal_pos(pGrid->get_x_for_note_rest_at_time(160.0), 5910.48731f) );
//        cout << test_name() << endl;
//        cout << pGrid->dump();
//        cout << "x(t=160.0) = " << fixed << setw(14) << setprecision(5) << pGrid->get_x_for_note_rest_at_time(160.0) << endl;

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_105)
    {
        //@105. Timepos greater than maximum entry. Return last staffobj position

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(time 2 4)"
            "(n c4 q)(r q)(barline simple)"
            "(n e4 q)(n e4 e)(n g4 e)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();
        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBSys = pGModel->get_system_for(scoreId, 128.0);
        TimeGridTable* pGrid = pBSys->get_time_grid_table();

        CHECK( is_equal_pos(pGrid->get_x_for_note_rest_at_time(260.0), 7672.18213f) );
//        cout << test_name() << endl;
//        cout << pGrid->dump();
//        cout << "x(t=160.0) = " << fixed << setw(14) << setprecision(5) << pGrid->get_x_for_note_rest_at_time(260.0) << endl;

        delete pIntor;
    }


    //@2xx. TimeGridTable::get_x_for_staffobj_at_time() ---------------------------------

    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_200)
    {
        //@200. Returns 0 if timepos lower than first entry

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();
        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBSys = pGModel->get_system_for(scoreId, 64.0);
        TimeGridTable* pGrid = pBSys->get_time_grid_table();

        CHECK( pGrid->get_x_for_staffobj_at_time(-1.0) == 0.0 );

//        cout << test_name() << endl;
//        cout << pGrid->dump();
//        cout << "x(t=-1.0) = " << pGrid->get_x_for_staffobj_at_time(-1.0) << endl;

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_201)
    {
        //@201. Get first staffobj position when more staffobjs at that timepos

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(time 2 4)"
            "(n c4 q)(r q)(barline simple)"
            "(n e4 q)(n e4 e)(n g4 e)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();
        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBSys = pGModel->get_system_for(scoreId, 128.0);
        TimeGridTable* pGrid = pBSys->get_time_grid_table();

        CHECK( is_equal_pos(pGrid->get_x_for_staffobj_at_time(128.0), 5309.59f) );
//        cout << test_name() << endl;
//        cout << pGrid->dump();
//        cout << "x(t=128.0) = " << setw(14) << setprecision(5) << pGrid->get_x_for_staffobj_at_time(128.0, false) << endl;

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_202)
    {
        //@202. Interpolated value for timepos between two table values

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(time 2 4)"
            "(n c4 q)(r q)(barline simple)"
            "(n e4 q)(n e4 e)(n g4 e)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();
        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBSys = pGModel->get_system_for(scoreId, 128.0);
        TimeGridTable* pGrid = pBSys->get_time_grid_table();

        CHECK( is_equal_pos(pGrid->get_x_for_staffobj_at_time(160.0), 5910.48731f) );
//        cout << test_name() << endl;
//        cout << pGrid->dump();
//        cout << "x(t=160.0) = " << fixed << setw(14) << setprecision(5) << pGrid->get_x_for_staffobj_at_time(160.0) << endl;

        delete pIntor;
    }

    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_203)
    {
        //@203. Timepos greater than maximum entry. Return last staffobj position

        MyDoorway doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(score (vers 2.0) "
            "(instrument (musicData (clef G)(key e)(time 2 4)"
            "(n c4 q)(r q)(barline simple)"
            "(n e4 q)(n e4 e)(n g4 e)(barline simple)"
            ")))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr);
        GraphicModel* pGModel = pIntor->get_graphic_model();
        ImoId scoreId = spDoc->get_im_root()->get_content_item(0)->get_id();
        GmoBoxSystem* pBSys = pGModel->get_system_for(scoreId, 128.0);
        TimeGridTable* pGrid = pBSys->get_time_grid_table();

        CHECK( is_equal_pos(pGrid->get_x_for_staffobj_at_time(260.0), 7672.18213f) );
//        cout << test_name() << endl;
//        cout << pGrid->dump();
//        cout << "x(t=160.0) = " << fixed << setw(14) << setprecision(5) << pGrid->get_x_for_staffobj_at_time(260.0) << endl;

        delete pIntor;
    }

//    TEST_FIXTURE(GraphicModelTestFixture, time_grid_table_002)
//    {
//        //@002. TimeGridTable for multimetric test score
//    }

};



//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#define LOMSE_INTERNAL_API
#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_presenter.h"
#include "private/lomse_document_p.h"
#include "lomse_interactor.h"
#include "lomse_graphic_view.h"
#include "lomse_tasks.h"
#include "lomse_graphical_model.h"
#include "lomse_shapes.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

////---------------------------------------------------------------------------------------
//static bool fNotified = false;
//static void my_callback_function(Notification* event)
//{
//    fNotified = true;
//}


//---------------------------------------------------------------------------------------
//MyDoorway: Derived class to avoid platform dependent code
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
        init_library(k_pix_format_rgba32, 96);
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
//MyTaskDragView: Derived class to access protected members
class MyTaskDragView : public TaskDragView
{
public:
    MyTaskDragView(Interactor* pIntor) : TaskDragView(pIntor) {}

    bool is_waiting_for_first_point() { return m_state == k_waiting_for_first_point; }
    bool is_waiting_for_second_point() { return m_state == k_waiting_for_second_point; }
    int get_state() { return m_state; }
};

//---------------------------------------------------------------------------------------
//MyTaskSelection: Derived class to access protected members
class MyTaskSelection : public TaskSelection
{
public:
    MyTaskSelection(Interactor* pIntor) : TaskSelection(pIntor) {}

    bool is_waiting_for_first_point() { return m_state == k_waiting_for_first_point; }
    bool is_waiting_for_point_2_left() { return m_state == k_waiting_for_point_2; }
    bool is_waiting_for_point_2_right() { return m_state == k_waiting_for_point_2; }
    int get_state() { return m_state; }
    Pixels first_point_x() { return m_xStart; }
    Pixels first_point_y() { return m_yStart; }
};

//---------------------------------------------------------------------------------------
//MyInteractor: Mock class for tests
class MyInteractor : public Interactor
{
protected:
    bool m_fSelRectInvoked;
    bool m_fSelObjInvoked;
    Rectangle<Pixels> m_selRectangle;
    Point<Pixels> m_selPoint;

public:
    MyInteractor(LibraryScope& libraryScope, WpDocument wpDoc, View* pView)
        : Interactor(libraryScope, wpDoc, pView, nullptr)
        , m_fSelRectInvoked(false)
        , m_fSelObjInvoked(false)
    {
    }
    //selection
    void task_action_select_object_and_show_contextual_menu(Pixels x, Pixels y,
                                                            unsigned UNUSED(flags) =0)
    {
        m_fSelObjInvoked = true;
        m_selPoint = Point<Pixels>(x, y);
    }
    void task_action_select_objects_in_screen_rectangle(Pixels x1, Pixels y1, Pixels x2, Pixels y2,
                                            unsigned UNUSED(flags) =0)
    {
        m_fSelRectInvoked = true;
        m_selRectangle.set_top_left( Point<Pixels>(x1, y1) );
        m_selRectangle.set_bottom_right( Point<Pixels>(x2, y2) );
    }

    //specific for tests
    bool select_objects_in_rectangle_invoked() { return m_fSelRectInvoked; }
    bool select_object_invoked() { return m_fSelObjInvoked; }
    bool sel_rectangle_is(Pixels x1, Pixels y1, Pixels x2, Pixels y2)
    {
        return  m_selRectangle.left() == x1
                && m_selRectangle.top() == y1
                && m_selRectangle.right() == x2
                && m_selRectangle.bottom() == y2;
    }
    bool sel_point_is(Pixels x, Pixels y)
    {
        return m_selPoint.x == x && m_selPoint.y == y;
    }


};


//---------------------------------------------------------------------------------------
class InteractorTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    InteractorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~InteractorTestFixture()    //TearDown fixture
    {
    }
};


SUITE(InteractorTest)
{

    TEST_FIXTURE(InteractorTestFixture, Interactor_CreatesGraphicModel)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        View* pView = Injector::inject_View(m_libraryScope, k_view_simple);
        SpInteractor pIntor(Injector::inject_Interactor(m_libraryScope, WpDocument(spDoc), pView, nullptr));

        CHECK( pIntor != nullptr );
        CHECK( pIntor->get_graphic_model() != nullptr );
    }

    //-- selecting objects --------------------------------------------------------------

    TEST_FIXTURE(InteractorTestFixture, Interactor_SelectObject)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        View* pView = Injector::inject_View(m_libraryScope, k_view_simple);
        SpInteractor pIntor(Injector::inject_Interactor(m_libraryScope, WpDocument(spDoc), pView, nullptr));
        pView->set_interactor(pIntor.get());
        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoBox* pBSliceStaff = pBSliceInstr->get_child_box(0);   //SliceStaff
        GmoShape* pClef = pBSliceStaff->get_shape(0);      //Clef

        pIntor->select_object(pClef);

        CHECK( pIntor->is_in_selection(pClef) == true );
    }

    TEST_FIXTURE(InteractorTestFixture, Interactor_SelectObjectAtScreenPoint)
    {
        //as coordinates conversion is involved, the View must be rendered
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = (VerticalBookView*)Injector::inject_View(libraryScope, k_view_vertical_book);
        SpInteractor pIntor(Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr));
        pView->set_interactor(pIntor.get());
        unsigned char buf[400];
        pView->set_rendering_buffer(buf, 10, 10);
        pView->redraw_bitmap();

        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoBox* pBSliceStaff = pBSliceInstr->get_child_box(0);   //SliceStaff
        GmoShape* pClef = pBSliceStaff->get_shape(0);      //Clef
        LUnits x = (pClef->get_left() + pClef->get_right()) / 2.0f;
        LUnits y = (pClef->get_top() + pClef->get_bottom()) / 2.0f;

        double vx = x;
        double vy = y;
        pIntor->model_point_to_device(&vx, &vy, 0);

        pIntor->task_action_select_object_and_show_contextual_menu(Pixels(vx), Pixels(vy), 0);

        CHECK( pIntor->is_in_selection(pClef) == true );
    }

    TEST_FIXTURE(InteractorTestFixture, Interactor_SelectObjectsAtScreenRectangle)
    {
        //AWARE: as coordinates conversion is involved, the View must be rendered
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = (VerticalBookView*)Injector::inject_View(libraryScope, k_view_vertical_book);
        SpInteractor pIntor(Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr));
        pView->set_interactor(pIntor.get());
        unsigned char buf[400];
        pView->set_rendering_buffer(buf, 10, 10);
        pView->redraw_bitmap();

        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoBox* pBSliceStaff = pBSliceInstr->get_child_box(0);   //SliceStaff
        GmoShape* pClef = pBSliceStaff->get_shape(0);      //Clef
        LUnits xLeft = pClef->get_left() - 200.0f;
        LUnits xRight = pClef->get_right() + 200.0f;
        LUnits yTop = pClef->get_top() - 200.0f;
        LUnits yBottom = pClef->get_bottom() + 200.0f;

        double x1 = xLeft;
        double y1 = yTop;
        pIntor->model_point_to_device(&x1, &y1, 0);
        double x2 = xRight;
        double y2 = yBottom;
        pIntor->model_point_to_device(&x2, &y2, 0);

        pIntor->task_action_select_objects_in_screen_rectangle(Pixels(x1), Pixels(y1),
                                                   Pixels(x2), Pixels(y2), 0);

        CHECK( pIntor->is_in_selection(pClef) == true );
    }


    // TaskDragView ---------------------------------------------------------------------

    TEST_FIXTURE(InteractorTestFixture, TaskDragView_Creation)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->create_empty();
        View* pView = Injector::inject_View(m_libraryScope, k_view_simple);
        SpInteractor pIntor(Injector::inject_Interactor(m_libraryScope, WpDocument(spDoc), pView, nullptr));
        pView->set_interactor(pIntor.get());
        MyTaskDragView task(pIntor.get());
        task.init_task();

        CHECK( task.is_waiting_for_first_point() == true );
    }

    TEST_FIXTURE(InteractorTestFixture, TaskDragView_MouseLeftDown)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->create_empty();
        View* pView = Injector::inject_View(m_libraryScope, k_view_simple);
        SpInteractor pIntor(Injector::inject_Interactor(m_libraryScope, WpDocument(spDoc), pView, nullptr));
        pView->set_interactor(pIntor.get());
        MyTaskDragView task(pIntor.get());
        task.init_task();

        task.process_event( Event(Event::k_mouse_move) );
        CHECK( task.is_waiting_for_first_point() == true );

        task.process_event( Event(Event::k_mouse_right_down) );
        CHECK( task.is_waiting_for_first_point() == true );

        task.process_event( Event(Event::k_mouse_left_down) );
        CHECK( task.is_waiting_for_second_point() == true );
    }

    TEST_FIXTURE(InteractorTestFixture, TaskDragView_MouseMove)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( LOMSE_NEW Document(libraryScope) );
        spDoc->create_empty();
        View* pView = Injector::inject_View(libraryScope, k_view_simple);
        SpInteractor pIntor(Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr));
        pView->set_interactor(pIntor.get());
        MyTaskDragView task(pIntor.get());
        task.init_task();
        task.process_event( Event(Event::k_mouse_left_down) );

        task.process_event( Event(Event::k_mouse_move) );
        CHECK( task.is_waiting_for_second_point() == true );

        task.process_event( Event(Event::k_mouse_right_down) );
        CHECK( task.is_waiting_for_second_point() == true );

        task.process_event( Event(Event::k_mouse_left_up) );
        CHECK( task.is_waiting_for_first_point() == true );
    }

    // TaskSelection --------------------------------------------------------------------

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_Creation)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->create_empty();
        GraphicView* pView = (GraphicView*)Injector::inject_View(libraryScope, k_view_simple);
        SpInteractor pIntor(Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr));
        pView->set_interactor(pIntor.get());
        MyTaskSelection task(pIntor.get());
        task.init_task();

        CHECK( task.is_waiting_for_first_point() == true );
        CHECK( task.first_point_x() == 0 );
        CHECK( task.first_point_y() == 0 );
    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseLeftDown)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->create_empty();
        GraphicView* pView = (GraphicView*)Injector::inject_View(libraryScope, k_view_simple);
        SpInteractor pIntor(Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr));
        pView->set_interactor(pIntor.get());
        MyTaskSelection task(pIntor.get());
        task.init_task();

        task.process_event( Event(Event::k_mouse_move, 20, 70) );
        CHECK( task.is_waiting_for_first_point() == true );

        task.process_event( Event(Event::k_mouse_left_down, 10, 33, k_mouse_left) );
        CHECK( task.is_waiting_for_point_2_left() == true );
        CHECK( task.first_point_x() == 10 );
        CHECK( task.first_point_y() == 33 );
    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseLeftMove)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->create_empty();
        GraphicView* pView = (GraphicView*)Injector::inject_View(libraryScope, k_view_simple);
        SpInteractor pIntor(Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr));
        pView->set_interactor(pIntor.get());
        MyTaskSelection task(pIntor.get());
        task.init_task();
        task.process_event( Event(Event::k_mouse_left_down, 10, 33, k_mouse_left) );

        task.process_event( Event(Event::k_mouse_move, 20, 70, k_mouse_left) );
        CHECK( task.is_waiting_for_point_2_left() == true );
    }

// TODO: This test fails because now, TaskSelection switches to TaskSelectionRectangle
//    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseLeftUp)
//    {
//        MyDoorway platform;
//        LibraryScope libraryScope(cout, &platform);
//        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
//        SpDocument spDoc( new Document(libraryScope) );
//        spDoc->create_empty();
//        GraphicView* pView = Injector::inject_View(libraryScope, k_view_simple);
//        MyInteractor* pIntor = LOMSE_NEW MyInteractor(libraryScope, WpDocument(spDoc), pView);
//        SpInteractor sp(pIntor);
//        pView->set_interactor(pIntor);
//        MyTaskSelection task(pIntor);
//        task.init_task();
//        task.process_event( Event(Event::k_mouse_left_down, 10, 33, k_mouse_left) );
//        task.process_event( Event(Event::k_mouse_move, 20, 70, k_mouse_left) );
//        CHECK( pIntor->select_objects_in_rectangle_invoked() == false );
//
//        task.process_event( Event(Event::k_mouse_left_up, 21, 75, k_mouse_left) );
//        CHECK( task.is_waiting_for_first_point() == true );
//        CHECK( pIntor->select_objects_in_rectangle_invoked() == true );
//        CHECK( pIntor->sel_rectangle_is(10, 33, 21, 75) == true );
//    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseRightDown)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->create_empty();
        GraphicView* pView = (GraphicView*)Injector::inject_View(libraryScope, k_view_simple);
        SpInteractor pIntor( Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr) );
        pView->set_interactor(pIntor.get());
        MyTaskSelection task(pIntor.get());
        task.init_task();

        task.process_event( Event(Event::k_mouse_move, 20, 70) );
        CHECK( task.is_waiting_for_first_point() == true );

        task.process_event( Event(Event::k_mouse_right_down, 10, 33, k_mouse_right) );
        CHECK( task.is_waiting_for_point_2_right() == true );
        CHECK( task.first_point_x() == 10 );
        CHECK( task.first_point_y() == 33 );
    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseRightMove)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->create_empty();
        GraphicView* pView = (GraphicView*)Injector::inject_View(libraryScope, k_view_simple);
        SpInteractor pIntor(Injector::inject_Interactor(libraryScope, WpDocument(spDoc), pView, nullptr));
        pView->set_interactor(pIntor.get());
        MyTaskSelection task(pIntor.get());
        task.init_task();
        task.process_event( Event(Event::k_mouse_right_down, 10, 33, k_mouse_right) );

        task.process_event( Event(Event::k_mouse_move, 11, 35, k_mouse_right) );
        CHECK( task.is_waiting_for_point_2_right() == true );
    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseRightUp)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->create_empty();
        GraphicView* pView = (GraphicView*)Injector::inject_View(libraryScope, k_view_simple);
        MyInteractor* pIntor = LOMSE_NEW MyInteractor(libraryScope, WpDocument(spDoc), pView);
        SpInteractor sp(pIntor);
        pView->set_interactor(pIntor);
        MyTaskSelection task(pIntor);
        task.init_task();
        task.process_event( Event(Event::k_mouse_right_down, 10, 33, k_mouse_right) );
        task.process_event( Event(Event::k_mouse_move, 11, 35, k_mouse_right) );
        CHECK( pIntor->select_object_invoked() == false );

        task.process_event( Event(Event::k_mouse_right_up, 12, 34, k_mouse_right) );
        CHECK( task.is_waiting_for_first_point() == true );
        CHECK( pIntor->select_object_invoked() == true );
        CHECK( pIntor->sel_point_is(10, 33) == true );
    }


    // find_click_info_at()

    TEST_FIXTURE(InteractorTestFixture, click_info_01)
    {
        //@01 - beamed chord invokes compute_stems_directions()
        //       chord: d4,g4 - f4,c5

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00205-multimetric.lmd");
        Document* pDoc = pPresenter->get_document_raw_ptr();
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);
        pIntor->get_graphic_model();        //force to engrave the score

        //pIntor->get_graphic_model()->dump_page(0, cout);

        double x = 12600.0;
        double y = 5200.0;
        pIntor->model_point_to_device(&x, &y, 0);
        cout << "x=" << x << ", y=" << y << endl;

//        ClickPointData data = pIntor->find_click_info_at(Pixels(x), Pixels(y)); //588, 255);
        ClickPointData data = pIntor->find_click_info_at(728, 335);

        cout << "instr=" << data.ml.iInstr << ", staff=" << data.iStaff
            << ", measure=" << data.ml.iMeasure << ", location=" << data.ml.location
            << ", pImo is " << (data.pImo ? data.pImo->get_name() : "nullptr") << endl;
        CHECK( data.ml.iInstr == 1 );
        CHECK( data.iStaff == 0 );
        CHECK( data.ml.iMeasure == 3 );
        CHECK( is_equal_time(data.ml.location, 32.0) );
        CHECK( data.pImo && data.pImo->is_instrument() );
    }





    //TEST_FIXTURE(InteractorTestFixture, NotificationReceived)
    //{
    //    fNotified = false;
    //    SpDocument spDoc( new Document(m_libraryScope) );
    //    spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q)(n d4 e)(barline simple))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   k_view_simple, spDoc.get());
    //    pPresenter->set_callback( &my_callback_function );
    //    spDoc->notify_that_document_has_been_modified();

    //    CHECK( fNotified == true );

    //    delete pPresenter;
    //}


    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest)
    //{
    //    fNotified = false;
    //    SpDocument spDoc( new Document(m_libraryScope) );
    //    spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q)(n d4 e)(barline simple))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, spDoc.get());
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n c4 q)
    //    ++cursor;   //(n d4 e)
    //    CHECK( is_equal_time( cursor.time(), 64.0f ));

    //    pPresenter->insert_rest(pView, "(r q)");

    //    CHECK( fNotified == true );
    //    CHECK( (*cursor)->to_string() == "(n d4 e)" );
    //    CHECK( cursor.is_pointing_object() );
    //    CHECK( is_equal_time( cursor.time(), 128.0f ));
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string_with_ids() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q) (r q) (n d4 e) (barline simple))))" );

    //    delete pPresenter;
    //}

    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest_AtEnd)
    //{
    //    fNotified = false;
    //    SpDocument spDoc( new Document(m_libraryScope) );
    //    spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, spDoc.get());
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n c4 q)
    //    ++cursor;   //at end

    //    pPresenter->insert_rest(pView, "(r q)");

    //    CHECK( fNotified == true );
    //    CHECK( *cursor == nullptr );
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q) (r q))))" );

    //    delete pPresenter;
    //}

    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest_DifferentVoice_TimeZero)
    //{
    //    fNotified = false;
    //    SpDocument spDoc( new Document(m_libraryScope) );
    //    spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n a3 q v1))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, spDoc.get());
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n a3 q v1)

    //    pPresenter->insert_rest(pView, "(r q v2)");

    //    CHECK( fNotified == true );
    //    CHECK( *cursor == nullptr );
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n a3 q v1) (goBack start) (r q v2))))" );

    //    delete pPresenter;
    //}

    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest_DifferentVoice_OtherTime)
    //{
    //    fNotified = false;
    //    SpDocument spDoc( new Document(m_libraryScope) );
    //    spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q v1)(n a3 q v1))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, spDoc.get());
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n c4 q v1)
    //    ++cursor;   //(n a3 q v1)

    //    pPresenter->insert_rest(pView, "(r q v2)");

    //    CHECK( fNotified == true );
    //    CHECK( *cursor == nullptr );
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q v1) (n a3 q v1) (goBack start) (goFwd 64) (r q v2))))" );

    //    delete pPresenter;
    //}

}


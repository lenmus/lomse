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

    //-----------------------------------------------------------------------------------
    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    //-----------------------------------------------------------------------------------
    bool check_click_data(ClickPointData& data, int iInstr, int iStaff, int iMeasure,
                          TimeUnits location, const string& pImoName, const char* name)
    {
        bool fTestOk = data.ml.iInstr == iInstr
                       && data.iStaff == iStaff
                       && data.ml.iMeasure == iMeasure
                       && is_equal_time(data.ml.location, location)
                       && (pImoName=="nullptr" ? data.pImo==nullptr
                                               : data.pImo->get_name() == pImoName);

        if (!fTestOk)
        {
            cout << "Error in test " << name << endl
                 << "    Result: "
                 << "instr=" << data.ml.iInstr << ", staff=" << data.iStaff
                 << ", measure=" << data.ml.iMeasure << ", location=" << data.ml.location
                 << ", pImo is " << (data.pImo==nullptr ? "nullptr"
                                                        : data.pImo->get_name()) << endl
                 << "  Expected: "
                 << "instr=" << iInstr << ", staff=" << iStaff
                 << ", measure=" << iMeasure << ", location=" << location
                 << ", pImo is " << pImoName <<endl;
        }
        return fTestOk;
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


    //@ find_click_info_at() ------------------------------------------------------------
    //These are indirect test. To avoid problems with Pixels to LUnits conversions
    //instead of testing Interactor::find_click_info_at(Pixels x, Pixels y) these tests
    //are for GModelAlgorithms::find_info_for_point(), that use LUnits. Data for the
    //tests (clicked point in LUnits and the results) are obtained by using lomseapp
    //and uncommented log code in that method and should be platform independent.

    TEST_FIXTURE(InteractorTestFixture, click_info_100)
    {
        //@100. Click on document background

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00205-multimetric.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        ClickPointData data = pIntor->find_click_info_at(1, 1);

        CHECK( check_click_data(data, -1, -1, -1, 0.0, "nullptr", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_200)
    {
        //@200. click on staff after final barline

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "07012-two-instruments-four-staves.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 13453.9;
        LUnits y = 8551.37;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 1, 1, 1, 256.0, "instrument", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_201)
    {
        //@201. click on staff, second measure, after initial barline

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "07012-two-instruments-four-staves.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 8484.21;
        LUnits y = 5081.57;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 0, 1, 1, 0.0, "instrument", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_202)
    {
        //@202. click on staff, first measure, after time signature

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00205-multimetric.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 4253.3;
        LUnits y = 5283.05;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 1, 0, 0, 0, "instrument", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_203)
    {
        //@203. click on staff, second measure, after first note

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00205-multimetric.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 6827.66;
        LUnits y = 4992.03;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 1, 0, 1, 32, "instrument", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_204)
    {
        //@204. click on staff, second measure, after last note
        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00205-multimetric.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 8954.31;
        LUnits y = 5171.12;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 1, 0, 1, 128, "instrument", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_205)
    {
        //@205. click on staff, fifth measure, after last note

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00205-multimetric.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 16744.6;
        LUnits y = 5283.05;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 1, 0, 4, 128.0, "instrument", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_300)
    {
        //@300. Click point is a note

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00205-multimetric.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 11125.7;
        LUnits y = 4992.03;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 1, 0, 2, 96, "note", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_301)
    {
        //@301. Click point is a note after last barline

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00136-clef-follows-note-when-note-displaced.lms");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 7723.1;
        LUnits y = 3827.97;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 0, 0, 1, 64, "note", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_302)
    {
        //@302. Click point is a barline

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00136-clef-follows-note-when-note-displaced.lms");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 6357.74;
        LUnits y = 4979.33;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 0, 0, 1, 0, "barline", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_303)
    {
        //@303. Click point is a prolog clef in third staff (gosht image)

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "50041-octave_shift.xml");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 1992.33;
        LUnits y = 8752.84;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 0, 0, 8, 0, "clef", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_304)
    {
        //@304. Intermediate clef at start of measure but displayed at end of prev.measure

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "50041-octave_shift.xml");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 15916.3;
        LUnits y = 3223.55;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 0, 0, 3, 0, "clef", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_305)
    {
        //@305. Click point is an intermediate clef between two notes

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00136-clef-follows-note-when-note-displaced.lms");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 7275.38;
        LUnits y = 3447.41;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 0, 0, 1, 64, "clef", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_306)
    {
        //@306. Click point is courtesy key at end of system

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "50400-time-key-after-break.xml");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 19095.1;
        LUnits y = 12894.2;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 1, 0, 6, 0, "key-signature", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_401)
    {
        //@401. Click point is an AuxObj aligned with ist parent staffobj

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "02021-all-fermatas.lms");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 9402.03;
        LUnits y = 4029.44;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 0, 0, 3, 0, "fermata", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_501)
    {
        //@501. Click point is a beam

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "07012-two-instruments-four-staves.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 9200.56;
        LUnits y = 6827.66;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 1, 0, 1, 0, "beam", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_502)
    {
        //@502. Click point is a wedge starting at previous measure

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "50040-wedge.xml");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 17975.8;
        LUnits y = 9804.97;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 0, 0, 12, 0, "wedge", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_503)
    {
        //@503. Click point is a slur starting in previous system

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "50041-octave_shift.xml");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 14326.9;
        LUnits y = 9760.2;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, 0, 0, 6, 0, "octave-shift", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_600)
    {
        //@600. Click point is on the document but not on a score

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "09001-paragraph-two-scores-in-vertical.lms");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 12670.4;
        LUnits y = 2417.66;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, -1, -1, -1, 0, "paragraph", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_601)
    {
        //@601. Click point on empty space, second measure, between two notes,
        //      inmediatelly above first staff. //TODO: should provide valid locator?

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "00205-multimetric.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 7835.02;
        LUnits y = 4745.79;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, -1, -1, -1, 0, "instrument", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_602)
    {
        //@602. Click point on, second measure, space between two staves of the same
        //      instrument. //TODO: should provide valid locator?

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "07012-two-instruments-four-staves.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 10364.6;
        LUnits y = 4298.07;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, -1, -1, -1, 0, "instrument", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_603)
    {
        //@603. Click point on space before system

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "07012-two-instruments-four-staves.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 1701.32;
        LUnits y = 5954.62;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, -1, -1, -1, 0, "score", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_604)
    {
        //@604. Click point on instrument brace

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "07012-two-instruments-four-staves.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 1634.16;
        LUnits y = 7208.22;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, -1, -1, -1, 0, "instrument", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_900)
    {
        //@900. after last barline, empty space between staves: on box-system, Imo is score

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "07012-two-instruments-four-staves.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 13386.7;
        LUnits y = 7678.32;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, -1, -1, -1, 0, "score", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_901)
    {
        //@901. after last barline, empty space inmediately above/below the staff: on box-system, Imo is score
        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "07012-two-instruments-four-staves.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 13722.5;
        LUnits y = 9066.24;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, -1, -1, -1, 0, "score", test_name()) );
    }

    TEST_FIXTURE(InteractorTestFixture, click_info_902)
    {
        //@902. empty space above/below the score: on box-doc-page, Imo is lenmusdoc
        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgb24, 82);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "07012-two-instruments-four-staves.lmd");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        //get graphic model clicked object
        LUnits x = 10745.2;
        LUnits y = 11461.5;
        GraphicModel* pGM = pIntor->get_graphic_model(); //this also forces to engrave the score
        GmoObj* pGmo = pGM->hit_test(0, x, y);

        //get clicked point info
        ClickPointData data = GModelAlgorithms::find_info_for_point(x, y, pGmo);

        CHECK( check_click_data(data, -1, -1, -1, 0, "lenmusdoc", test_name()) );
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


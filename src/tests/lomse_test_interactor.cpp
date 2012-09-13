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
#include "lomse_injectors.h"
#include "lomse_presenter.h"
#include "lomse_document.h"
#include "lomse_interactor.h"
#include "lomse_graphic_view.h"
#include "lomse_tasks.h"
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
//MyDoorway: Derived class to avoid platform depentent code
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
    bool is_waiting_for_point_2_left() { return m_state == k_waiting_for_point_2_left; }
    bool is_waiting_for_point_2_right() { return m_state == k_waiting_for_point_2_right; }
    int get_state() { return m_state; }
    Pixels first_point_x() { return m_xStart; }
    Pixels first_point_y() { return m_yStart; }
};

//---------------------------------------------------------------------------------------
//MyInteractor: Mock class for tests
class MyInteractor : public EditInteractor
{
protected:
    bool m_fSelRectInvoked;
    bool m_fSelObjInvoked;
    Rectangle<Pixels> m_selRectangle;
    Point<Pixels> m_selPoint;

public:
    MyInteractor(LibraryScope& libraryScope, Document* pDoc, View* pView)
        : EditInteractor(libraryScope, pDoc, pView)
        , m_fSelRectInvoked(false)
        , m_fSelObjInvoked(false)
    {
    }
    //selection
    void select_object_at_screen_point(Pixels x, Pixels y, unsigned flags=0)
    {
        m_fSelObjInvoked = true;
        m_selPoint = Point<Pixels>(x, y);
    }
    void select_objects_in_screen_rectangle(Pixels x1, Pixels y1, Pixels x2, Pixels y2,
                                            unsigned flags=0)
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

    InteractorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
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
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        View* pView = Injector::inject_View(m_libraryScope, ViewFactory::k_view_simple,
                                            &doc);
        Interactor* pIntor = Injector::inject_Interactor(m_libraryScope, &doc, pView);

        CHECK( pIntor != NULL );
        CHECK( pIntor->get_graphic_model() != NULL );

        delete pIntor;
    }

    //-- selecting objects --------------------------------------------------------------

    TEST_FIXTURE(InteractorTestFixture, Interactor_SelectObject)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        View* pView = Injector::inject_View(m_libraryScope, ViewFactory::k_view_simple,
                                            &doc);
        Interactor* pIntor = Injector::inject_Interactor(m_libraryScope, &doc, pView);
        pView->set_interactor(pIntor);
        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoShape* pClef = pBSliceInstr->get_shape(0);      //Clef

        CHECK( pClef->is_selected() == false );
        pIntor->select_object(pClef);

        CHECK( pIntor->is_in_selection(pClef) == true );
        CHECK( pClef->is_selected() == true );

        delete pIntor;
    }

    TEST_FIXTURE(InteractorTestFixture, Interactor_SelectObjectAtScreenPoint)
    {
        //as coordinates conversion is involved, the View must be rendered
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Document doc(libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = Injector::inject_VerticalBookView(libraryScope, &doc);
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        pView->set_interactor(pIntor);
        RenderingBuffer rbuf;
        pView->set_rendering_buffer(&rbuf);
        pView->redraw_bitmap();

        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoShape* pClef = pBSliceInstr->get_shape(0);   //Clef
        LUnits x = (pClef->get_left() + pClef->get_right()) / 2.0f;
        LUnits y = (pClef->get_top() + pClef->get_bottom()) / 2.0f;

        double vx = x;
        double vy = y;
        pIntor->model_point_to_screen(&vx, &vy, 0);

        CHECK( pClef->is_selected() == false );
        pIntor->select_object_at_screen_point(Pixels(vx), Pixels(vy));

        CHECK( pIntor->is_in_selection(pClef) == true );
        CHECK( pClef->is_selected() == true );

        delete pIntor;
    }

    TEST_FIXTURE(InteractorTestFixture, Interactor_SelectObjectsAtScreenRectangle)
    {
        //AWARE: as coordinates conversion is involved, the View must be rendered
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Document doc(libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = Injector::inject_VerticalBookView(libraryScope, &doc);
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, &doc, pView);
        pView->set_interactor(pIntor);
        RenderingBuffer rbuf;
        pView->set_rendering_buffer(&rbuf);
        pView->redraw_bitmap();

        GraphicModel* pModel = pIntor->get_graphic_model();
        GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);         //System
        GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        GmoShape* pClef = pBSliceInstr->get_shape(0);   //Clef
        LUnits xLeft = pClef->get_left() - 200.0f;
        LUnits xRight = pClef->get_right() + 200.0f;
        LUnits yTop = pClef->get_top() - 200.0f;
        LUnits yBottom = pClef->get_bottom() + 200.0f;

        double x1 = xLeft;
        double y1 = yTop;
        pIntor->model_point_to_screen(&x1, &y1, 0);
        double x2 = xRight;
        double y2 = yBottom;
        pIntor->model_point_to_screen(&x2, &y2, 0);

        CHECK( pClef->is_selected() == false );
        pIntor->select_objects_in_screen_rectangle(Pixels(x1), Pixels(y1),
                                                   Pixels(x2), Pixels(y2));

        CHECK( pIntor->is_in_selection(pClef) == true );
        CHECK( pClef->is_selected() == true );

        delete pIntor;
    }


    // TaskDragView ---------------------------------------------------------------------

    TEST_FIXTURE(InteractorTestFixture, TaskDragView_Creation)
    {
        Document* pDoc = Injector::inject_Document(m_libraryScope);
        pDoc->create_empty();
        View* pView = Injector::inject_View(m_libraryScope, ViewFactory::k_view_simple,
                                            pDoc);
        Interactor* pIntor = Injector::inject_Interactor(m_libraryScope, pDoc, pView);
        pView->set_interactor(pIntor);
        MyTaskDragView task(pIntor);
        task.init_task();

        CHECK( task.is_waiting_for_first_point() == true );

        delete pDoc;
        delete pIntor;
    }

    TEST_FIXTURE(InteractorTestFixture, TaskDragView_MouseLeftDown)
    {
        Document* pDoc = Injector::inject_Document(m_libraryScope);
        pDoc->create_empty();
        View* pView = Injector::inject_View(m_libraryScope, ViewFactory::k_view_simple,
                                            pDoc);
        Interactor* pIntor = Injector::inject_Interactor(m_libraryScope, pDoc, pView);
        pView->set_interactor(pIntor);
        MyTaskDragView task(pIntor);
        task.init_task();

        task.process_event( Event(Event::k_mouse_move) );
        CHECK( task.is_waiting_for_first_point() == true );

        task.process_event( Event(Event::k_mouse_right_down) );
        CHECK( task.is_waiting_for_first_point() == true );

        task.process_event( Event(Event::k_mouse_left_down) );
        CHECK( task.is_waiting_for_second_point() == true );

        delete pDoc;
        delete pIntor;
    }

    TEST_FIXTURE(InteractorTestFixture, TaskDragView_MouseMove)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Document* pDoc= LOMSE_NEW Document(libraryScope);
        pDoc->create_empty();
        View* pView = Injector::inject_View(libraryScope, ViewFactory::k_view_simple,
                                            pDoc);
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, pDoc, pView);
        pView->set_interactor(pIntor);
        MyTaskDragView task(pIntor);
        task.init_task();
        task.process_event( Event(Event::k_mouse_left_down) );

        task.process_event( Event(Event::k_mouse_move) );
        CHECK( task.is_waiting_for_second_point() == true );

        task.process_event( Event(Event::k_mouse_right_down) );
        CHECK( task.is_waiting_for_second_point() == true );

        task.process_event( Event(Event::k_mouse_left_up) );
        CHECK( task.is_waiting_for_first_point() == true );

        delete pDoc;
        delete pIntor;
    }

    // TaskSelection --------------------------------------------------------------------

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_Creation)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Document* pDoc = Injector::inject_Document(libraryScope);
        pDoc->create_empty();
        GraphicView* pView = Injector::inject_SimpleView(libraryScope, pDoc);
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, pDoc, pView);
        pView->set_interactor(pIntor);
        MyTaskSelection task(pIntor);
        task.init_task();

        CHECK( task.is_waiting_for_first_point() == true );
        CHECK( task.first_point_x() == 0 );
        CHECK( task.first_point_y() == 0 );

        delete pDoc;
        delete pIntor;
    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseLeftDown)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Document* pDoc = Injector::inject_Document(libraryScope);
        pDoc->create_empty();
        GraphicView* pView = Injector::inject_SimpleView(libraryScope, pDoc);
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, pDoc, pView);
        pView->set_interactor(pIntor);
        MyTaskSelection task(pIntor);
        task.init_task();

        task.process_event( Event(Event::k_mouse_move, 20, 70) );
        CHECK( task.is_waiting_for_first_point() == true );

        task.process_event( Event(Event::k_mouse_left_down, 10, 33, k_mouse_left) );
        CHECK( task.is_waiting_for_point_2_left() == true );
        CHECK( task.first_point_x() == 10 );
        CHECK( task.first_point_y() == 33 );

        delete pDoc;
        delete pIntor;
    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseLeftMove)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Document* pDoc = Injector::inject_Document(libraryScope);
        pDoc->create_empty();
        GraphicView* pView = Injector::inject_SimpleView(libraryScope, pDoc);
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, pDoc, pView);
        pView->set_interactor(pIntor);
        MyTaskSelection task(pIntor);
        task.init_task();
        task.process_event( Event(Event::k_mouse_left_down, 10, 33, k_mouse_left) );

        task.process_event( Event(Event::k_mouse_move, 20, 70, k_mouse_left) );
        CHECK( task.is_waiting_for_point_2_left() == true );

        delete pDoc;
        delete pIntor;
    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseLeftUp)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Document* pDoc = Injector::inject_Document(libraryScope);
        pDoc->create_empty();
        GraphicView* pView = Injector::inject_SimpleView(libraryScope, pDoc);
        MyInteractor* pIntor = LOMSE_NEW MyInteractor(libraryScope, pDoc, pView);
        pView->set_interactor(pIntor);
        MyTaskSelection task(pIntor);
        task.init_task();
        task.process_event( Event(Event::k_mouse_left_down, 10, 33, k_mouse_left) );
        task.process_event( Event(Event::k_mouse_move, 20, 70, k_mouse_left) );
        CHECK( pIntor->select_objects_in_rectangle_invoked() == false );

        task.process_event( Event(Event::k_mouse_left_up, 21, 75, k_mouse_left) );
        CHECK( task.is_waiting_for_first_point() == true );
        CHECK( pIntor->select_objects_in_rectangle_invoked() == true );
        CHECK( pIntor->sel_rectangle_is(10, 33, 21, 75) == true );

        delete pDoc;
        delete pIntor;
    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseRightDown)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Document* pDoc = Injector::inject_Document(libraryScope);
        pDoc->create_empty();
        GraphicView* pView = Injector::inject_SimpleView(libraryScope, pDoc);
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, pDoc, pView);
        pView->set_interactor(pIntor);
        MyTaskSelection task(pIntor);
        task.init_task();

        task.process_event( Event(Event::k_mouse_move, 20, 70) );
        CHECK( task.is_waiting_for_first_point() == true );

        task.process_event( Event(Event::k_mouse_right_down, 10, 33, k_mouse_right) );
        CHECK( task.is_waiting_for_point_2_right() == true );
        CHECK( task.first_point_x() == 10 );
        CHECK( task.first_point_y() == 33 );

        delete pDoc;
        delete pIntor;
    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseRightMove)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Document* pDoc = Injector::inject_Document(libraryScope);
        pDoc->create_empty();
        GraphicView* pView = Injector::inject_SimpleView(libraryScope, pDoc);
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, pDoc, pView);
        pView->set_interactor(pIntor);
        MyTaskSelection task(pIntor);
        task.init_task();
        task.process_event( Event(Event::k_mouse_right_down, 10, 33, k_mouse_right) );

        task.process_event( Event(Event::k_mouse_move, 11, 35, k_mouse_right) );
        CHECK( task.is_waiting_for_point_2_right() == true );

        delete pDoc;
        delete pIntor;
    }

    TEST_FIXTURE(InteractorTestFixture, TaskSelection_MouseRightUp)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Document* pDoc = Injector::inject_Document(libraryScope);
        pDoc->create_empty();
        GraphicView* pView = Injector::inject_SimpleView(libraryScope, pDoc);
        MyInteractor* pIntor = LOMSE_NEW MyInteractor(libraryScope, pDoc, pView);
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

        delete pDoc;
        delete pIntor;
    }

    //TEST_FIXTURE(InteractorTestFixture, NotificationReceived)
    //{
    //    fNotified = false;
    //    Document* pDoc = Injector::inject_Document(m_libraryScope);
    //    pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q)(n d4 e)(barline simple))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   ViewFactory::k_view_simple, pDoc);
    //    pPresenter->set_callback( &my_callback_function );
    //    pDoc->notify_that_document_has_been_modified();

    //    CHECK( fNotified == true );

    //    delete pPresenter;
    //}


    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest)
    //{
    //    fNotified = false;
    //    Document* pDoc = Injector::inject_Document(m_libraryScope);
    //    pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q)(n d4 e)(barline simple))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, pDoc);
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
    //    Document* pDoc = Injector::inject_Document(m_libraryScope);
    //    pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, pDoc);
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n c4 q)
    //    ++cursor;   //at end

    //    pPresenter->insert_rest(pView, "(r q)");

    //    CHECK( fNotified == true );
    //    CHECK( *cursor == NULL );
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q) (r q))))" );

    //    delete pPresenter;
    //}

    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest_DifferentVoice_TimeZero)
    //{
    //    fNotified = false;
    //    Document* pDoc = Injector::inject_Document(m_libraryScope);
    //    pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n a3 q v1))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, pDoc);
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n a3 q v1)

    //    pPresenter->insert_rest(pView, "(r q v2)");

    //    CHECK( fNotified == true );
    //    CHECK( *cursor == NULL );
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n a3 q v1) (goBack start) (r q v2))))" );

    //    delete pPresenter;
    //}

    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest_DifferentVoice_OtherTime)
    //{
    //    fNotified = false;
    //    Document* pDoc = Injector::inject_Document(m_libraryScope);
    //    pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q v1)(n a3 q v1))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, pDoc);
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n c4 q v1)
    //    ++cursor;   //(n a3 q v1)

    //    pPresenter->insert_rest(pView, "(r q v2)");

    //    CHECK( fNotified == true );
    //    CHECK( *cursor == NULL );
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q v1) (n a3 q v1) (goBack start) (goFwd 64) (r q v2))))" );

    //    delete pPresenter;
    //}

}


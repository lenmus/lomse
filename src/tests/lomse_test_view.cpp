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
#include "private/lomse_document_p.h"
#include "lomse_graphic_view.h"
#include "lomse_doorway.h"
#include "lomse_screen_drawer.h"
#include "lomse_interactor.h"

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
//helper, to have access to protected/private members
class MyVerticalView : public VerticalBookView
{
public:
    MyVerticalView(LibraryScope& libraryScope, ScreenDrawer* pDrawer)
        : VerticalBookView(libraryScope, pDrawer)
    {
    }
    virtual ~MyVerticalView() {}

    list<URect>& my_get_page_bounds() { return m_pageBounds; }
    bool my_shift_right_x_to_be_on_page(double* xLeft)
    {
        return shift_right_x_to_be_on_page(xLeft);
    }
    bool my_shift_left_x_to_be_on_page(double* xRight)
    {
        return shift_left_x_to_be_on_page(xRight);
    }
    bool my_shift_down_y_to_be_on_page(double* yTop)
    {
        return shift_down_y_to_be_on_page(yTop);
    }
    bool my_shift_up_y_to_be_on_page(double* yBottom)
    {
        return shift_up_y_to_be_on_page(yBottom);
    }
    void my_trimmed_rectangle_to_page_rectangles(list<PageRectangle*>* rectangles,
                                                 double xLeft, double yTop,
                                                 double xRight, double yBottom)
    {
        trimmed_rectangle_to_page_rectangles(rectangles, xLeft, yTop, xRight, yBottom);
    }
    void my_normalize_rectangle(double* xLeft, double* yTop,
                                double* xRight, double* yBottom)
    {
        normalize_rectangle(xLeft, yTop, xRight, yBottom);
    }

};

//---------------------------------------------------------------------------------------
class GraphicViewTestFixture
{
public:
    std::string m_scores_path;

    GraphicViewTestFixture()     //SetUp fixture
    {
        m_scores_path = TESTLIB_SCORES_PATH;
    }

    ~GraphicViewTestFixture()    //TearDown fixture
    {
    }
};


SUITE(GraphicViewTest)
{

    //-- coordinates conversion ---------------------------------------------------------

    TEST_FIXTURE(GraphicViewTestFixture, EditView_ScreenPointToPage_None)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = Injector::inject_VerticalBookView(libraryScope, spDoc.get());
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, spDoc, pView, nullptr);
        pView->set_interactor(pIntor);
        RenderingBuffer rbuf;
        pView->set_rendering_buffer(&rbuf);
        pView->redraw_bitmap();

        //page top-left corner is placed, in vertical book view, at (0+, 0+) pixels
        VPoint screen(3, 3);
        CHECK( pView->page_at_screen_point(double(screen.x), double(screen.y)) == 0 );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicViewTestFixture, EditView_ScreenPointToPage_First)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = Injector::inject_VerticalBookView(libraryScope, spDoc.get());
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, spDoc, pView, nullptr);
        pView->set_interactor(pIntor);
        RenderingBuffer rbuf;
        pView->set_rendering_buffer(&rbuf);
        pView->redraw_bitmap();

        VPoint screen(100, 100);
        CHECK( pView->page_at_screen_point(double(screen.x), double(screen.y)) == 0 );

        delete pIntor;
    }

//    TEST_FIXTURE(GraphicViewTestFixture, EditView_ScreenPointToPage_Second)
//    {
//        MyDoorway platform;
//        LibraryScope libraryScope(cout, &platform);
//        SpDocument spDoc( new Document(libraryScope) );
//        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
//        VerticalBookView* pView = Injector::inject_VerticalBookView(libraryScope, spDoc);
//        Interactor* pIntor = Injector::inject_Interactor(libraryScope, spDoc, pView, nullptr);
//        pView->set_interactor(pIntor);
//        RenderingBuffer rbuf;
//        pView->set_rendering_buffer(&rbuf);
//        pView->redraw_bitmap();
//
//        VPoint screen(100, 600);
//        CHECK( pView->page_at_screen_point(double(screen.x), double(screen.y)) == 1 );
//
//        delete pIntor;
//    }

    TEST_FIXTURE(GraphicViewTestFixture, VerticalView_ModelToScreen)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = Injector::inject_VerticalBookView(libraryScope, spDoc.get());
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, spDoc, pView, nullptr);
        pView->set_interactor(pIntor);
        RenderingBuffer rbuf;
        pView->set_rendering_buffer(&rbuf);
        pView->redraw_bitmap();

        //page top-left corner is placed, in vertical book view, at (0+, 0+)
        double vx = 0.0;
        double vy = 0.0;
        int iPage = 0;
        pIntor->model_point_to_screen(&vx, &vy, iPage);
        Pixels x = Pixels(vx);
        Pixels y = Pixels(vy);

//        cout << "x=" << x << ", y=" << y << endl;
        CHECK( x == 0 );
        CHECK( y == 0 );

        delete pIntor;
    }

    TEST_FIXTURE(GraphicViewTestFixture, VerticalView_ScreenToModel)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        SpDocument spDoc( new Document(libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = Injector::inject_VerticalBookView(libraryScope, spDoc.get());
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, spDoc, pView, nullptr);
        pView->set_interactor(pIntor);
        RenderingBuffer rbuf;
        pView->set_rendering_buffer(&rbuf);
        pView->redraw_bitmap();

        //page top-left corner is placed, in vertical book view, at (0+, 0+)
        double vx = 1.0;
        double vy = 1.0;
        pIntor->screen_point_to_page_point(&vx, &vy);

//        cout << "vx=" << vx << ", vy=" << vy << endl;
        CHECK( vx > 21.0  && vx < 31.0);       //tolerance: 10 LUnits
        CHECK( vy > 21.0  && vy < 31.0);

        delete pIntor;
    }

    // normalize_rectangle --------------------------------------------------------------

    TEST_FIXTURE(GraphicViewTestFixture, normalize_rectangle_1)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);

        double xLeft = 10.0;
        double yTop = 12.0;
        double xRight = 14.0;
        double yBottom = 26.0;
        view.my_normalize_rectangle(&xLeft, &yTop, &xRight, &yBottom);

        CHECK( xLeft == 10.0 );
        CHECK( yTop == 12.0 );
        CHECK( xRight == 14.0 );
        CHECK( yBottom == 26.0 );
    }

    TEST_FIXTURE(GraphicViewTestFixture, normalize_rectangle_2)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);

        double xLeft = 14.0;
        double yTop = 26.0;
        double xRight = 10.0;
        double yBottom = 12.0;
        view.my_normalize_rectangle(&xLeft, &yTop, &xRight, &yBottom);

        CHECK( xLeft == 10.0 );
        CHECK( yTop == 12.0 );
        CHECK( xRight == 14.0 );
        CHECK( yBottom == 26.0 );
    }

    TEST_FIXTURE(GraphicViewTestFixture, normalize_rectangle_3)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);

        double xLeft = 10.0;
        double yTop = 26.0;
        double xRight = 14.0;
        double yBottom = 12.0;
        view.my_normalize_rectangle(&xLeft, &yTop, &xRight, &yBottom);

        CHECK( xLeft == 10.0 );
        CHECK( yTop == 12.0 );
        CHECK( xRight == 14.0 );
        CHECK( yBottom == 26.0 );
    }

    TEST_FIXTURE(GraphicViewTestFixture, normalize_rectangle_4)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);

        double xLeft = 14.0;
        double yTop = 12.0;
        double xRight = 10.0;
        double yBottom = 26.0;
        view.my_normalize_rectangle(&xLeft, &yTop, &xRight, &yBottom);

        CHECK( xLeft == 10.0 );
        CHECK( yTop == 12.0 );
        CHECK( xRight == 14.0 );
        CHECK( yBottom == 26.0 );
    }

    TEST_FIXTURE(GraphicViewTestFixture, shift_right_x_to_be_on_page)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);
        list<URect>& pageBounds = view.my_get_page_bounds();
        pageBounds.push_back( URect(0, 0, 100, 200) );
        pageBounds.push_back( URect(0, 210, 100, 200) );
        pageBounds.push_back( URect(0, 420, 100, 200) );

        double xLeft = -10.0;
        CHECK( view.my_shift_right_x_to_be_on_page(&xLeft) == true );
        CHECK( xLeft == 0.0 );

        xLeft = 10.0;
        CHECK( view.my_shift_right_x_to_be_on_page(&xLeft) == true );
        CHECK( xLeft == 10.0 );

        xLeft = 110.0;
        CHECK( view.my_shift_right_x_to_be_on_page(&xLeft) == false );
        CHECK( xLeft == 110.0 );
    }

    TEST_FIXTURE(GraphicViewTestFixture, shift_left_x_to_be_on_page)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);
        list<URect>& pageBounds = view.my_get_page_bounds();
        pageBounds.push_back( URect(0, 0, 100, 200) );
        pageBounds.push_back( URect(0, 210, 100, 200) );
        pageBounds.push_back( URect(0, 420, 100, 200) );

        double xRight = 150.0;
        CHECK( view.my_shift_left_x_to_be_on_page(&xRight) == true );
        CHECK( xRight == 100.0 );

        xRight = 70.0;
        CHECK( view.my_shift_left_x_to_be_on_page(&xRight) == true );
        CHECK( xRight == 70.0 );

        xRight = -10.0;
        CHECK( view.my_shift_left_x_to_be_on_page(&xRight) == false );
        CHECK( xRight == -10.0 );
    }

    TEST_FIXTURE(GraphicViewTestFixture, shift_down_y_to_be_on_page)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);
        list<URect>& pageBounds = view.my_get_page_bounds();
        pageBounds.push_back( URect(0, 0, 100, 200) );
        pageBounds.push_back( URect(0, 210, 100, 200) );
        pageBounds.push_back( URect(0, 420, 100, 200) );

        double yTop = -10.0;
        CHECK( view.my_shift_down_y_to_be_on_page(&yTop) == true );
        CHECK( yTop == 0.0 );

        yTop = 70.0;
        CHECK( view.my_shift_down_y_to_be_on_page(&yTop) == true );
        CHECK( yTop == 70.0 );

        yTop = 415.0;
        CHECK( view.my_shift_down_y_to_be_on_page(&yTop) == true );
        CHECK( yTop == 420.0 );

        yTop = 700.0;
        CHECK( view.my_shift_down_y_to_be_on_page(&yTop) == false );
        CHECK( yTop == 700.0 );
    }

    TEST_FIXTURE(GraphicViewTestFixture, shift_up_y_to_be_on_page)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);
        list<URect>& pageBounds = view.my_get_page_bounds();
        pageBounds.push_back( URect(0, 0, 100, 200) );
        pageBounds.push_back( URect(0, 210, 100, 200) );
        pageBounds.push_back( URect(0, 420, 100, 200) );

        double yBottom = -10.0;
        CHECK( view.my_shift_up_y_to_be_on_page(&yBottom) == false );
        CHECK( yBottom == -10.0 );

        yBottom = 70.0;
        CHECK( view.my_shift_up_y_to_be_on_page(&yBottom) == true );
        CHECK( yBottom == 70.0 );

        yBottom = 415.0;
        CHECK( view.my_shift_up_y_to_be_on_page(&yBottom) == true );
        CHECK( yBottom == 410.0 );

        yBottom = 700.0;
        CHECK( view.my_shift_up_y_to_be_on_page(&yBottom) == true );
        CHECK( yBottom == 620.0 );
    }

    TEST_FIXTURE(GraphicViewTestFixture, trim_rectangle_to_be_on_pages_ok)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);
        list<URect>& pageBounds = view.my_get_page_bounds();
        pageBounds.push_back( URect(0, 0, 100, 200) );
        pageBounds.push_back( URect(0, 210, 100, 200) );
        pageBounds.push_back( URect(0, 420, 100, 200) );

        double xLeft = -10.0;
        double yTop = 10.0;
        double xRight = 50.0;
        double yBottom = 500.0;
        CHECK( view.trim_rectangle_to_be_on_pages(&xLeft, &yTop, &xRight, &yBottom) == true );
        CHECK( xLeft == 0.0 );
        CHECK( yTop == 10.0 );
        CHECK( xRight == 50.0 );
        CHECK( yBottom == 500.0 );
//        cout << "left=" << xLeft << ", top=" << yTop << ", right=" << xRight << ", bottom=" << yBottom << endl;
    }

    TEST_FIXTURE(GraphicViewTestFixture, trim_rectangle_to_be_on_pages_out)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);
        list<URect>& pageBounds = view.my_get_page_bounds();
        pageBounds.push_back( URect(0, 0, 100, 200) );
        pageBounds.push_back( URect(0, 210, 100, 200) );
        pageBounds.push_back( URect(0, 420, 100, 200) );

        double xLeft = 110.0;
        double yTop =  10.0;
        double xRight = 150.0;
        double yBottom = 500.0;
        CHECK( view.trim_rectangle_to_be_on_pages(&xLeft, &yTop, &xRight, &yBottom) == false );
        CHECK( xLeft == 110.0 );
        CHECK( yTop == 10.0 );
        CHECK( xRight == 100.0 );
        CHECK( yBottom == 500.0 );
//        cout << "left=" << xLeft << ", top=" << yTop << ", right=" << xRight << ", bottom=" << yBottom << endl;
    }

    TEST_FIXTURE(GraphicViewTestFixture, trimmed_rectangle_to_page_rectangles)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);
        list<URect>& pageBounds = view.my_get_page_bounds();
        pageBounds.push_back( URect(0, 0, 100, 200) );
        pageBounds.push_back( URect(0, 210, 100, 200) );
        pageBounds.push_back( URect(0, 420, 100, 200) );

        list<PageRectangle*> rectangles;
        view.my_trimmed_rectangle_to_page_rectangles(&rectangles, 0.0, 10.0, 50.0, 500.0);

        CHECK( rectangles.size() == 3 );
//        cout << "num.rectangles=" << rectangles.size() << endl;
        list<PageRectangle*>::iterator it = rectangles.begin();
        CHECK( (*it)->iPage == 0 );
        CHECK( (*it)->rect.left() == 0.0f );
        CHECK( (*it)->rect.top() == 10.0f );
        CHECK( (*it)->rect.right() == 50.0f );
        CHECK( (*it)->rect.bottom() == 200.0f );
        //cout << "LT=(" << (*it)->rect.left() << ", " << (*it)->rect.top() <<
        //    "), RB=(" << (*it)->rect.right() << ", " << (*it)->rect.bottom() << ")" << endl;
        delete *it;
        ++it;
        CHECK( (*it)->iPage == 1 );
        CHECK( (*it)->rect.left() == 0.0f );
        CHECK( (*it)->rect.top() == 0.0f );
        CHECK( (*it)->rect.right() == 50.0f );
        CHECK( (*it)->rect.bottom() == 200.0f );
        //cout << "LT=(" << (*it)->rect.left() << ", " << (*it)->rect.top() <<
        //    "), RB=(" << (*it)->rect.right() << ", " << (*it)->rect.bottom() << ")" << endl;
        delete *it;
        ++it;
        CHECK( (*it)->iPage == 2 );
        CHECK( (*it)->rect.left() == 0.0f );
        CHECK( (*it)->rect.top() == 0.0f );
        CHECK( (*it)->rect.right() == 50.0f );
        CHECK( (*it)->rect.bottom() == 80.0f );
        //cout << "LT=(" << (*it)->rect.left() << ", " << (*it)->rect.top() <<
        //    "), RB=(" << (*it)->rect.right() << ", " << (*it)->rect.bottom() << ")" << endl;
        delete *it;

        rectangles.clear();
    }

    TEST_FIXTURE(GraphicViewTestFixture, trimmed_rectangle_to_page_rectangles_2)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
        MyVerticalView view(libraryScope, pDrawer);
        list<URect>& pageBounds = view.my_get_page_bounds();
        pageBounds.push_back( URect(0, 0, 100, 200) );
        pageBounds.push_back( URect(0, 210, 100, 200) );
        pageBounds.push_back( URect(0, 420, 100, 200) );

        list<PageRectangle*> rectangles;
        view.my_trimmed_rectangle_to_page_rectangles(&rectangles, 0.0, 300.0, 50.0, 500.0);

        CHECK( rectangles.size() == 2 );
//        cout << "num.rectangles=" << rectangles.size() << endl;
        list<PageRectangle*>::iterator it = rectangles.begin();
        CHECK( (*it)->iPage == 1 );
        CHECK( (*it)->rect.left() == 0.0f );
        CHECK( (*it)->rect.top() == 90.0f );
        CHECK( (*it)->rect.right() == 50.0f );
        CHECK( (*it)->rect.bottom() == 200.0f );
        //cout << "LT=(" << (*it)->rect.left() << ", " << (*it)->rect.top() <<
        //    "), RB=(" << (*it)->rect.right() << ", " << (*it)->rect.bottom() << ")" << endl;
        delete *it;
        ++it;
        CHECK( (*it)->iPage == 2 );
        CHECK( (*it)->rect.left() == 0.0f );
        CHECK( (*it)->rect.top() == 0.0f );
        CHECK( (*it)->rect.right() == 50.0f );
        CHECK( (*it)->rect.bottom() == 80.0f );
        //cout << "LT=(" << (*it)->rect.left() << ", " << (*it)->rect.top() <<
        //    "), RB=(" << (*it)->rect.right() << ", " << (*it)->rect.bottom() << ")" << endl;
        delete *it;

        rectangles.clear();
    }

    //TEST_FIXTURE(GraphicViewTestFixture, EditView_UpdateWindow)
    //{
    //    MyDoorway platform;
    //    LibraryScope libraryScope(cout, &platform);
    //    SpDocument spDoc( new Document(libraryScope) );
    //    spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
    //        "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
    //    VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
    //        Injector::inject_View(libraryScope, k_view_vertical_book, spDoc) );
    //    CHECK( m_platform.update_window_invoked() == false );
    //    pView->request_window_update();
    //    CHECK( m_platform.update_window_invoked() == true );
    //    delete pView;
    //}

    //TEST_FIXTURE(GraphicViewTestFixture, EditView_CreatesBitmap)
    //{
    //    MyDoorway platform;
    //    LibraryScope libraryScope(cout, &platform);
    //    SpDocument spDoc( new Document(libraryScope) );
    //    spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
    //        "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
    //    VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
    //        Injector::inject_View(libraryScope, k_view_vertical_book, spDoc) );
    //    pView->on_draw();
    //    CHECK( ????????????????????????? );
    //    delete pView;
    //}

}


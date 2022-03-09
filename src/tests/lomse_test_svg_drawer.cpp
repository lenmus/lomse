//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2022. All rights reserved.
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
#include "lomse_svg_drawer.h"

//to get output using test scores
#include <lomse_doorway.h>
#include <lomse_graphic_view.h>     //for k_view_vertical_book
#include <lomse_presenter.h>        //Presenter
#include <lomse_interactor.h>       //Interactor

//to test shapes
#include <lomse_glyphs.h>
#include <private/lomse_internal_model_p.h>     //enums
#include <lomse_im_factory.h>
#include <lomse_shape_barline.h>
#include <lomse_shape_beam.h>
#include <lomse_shape_brace_bracket.h>
#include <lomse_shape_line.h>
#include <lomse_shape_note.h>
#include <lomse_shape_staff.h>


//std
#include <list>

using namespace UnitTest;
using namespace std;
using namespace lomse;

////---------------------------------------------------------------------------------------
//class MyImoObj : public ImoDto
//{
//public:
//    MyImoObj() : ImoSimpleObj(k_imo_arpeggio_dto) {}
//
//}

//---------------------------------------------------------------------------------------
class SvgDrawerTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    SvgDrawerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~SvgDrawerTestFixture()    //TearDown fixture
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    void run_test_for(GmoShape& shape, const stringstream& expected)
    {
        RenderOptions ropts;
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        options.indent = 3;
        options.add_newlines = true;
        shape.on_draw(&drawer, ropts);

        check_expected(ss.str(), expected.str());
    }

    void check_expected(const string& ss, const string& expected)
    {
        if (ss != expected)
        {
            CHECK( false );
            cout << "  result=[" << ss << "]" << endl;
            cout << "expected=[" << expected << "]" << endl;
        }
    }
};


SUITE(SvgDrawerTest)
{

    //@ initial steps -------------------------------------------------------------------

    TEST_FIXTURE(SvgDrawerTestFixture, initial_01)
    {
        //@01. svg tag generated, including viewBox
        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgba32, 96);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.new_document(k_view_single_page);
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

        stringstream svg;
        int page = 0;
        pIntor->render_as_svg(svg, page);

        stringstream expected;
        expected << "<svg xmlns='http://www.w3.org/2000/svg' version='1.1' "
            << "viewBox='0 0 21000 4000'><path d=' M 0 0 H 21000 V 4000 H 0 V 0' "
            << "fill='#fff' stroke='#fff'/></svg>";
        check_expected(svg.str(), expected.str());
    }


    //@ options -------------------------------------------------------------------------

    TEST_FIXTURE(SvgDrawerTestFixture, options_01)
    {
        //@01. composite notation by default neither adds id nor class
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        drawer.start_composite_notation("m34", "staff");
        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);
        drawer.end_composite_notation();

        stringstream expected;
        expected << "<g><rect x='50' y='70' width='130' height='176'/></g>";
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, options_02)
    {
        //@02. composite notation adds id
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        drawer.start_composite_notation("m34", "staff");
        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);
        drawer.end_composite_notation();

        stringstream expected;
        expected << "<g id='m34'><rect x='50' y='70' width='130' height='176'/></g>";
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, options_03)
    {
        //@03. composite notation adds class
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_class = true;
        drawer.start_composite_notation("m34", "staff");
        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);
        drawer.end_composite_notation();

        stringstream expected;
        expected << "<g class='staff'><rect x='50' y='70' width='130' height='176'/></g>";
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, options_04)
    {
        //@04. composite notation adds id and class
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        drawer.start_composite_notation("m34", "staff");
        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);
        drawer.end_composite_notation();

        stringstream expected;
        expected << "<g id='m34' class='staff'><rect x='50' y='70' width='130' height='176'/></g>";
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, options_05)
    {
        //@05. simple notation adds id and class
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        drawer.start_simple_notation("m34", "staff");
        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);

        stringstream expected;
        expected << "<rect id='m34' class='staff' x='50' y='70' width='130' height='176'/>";
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, options_06)
    {
        //@06. used id and class are removed
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        drawer.start_simple_notation("m30", "rectangle");
        drawer.rect(UPoint(40.0, 60.0), USize(120.0, 100.0), 0.0);
        drawer.start_composite_notation("m34", "staff");
        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);
        drawer.end_composite_notation();

        stringstream expected;
        expected << "<rect id='m30' class='rectangle' x='40' y='60' width='120' height='100'/>"
            << "<g id='m34' class='staff'><rect x='50' y='70' width='130' height='176'/></g>";
        CHECK( ss.str() == expected.str() );
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, options_07)
    {
        //@07. indentation added
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.indent = 3;
        options.add_id = true;
        options.add_class = true;
        drawer.start_simple_notation("m30", "rectangle");
        drawer.rect(UPoint(40.0, 60.0), USize(120.0, 100.0), 0.0);
        drawer.start_composite_notation("m34", "staff");
        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);
        drawer.end_composite_notation();

        stringstream expected;
        expected << "<rect id='m30' class='rectangle' x='40' y='60' width='120' height='100'/>"
            << "<g id='m34' class='staff'>   <rect x='50' y='70' width='130' height='176'/></g>";
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, options_08)
    {
        //@08. indentation and new line added
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.indent = 3;
        options.add_newlines = true;
        drawer.start_simple_notation("m30", "rectangle");
        drawer.rect(UPoint(40.0, 60.0), USize(120.0, 100.0), 0.0);
        drawer.start_composite_notation("m34", "staff");
        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);
        drawer.end_composite_notation();

        stringstream expected;
        expected << "<rect x='40' y='60' width='120' height='100'/>" << endl
            << "<g>" << endl
            << "   <rect x='50' y='70' width='130' height='176'/>" << endl
            << "</g>" << endl;
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, options_09)
    {
        //@09. second level group with indentation
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.indent = 3;
        options.add_id = true;
        options.add_class = true;
        options.add_newlines = true;
        drawer.start_composite_notation("m30", "rectangle");
        drawer.rect(UPoint(40.0, 60.0), USize(120.0, 100.0), 0.0);
        drawer.start_composite_notation("m34", "staff");
        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);
        drawer.end_composite_notation();

        stringstream expected;
        expected
            << "<g id='m30' class='rectangle'>" << endl
            << "   <rect x='40' y='60' width='120' height='100'/>" << endl
            << "   <g id='m34' class='staff'>" << endl
            << "      <rect x='50' y='70' width='130' height='176'/>" << endl
            << "   </g>" << endl;
        check_expected(ss.str(), expected.str());
    }


    //@ circle --------------------------------------------------------------------------
    TEST_FIXTURE(SvgDrawerTestFixture, circle_01)
    {
        //@01. indentation and new line added
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        options.indent = 3;
        options.add_newlines = true;
        drawer.start_composite_notation("m34", "staff");
        drawer.circle(50.0, 70.0, 20.0);
        drawer.end_composite_notation();
        drawer.start_simple_notation("m30", "rectangle");
        drawer.circle(40.0, 60.0, 10.0);

        stringstream expected;
        expected << "<g id='m34' class='staff'>" << endl
            << "   <circle cx='50' cy='70' r='20'/>" << endl
            << "</g>" << endl
            << "<circle id='m30' class='rectangle' cx='40' cy='60' r='10'/>" << endl;
        check_expected(ss.str(), expected.str());
    }


    //@ draw_glyph ----------------------------------------------------------------------
    TEST_FIXTURE(SvgDrawerTestFixture, draw_glyph_01)
    {
        //@01. indentation and new line added
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        options.indent = 3;
        options.add_newlines = true;
        drawer.start_composite_notation("m34", "staff");
        drawer.draw_glyph(50.0, 70.0, 'a');
        drawer.end_composite_notation();
        drawer.start_simple_notation("m30", "rectangle");
        drawer.draw_glyph(40.0, 60.0, 'b');

        stringstream expected;
        expected << "<g id='m34' class='staff'>" << endl
            << "   <text x='50' y='70' fill='#000' font-family='Bravura' font-size='352.778'>&#97;</text>" << endl
            << "</g>" << endl
            << "<text id='m30' class='rectangle' x='40' y='60' fill='#000' font-family='Bravura' font-size='352.778'>&#98;</text>" << endl;
        check_expected(ss.str(), expected.str());
    }


    //@ draw_glyph_rotated --------------------------------------------------------------
    TEST_FIXTURE(SvgDrawerTestFixture, draw_glyph_rotated_01)
    {
        //@01. indentation and new line added
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        options.indent = 3;
        options.add_newlines = true;
        drawer.start_composite_notation("m34", "staff");
        drawer.draw_glyph_rotated(50.0, 70.0, 'a', 3.141592654/2.0);
        drawer.end_composite_notation();
        drawer.start_simple_notation("m30", "rectangle");
        drawer.draw_glyph_rotated(40.0, 60.0, 'b', 3.141592654/2.0);

        stringstream expected;
        expected << "<g id='m34' class='staff'>" << endl
            << "   <text x='50' y='70' fill='#000' transform='rotate(90,50,70)' font-family='Bravura' font-size='352.778'>&#97;</text>" << endl
            << "</g>" << endl
            << "<text id='m30' class='rectangle' x='40' y='60' fill='#000' transform='rotate(90,40,60)' font-family='Bravura' font-size='352.778'>&#98;</text>" << endl;
        check_expected(ss.str(), expected.str());
    }


    //@ draw_text -----------------------------------------------------------------------
    TEST_FIXTURE(SvgDrawerTestFixture, draw_text_01)
    {
        //@01. indentation and new line added
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        options.indent = 3;
        options.add_newlines = true;
        drawer.start_composite_notation("m34", "staff");
        drawer.draw_text(50.0, 70.0, "hello");
        drawer.end_composite_notation();
        drawer.start_simple_notation("m30", "rectangle");
        drawer.draw_text(40.0, 60.0, "world");

        stringstream expected;
        expected << "<g id='m34' class='staff'>" << endl
            << "   <text x='50' y='70' fill='#000' font-family='Bravura' font-size='352.778'>hello</text>" << endl
            << "</g>" << endl
            << "<text id='m30' class='rectangle' x='40' y='60' fill='#000' font-family='Bravura' font-size='352.778'>world</text>" << endl;
        check_expected(ss.str(), expected.str());
    }


    //@ line ----------------------------------------------------------------------------
    TEST_FIXTURE(SvgDrawerTestFixture, line_01)
    {
        //@01. indentation and new line added
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        options.indent = 3;
        options.add_newlines = true;
        drawer.start_composite_notation("m34", "staff");
        drawer.begin_path();
        drawer.line(50.0, 70.0, 20.0, 30.0, 5.0);
        drawer.end_path();
        drawer.end_composite_notation();
        drawer.start_simple_notation("m30", "rectangle");
        drawer.begin_path();
        drawer.line(40.0, 60.0, 10.0, 35.0, 7.0);
        drawer.end_path();

        stringstream expected;
        expected << "<g id='m34' class='staff'>" << endl
            << "   <path d=' M 52 68.5 L 48 71.5 L 18 31.5 L 22 28.5'/>" << endl
            << "</g>" << endl
            << "<path id='m30' class='rectangle' d=' M 42.2406 57.3112 L 37.7594 62.6888 L 7.75935 37.6888 L 12.2406 32.3112'/>" << endl;
        check_expected(ss.str(), expected.str());
    }


    //@ line_with_markers ---------------------------------------------------------------
    TEST_FIXTURE(SvgDrawerTestFixture, line_with_markers_01)
    {
        //@01. indentation and new line added
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        options.indent = 3;
        options.add_newlines = true;
        drawer.start_composite_notation("m34", "staff");
        drawer.begin_path();
        drawer.line_with_markers(UPoint(50.0, 70.0), UPoint(20.0, 30.0), 5.0, k_cap_none, k_cap_none);
        drawer.end_path();
        drawer.end_composite_notation();
        drawer.start_simple_notation("m30", "rectangle");
        drawer.begin_path();
        drawer.line_with_markers(UPoint(40.0, 60.0), UPoint(10.0, 35.0), 7.0, k_cap_none, k_cap_none);
        drawer.end_path();

        stringstream expected;
        expected << "<g id='m34' class='staff'>" << endl
            << "   <path d=' M 52 68.5 L 48 71.5 L 18 31.5 L 22 28.5 Z'/>" << endl
            << "</g>" << endl
            << "<path id='m30' class='rectangle' d=' M 42.2406 57.3112 L 37.7594 62.6888 L 7.75935 37.6888 L 12.2406 32.3112 Z'/>" << endl;
        check_expected(ss.str(), expected.str());
    }


    //@ path ----------------------------------------------------------------------------

    TEST_FIXTURE(SvgDrawerTestFixture, path_01)
    {
        //@01. end_path() generates the svg code
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        drawer.begin_path();
        drawer.stroke(Color(0,0,0));
        drawer.stroke_width(50.0);
        drawer.move_to(2000, 3000);
        drawer.line_to(18000, 3300);
        drawer.end_path();

        stringstream expected;
        expected << "<path d=' M 2000 3000 L 18000 3300' stroke='#000' stroke-width='50'/>";
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, path_02)
    {
        //@02. indentation and new line added
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        options.indent = 3;
        options.add_newlines = true;

        drawer.start_composite_notation("m34", "staff");
        drawer.begin_path();
        drawer.move_to(2000, 3000);
        drawer.line_to(18000, 3300);
        drawer.end_path();
        drawer.end_composite_notation();

        drawer.start_simple_notation("m30", "rectangle");
        drawer.begin_path();
        drawer.move_to(20, 30);
        drawer.line_to(180, 33);
        drawer.end_path();


        stringstream expected;
        expected << "<g id='m34' class='staff'>" << endl
            << "   <path d=' M 2000 3000 L 18000 3300'/>" << endl
            << "</g>" << endl
            << "<path id='m30' class='rectangle' d=' M 20 30 L 180 33'/>" << endl;
        check_expected(ss.str(), expected.str());
    }


    //@ rect ----------------------------------------------------------------------------

    TEST_FIXTURE(SvgDrawerTestFixture, rect_01)
    {
        //@01. rect() when radius is zero
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);

        stringstream expected;
        expected << "<rect x='50' y='70' width='130' height='176'/>";
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, rect_02)
    {
        //@02. rect() when radius not zero
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 10.0);

        stringstream expected;
        expected << "<rect x='50' y='70' width='130' height='176' rx='10' ry='10'/>";
        check_expected(ss.str(), expected.str());
    }

    TEST_FIXTURE(SvgDrawerTestFixture, rect_03)
    {
        //@03. indentation and new line added
        stringstream ss;
        SvgOptions options;
        SvgDrawer drawer(m_libraryScope, ss, options);

        options.add_id = true;
        options.add_class = true;
        options.indent = 3;
        options.add_newlines = true;
        drawer.start_composite_notation("m34", "staff");
        drawer.rect(UPoint(50.0, 70.0), USize(130.0, 176.0), 0.0);
        drawer.end_composite_notation();
        drawer.start_simple_notation("m30", "rectangle");
        drawer.rect(UPoint(40.0, 60.0), USize(120.0, 100.0), 0.0);

        stringstream expected;
        expected << "<g id='m34' class='staff'>" << endl
            << "   <rect x='50' y='70' width='130' height='176'/>" << endl
            << "</g>" << endl
            << "<rect id='m30' class='rectangle' x='40' y='60' width='120' height='100'/>" << endl;
        check_expected(ss.str(), expected.str());
    }


    //@ shapes --------------------------------------------------------------------------

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_100)
    {
        //@100 shape GmoShapeAccidentals. Empty container
        ImoArpeggioDto imo;
        imo.set_id(83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeAccidentals shape(&imo, 0, pos, Color(0,0,0));

        stringstream expected;

        run_test_for(shape, expected);
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_101)
    {
        //@101 shape GmoShapeAccidentals. Has accidentals
        ImoArpeggioDto imo;
        imo.set_id(83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeAccidentals shape(&imo, 0, pos, Color(0,0,0));
        ImoArpeggioDto imo2;
        imo2.set_id(84);
        UPoint pos2(300.0f, 600.0f);
        GmoShapeAccidental* shape2 =
            LOMSE_NEW GmoShapeAccidental(&imo2, 0, k_glyph_accidentalNaturalSharp, pos,
                                         Color(0,0,0), m_libraryScope, 21.0);
        shape.add(shape2);

        stringstream expected;
        expected << "<text class='accidental-sign' x='200' y='500' fill='#000' "
            << "font-family='Bravura' font-size='740.834'>&#57960;</text>" << endl;

        run_test_for(shape, expected);
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_200)
    {
        //@200 shape GmoShapeAccidental
        ImoArpeggioDto imo;
        imo.set_id(83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeAccidental shape(&imo, 0, k_glyph_accidentalNaturalSharp, pos,
                                 Color(0,0,0), m_libraryScope, 21.0);

        stringstream expected;
        expected << "<text class='accidental-sign' x='200' y='500' fill='#000' "
            << "font-family='Bravura' font-size='740.834'>&#57960;</text>" << endl;

        run_test_for(shape, expected);
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_300)
    {
        //@300 shape GmoShapeArpeggio
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_arpeggio, &doc, 83);
        GmoShapeArpeggio shape(pImo, 0, 200.0f, 500.0f, 1200.0f,
                               true, true, Color(0,0,0), m_libraryScope, 21.0);

        stringstream expected;
        expected
            << "<g id='m83' class='arpeggio'>" << endl
            << "   <text x='140' y='1123.46' fill='#000' transform='rotate(-90,140,1123.46)' "
            <<        "font-family='Bravura' font-size='740.834'>&#60073;</text>" << endl
            << "   <text x='140' y='934.535' fill='#000' transform='rotate(-90,140,934.535)' "
            <<        "font-family='Bravura' font-size='740.834'>&#60077;</text>" << endl
            << "</g>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_400)
    {
        //@400 shape GmoShapeArticulation
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_articulation_symbol, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeArticulation shape(pImo, 0, k_glyph_staccato_above, pos,
                                   Color(0,0,0), m_libraryScope, 21.0);

        stringstream expected;
        expected << "<text class='articulation' x='200' y='500' fill='#000' "
            << "font-family='Bravura' font-size='740.834'>&#58530;</text>" << endl;

        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_500)
    {
        //@500 shape GmoShapeBarline
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_barline, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeBarline shape(pImo, 0, k_barline_double_repetition, 200.0, 500.0, 600.0, 27.0, 100.0, 72.0,
                              36.0, Color(0,0,0), 0.0);

        stringstream expected;
        expected
            << "<g id='m83' class='barline'>" << endl
            << "   <circle cx='236' cy='766.4' r='36' fill='#000' stroke='#000'/>" << endl
            << "   <circle cx='236' cy='939.2' r='36' fill='#000' stroke='#000'/>" << endl
            << "   <path d=' M 371 500 L 344 500 L 344 600 L 371 600' fill='#000' stroke='#000'/>" << endl
            << "   <path d=' M 470 500 L 443 500 L 443 600 L 470 600' fill='#000' stroke='#000'/>" << endl
            << "   <circle cx='578' cy='766.4' r='36' fill='#000' stroke='#000'/>" << endl
            << "   <circle cx='578' cy='939.2' r='36' fill='#000' stroke='#000'/>" << endl
            << "</g>" << endl;

        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_600)
    {
        //@600 shape GmoShapeBeam
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_beam, &doc, 83);
        GmoShapeBeam shape(pImo, 50.0);
        std::list<LUnits> segments = {100.0, 150.0, 200.0, 180.0};
        UPoint pos(200.0f, 500.0f);
        shape.set_layout_data(segments, pos, USize(400.0, 80.0), false, false,
                              EComputedBeam::k_beam_above, 0);

        stringstream expected;
        expected
            << "<g id='m83' class='beam'>" << endl
            << "   <path d=' M 300 623.899 L 300 676.101 L 400 706.101 L 400 653.899' fill='#000'/>" << endl
            << "</g>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_700)
    {
        //@700 shape GmoShapeBrace. For instrument
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_instrument, &doc, 83);
        GmoShapeBrace shape(pImo, 0, 100.0, 200.0, 200.0, 800.0, Color(0,0,0));

        stringstream expected;
        expected << "<path id='m83-brace' class='instrument-brace' d=' M 100 501.101 L 100 498.899 "
            << "S 127.687 489.976 142.834 478.483 S 159.772 465.612 159.772 450.098 "
            << "S 159.772 438.386 146.743 413.728 S 123.616 369.829 122.476 367.422 "
            << "S 109.283 338.508 109.283 317.446 S 109.283 300.245 117.752 282.25 "
            << "S 127.199 262.348 146.417 242.505 S 167.427 220.856 200 200 "
            << "L 200 202.202 S 180.456 218.508 169.381 230.294 "
            << "S 155.7 244.853 149.186 257.945 S 142.02 272.446 142.02 288.239 "
            << "S 142.02 307.055 157.003 334.266 S 171.01 356.957 185.016 379.706 "
            << "S 200 404.878 200 418.777 S 200 439.78 188.599 453.826 "
            << "S 180.456 467.872 158.143 479.217 S 138.111 489.388 103.583 499.853 "
            << "M 100 501.101 S 127.687 510.024 142.834 521.517 "
            << "S 159.772 534.388 159.772 549.755 S 159.772 561.615 146.743 586.272 "
            << "S 123.616 630.171 122.476 632.578 S 109.283 661.492 109.283 682.554 "
            << "S 109.283 699.755 117.752 717.75 S 127.199 737.652 146.417 757.495 "
            << "S 167.427 779.144 200 800 L 200 797.798 "
            << "S 180.456 781.492 169.381 769.706 S 155.7 755.147 149.186 742.055 "
            << "S 142.02 727.554 142.02 711.761 S 142.02 692.945 157.003 665.734 "
            << "S 171.01 643.043 185.016 620.294 S 200 595.122 200 581.223 "
            << "S 200 560.22 188.599 546.174 S 180.456 532.128 158.143 520.783 "
            << "S 138.111 510.612 103.583 499.853 Z' fill='#000'/>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_701)
    {
        //@701 shape GmoShapeBrace. For group
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_instrument_groups, &doc, 83);
        GmoShapeBrace shape(pImo, 0, 100.0, 200.0, 200.0, 800.0, Color(0,0,0));

        stringstream expected;
        expected << "<path id='m83-brace' class='instr-groups-brace' d=' M 100 501.101 L 100 498.899 "
            << "S 127.687 489.976 142.834 478.483 S 159.772 465.612 159.772 450.098 "
            << "S 159.772 438.386 146.743 413.728 S 123.616 369.829 122.476 367.422 "
            << "S 109.283 338.508 109.283 317.446 S 109.283 300.245 117.752 282.25 "
            << "S 127.199 262.348 146.417 242.505 S 167.427 220.856 200 200 "
            << "L 200 202.202 S 180.456 218.508 169.381 230.294 "
            << "S 155.7 244.853 149.186 257.945 S 142.02 272.446 142.02 288.239 "
            << "S 142.02 307.055 157.003 334.266 S 171.01 356.957 185.016 379.706 "
            << "S 200 404.878 200 418.777 S 200 439.78 188.599 453.826 "
            << "S 180.456 467.872 158.143 479.217 S 138.111 489.388 103.583 499.853 "
            << "M 100 501.101 S 127.687 510.024 142.834 521.517 "
            << "S 159.772 534.388 159.772 549.755 S 159.772 561.615 146.743 586.272 "
            << "S 123.616 630.171 122.476 632.578 S 109.283 661.492 109.283 682.554 "
            << "S 109.283 699.755 117.752 717.75 S 127.199 737.652 146.417 757.495 "
            << "S 167.427 779.144 200 800 L 200 797.798 "
            << "S 180.456 781.492 169.381 769.706 S 155.7 755.147 149.186 742.055 "
            << "S 142.02 727.554 142.02 711.761 S 142.02 692.945 157.003 665.734 "
            << "S 171.01 643.043 185.016 620.294 S 200 595.122 200 581.223 "
            << "S 200 560.22 188.599 546.174 S 180.456 532.128 158.143 520.783 "
            << "S 138.111 510.612 103.583 499.853 Z' fill='#000'/>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_800)
    {
        //@800 shape GmoShapeBracket. For instrument
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_instrument, &doc, 83);
        GmoShapeBracket shape(pImo, 0, 100.0, 200.0, 200.0, 800.0, 50.0, Color(0,0,0));

        stringstream expected;
        expected << "<path id='m83-bracket' class='instrument-bracket' d=' M 100 176.98 "
            << "S 188.925 175.33 282.736 166.172 S 308.795 162.459 326.71 158.416 "
            << "S 340.717 155.281 357.655 150 L 372.313 151.32 "
            << "S 361.889 156.848 347.557 161.469 S 335.831 165.264 313.355 170.957 "
            << "S 295.765 175.248 279.479 178.218 S 266.124 180.693 244.3 183.663 "
            << "S 231.596 185.479 200 189.521 L 200 806.518 "
            << "S 231.596 810.561 244.3 812.376 S 266.124 815.347 279.479 817.822 "
            << "S 295.765 820.792 313.355 825.083 S 335.831 830.776 347.557 834.571 "
            << "S 361.889 839.191 372.313 844.719 L 357.655 846.04 "
            << "S 340.717 840.759 326.71 837.624 S 308.795 840.924 282.736 829.868 "
            << "S 188.925 820.71 100 823.02 Z' fill='#000'/>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_801)
    {
        //@801 shape GmoShapeBracket. For group
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_instrument_groups, &doc, 83);
        GmoShapeBracket shape(pImo, 0, 100.0, 200.0, 200.0, 800.0, 50.0, Color(0,0,0));

        stringstream expected;
        expected << "<path id='m83-bracket' class='instr-groups-bracket' d=' M 100 176.98 "
            << "S 188.925 175.33 282.736 166.172 S 308.795 162.459 326.71 158.416 "
            << "S 340.717 155.281 357.655 150 L 372.313 151.32 "
            << "S 361.889 156.848 347.557 161.469 S 335.831 165.264 313.355 170.957 "
            << "S 295.765 175.248 279.479 178.218 S 266.124 180.693 244.3 183.663 "
            << "S 231.596 185.479 200 189.521 L 200 806.518 "
            << "S 231.596 810.561 244.3 812.376 S 266.124 815.347 279.479 817.822 "
            << "S 295.765 820.792 313.355 825.083 S 335.831 830.776 347.557 834.571 "
            << "S 361.889 839.191 372.313 844.719 L 357.655 846.04 "
            << "S 340.717 840.759 326.71 837.624 S 308.795 840.924 282.736 829.868 "
            << "S 188.925 820.71 100 823.02 Z' fill='#000'/>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_900)
    {
        //@900 shape GmoShapeClef
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_clef, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeClef shape(pImo, 0, k_glyph_f_clef, pos, Color(0,0,0), m_libraryScope, 21.0);

        stringstream expected;
        expected << "<text id='m83' class='clef' x='200' y='500' fill='#000' "
            << "font-family='Bravura' font-size='740.834'>&#57442;</text>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_1000)
    {
        //@1000 shape GmoShapeCodaSegno
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_symbol_repetition_mark, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeCodaSegno shape(pImo, 0, k_glyph_coda, pos, Color(0,0,0), m_libraryScope, 21.0);

        stringstream expected;
        expected << "<text id='m83' class='coda-segno' x='200' y='500' fill='#000' "
            << "font-family='Bravura' font-size='740.834'>&#57416;</text>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_1100)
    {
        //@1100 shape GmoShapeDebug
        GmoShapeDebug shape(Color(0,0,0));
        shape.add_vertex('M', 100.0, 200.0);
        shape.add_vertex('L', 250.0, 450.0);
        shape.add_vertex('Z', 0.0f, 0.0f);
        shape.close_vertex_list();

        stringstream expected;
        expected << "<path class='shape-debug' d=' M 100 200 L 250 450 Z' fill='#000'/>" << endl;
        run_test_for(shape, expected);
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_1200)
    {
        //@1200 shape GmoShapeDot
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_note_regular, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeDot shape(pImo, 0, pos, Color(0,0,0), m_libraryScope, 21.0);

        stringstream expected;
        expected << "<text class='dot' x='200' y='500' fill='#000' "
            << "font-family='Bravura' font-size='740.834'>&#57831;</text>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_1300)
    {
        //@1300 shape GmoShapeDynamicsMark
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_fermata, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeDynamicsMark shape(pImo, 0, k_glyph_fermata_above, pos, Color(0,0,0), m_libraryScope, 21.0);

        stringstream expected;
        expected << "<text id='m83' class='dynamics-mark' x='200' y='500' fill='#000' "
            << "font-family='Bravura' font-size='740.834'>&#58560;</text>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_1400)
    {
        //@1400 shape GmoShapeFermata
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_dynamics_mark, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeFermata shape(pImo, 0, k_glyph_dynamic_mf, pos, Color(0,0,0), m_libraryScope, 21.0);

        stringstream expected;
        expected << "<text id='m83' class='fermata' x='200' y='500' fill='#000' "
            << "font-family='Bravura' font-size='740.834'>&#58669;</text>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_1500)
    {
        //@1500 shape GmoShapeFingeringContainer. Empty container
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_fingering, &doc, 83);
        GmoShapeFingeringContainer shape(pImo, 0);

        stringstream expected;
        expected << "<g id='m83' class='fingering'>" << endl
                 << "</g>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_1501)
    {
        //@1501 shape GmoShapeFingeringContainer. Has fingering
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_fingering, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeFingeringContainer shape(pImo, 0);
        GmoShapeFingering* shape2 =
            LOMSE_NEW GmoShapeFingering(pImo, 0, k_glyph_fingering_3, pos,
                                        m_libraryScope, Color(0,0,0), 21.0);
        shape.add(shape2);

        stringstream expected;
        expected
            << "<g id='m83' class='fingering'>" << endl
            << "   <text x='200' y='500' fill='#000' "
            <<      "font-family='Bravura' font-size='740.834'>&#60691;</text>" << endl
            << "</g>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_1600)
    {
        //@1600 shape GmoShapeFingering
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_fingering, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeFingering shape(pImo, 0, k_glyph_fingering_3, pos, m_libraryScope, Color(0,0,0), 21.0);

        stringstream expected;
        expected << "<text x='200' y='500' fill='#000' "
            << "font-family='Bravura' font-size='740.834'>&#60691;</text>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_1700)
    {
        //@1700 shape GmoShapeFlag
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_note_regular, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeFlag shape(pImo, 0, k_glyph_16th_flag_up, pos,
                           Color(0,0,0), m_libraryScope, 21.0);

        stringstream expected;
        expected << "<text class='flag' x='200' y='500' fill='#000' "
            << "font-family='Bravura' font-size='740.834'>&#57922;</text>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_1800)
    {
        //@1800 shape GmoShapeGraceStroke
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_note_regular, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeGraceStroke shape(pImo, 200.0f, 500.0f, 250.0f, 550.0f, 5.0, Color(0,0,0));

        stringstream expected;
        expected << "<path class='grace-stroke' d=' M 198.232 501.768 L 201.768 498.232 "
            << "L 251.768 548.232 L 248.232 551.768 Z' fill='#000' "
            << "stroke='#000' stroke-width='5'/>" << endl;
        run_test_for(shape, expected);

        delete pImo;
    }

//TODO        //@1900 shape GmoShapeImage

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_2000)
    {
        //@2000 shape GmoShapeInvisible
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_note_regular, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeInvisible shape(pImo, 0, pos, USize(100.0f, 300.0f));

        stringstream expected;
        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_2100)
    {
        //@2100 shape GmoShapeKeySignature. Empty container
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_key_signature, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeKeySignature shape(pImo, 0, pos, Color(0,0,0), m_libraryScope);

        stringstream expected;
        expected
            << "<g id='m83' class='key-signature'>" << endl
            << "</g>" << endl;

        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_2101)
    {
        //@2101 shape GmoShapeKeySignature. Has accidentals
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_key_signature, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeKeySignature shape(pImo, 0, pos, Color(0,0,0), m_libraryScope);
        ImoArpeggioDto imo2;
        imo2.set_id(84);
        UPoint pos2(300.0f, 600.0f);
        GmoShapeAccidental* shape2 =
            LOMSE_NEW GmoShapeAccidental(&imo2, 0, k_glyph_accidentalNaturalSharp, pos,
                                         Color(0,0,0), m_libraryScope, 21.0);
        shape.add(shape2);

        stringstream expected;
        expected
            << "<g id='m83' class='key-signature'>" << endl
            << "   <text class='accidental-sign' x='200' y='500' fill='#000' "
            <<       "font-family='Bravura' font-size='740.834'>&#57960;</text>" << endl
            << "</g>" << endl;

        run_test_for(shape, expected);

        delete pImo;
    }

//TODO        //@2200 shape GmoShapeLine
//TODO        //@2300 shape GmoShapeLyrics
//TODO        //@2400 shape GmoShapeMetronomeGlyph
//TODO        //@2500 shape GmoShapeMetronomeMark
//TODO        //@2600 shape GmoShapeNote
//TODO        //@2700 shape GmoShapeChordBaseNote

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_2800)
    {
        //@2800 shape GmoShapeNotehead
        ImoArpeggioDto imo;
        imo.set_id(83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeNotehead shape(&imo, 0, k_glyph_notehead_quarter, pos, Color(0,0,0),
                               m_libraryScope, 21.0);

        stringstream expected;
        expected << "<text class='notehead' x='200' y='500' fill='#000' font-family='Bravura' font-size='740.834'>&#57508;</text>" << endl;
        run_test_for(shape, expected);
    }
//TODO        //@2900 shape GmoShapeOctaveShift
//TODO        //@3000 shape GmoShapeOctaveGlyph
//TODO        //@3100 shape GmoShapeOrnament
//TODO        //@3200 shape GmoShapePedalGlyph
//TODO        //@3300 shape GmoShapePedalLine
//TODO        //@3400 shape GmoShapeRectangle
//TODO        //@3500 shape GmoShapeRest
//TODO        //@3600 shape GmoShapeRestGlyph
//TODO        //@3700 shape GmoShapeSlur
//TODO        //@3800 shape GmoShapeSquaredBracket

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_3900)
    {
        //@3900 shape GmoShapeStaff
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_instrument, &doc, 83);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff shape(pImo, 0, pInfo, 0, 20.0f, Color(0,0,0));

        stringstream expected;
        expected
            << "<path id='m83-staff' class='staff-lines' d=' M 0 0 L 20 0 M 0 180 "
            << "L 20 180 M 0 360 L 20 360 M 0 540 L 20 540 M 0 720 L 20 720' "
            << "stroke='#000' stroke-width='15'/>" << endl;
        run_test_for(shape, expected);

        delete pImo;
        delete pInfo;
    }
//TODO        //@4000 shape GmoShapeStem
//TODO        //@4100 shape GmoShapeTechnical
//TODO        //@4200 shape GmoShapeText
//TODO        //@4300 shape GmoShapeTextBox
//TODO        //@4400 shape GmoShapeSlurTie
//TODO        //@4500 shape GmoShapeTimeGlyph

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_4600)
    {
        //@4600 shape GmoShapeTimeSignature. Empty container
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_time_signature, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeTimeSignature shape(pImo, 0, pos, Color(0,0,0), m_libraryScope);

        stringstream expected;
        expected
            << "<g id='m83' class='time-signature'>" << endl
            << "</g>" << endl;

        run_test_for(shape, expected);

        delete pImo;
    }

    TEST_FIXTURE(SvgDrawerTestFixture, shapes_4601)
    {
        //@4601 shape GmoShapeTimeSignature. With content
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_time_signature, &doc, 83);
        UPoint pos(200.0f, 500.0f);
        GmoShapeTimeSignature shape(pImo, 0, pos, Color(0,0,0), m_libraryScope);
        ImoArpeggioDto imo2;
        imo2.set_id(84);
        UPoint pos2(300.0f, 600.0f);
        GmoShapeTimeGlyph* shape2 =
            LOMSE_NEW GmoShapeTimeGlyph(&imo2, 0, k_glyph_common_time, pos,
                                         Color(0,0,0), m_libraryScope, 21.0);
        shape.add(shape2);

        stringstream expected;
        expected
            << "<g id='m83' class='time-signature'>" << endl
            << "   <text class='time-glyph' x='200' y='500' fill='#000' font-family='Bravura' "
            <<       "font-size='740.834'>&#57482;</text>" << endl
            << "</g>" << endl;

        run_test_for(shape, expected);

        delete pImo;
    }
//TODO        //@4700 shape GmoShapeTuplet
//TODO        //@4800 shape GmoShapeVoltaBracket
//TODO        //@4900 shape GmoShapeWedge
//TODO        //@5000 shape GmoShapeWord


    //@ develop -------------------------------------------------------------------------

    TEST_FIXTURE(SvgDrawerTestFixture, develop_01)
    {
        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgba32, 96);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
//    k_view_vertical_book,
//    k_view_horizontal_book,
//    k_view_single_system,
//    k_view_single_page,
//    k_view_free_flow,
//    k_view_half_page,
        Presenter* pPresenter = doorway.open_document(k_view_free_flow,
            //m_scores_path + "unit-tests/svg/01-arpeggios.xml");
            m_scores_path + "00023-spacing-in-prolog-two-instr.lms");
            //m_scores_path + "MozartTrio.mxl");
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);

//        pIntor->set_view_background(Color(100,100,100));
//        ADocument doc = pPresenter->get_document();
//        doc.set_page_size(USize(15000,30000));
        double width = 350;
        pIntor->set_svg_canvas_width(width);        //in CSS pixels
        pIntor->svg_indent(4);
        pIntor->svg_add_id(true);
        pIntor->svg_add_class(true);
        pIntor->svg_add_newlines(true);

        stringstream svg;
        int page = 0;
        pIntor->render_as_svg(svg, page);

//        cout << test_name() << endl;
//        USize size = pIntor->get_page_size(page);
//        cout << "width=" << size.width << " LU, height=" << size.height << " LU" << endl;
//        double width = size.width;
//        double height = size.height;
//        pIntor->model_point_to_device(&width, &height, 0);
//        cout << "width=" << width << "px, height=" << height << "px" << endl;

//        cout << svg.str() << endl;

        ofstream file1(m_scores_path + "../z_test_svg.html", ios::out);
        if (file1.good())
        {
            stringstream out;
            out << "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                << "<title>Lomse SVG</title></head><body><h1>Test of Lomse SVG</h1>"
                << "<div style='margin-left:40px; width:" << width << "px;border: 1px solid #000;'>";
            out << svg.str();
            out << "</div></body></html>";

            string str = out.str();
            file1.write(str.c_str(), str.size());

            file1.close();
        }
        else
        {
            std::cout << "file error write" << endl;
        }
    }

}


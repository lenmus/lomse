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
#include "lomse_box_content_layouter.h"
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_analyser.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"
#include "lomse_calligrapher.h"
#include "lomse_shape_text.h"

#include <cmath>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// EngroutersCreator tests
//=======================================================================================

//---------------------------------------------------------------------------------------
// helper, to access protected members
class MyEngroutersCreator : public EngroutersCreator
{
public:
    MyEngroutersCreator(std::list<Engrouter*>& engrouters, LibraryScope& libraryScope)
        : EngroutersCreator(engrouters, libraryScope)
    {
    }
    virtual ~MyEngroutersCreator() {}

    void my_create_text_item_engrouters(ImoTextItem* pText) { create_text_item_engrouters(pText); }
    void my_measure_engrouters() { measure_engrouters(); }

    void my_delete_engrouters() {
        std::list<Engrouter*>::iterator it;
        for (it = m_engrouters.begin(); it != m_engrouters.end(); ++it)
            delete *it;
    }

};

//---------------------------------------------------------------------------------------
class EngroutersCreatorTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    EngroutersCreatorTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~EngroutersCreatorTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

};


SUITE(EngroutersCreatorTest)
{

    TEST_FIXTURE(EngroutersCreatorTestFixture, CreateEngrouters_AtomicEngrouter)
    {
        Document doc(m_libraryScope);
        ImoButton* pImo = static_cast<ImoButton*>(
                                ImFactory::inject(k_imo_button, &doc) );
        pImo->set_label("Click me!");
        pImo->set_size(USize(2000.0f, 600.0f));
        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.create_engrouters(pImo);

        CHECK( engrouters.size() == 1 );
        ButtonEngrouter* pEngrouter = dynamic_cast<ButtonEngrouter*>( engrouters.front() );
        CHECK( pEngrouter != NULL );

        creator.my_delete_engrouters();
        delete pImo;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WrapperEngrouterCreatesBox)
    {
        Document doc(m_libraryScope);
        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.create_engrouters(pImo);

        CHECK( engrouters.size() == 1 );
        BoxEngrouter* pEngrouter = dynamic_cast<BoxEngrouter*>( engrouters.front() );
        CHECK( pEngrouter != NULL );

        creator.my_delete_engrouters();
        delete pImo;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WrapperEngrouterContentAddedToBox)
    {
        Document doc(m_libraryScope);
        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
        ImoButton* pBt1 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt1->set_label("Accept");
        pBt1->set_size(USize(2000.0f, 600.0f));
        pImo->append_child_imo(pBt1);
        ImoButton* pBt2 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt2->set_label("Cancel");
        pBt2->set_size(USize(2000.0f, 600.0f));
        pImo->append_child_imo(pBt2);

        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.create_engrouters(pImo);

        CHECK( engrouters.size() == 1 );
        BoxEngrouter* pEngrouter = dynamic_cast<BoxEngrouter*>( engrouters.front() );
        CHECK( pEngrouter != NULL );

        std::list<Engrouter*>& children = pEngrouter->get_engrouters();
        std::list<Engrouter*>::iterator it = children.begin();
        CHECK( children.size() == 2 );
        CHECK( dynamic_cast<ButtonEngrouter*>(*it) != NULL );
        ++it;
        CHECK( dynamic_cast<ButtonEngrouter*>(*it) != NULL );

        creator.my_delete_engrouters();
        delete pImo;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WrapperBoxEngrouterHasSize_Linear)
    {
        Document doc(m_libraryScope);
        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
        ImoButton* pBt1 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt1->set_label("Accept");
        pBt1->set_size(USize(2000.0f, 600.0f));
        pImo->append_child_imo(pBt1);
        ImoButton* pBt2 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt2->set_label("Cancel");
        pBt2->set_size(USize(2000.0f, 600.0f));
        pImo->append_child_imo(pBt2);

        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.create_engrouters(pImo);

        CHECK( engrouters.size() == 1 );
        BoxEngrouter* pEngrouter = dynamic_cast<BoxEngrouter*>( engrouters.front() );
        CHECK( pEngrouter != NULL );
        CHECK( pEngrouter->get_width() == 4000.0f );
        CHECK( pEngrouter->get_height() == 600.0f );
        std::list<Engrouter*>& children = pEngrouter->get_engrouters();
        std::list<Engrouter*>::iterator it = children.begin();
        CHECK( children.size() == 2 );
        CHECK( (*it)->get_position() == UPoint(0.0f, 0.0f) );
        ++it;
        CHECK( (*it)->get_position() == UPoint(2000.0f, 0.0f) );

        creator.my_delete_engrouters();
        delete pImo;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WrapperBoxEngrouterHasSize_Constrained)
    {
        Document doc(m_libraryScope);
        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
        pImo->set_width(3000.0f);
        ImoButton* pBt1 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt1->set_label("Accept");
        pBt1->set_size(USize(2000.0f, 600.0f));
        pImo->append_child_imo(pBt1);
        ImoButton* pBt2 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt2->set_label("Cancel");
        pBt2->set_size(USize(2000.0f, 600.0f));
        pImo->append_child_imo(pBt2);

        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.create_engrouters(pImo);

        CHECK( engrouters.size() == 1 );
        BoxEngrouter* pEngrouter = dynamic_cast<BoxEngrouter*>( engrouters.front() );
        CHECK( pEngrouter != NULL );
        CHECK( pEngrouter->get_width() == 3000.0f );
        CHECK( pEngrouter->get_height() == 1200.0f );
        std::list<Engrouter*>& children = pEngrouter->get_engrouters();
        std::list<Engrouter*>::iterator it = children.begin();
        CHECK( children.size() == 2 );
        CHECK( (*it)->get_position() == UPoint(0.0f, 0.0f) );
        ++it;
        CHECK( (*it)->get_position() == UPoint(0.0f, 600.0f) );

        creator.my_delete_engrouters();
        delete pImo;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, SplitTextItem_InitialSpaces)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text(" This is a paragraph");
        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.my_create_text_item_engrouters(pText);

        WordEngrouter *pEngrouter = dynamic_cast<WordEngrouter*>( engrouters.front() );
        CHECK( pEngrouter->get_text() == " " );

        creator.my_delete_engrouters();
        delete pText;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, SplitTextItem_FirstWordAfterInitialSpaces)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text(" This is a paragraph");
        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.my_create_text_item_engrouters(pText);

        CHECK( engrouters.size() == 5 );
        std::list<Engrouter*>::iterator it = engrouters.begin();
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>(*it);
        CHECK( pEngrouter->get_text() == " " );
        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
        CHECK( pEngrouter->get_text() == "This " );
        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
        CHECK( pEngrouter->get_text() == "is " );
        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
        CHECK( pEngrouter->get_text() == "a " );
        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
        CHECK( pEngrouter->get_text() == "paragraph" );

        creator.my_delete_engrouters();
        delete pText;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, SplitTextItem_SpacesCompressed)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text("  This   is   a   paragraph");
        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.my_create_text_item_engrouters(pText);

        CHECK( engrouters.size() == 5 );
        std::list<Engrouter*>::iterator it = engrouters.begin();
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>(*it);
        CHECK( pEngrouter->get_text() == " " );
        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
        CHECK( pEngrouter->get_text() == "This " );
        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
        CHECK( pEngrouter->get_text() == "is " );
        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
        CHECK( pEngrouter->get_text() == "a " );
        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
        CHECK( pEngrouter->get_text() == "paragraph" );

        creator.my_delete_engrouters();
        delete pText;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, SplitTextItem_OneWord)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text("Hello!");
        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.my_create_text_item_engrouters(pText);

        WordEngrouter *pEngrouter = dynamic_cast<WordEngrouter*>( engrouters.front() );
        CHECK( engrouters.size() == 1 );
        CHECK( pEngrouter->get_text() == "Hello!" );

        creator.my_delete_engrouters();
        delete pText;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, SplitTextItem_OneWordSpaces)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text("Hello!   ");
        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.my_create_text_item_engrouters(pText);

        WordEngrouter *pEngrouter = dynamic_cast<WordEngrouter*>( engrouters.front() );
        CHECK( engrouters.size() == 1 );
        CHECK( pEngrouter->get_text() == "Hello! " );

        creator.my_delete_engrouters();
        delete pText;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WordEngrouterHasStyle)
    {
        Document doc(m_libraryScope);
        ImoStyle* pStyle = static_cast<ImoStyle*>(
                                ImFactory::inject(k_imo_style, &doc) );
        pStyle->set_name("test");
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text("Hello!");
        pText->set_style(pStyle);
        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.my_create_text_item_engrouters(pText);

        WordEngrouter *pEngrouter = dynamic_cast<WordEngrouter*>( engrouters.front() );
        CHECK( pEngrouter->get_style() != NULL );

        creator.my_delete_engrouters();
        delete pText;
        delete pStyle;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WordEngrouterMeasurements)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pStyle = doc.get_default_style();
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text("Hello!");
        pText->set_style(pStyle);
        std::list<Engrouter*> engrouters;
        MyEngroutersCreator creator(engrouters, m_libraryScope);
        creator.my_create_text_item_engrouters(pText);
        creator.my_measure_engrouters();

//        WordEngrouter *pEngrouter = dynamic_cast<WordEngrouter*>( engrouters.front() );
//        cout << "ascent = " << pEngrouter->get_ascent() << endl;
//        cout << "descent = " << pEngrouter->get_descent() << endl;
//        cout << "height = " << pEngrouter->get_height() << endl;

        creator.my_delete_engrouters();
        delete pText;
    }

    // shapes creation ------------------------------------------------------------------

//    TEST_FIXTURE(EngroutersCreatorTestFixture, CreateGmo_AtomicEngrouter)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        ImoButton* pImo = static_cast<ImoButton*>(
//                                ImFactory::inject(k_imo_button, &doc) );
//        pImo->set_label("Click me!");
//        pImo->set_size(USize(2000.0f, 600.0f));
//        std::list<Engrouter*> engrouters;
//        MyEngroutersCreator creator(engrouters, m_libraryScope);
//        creator.create_engrouters(pImo);
//        Engrouter* pEngrouter = engrouters.front();
//        UPoint pos(1500.0f, 2700.0f);
//        ImoStyle* pStyle = doc.get_default_style();
//        pImo->set_style(pStyle);
//                        //  top     middle  base    bottom)
//        LineReferences refs(100.0f, 300.0f, 400.0f, 600.0f);
//        GmoObj* pGmo = pEngrouter->create_gm_object(pos, refs);
//
//        CHECK( pGmo != NULL );
//        CHECK( pGmo->is_shape_button() == true );
//        CHECK( pGmo->get_origin() == UPoint(1500.0f, 2700.0f + 100.0f) );
//
//        creator.my_delete_engrouters();
//        delete pGmo;
//        delete pImo;
//    }

//    TEST_FIXTURE(EngroutersCreatorTestFixture, CreateGmo_BoxNoContent)
//    {
//        Document doc(m_libraryScope);
//        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
//                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
//        USize size(500.0f, 600.0f);
//        pImo->set_size(size);
//        std::list<Engrouter*> engrouters;
//        MyEngroutersCreator creator(engrouters, m_libraryScope);
//        creator.create_engrouters(pImo);
//        Engrouter* pEngrouter = engrouters.front();
//        UPoint pos(1500.0f, 2700.0f);
//                        //  top     middle  base    bottom)
//        LineReferences refs(100.0f, 300.0f, 400.0f, 600.0f);
//        GmoObj* pGmo = pEngrouter->create_gm_object(pos, refs);    //(800-0)/2 = 400 valign
//
//        //cout << "box size = (" << pGmo->get_size().width << ", "
//        //     << pGmo->get_size().height << ")" << endl;
//        //cout << "box pos = (" << pGmo->get_origin().x << ", "
//        //     << pGmo->get_origin().y << ")" << endl;
//        CHECK( pGmo != NULL );
//        CHECK( pGmo->is_box_inline() == true );
//        CHECK( pGmo->get_origin() == UPoint(1500.0f, 2700.0f + 400.0f) );
//        CHECK( pGmo->get_size() == USize(500.0f, 0.0f) );
//
//        creator.my_delete_engrouters();
//        delete pGmo;
//        delete pImo;
//    }

//    TEST_FIXTURE(EngroutersCreatorTestFixture, CreateGmo_BoxContent)
//    {
//        Document doc(m_libraryScope);
//        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
//                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
//        ImoButton* pBt1 = static_cast<ImoButton*>(
//                                    ImFactory::inject(k_imo_button, &doc));
//        pBt1->set_label("Accept");
//        pBt1->set_size(USize(2000.0f, 600.0f));
//        pImo->append_child_imo(pBt1);
//        ImoButton* pBt2 = static_cast<ImoButton*>(
//                                    ImFactory::inject(k_imo_button, &doc));
//        pBt2->set_label("Cancel");
//        pBt2->set_size(USize(2000.0f, 600.0f));
//        pImo->append_child_imo(pBt2);
//
//        doc.create_empty();
//        ImoStyle* pStyle = doc.get_default_style();
//        pBt1->set_style(pStyle);
//        pBt2->set_style(pStyle);
//        std::list<Engrouter*> engrouters;
//        MyEngroutersCreator creator(engrouters, m_libraryScope);
//        creator.create_engrouters(pImo);
//        Engrouter* pEngrouter = engrouters.front();
//        UPoint pos(1500.0f, 2700.0f);
//                        //  top     middle  base    bottom)
//        LineReferences refs(100.0f, 300.0f, 400.0f, 600.0f);
//        GmoObj* pGmo = pEngrouter->create_gm_object(pos, refs);    //(800-600)/2 = 100 valign
//
//        //cout << "box size = (" << pGmo->get_size().width << ", "
//        //     << pGmo->get_size().height << ")" << endl;
//        //cout << "box pos = (" << pGmo->get_origin().x << ", "
//        //     << pGmo->get_origin().y << ")" << endl;
//        CHECK( pGmo != NULL );
//        CHECK( pGmo->is_box_inline() == true );
//        CHECK( pGmo->get_origin() == UPoint(1500.0f, 2700.0f + 100.0f) );
//        CHECK( pGmo->get_size() == USize(4000.0f, 600.0f) );
//
//        creator.my_delete_engrouters();
//        delete pGmo;
//        delete pImo;
//    }


};



//=======================================================================================
// BoxContentLayouter tests
//=======================================================================================

//---------------------------------------------------------------------------------------
// helper, access to protected members
class MyBoxContentLayouter : public BoxContentLayouter
{
protected:
    LUnits m_firstLineIndent;
    string m_prefix;

public:
    MyBoxContentLayouter(ImoContentObj* pImo, GraphicModel* pGModel,
                        LibraryScope& libraryScope, ImoStyles* pStyles)
        : BoxContentLayouter(pImo, NULL, pGModel, libraryScope, pStyles)
        , m_firstLineIndent(0.0f)
        , m_prefix("")
    {
    }
    MyBoxContentLayouter(LibraryScope& libraryScope, LineReferences& refs)
        : BoxContentLayouter(NULL, NULL, NULL, libraryScope, NULL)
    {
        m_lineRefs = refs;
    }
    virtual ~MyBoxContentLayouter() {}

    //mandatory overrides
    LUnits get_first_line_indent() { return m_firstLineIndent; }
    string get_first_line_prefix() { return m_prefix; }

    GmoBox* my_get_main_box() { return m_pItemMainBox; }
    UPoint my_get_cursor() { return m_pageCursor; }

    bool my_is_first_line() { return is_first_line(); }
    void my_set_first_line_indent(LUnits value) { m_firstLineIndent = value; }
    void my_set_first_line_prefix(const string& prefix) { m_prefix = prefix; }
    void my_page_initializations(GmoBox* pMainBox) { page_initializations(pMainBox); }
    void my_create_engrouters() { create_engrouters(); }
    void my_add_line() { add_line(); }
    void my_add_end_margins() { add_end_margins(); }
    void my_set_box_height() { set_box_height(); }
    bool my_enough_space_in_box() { return enough_space_in_box(); }
    void my_update_line_references(LineReferences& engr, LUnits shift, bool fUpdateText) {
        update_line_references(engr, shift, fUpdateText);
    }

};


//---------------------------------------------------------------------------------------
class BoxContentLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    BoxContentLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~BoxContentLayouterTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

};


SUITE(BoxContentLayouterTest)
{

    // para -----------------------------------------------------------------------------

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_Measure)
    {
        //The objective of this test is not to test something but to measure a
        //paragraph without margins, so that in following tests we can check
        //paragraph dimensions when adding border, margin, padding, etc.

        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyBoxContentLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        CHECK( lyt.my_is_first_line() == true );
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
//        cout << "box: org=(" << pParaBox->get_origin().x << ", "
//             << pParaBox->get_origin().y << ") size=("
//             << pParaBox->get_size().width << ", "
//             << pParaBox->get_size().height << ")"
//             << endl;
//        cout << "cursor=(" << lyt.my_get_cursor().x << ", "
//             << lyt.my_get_cursor().y << ")" << endl;
//        cout << "word: org=(" << pWord->get_left() << ", "
//             << pWord->get_top() << ")" << endl;
        CHECK( is_equal(lyt.my_get_cursor().x, 0.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 635.0f) );
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 635.0f) );
        CHECK( is_equal(pWord->get_left(), 0.0f) );
        CHECK( is_equal(pWord->get_top(), 0.0f) );
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_TopMargin)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_top(1000.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyBoxContentLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
        //cout << "box: org=(" << pParaBox->get_origin().x << ", "
        //     << pParaBox->get_origin().y << ") size=("
        //     << pParaBox->get_size().width << ", "
        //     << pParaBox->get_size().height << ")"
        //     << endl;
        //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
        //     << lyt.my_get_cursor().y << ")" << endl;
        //cout << "word: org=(" << pWord->get_left() << ", "
        //     << pWord->get_top() << ")" << endl;
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 1635.0f) );
        CHECK( is_equal(pWord->get_left(), 0.0f) );
        CHECK( is_equal(pWord->get_top(), 1000.0f) );
        CHECK( is_equal(lyt.my_get_cursor().x, 0.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 1635.0f) );
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_LeftMargin)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_left(1000.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyBoxContentLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
        //cout << "box: org=(" << pParaBox->get_origin().x << ", "
        //     << pParaBox->get_origin().y << ") size=("
        //     << pParaBox->get_size().width << ", "
        //     << pParaBox->get_size().height << ")"
        //     << endl;
        //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
        //     << lyt.my_get_cursor().y << ")" << endl;
        //cout << "word: org=(" << pWord->get_left() << ", "
        //     << pWord->get_top() << ")" << endl;
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 635.0f) );
        CHECK( is_equal(pWord->get_left(), 1000.0f) );
        CHECK( is_equal(pWord->get_top(), 0.0f) );
        CHECK( is_equal(lyt.my_get_cursor().x, 1000.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 635.0f) );
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_LeftBottomMargin)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_left(1000.0f);
        pParaStyle->margin_bottom(1000.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyBoxContentLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
        //cout << "box: org=(" << pParaBox->get_origin().x << ", "
        //     << pParaBox->get_origin().y << ") size=("
        //     << pParaBox->get_size().width << ", "
        //     << pParaBox->get_size().height << ")"
        //     << endl;
        //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
        //     << lyt.my_get_cursor().y << ")" << endl;
        //cout << "word: org=(" << pWord->get_left() << ", "
        //     << pWord->get_top() << ")" << endl;
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 1635.0f) );
        CHECK( is_equal(pWord->get_left(), 1000.0f) );
        CHECK( is_equal(pWord->get_top(), 0.0f) );
        CHECK( is_equal(lyt.my_get_cursor().x, 1000.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 1635.0f) );
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_FirstLineIndent)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_left(1000.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyBoxContentLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.my_set_first_line_indent(2000.0f);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
        //cout << "box: org=(" << pParaBox->get_origin().x << ", "
        //     << pParaBox->get_origin().y << ") size=("
        //     << pParaBox->get_size().width << ", "
        //     << pParaBox->get_size().height << ")"
        //     << endl;
        //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
        //     << lyt.my_get_cursor().y << ")" << endl;
        //cout << "word: org=(" << pWord->get_left() << ", "
        //     << pWord->get_top() << ")" << endl;
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 635.0f) );
        CHECK( is_equal(pWord->get_left(), 3000.0f) );
        CHECK( is_equal(pWord->get_top(), 0.0f) );
        CHECK( is_equal(lyt.my_get_cursor().x, 3000.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 635.0f) );
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_NotEnoughSpaceInPage)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_top(1000.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyBoxContentLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 1300.0f);
        lyt.layout_in_box();

        CHECK( lyt.is_item_layouted() == false );
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_EmptyParagraphHeight)
    {
        //If the paragraph is empty, it is considered to contain a line with a
        //"strut" (an invisible glyph of zero width) with the current paragraph
        //font size. Therefore, paragraph height is given by:
        //      paragrph height = line-height * font-size
        //plus the paragraph padding & margin.

        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_top(1000.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyBoxContentLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
//        cout << "box: org=(" << pParaBox->get_origin().x << ", "
//             << pParaBox->get_origin().y << ") size=("
//             << pParaBox->get_size().width << ", "
//             << pParaBox->get_size().height << ")"
//             << endl;
//        cout << "cursor=(" << lyt.my_get_cursor().x << ", "
//             << lyt.my_get_cursor().y << ")" << endl;
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 1635.0f) );
        CHECK( is_equal(lyt.my_get_cursor().x, 0.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 1635.0f) );
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_UpdateReferences_text_larger_up)
    {
        LineReferences line(100.0f, 300.0f, 400.0f, 600.0f); //top, middle, base, bottom
        LineReferences engr(120.0f, 350.0f, 500.0f, 650.0f); //top, middle, base, bottom

        MyBoxContentLayouter lyt(m_libraryScope, line);
        LUnits shift = 400.0f - 500.0f;
        lyt.my_update_line_references(engr, shift, true /*text*/);

        LineReferences& refs = lyt.get_line_refs();

//        cout << "top=" << refs.textTop << ", middle=" << refs.middleline
//             << ", base=" << refs.baseline << ", bottom=" << refs.textBottom
//             << ", height=" << refs.lineHeight << endl;

        CHECK( is_equal(refs.textTop, 160.0f) );        //mean(100+100, 120)
        CHECK( is_equal(refs.middleline, 375.0f) );     //mean(100+300, 350)
        CHECK( is_equal(refs.baseline, 500.0f) );       //max(100+400, 500)
        CHECK( is_equal(refs.textBottom, 675.0f) );     //mean(100+600, 650)
        CHECK( is_equal(refs.lineHeight, 800.0f) );     //max(100+700, 770)
        CHECK( is_equal(refs.supperLine, 120.0f) );     //engr.top
        CHECK( is_equal(refs.subLine, 500.0f) );        //engr.base
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_UpdateReferences_text_smaller_down)
    {
        LineReferences line(120.0f, 350.0f, 500.0f, 650.0f); //top, middle, base, bottom
        LineReferences engr(100.0f, 300.0f, 400.0f, 600.0f); //top, middle, base, bottom

        MyBoxContentLayouter lyt(m_libraryScope, line);
        LUnits shift = 500.0f - 400.0f;
        lyt.my_update_line_references(engr, shift, true /*text*/);

        LineReferences& refs = lyt.get_line_refs();

//        cout << "top=" << refs.textTop << ", middle=" << refs.middleline
//             << ", base=" << refs.baseline << ", bottom=" << refs.textBottom
//             << ", height=" << refs.lineHeight << endl;

        CHECK( is_equal(refs.textTop, 160.0f) );        //mean(120, 100+100)
        CHECK( is_equal(refs.middleline, 375.0f) );     //mean(350, 100+300)
        CHECK( is_equal(refs.baseline, 500.0f) );       //max(500, 100+400)
        CHECK( is_equal(refs.textBottom, 675.0f) );     //mean(650, 100+600)
        CHECK( is_equal(refs.lineHeight, 800.0f) );     //max(770, 100+700)
        CHECK( is_equal(refs.supperLine, 200.0f) );     //engr.top + shift
        CHECK( is_equal(refs.subLine, 500.0f) );        //engr.base + shift
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_UpdateReferences_notext_larger_down)
    {
        LineReferences line(100.0f, 300.0f, 400.0f, 600.0f); //top, middle, base, bottom
        LineReferences engr(120.0f, 350.0f, 500.0f, 650.0f); //top, middle, base, bottom

        MyBoxContentLayouter lyt(m_libraryScope, line);
        LUnits shift = 400.0f - 350.0f;     //i.e. sub align
        lyt.my_update_line_references(engr, shift, false /*text, no_baseline / no text*/);

        LineReferences& refs = lyt.get_line_refs();

//        cout << "top=" << refs.textTop << ", middle=" << refs.middleline
//             << ", base=" << refs.baseline << ", bottom=" << refs.textBottom
//             << ", height=" << refs.lineHeight << endl;

        CHECK( is_equal(refs.textTop, 100.0f) );        //no change
        CHECK( is_equal(refs.middleline, 350.0f) );     //mean(300, 350+50)
        CHECK( is_equal(refs.baseline, 400.0f) );       //no change
        CHECK( is_equal(refs.textBottom, 600.0f) );     //no change
        CHECK( is_equal(refs.lineHeight, 820.0f) );     //max(700, 770+50)
        CHECK( is_equal(refs.supperLine, 100.0f) );     //no change
        CHECK( is_equal(refs.subLine, 400.0f) );        //no change
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_UpdateReferences_notext_smaller_up)
    {
        LineReferences line(120.0f, 350.0f, 500.0f, 650.0f); //top, middle, base, bottom
        LineReferences engr(100.0f, 300.0f, 400.0f, 600.0f); //top, middle, base, bottom

        MyBoxContentLayouter lyt(m_libraryScope, line);
        LUnits shift = 120.0f - 300.0f;     //i.e. supper align
        lyt.my_update_line_references(engr, shift, false /*text, no_baseline / no text*/);

        LineReferences& refs = lyt.get_line_refs();

//        cout << "top=" << refs.textTop << ", middle=" << refs.middleline
//             << ", base=" << refs.baseline << ", bottom=" << refs.textBottom
//             << ", height=" << refs.lineHeight << endl;

        CHECK( is_equal(refs.textTop, 120.0f) );        //no change
        CHECK( is_equal(refs.middleline, 415.0f) );     //mean(350+180, 300)
        CHECK( is_equal(refs.baseline, 500.0f) );       //no change
        CHECK( is_equal(refs.textBottom, 650.0f) );     //no change
        CHECK( is_equal(refs.lineHeight, 950.0f) );     //max(770+180, 700)
        CHECK( is_equal(refs.supperLine, 120.0f) );     //no change
        CHECK( is_equal(refs.subLine, 500.0f) );        //no change
    }

    TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_AlignBaseline)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pParaStyle = doc.create_private_style();
        pParaStyle->margin(0.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        ImoStyle* pTextStyle1 = doc.create_private_style();
        pTextStyle1->margin(0.0f)->vertical_align(ImoStyle::k_valign_baseline)->font_size(12.0f);
        pPara->add_text_item("Exercise ", pTextStyle1);
        ImoStyle* pTextStyle2 = doc.create_private_style();
        pTextStyle2->margin(0.0f)->vertical_align(ImoStyle::k_valign_baseline)->font_size(14.0f);
        pPara->add_text_item("Options ", pTextStyle2);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyBoxContentLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
//        GmoShapeWord* pWord1 = static_cast<GmoShapeWord*>( pParaBox->get_shape(0) );
//        GmoShapeWord* pWord2 = static_cast<GmoShapeWord*>( pParaBox->get_shape(1) );
//        cout << "box: org=(" << pParaBox->get_origin().x << ", "
//             << pParaBox->get_origin().y << ") size=("
//             << pParaBox->get_size().width << ", "
//             << pParaBox->get_size().height << ")"
//             << endl;
//        cout << "word1.top=" << pWord1->get_top() << ", baseline="
//             << pWord1->get_baseline() << endl;
//        cout << "word2.top=(" << pWord2->get_top() << ", baseline="
//             << pWord2->get_baseline() << endl;
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 740.833f) );
//TODO: REVIEW THESE
//        CHECK( pWord1->get_top() > 0.0f );
//        CHECK( is_equal(pWord2->get_top(), 0.0f) );
//        CHECK( is_equal(pWord1->get_baseline(), pWord2->get_baseline()) );
    }

////    //TEST_FIXTURE(BoxContentLayouterTestFixture, Paragraph_TwoPagesCreated)
////    //{
////    //    Document doc(m_libraryScope);
////    //    doc.from_string("(lenmusdoc (version 0.0)(content "
////    //        "(para (text \"Line one one one one one one one one one one\")"
////    //        "      (text \"Line two two two two two two two two two two\")"
////    //        ") ))" );
////
////    //    DomunetLayouter lyt(m_libraryScope);
////    //    lyt.prepare_to_start_layout();
////    //    lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 1500.0f);
////    //    lyt.layout_in_box();
////
////    //    CHECK( lyt.is_item_layouted() == false );
////    //}

};

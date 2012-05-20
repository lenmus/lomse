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
#include "lomse_engrouters.h"
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

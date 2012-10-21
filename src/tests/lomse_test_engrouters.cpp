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
#include "lomse_ldp_analyser.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"
#include "lomse_calligrapher.h"
#include "lomse_shape_text.h"
#include "lomse_text_splitter.h"

#include "utf8.h"
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
    MyEngroutersCreator(ImoInlinesContainer* pPara, LibraryScope& libraryScope)
        : EngroutersCreator(libraryScope, pPara->begin(), pPara->end())
    {
    }
    virtual ~MyEngroutersCreator() {}

    inline TextSplitter* my_create_text_splitter_for(ImoTextItem* pText) {
        return create_text_splitter_for(pText);
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
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~EngroutersCreatorTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

    string to_str(const wstring& wtext)
    {
        string utf8result;
        utf8::utf32to8(wtext.begin(), wtext.end(), back_inserter(utf8result));
        return utf8result;
    }

};


SUITE(EngroutersCreatorTest)
{

    TEST_FIXTURE(EngroutersCreatorTestFixture, default_text_splitter)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoTextItem* pText = pPara->add_text_item("Hello world!");
        MyEngroutersCreator creator(pPara, m_libraryScope);

        TextSplitter* pSplitter = creator.my_create_text_splitter_for(pText);

        CHECK( dynamic_cast<DefaultTextSplitter*>( pSplitter ) != NULL );
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, chinesse_text_splitter)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoDocument* pDoc = doc.get_imodoc();
        pDoc->set_language("zh_CN");
        ImoParagraph* pPara = doc.add_paragraph();
        ImoTextItem* pText = pPara->add_text_item("编辑名称，缩写，MIDI设置和其他特性");
        MyEngroutersCreator creator(pPara, m_libraryScope);

        TextSplitter* pSplitter = creator.my_create_text_splitter_for(pText);

        CHECK( dynamic_cast<ChineseTextSplitter*>( pSplitter ) != NULL );
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, no_content)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        MyEngroutersCreator creator(pPara, m_libraryScope);

        CHECK( creator.more_content() == false );

        LUnits availableWidth = 3000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth);

        CHECK( pEngr == NULL );
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, one_atomic_item)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_button("Click me!", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        CHECK( creator.more_content() == true );

        LUnits availableWidth = 3000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth);

        CHECK( pEngr != NULL );
        ButtonEngrouter* pEngrouter = dynamic_cast<ButtonEngrouter*>( pEngr );
        CHECK( pEngrouter != NULL );

//        CHECK( creator.more_content() == false );

        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, one_atomic_item_no_space)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_button("Click me!", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 1500.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth);

        CHECK( pEngr == NULL );

        CHECK( creator.more_content() == true );
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, return_saved_engrouter)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_button("Click me!", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);
        LUnits availableWidth = 1500.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth);
        CHECK( pEngr == NULL );

        CHECK( creator.more_content() == true );

        availableWidth = 3000.0f;
        pEngr = creator.create_next_engrouter(availableWidth);

        CHECK( pEngr != NULL );
        ButtonEngrouter* pEngrouter = dynamic_cast<ButtonEngrouter*>( pEngr );
        CHECK( pEngrouter != NULL );

        CHECK( creator.more_content() == false );

        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, two_atomic_items)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_button("Accept", USize(2000.0f, 600.0f));
        pPara->add_button("Cancel", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 2000.0f;
        Engrouter* pEngr1 = creator.create_next_engrouter(availableWidth);
        CHECK( pEngr1 != NULL );
        ButtonEngrouter* pBtEngr1 = dynamic_cast<ButtonEngrouter*>( pEngr1 );
        CHECK( pBtEngr1 != NULL );

        availableWidth = 2000.0f;
        Engrouter* pEngr2 = creator.create_next_engrouter(availableWidth);
        CHECK( pEngr2 != NULL );
        ButtonEngrouter* pBtEngr2 = dynamic_cast<ButtonEngrouter*>( pEngr2 );
        CHECK( pBtEngr2 != NULL );

        CHECK( pEngr1->get_creator_imo() != pEngr2->get_creator_imo() );

        CHECK( creator.more_content() == false );

        delete pEngr1;
        delete pEngr2;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, text_engrouter)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_text_item("Hello world!");
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 1000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth);

        CHECK( pEngr != NULL );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != NULL );
        CHECK( pEngrouter->get_text() == L"Hello" );
//        cout << "chunk = '" << to_str( pEngrouter->get_text() ) << "'" << endl;
//        cout << "size = " << pEngrouter->get_width() << endl;
        CHECK( pEngr->break_requested() == true );
        CHECK( creator.more_content() == true );

        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, text_engrouter_no_more_text)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_text_item("Hello");
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 2000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth);

        CHECK( pEngr != NULL );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != NULL );
        CHECK( pEngrouter->get_text() == L"Hello" );
        CHECK( pEngr->break_requested() == false );

        CHECK( creator.more_content() == false );

        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, text_engrouter_two_chunks)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_text_item("This is a paragraph");
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 1000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth);

        CHECK( pEngr != NULL );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != NULL );
        CHECK( pEngrouter->get_text() == L"This" );
        CHECK( creator.more_content() == true );
        CHECK( pEngr->break_requested() == true );
        delete pEngr;

        pEngr = creator.create_next_engrouter(5000.0f);

        CHECK( pEngr != NULL );
        pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != NULL );
        CHECK( pEngrouter->get_text() == L"is a paragraph" );
        CHECK( creator.more_content() == false );
        CHECK( pEngr->break_requested() == false );
        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, next_item_after_text)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_text_item("This is a paragraph");
        pPara->add_button("Click me!", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        Engrouter* pEngr = creator.create_next_engrouter(1000.0f);
        CHECK( pEngr != NULL );
        delete pEngr;

        pEngr = creator.create_next_engrouter(5000.0f);
        CHECK( pEngr != NULL );
        CHECK( creator.more_content() == true );
        CHECK( pEngr->break_requested() == false );
        delete pEngr;

        pEngr = creator.create_next_engrouter(3000.0f);
        CHECK( pEngr != NULL );
        ButtonEngrouter* pEngrouter = dynamic_cast<ButtonEngrouter*>( pEngr );
        CHECK( pEngrouter != NULL );
        CHECK( creator.more_content() == false );
        CHECK( pEngr->break_requested() == false );
        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, skip_empty_text)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_text_item("");
        pPara->add_button("Click me!", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        Engrouter* pEngr = creator.create_next_engrouter(1000.0f);
        CHECK( pEngr != NULL );
        NullEngrouter* pEngr1 = dynamic_cast<NullEngrouter*>( pEngr );
        CHECK( pEngr1 != NULL );
        CHECK( creator.more_content() == true );
        CHECK( pEngr->break_requested() == false );
        delete pEngr;

        pEngr = creator.create_next_engrouter(3000.0f);
        CHECK( pEngr != NULL );
        ButtonEngrouter* pEngr2 = dynamic_cast<ButtonEngrouter*>( pEngr );
        CHECK( pEngr2 != NULL );
        CHECK( creator.more_content() == false );
        CHECK( pEngr->break_requested() == false );
        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WrapperEngrouterCreatesBox)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_inline_box();
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 2000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth);

        CHECK( pEngr != NULL );
        BoxEngrouter* pEngrouter = dynamic_cast<BoxEngrouter*>( pEngr );
        CHECK( pEngrouter != NULL );

        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WrapperEngrouterContentAddedToBox)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoInlineWrapper* pWp = pPara->add_inline_box();
        pWp->add_button("Accept", USize(2000.0f, 600.0f));
        pWp->add_button("Cancel", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 8000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth);

        CHECK( pEngr != NULL );
        BoxEngrouter* pEngrouter = dynamic_cast<BoxEngrouter*>( pEngr );
        CHECK( pEngrouter != NULL );

        std::list<Engrouter*>& children = pEngrouter->get_engrouters();
        std::list<Engrouter*>::iterator it = children.begin();
        CHECK( children.size() == 2 );
//        cout << children.size() << endl;
        CHECK( dynamic_cast<ButtonEngrouter*>(*it) != NULL );
        ++it;
        CHECK( dynamic_cast<ButtonEngrouter*>(*it) != NULL );

        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WrapperBoxEngrouterHasSize_Linear)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoInlineWrapper* pWp = pPara->add_inline_box();
        pWp->add_button("Accept", USize(2000.0f, 600.0f));
        pWp->add_button("Cancel", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 8000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth);

        CHECK( pEngr != NULL );
        BoxEngrouter* pEngrouter = dynamic_cast<BoxEngrouter*>( pEngr );
        CHECK( pEngrouter != NULL );

//        cout << "box width = " << pEngrouter->get_width() << endl;
//        cout << "box height = " << pEngrouter->get_height() << endl;
        CHECK( pEngrouter->get_width() == 4000.0f );
        CHECK( pEngrouter->get_height() == 600.0f );
        std::list<Engrouter*>& children = pEngrouter->get_engrouters();
        std::list<Engrouter*>::iterator it = children.begin();
        CHECK( children.size() == 2 );
        CHECK( (*it)->get_position() == UPoint(0.0f, 0.0f) );
//        cout << "first button pos = (" << (*it)->get_position().x << ", "
//              << (*it)->get_position().y << ")" << endl;
        ++it;
        CHECK( (*it)->get_position() == UPoint(2000.0f, 0.0f) );
//        cout << "2nd button pos = (" << (*it)->get_position().x << ", "
//              << (*it)->get_position().y << ")" << endl;

        delete pEngr;
    }

//    TEST_FIXTURE(EngroutersCreatorTestFixture, SplitTextItem_InitialSpaces)
//    {
//        Document doc(m_libraryScope);
//        ImoTextItem* pText = static_cast<ImoTextItem*>(
//                                ImFactory::inject(k_imo_text_item, &doc) );
//        pText->set_text(" This is a paragraph");
//        std::list<Engrouter*> engrouters;
//        MyEngroutersCreator creator(engrouters, m_libraryScope);
//        creator.my_create_text_item_engrouters(pText);
//
//        WordEngrouter *pEngrouter = dynamic_cast<WordEngrouter*>( engrouters.front() );
//        CHECK( pEngrouter->get_text() == " " );
//
//        creator.my_delete_engrouters();
//        delete pText;
//    }
//
//    TEST_FIXTURE(EngroutersCreatorTestFixture, SplitTextItem_FirstWordAfterInitialSpaces)
//    {
//        Document doc(m_libraryScope);
//        ImoTextItem* pText = static_cast<ImoTextItem*>(
//                                ImFactory::inject(k_imo_text_item, &doc) );
//        pText->set_text(" This is a paragraph");
//        std::list<Engrouter*> engrouters;
//        MyEngroutersCreator creator(engrouters, m_libraryScope);
//        creator.my_create_text_item_engrouters(pText);
//
//        CHECK( engrouters.size() == 5 );
//        std::list<Engrouter*>::iterator it = engrouters.begin();
//        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>(*it);
//        CHECK( pEngrouter->get_text() == " " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter->get_text() == "This " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter->get_text() == "is " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter->get_text() == "a " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter->get_text() == "paragraph" );
//
//        creator.my_delete_engrouters();
//        delete pText;
//    }
//
//    TEST_FIXTURE(EngroutersCreatorTestFixture, SplitTextItem_SpacesCompressed)
//    {
//        Document doc(m_libraryScope);
//        ImoTextItem* pText = static_cast<ImoTextItem*>(
//                                ImFactory::inject(k_imo_text_item, &doc) );
//        pText->set_text("  This   is   a   paragraph");
//        std::list<Engrouter*> engrouters;
//        MyEngroutersCreator creator(engrouters, m_libraryScope);
//        creator.my_create_text_item_engrouters(pText);
//
//        CHECK( engrouters.size() == 5 );
//        std::list<Engrouter*>::iterator it = engrouters.begin();
//        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>(*it);
//        CHECK( pEngrouter->get_text() == " " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter->get_text() == "This " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter->get_text() == "is " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter->get_text() == "a " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter->get_text() == "paragraph" );
//
//        creator.my_delete_engrouters();
//        delete pText;
//    }
//
//    TEST_FIXTURE(EngroutersCreatorTestFixture, SplitTextItem_OneWord)
//    {
//        Document doc(m_libraryScope);
//        ImoTextItem* pText = static_cast<ImoTextItem*>(
//                                ImFactory::inject(k_imo_text_item, &doc) );
//        pText->set_text("Hello!");
//        std::list<Engrouter*> engrouters;
//        MyEngroutersCreator creator(engrouters, m_libraryScope);
//        creator.my_create_text_item_engrouters(pText);
//
//        WordEngrouter *pEngrouter = dynamic_cast<WordEngrouter*>( engrouters.front() );
//        CHECK( engrouters.size() == 1 );
//        CHECK( pEngrouter->get_text() == "Hello!" );
//
//        creator.my_delete_engrouters();
//        delete pText;
//    }
//
//    TEST_FIXTURE(EngroutersCreatorTestFixture, SplitTextItem_OneWordSpaces)
//    {
//        Document doc(m_libraryScope);
//        ImoTextItem* pText = static_cast<ImoTextItem*>(
//                                ImFactory::inject(k_imo_text_item, &doc) );
//        pText->set_text("Hello!   ");
//        std::list<Engrouter*> engrouters;
//        MyEngroutersCreator creator(engrouters, m_libraryScope);
//        creator.my_create_text_item_engrouters(pText);
//
//        WordEngrouter *pEngrouter = dynamic_cast<WordEngrouter*>( engrouters.front() );
//        CHECK( engrouters.size() == 1 );
//        CHECK( pEngrouter->get_text() == "Hello! " );
//
//        creator.my_delete_engrouters();
//        delete pText;
//    }

};

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
#include "private/lomse_document_p.h"
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

        CHECK( dynamic_cast<DefaultTextSplitter*>( pSplitter ) != nullptr );

        delete pSplitter;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, chinesse_text_splitter)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoDocument* pDoc = doc.get_im_root();
        pDoc->set_language("zh_CN");
        ImoParagraph* pPara = doc.add_paragraph();
        ImoTextItem* pText = pPara->add_text_item("编辑名称，缩写，MIDI设置和其他特性");
        MyEngroutersCreator creator(pPara, m_libraryScope);

        TextSplitter* pSplitter = creator.my_create_text_splitter_for(pText);

        CHECK( dynamic_cast<ChineseTextSplitter*>( pSplitter ) != nullptr );

        delete pSplitter;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, no_content)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        MyEngroutersCreator creator(pPara, m_libraryScope);

        CHECK( creator.more_content() == false );

        LUnits availableWidth = 3000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr == nullptr );
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, one_atomic_item)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_button(m_libraryScope, "Click me!", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        CHECK( creator.more_content() == true );

        LUnits availableWidth = 3000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr != nullptr );
        ControlEngrouter* pEngrouter = dynamic_cast<ControlEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );

//        CHECK( creator.more_content() == false );

        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, one_atomic_item_no_space)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_button(m_libraryScope, "Click me!", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 1500.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr == nullptr );

        CHECK( creator.more_content() == true );
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, return_saved_engrouter)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_button(m_libraryScope, "Click me!", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);
        LUnits availableWidth = 1500.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);
        CHECK( pEngr == nullptr );

        CHECK( creator.more_content() == true );

        availableWidth = 3000.0f;
        pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr != nullptr );
        ControlEngrouter* pEngrouter = dynamic_cast<ControlEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );

        CHECK( creator.more_content() == false );

        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, two_atomic_items)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_button(m_libraryScope, "Accept", USize(2000.0f, 600.0f));
        pPara->add_button(m_libraryScope, "Cancel", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 2000.0f;
        Engrouter* pEngr1 = creator.create_next_engrouter(availableWidth, false);
        CHECK( pEngr1 != nullptr );
        ControlEngrouter* pBtEngr1 = dynamic_cast<ControlEngrouter*>( pEngr1 );
        CHECK( pBtEngr1 != nullptr );

        availableWidth = 2000.0f;
        Engrouter* pEngr2 = creator.create_next_engrouter(availableWidth, false);
        CHECK( pEngr2 != nullptr );
        ControlEngrouter* pBtEngr2 = dynamic_cast<ControlEngrouter*>( pEngr2 );
        CHECK( pBtEngr2 != nullptr );

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
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"Hello" );
//        cout << "chunk = '" << to_str( pEngrouter->get_text() ) << "'" << endl;
//        cout << "size = " << pEngrouter->get_width() << endl;
        CHECK( pEngr && pEngr->break_requested() == true );
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
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"Hello" );
        CHECK( pEngr && pEngr->break_requested() == false );

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
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"This" );
        CHECK( creator.more_content() == true );
        CHECK( pEngr && pEngr->break_requested() == true );
        delete pEngr;

        pEngr = creator.create_next_engrouter(5000.0f, false);

        CHECK( pEngr != nullptr );
        pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"is a paragraph" );
        CHECK( creator.more_content() == false );
        CHECK( pEngr && pEngr->break_requested() == false );
        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, next_item_after_text)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_text_item("This is a paragraph");
        pPara->add_button(m_libraryScope, "Click me!", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        Engrouter* pEngr = creator.create_next_engrouter(1000.0f, false);
        CHECK( pEngr != nullptr );
        delete pEngr;

        pEngr = creator.create_next_engrouter(5000.0f, false);
        CHECK( pEngr != nullptr );
        CHECK( creator.more_content() == true );
        CHECK( pEngr && pEngr->break_requested() == false );
        delete pEngr;

        pEngr = creator.create_next_engrouter(3000.0f, false);
        CHECK( pEngr != nullptr );
        ControlEngrouter* pEngrouter = dynamic_cast<ControlEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( creator.more_content() == false );
        CHECK( pEngr && pEngr->break_requested() == false );
        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, skip_empty_text)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_text_item("");
        pPara->add_button(m_libraryScope, "Click me!", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        Engrouter* pEngr = creator.create_next_engrouter(1000.0f, false);
        CHECK( pEngr != nullptr );
        NullEngrouter* pEngr1 = dynamic_cast<NullEngrouter*>( pEngr );
        CHECK( pEngr1 != nullptr );
        CHECK( creator.more_content() == true );
        CHECK( pEngr && pEngr->break_requested() == false );
        delete pEngr;

        pEngr = creator.create_next_engrouter(3000.0f, false);
        CHECK( pEngr != nullptr );
        ControlEngrouter* pEngr2 = dynamic_cast<ControlEngrouter*>( pEngr );
        CHECK( pEngr2 != nullptr );
        CHECK( creator.more_content() == false );
        CHECK( pEngr && pEngr->break_requested() == false );
        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, text_engrouter_remove_left_space_1)
    {
        /// remove initial spaces if first engrouter in the line
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_text_item(" This is a paragraph ");
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 1000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, true);

        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"This" );
        CHECK( creator.more_content() == true );
        CHECK( pEngr && pEngr->break_requested() == true );
        delete pEngr;

        pEngr = creator.create_next_engrouter(5000.0f, false);

        CHECK( pEngr != nullptr );
        pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"is a paragraph " );
        CHECK( creator.more_content() == false );
        CHECK( pEngr && pEngr->break_requested() == false );
        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, text_engrouter_remove_left_space_2)
    {
        /// preserve initial spaces if not first engrouter in the line
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        pPara->add_text_item(" This is a paragraph ");
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 1000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L" This" );
        CHECK( creator.more_content() == true );
        CHECK( pEngr && pEngr->break_requested() == true );
        delete pEngr;

        pEngr = creator.create_next_engrouter(5000.0f, false);

        CHECK( pEngr != nullptr );
        pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"is a paragraph " );
        CHECK( creator.more_content() == false );
        CHECK( pEngr && pEngr->break_requested() == false );
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
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr != nullptr );
        BoxEngrouter* pEngrouter = dynamic_cast<BoxEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );

        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WrapperEngrouterContentAddedToBox)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoInlineWrapper* pWp = pPara->add_inline_box();
        pWp->add_button(m_libraryScope, "Accept", USize(2000.0f, 600.0f));
        pWp->add_button(m_libraryScope, "Cancel", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 8000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr != nullptr );
        BoxEngrouter* pEngrouter = dynamic_cast<BoxEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );

        std::list<Engrouter*>& children = pEngrouter->get_engrouters();
        std::list<Engrouter*>::iterator it = children.begin();
        CHECK( children.size() == 2 );
//        cout << children.size() << endl;
        CHECK( dynamic_cast<ControlEngrouter*>(*it) != nullptr );
        ++it;
        CHECK( dynamic_cast<ControlEngrouter*>(*it) != nullptr );

        delete pEngr;
    }

    TEST_FIXTURE(EngroutersCreatorTestFixture, WrapperBoxEngrouterHasSize_Linear)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoInlineWrapper* pWp = pPara->add_inline_box();
        pWp->add_button(m_libraryScope, "Accept", USize(2000.0f, 600.0f));
        pWp->add_button(m_libraryScope, "Cancel", USize(2000.0f, 600.0f));
        MyEngroutersCreator creator(pPara, m_libraryScope);

        LUnits availableWidth = 8000.0f;
        Engrouter* pEngr = creator.create_next_engrouter(availableWidth, false);

        CHECK( pEngr != nullptr );
        BoxEngrouter* pEngrouter = dynamic_cast<BoxEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );

        //cout << "box width = " << pEngrouter->get_width() << endl;
        //cout << "box height = " << pEngrouter->get_height() << endl;
        CHECK( pEngrouter && pEngrouter->get_width() == 4000.0f );
        CHECK( pEngrouter && pEngrouter->get_height() == 900.0f );
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
//        CHECK( pEngrouter && pEngrouter->get_text() == " " );
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
//        CHECK( pEngrouter && pEngrouter->get_text() == " " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter && pEngrouter->get_text() == "This " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter && pEngrouter->get_text() == "is " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter && pEngrouter->get_text() == "a " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter && pEngrouter->get_text() == "paragraph" );
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
//        CHECK( pEngrouter && pEngrouter->get_text() == " " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter && pEngrouter->get_text() == "This " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter && pEngrouter->get_text() == "is " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter && pEngrouter->get_text() == "a " );
//        pEngrouter = dynamic_cast<WordEngrouter*>(*(++it));
//        CHECK( pEngrouter && pEngrouter->get_text() == "paragraph" );
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
//        CHECK( pEngrouter && pEngrouter->get_text() == "Hello!" );
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
//        CHECK( pEngrouter && pEngrouter->get_text() == "Hello! " );
//
//        creator.my_delete_engrouters();
//        delete pText;
//    }

};

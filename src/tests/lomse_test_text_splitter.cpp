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
#include "lomse_config.h"

//classes related to these tests
#include "lomse_text_splitter.h"
#include "lomse_injectors.h"
#include "private/lomse_document_p.h"
#include "lomse_internal_model.h"
#include "lomse_im_factory.h"
#include "lomse_engrouters.h"

#include "utf8.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// access to protected members
class MyDefaultTextSplitter : public DefaultTextSplitter
{
public:
    MyDefaultTextSplitter(ImoTextItem* pText, LibraryScope& libraryScope)
        : DefaultTextSplitter(pText, libraryScope)
    {
    }

    std::vector<LUnits>& my_get_glyph_widths() { return m_glyphWidths; }
    const wstring& my_get_glyphs() { return m_glyphs; }

};

//---------------------------------------------------------------------------------------
class DefaultTextSplitterTestFixture
{
public:
    LibraryScope m_libraryScope;

    DefaultTextSplitterTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~DefaultTextSplitterTestFixture()    //TearDown fixture
    {
    }

    ImoTextItem* prepare_test(Document& doc, const string& data)
    {
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        return pPara->add_text_item(data);
    }

    string to_str(const wstring& wtext)
    {
        string utf8result;
        utf8::utf32to8(wtext.begin(), wtext.end(), back_inserter(utf8result));
        return utf8result;
    }
};


SUITE(DefaultTextSplitterTest)
{

    TEST_FIXTURE(DefaultTextSplitterTestFixture, text_measured)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "This is a paragraph");

        MyDefaultTextSplitter splitter(pText, m_libraryScope);

        std::vector<LUnits>& widths = splitter.my_get_glyph_widths();
        CHECK( widths.size() == 19 );
        CHECK( splitter.more_text() == true );

//        //to prepare some texts
//        LUnits widthFirst = 0.0f;
//        for (int i=0; i < 6; ++i)
//            widthFirst += widths[i];
//        cout << "First chunk width = " << widthFirst << endl;
    }

    TEST_FIXTURE(DefaultTextSplitterTestFixture, text_measured_non_latin)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "This is Chinese 音乐老师");

        MyDefaultTextSplitter splitter(pText, m_libraryScope);

        std::vector<LUnits>& widths = splitter.my_get_glyph_widths();
        CHECK( widths.size() == 20 );
    }

    TEST_FIXTURE(DefaultTextSplitterTestFixture, first_chunk)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "This is a paragraph");
        DefaultTextSplitter splitter(pText, m_libraryScope);

        Engrouter* pEngr = splitter.get_next_text_engrouter(1000.0f, false);

        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"This" );
//        cout << "chunk = '" << to_str( pEngrouter->get_text() ) << "'" << endl;
//        cout << "size = " << pEngrouter->get_width() << endl;
        CHECK( splitter.more_text() == true );

        delete pEngr;
    }

    TEST_FIXTURE(DefaultTextSplitterTestFixture, next_chunk)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "This is a paragraph");
        DefaultTextSplitter splitter(pText, m_libraryScope);

        Engrouter* pEngr = splitter.get_next_text_engrouter(1000.0f, false);
        delete pEngr;
        pEngr = splitter.get_next_text_engrouter(6000.0f, false);

        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"is a paragraph" );
//        cout << "chunk = '" << to_str( pEngrouter->get_text() ) << "'" << endl;
//        cout << "size = " << pEngrouter->get_width() << endl;
        CHECK( splitter.more_text() == false );

        delete pEngr;
    }

    TEST_FIXTURE(DefaultTextSplitterTestFixture, no_more_chunks)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "This is a paragraph");
        DefaultTextSplitter splitter(pText, m_libraryScope);

        Engrouter* pEngr = splitter.get_next_text_engrouter(1000.0f, false);
        delete pEngr;
        pEngr = splitter.get_next_text_engrouter(6000.0f, false);
        delete pEngr;
        CHECK( splitter.more_text() == false );

        pEngr = splitter.get_next_text_engrouter(6000.0f, false);
        CHECK( pEngr != nullptr );
        NullEngrouter* pEngrouter = dynamic_cast<NullEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( splitter.more_text() == false );

        delete pEngr;
    }

    TEST_FIXTURE(DefaultTextSplitterTestFixture, empty_text)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "");
        DefaultTextSplitter splitter(pText, m_libraryScope);

        CHECK( splitter.more_text() == false );

        Engrouter* pEngr = splitter.get_next_text_engrouter(1000.0f, false);
        CHECK( pEngr != nullptr );
        NullEngrouter* pEngrouter = dynamic_cast<NullEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( splitter.more_text() == false );

        delete pEngr;
    }

    TEST_FIXTURE(DefaultTextSplitterTestFixture, first_chunk_several_spaces)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "This   is a paragraph");
        DefaultTextSplitter splitter(pText, m_libraryScope);

        Engrouter* pEngr = splitter.get_next_text_engrouter(1000.0f, false);

        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"This" );
//        cout << "chunk = '" << to_str( pEngrouter->get_text() ) << "'" << endl;
//        cout << "size = " << pEngrouter->get_width() << endl;
        CHECK( splitter.more_text() == true );
        delete pEngr;

        pEngr = splitter.get_next_text_engrouter(6000.0f, false);

        CHECK( pEngr != nullptr );
        pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"is a paragraph" );
//        cout << "chunk = '" << to_str( pEngrouter->get_text() ) << "'" << endl;
//        cout << "size = " << pEngrouter->get_width() << endl;
        CHECK( splitter.more_text() == false );

        delete pEngr;
    }

    TEST_FIXTURE(DefaultTextSplitterTestFixture, first_just_before_space)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "Hello world!");
        DefaultTextSplitter splitter(pText, m_libraryScope);

        Engrouter* pEngr = splitter.get_next_text_engrouter(1000.0f, false);

        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        CHECK( pEngrouter && pEngrouter->get_text() == L"Hello" );
//        cout << "chunk = '" << to_str( pEngrouter->get_text() ) << "'" << endl;
//        cout << "size = " << pEngrouter->get_width() << endl;
        CHECK( splitter.more_text() == true );
        delete pEngr;
    }

};


//=======================================================================================
// Chinese text splitter tests
//=======================================================================================

//---------------------------------------------------------------------------------------
// access to protected members
class MyChineseTextSplitter : public ChineseTextSplitter
{
public:
    MyChineseTextSplitter(ImoTextItem* pText, LibraryScope& libraryScope)
        : ChineseTextSplitter(pText, libraryScope)
    {
    }

    std::vector<LUnits>& my_get_glyph_widths() { return m_glyphWidths; }
    const wstring& my_get_glyphs() { return m_glyphs; }

};

//---------------------------------------------------------------------------------------
class ChineseTextSplitterTestFixture
{
public:
    LibraryScope m_libraryScope;

    ChineseTextSplitterTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ChineseTextSplitterTestFixture()    //TearDown fixture
    {
    }

    ImoTextItem* prepare_test(Document& doc, const string& data)
    {
        doc.create_empty();
        ImoStyle* pStyle = doc.create_style("chinese");
        pStyle->font_name("Noto Sans CJK SC");  //WenQuanYi Zen Hei");
        ImoParagraph* pPara = doc.add_paragraph();
        ImoTextItem* pText = pPara->add_text_item(data, pStyle);
        pText->set_language("zh_CN");
        return pText;
    }

    bool is_valid_font(WordEngrouter* pEngrouter)
    {
        const string& fontpath = pEngrouter->get_font_file();
        return (fontpath.find("NotoSansCJK") != std::string::npos);
    }

    string to_str(const wstring& wtext)
    {
        string utf8result;
        utf8::utf32to8(wtext.begin(), wtext.end(), back_inserter(utf8result));
        return utf8result;
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }
};


SUITE(ChineseTextSplitterTest)
{

    TEST_FIXTURE(ChineseTextSplitterTestFixture, text_measured)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "编辑名称，缩写，MIDI设置和其他特性");

        MyChineseTextSplitter splitter(pText, m_libraryScope);

        std::vector<LUnits>& widths = splitter.my_get_glyph_widths();
        CHECK( widths.size() == 19 );
        CHECK( splitter.more_text() == true );

        ////to prepare some texts
        //LUnits widthFirst = 0.0f;
        //for (int i=0; i < 6; ++i)
        //    widthFirst += widths[i];
        //cout << "First chunk width = " << widthFirst << endl;       //2540.0f
    }

    TEST_FIXTURE(ChineseTextSplitterTestFixture, first_chunk)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "编辑名称，缩写，MIDI设置和其他特性");
        ChineseTextSplitter splitter(pText, m_libraryScope);

        Engrouter* pEngr = splitter.get_next_text_engrouter(2600.0f, false);

        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        if (is_valid_font(pEngrouter))
        {
            CHECK( to_str( pEngrouter->get_text() ) == "编辑名称，缩" );
            //cout << "chunk = '" << to_str( pEngrouter->get_text() ) << "'" << endl;
            //cout << "size = " << pEngrouter->get_width() << endl;
            CHECK( splitter.more_text() == true );
        }
        else
            cout << "Test ChineseTextSplitterTest " << test_name()
                 << " skipped. Needed font not installed." << endl;

        delete pEngr;
    }

    TEST_FIXTURE(ChineseTextSplitterTestFixture, next_chunk)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "编辑名称，缩写，MIDI设置和其他特性");
        ChineseTextSplitter splitter(pText, m_libraryScope);

        Engrouter* pEngr = splitter.get_next_text_engrouter(2600.0f, false);
        CHECK( splitter.more_text() == true );
        CHECK( pEngr != nullptr );
        WordEngrouter* pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        if (!is_valid_font(pEngrouter))
        {
            cout << "Test ChineseTextSplitterTest " << test_name()
                 << " skipped. Needed font not installed." << endl;
        }
        delete pEngr;

        pEngr = splitter.get_next_text_engrouter(10000.0f, false);
        CHECK( pEngr != nullptr );
        pEngrouter = dynamic_cast<WordEngrouter*>( pEngr );
        CHECK( pEngrouter != nullptr );
        if (pEngrouter)
        {
            CHECK( to_str( pEngrouter->get_text() ) == "写，MIDI设置和其他特性" );
            //cout << "chunk = '" << to_str( pEngrouter->get_text() ) << "'" << endl;
            //cout << "size = " << pEngrouter->get_width() << endl;
            CHECK( splitter.more_text() == false );
        }
        delete pEngr;
    }

    TEST_FIXTURE(ChineseTextSplitterTestFixture, no_more_chunks)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = prepare_test(doc, "编辑名称，缩写，MIDI设置和其他特性");
        ChineseTextSplitter splitter(pText, m_libraryScope);

        Engrouter* pEngr = splitter.get_next_text_engrouter(2600.0f, false);
        delete pEngr;
        pEngr = splitter.get_next_text_engrouter(10000.0f, false);
        delete pEngr;
        CHECK( splitter.more_text() == false );

        pEngr = splitter.get_next_text_engrouter(5000.0f, false);
        CHECK( pEngr == nullptr );
        CHECK( splitter.more_text() == false );
        delete pEngr;
    }

}


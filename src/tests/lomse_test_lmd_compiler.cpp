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
#include "lomse_injectors.h"
#include "private/lomse_document_p.h"
#include "lomse_lmd_compiler.h"
#include "lomse_internal_model.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LmdCompilerTestFixture
{
public:

    LibraryScope m_libraryScope;
    std::string m_scores_path;

    LmdCompilerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~LmdCompilerTestFixture()    //TearDown fixture
    {
    }
};

SUITE(LmdCompilerTest)
{
    TEST_FIXTURE(LmdCompilerTestFixture, LmdCompilerCreateEmpty)
    {
        Document doc(m_libraryScope);
        LmdCompiler compiler(m_libraryScope, &doc);
        ImoObj* pRoot =  compiler.create_empty();
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
        CHECK( pDoc->get_version() == "0.0" );
        CHECK( pDoc->get_num_content_items() == 0 );
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LmdCompilerTestFixture, LmdCompilerFromString)
    {
        Document doc(m_libraryScope);
        LmdCompiler compiler(m_libraryScope, &doc);
        string src =
            "<lenmusdoc vers='0.0'>"
                "<content>"
                    "<ldpmusic>"
                        "(score (vers 1.6)(instrument (musicData (clef G)(n c4 q.))))"
                    "</ldpmusic>"
                "</content>"
            "</lenmusdoc>";
        ImoObj* pRoot =  compiler.compile_string(src);
        CHECK( compiler.get_file_locator() == "string:" );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
        CHECK( pDoc->get_version() == "0.0" );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_staffobjs_table() != nullptr );
        CHECK( pScore->get_version_string() == "1.6" );
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //TEST_FIXTURE(LmdCompilerTestFixture, LmdCompilerScoreIdsFixed)
    //{
    //    Document doc(m_libraryScope);
    //    LmdCompiler compiler(m_libraryScope, &doc);
    //    ImoObj* pRoot =  compiler.compile_string("(score (vers 1.6) (language en iso-8859-1) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.Truncate 1) (instrument (musicData)))" );
    //    ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
    //    CHECK( pDoc->get_version() == "0.0" );
    //    CHECK( pDoc->get_num_content_items() == 1 );
    //    ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
    //    CHECK( pScore != nullptr );
    //    //cout << "id=" << pScore->get_id() << endl;
    //    CHECK( pScore->get_id() == 3L );
    //    CHECK( pScore->get_num_instruments() == 1 );
    //    ImoInstrument* pInstr = pScore->get_instrument(0);
    //    CHECK( pInstr->get_id() == 12L );
    //    if (pRoot && !pRoot->is_document()) delete pRoot;
    //}

    TEST_FIXTURE(LmdCompilerTestFixture, LmdCompilerFromFile)
    {
        Document doc(m_libraryScope);
        LmdCompiler compiler(m_libraryScope, &doc);
        string path = m_scores_path + "08011-paragraph.lmd";
        ImoObj* pRoot =  compiler.compile_file(path);
        CHECK( compiler.get_file_locator() == path );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
        CHECK( pDoc->get_version() == "0.0" );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != nullptr );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "This is a text for this example." );

        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

//    TEST_FIXTURE(LmdCompilerTestFixture, LmdCompilerFromInput)
//    {
//        Document doc(m_libraryScope);
//        LmdCompiler compiler(m_libraryScope, &doc);
//        string path = m_scores_path + "00011-empty-fill-page.lms";
//        LdpFileReader reader(path);
//        ImoObj* pRoot =  compiler.compile_input(reader);
//        CHECK( compiler.get_file_locator() == path );
//    //    ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
//    //    CHECK( pDoc->get_version() == "0.0" );
//    //    CHECK( pDoc->get_num_content_items() == 1 );
//    //    ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
//    //    CHECK( pScore != nullptr );
//    //    CHECK( pScore->get_id() == 3L );
//    //    CHECK( pScore->get_num_instruments() == 1 );
//    //    ImoInstrument* pInstr = pScore->get_instrument(0);
//    //    CHECK( pInstr->get_id() == 11L );
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }

};


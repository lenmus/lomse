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
#include "lomse_ldp_compiler.h"
#include "lomse_internal_model.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LdpCompilerTestFixture
{
public:

    LibraryScope m_libraryScope;
    std::string m_scores_path;

    LdpCompilerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~LdpCompilerTestFixture()    //TearDown fixture
    {
    }
};

SUITE(LdpCompilerTest)
{
    TEST_FIXTURE(LdpCompilerTestFixture, LdpCompilerCreateEmpty)
    {
        Document doc(m_libraryScope);
        LdpCompiler compiler(m_libraryScope, &doc);
        ImoObj* pRoot =  compiler.create_empty();
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_version() == "0.0" );
            CHECK( pDoc->get_num_content_items() == 0 );
        }

        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpCompilerTestFixture, LdpCompilerFromString)
    {
        Document doc(m_libraryScope);
        LdpCompiler compiler(m_libraryScope, &doc);
        ImoObj* pRoot =  compiler.compile_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.Truncate 1) (instrument (musicData)))))" );
        CHECK( compiler.get_file_locator() == "string:" );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_version() == "0.0" );
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            CHECK( pScore && pScore->get_num_instruments() == 1 );
            CHECK( pScore && pScore->get_staffobjs_table() != nullptr );
            CHECK( pScore && pScore->get_version_string() == "1.6" );
        }

        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpCompilerTestFixture, LdpCompilerFromFile)
    {
        Document doc(m_libraryScope);
        LdpCompiler compiler(m_libraryScope, &doc);
        string path = m_scores_path + "00011-empty-fill-page.lms";
        ImoObj* pRoot =  compiler.compile_file(path);
        CHECK( compiler.get_file_locator() == path );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_version() == "0.0" );
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            CHECK( pScore && pScore->get_num_instruments() == 1 );
        }

        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpCompilerTestFixture, LdpCompilerFromInput)
    {
        Document doc(m_libraryScope);
        LdpCompiler compiler(m_libraryScope, &doc);
        string path = m_scores_path + "00011-empty-fill-page.lms";
        LdpFileReader reader(path);
        ImoObj* pRoot =  compiler.compile_input(reader);
        CHECK( compiler.get_file_locator() == path );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_version() == "0.0" );
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            CHECK( pScore && pScore->get_num_instruments() == 1 );
        }

        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

};


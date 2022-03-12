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
#include "lomse_file_system.h"
//#include "lomse_ldp_analyser.h"
//#include "lomse_internal_model.h"
//#include "lomse_im_note.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class DocLocatorTestFixture
{
public:

    DocLocatorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~DocLocatorTestFixture()    //TearDown fixture
    {
    }

    LibraryScope m_libraryScope;
};

SUITE(DocLocatorTest)
{

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_1)
    {
        DocLocator loc("/data/books/book1.lms");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_full_path() == "/data/books/book1.lms" );
        CHECK( loc.get_path() == "/data/books/" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_none );
        CHECK( loc.get_inner_fullpath() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_as_string() == "/data/books/book1.lms" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_2)
    {
        DocLocator loc("c:\\data\\books\\book1.lms");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_full_path() == "c:\\data\\books\\book1.lms" );
        CHECK( loc.get_path() == "c:\\data\\books\\" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_none );
        CHECK( loc.get_inner_fullpath() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_as_string() == "c:\\data\\books\\book1.lms" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_3)
    {
        DocLocator loc("file:/data/books/book1.lms");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_full_path() == "/data/books/book1.lms" );
        CHECK( loc.get_path() == "/data/books/" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_none );
        CHECK( loc.get_inner_fullpath() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_as_string() == "/data/books/book1.lms" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_4)
    {
        DocLocator loc("file:c:\\data\\books\\book1.lms");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_full_path() == "c:\\data\\books\\book1.lms" );
        CHECK( loc.get_path() == "c:\\data\\books\\" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_none );
        CHECK( loc.get_inner_fullpath() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_as_string() == "c:\\data\\books\\book1.lms" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_5)
    {
        DocLocator loc("/data/books/book1.lmb#zip:images/picture1.png");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_full_path() == "/data/books/book1.lmb" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_zip );
        CHECK( loc.get_inner_fullpath() == "images/picture1.png" );
        CHECK( loc.get_inner_file() == "picture1.png" );
        CHECK( loc.get_locator_as_string() == "/data/books/book1.lmb#zip:images/picture1.png" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_6)
    {
        DocLocator loc("/data/books/book1.lmb#zip:");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_full_path() == "/data/books/book1.lmb" );
        CHECK( loc.get_path() == "/data/books/" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_zip );
        CHECK( loc.get_inner_fullpath() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_as_string() == "/data/books/book1.lmb#zip:" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_string_1)
    {
        DocLocator loc("string:");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_string );
        CHECK( loc.get_full_path() == "" );
        CHECK( loc.get_path() == "" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_none );
        CHECK( loc.get_inner_fullpath() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_as_string() == "string:" );
    }

//    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_error_1)
//    {
//        DocLocator loc("c:\\data\\books\\book1.lmb/zip:images/picture1.png");
//        CHECK( loc.is_valid() == false );
//    }
//

}


//---------------------------------------------------------------------------------------
class LmbDocLocatorTestFixture
{
public:

    LmbDocLocatorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~LmbDocLocatorTestFixture()    //TearDown fixture
    {
    }

    LibraryScope m_libraryScope;
};

SUITE(LmbDocLocatorTest)
{

    TEST_FIXTURE(LmbDocLocatorTestFixture, LmbDocLocator_1)
    {
        LmbDocLocator loc("/data/books/book1.lms");
        CHECK( loc.is_valid_lmb() == false );
    }

    TEST_FIXTURE(LmbDocLocatorTestFixture, LmbDocLocator_2)
    {
        LmbDocLocator loc("/data/books/book1.lmb#zip:content/page1.lms");
        CHECK( loc.is_valid_lmb() == true );
    }

}

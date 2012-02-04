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
#include "lomse_injectors.h"
#include "lomse_file_system.h"
//#include "lomse_analyser.h"
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
        CHECK( loc.get_path() == "/data/books/book1.lms" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_none );
        CHECK( loc.get_inner_path() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_string() == "/data/books/book1.lms" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_2)
    {
        DocLocator loc("c:\\data\\books\\book1.lms");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_path() == "c:\\data\\books\\book1.lms" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_none );
        CHECK( loc.get_inner_path() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_string() == "c:\\data\\books\\book1.lms" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_3)
    {
        DocLocator loc("file:/data/books/book1.lms");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_path() == "/data/books/book1.lms" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_none );
        CHECK( loc.get_inner_path() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_string() == "/data/books/book1.lms" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_4)
    {
        DocLocator loc("file:c:\\data\\books\\book1.lms");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_path() == "c:\\data\\books\\book1.lms" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_none );
        CHECK( loc.get_inner_path() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_string() == "c:\\data\\books\\book1.lms" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_5)
    {
        DocLocator loc("/data/books/book1.lmb#zip:images/picture1.png");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_path() == "/data/books/book1.lmb" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_zip );
        CHECK( loc.get_inner_path() == "images/picture1.png" );
        CHECK( loc.get_inner_file() == "picture1.png" );
        CHECK( loc.get_locator_string() == "/data/books/book1.lmb#zip:images/picture1.png" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_6)
    {
        DocLocator loc("/data/books/book1.lmb#zip:");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_file );
        CHECK( loc.get_path() == "/data/books/book1.lmb" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_zip );
        CHECK( loc.get_inner_path() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_string() == "/data/books/book1.lmb#zip:" );
    }

    TEST_FIXTURE(DocLocatorTestFixture, DocLocator_string_1)
    {
        DocLocator loc("string:");
        CHECK( loc.is_valid() == true );
        CHECK( loc.get_protocol() == DocLocator::k_string );
        CHECK( loc.get_path() == "" );
        CHECK( loc.get_inner_protocol() == DocLocator::k_none );
        CHECK( loc.get_inner_path() == "" );
        CHECK( loc.get_inner_file() == "" );
        CHECK( loc.get_locator_string() == "string:" );
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

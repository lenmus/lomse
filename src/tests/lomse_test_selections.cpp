//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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
#include "lomse_gm_basic.h"
#include "lomse_selections.h"
#include "lomse_shapes.h"
#include "lomse_document.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;



//---------------------------------------------------------------------------------------
// helper class for accessing protected members
class MySelectionSet : public SelectionSet
{
public:
    MySelectionSet(Document* pDoc) : SelectionSet(pDoc) {}
    ~MySelectionSet() {}

    void clear_valid() { m_fValid = false; }

};

//---------------------------------------------------------------------------------------
class SelectionSetTestFixture
{
public:
    LibraryScope m_libraryScope;

    SelectionSetTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~SelectionSetTestFixture()    //TearDown fixture
    {
    }
};

SUITE(SelectionSetTest)
{

    TEST_FIXTURE(SelectionSetTestFixture, SelectionSetTest_0000)
    {
        //0000. Initially empty and valid
        Document doc(m_libraryScope);
        MySelectionSet sel(&doc);

        CHECK( sel.is_valid() == true );
        CHECK( sel.empty() == true );
    }

    TEST_FIXTURE(SelectionSetTestFixture, SelectionSetTest_0001)
    {
        //0001. clearing not valid set, sets it as valid
        Document doc(m_libraryScope);
        MySelectionSet sel(&doc);
        sel.clear_valid();
        CHECK( sel.is_valid() == false );

        sel.clear();

        CHECK( sel.is_valid() == true );
    }

    TEST_FIXTURE(SelectionSetTestFixture, SelectionSetTest_0101)
    {
        //0101. Adding GmoObj to valid set, keeps it valid
        Document doc(m_libraryScope);
        MySelectionSet sel(&doc);
        GmoShapeClef clef(nullptr, 1, 1, UPoint(0.0f, 0.0f), Color(0,0,0),
                          m_libraryScope, 21.0);
        //CHECK( sel.contains(&clef) == false );

        sel.add(&clef);

        //CHECK( sel.contains(&clef) == true );
        CHECK( sel.is_valid() == true );
    }

    TEST_FIXTURE(SelectionSetTestFixture, SelectionSetTest_Clear)
    {
        Document doc(m_libraryScope);
        MySelectionSet sel(&doc);
        GmoShapeClef clef(nullptr, 1, 1, UPoint(0.0f, 0.0f), Color(0,0,0),
                          m_libraryScope, 21.0);
        sel.add(&clef);
        sel.clear();

        //CHECK( sel.contains(&clef) == false );
//        CHECK( clef.is_selected() == false );
    }


}



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
#include "lomse_gm_basic.h"
#include "lomse_selections.h"
#include "lomse_shapes.h"
#include "private/lomse_document_p.h"

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



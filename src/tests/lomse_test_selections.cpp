//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_selections.h"
#include "lomse_shapes.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class SelectionsTestFixture
{
public:
    LibraryScope m_libraryScope;

    SelectionsTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
    }

    ~SelectionsTestFixture()    //TearDown fixture
    {
    }
};

SUITE(SelectionsTest)
{

    TEST_FIXTURE(SelectionsTestFixture, SelectionsTest_Add)
    {
        SelectionSet set;
        GmoShapeClef clef(NULL, 1, 1, UPoint(0.0f, 0.0f), Color(0,0,0), 
                          m_libraryScope, 21.0);

        CHECK( set.contains(&clef) == false );
        CHECK( clef.is_selected() == false );

        set.add(&clef);

        CHECK( set.contains(&clef) == true );
        CHECK( clef.is_selected() == true );
    }

    TEST_FIXTURE(SelectionsTestFixture, SelectionsTest_Clear)
    {
        SelectionSet set;
        GmoShapeClef clef(NULL, 1, 1, UPoint(0.0f, 0.0f), Color(0,0,0), 
                          m_libraryScope, 21.0);
        set.add(&clef);
        set.clear();

        CHECK( set.contains(&clef) == false );
        CHECK( clef.is_selected() == false );
    }


}



//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_parser.h"
#include "lomse_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LinkerTestFixture
{
public:

    LinkerTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
    }

    ~LinkerTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
};

SUITE(LinkerTest)
{

//    TEST_FIXTURE(LinkerTestFixture, AnalyserOneOrMorePresentOne)
//    {
//        stringstream errormsg;
//        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        SpLdpTree tree = parser.parse_text("(score (vers 1.6)(instrument (musicData)))");
//        Analyser a(errormsg, m_pLibraryScope->ldp_factory());
//        a.analyse_tree(tree);
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImScore* pScore = dynamic_cast<ImScore*>( tree->get_root()->get_imobj() );
//        CHECK( pScore != NULL );
//        //CHECK( pScore->get_num_instruments() == 1 );
//        delete tree->get_root();
//    }

}


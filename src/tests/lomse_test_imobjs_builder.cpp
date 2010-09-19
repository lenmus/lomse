//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifdef _LM_DEBUG_

#include <UnitTest++.h>
#include <sstream>

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_parser.h"
#include "lomse_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_basic_model.h"
#include "lomse_model_builder.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class ImObjsBuilderTestFixture
{
public:

    ImObjsBuilderTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_pLdpFactory = m_pLibraryScope->ldp_factory();
    }

    ~ImObjsBuilderTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    LdpFactory* m_pLdpFactory;
};


SUITE(ImObjsBuilderTest)
{

    TEST_FIXTURE(ImObjsBuilderTestFixture, ImObjsBuilder_Empty)
    {
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 2.0) (content))" );
        Analyser a(cout, m_pLdpFactory);
        BasicModel* pBasicModel = a.analyse_tree(tree);
        ImObjectsBuilder imb(cout);
        ImoDocument* pDoc = imb.create_objects(pBasicModel);

        CHECK( pDoc != NULL );
        CHECK( pDoc->get_version() == "2.0" );
        CHECK( pDoc->get_num_content_items() == 0 );
    }

}


#endif  // _LM_DEBUG_


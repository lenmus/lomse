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
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_document_layouter.h"
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class DocLayouterTestFixture
{
public:

    DocLayouterTestFixture()
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_pLdpFactory = m_pLibraryScope->ldp_factory();
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~DocLayouterTestFixture()
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    LdpFactory* m_pLdpFactory;
    std::string m_scores_path;
};

SUITE(DocLayouterTest)
{

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_ReturnsGModel)
    {
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model() );
        USize pagesize(21000.0f, 29700.0f);
        dl.layout_document(pagesize);
        GraphicModel* pGModel = dl.get_gm_model();
        CHECK( pGModel != NULL );
        CHECK( pGModel->get_num_pages() == 1 );
        delete pGModel;
    }

};

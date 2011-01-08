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
#include "lomse_document.h"
#include "lomse_parser.h"
#include "lomse_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_compiler.h"
#include "lomse_model_builder.h"
#include "lomse_basic_model.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class ModelBuilderTestFixture
{
public:

    ModelBuilderTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~ModelBuilderTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

SUITE(ModelBuilderTest)
{

    //This just checks that the score has an associated ColStaffObjs
    TEST_FIXTURE(ModelBuilderTestFixture, ModelBuilderScore)
    {
        DocumentScope documentScope(cout);
        LdpParser* parser = Injector::inject_LdpParser(*m_pLibraryScope, documentScope);
        Analyser* analyser = Injector::inject_Analyser(*m_pLibraryScope, documentScope);
        ModelBuilder* builder = Injector::inject_ModelBuilder(documentScope);
        LdpCompiler compiler(parser, analyser, builder, documentScope.id_assigner());
        InternalModel* pIModel = compiler.compile_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (barline simple))))))" );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pIModel->get_root());
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_staffobjs_table() != NULL );

        delete pIModel;
    }

}



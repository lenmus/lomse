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
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_compiler.h"
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
        m_scores_path = LOMSE_TEST_SCORES_PATH;
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
        InternalModel* pIModel = compiler.create_empty();
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pIModel->get_root());
        CHECK( pDoc->get_version() == "0.0" );
        CHECK( pDoc->get_num_content_items() == 0 );
        delete pIModel;
    }

    TEST_FIXTURE(LdpCompilerTestFixture, LdpCompilerFromString)
    {
        Document doc(m_libraryScope);
        LdpCompiler compiler(m_libraryScope, &doc);
        InternalModel* pIModel = compiler.compile_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData )))))" );
        CHECK( compiler.get_file_locator() == "string:" );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pIModel->get_root());
        CHECK( pDoc->get_version() == "0.0" );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_staffobjs_table() != NULL );
        CHECK( pScore->get_version() == "1.6" );
        delete pIModel;
    }

    //TEST_FIXTURE(LdpCompilerTestFixture, LdpCompilerScoreIdsFixed)
    //{
    //    Document doc(m_libraryScope);
    //    LdpCompiler compiler(m_libraryScope, &doc);
    //    InternalModel* pIModel = compiler.compile_string("(score (vers 1.6) (language en iso-8859-1) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData )))" );
    //    ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pIModel->get_root());
    //    CHECK( pDoc->get_version() == "0.0" );
    //    CHECK( pDoc->get_num_content_items() == 1 );
    //    ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
    //    CHECK( pScore != NULL );
    //    //cout << "id=" << pScore->get_id() << endl;
    //    CHECK( pScore->get_id() == 3L );
    //    CHECK( pScore->get_num_instruments() == 1 );
    //    ImoInstrument* pInstr = pScore->get_instrument(0);
    //    CHECK( pInstr->get_id() == 12L );
    //    delete pIModel;
    //}

    TEST_FIXTURE(LdpCompilerTestFixture, LdpCompilerFromFile)
    {
        Document doc(m_libraryScope);
        LdpCompiler compiler(m_libraryScope, &doc);
        string path = m_scores_path + "00011-empty-fill-page.lms";
        InternalModel* pIModel = compiler.compile_file(path);
        CHECK( compiler.get_file_locator() == path );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pIModel->get_root());
        CHECK( pDoc->get_version() == "0.0" );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
    //    CHECK( pScore->get_id() == 3L );
        CHECK( pScore->get_num_instruments() == 1 );
    //    ImoInstrument* pInstr = pScore->get_instrument(0);
    //    CHECK( pInstr->get_id() == 11L );
        delete pIModel;
    }

    TEST_FIXTURE(LdpCompilerTestFixture, LdpCompilerFromInput)
    {
        Document doc(m_libraryScope);
        LdpCompiler compiler(m_libraryScope, &doc);
        string path = m_scores_path + "00011-empty-fill-page.lms";
        LdpFileReader reader(path);
        InternalModel* pIModel = compiler.compile_input(reader);
        CHECK( compiler.get_file_locator() == path );
    //    ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pIModel->get_root());
    //    CHECK( pDoc->get_version() == "0.0" );
    //    CHECK( pDoc->get_num_content_items() == 1 );
    //    ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
    //    CHECK( pScore != NULL );
    //    CHECK( pScore->get_id() == 3L );
    //    CHECK( pScore->get_num_instruments() == 1 );
    //    ImoInstrument* pInstr = pScore->get_instrument(0);
    //    CHECK( pInstr->get_id() == 11L );
        delete pIModel;
    }

};


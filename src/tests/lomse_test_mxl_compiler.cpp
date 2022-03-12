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
#include "private/lomse_document_p.h"
#include "lomse_mxl_compiler.h"
#include "lomse_internal_model.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class MxlCompilerTestFixture
{
public:

    LibraryScope m_libraryScope;
    std::string m_scores_path;

    MxlCompilerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~MxlCompilerTestFixture()    //TearDown fixture
    {
    }
};

SUITE(MxlCompilerTest)
{

    TEST_FIXTURE(MxlCompilerTestFixture, MxlCompilerFromString_001)
    {
        //001 - compile from string
        Document doc(m_libraryScope);
        MxlCompiler compiler(m_libraryScope, &doc);
        string src =
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<divisions>1</divisions><key><fifths>0</fifths></key>"
                "<time><beats>4</beats><beat-type>4</beat-type></time>"
                "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><type>whole</type></note>"
            "</measure>"
            "</part></score-partwise>";
        ImoObj* pRoot =  compiler.compile_string(src);
        CHECK( compiler.get_file_locator() == "string:" );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
        CHECK( pDoc && pDoc->get_version() == "0.0" );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        CHECK( pScore && pScore->get_staffobjs_table() != nullptr );
        delete pRoot;
    }

    TEST_FIXTURE(MxlCompilerTestFixture, MxlCompilerFromFile_100)
    {
        //100 - compile raw xml file format
        Document doc(m_libraryScope);
        MxlCompiler compiler(m_libraryScope, &doc);
        string path = m_scores_path + "50000-hello-world.xml";
        ImoObj* pRoot =  compiler.compile_file(path);
        CHECK( compiler.get_file_locator() == path );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
        CHECK( pDoc && pDoc->get_version() == "0.0" );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        CHECK( pScore && pScore->get_staffobjs_table() != nullptr );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr && pInstr->get_num_staves() == 1 );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );
        CHECK( pMD && pMD->get_num_items() == 5 );
        ImoObj* pImo = pMD->get_first_child();
        CHECK( pImo && pImo->is_clef() == true );

//        cout << "Test: MxlCompilerFromFile_100" << endl;
//        cout << doc.to_string() << endl;

        delete pRoot;
    }

};


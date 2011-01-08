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

#ifdef _LM_DEBUG_

#include <UnitTest++.h>
#include <sstream>

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_ldp_exporter.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"

#include "lomse_compiler.h"
#include "lomse_basic_model.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


class LdpExporterTestFixture
{
public:

    LdpExporterTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_scores_path = "../../../test-scores/";        //linux CodeBlobks
        //m_scores_path = "../../../../test-scores/";        //windows MS Visual studio .NET
    }

    ~LdpExporterTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

SUITE(LdpExporterTest)
{
    // clef ------------------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, ExportLdp_clef)
    {
        ImoClef obj(ImoClef::k_F4);
        LdpExporter exporter;
        string source = exporter.get_source(&obj);
        //cout << "\"" << source << "\"" << endl;
        CHECK( source == "(clef F4 p1)" );
    }

    // lenmusdoc ----------------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, ExportLdp_lenmusdoc_empty)
    {
        ImoDocument doc("2.3");
        LdpExporter exporter;
        string source = exporter.get_source(&doc);
        //cout << source << endl;
        CHECK( source == "(lenmusdoc (vers 2.3) (content))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, ExportLdp_ErrorNotImplemented)
    {
        ImoTie tie;
        LdpExporter exporter;
        string source = exporter.get_source(&tie);
        //cout << source << endl;
        CHECK( source == "(TODO: Add this element to LdpExporter::new_generator)" );
    }

    // color ------------------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, ExportLdp_color)
    {
        ImoClef obj(ImoClef::k_G2);
        obj.set_color( rgba16(127, 40, 12, 128) );
        LdpExporter exporter;
        string source = exporter.get_source(&obj);
        //cout << "\"" << source << "\"" << endl;
        CHECK( source == "(clef G p1 (color #7f280c80))" );
    }

//    // user location ----------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, ExportLdp_user_location)
//    {
//        ImoClef obj;
//        obj.set_type(ImoClef::k_G2);
//        obj.set_user_location_x(30.0f);
//        obj.set_user_location_y(-7.05f);
//        LdpExporter exporter;
//        string source = exporter.get_source(&obj);
//        cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(clef G3 p1 (dx 30.0000) (dy -7.0500))" );
//    }
//
//    // note ------------------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, ExportLdp_Note)
//    {
//        ImoNote obj;
//        obj.set_octave(4);
//        obj.set_step(ImoNote::D);
//        obj.set_duration(ImoNote::k_eighth);
//        //obj.set_dots(1);
//        LdpExporter exporter;
//        string source = exporter.get_source(&obj);
//        cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(n d4 e. p1)" );
//    }

    // compiler usage ---------------------------------------------------------------------------

//    TEST_FIXTURE(LdpExporterTestFixture, ExportLdp_Compiler)
//    {
//        DocumentScope documentScope(cout);
//        LdpCompiler compiler(*m_pLibraryScope, documentScope);
//        BasicModel* pBasicModel = compiler.create_basic_model("(n d4 e.)" );
//        ImoObj* pObj = pBasicModel->get_root();
//        CHECK( pObj != NULL );
//        CHECK( pObj->is_note() == true );
//        LdpExporter exporter;
//        string source = exporter.get_source(pObj);
//        cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(n d4 e. p1)" );
//        delete pBasicModel;
//    }
};

#endif  // _LM_DEBUG_


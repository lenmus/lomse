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
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_basic_model.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class InternalModelTestFixture
{
public:

    InternalModelTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_scores_path = LML_TEST_SCORES_PATH;
    }

    ~InternalModelTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

SUITE(InternalModelTest)
{

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_Clef)
    {
        ImoClef* pClef = new ImoClef(ImoClef::k_G3);
        CHECK( pClef->has_attachments() == false );
        delete pClef;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ClefWithAttachments)
    {
        ImoClef* pClef = new ImoClef(ImoClef::k_G3);
        CHECK( pClef->has_attachments() == false );
        ImoTextString* pText = new ImoTextString("Hello world");
        pClef->attach(pText);
        CHECK( pClef->has_attachments() == true );
        ImoObj* pImo = pClef->get_attachment(0);
        CHECK( pImo->is_text_string() );
        ImoTextString* pTS = dynamic_cast<ImoTextString*>( pImo );
        CHECK( pTS->get_text() == "Hello world" );
        delete pClef;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_EmptyDocument)
    {
        ImoDocument* pDoc = new ImoDocument("3.7");
        CHECK( pDoc->get_content() == NULL );
        CHECK( pDoc->get_num_content_items() == 0 );
        CHECK( pDoc->has_attachments() == false );
        CHECK( pDoc->get_version() == "3.7" );
        delete pDoc;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_DocumentWithText)
    {
        ImoDocument* pDoc = new ImoDocument();
        ImoContent* pContent = new ImoContent();
        pDoc->append_child(pContent);
        ImoTextString* pText = new ImoTextString("Hello world");
        pContent->append_child(pText);
        CHECK( pDoc->get_num_content_items() == 1 );
        CHECK( pDoc->get_content_item(0) == pText );
        CHECK( pDoc->has_attachments() == false );
        CHECK( pDoc->get_version() == "" );
        delete pDoc;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_EmptyInstrument)
    {
        ImoInstrument* pInstr = new ImoInstrument();
        CHECK( pInstr->get_musicdata() == NULL );
        CHECK( pInstr->has_attachments() == false );
        delete pInstr;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_InstrumentWithContent)
    {
        ImoInstrument* pInstr = new ImoInstrument();
        ImoMusicData* pMD = new ImoMusicData();
        pInstr->append_child(pMD);
        CHECK( pInstr->get_musicdata() == pMD );
        ImoClef* pClef = new ImoClef(ImoClef::k_G3);
        pMD->append_child(pClef);
        delete pInstr;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_EmptyScore)
    {
        ImoScore* pScore = new ImoScore();
        CHECK( pScore->has_options() == false );
        ImoInstruments* pColInstr = pScore->get_instruments();
        CHECK( pColInstr != NULL );
        CHECK( pColInstr->get_num_children() == 0 );
        CHECK( pScore->get_num_instruments() == 0 );
        delete pScore;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ScoreWithOption)
    {
        ImoScore* pScore = new ImoScore();
        ImoOptionInfo* pOpt = new ImoOptionInfo("Staff.Green");
        pOpt->set_bool_value(true);
        CHECK( pOpt->get_bool_value() == true );
        pScore->add_option(pOpt);
        CHECK( pScore->has_options() == true );
        CHECK( pScore->get_option("Staff.Green") == pOpt );
        CHECK( pScore->get_num_instruments() == 0 );
        delete pScore;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ScoreWithInstrument)
    {
        ImoScore* pScore = new ImoScore();
        ImoOptionInfo* pOpt = new ImoOptionInfo("Staff.Green");
        pOpt->set_bool_value(true);
        pScore->add_option(pOpt);

        ImoInstrument* pInstr = new ImoInstrument();
        ImoMusicData* pMD = new ImoMusicData();
        pInstr->append_child(pMD);
        ImoClef* pClef = new ImoClef(ImoClef::k_G3);
        ImoTextString* pText = new ImoTextString("Hello world");
        pClef->attach(pText);
        pMD->append_child(pClef);
        pScore->add_instrument(pInstr);

        CHECK( pScore->has_options() == true );
        CHECK( pScore->get_option("Staff.Green") == pOpt );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_instrument(0) == pInstr );

        delete pScore;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_GroupTwoInstruments)
    {
        ImoInstrGroup* pGroup = new ImoInstrGroup();
        CHECK( pGroup->get_num_instruments() == 0 );
        ImoInstrument* pInstr1 = new ImoInstrument();
        CHECK( pInstr1->is_in_group() == false );
        CHECK( pInstr1->get_group() == NULL );

        pGroup->add_instrument(pInstr1);
        CHECK( pGroup->get_num_instruments() == 1 );
        CHECK( pGroup->get_instrument(0) == pInstr1 );
        CHECK( pInstr1->is_in_group() == true );
        CHECK( pInstr1->get_group() == pGroup );

        ImoInstrument* pInstr2 = new ImoInstrument();
        pGroup->add_instrument(pInstr2);
        CHECK( pGroup->get_num_instruments() == 2 );
        CHECK( pGroup->get_instrument(0) == pInstr1 );
        CHECK( pGroup->get_instrument(1) == pInstr2 );
        CHECK( pInstr2->is_in_group() == true );
        CHECK( pInstr2->get_group() == pGroup );

        delete pGroup;
        delete pInstr1;
        delete pInstr2;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ScoreWithInstrGroup)
    {
        ImoScore* pScore = new ImoScore();

        ImoInstrGroup* pGroup = new ImoInstrGroup();
        ImoInstrument* pInstr1 = new ImoInstrument();
        pGroup->add_instrument(pInstr1);
        ImoInstrument* pInstr2 = new ImoInstrument();
        pGroup->add_instrument(pInstr2);

        pScore->add_instruments_group(pGroup);
        CHECK( pScore->get_num_instruments() == 2 );
        CHECK( pScore->get_instrument(0) == pInstr1 );
        CHECK( pScore->get_instrument(1) == pInstr2 );

        ImoInstrument* pInstr3 = new ImoInstrument();
        pScore->add_instrument(pInstr3);

        CHECK( pScore->get_num_instruments() == 3 );
        CHECK( pScore->get_instrument(2) == pInstr3 );

        delete pScore;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_DocumentWithScore)
    {
        ImoDocument* pDoc = new ImoDocument();
        ImoContent* pContent = new ImoContent();
        pDoc->append_child(pContent);

        ImoScore* pScore = new ImoScore();
        ImoOptionInfo* pOpt = new ImoOptionInfo("Staff.Green");
        pOpt->set_bool_value(true);
        pScore->add_option(pOpt);
        ImoInstrument* pInstr = new ImoInstrument();
        ImoMusicData* pMD = new ImoMusicData();
        pInstr->append_child(pMD);
        ImoClef* pClef = new ImoClef(ImoClef::k_G3);
        ImoTextString* pText = new ImoTextString("Hello world");
        pClef->attach(pText);
        pMD->append_child(pClef);
        pScore->add_instrument(pInstr);

        pContent->append_child(pScore);
        CHECK( pDoc->get_num_content_items() == 1 );
        CHECK( pDoc->get_content_item(0) == pScore );
        CHECK( pDoc->has_attachments() == false );
        CHECK( pDoc->get_version() == "" );

        delete pDoc;
    }

}


#endif  // _LM_DEBUG_


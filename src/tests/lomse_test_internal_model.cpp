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
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_im_figured_bass.h"
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
        m_scores_path = LOMSE_TEST_SCORES_PATH;
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
        ImoScoreText* pText = new ImoScoreText("Hello world");
        pClef->attach(pText);
        CHECK( pClef->has_attachments() == true );
        ImoObj* pImo = pClef->get_attachment(0);
        CHECK( pImo->is_score_text() );
        ImoScoreText* pTS = dynamic_cast<ImoScoreText*>( pImo );
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
        ImoScoreText* pText = new ImoScoreText("Hello world");
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
        ImoScoreText* pText = new ImoScoreText("Hello world");
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
        ImoScoreText* pText = new ImoScoreText("Hello world");
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

    // ImoPageInfo ---------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_PageInfoDefaults)
    {
        ImoPageInfo info;
        CHECK( info.is_page_info() == true );
        CHECK( info.get_top_margin() == 2000.0f );
        CHECK( info.get_bottom_margin() == 2000.0f );
        CHECK( info.get_left_margin() == 2000.0f );
        CHECK( info.get_right_margin() == 1500.0f );
        CHECK( info.get_binding_margin() == 0.0f );
        CHECK( info.is_portrait() == true );
    }

    // ImoTextStyleInfo ---------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_Score_GetDefaultStyle)
    {
        ImoScore score;
        ImoTextStyleInfo* pStyle = score.get_default_style_info();
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Default style" );
        CHECK( pStyle->get_color() == rgba16(0,0,0,255) );
        CHECK( pStyle->get_font_name() == "Times New Roman" );
        CHECK( pStyle->get_font_style() == ImoFontInfo::k_normal );
        CHECK( pStyle->get_font_weight() == ImoFontInfo::k_normal );
        CHECK( pStyle->get_font_size() == 10 );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_Score_GetStyle)
    {
        ImoScore score;
	    ImoTextStyleInfo* pStyle = new ImoTextStyleInfo();
	    pStyle->set_name("Test style");
        pStyle->set_color( rgba16(15,16,27,132) );
        pStyle->set_font_name("Callamet");
        pStyle->set_font_size(12);
        pStyle->set_font_style(ImoFontInfo::k_normal);
        pStyle->set_font_weight(ImoFontInfo::k_bold);
        score.add_style_info(pStyle);

        ImoTextStyleInfo* pStyle2 = score.get_style_info("Test style");
        CHECK( pStyle == pStyle2 );
    }

    // ImoBoxInfo -------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_BoxInfoDefaults)
    {
        ImoBoxInfo info;
        CHECK( info.is_box_info() == true );
        CHECK( info.get_height() == 100.0f );
        CHECK( info.get_width() == 160.0f );
        CHECK( info.get_position() == TPoint(0.0f, 0.0f) );
        CHECK( info.get_bg_color() == rgba16(255,255,255,255) );
        CHECK( info.get_border_color() == rgba16(0,0,0,255) );
        CHECK( info.get_border_width() == 1.0f );
        CHECK( info.get_border_style() == k_line_solid );
    }

    // ImoCursorInfo -------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_CursorInfoDefaults)
    {
        ImoCursorInfo info;
        CHECK( info.is_cursor_info() == true );
        CHECK( info.get_instrument() == 0 );
        CHECK( info.get_staff() == 0 );
        CHECK( info.get_time() == 0.0f );
        CHECK( info.get_id() == -1L );
    }

    // ImoFiguredBassInfo ---------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_FigBasInfoFromString_63)
    {
        ImoFiguredBassInfo info("6 3");
        CHECK( info.get_quality(3) == k_interval_as_implied );
        CHECK( info.get_source(3) == "3" );
        CHECK( info.get_prefix(3) == "" );
        CHECK( info.get_suffix(3) == "" );
        CHECK( info.get_over(3) == "" );
        CHECK( info.get_figured_bass_string() == "6 3" );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_FigBasInfoFromString_5)
    {
        ImoFiguredBassInfo info("5");
        //CHECK( info.get_quality(3) == k_interval_as_implied );
        //CHECK( info.get_source(3) == "3" );
        //CHECK( info.get_prefix(3) == "" );
        //CHECK( info.get_suffix(3) == "" );
        //CHECK( info.get_over(3) == "" );
        CHECK( info.get_figured_bass_string() == "5" );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_FigBasInfoFromString_6)
    {
        ImoFiguredBassInfo info("6");
        //CHECK( info.get_quality(3) == k_interval_as_implied );
        //CHECK( info.get_source(3) == "3" );
        //CHECK( info.get_prefix(3) == "" );
        //CHECK( info.get_suffix(3) == "" );
        //CHECK( info.get_over(3) == "" );
        CHECK( info.get_figured_bass_string() == "6" );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_FigBasInfoFromString_64)
    {
        ImoFiguredBassInfo info("6 4");
        //CHECK( info.get_quality(3) == k_interval_as_implied );
        //CHECK( info.get_source(3) == "3" );
        //CHECK( info.get_prefix(3) == "" );
        //CHECK( info.get_suffix(3) == "" );
        //CHECK( info.get_over(3) == "" );
        CHECK( info.get_figured_bass_string() == "6 4" );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_FigBasInfoFromString_7)
    {
        ImoFiguredBassInfo info("7");
        //CHECK( info.get_quality(3) == k_interval_as_implied );
        //CHECK( info.get_source(3) == "3" );
        //CHECK( info.get_prefix(3) == "" );
        //CHECK( info.get_suffix(3) == "" );
        //CHECK( info.get_over(3) == "" );
        CHECK( info.get_figured_bass_string() == "7" );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_FigBasInfoFromString_7m)
    {
        ImoFiguredBassInfo info("7/");
        //CHECK( info.get_quality(3) == k_interval_as_implied );
        //CHECK( info.get_source(3) == "3" );
        //CHECK( info.get_prefix(3) == "" );
        //CHECK( info.get_suffix(3) == "" );
        //CHECK( info.get_over(3) == "" );
        CHECK( info.get_figured_bass_string() == "7/" );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_FigBasInfoFromString_4)
    {
        ImoFiguredBassInfo info("4");   //5 4
        CHECK( info.is_sounding(3) == false );
        CHECK( info.is_sounding(5) == true );
        CHECK( info.get_figured_bass_string() == "4" );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_FigBasInfoFromString_9)
    {
        ImoFiguredBassInfo info("9");
        //CHECK( info.get_quality(3) == k_interval_as_implied );
        //CHECK( info.get_source(3) == "3" );
        //CHECK( info.get_prefix(3) == "" );
        //CHECK( info.get_suffix(3) == "" );
        //CHECK( info.get_over(3) == "" );
        CHECK( info.get_figured_bass_string() == "9" );
    }

}



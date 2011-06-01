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
#include "lomse_internal_model.h"
#include "lomse_im_figured_bass.h"
#include "lomse_basic_objects.h"
#include "lomse_im_note.h"
#include "lomse_time.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
SUITE(InternalModelTest)
{

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_EmptyDocument)
    {
        ImoDocument* pDoc = new ImoDocument("3.7");
        CHECK( pDoc->get_content() == NULL );
        CHECK( pDoc->get_num_content_items() == 0 );
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
        CHECK( pDoc->get_version() == "" );
        delete pDoc;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_EmptyInstrument_OneStaff)
    {
        ImoInstrument instr;
        CHECK( instr.get_musicdata() == NULL );
        CHECK( instr.get_num_staves() == 1 );
        CHECK( instr.is_in_group() == false );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_EmptyInstrument_AddStaff)
    {
        ImoInstrument instr;
        instr.add_staff();
        CHECK( instr.get_musicdata() == NULL );
        CHECK( instr.get_num_staves() == 2 );
        CHECK( instr.is_in_group() == false );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_Instrument_SetNumStaves)
    {
        ImoInstrument instr;
        instr.add_staff();
        instr.add_staff();
        CHECK( instr.get_num_staves() == 3 );
        ImoStaffInfo* pStaff = instr.get_staff(0);
        CHECK( pStaff != NULL );
        CHECK( pStaff->get_num_lines() == 5 );
        CHECK( pStaff->get_staff_margin() == 1000.0f );
        CHECK( pStaff->get_line_spacing() == 180.0f );
        CHECK( pStaff->get_line_thickness() == 15.0f );
        CHECK( pStaff->get_height() == 735.0f );
        pStaff = instr.get_staff(2);
        CHECK( pStaff != NULL );
        CHECK( pStaff->get_num_lines() == 5 );
        CHECK( pStaff->get_staff_margin() == 1000.0f );
        CHECK( pStaff->get_line_spacing() == 180.0f );
        CHECK( pStaff->get_line_thickness() == 15.0f );
        CHECK( pStaff->get_height() == 735.0f );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_InstrumentWithContent)
    {
        ImoInstrument instr;
        ImoMusicData* pMD = new ImoMusicData();
        instr.append_child(pMD);
        CHECK( instr.get_musicdata() == pMD );
        //ImoClef* pClef = new ImoClef(ImoClef::k_G2);
        //pMD->append_child(pClef);
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_Instrument_ReplaceStaffInfo)
    {
        ImoInstrument instr;
        instr.add_staff();
        instr.add_staff();
        ImoStaffInfo* pInfo = new ImoStaffInfo();
        pInfo->set_staff_number(2);
        pInfo->set_line_spacing(400.0f);
        instr.replace_staff_info(pInfo);

        CHECK( instr.get_num_staves() == 3 );
        ImoStaffInfo* pStaff = instr.get_staff(2);
        CHECK( pStaff != NULL );
        CHECK( pStaff->get_num_lines() == 5 );
        CHECK( pStaff->get_staff_margin() == 1000.0f );
        CHECK( pStaff->get_line_spacing() == 400.0f );
        CHECK( pStaff->get_line_thickness() == 15.0f );
        CHECK( pStaff->get_height() == 1615.0f );
        pStaff = instr.get_staff(1);
        CHECK( pStaff != NULL );
        CHECK( pStaff->get_num_lines() == 5 );
        CHECK( pStaff->get_staff_margin() == 1000.0f );
        CHECK( pStaff->get_line_spacing() == 180.0f );
        CHECK( pStaff->get_line_thickness() == 15.0f );
        CHECK( pStaff->get_height() == 735.0f );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_EmptyScore)
    {
        ImoScore* pScore = new ImoScore();
        CHECK( pScore->has_options() == true );
        CHECK( pScore->get_options()->get_num_items() == 9 );
        CHECK( pScore->get_option("Render.SpacingFactor")->get_float_value() == 0.547f );
        CHECK( pScore->get_option("Score.FillPageWithEmptyStaves")->get_bool_value() == false );
        CHECK( pScore->get_option("StaffLines.StopAtFinalBarline")->get_bool_value() == true );
        CHECK( pScore->get_option("Score.JustifyFinalBarline")->get_bool_value() == false );
        CHECK( pScore->get_option("StaffLines.Hide")->get_bool_value() == false );
        CHECK( pScore->get_option("Staff.DrawLeftBarline")->get_bool_value() == true );
        CHECK( pScore->get_option("Staff.UpperLegerLines.Displacement")->get_long_value() == 0L  );
        CHECK( pScore->get_option("Render.SpacingMethod")->get_long_value() == long(k_spacing_proportional) );
        CHECK( pScore->get_option("Render.SpacingValue")->get_long_value() == 15L );
        ImoInstruments* pColInstr = pScore->get_instruments();
        CHECK( pColInstr != NULL );
        CHECK( pColInstr->get_num_children() == 0 );
        CHECK( pScore->get_num_instruments() == 0 );
        delete pScore;
    }

    // score options --------------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ChangeFloatOption)
    {
        ImoScore score;
        CHECK( score.has_options() == true );
        CHECK( score.get_options()->get_num_items() == 9 );
        CHECK( score.get_option("Render.SpacingFactor")->get_float_value() == 0.547f );

        score.set_float_option("Render.SpacingFactor", 0.600f);

        CHECK( score.get_options()->get_num_items() == 9 );
        CHECK( score.get_option("Render.SpacingFactor")->get_float_value() == 0.600f );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ChangeBoolOption)
    {
        ImoScore score;
        CHECK( score.has_options() == true );
        CHECK( score.get_options()->get_num_items() == 9 );
        CHECK( score.get_option("StaffLines.Hide")->get_bool_value() == false );

        score.set_bool_option("StaffLines.Hide", true);

        CHECK( score.get_options()->get_num_items() == 9 );
        CHECK( score.get_option("StaffLines.Hide")->get_bool_value() == true );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ScoreWithBoolOption)
    {
        ImoScore* pScore = new ImoScore();
        ImoOptionInfo opt("Staff.Green");
        opt.set_bool_value(true);
        CHECK( opt.get_bool_value() == true );
        pScore->set_option(&opt);
        CHECK( pScore->has_options() == true );
        ImoOptionInfo* pOpt2 = pScore->get_option("Staff.Green");
        CHECK( pOpt2 != NULL );
        CHECK( pOpt2->get_bool_value() == true );
        CHECK( pScore->get_num_instruments() == 0 );
        delete pScore;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ScoreWithLongOption)
    {
        ImoScore* pScore = new ImoScore();
        ImoOptionInfo opt("Staff.Dots");
        opt.set_long_value(27L);
        CHECK( opt.get_long_value() == 27L );
        pScore->set_option(&opt);
        CHECK( pScore->has_options() == true );
        ImoOptionInfo* pOpt2 = pScore->get_option("Staff.Dots");
        CHECK( pOpt2 != NULL );
        CHECK( pOpt2->get_long_value() == 27L );
        CHECK( pScore->get_num_instruments() == 0 );
        delete pScore;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ScoreWithFloatOption)
    {
        ImoScore* pScore = new ImoScore();
        ImoOptionInfo opt("Staff.Pi");
        opt.set_float_value(3.1416f);
        CHECK( opt.get_float_value() == 3.1416f );
        pScore->set_option(&opt);
        CHECK( pScore->has_options() == true );
        ImoOptionInfo* pOpt2 = pScore->get_option("Staff.Pi");
        CHECK( pOpt2 != NULL );
        CHECK( pOpt2->get_float_value() == 3.1416f );
        CHECK( pScore->get_num_instruments() == 0 );
        delete pScore;
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ScoreAddBoolOption)
    {
        ImoScore score;
        score.set_bool_option("Staff.Green", true);
        CHECK( score.has_options() == true );
        CHECK( score.get_options()->get_num_items() == 10 );
        CHECK( score.get_option("Staff.Green")->get_bool_value() == true );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ScoreAddLongOption)
    {
        ImoScore score;
        score.set_long_option("Staff.LongWidth", 700L);
        CHECK( score.has_options() == true );
        CHECK( score.get_options()->get_num_items() == 10 );
        CHECK( score.get_option("Staff.LongWidth")->get_long_value() == 700L );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ScoreAddFloatOption)
    {
        ImoScore score;
        score.set_float_option("Staff.Green", 0.66f);
        CHECK( score.has_options() == true );
        CHECK( score.get_options()->get_num_items() == 10 );
        CHECK( score.get_option("Staff.Green")->get_float_value() == 0.66f );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ScoreWithInstrument)
    {
        ImoScore* pScore = new ImoScore();
        ImoOptionInfo opt("Staff.Green");
        opt.set_bool_value(true);
        pScore->set_option(&opt);

        ImoInstrument* pInstr = new ImoInstrument();
        ImoMusicData* pMD = new ImoMusicData();
        pInstr->append_child(pMD);
        ImoClef* pClef = new ImoClef(ImoClef::k_G2);
        ImoScoreText* pText = new ImoScoreText("Hello world");
        pClef->add_attachment(pText);
        pMD->append_child(pClef);
        pScore->add_instrument(pInstr);

        CHECK( pScore->has_options() == true );
        ImoOptionInfo* pOpt2 = pScore->get_option("Staff.Green");
        CHECK( pOpt2 != NULL );
        CHECK( pOpt2->get_bool_value() == true );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_instrument(0) == pInstr );

        delete pScore;
    }


    // instruments group ----------------------------------------------------------------

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
        ImoOptionInfo opt("Staff.Green");
        opt.set_bool_value(true);
        pScore->set_option(&opt);
        ImoInstrument* pInstr = new ImoInstrument();
        ImoMusicData* pMD = new ImoMusicData();
        pInstr->append_child(pMD);
        ImoClef* pClef = new ImoClef(ImoClef::k_G2);
        ImoScoreText* pText = new ImoScoreText("Hello world");
        pClef->add_attachment(pText);
        pMD->append_child(pClef);
        pScore->add_instrument(pInstr);

        pContent->append_child(pScore);
        CHECK( pDoc->get_num_content_items() == 1 );
        CHECK( pDoc->get_content_item(0) == pScore );
        CHECK( pDoc->get_version() == "" );

        delete pDoc;
    }

    // ImoPageInfo ----------------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_PageInfoDefaults)
    {
        ImoPageInfo info;
        CHECK( info.is_page_info() == true );
        CHECK( info.get_top_margin() == 2000.0f );
        CHECK( info.get_bottom_margin() == 2000.0f );
        CHECK( info.get_left_margin() == 1500.0f );
        CHECK( info.get_right_margin() == 1500.0f );
        CHECK( info.get_binding_margin() == 0.0f );
        CHECK( info.is_portrait() == true );
    }

    // ImoTextStyleInfo -----------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_Score_GetDefaultStyle)
    {
        ImoScore score;
        ImoTextStyleInfo* pStyle = score.get_default_style_info();
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Default style" );
        CHECK( pStyle->get_color() == Color(0,0,0,255) );
        CHECK( pStyle->get_font_name() == "Liberation serif" );
        CHECK( pStyle->get_font_style() == ImoFontInfo::k_normal );
        CHECK( pStyle->get_font_weight() == ImoFontInfo::k_normal );
        CHECK( pStyle->get_font_size() == 10 );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_Score_GetStyle)
    {
        ImoScore score;
	    ImoTextStyleInfo* pStyle = new ImoTextStyleInfo("Test style", "Callamet",
            12.0f, ImoFontInfo::k_normal, ImoFontInfo::k_bold, Color(15,16,27,132) );
        score.add_style_info(pStyle);

        ImoTextStyleInfo* pStyle2 = score.get_style_info("Test style");
        CHECK( pStyle == pStyle2 );
    }

    // ImoTextBlockInfo -----------------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_BoxInfoDefaults)
    {
        ImoTextBlockInfo info;
        CHECK( info.is_box_info() == true );
        CHECK( info.get_height() == 100.0f );
        CHECK( info.get_width() == 160.0f );
        CHECK( info.get_position() == TPoint(0.0f, 0.0f) );
        CHECK( info.get_bg_color() == Color(255,255,255,255) );
        CHECK( info.get_border_color() == Color(0,0,0,255) );
        CHECK( info.get_border_width() == 1.0f );
        CHECK( info.get_border_style() == k_line_solid );
    }

    // ImoCursorInfo --------------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_CursorInfoDefaults)
    {
        ImoCursorInfo info;
        CHECK( info.is_cursor_info() == true );
        CHECK( info.get_instrument() == 0 );
        CHECK( info.get_staff() == 0 );
        CHECK( info.get_time() == 0.0f );
        CHECK( info.get_id() == -1L );
    }

    // ImoFiguredBassInfo ---------------------------------------------------------------

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

    // RelDataObjs ----------------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ClefNoReldataobjs)
    {
        ImoClef clef(ImoClef::k_G2);
        CHECK( clef.has_reldataobjs() == false );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_AddReldataobj)
    {
        ImoClef clef(ImoClef::k_G2);
        ImoMidiInfo* pMidi = new ImoMidiInfo();
        clef.add_reldataobj(pMidi);

        CHECK( clef.has_reldataobjs() == true );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_GetNumReldataobjs)
    {
        ImoClef clef(ImoClef::k_G2);
        ImoMidiInfo* pMidi = new ImoMidiInfo();
        clef.add_reldataobj(pMidi);
        ImoFontInfo* pFont = new ImoFontInfo();
        clef.add_reldataobj(pFont);

        CHECK( clef.get_num_reldataobjs() == 2 );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_GetReldataobj)
    {
        ImoClef clef(ImoClef::k_G2);
        ImoMidiInfo* pMidi = new ImoMidiInfo();
        clef.add_reldataobj(pMidi);
        ImoFontInfo* pFont = new ImoFontInfo();
        clef.add_reldataobj(pFont);

        CHECK( clef.get_reldataobj(0) == pMidi );
        CHECK( clef.get_reldataobj(1) == pFont );
    }

    TEST_FIXTURE(InternalModelTestFixture, RemoveReldataobj)
    {
        ImoClef clef(ImoClef::k_G2);
        ImoMidiInfo* pMidi = new ImoMidiInfo();
        clef.add_reldataobj(pMidi);
        ImoFontInfo* pFont = new ImoFontInfo();
        clef.add_reldataobj(pFont);

        clef.remove_reldataobj(pMidi);

        CHECK( clef.get_num_reldataobjs() == 1 );
        CHECK( clef.get_reldataobj(0) == pFont );
    }

    TEST_FIXTURE(InternalModelTestFixture, RemoveAllReldataobjs)
    {
        ImoClef clef(ImoClef::k_G2);
        ImoMidiInfo* pMidi = new ImoMidiInfo();
        clef.add_reldataobj(pMidi);
        ImoFontInfo* pFont = new ImoFontInfo();
        clef.add_reldataobj(pFont);

        clef.remove_reldataobj(pMidi);
        clef.remove_reldataobj(pFont);

        CHECK( clef.get_num_reldataobjs() == 0 );
        CHECK( clef.get_reldataobjs() == NULL );
    }

    TEST_FIXTURE(InternalModelTestFixture, FindReldataobj)
    {
        ImoClef clef(ImoClef::k_G2);
        ImoMidiInfo* pMidi = new ImoMidiInfo();
        clef.add_reldataobj(pMidi);
        ImoFontInfo* pFont = new ImoFontInfo();
        clef.add_reldataobj(pFont);

        CHECK( clef.find_reldataobj(ImoObj::k_font_info) == pFont );
        CHECK( clef.find_reldataobj(ImoObj::k_midi_info) == pMidi );
    }


    // attachments ----------------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_ClefNoAttachments)
    {
        ImoClef clef(ImoClef::k_G2);
        CHECK( clef.has_attachments() == false );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_AddAttachment)
    {
        ImoClef clef(ImoClef::k_G2);
        ImoScoreText* pText = new ImoScoreText("Hello world");
        clef.add_attachment(pText);

        CHECK( clef.has_attachments() == true );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_GetNumAttachments)
    {
        ImoClef clef(ImoClef::k_G2);
        ImoScoreText* pText = new ImoScoreText("Hello world");
        clef.add_attachment(pText);
        DtoFermata fmt;
        ImoFermata* pFmt = new ImoFermata(fmt);
        clef.add_attachment(pFmt);

        CHECK( clef.get_num_attachments() == 2 );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_GetAttachment)
    {
        ImoClef clef(ImoClef::k_G2);

        DtoFermata fmt;
        ImoFermata* pFmt = new ImoFermata(fmt);
        clef.add_attachment(pFmt);

        ImoScoreText* pText = new ImoScoreText("Hello world");
        clef.add_attachment(pText);

        CHECK( clef.get_attachment(0) == pFmt );
        CHECK( clef.get_attachment(1) == pText );
    }

    TEST_FIXTURE(InternalModelTestFixture, RemoveAttachment)
    {
        ImoClef clef(ImoClef::k_G2);
        ImoScoreText* pText = new ImoScoreText("Hello world");
        clef.add_attachment(pText);
        DtoFermata fmt;
        ImoFermata* pFmt = new ImoFermata(fmt);
        clef.add_attachment(pFmt);

        clef.remove_attachment(pText);

        CHECK( clef.get_num_attachments() == 1 );
        CHECK( clef.get_attachment(0) == pFmt );
    }

    TEST_FIXTURE(InternalModelTestFixture, RemoveAllAttachments)
    {
        ImoClef clef(ImoClef::k_G2);
        ImoScoreText* pText = new ImoScoreText("Hello world");
        clef.add_attachment(pText);
        DtoFermata fmt;
        ImoFermata* pFmt = new ImoFermata(fmt);
        clef.add_attachment(pFmt);

        clef.remove_attachment(pText);
        clef.remove_attachment(pFmt);

        CHECK( clef.get_num_attachments() == 0 );
        CHECK( clef.get_attachments() != NULL );
    }

    TEST_FIXTURE(InternalModelTestFixture, IncludeInRelation)
    {
        DtoNote dtoNote;
        dtoNote.set_step(1);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(k_eighth);
        dtoNote.set_dots(0);
        ImoNote note(dtoNote);

        ImoTieDto dto;
        ImoTieData* pData = new ImoTieData(&dto);
        ImoTie* pTie = new ImoTie();
        note.include_in_relation(pTie, pData);

        CHECK( note.get_num_attachments() == 1 );
        CHECK( note.get_attachment(0) == pTie );
        CHECK( note.get_num_reldataobjs() == 1 );
        CHECK( note.get_reldataobj(0) == pData );
        CHECK( pTie->get_data_for(&note) == pData );
    }

    TEST_FIXTURE(InternalModelTestFixture, RemoveFromRelation)
    {
        DtoNote dtoNote;
        dtoNote.set_step(1);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(k_eighth);
        dtoNote.set_dots(0);
        ImoNote note(dtoNote);

        ImoTieDto dto;
        ImoTieData* pData = new ImoTieData(&dto);
        ImoTie* pTie = new ImoTie();
        note.include_in_relation(pTie, pData);

        note.remove_from_relation(pTie);

        CHECK( note.get_num_attachments() == 0 );
        CHECK( note.get_attachments() != NULL );
        CHECK( note.get_num_reldataobjs() == 0 );
        CHECK( note.get_reldataobjs() == NULL );
    }

    TEST_FIXTURE(InternalModelTestFixture, RemoveFromAllRelations)
    {
        DtoNote dtoNote;
        dtoNote.set_step(1);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(k_eighth);
        dtoNote.set_dots(0);
        ImoNote note(dtoNote);

        ImoTieDto dto;
        ImoTieData* pData = new ImoTieData(&dto);
        ImoTie* pTie = new ImoTie();
        note.include_in_relation(pTie, pData);

        ImoScoreText* pText = new ImoScoreText("Hello world");
        note.add_attachment(pText);

        CHECK( note.get_num_attachments() == 2 );
        CHECK( note.get_num_reldataobjs() == 1 );

        ImoAttachments* pAttachments = note.get_attachments();
        pAttachments->remove_from_all_relations(&note);

        CHECK( note.get_num_attachments() == 0 );
        CHECK( note.get_num_reldataobjs() == 0 );
        CHECK( note.get_reldataobjs() == NULL );
    }

    TEST_FIXTURE(InternalModelTestFixture, AutoDelete)
    {
        ImoNote note(k_step_D, 4, k_eighth);

        ImoTieDto dto;
        ImoTieData* pData = new ImoTieData(&dto);
        ImoTie* pTie = new ImoTie();
        note.include_in_relation(pTie, pData);

        pTie->remove_all();
        delete pTie;

        CHECK( note.get_num_attachments() == 0 );
        CHECK( note.get_num_reldataobjs() == 0 );
        CHECK( note.get_reldataobjs() == NULL );
    }

    TEST_FIXTURE(InternalModelTestFixture, InternalModel_AttachmentsOrdered)
    {
        //@ Attachments must be rendered in a predefined order, i.e. beams before
        //@ tuplets. For this, they must be stored in renderization order.

        ImoNote note(k_step_A, 4, k_eighth);

        ImoScoreText* pText = new ImoScoreText("Hello world");
        note.add_attachment(pText);

        DtoFermata fermata;
        ImoFermata* pFermata = new ImoFermata(fermata);
        note.add_attachment(pFermata);

        ImoTieDto dtoTie;
        ImoTieData* pTieData = new ImoTieData(&dtoTie);
        ImoTie* pTie = new ImoTie();
        note.include_in_relation(pTie, pTieData);

        ImoBeamDto dtoBeam;
        ImoBeamData* pBeamData = new ImoBeamData(&dtoBeam);
        ImoBeam* pBeam = new ImoBeam();
        note.include_in_relation(pBeam, pBeamData);

        CHECK( note.get_attachment(0) == pTie );
        CHECK( note.get_attachment(1) == pBeam );
        CHECK( note.get_attachment(2) == pFermata );
        CHECK( note.get_attachment(3) == pText );
//        cout << "order = " << note.get_attachment(0)->get_obj_type() << ", "
//             << note.get_attachment(1)->get_obj_type() << ", "
//             << note.get_attachment(2)->get_obj_type() << ", "
//             << note.get_attachment(3)->get_obj_type() << endl;
    }


    // ImoNote --------------------------------------------------------------------------

    TEST_FIXTURE(InternalModelTestFixture, Note_ConstructorDefaults)
    {
        ImoNote note(k_step_A, 4, k_eighth);

        CHECK( note.get_step() == k_step_A );
        CHECK( note.get_octave() == 4 );
        CHECK( note.get_note_type() == k_eighth );
        CHECK( note.get_dots() == 0 );
        CHECK( note.get_voice() == 0 );
        CHECK( note.get_staff() == 0 );
        CHECK( note.get_duration() == 32.0f );
        CHECK( note.get_accidentals() == k_no_accidentals );
        CHECK( note.get_stem_direction() == k_stem_default );
    }

    TEST_FIXTURE(InternalModelTestFixture, Note_ConstructorFull)
    {
        ImoNote note(k_step_A, 4, k_eighth, k_flat, 1, 2, 3, k_stem_up);

        CHECK( note.get_step() == k_step_A );
        CHECK( note.get_octave() == 4 );
        CHECK( note.get_note_type() == k_eighth );
        CHECK( note.get_dots() == 1 );
        CHECK( note.get_voice() == 3 );
        CHECK( note.get_staff() == 2 );
        CHECK( note.get_duration() == 48.0f );
        CHECK( note.get_accidentals() == k_flat );
        CHECK( note.get_stem_direction() == k_stem_up );
    }

    TEST_FIXTURE(InternalModelTestFixture, Note_TupletModifiesDuration)
    {
        //@ Note duration is modified by tuplet

        ImoNote note(k_step_A, 4, k_eighth);
        CHECK( is_equal_time(note.get_duration(), 32.0f) );

        ImoTupletDto dto;
        dto.set_tuplet_type(ImoTupletDto::k_start);
        dto.set_note_rest(&note);
        dto.set_actual_number(3);
        dto.set_normal_number(2);
        ImoTupletData* pTupletData = new ImoTupletData(&dto);
        ImoTuplet* pTuplet = new ImoTuplet(&dto);
        note.include_in_relation(pTuplet, pTupletData);

        //cout << "note duration = " << note.get_duration() << endl;
        CHECK( note.is_in_tuplet() == true );
        CHECK( is_equal_time(note.get_duration(), 21.3333f) );
    }



}



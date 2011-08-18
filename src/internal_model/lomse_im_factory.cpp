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

#include "lomse_im_factory.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
//#include "lomse_parser.h"
//#include "lomse_analyser.h"
#include "lomse_document.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
ImoObj* ImFactory::inject(Document* pDoc, const std::string& ldpSource)
{
    return pDoc->create_object(ldpSource);
}

//---------------------------------------------------------------------------------------
ImoObj* ImFactory::inject(int type, Document* pDoc)
{
    ImoObj* pObj = NULL;
    switch(type)
    {
        case k_imo_attachments:         pObj = new ImoAttachments();        break;
        case k_imo_barline:             pObj = new ImoBarline();            break;
        case k_imo_beam:                pObj = new ImoBeam();               break;
        case k_imo_beam_dto:            pObj = new ImoBeamDto();            break;
        case k_imo_bezier_info:         pObj = new ImoBezierInfo();         break;
//        case k_imo_border_dto:      pObj = new ImoBorderDto();              break;
        case k_imo_textblock_info:      pObj = new ImoTextBlockInfo();      break;
        case k_imo_button:              pObj = new ImoButton();             break;
        case k_imo_chord:               pObj = new ImoChord();              break;
        case k_imo_clef:                pObj = new ImoClef();               break;
        case k_imo_color_dto:           pObj = new ImoColorDto();           break;
        case k_imo_content:             pObj = new ImoContent(pDoc);        break;
        case k_imo_control:             pObj = new ImoControl();            break;
        case k_imo_cursor_info:         pObj = new ImoCursorInfo();         break;
        case k_imo_document:            pObj = new ImoDocument(pDoc);       break;
        case k_imo_dynamic:             pObj = new ImoDynamic();            break;
        case k_imo_fermata:             pObj = new ImoFermata();            break;
//        case k_imo_figured_bass:    pObj = new ImoFiguredBass();              break;
//        case k_imo_figured_bass_info:   pObj = new ImoFBInfo();          break;
        case k_imo_font_style_dto:      pObj = new ImoFontStyleDto();       break;
        case k_imo_go_back_fwd:         pObj = new ImoGoBackFwd();          break;
        case k_imo_heading:             pObj = new ImoHeading();            break;
        case k_imo_inline_wrapper:      pObj = new ImoInlineWrapper();      break;
        case k_imo_instr_group:         pObj = new ImoInstrGroup();         break;
        case k_imo_instrument:          pObj = new ImoInstrument(pDoc);     break;
        case k_imo_instruments:         pObj = new ImoInstruments();        break;
        case k_imo_instrument_groups:   pObj = new ImoInstrGroups();        break;
        case k_imo_key_signature:       pObj = new ImoKeySignature();       break;
        case k_imo_line:                pObj = new ImoLine();               break;
        case k_imo_line_style:          pObj = new ImoLineStyle();          break;
        case k_imo_link:                pObj = new ImoLink();               break;
        case k_imo_metronome_mark:      pObj = new ImoMetronomeMark();      break;
        case k_imo_midi_info:           pObj = new ImoMidiInfo();           break;
        case k_imo_music_data:          pObj = new ImoMusicData();          break;
        case k_imo_note:                pObj = new ImoNote();               break;
        case k_imo_option:              pObj = new ImoOptionInfo();         break;
        case k_imo_options:             pObj = new ImoOptions();            break;
        case k_imo_page_info:           pObj = new ImoPageInfo();           break;
        case k_imo_para:                pObj = new ImoParagraph();          break;
        case k_imo_param_info:          pObj = new ImoParamInfo();          break;
//        case k_imo_point_dto:       pObj = new ImoParagraph();              break;
        case k_imo_reldataobjs:         pObj = new ImoReldataobjs();        break;
        case k_imo_rest:                pObj = new ImoRest();               break;
        case k_imo_score:               pObj = new ImoScore(pDoc);          break;
        case k_imo_score_text:          pObj = new ImoScoreText();          break;
        case k_imo_score_title:         pObj = new ImoScoreTitle();         break;
//        case k_imo_size_dto:        pObj = new ImoParagraph();              break;
        case k_imo_slur:                pObj = new ImoSlur();               break;
        case k_imo_slur_dto:            pObj = new ImoSlurDto();            break;
        case k_imo_spacer:              pObj = new ImoSpacer();             break;
        case k_imo_staff_info:          pObj = new ImoStaffInfo();          break;
        case k_imo_style:               pObj = new ImoStyle();              break;
        case k_imo_styles:              pObj = new ImoStyles(pDoc);         break;
        case k_imo_system_info:         pObj = new ImoSystemInfo();         break;
        case k_imo_text_box:            pObj = new ImoTextBox();            break;
        case k_imo_text_info:           pObj = new ImoTextInfo();           break;
        case k_imo_text_item:           pObj = new ImoTextItem();           break;
//        case k_imo_text_style:      pObj = new ImoParagraph();              break;
        case k_imo_tie:                 pObj = new ImoTie();                break;
        case k_imo_tie_dto:             pObj = new ImoTieDto();             break;
        case k_imo_time_signature:      pObj = new ImoTimeSignature();      break;
        case k_imo_tuplet:              pObj = new ImoTuplet();             break;
        case k_imo_tuplet_dto:          pObj = new ImoTupletDto();          break;

        default:
            throw std::runtime_error("[ImFactory::inject] invalid type.");
    }
    if (!pObj->is_dto())
        pObj->set_id( pDoc->new_id() );
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoBeamData* ImFactory::inject_beam_data(Document* pDoc, ImoBeamDto* pDto)
{
    ImoBeamData* pObj = new ImoBeamData(pDto);
    pObj->set_id( pDoc->new_id() );
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoTieData* ImFactory::inject_tie_data(Document* pDoc, ImoTieDto* pDto)
{
    ImoTieData* pObj = new ImoTieData(pDto);
    pObj->set_id( pDoc->new_id() );
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoTupletData* ImFactory::inject_tuplet_data(Document* pDoc, ImoTupletDto* pDto)
{
    ImoTupletData* pObj = new ImoTupletData(pDto);
    pObj->set_id( pDoc->new_id() );
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoSlurData* ImFactory::inject_slur_data(Document* pDoc, ImoSlurDto* pDto)
{
    ImoSlurData* pObj = new ImoSlurData(pDto);
    pObj->set_id( pDoc->new_id() );
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoTuplet* ImFactory::inject_tuplet(Document* pDoc, ImoTupletDto* pDto)
{
    ImoTuplet* pObj = new ImoTuplet(pDto);
    pObj->set_id( pDoc->new_id() );
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoTextBox* ImFactory::inject_text_box(Document* pDoc, ImoTextBlockInfo& dto)
{
    ImoTextBox* pObj = new ImoTextBox(dto);
    pObj->set_id( pDoc->new_id() );
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoNote* ImFactory::inject_note(Document* pDoc, int step, int octave,
                                int noteType, int accidentals, int dots,
                                int staff, int voice, int stem)
{
    ImoNote* pObj = new ImoNote(step, octave, noteType, accidentals, dots,
                                staff, voice, stem);
    pObj->set_id( pDoc->new_id() );
    return pObj;
}


}  //namespace lomse

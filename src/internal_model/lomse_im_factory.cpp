//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#include "lomse_im_factory.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_logger.h"
#include "lomse_document.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
ImoObj* ImFactory::inject(Document* pDoc, const std::string& ldpSource)
{
    return pDoc->create_object_from_ldp(ldpSource);
}

//---------------------------------------------------------------------------------------
ImoObj* ImFactory::inject(int type, Document* pDoc, ImoId id)
{
    ImoObj* pObj = nullptr;

    if (!(type > k_imo_dto && type < k_imo_dto_last))
        id = pDoc->reserve_id(id);

    switch(type)
    {
        case k_imo_anonymous_block:     pObj = LOMSE_NEW ImoAnonymousBlock();     break;
        case k_imo_articulation_symbol: pObj = LOMSE_NEW ImoArticulationSymbol(); break;
        case k_imo_articulation_line:   pObj = LOMSE_NEW ImoArticulationLine();   break;
        case k_imo_attachments:         pObj = LOMSE_NEW ImoAttachments();        break;
        case k_imo_barline:             pObj = LOMSE_NEW ImoBarline();            break;
        case k_imo_beam:                pObj = LOMSE_NEW ImoBeam();               break;
        case k_imo_beam_dto:            pObj = LOMSE_NEW ImoBeamDto();            break;
        case k_imo_bezier_info:         pObj = LOMSE_NEW ImoBezierInfo();         break;
        case k_imo_button:              pObj = LOMSE_NEW ImoButton();             break;
        case k_imo_chord:               pObj = LOMSE_NEW ImoChord();              break;
        case k_imo_clef:                pObj = LOMSE_NEW ImoClef();               break;
        case k_imo_color_dto:           pObj = LOMSE_NEW ImoColorDto();           break;
        case k_imo_content:             pObj = LOMSE_NEW ImoContent();            break;
        case k_imo_cursor_info:         pObj = LOMSE_NEW ImoCursorInfo();         break;
        case k_imo_direction:           pObj = LOMSE_NEW ImoDirection();          break;
        case k_imo_document:            pObj = LOMSE_NEW ImoDocument();           break;
        case k_imo_dynamic:             pObj = LOMSE_NEW ImoDynamic();            break;
        case k_imo_dynamics_mark:       pObj = LOMSE_NEW ImoDynamicsMark();       break;
        case k_imo_fermata:             pObj = LOMSE_NEW ImoFermata();            break;
        case k_imo_font_style_dto:      pObj = LOMSE_NEW ImoFontStyleDto();       break;
        case k_imo_go_back_fwd:         pObj = LOMSE_NEW ImoGoBackFwd();          break;
        case k_imo_heading:             pObj = LOMSE_NEW ImoHeading();            break;
        case k_imo_image:               pObj = LOMSE_NEW ImoImage();              break;
        case k_imo_inline_wrapper:      pObj = LOMSE_NEW ImoInlineWrapper();      break;
        case k_imo_instr_group:         pObj = LOMSE_NEW ImoInstrGroup();         break;
        case k_imo_instrument:          pObj = LOMSE_NEW ImoInstrument();         break;
        case k_imo_instruments:         pObj = LOMSE_NEW ImoInstruments();        break;
        case k_imo_instrument_groups:   pObj = LOMSE_NEW ImoInstrGroups();        break;
        case k_imo_key_signature:       pObj = LOMSE_NEW ImoKeySignature();       break;
        case k_imo_line:                pObj = LOMSE_NEW ImoLine();               break;
        case k_imo_line_style:          pObj = LOMSE_NEW ImoLineStyle();          break;
        case k_imo_list:                pObj = LOMSE_NEW ImoList(pDoc);           break;
        case k_imo_listitem:            pObj = LOMSE_NEW ImoListItem(pDoc);       break;
        case k_imo_link:                pObj = LOMSE_NEW ImoLink();               break;
        case k_imo_lyric:               pObj = LOMSE_NEW ImoLyric();              break;
        case k_imo_lyrics_text_info:    pObj = LOMSE_NEW ImoLyricsTextInfo();     break;
        case k_imo_metronome_mark:      pObj = LOMSE_NEW ImoMetronomeMark();      break;
        case k_imo_midi_info:           pObj = LOMSE_NEW ImoMidiInfo();           break;
        case k_imo_multicolumn:         pObj = LOMSE_NEW ImoMultiColumn(pDoc);    break;
        case k_imo_music_data:          pObj = LOMSE_NEW ImoMusicData();          break;
        case k_imo_note:                pObj = LOMSE_NEW ImoNote();               break;
        case k_imo_octave_shift:        pObj = LOMSE_NEW ImoOctaveShift();        break;
        case k_imo_octave_shift_dto:    pObj = LOMSE_NEW ImoOctaveShiftDto();     break;
        case k_imo_option:              pObj = LOMSE_NEW ImoOptionInfo();         break;
        case k_imo_options:             pObj = LOMSE_NEW ImoOptions();            break;
        case k_imo_ornament:            pObj = LOMSE_NEW ImoOrnament();           break;
        case k_imo_page_info:           pObj = LOMSE_NEW ImoPageInfo();           break;
        case k_imo_para:                pObj = LOMSE_NEW ImoParagraph();          break;
        case k_imo_param_info:          pObj = LOMSE_NEW ImoParamInfo();          break;
        case k_imo_relations:           pObj = LOMSE_NEW ImoRelations();          break;
        case k_imo_rest:                pObj = LOMSE_NEW ImoRest();               break;
        case k_imo_score:               pObj = LOMSE_NEW ImoScore(pDoc);          break;
        case k_imo_score_line:          pObj = LOMSE_NEW ImoScoreLine();          break;
        case k_imo_score_player:        pObj = LOMSE_NEW ImoScorePlayer();        break;
        case k_imo_score_text:          pObj = LOMSE_NEW ImoScoreText();          break;
        case k_imo_score_title:         pObj = LOMSE_NEW ImoScoreTitle();         break;
        case k_imo_slur:                pObj = LOMSE_NEW ImoSlur();               break;
        case k_imo_slur_dto:            pObj = LOMSE_NEW ImoSlurDto();            break;
        case k_imo_sound_change:        pObj = LOMSE_NEW ImoSoundChange();        break;
        case k_imo_sound_info:          pObj = LOMSE_NEW ImoSoundInfo();          break;
        case k_imo_sounds:              pObj = LOMSE_NEW ImoSounds();             break;
        case k_imo_staff_info:          pObj = LOMSE_NEW ImoStaffInfo();          break;
        case k_imo_style:               pObj = LOMSE_NEW ImoStyle();              break;
        case k_imo_styles:              pObj = LOMSE_NEW ImoStyles(pDoc);         break;
        case k_imo_symbol_repetition_mark:  pObj = LOMSE_NEW ImoSymbolRepetitionMark();   break;
        case k_imo_system_break:        pObj = LOMSE_NEW ImoSystemBreak();        break;
        case k_imo_system_info:         pObj = LOMSE_NEW ImoSystemInfo();         break;
        case k_imo_table:               pObj = LOMSE_NEW ImoTable();              break;
        case k_imo_table_cell:          pObj = LOMSE_NEW ImoTableCell(pDoc);      break;
        case k_imo_table_body:          pObj = LOMSE_NEW ImoTableBody();          break;
        case k_imo_table_head:          pObj = LOMSE_NEW ImoTableHead();          break;
        case k_imo_table_row:           pObj = LOMSE_NEW ImoTableRow(pDoc);       break;
        case k_imo_technical:           pObj = LOMSE_NEW ImoTechnical();          break;
        case k_imo_textblock_info:      pObj = LOMSE_NEW ImoTextBlockInfo();      break;
        case k_imo_text_box:            pObj = LOMSE_NEW ImoTextBox();            break;
        case k_imo_text_info:           pObj = LOMSE_NEW ImoTextInfo();           break;
        case k_imo_text_item:           pObj = LOMSE_NEW ImoTextItem();           break;
        case k_imo_text_repetition_mark:   pObj = LOMSE_NEW ImoTextRepetitionMark();   break;
        case k_imo_tie:                 pObj = LOMSE_NEW ImoTie();                break;
        case k_imo_tie_dto:             pObj = LOMSE_NEW ImoTieDto();             break;
        case k_imo_time_modification_dto:  pObj = LOMSE_NEW ImoTimeModificationDto();  break;
        case k_imo_time_signature:      pObj = LOMSE_NEW ImoTimeSignature();      break;
        case k_imo_tuplet:              pObj = LOMSE_NEW ImoTuplet();             break;
        case k_imo_tuplet_dto:          pObj = LOMSE_NEW ImoTupletDto();          break;
        case k_imo_volta_bracket:       pObj = LOMSE_NEW ImoVoltaBracket();       break;
        case k_imo_volta_bracket_dto:   pObj = LOMSE_NEW ImoVoltaBracketDto();    break;
        case k_imo_wedge:               pObj = LOMSE_NEW ImoWedge();              break;
        case k_imo_wedge_dto:           pObj = LOMSE_NEW ImoWedgeDto();           break;
        default:
        {
            LOMSE_LOG_ERROR("[ImFactory::inject] invalid type.");
            throw runtime_error("[ImFactory::inject] invalid type.");
        }
    }

    if (!pObj->is_dto())
    {
        pObj->set_id(id);
        pDoc->assign_id(pObj);
    }
    pObj->set_owner_document(pDoc);
    pObj->initialize_object(pDoc);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoBeamData* ImFactory::inject_beam_data(Document* pDoc, ImoBeamDto* pDto)
{
    ImoBeamData* pObj = LOMSE_NEW ImoBeamData(pDto);
    pDoc->assign_id(pObj);
    pObj->set_owner_document(pDoc);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoTieData* ImFactory::inject_tie_data(Document* pDoc, ImoTieDto* pDto)
{
    ImoTieData* pObj = LOMSE_NEW ImoTieData(pDto);
    pDoc->assign_id(pObj);
    pObj->set_owner_document(pDoc);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoSlurData* ImFactory::inject_slur_data(Document* pDoc, ImoSlurDto* pDto)
{
    ImoSlurData* pObj = LOMSE_NEW ImoSlurData(pDto);
    pDoc->assign_id(pObj);
    pObj->set_owner_document(pDoc);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoTuplet* ImFactory::inject_tuplet(Document* pDoc, ImoTupletDto* pDto)
{
    ImoTuplet* pObj = LOMSE_NEW ImoTuplet(pDto);
    pObj->set_id( pDto->get_id() );
    pDoc->assign_id(pObj);
    pObj->set_owner_document(pDoc);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoTextBox* ImFactory::inject_text_box(Document* pDoc, ImoTextBlockInfo& dto, ImoId id)
{
    ImoTextBox* pObj = LOMSE_NEW ImoTextBox(dto);
    pObj->set_id(id);
    pDoc->assign_id(pObj);
    pObj->set_owner_document(pDoc);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoNote* ImFactory::inject_note(Document* pDoc, int step, int octave,
                                int noteType, EAccidentals accidentals,
                                int dots, int staff, int voice, int stem)
{
    ImoNote* pObj = LOMSE_NEW ImoNote(step, octave, noteType, accidentals, dots,
                                staff, voice, stem);
    pDoc->assign_id(pObj);
    pObj->set_owner_document(pDoc);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoMultiColumn* ImFactory::inject_multicolumn(Document* pDoc)
{
    ImoMultiColumn* pObj = LOMSE_NEW ImoMultiColumn(pDoc);
    pDoc->assign_id(pObj);
    pObj->set_owner_document(pDoc);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoImage* ImFactory::inject_image(Document* pDoc, unsigned char* imgbuf, VSize bmpSize,
                                  EPixelFormat format, USize imgSize)
{
    ImoImage* pObj = LOMSE_NEW ImoImage(imgbuf, bmpSize, format, imgSize);
    pDoc->assign_id(pObj);
    pObj->set_owner_document(pDoc);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoControl* ImFactory::inject_control(Document* pDoc, Control* ctrol)
{
    ImoControl* pObj = LOMSE_NEW ImoControl(ctrol);
    pDoc->assign_id(pObj);
    pObj->set_owner_document(pDoc);
    return pObj;
}


}  //namespace lomse

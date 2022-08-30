//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_im_factory.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_logger.h"
#include "private/lomse_document_p.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
ImoObj* ImFactory::inject(DocModel* pDocModel, const std::string& ldpSource)
{
    return pDocModel->get_owner_document()->create_object_from_ldp(ldpSource);
}

//---------------------------------------------------------------------------------------
ImoObj* ImFactory::inject(int type, DocModel* pDocModel, ImoId id)
{
    ImoObj* pObj = nullptr;

    if (!(type > k_imo_dto && type < k_imo_dto_last))
        id = pDocModel->reserve_id(id);

    switch(type)
    {
        case k_imo_anonymous_block:     pObj = LOMSE_NEW ImoAnonymousBlock();     break;
        case k_imo_arpeggio:            pObj = LOMSE_NEW ImoArpeggio();           break;
        case k_imo_arpeggio_dto:        pObj = LOMSE_NEW ImoArpeggioDto();        break;
        case k_imo_articulation_symbol: pObj = LOMSE_NEW ImoArticulationSymbol(); break;
        case k_imo_articulation_line:   pObj = LOMSE_NEW ImoArticulationLine();   break;
        case k_imo_attachments:         pObj = LOMSE_NEW ImoAttachments();        break;
        case k_imo_barline:             pObj = LOMSE_NEW ImoBarline();            break;
        case k_imo_beam:                pObj = LOMSE_NEW ImoBeam();               break;
        case k_imo_beam_data:           pObj = LOMSE_NEW ImoBeamData();           break;
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
        case k_imo_fingering:           pObj = LOMSE_NEW ImoFingering();          break;
        case k_imo_font_style_dto:      pObj = LOMSE_NEW ImoFontStyleDto();       break;
        case k_imo_fret_string:         pObj = LOMSE_NEW ImoFretString();         break;
        case k_imo_go_back_fwd:         pObj = LOMSE_NEW ImoGoBackFwd();          break;
        case k_imo_grace_relobj:        pObj = LOMSE_NEW ImoGraceRelObj();        break;
        case k_imo_heading:             pObj = LOMSE_NEW ImoHeading();            break;
        case k_imo_image:               pObj = LOMSE_NEW ImoImage();              break;
        case k_imo_inline_wrapper:      pObj = LOMSE_NEW ImoInlineWrapper();      break;
        case k_imo_instr_group:         pObj = LOMSE_NEW ImoInstrGroup();         break;
        case k_imo_instrument:          pObj = LOMSE_NEW ImoInstrument();         break;
        case k_imo_instruments:         pObj = LOMSE_NEW ImoInstruments();        break;
        case k_imo_instrument_groups:   pObj = LOMSE_NEW ImoInstrGroups();        break;
        case k_imo_key_signature:       pObj = LOMSE_NEW ImoKeySignature();       break;
        case k_imo_line:                pObj = LOMSE_NEW ImoLine();               break;
        case k_imo_line_style_dto:      pObj = LOMSE_NEW ImoLineStyleDto();       break;
        case k_imo_list:                pObj = LOMSE_NEW ImoList();               break;
        case k_imo_listitem:            pObj = LOMSE_NEW ImoListItem();           break;
        case k_imo_link:                pObj = LOMSE_NEW ImoLink();               break;
        case k_imo_lyric:               pObj = LOMSE_NEW ImoLyric();              break;
        case k_imo_lyrics_text_info:    pObj = LOMSE_NEW ImoLyricsTextInfo();     break;
        case k_imo_metronome_mark:      pObj = LOMSE_NEW ImoMetronomeMark();      break;
        case k_imo_midi_info:           pObj = LOMSE_NEW ImoMidiInfo();           break;
        case k_imo_multicolumn:         pObj = LOMSE_NEW ImoMultiColumn();        break;
        case k_imo_music_data:          pObj = LOMSE_NEW ImoMusicData();          break;
        case k_imo_note_cue:            pObj = LOMSE_NEW ImoNote(k_imo_note_cue); break;
        case k_imo_note_grace:          pObj = LOMSE_NEW ImoGraceNote();          break;
        case k_imo_note_regular:        pObj = LOMSE_NEW ImoNote(k_imo_note_regular);  break;
        case k_imo_octave_shift:        pObj = LOMSE_NEW ImoOctaveShift();        break;
        case k_imo_octave_shift_dto:    pObj = LOMSE_NEW ImoOctaveShiftDto();     break;
        case k_imo_option:              pObj = LOMSE_NEW ImoOptionInfo();         break;
        case k_imo_options:             pObj = LOMSE_NEW ImoOptions();            break;
        case k_imo_ornament:            pObj = LOMSE_NEW ImoOrnament();           break;
        case k_imo_page_info:           pObj = LOMSE_NEW ImoPageInfo();           break;
        case k_imo_para:                pObj = LOMSE_NEW ImoParagraph();          break;
        case k_imo_param_info:          pObj = LOMSE_NEW ImoParamInfo();          break;
        case k_imo_pedal_mark:          pObj = LOMSE_NEW ImoPedalMark();          break;
        case k_imo_pedal_line:          pObj = LOMSE_NEW ImoPedalLine();          break;
        case k_imo_pedal_line_dto:      pObj = LOMSE_NEW ImoPedalLineDto();       break;
        case k_imo_relations:           pObj = LOMSE_NEW ImoRelations();          break;
        case k_imo_rest:                pObj = LOMSE_NEW ImoRest();               break;
        case k_imo_score:               pObj = LOMSE_NEW ImoScore();              break;
        case k_imo_score_line:          pObj = LOMSE_NEW ImoScoreLine();          break;
#if (LOMSE_ENABLE_THREADS == 1)
        case k_imo_score_player:        pObj = LOMSE_NEW ImoScorePlayer();        break;
#endif
        case k_imo_score_text:          pObj = LOMSE_NEW ImoScoreText();          break;
        case k_imo_score_title:         pObj = LOMSE_NEW ImoScoreTitle();         break;
        case k_imo_score_titles:        pObj = LOMSE_NEW ImoScoreTitles();        break;
        case k_imo_slur:                pObj = LOMSE_NEW ImoSlur();               break;
//        case k_imo_slur_data:           pObj = LOMSE_NEW ImoSlurData();           break;
        case k_imo_slur_dto:            pObj = LOMSE_NEW ImoSlurDto();            break;
        case k_imo_sound_change:        pObj = LOMSE_NEW ImoSoundChange();        break;
        case k_imo_sound_info:          pObj = LOMSE_NEW ImoSoundInfo();          break;
        case k_imo_sounds:              pObj = LOMSE_NEW ImoSounds();             break;
        case k_imo_parameters:          pObj = LOMSE_NEW ImoParameters();         break;
        case k_imo_staff_info:          pObj = LOMSE_NEW ImoStaffInfo();          break;
        case k_imo_style:               pObj = LOMSE_NEW ImoStyle();              break;
        case k_imo_styles:              pObj = LOMSE_NEW ImoStyles();             break;
        case k_imo_symbol_repetition_mark:  pObj = LOMSE_NEW ImoSymbolRepetitionMark();   break;
        case k_imo_system_break:        pObj = LOMSE_NEW ImoSystemBreak();        break;
        case k_imo_system_info:         pObj = LOMSE_NEW ImoSystemInfo();         break;
        case k_imo_table:               pObj = LOMSE_NEW ImoTable();              break;
        case k_imo_table_cell:          pObj = LOMSE_NEW ImoTableCell();          break;
        case k_imo_table_body:          pObj = LOMSE_NEW ImoTableBody();          break;
        case k_imo_table_head:          pObj = LOMSE_NEW ImoTableHead();          break;
        case k_imo_table_row:           pObj = LOMSE_NEW ImoTableRow();           break;
        case k_imo_technical:           pObj = LOMSE_NEW ImoTechnical();          break;
        case k_imo_textblock_info:      pObj = LOMSE_NEW ImoTextBlockInfo();      break;
        case k_imo_text_box:            pObj = LOMSE_NEW ImoTextBox();            break;
        case k_imo_text_item:           pObj = LOMSE_NEW ImoTextItem();           break;
        case k_imo_text_repetition_mark:   pObj = LOMSE_NEW ImoTextRepetitionMark();   break;
        case k_imo_tie:                 pObj = LOMSE_NEW ImoTie();                break;
        case k_imo_tie_data:            pObj = LOMSE_NEW ImoTieData();            break;
        case k_imo_tie_dto:             pObj = LOMSE_NEW ImoTieDto();             break;
        case k_imo_time_modification_dto:  pObj = LOMSE_NEW ImoTimeModificationDto();  break;
        case k_imo_time_signature:      pObj = LOMSE_NEW ImoTimeSignature();      break;
        case k_imo_transpose:           pObj = LOMSE_NEW ImoTranspose();          break;
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
        pDocModel->assign_id(pObj);
    }
    pObj->set_owner_model(pDocModel);
    pObj->initialize_object();
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoObj* ImFactory::clone(ImoObj* a)
{
    ImoObj* pImo = nullptr;
    switch(a->get_obj_type())
    {
        case k_imo_anonymous_block:     pImo = LOMSE_NEW ImoAnonymousBlock(*(static_cast<ImoAnonymousBlock*>(a)));          break;
        case k_imo_arpeggio:            pImo = LOMSE_NEW ImoArpeggio(*(static_cast<ImoArpeggio*>(a)));                      break;
        case k_imo_arpeggio_dto:        pImo = LOMSE_NEW ImoArpeggioDto(*(static_cast<ImoArpeggioDto*>(a)));                break;
        case k_imo_articulation_symbol: pImo = LOMSE_NEW ImoArticulationSymbol(*(static_cast<ImoArticulationSymbol*>(a)));  break;
        case k_imo_articulation_line:   pImo = LOMSE_NEW ImoArticulationLine(*(static_cast<ImoArticulationLine*>(a)));      break;
        case k_imo_attachments:         pImo = LOMSE_NEW ImoAttachments(*(static_cast<ImoAttachments*>(a)));                break;
        case k_imo_barline:             pImo = LOMSE_NEW ImoBarline(*(static_cast<ImoBarline*>(a)));                        break;
        case k_imo_beam:                pImo = LOMSE_NEW ImoBeam(*(static_cast<ImoBeam*>(a)));                              break;
        case k_imo_beam_data:           pImo = LOMSE_NEW ImoBeamData(*(static_cast<ImoBeamData*>(a)));                      break;
        case k_imo_beam_dto:            pImo = LOMSE_NEW ImoBeamDto(*(static_cast<ImoBeamDto*>(a)));                        break;
        case k_imo_bezier_info:         pImo = LOMSE_NEW ImoBezierInfo(*(static_cast<ImoBezierInfo*>(a)));                  break;
        case k_imo_button:              pImo = LOMSE_NEW ImoButton(*(static_cast<ImoButton*>(a)));                          break;
        case k_imo_chord:               pImo = LOMSE_NEW ImoChord(*(static_cast<ImoChord*>(a)));                            break;
        case k_imo_clef:                pImo = LOMSE_NEW ImoClef(*(static_cast<ImoClef*>(a)));                              break;
        case k_imo_color_dto:           pImo = LOMSE_NEW ImoColorDto(*(static_cast<ImoColorDto*>(a)));                      break;
        case k_imo_control:             pImo = LOMSE_NEW ImoControl(*(static_cast<ImoControl*>(a)));                        break;
        case k_imo_content:             pImo = LOMSE_NEW ImoContent(*(static_cast<ImoContent*>(a)));                        break;
        case k_imo_cursor_info:         pImo = LOMSE_NEW ImoCursorInfo(*(static_cast<ImoCursorInfo*>(a)));                  break;
        case k_imo_direction:           pImo = LOMSE_NEW ImoDirection(*(static_cast<ImoDirection*>(a)));                    break;
        case k_imo_document:            pImo = LOMSE_NEW ImoDocument(*(static_cast<ImoDocument*>(a)));                      break;
        case k_imo_dynamic:             pImo = LOMSE_NEW ImoDynamic(*(static_cast<ImoDynamic*>(a)));                        break;
        case k_imo_dynamics_mark:       pImo = LOMSE_NEW ImoDynamicsMark(*(static_cast<ImoDynamicsMark*>(a)));              break;
        case k_imo_fermata:             pImo = LOMSE_NEW ImoFermata(*(static_cast<ImoFermata*>(a)));                        break;
        case k_imo_fingering:           pImo = LOMSE_NEW ImoFingering(*(static_cast<ImoFingering*>(a)));                    break;
        case k_imo_font_style_dto:      pImo = LOMSE_NEW ImoFontStyleDto(*(static_cast<ImoFontStyleDto*>(a)));              break;
        case k_imo_fret_string:         pImo = LOMSE_NEW ImoFretString(*(static_cast<ImoFretString*>(a)));                  break;
        case k_imo_go_back_fwd:         pImo = LOMSE_NEW ImoGoBackFwd(*(static_cast<ImoGoBackFwd*>(a)));                    break;
        case k_imo_grace_relobj:        pImo = LOMSE_NEW ImoGraceRelObj(*(static_cast<ImoGraceRelObj*>(a)));                break;
        case k_imo_heading:             pImo = LOMSE_NEW ImoHeading(*(static_cast<ImoHeading*>(a)));                        break;
        case k_imo_image:               pImo = LOMSE_NEW ImoImage(*(static_cast<ImoImage*>(a)));                            break;
        case k_imo_inline_wrapper:      pImo = LOMSE_NEW ImoInlineWrapper(*(static_cast<ImoInlineWrapper*>(a)));            break;
        case k_imo_instr_group:         pImo = LOMSE_NEW ImoInstrGroup(*(static_cast<ImoInstrGroup*>(a)));                  break;
        case k_imo_instrument:          pImo = LOMSE_NEW ImoInstrument(*(static_cast<ImoInstrument*>(a)));                  break;
        case k_imo_instruments:         pImo = LOMSE_NEW ImoInstruments(*(static_cast<ImoInstruments*>(a)));                break;
        case k_imo_instrument_groups:   pImo = LOMSE_NEW ImoInstrGroups(*(static_cast<ImoInstrGroups*>(a)));                break;
        case k_imo_key_signature:       pImo = LOMSE_NEW ImoKeySignature(*(static_cast<ImoKeySignature*>(a)));              break;
        case k_imo_line:                pImo = LOMSE_NEW ImoLine(*(static_cast<ImoLine*>(a)));                              break;
        case k_imo_line_style_dto:      pImo = LOMSE_NEW ImoLineStyleDto(*(static_cast<ImoLineStyleDto*>(a)));              break;
        case k_imo_list:                pImo = LOMSE_NEW ImoList(*(static_cast<ImoList*>(a)));                              break;
        case k_imo_listitem:            pImo = LOMSE_NEW ImoListItem(*(static_cast<ImoListItem*>(a)));                      break;
        case k_imo_link:                pImo = LOMSE_NEW ImoLink(*(static_cast<ImoLink*>(a)));                              break;
        case k_imo_lyric:               pImo = LOMSE_NEW ImoLyric(*(static_cast<ImoLyric*>(a)));                            break;
        case k_imo_lyrics_text_info:    pImo = LOMSE_NEW ImoLyricsTextInfo(*(static_cast<ImoLyricsTextInfo*>(a)));          break;
        case k_imo_metronome_mark:      pImo = LOMSE_NEW ImoMetronomeMark(*(static_cast<ImoMetronomeMark*>(a)));            break;
        case k_imo_midi_info:           pImo = LOMSE_NEW ImoMidiInfo(*(static_cast<ImoMidiInfo*>(a)));                      break;
        case k_imo_multicolumn:         pImo = LOMSE_NEW ImoMultiColumn(*(static_cast<ImoMultiColumn*>(a)));                break;
        case k_imo_music_data:          pImo = LOMSE_NEW ImoMusicData(*(static_cast<ImoMusicData*>(a)));                    break;
        case k_imo_note_cue:            pImo = LOMSE_NEW ImoNote(*(static_cast<ImoNote*>(a)));                              break;
        case k_imo_note_grace:          pImo = LOMSE_NEW ImoGraceNote(*(static_cast<ImoGraceNote*>(a)));                    break;
        case k_imo_note_regular:        pImo = LOMSE_NEW ImoNote(*(static_cast<ImoNote*>(a)));                              break;
        case k_imo_octave_shift:        pImo = LOMSE_NEW ImoOctaveShift(*(static_cast<ImoOctaveShift*>(a)));                break;
        case k_imo_octave_shift_dto:    pImo = LOMSE_NEW ImoOctaveShiftDto(*(static_cast<ImoOctaveShiftDto*>(a)));          break;
        case k_imo_option:              pImo = LOMSE_NEW ImoOptionInfo(*(static_cast<ImoOptionInfo*>(a)));                  break;
        case k_imo_options:             pImo = LOMSE_NEW ImoOptions(*(static_cast<ImoOptions*>(a)));                        break;
        case k_imo_ornament:            pImo = LOMSE_NEW ImoOrnament(*(static_cast<ImoOrnament*>(a)));                      break;
        case k_imo_page_info:           pImo = LOMSE_NEW ImoPageInfo(*(static_cast<ImoPageInfo*>(a)));                      break;
        case k_imo_para:                pImo = LOMSE_NEW ImoParagraph(*(static_cast<ImoParagraph*>(a)));                    break;
        case k_imo_param_info:          pImo = LOMSE_NEW ImoParamInfo(*(static_cast<ImoParamInfo*>(a)));                    break;
        case k_imo_pedal_mark:          pImo = LOMSE_NEW ImoPedalMark(*(static_cast<ImoPedalMark*>(a)));                    break;
        case k_imo_pedal_line:          pImo = LOMSE_NEW ImoPedalLine(*(static_cast<ImoPedalLine*>(a)));                    break;
        case k_imo_pedal_line_dto:      pImo = LOMSE_NEW ImoPedalLineDto(*(static_cast<ImoPedalLineDto*>(a)));              break;
        case k_imo_relations:           pImo = LOMSE_NEW ImoRelations(*(static_cast<ImoRelations*>(a)));                    break;
        case k_imo_rest:                pImo = LOMSE_NEW ImoRest(*(static_cast<ImoRest*>(a)));                              break;
        case k_imo_score:               pImo = LOMSE_NEW ImoScore(*(static_cast<ImoScore*>(a)));                            break;
        case k_imo_score_line:          pImo = LOMSE_NEW ImoScoreLine(*(static_cast<ImoScoreLine*>(a)));                    break;
#if (LOMSE_ENABLE_THREADS == 1)
        case k_imo_score_player:        pImo = LOMSE_NEW ImoScorePlayer(*(static_cast<ImoScorePlayer*>(a)));                break;
#endif
        case k_imo_score_text:          pImo = LOMSE_NEW ImoScoreText(*(static_cast<ImoScoreText*>(a)));                    break;
        case k_imo_score_title:         pImo = LOMSE_NEW ImoScoreTitle(*(static_cast<ImoScoreTitle*>(a)));                  break;
        case k_imo_score_titles:        pImo = LOMSE_NEW ImoScoreTitles(*(static_cast<ImoScoreTitles*>(a)));                break;
        case k_imo_slur:                pImo = LOMSE_NEW ImoSlur(*(static_cast<ImoSlur*>(a)));                              break;
        case k_imo_slur_data:           pImo = LOMSE_NEW ImoSlurData(*(static_cast<ImoSlurData*>(a)));                      break;
        case k_imo_slur_dto:            pImo = LOMSE_NEW ImoSlurDto(*(static_cast<ImoSlurDto*>(a)));                        break;
        case k_imo_sound_change:        pImo = LOMSE_NEW ImoSoundChange(*(static_cast<ImoSoundChange*>(a)));                break;
        case k_imo_sound_info:          pImo = LOMSE_NEW ImoSoundInfo(*(static_cast<ImoSoundInfo*>(a)));                    break;
        case k_imo_sounds:              pImo = LOMSE_NEW ImoSounds(*(static_cast<ImoSounds*>(a)));                          break;
        case k_imo_parameters:          pImo = LOMSE_NEW ImoParameters(*(static_cast<ImoParameters*>(a)));                  break;
        case k_imo_staff_info:          pImo = LOMSE_NEW ImoStaffInfo(*(static_cast<ImoStaffInfo*>(a)));                    break;
        case k_imo_style:               pImo = LOMSE_NEW ImoStyle(*(static_cast<ImoStyle*>(a)));                            break;
        case k_imo_styles:              pImo = LOMSE_NEW ImoStyles(*(static_cast<ImoStyles*>(a)));                          break;
        case k_imo_symbol_repetition_mark:  pImo = LOMSE_NEW ImoSymbolRepetitionMark(*(static_cast<ImoSymbolRepetitionMark*>(a)));    break;
        case k_imo_system_break:        pImo = LOMSE_NEW ImoSystemBreak(*(static_cast<ImoSystemBreak*>(a)));                break;
        case k_imo_system_info:         pImo = LOMSE_NEW ImoSystemInfo(*(static_cast<ImoSystemInfo*>(a)));                  break;
        case k_imo_table:               pImo = LOMSE_NEW ImoTable(*(static_cast<ImoTable*>(a)));                            break;
        case k_imo_table_cell:          pImo = LOMSE_NEW ImoTableCell(*(static_cast<ImoTableCell*>(a)));                    break;
        case k_imo_table_body:          pImo = LOMSE_NEW ImoTableBody(*(static_cast<ImoTableBody*>(a)));                    break;
        case k_imo_table_head:          pImo = LOMSE_NEW ImoTableHead(*(static_cast<ImoTableHead*>(a)));                    break;
        case k_imo_table_row:           pImo = LOMSE_NEW ImoTableRow(*(static_cast<ImoTableRow*>(a)));                      break;
        case k_imo_technical:           pImo = LOMSE_NEW ImoTechnical(*(static_cast<ImoTechnical*>(a)));                    break;
        case k_imo_textblock_info:      pImo = LOMSE_NEW ImoTextBlockInfo(*(static_cast<ImoTextBlockInfo*>(a)));            break;
        case k_imo_text_box:            pImo = LOMSE_NEW ImoTextBox(*(static_cast<ImoTextBox*>(a)));                        break;
        case k_imo_text_item:           pImo = LOMSE_NEW ImoTextItem(*(static_cast<ImoTextItem*>(a)));                      break;
        case k_imo_text_repetition_mark:   pImo = LOMSE_NEW ImoTextRepetitionMark(*(static_cast<ImoTextRepetitionMark*>(a)));    break;
        case k_imo_tie:                 pImo = LOMSE_NEW ImoTie(*(static_cast<ImoTie*>(a)));                                break;
        case k_imo_tie_data:            pImo = LOMSE_NEW ImoTieData(*(static_cast<ImoTieData*>(a)));                        break;
        case k_imo_tie_dto:             pImo = LOMSE_NEW ImoTieDto(*(static_cast<ImoTieDto*>(a)));                          break;
        case k_imo_time_modification_dto:  pImo = LOMSE_NEW ImoTimeModificationDto(*(static_cast<ImoTimeModificationDto*>(a)));    break;
        case k_imo_time_signature:      pImo = LOMSE_NEW ImoTimeSignature(*(static_cast<ImoTimeSignature*>(a)));            break;
        case k_imo_transpose:           pImo = LOMSE_NEW ImoTranspose(*(static_cast<ImoTranspose*>(a)));                    break;
        case k_imo_tuplet:              pImo = LOMSE_NEW ImoTuplet(*(static_cast<ImoTuplet*>(a)));                          break;
        case k_imo_tuplet_dto:          pImo = LOMSE_NEW ImoTupletDto(*(static_cast<ImoTupletDto*>(a)));                    break;
        case k_imo_volta_bracket:       pImo = LOMSE_NEW ImoVoltaBracket(*(static_cast<ImoVoltaBracket*>(a)));              break;
        case k_imo_volta_bracket_dto:   pImo = LOMSE_NEW ImoVoltaBracketDto(*(static_cast<ImoVoltaBracketDto*>(a)));        break;
        case k_imo_wedge:               pImo = LOMSE_NEW ImoWedge(*(static_cast<ImoWedge*>(a)));                            break;
        case k_imo_wedge_dto:           pImo = LOMSE_NEW ImoWedgeDto(*(static_cast<ImoWedgeDto*>(a)));                      break;
        default:
        {
            stringstream msg;
            msg << "Fatal. Object not included in clone table: " << a->get_name() << ". Object not cloned!";
            LOMSE_LOG_ERROR(msg.str());
        }
    }

    return pImo;
}

//---------------------------------------------------------------------------------------
ImoBeamData* ImFactory::inject_beam_data(DocModel* pDocModel, ImoBeamDto* pDto)
{
    ImoBeamData* pObj = LOMSE_NEW ImoBeamData(pDto);
    pDocModel->assign_id(pObj);
    pObj->set_owner_model(pDocModel);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoTieData* ImFactory::inject_tie_data(DocModel* pDocModel, ImoTieDto* pDto)
{
    ImoTieData* pObj = LOMSE_NEW ImoTieData(pDto);
    pDocModel->assign_id(pObj);
    pObj->set_owner_model(pDocModel);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoSlurData* ImFactory::inject_slur_data(DocModel* pDocModel, ImoSlurDto* pDto)
{
    ImoSlurData* pObj = LOMSE_NEW ImoSlurData(pDto);
    pDocModel->assign_id(pObj);
    pObj->set_owner_model(pDocModel);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoTuplet* ImFactory::inject_tuplet(DocModel* pDocModel, ImoTupletDto* pDto)
{
    ImoTuplet* pObj = LOMSE_NEW ImoTuplet(pDto);
    pObj->set_id( pDto->get_id() );
    pDocModel->assign_id(pObj);
    pObj->set_owner_model(pDocModel);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoTextBox* ImFactory::inject_text_box(DocModel* pDocModel, ImoTextBlockInfo& dto, ImoId id)
{
    ImoTextBox* pObj = LOMSE_NEW ImoTextBox(dto);
    pObj->set_id(id);
    pDocModel->assign_id(pObj);
    pObj->set_owner_model(pDocModel);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoNote* ImFactory::inject_note(DocModel* pDocModel, int step, int octave,
                                int noteType, EAccidentals accidentals,
                                int dots, int staff, int voice, int stem)
{
    ImoNote* pObj = LOMSE_NEW ImoNote(step, octave, noteType, accidentals, dots,
                                staff, voice, stem);
    pDocModel->assign_id(pObj);
    pObj->set_owner_model(pDocModel);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoMultiColumn* ImFactory::inject_multicolumn(DocModel* pDocModel)
{
    ImoMultiColumn* pObj = LOMSE_NEW ImoMultiColumn();
    pDocModel->assign_id(pObj);
    pObj->set_owner_model(pDocModel);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoImage* ImFactory::inject_image(DocModel* pDocModel, unsigned char* imgbuf, VSize bmpSize,
                                  EPixelFormat format, USize imgSize)
{
    ImoImage* pObj = LOMSE_NEW ImoImage(imgbuf, bmpSize, format, imgSize);
    pDocModel->assign_id(pObj);
    pObj->set_owner_model(pDocModel);
    return pObj;
}

//---------------------------------------------------------------------------------------
ImoControl* ImFactory::inject_control(DocModel* pDocModel)
{
    ImoControl* pObj = LOMSE_NEW ImoControl(k_imo_control);
    pDocModel->assign_id(pObj);
    pObj->set_owner_model(pDocModel);
    return pObj;
}



//factory injector, from type
ImoObj* ImFactory::inject(int type, Document* pDoc, ImoId id)
    { return inject(type, pDoc->get_doc_model(), id); }

//factory injector, from LDP source code
ImoObj* ImFactory::inject(Document* pDoc, const std::string& ldpSource)
    { return inject(pDoc->get_doc_model(), ldpSource); }

//specific injectors, to simplify testing
ImoNote* ImFactory::inject_note(Document* pDoc, int step, int octave,
                            int noteType, EAccidentals accidentals,
                            int dots, int staff, int voice,
                            int stem)
    { return inject_note(pDoc->get_doc_model(),step, octave,
                         noteType, accidentals, dots, staff, voice, stem); }

ImoBeamData* ImFactory::inject_beam_data(Document* pDoc, ImoBeamDto* pDto)
    { return inject_beam_data(pDoc->get_doc_model(), pDto); }

ImoTieData* ImFactory::inject_tie_data(Document* pDoc, ImoTieDto* pDto)
    { return inject_tie_data(pDoc->get_doc_model(), pDto); }

ImoSlurData* ImFactory::inject_slur_data(Document* pDoc, ImoSlurDto* pDto)
    { return inject_slur_data(pDoc->get_doc_model(), pDto); }

ImoTuplet* ImFactory::inject_tuplet(Document* pDoc, ImoTupletDto* pDto)
    { return inject_tuplet(pDoc->get_doc_model(), pDto); }

ImoTextBox* ImFactory::inject_text_box(Document* pDoc, ImoTextBlockInfo& dto, ImoId id)
    { return inject_text_box(pDoc->get_doc_model(), dto, id); }

ImoMultiColumn* ImFactory::inject_multicolumn(Document* pDoc)
    { return inject_multicolumn(pDoc->get_doc_model()); }

ImoImage* ImFactory::inject_image(Document* pDoc, unsigned char* imgbuf,
                              VSize bmpSize, EPixelFormat format, USize imgSize)
    { return inject_image(pDoc->get_doc_model(), imgbuf, bmpSize, format, imgSize); }

ImoControl* ImFactory::inject_control(Document* pDoc)
    { return inject_control(pDoc->get_doc_model()); }


}  //namespace lomse

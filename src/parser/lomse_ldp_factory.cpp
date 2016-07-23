//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#include "lomse_ldp_factory.h"

#include "lomse_logger.h"

#include <sstream>
#include <iostream>
using namespace std;

namespace lomse
{

class LdpFunctor
{
public:
	LdpFunctor() {}
	virtual ~LdpFunctor() {}
    virtual LdpElement* operator ()() = 0;
};


template<ELdpElement type>
class LdpElementFunctor : public LdpFunctor
{
public:
	LdpElement* operator ()() {  return LdpObject<type>::new_ldp_object(); }
};


LdpFactory::LdpFactory()
{
    //Register all ldp elements

    //simple, generic elements
    m_TypeToName[k_label] = "label";
    m_TypeToName[k_number] = "number";
    m_TypeToName[k_string] = "string";

    //composite elements
    m_TypeToName[k_abbrev] = "abbrev";
    m_TypeToName[k_above] = "above";
    m_TypeToName[k_accent] = "accent";
    m_TypeToName[k_anchorLine] = "anchorLine";
    m_TypeToName[k_background_color] = "background-color";
    m_TypeToName[k_barline] = "barline";
    m_TypeToName[k_beam] = "beam";
    m_TypeToName[k_below] = "below";
    m_TypeToName[k_bezier] = "bezier";
    m_TypeToName[k_bold] = "bold";
    m_TypeToName[k_bold_italic] = "bold-italic";
    m_TypeToName[k_border] = "border";
    m_TypeToName[k_border_width] = "border-width";
    m_TypeToName[k_border_width_top] = "border-width-top";
    m_TypeToName[k_border_width_right] = "border-width-right";
    m_TypeToName[k_border_width_bottom] = "border-width-bottom";
    m_TypeToName[k_border_width_left] = "border-width-left";
    m_TypeToName[k_brace] = "brace";
    m_TypeToName[k_bracket] = "bracket";
    m_TypeToName[k_bracketType] = "bracketType";
    m_TypeToName[k_breath_mark] = "breathMark";
    m_TypeToName[k_caesura] = "caesura";
    m_TypeToName[k_center] = "center";
    m_TypeToName[k_chord] = "chord";
    m_TypeToName[k_classid] = "classid";
    m_TypeToName[k_clef] = "clef";
    m_TypeToName[k_color] = "color";
    m_TypeToName[k_colspan] = "colspan";
    m_TypeToName[k_content] = "content";
    m_TypeToName[k_creationMode] = "creationMode";
    m_TypeToName[k_ctrol1_x] = "ctrol1-x";
    m_TypeToName[k_ctrol1_y] = "ctrol1-y";
    m_TypeToName[k_ctrol2_x] = "ctrol2-x";
    m_TypeToName[k_ctrol2_y] = "ctrol2-y";
    m_TypeToName[k_cursor] = "cursor";
    m_TypeToName[k_defineStyle] = "defineStyle";
    m_TypeToName[k_doit] = "doit";
    m_TypeToName[k_down] = "down";
    m_TypeToName[k_displayBracket] = "displayBracket";
    m_TypeToName[k_displayNumber] = "displayNumber";
    m_TypeToName[k_duration] = "duration";
    m_TypeToName[k_dx] = "dx";
    m_TypeToName[k_dy] = "dy";
    m_TypeToName[k_dynamic] = "dynamic";
    m_TypeToName[k_dynamics_mark] = "k_dynamics_mark";
    m_TypeToName[k_end] = "end";
    m_TypeToName[k_end_x] = "end-x";
    m_TypeToName[k_end_y] = "end-y";
    m_TypeToName[k_endPoint] = "endPoint";
    m_TypeToName[k_falloff] = "falloff";
    m_TypeToName[k_fbline] = "fbline";
    m_TypeToName[k_fermata] = "fermata";
    m_TypeToName[k_figuredBass] = "figuredBass";
    m_TypeToName[k_file] = "file";
    m_TypeToName[k_font] = "font";
    m_TypeToName[k_font_file] = "font-file";
    m_TypeToName[k_font_name] = "font-name";
    m_TypeToName[k_font_size] = "font-size";
    m_TypeToName[k_font_style] = "font-style";
    m_TypeToName[k_font_weight] = "font-weight";
    m_TypeToName[k_goBack] = "goBack";
    m_TypeToName[k_goFwd] = "goFwd";
    m_TypeToName[k_graphic] = "graphic";
    m_TypeToName[k_group] = "group";
    m_TypeToName[k_hasWidth] = "hasWidth";
    m_TypeToName[k_heading] = "heading";
    m_TypeToName[k_height] = "height";
    m_TypeToName[k_image] = "image";
    m_TypeToName[k_infoMIDI] = "infoMIDI";
    m_TypeToName[k_instrIds] = "instrIds";
    m_TypeToName[k_instrument] = "instrument";
    m_TypeToName[k_font_style_italic] = "italic";
    m_TypeToName[k_itemizedlist] = "itemizedlist";
    m_TypeToName[k_joinBarlines] = "joinBarlines";
    m_TypeToName[k_key_signature] = "key";
    m_TypeToName[k_landscape] = "landscape";
    m_TypeToName[k_language] = "language";
    m_TypeToName[k_left] = "left";
    m_TypeToName[k_legato_duro] = "legato-duro";
    m_TypeToName[k_lenmusdoc] = "lenmusdoc";
    m_TypeToName[k_link] = "link";
    m_TypeToName[k_line] = "line";
    m_TypeToName[k_lineCapEnd] = "lineCapEnd";
    m_TypeToName[k_lineCapStart] = "lineCapStart";
    m_TypeToName[k_lineStyle] = "lineStyle";
    m_TypeToName[k_lineThickness] = "lineThickness";
    m_TypeToName[k_line_height] = "line-height";
    m_TypeToName[k_listitem] = "listitem";
    m_TypeToName[k_lyric] = "lyric";
    m_TypeToName[k_marccato] = "marccato";
    m_TypeToName[k_marccato_legato] = "marccato-legato";
    m_TypeToName[k_marccato_staccato] = "marccato-staccato";
    m_TypeToName[k_marccato_staccatissimo] = "marccato-staccatissimo";
    m_TypeToName[k_margin] = "margin";
    m_TypeToName[k_margin_top] = "margin-top";
    m_TypeToName[k_margin_right] = "margin-right";
    m_TypeToName[k_margin_bottom] = "margin-bottom";
    m_TypeToName[k_margin_left] = "margin-left";
    m_TypeToName[k_max_height] = "max-height";
    m_TypeToName[k_max_width] = "max-width";
    m_TypeToName[k_melisma] = "melisma";
    m_TypeToName[k_meta] = "meta";
    m_TypeToName[k_metronome] = "metronome";
    m_TypeToName[k_mezzo_staccato] = "mezzo-staccato";
    m_TypeToName[k_mezzo_staccatissimo] = "mezzo-staccatissimo";
    m_TypeToName[k_min_height] = "min-height";
    m_TypeToName[k_min_width] = "min-width";
    m_TypeToName[k_mm] = "mm";
    m_TypeToName[k_musicData] = "musicData";
    m_TypeToName[k_na] = "na";  //note in chord
    m_TypeToName[k_name] = "name";
    m_TypeToName[k_newSystem] = "newSystem";
    m_TypeToName[k_no] = "no";
    m_TypeToName[k_normal] = "normal";
    m_TypeToName[k_note] = "n";   //note
    m_TypeToName[k_opt] = "opt";
    m_TypeToName[k_orderedlist] = "orderedlist";
    m_TypeToName[k_padding] = "padding";
    m_TypeToName[k_padding_top] = "padding-top";
    m_TypeToName[k_padding_right] = "padding-right";
    m_TypeToName[k_padding_bottom] = "padding-bottom";
    m_TypeToName[k_padding_left] = "padding-left";
    m_TypeToName[k_pageLayout] = "pageLayout";
    m_TypeToName[k_pageMargins] = "pageMargins";
    m_TypeToName[k_pageSize] = "pageSize";
    m_TypeToName[k_para] = "para";
    m_TypeToName[k_parameter] = "param";
    m_TypeToName[k_parenthesis] = "parenthesis";
    m_TypeToName[k_parts] = "parts";
    m_TypeToName[k_pitch] = "pitch";
    m_TypeToName[k_playLabel] = "playLabel";
    m_TypeToName[k_plop] = "plop";
    m_TypeToName[k_portrait] = "portrait";
    m_TypeToName[k_rest] = "r";   //rest
    m_TypeToName[k_right] = "right";
    m_TypeToName[k_rowspan] = "rowspan";
    m_TypeToName[k_scoop] = "scoop";
    m_TypeToName[k_score] = "score";
    m_TypeToName[k_score_player] = "scorePlayer";
    m_TypeToName[k_settings] = "settings";
    m_TypeToName[k_slur] = "slur";
    m_TypeToName[k_size] = "size";
    m_TypeToName[k_spacer] = "spacer";
    m_TypeToName[k_split] = "split";
    m_TypeToName[k_staccato] = "staccato";
    m_TypeToName[k_staccato_duro] = "staccato-duro";
    m_TypeToName[k_staccatissimo] = "staccatissimo";
    m_TypeToName[k_staccatissimo_duro] = "staccatissimo-duro";
    m_TypeToName[k_staff] = "staff";
    m_TypeToName[k_staffDistance] = "staffDistance";
    m_TypeToName[k_staffLines] = "staffLines";
    m_TypeToName[k_staffNum] = "staffNum";
    m_TypeToName[k_staffSpacing] = "staffSpacing";
    m_TypeToName[k_staffType] = "staffType";
    m_TypeToName[k_start_x] = "start-x";
    m_TypeToName[k_start_y] = "start-y";
    m_TypeToName[k_startPoint] = "startPoint";
    m_TypeToName[k_start] = "start";
    m_TypeToName[k_staves] = "staves";
    m_TypeToName[k_stem] = "stem";
    m_TypeToName[k_stopLabel] = "stopLabel";
    m_TypeToName[k_stress] = "stress";
    m_TypeToName[k_style] = "style";
    m_TypeToName[k_styles] = "styles";
    m_TypeToName[k_syllable] = "syl";
    m_TypeToName[k_symbol] = "symbol";
    m_TypeToName[k_symbolSize] = "symbolSize";
    m_TypeToName[k_systemLayout] = "systemLayout";
    m_TypeToName[k_systemMargins] = "systemMargins";
    m_TypeToName[k_table] = "table";
    m_TypeToName[k_table_cell] = "tableCell";
    m_TypeToName[k_table_column] = "tableColumn";
    m_TypeToName[k_table_col_width] = "table-col-width";
    m_TypeToName[k_table_body] = "tableBody";
    m_TypeToName[k_table_head] = "tableHead";
    m_TypeToName[k_table_row] = "tableRow";
    m_TypeToName[k_tenuto] = "tenuto";
    m_TypeToName[k_text] = "text";
    m_TypeToName[k_textbox] = "textbox";
    m_TypeToName[k_text_align] = "text-align";
    m_TypeToName[k_text_decoration] = "text-decoration";
    m_TypeToName[k_tie] = "tie";
    m_TypeToName[k_time_modification] = "tm";
    m_TypeToName[k_time_signature] = "time";
    m_TypeToName[k_title] = "title";
    m_TypeToName[k_tuplet] = "t";
    m_TypeToName[k_txt] = "txt";
    m_TypeToName[k_undefined] = "undefined";
    m_TypeToName[k_undoData] = "undoData";
    m_TypeToName[k_up] = "up";
    m_TypeToName[k_url] = "url";
    m_TypeToName[k_unstress] = "unstress";
    m_TypeToName[k_value] = "value";
    m_TypeToName[k_vers] = "vers";
    m_TypeToName[k_vertical_align] = "vertical-align";
    m_TypeToName[k_visible] = "visible";
    m_TypeToName[k_voice] = "voice";
    m_TypeToName[k_width] = "width";
    m_TypeToName[k_yes] = "yes";


    //Register all types
    m_NameToFunctor["label"] = LOMSE_NEW LdpElementFunctor<k_label>;
    m_NameToFunctor["number"] = LOMSE_NEW LdpElementFunctor<k_number>;
    m_NameToFunctor["string"] = LOMSE_NEW LdpElementFunctor<k_string>;

    m_NameToFunctor["abbrev"] = LOMSE_NEW LdpElementFunctor<k_abbrev>;
    m_NameToFunctor["above"] = LOMSE_NEW LdpElementFunctor<k_above>;
    m_NameToFunctor["accent"] = LOMSE_NEW LdpElementFunctor<k_accent>;
    m_NameToFunctor["anchorLine"] = LOMSE_NEW LdpElementFunctor<k_anchorLine>;
    m_NameToFunctor["background-color"] = LOMSE_NEW LdpElementFunctor<k_background_color>;
    m_NameToFunctor["barline"] = LOMSE_NEW LdpElementFunctor<k_barline>;
    m_NameToFunctor["beam"] = LOMSE_NEW LdpElementFunctor<k_beam>;
    m_NameToFunctor["below"] = LOMSE_NEW LdpElementFunctor<k_below>;
    m_NameToFunctor["bezier"] = LOMSE_NEW LdpElementFunctor<k_bezier>;
    m_NameToFunctor["bold"] = LOMSE_NEW LdpElementFunctor<k_bold>;
    m_NameToFunctor["bold_italic"] = LOMSE_NEW LdpElementFunctor<k_bold_italic>;
    m_NameToFunctor["border"] = LOMSE_NEW LdpElementFunctor<k_border>;
    m_NameToFunctor["border-width"] = LOMSE_NEW LdpElementFunctor<k_border_width>;
    m_NameToFunctor["border-width-top"] = LOMSE_NEW LdpElementFunctor<k_border_width_top>;
    m_NameToFunctor["border-width-right"] = LOMSE_NEW LdpElementFunctor<k_border_width_right>;
    m_NameToFunctor["border-width-bottom"] = LOMSE_NEW LdpElementFunctor<k_border_width_bottom>;
    m_NameToFunctor["border-width-left"] = LOMSE_NEW LdpElementFunctor<k_border_width_left>;
    m_NameToFunctor["brace"] = LOMSE_NEW LdpElementFunctor<k_brace>;
    m_NameToFunctor["bracket"] = LOMSE_NEW LdpElementFunctor<k_bracket>;
    m_NameToFunctor["bracketType"] = LOMSE_NEW LdpElementFunctor<k_bracketType>;
    m_NameToFunctor["breathMark"] = LOMSE_NEW LdpElementFunctor<k_breath_mark>;
    m_NameToFunctor["caesura"] = LOMSE_NEW LdpElementFunctor<k_caesura>;
    m_NameToFunctor["center"] = LOMSE_NEW LdpElementFunctor<k_center>;
    m_NameToFunctor["chord"] = LOMSE_NEW LdpElementFunctor<k_chord>;
    m_NameToFunctor["classid"] = LOMSE_NEW LdpElementFunctor<k_classid>;
    m_NameToFunctor["clef"] = LOMSE_NEW LdpElementFunctor<k_clef>;
    m_NameToFunctor["color"] = LOMSE_NEW LdpElementFunctor<k_color>;
    m_NameToFunctor["colspan"] = LOMSE_NEW LdpElementFunctor<k_colspan>;
    m_NameToFunctor["content"] = LOMSE_NEW LdpElementFunctor<k_content>;
    m_NameToFunctor["creationMode"] = LOMSE_NEW LdpElementFunctor<k_creationMode>;
    m_NameToFunctor["ctrol1-x"] = LOMSE_NEW LdpElementFunctor<k_ctrol1_x>;
    m_NameToFunctor["ctrol1-y"] = LOMSE_NEW LdpElementFunctor<k_ctrol1_y>;
    m_NameToFunctor["ctrol2-x"] = LOMSE_NEW LdpElementFunctor<k_ctrol2_x>;
    m_NameToFunctor["ctrol2-y"] = LOMSE_NEW LdpElementFunctor<k_ctrol2_y>;
    m_NameToFunctor["cursor"] = LOMSE_NEW LdpElementFunctor<k_cursor>;
    m_NameToFunctor["defineStyle"] = LOMSE_NEW LdpElementFunctor<k_defineStyle>;
    m_NameToFunctor["doit"] = LOMSE_NEW LdpElementFunctor<k_doit>;
    m_NameToFunctor["down"] = LOMSE_NEW LdpElementFunctor<k_down>;
    m_NameToFunctor["duration"] = LOMSE_NEW LdpElementFunctor<k_duration>;
    m_NameToFunctor["displayBracket"] = LOMSE_NEW LdpElementFunctor<k_displayBracket>;
    m_NameToFunctor["displayNumber"] = LOMSE_NEW LdpElementFunctor<k_displayNumber>;
    m_NameToFunctor["dx"] = LOMSE_NEW LdpElementFunctor<k_dx>;
    m_NameToFunctor["dy"] = LOMSE_NEW LdpElementFunctor<k_dy>;
    m_NameToFunctor["dyn"] = LOMSE_NEW LdpElementFunctor<k_dynamics_mark>;
    m_NameToFunctor["dynamic"] = LOMSE_NEW LdpElementFunctor<k_dynamic>;
    m_NameToFunctor["end"] = LOMSE_NEW LdpElementFunctor<k_end>;
    m_NameToFunctor["end-x"] = LOMSE_NEW LdpElementFunctor<k_end_x>;
    m_NameToFunctor["end-y"] = LOMSE_NEW LdpElementFunctor<k_end_y>;
    m_NameToFunctor["endPoint"] = LOMSE_NEW LdpElementFunctor<k_endPoint>;
    m_NameToFunctor["falloff"] = LOMSE_NEW LdpElementFunctor<k_falloff>;
    m_NameToFunctor["fbline"] = LOMSE_NEW LdpElementFunctor<k_fbline>;
    m_NameToFunctor["fermata"] = LOMSE_NEW LdpElementFunctor<k_fermata>;
    m_NameToFunctor["figuredBass"] = LOMSE_NEW LdpElementFunctor<k_figuredBass>;
    m_NameToFunctor["file"] = LOMSE_NEW LdpElementFunctor<k_file>;
    m_NameToFunctor["font"] = LOMSE_NEW LdpElementFunctor<k_font>;
    m_NameToFunctor["font-file"] = LOMSE_NEW LdpElementFunctor<k_font_file>;
    m_NameToFunctor["font-name"] = LOMSE_NEW LdpElementFunctor<k_font_name>;
    m_NameToFunctor["font-size"] = LOMSE_NEW LdpElementFunctor<k_font_size>;
    m_NameToFunctor["font-style"] = LOMSE_NEW LdpElementFunctor<k_font_style>;
    m_NameToFunctor["font-weight"] = LOMSE_NEW LdpElementFunctor<k_font_weight>;
    m_NameToFunctor["goBack"] = LOMSE_NEW LdpElementFunctor<k_goBack>;
    m_NameToFunctor["goFwd"] = LOMSE_NEW LdpElementFunctor<k_goFwd>;
    m_NameToFunctor["graphic"] = LOMSE_NEW LdpElementFunctor<k_graphic>;
    m_NameToFunctor["group"] = LOMSE_NEW LdpElementFunctor<k_group>;
    m_NameToFunctor["hasWidth"] = LOMSE_NEW LdpElementFunctor<k_hasWidth>;
    m_NameToFunctor["heading"] = LOMSE_NEW LdpElementFunctor<k_heading>;
    m_NameToFunctor["height"] = LOMSE_NEW LdpElementFunctor<k_height>;
    m_NameToFunctor["image"] = LOMSE_NEW LdpElementFunctor<k_image>;
    m_NameToFunctor["infoMIDI"] = LOMSE_NEW LdpElementFunctor<k_infoMIDI>;
    m_NameToFunctor["instrIds"] = LOMSE_NEW LdpElementFunctor<k_instrIds>;
    m_NameToFunctor["instrument"] = LOMSE_NEW LdpElementFunctor<k_instrument>;
    m_NameToFunctor["italic"] = LOMSE_NEW LdpElementFunctor<k_font_style_italic>;
    m_NameToFunctor["itemizedlist"] = LOMSE_NEW LdpElementFunctor<k_itemizedlist>;
    m_NameToFunctor["joinBarlines"] = LOMSE_NEW LdpElementFunctor<k_joinBarlines>;
    m_NameToFunctor["key"] = LOMSE_NEW LdpElementFunctor<k_key_signature>;
    m_NameToFunctor["landscape"] = LOMSE_NEW LdpElementFunctor<k_landscape>;
    m_NameToFunctor["language"] = LOMSE_NEW LdpElementFunctor<k_language>;
    m_NameToFunctor["left"] = LOMSE_NEW LdpElementFunctor<k_left>;
    m_NameToFunctor["legato-duro"] = LOMSE_NEW LdpElementFunctor<k_legato_duro>;
    m_NameToFunctor["lenmusdoc"] = LOMSE_NEW LdpElementFunctor<k_lenmusdoc>;
    m_NameToFunctor["link"] = LOMSE_NEW LdpElementFunctor<k_link>;
    m_NameToFunctor["line"] = LOMSE_NEW LdpElementFunctor<k_line>;
    m_NameToFunctor["lineCapEnd"] = LOMSE_NEW LdpElementFunctor<k_lineCapEnd>;
    m_NameToFunctor["lineCapStart"] = LOMSE_NEW LdpElementFunctor<k_lineCapStart>;
    m_NameToFunctor["lineStyle"] = LOMSE_NEW LdpElementFunctor<k_lineStyle>;
    m_NameToFunctor["lineThickness"] = LOMSE_NEW LdpElementFunctor<k_lineThickness>;
    m_NameToFunctor["line-height"] = LOMSE_NEW LdpElementFunctor<k_line_height>;
    m_NameToFunctor["listitem"] = LOMSE_NEW LdpElementFunctor<k_listitem>;
    m_NameToFunctor["lyric"] = LOMSE_NEW LdpElementFunctor<k_lyric>;
    m_NameToFunctor["marccato"] = LOMSE_NEW LdpElementFunctor<k_marccato>;
    m_NameToFunctor["marccato-legato"] = LOMSE_NEW LdpElementFunctor<k_marccato_legato>;
    m_NameToFunctor["marccato-staccato"] = LOMSE_NEW LdpElementFunctor<k_marccato_staccato>;
    m_NameToFunctor["marccato-staccatissimo"] = LOMSE_NEW LdpElementFunctor<k_marccato_staccatissimo>;
    m_NameToFunctor["margin"] = LOMSE_NEW LdpElementFunctor<k_margin>;
    m_NameToFunctor["margin-top"] = LOMSE_NEW LdpElementFunctor<k_margin_top>;
    m_NameToFunctor["margin-right"] = LOMSE_NEW LdpElementFunctor<k_margin_right>;
    m_NameToFunctor["margin-bottom"] = LOMSE_NEW LdpElementFunctor<k_margin_bottom>;
    m_NameToFunctor["margin-left"] = LOMSE_NEW LdpElementFunctor<k_margin_left>;
    m_NameToFunctor["max-height"] = LOMSE_NEW LdpElementFunctor<k_max_height>;
    m_NameToFunctor["max-width"] = LOMSE_NEW LdpElementFunctor<k_max_width>;
    m_NameToFunctor["melisma"] = LOMSE_NEW LdpElementFunctor<k_melisma>;
    m_NameToFunctor["meta"] = LOMSE_NEW LdpElementFunctor<k_meta>;
    m_NameToFunctor["metronome"] = LOMSE_NEW LdpElementFunctor<k_metronome>;
    m_NameToFunctor["mezzo-staccato"] = LOMSE_NEW LdpElementFunctor<k_mezzo_staccato>;
    m_NameToFunctor["mezzo-staccatissimo"] = LOMSE_NEW LdpElementFunctor<k_mezzo_staccatissimo>;
    m_NameToFunctor["min-height"] = LOMSE_NEW LdpElementFunctor<k_min_height>;
    m_NameToFunctor["min-width"] = LOMSE_NEW LdpElementFunctor<k_min_width>;
    m_NameToFunctor["mm"] = LOMSE_NEW LdpElementFunctor<k_mm>;
    m_NameToFunctor["musicData"] = LOMSE_NEW LdpElementFunctor<k_musicData>;
    m_NameToFunctor["n"] = LOMSE_NEW LdpElementFunctor<k_note>;   //note
    m_NameToFunctor["na"] = LOMSE_NEW LdpElementFunctor<k_na>;    //note in chord
    m_NameToFunctor["name"] = LOMSE_NEW LdpElementFunctor<k_name>;
    m_NameToFunctor["newSystem"] = LOMSE_NEW LdpElementFunctor<k_newSystem>;
    m_NameToFunctor["no"] = LOMSE_NEW LdpElementFunctor<k_no>;
    m_NameToFunctor["normal"] = LOMSE_NEW LdpElementFunctor<k_normal>;
    m_NameToFunctor["opt"] = LOMSE_NEW LdpElementFunctor<k_opt>;
    m_NameToFunctor["orderedlist"] = LOMSE_NEW LdpElementFunctor<k_orderedlist>;
    m_NameToFunctor["padding"] = LOMSE_NEW LdpElementFunctor<k_padding>;
    m_NameToFunctor["padding-top"] = LOMSE_NEW LdpElementFunctor<k_padding_top>;
    m_NameToFunctor["padding-right"] = LOMSE_NEW LdpElementFunctor<k_padding_right>;
    m_NameToFunctor["padding-bottom"] = LOMSE_NEW LdpElementFunctor<k_padding_bottom>;
    m_NameToFunctor["padding-left"] = LOMSE_NEW LdpElementFunctor<k_padding_left>;
    m_NameToFunctor["pageLayout"] = LOMSE_NEW LdpElementFunctor<k_pageLayout>;
    m_NameToFunctor["pageMargins"] = LOMSE_NEW LdpElementFunctor<k_pageMargins>;
    m_NameToFunctor["pageSize"] = LOMSE_NEW LdpElementFunctor<k_pageSize>;
    m_NameToFunctor["para"] = LOMSE_NEW LdpElementFunctor<k_para>;
    m_NameToFunctor["param"] = LOMSE_NEW LdpElementFunctor<k_parameter>;
    m_NameToFunctor["parenthesis"] = LOMSE_NEW LdpElementFunctor<k_parenthesis>;
    m_NameToFunctor["parts"] = LOMSE_NEW LdpElementFunctor<k_parts>;
    m_NameToFunctor["pitch"] = LOMSE_NEW LdpElementFunctor<k_pitch>;
    m_NameToFunctor["playLabel"] = LOMSE_NEW LdpElementFunctor<k_playLabel>;
    m_NameToFunctor["plop"] = LOMSE_NEW LdpElementFunctor<k_plop>;
    m_NameToFunctor["portrait"] = LOMSE_NEW LdpElementFunctor<k_portrait>;
    m_NameToFunctor["r"] = LOMSE_NEW LdpElementFunctor<k_rest>;   //rest
    m_NameToFunctor["right"] = LOMSE_NEW LdpElementFunctor<k_right>;
    m_NameToFunctor["rowspan"] = LOMSE_NEW LdpElementFunctor<k_rowspan>;
    m_NameToFunctor["scoop"] = LOMSE_NEW LdpElementFunctor<k_scoop>;
    m_NameToFunctor["score"] = LOMSE_NEW LdpElementFunctor<k_score>;
    m_NameToFunctor["scorePlayer"] = LOMSE_NEW LdpElementFunctor<k_score_player>;
    m_NameToFunctor["settings"] = LOMSE_NEW LdpElementFunctor<k_settings>;
    m_NameToFunctor["size"] = LOMSE_NEW LdpElementFunctor<k_size>;
    m_NameToFunctor["slur"] = LOMSE_NEW LdpElementFunctor<k_slur>;
    m_NameToFunctor["spacer"] = LOMSE_NEW LdpElementFunctor<k_spacer>;
    m_NameToFunctor["split"] = LOMSE_NEW LdpElementFunctor<k_split>;
    m_NameToFunctor["staccato"] = LOMSE_NEW LdpElementFunctor<k_staccato>;
    m_NameToFunctor["staccato-duro"] = LOMSE_NEW LdpElementFunctor<k_staccato_duro>;
    m_NameToFunctor["staccatissimo"] = LOMSE_NEW LdpElementFunctor<k_staccatissimo>;
    m_NameToFunctor["staccatissimo-duro"] = LOMSE_NEW LdpElementFunctor<k_staccatissimo_duro>;
    m_NameToFunctor["staff"] = LOMSE_NEW LdpElementFunctor<k_staff>;
    m_NameToFunctor["staffDistance"] = LOMSE_NEW LdpElementFunctor<k_staffDistance>;
    m_NameToFunctor["staffLines"] = LOMSE_NEW LdpElementFunctor<k_staffLines>;
    m_NameToFunctor["staffNum"] = LOMSE_NEW LdpElementFunctor<k_staffNum>;
    m_NameToFunctor["staffSpacing"] = LOMSE_NEW LdpElementFunctor<k_staffSpacing>;
    m_NameToFunctor["staffType"] = LOMSE_NEW LdpElementFunctor<k_staffType>;
    m_NameToFunctor["start"] = LOMSE_NEW LdpElementFunctor<k_start>;
    m_NameToFunctor["start-x"] = LOMSE_NEW LdpElementFunctor<k_start_x>;
    m_NameToFunctor["start-y"] = LOMSE_NEW LdpElementFunctor<k_start_y>;
    m_NameToFunctor["startPoint"] = LOMSE_NEW LdpElementFunctor<k_startPoint>;
    m_NameToFunctor["staves"] = LOMSE_NEW LdpElementFunctor<k_staves>;
    m_NameToFunctor["stem"] = LOMSE_NEW LdpElementFunctor<k_stem>;
    m_NameToFunctor["stopLabel"] = LOMSE_NEW LdpElementFunctor<k_stopLabel>;
    m_NameToFunctor["stress"] = LOMSE_NEW LdpElementFunctor<k_stress>;
    m_NameToFunctor["style"] = LOMSE_NEW LdpElementFunctor<k_style>;
    m_NameToFunctor["styles"] = LOMSE_NEW LdpElementFunctor<k_styles>;
    m_NameToFunctor["syl"] = LOMSE_NEW LdpElementFunctor<k_syllable>;
    m_NameToFunctor["symbol"] = LOMSE_NEW LdpElementFunctor<k_symbol>;
    m_NameToFunctor["symbolSize"] = LOMSE_NEW LdpElementFunctor<k_symbolSize>;
    m_NameToFunctor["systemLayout"] = LOMSE_NEW LdpElementFunctor<k_systemLayout>;
    m_NameToFunctor["systemMargins"] = LOMSE_NEW LdpElementFunctor<k_systemMargins>;
    m_NameToFunctor["t"] = LOMSE_NEW LdpElementFunctor<k_tuplet>;
    m_NameToFunctor["table"] = LOMSE_NEW LdpElementFunctor<k_table>;
    m_NameToFunctor["table-col-width"] = LOMSE_NEW LdpElementFunctor<k_table_col_width>;
    m_NameToFunctor["tableCell"] = LOMSE_NEW LdpElementFunctor<k_table_cell>;
    m_NameToFunctor["tableColumn"] = LOMSE_NEW LdpElementFunctor<k_table_column>;
    m_NameToFunctor["tableBody"] = LOMSE_NEW LdpElementFunctor<k_table_body>;
    m_NameToFunctor["tableHead"] = LOMSE_NEW LdpElementFunctor<k_table_head>;
    m_NameToFunctor["tableRow"] = LOMSE_NEW LdpElementFunctor<k_table_row>;
    m_NameToFunctor["tenuto"] = LOMSE_NEW LdpElementFunctor<k_tenuto>;
    m_NameToFunctor["text"] = LOMSE_NEW LdpElementFunctor<k_text>;
    m_NameToFunctor["textbox"] = LOMSE_NEW LdpElementFunctor<k_textbox>;
    m_NameToFunctor["text-align"] = LOMSE_NEW LdpElementFunctor<k_text_align>;
    m_NameToFunctor["text-decoration"] = LOMSE_NEW LdpElementFunctor<k_text_decoration>;
    m_NameToFunctor["tie"] = LOMSE_NEW LdpElementFunctor<k_tie>;
    m_NameToFunctor["time"] = LOMSE_NEW LdpElementFunctor<k_time_signature>;
    m_NameToFunctor["title"] = LOMSE_NEW LdpElementFunctor<k_title>;
    m_NameToFunctor["tm"] = LOMSE_NEW LdpElementFunctor<k_time_modification>;
    m_NameToFunctor["txt"] = LOMSE_NEW LdpElementFunctor<k_txt>;
    m_NameToFunctor["undefined"] = LOMSE_NEW LdpElementFunctor<k_undefined>;
    m_NameToFunctor["undoData"] = LOMSE_NEW LdpElementFunctor<k_undoData>;
    m_NameToFunctor["unstress"] = LOMSE_NEW LdpElementFunctor<k_unstress>;
    m_NameToFunctor["up"] = LOMSE_NEW LdpElementFunctor<k_up>;
    m_NameToFunctor["url"] = LOMSE_NEW LdpElementFunctor<k_url>;
    m_NameToFunctor["value"] = LOMSE_NEW LdpElementFunctor<k_value>;
    m_NameToFunctor["vers"] = LOMSE_NEW LdpElementFunctor<k_vers>;
    m_NameToFunctor["vertical-align"] = LOMSE_NEW LdpElementFunctor<k_vertical_align>;
    m_NameToFunctor["visible"] = LOMSE_NEW LdpElementFunctor<k_visible>;
    m_NameToFunctor["voice"] = LOMSE_NEW LdpElementFunctor<k_voice>;
    m_NameToFunctor["width"] = LOMSE_NEW LdpElementFunctor<k_width>;
    m_NameToFunctor["yes"] = LOMSE_NEW LdpElementFunctor<k_yes>;

}

LdpFactory::~LdpFactory()
{
	map<std::string, LdpFunctor*>::const_iterator it;
    for (it = m_NameToFunctor.begin(); it != m_NameToFunctor.end(); ++it)
        delete it->second;

    m_NameToFunctor.clear();
    m_TypeToName.clear();
}

LdpElement* LdpFactory::create(const std::string& name, int numLine) const
{
	map<std::string, LdpFunctor*>::const_iterator it
        = m_NameToFunctor.find(name);
	if (it != m_NameToFunctor.end())
    {
		LdpFunctor* f = it->second;
		LdpElement* element = (*f)();
		element->set_name(name);
        element->set_num_line(numLine);
		return element;
	}
    else
    {
        LdpElement* element = create(k_undefined, numLine);
		//element->set_name(name);
        return element;
    }
}

LdpElement* LdpFactory::create(ELdpElement type, int numLine) const
{
	map<ELdpElement, std::string>::const_iterator it = m_TypeToName.find( type );
	if (it != m_TypeToName.end())
		return create(it->second, numLine);

    std::stringstream err;
    err << "[LdpFactory::create] invoked with unknown type \""
        << type << "\"" << endl;
    LOMSE_LOG_ERROR( err.str() );
    throw runtime_error( err.str() );
	return 0;
}

const std::string& LdpFactory::get_name(ELdpElement type) const
{
	map<ELdpElement, std::string>::const_iterator it = m_TypeToName.find( type );
	if (it != m_TypeToName.end())
		return it->second;
    else
    {
        LOMSE_LOG_ERROR("[LdpFactory::get_name]. Invalid type" );
        throw std::runtime_error("[LdpFactory::get_name]. Invalid type" );
    }
}

}   //namespace lomse

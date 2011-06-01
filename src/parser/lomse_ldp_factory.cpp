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

#ifdef VC6
# pragma warning (disable : 4786)
#endif

#include "lomse_ldp_factory.h"

#include <sstream>
#include <iostream>
#include "lomse_exceptions.h"

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
    m_TypeToName[k_anchorLine] = "anchorLine";
    m_TypeToName[k_barline] = "barline";
    m_TypeToName[k_beam] = "beam";
    m_TypeToName[k_below] = "below";
    m_TypeToName[k_bezier] = "bezier";
    m_TypeToName[k_bold] = "bold";
    m_TypeToName[k_bold_italic] = "bold-italic";
    m_TypeToName[k_border] = "border";
    m_TypeToName[k_brace] = "brace";
    m_TypeToName[k_bracket] = "bracket";
    m_TypeToName[k_bracketType] = "bracketType";
    m_TypeToName[k_center] = "center";
    m_TypeToName[k_chord] = "chord";
    m_TypeToName[k_clef] = "clef";
    m_TypeToName[k_color] = "color";
    m_TypeToName[k_content] = "content";
    m_TypeToName[k_creationMode] = "creationMode";
    m_TypeToName[k_ctrol1_x] = "ctrol1-x";
    m_TypeToName[k_ctrol1_y] = "ctrol1-y";
    m_TypeToName[k_ctrol2_x] = "ctrol2-x";
    m_TypeToName[k_ctrol2_y] = "ctrol2-y";
    m_TypeToName[k_cursor] = "cursor";
    m_TypeToName[k_defineStyle] = "defineStyle";
    m_TypeToName[k_down] = "down";
    m_TypeToName[k_displayBracket] = "displayBracket";
    m_TypeToName[k_displayNumber] = "displayNumber";
    m_TypeToName[k_duration] = "duration";
    m_TypeToName[k_dx] = "dx";
    m_TypeToName[k_dy] = "dy";
    m_TypeToName[k_dynamic] = "dynamic";
    m_TypeToName[k_end] = "end";
    m_TypeToName[k_end_x] = "end-x";
    m_TypeToName[k_end_y] = "end-y";
    m_TypeToName[k_endPoint] = "endPoint";
    m_TypeToName[k_fbline] = "fbline";
    m_TypeToName[k_fermata] = "fermata";
    m_TypeToName[k_figuredBass] = "figuredBass";
    m_TypeToName[k_font] = "font";
    m_TypeToName[k_goBack] = "goBack";
    m_TypeToName[k_goFwd] = "goFwd";
    m_TypeToName[k_graphic] = "graphic";
    m_TypeToName[k_group] = "group";
    m_TypeToName[k_hasWidth] = "hasWidth";
    m_TypeToName[k_heading] = "heading";
    m_TypeToName[k_height] = "height";
    m_TypeToName[k_infoMIDI] = "infoMIDI";
    m_TypeToName[k_instrument] = "instrument";
    m_TypeToName[k_italic] = "italic";
    m_TypeToName[k_joinBarlines] = "joinBarlines";
    m_TypeToName[k_key_signature] = "key";
    m_TypeToName[k_landscape] = "landscape";
    m_TypeToName[k_language] = "language";
    m_TypeToName[k_left] = "left";
    m_TypeToName[k_lenmusdoc] = "lenmusdoc";
    m_TypeToName[k_line] = "line";
    m_TypeToName[k_lineCapEnd] = "lineCapEnd";
    m_TypeToName[k_lineCapStart] = "lineCapStart";
    m_TypeToName[k_lineStyle] = "lineStyle";
    m_TypeToName[k_lineThickness] = "lineThickness";
    m_TypeToName[k_meta] = "meta";
    m_TypeToName[k_metronome] = "metronome";
    m_TypeToName[k_musicData] = "musicData";
    m_TypeToName[k_na] = "na";  //note in chord
    m_TypeToName[k_name] = "name";
    m_TypeToName[k_newSystem] = "newSystem";
    m_TypeToName[k_no] = "no";
    m_TypeToName[k_normal] = "normal";
    m_TypeToName[k_note] = "n";   //note
    m_TypeToName[k_opt] = "opt";
    m_TypeToName[k_pageLayout] = "pageLayout";
    m_TypeToName[k_pageMargins] = "pageMargins";
    m_TypeToName[k_pageSize] = "pageSize";
    m_TypeToName[k_para] = "para";
    m_TypeToName[k_parenthesis] = "parenthesis";
    m_TypeToName[k_pitch] = "pitch";
    m_TypeToName[k_portrait] = "portrait";
    m_TypeToName[k_rest] = "r";   //rest
    m_TypeToName[k_right] = "right";
    m_TypeToName[k_score] = "score";
    m_TypeToName[k_settings] = "settings";
    m_TypeToName[k_slur] = "slur";
    m_TypeToName[k_size] = "size";
    m_TypeToName[k_spacer] = "spacer";
    m_TypeToName[k_split] = "split";
    m_TypeToName[k_staff] = "staff";
    m_TypeToName[k_staffDistance] = "staffDistance";
    m_TypeToName[k_staffLines] = "staffLines";
    m_TypeToName[k_staffNum] = "staffNum";
    m_TypeToName[k_staffSpacing] = "staffSpacing";
    m_TypeToName[k_staffType] = "staffType";
    m_TypeToName[k_start_x] = "start-x";
    m_TypeToName[k_start_y] = "start-y";
    m_TypeToName[k_startPoint] = "startPoint";
    m_TypeToName[k_styles] = "styles";
    m_TypeToName[k_start] = "start";
    m_TypeToName[k_staves] = "staves";
    m_TypeToName[k_stem] = "stem";
    m_TypeToName[k_style] = "style";
    m_TypeToName[k_styles] = "styles";
    m_TypeToName[k_symbol] = "symbol";
    m_TypeToName[k_symbolSize] = "symbolSize";
    m_TypeToName[k_style] = "style";
    m_TypeToName[k_styles] = "styles";
    m_TypeToName[k_systemLayout] = "systemLayout";
    m_TypeToName[k_systemMargins] = "systemMargins";
    m_TypeToName[k_tuplet] = "t";
    m_TypeToName[k_text] = "text";
    m_TypeToName[k_textbox] = "textbox";
    m_TypeToName[k_tie] = "tie";
    m_TypeToName[k_time_signature] = "time";
    m_TypeToName[k_title] = "title";
    m_TypeToName[k_txt] = "txt";
    m_TypeToName[k_undefined] = "undefined";
    m_TypeToName[k_undoData] = "undoData";
    m_TypeToName[k_up] = "up";
    m_TypeToName[k_vers] = "vers";
    m_TypeToName[k_visible] = "visible";
    m_TypeToName[k_voice] = "voice";
    m_TypeToName[k_width] = "width";
    m_TypeToName[k_yes] = "yes";


    //Register all types
    m_NameToFunctor["label"] = new LdpElementFunctor<k_label>;
    m_NameToFunctor["number"] = new LdpElementFunctor<k_number>;
    m_NameToFunctor["string"] = new LdpElementFunctor<k_string>;

    m_NameToFunctor["abbrev"] = new LdpElementFunctor<k_abbrev>;
    m_NameToFunctor["above"] = new LdpElementFunctor<k_above>;
    m_NameToFunctor["anchorLine"] = new LdpElementFunctor<k_anchorLine>;
    m_NameToFunctor["barline"] = new LdpElementFunctor<k_barline>;
    m_NameToFunctor["beam"] = new LdpElementFunctor<k_beam>;
    m_NameToFunctor["below"] = new LdpElementFunctor<k_below>;
    m_NameToFunctor["bezier"] = new LdpElementFunctor<k_bezier>;
    m_NameToFunctor["bold"] = new LdpElementFunctor<k_bold>;
    m_NameToFunctor["bold_italic"] = new LdpElementFunctor<k_bold_italic>;
    m_NameToFunctor["border"] = new LdpElementFunctor<k_border>;
    m_NameToFunctor["brace"] = new LdpElementFunctor<k_brace>;
    m_NameToFunctor["bracket"] = new LdpElementFunctor<k_bracket>;
    m_NameToFunctor["bracketType"] = new LdpElementFunctor<k_bracketType>;
    m_NameToFunctor["center"] = new LdpElementFunctor<k_center>;
    m_NameToFunctor["chord"] = new LdpElementFunctor<k_chord>;
    m_NameToFunctor["clef"] = new LdpElementFunctor<k_clef>;
    m_NameToFunctor["color"] = new LdpElementFunctor<k_color>;
    m_NameToFunctor["content"] = new LdpElementFunctor<k_content>;
    m_NameToFunctor["creationMode"] = new LdpElementFunctor<k_creationMode>;
    m_NameToFunctor["ctrol1-x"] = new LdpElementFunctor<k_ctrol1_x>;
    m_NameToFunctor["ctrol1-y"] = new LdpElementFunctor<k_ctrol1_y>;
    m_NameToFunctor["ctrol2-x"] = new LdpElementFunctor<k_ctrol2_x>;
    m_NameToFunctor["ctrol2-y"] = new LdpElementFunctor<k_ctrol2_y>;
    m_NameToFunctor["cursor"] = new LdpElementFunctor<k_cursor>;
    m_NameToFunctor["defineStyle"] = new LdpElementFunctor<k_defineStyle>;
    m_NameToFunctor["down"] = new LdpElementFunctor<k_down>;
    m_NameToFunctor["duration"] = new LdpElementFunctor<k_duration>;
    m_NameToFunctor["displayBracket"] = new LdpElementFunctor<k_displayBracket>;
    m_NameToFunctor["displayNumber"] = new LdpElementFunctor<k_displayNumber>;
    m_NameToFunctor["dx"] = new LdpElementFunctor<k_dx>;
    m_NameToFunctor["dy"] = new LdpElementFunctor<k_dy>;
    m_NameToFunctor["dynamic"] = new LdpElementFunctor<k_dynamic>;
    m_NameToFunctor["end"] = new LdpElementFunctor<k_end>;
    m_NameToFunctor["end-x"] = new LdpElementFunctor<k_end_x>;
    m_NameToFunctor["end-y"] = new LdpElementFunctor<k_end_y>;
    m_NameToFunctor["endPoint"] = new LdpElementFunctor<k_endPoint>;
    m_NameToFunctor["fbline"] = new LdpElementFunctor<k_fbline>;
    m_NameToFunctor["fermata"] = new LdpElementFunctor<k_fermata>;
    m_NameToFunctor["figuredBass"] = new LdpElementFunctor<k_figuredBass>;
    m_NameToFunctor["font"] = new LdpElementFunctor<k_font>;
    m_NameToFunctor["goBack"] = new LdpElementFunctor<k_goBack>;
    m_NameToFunctor["goFwd"] = new LdpElementFunctor<k_goFwd>;
    m_NameToFunctor["graphic"] = new LdpElementFunctor<k_graphic>;
    m_NameToFunctor["group"] = new LdpElementFunctor<k_group>;
    m_NameToFunctor["hasWidth"] = new LdpElementFunctor<k_hasWidth>;
    m_NameToFunctor["heading"] = new LdpElementFunctor<k_heading>;
    m_NameToFunctor["height"] = new LdpElementFunctor<k_height>;
    m_NameToFunctor["infoMIDI"] = new LdpElementFunctor<k_infoMIDI>;
    m_NameToFunctor["instrument"] = new LdpElementFunctor<k_instrument>;
    m_NameToFunctor["italic"] = new LdpElementFunctor<k_italic>;
    m_NameToFunctor["joinBarlines"] = new LdpElementFunctor<k_joinBarlines>;
    m_NameToFunctor["key"] = new LdpElementFunctor<k_key_signature>;
    m_NameToFunctor["landscape"] = new LdpElementFunctor<k_landscape>;
    m_NameToFunctor["language"] = new LdpElementFunctor<k_language>;
    m_NameToFunctor["left"] = new LdpElementFunctor<k_left>;
    m_NameToFunctor["lenmusdoc"] = new LdpElementFunctor<k_lenmusdoc>;
    m_NameToFunctor["line"] = new LdpElementFunctor<k_line>;
    m_NameToFunctor["lineCapEnd"] = new LdpElementFunctor<k_lineCapEnd>;
    m_NameToFunctor["lineCapStart"] = new LdpElementFunctor<k_lineCapStart>;
    m_NameToFunctor["lineStyle"] = new LdpElementFunctor<k_lineStyle>;
    m_NameToFunctor["lineThickness"] = new LdpElementFunctor<k_lineThickness>;
    m_NameToFunctor["meta"] = new LdpElementFunctor<k_meta>;
    m_NameToFunctor["metronome"] = new LdpElementFunctor<k_metronome>;
    m_NameToFunctor["musicData"] = new LdpElementFunctor<k_musicData>;
    m_NameToFunctor["n"] = new LdpElementFunctor<k_note>;   //note
    m_NameToFunctor["na"] = new LdpElementFunctor<k_na>;    //note in chord
    m_NameToFunctor["name"] = new LdpElementFunctor<k_name>;
    m_NameToFunctor["newSystem"] = new LdpElementFunctor<k_newSystem>;
    m_NameToFunctor["no"] = new LdpElementFunctor<k_no>;
    m_NameToFunctor["normal"] = new LdpElementFunctor<k_normal>;
    m_NameToFunctor["opt"] = new LdpElementFunctor<k_opt>;
    m_NameToFunctor["pageLayout"] = new LdpElementFunctor<k_pageLayout>;
    m_NameToFunctor["pageMargins"] = new LdpElementFunctor<k_pageMargins>;
    m_NameToFunctor["pageSize"] = new LdpElementFunctor<k_pageSize>;
    m_NameToFunctor["para"] = new LdpElementFunctor<k_para>;
    m_NameToFunctor["parenthesis"] = new LdpElementFunctor<k_parenthesis>;
    m_NameToFunctor["pitch"] = new LdpElementFunctor<k_pitch>;
    m_NameToFunctor["portrait"] = new LdpElementFunctor<k_portrait>;
    m_NameToFunctor["r"] = new LdpElementFunctor<k_rest>;   //rest
    m_NameToFunctor["right"] = new LdpElementFunctor<k_right>;
    m_NameToFunctor["score"] = new LdpElementFunctor<k_score>;
    m_NameToFunctor["settings"] = new LdpElementFunctor<k_settings>;
    m_NameToFunctor["size"] = new LdpElementFunctor<k_size>;
    m_NameToFunctor["slur"] = new LdpElementFunctor<k_slur>;
    m_NameToFunctor["spacer"] = new LdpElementFunctor<k_spacer>;
    m_NameToFunctor["split"] = new LdpElementFunctor<k_split>;
    m_NameToFunctor["staff"] = new LdpElementFunctor<k_staff>;
    m_NameToFunctor["staffDistance"] = new LdpElementFunctor<k_staffDistance>;
    m_NameToFunctor["staffLines"] = new LdpElementFunctor<k_staffLines>;
    m_NameToFunctor["staffNum"] = new LdpElementFunctor<k_staffNum>;
    m_NameToFunctor["staffSpacing"] = new LdpElementFunctor<k_staffSpacing>;
    m_NameToFunctor["staffType"] = new LdpElementFunctor<k_staffType>;
    m_NameToFunctor["start"] = new LdpElementFunctor<k_start>;
    m_NameToFunctor["start-x"] = new LdpElementFunctor<k_start_x>;
    m_NameToFunctor["start-y"] = new LdpElementFunctor<k_start_y>;
    m_NameToFunctor["startPoint"] = new LdpElementFunctor<k_startPoint>;
    m_NameToFunctor["styles"] = new LdpElementFunctor<k_styles>;
    m_NameToFunctor["staves"] = new LdpElementFunctor<k_staves>;
    m_NameToFunctor["stem"] = new LdpElementFunctor<k_stem>;
    m_NameToFunctor["style"] = new LdpElementFunctor<k_style>;
    m_NameToFunctor["styles"] = new LdpElementFunctor<k_styles>;
    m_NameToFunctor["symbol"] = new LdpElementFunctor<k_symbol>;
    m_NameToFunctor["symbolSize"] = new LdpElementFunctor<k_symbolSize>;
    m_NameToFunctor["systemLayout"] = new LdpElementFunctor<k_systemLayout>;
    m_NameToFunctor["systemMargins"] = new LdpElementFunctor<k_systemMargins>;
    m_NameToFunctor["t"] = new LdpElementFunctor<k_tuplet>;
    m_NameToFunctor["text"] = new LdpElementFunctor<k_text>;
    m_NameToFunctor["textbox"] = new LdpElementFunctor<k_textbox>;
    m_NameToFunctor["tie"] = new LdpElementFunctor<k_tie>;
    m_NameToFunctor["time"] = new LdpElementFunctor<k_time_signature>;
    m_NameToFunctor["title"] = new LdpElementFunctor<k_title>;
    m_NameToFunctor["txt"] = new LdpElementFunctor<k_txt>;
    m_NameToFunctor["undefined"] = new LdpElementFunctor<k_undefined>;
    m_NameToFunctor["undoData"] = new LdpElementFunctor<k_undoData>;
    m_NameToFunctor["up"] = new LdpElementFunctor<k_up>;
    m_NameToFunctor["vers"] = new LdpElementFunctor<k_vers>;
    m_NameToFunctor["visible"] = new LdpElementFunctor<k_visible>;
    m_NameToFunctor["voice"] = new LdpElementFunctor<k_voice>;
    m_NameToFunctor["width"] = new LdpElementFunctor<k_width>;
    m_NameToFunctor["yes"] = new LdpElementFunctor<k_yes>;

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
    err << "LdpFactory::create called with unknown type \""
        << type << "\"" << endl;
    throw std::runtime_error( err.str() );
	return 0;
}

const std::string& LdpFactory::get_name(ELdpElement type) const
{
	map<ELdpElement, std::string>::const_iterator it = m_TypeToName.find( type );
	if (it != m_TypeToName.end())
		return it->second;
    else
        throw std::runtime_error( "[LdpFactory::get_name]. Invalid type" );
}

}   //namespace lomse

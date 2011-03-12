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

#include "lomse_internal_model.h"

#include <algorithm>
#include <math.h>                   //pow
#include "lomse_staffobjs_table.h"
#include "lomse_im_note.h"
#include "lomse_basic_objects.h"

using namespace std;

namespace lomse
{


//---------------------------------------------------------------------------------------
// ImoObj implementation
//---------------------------------------------------------------------------------------

ImoObj::ImoObj(int objtype, DtoObj& dto)
    : m_id( dto.get_id() )
    , m_objtype( objtype )
{
}

//---------------------------------------------------------------------------------------
ImoObj::~ImoObj()
{
    NodeInTree<ImoObj>::children_iterator it(this);
    it = begin();
    while (it != end())
    {
        ImoObj* child = *it;
        ++it;
	    delete child;
    }
}

//---------------------------------------------------------------------------------------
void ImoObj::accept_in(BaseVisitor& v)
{
    Visitor<ImoObj>* p = dynamic_cast<Visitor<ImoObj>*>(&v);
    if (p)
    {
        p->start_visit(this);
    }
}

//---------------------------------------------------------------------------------------
void ImoObj::accept_out(BaseVisitor& v)
{
    Visitor<ImoObj>* p = dynamic_cast<Visitor<ImoObj>*>(&v);
    if (p)
    {
        p->start_visit(this);
    }
}

//---------------------------------------------------------------------------------------
ImoObj* ImoObj::get_child_of_type(int objtype)
{
    for (int i=0; i < get_num_children(); i++)
    {
        ImoObj* pChild = get_child(i);
        if (pChild->get_obj_type() == objtype)
            return pChild;
    }
    return NULL;
}


//---------------------------------------------------------------------------------------
// ImoBinaryRelObj implementation
//---------------------------------------------------------------------------------------

//ImoBinaryRelObj::ImoBinaryRelObj(ImoDocObj* pOwner, long id, int type,
//                               ImoStaffObj* pStartSO, ImoStaffObj* pEndSO)
//    : ImoRelObj(pOwner, id, type)
//    , m_pStartSO(pStartSO)
//    , m_pEndSO(pEndSO)
//{
//}
//
ImoBinaryRelObj::~ImoBinaryRelObj()
{
}

//---------------------------------------------------------------------------------------
//void ImoBinaryRelObj::remove(ImoStaffObj* pSO)
//{
//    if (m_pStartSO == pSO)
//        m_pStartSO = (ImoStaffObj*)NULL;
//    else if (m_pEndSO == pSO)
//        m_pEndSO = (ImoStaffObj*)NULL;
//}



//---------------------------------------------------------------------------------------
// ImoMultiRelObj implementation
//---------------------------------------------------------------------------------------

//ImoMultiRelObj::ImoMultiRelObj(ImoDocObj* pOwner, long id, int type)
//    : ImoRelObj(pOwner, id, type)
//{
//}

//---------------------------------------------------------------------------------------
ImoMultiRelObj::~ImoMultiRelObj()
{
    m_relatedObjects.clear();
}

//---------------------------------------------------------------------------------------
//void ImoMultiRelObj::remove(ImoStaffObj* pSO)
//{
//    //remove StaffObj.
//	//AWARE: This method is always invoked by a SO. Therefore it will
//	//not inform back the SO, as this is unnecessary and causes problems when
//	//deleting the relationship object
//
//    std::list<ImoStaffObj*>::iterator it;
//    it = find(m_relatedObjects.begin(), m_relatedObjects.end(), pSO);
//    m_relatedObjects.erase(it);
//    on_relationship_modified();
//}

//---------------------------------------------------------------------------------------
void ImoMultiRelObj::push_back(ImoStaffObj* pSO)
{
    m_relatedObjects.push_back(pSO);
    //on_relationship_modified();
}

//---------------------------------------------------------------------------------------
//void ImoMultiRelObj::include(ImoStaffObj* pSO, int index)
//{
//    // Add a note to the relation. index is the position that the added StaffObj
//    // must occupy (0..n). If index == -1, StaffObj will be added at the end.
//
//	//add the StaffObj
//	if (index == -1 || index == get_num_objects())
//		m_relatedObjects.push_back(pSO);
//	else
//	{
//		int iN;
//		std::list<ImoStaffObj*>::iterator it;
//		for(iN=0, it=m_relatedObjects.begin(); it != m_relatedObjects.end(); ++it, iN++)
//		{
//			if (iN == index)
//			{
//				//insert before current item
//				m_relatedObjects.insert(it, pSO);
//				break;
//			}
//		}
//	}
//
//    on_relationship_modified();
//}
//
//---------------------------------------------------------------------------------------
//int ImoMultiRelObj::get_object_index(ImoStaffObj* pSO)
//{
//	//returns the position in the StaffObjs list (0..n)
//
//	int iN;
//    std::list<ImoStaffObj*>::iterator it;
//    for(iN=0, it=m_relatedObjects.begin(); it != m_relatedObjects.end(); ++it, iN++)
//	{
//		if (pSO == *it)
//            return iN;
//	}
//	return 0;			//compiler happy
//}



//---------------------------------------------------------------------------------------
// ImoComponentObj implementation
//---------------------------------------------------------------------------------------

ImoComponentObj::ImoComponentObj(int objtype, DtoComponentObj& dto)
    : ImoDocObj(objtype, dto)
    , m_fVisible( dto.is_visible() )
    , m_color( dto.get_color() )
{
}

//---------------------------------------------------------------------------------------
void ImoComponentObj::set_color(Color color)
{
    m_color = color;
}


//---------------------------------------------------------------------------------------
// ImoStaffObj implementation
//---------------------------------------------------------------------------------------

ImoStaffObj::ImoStaffObj(int objtype, DtoStaffObj& dto)
    : ImoComponentObj(objtype, dto)
    , m_staff( dto.get_staff() )
{
}


//---------------------------------------------------------------------------------------
// ImoAuxObj implementation
//---------------------------------------------------------------------------------------

ImoAuxObj::ImoAuxObj(int objtype, DtoAuxObj& dto)
    : ImoComponentObj(objtype, dto)
{
}


//---------------------------------------------------------------------------------------
// ImoBarline implementation
//---------------------------------------------------------------------------------------

ImoBarline::ImoBarline(DtoBarline& dto, long id)
    : ImoStaffObj(ImoObj::k_barline, dto)
    , m_type( dto.get_barline_type() )
{
    set_id(id);
}



//=======================================================================================
// ImoBeamInfo implementation
//=======================================================================================
ImoBeamInfo::ImoBeamInfo()
    : ImoSimpleObj(ImoObj::k_beam_info)
    , m_beamNum(0)
    , m_pBeamElm(NULL)
    , m_pNR(NULL)
{
    for (int level=0; level < 6; level++)
    {
        m_beamType[level] = ImoBeam::k_none;
        m_repeat[level] = false;
    }
}

//---------------------------------------------------------------------------------------
ImoBeamInfo::ImoBeamInfo(LdpElement* pBeamElm)
    : ImoSimpleObj(ImoObj::k_beam_info)
    , m_beamNum(0)
    , m_pBeamElm(pBeamElm)
    , m_pNR(NULL)
{
    for (int level=0; level < 6; level++)
    {
        m_beamType[level] = ImoBeam::k_none;
        m_repeat[level] = false;
    }
}

//---------------------------------------------------------------------------------------
int ImoBeamInfo::get_line_number()
{
    if (m_pBeamElm)
        return m_pBeamElm->get_line_number();
    else
        return 0;
}

//---------------------------------------------------------------------------------------
void ImoBeamInfo::set_beam_type(int level, int type)
{
    m_beamType[level] = type;
}

//---------------------------------------------------------------------------------------
int ImoBeamInfo::get_beam_type(int level)
{
    return m_beamType[level];
}

//---------------------------------------------------------------------------------------
bool ImoBeamInfo::is_end_of_beam()
{
    for (int level=0; level < 6; level++)
    {
        if (m_beamType[level] == ImoBeam::k_begin
            || m_beamType[level] == ImoBeam::k_forward
            || m_beamType[level] == ImoBeam::k_continue
            )
            return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------
void ImoBeamInfo::set_repeat(int level, bool value)
{
    m_repeat[level] = value;
}

//---------------------------------------------------------------------------------------
bool ImoBeamInfo::get_repeat(int level)
{
    return m_repeat[level];
}


//=======================================================================================
// ImoBeamInfo implementation
//=======================================================================================
ImoBezierInfo::ImoBezierInfo(ImoBezierInfo* pBezier)
    : ImoSimpleObj(ImoObj::k_bezier_info)
{
    if (pBezier)
    {
        for (int i=0; i < 4; ++i)
            m_tPoints[i] = pBezier->get_point(i);
    }
}


//=======================================================================================
// ImoClef implementation
//=======================================================================================
ImoClef::ImoClef(DtoClef& dto)
    : ImoStaffObj(ImoObj::k_clef, dto)
    , m_clefType( dto.get_clef_type() )
    , m_symbolSize( dto.get_symbol_size() )
{
}

//---------------------------------------------------------------------------------------
ImoClef::ImoClef(int clefType)
    : ImoStaffObj(ImoObj::k_clef)
    , m_clefType(clefType)
    , m_symbolSize(k_size_default)
{
}



//---------------------------------------------------------------------------------------
// ImoColorDto implementation
//---------------------------------------------------------------------------------------

ImoColorDto::ImoColorDto(Int8u r, Int8u g, Int8u b, Int8u a)
    : ImoSimpleObj(ImoObj::k_color_info)
    , m_color(r, g, b, a)
    , m_ok(true)
{
}

//---------------------------------------------------------------------------------------
Int8u ImoColorDto::convert_from_hex(const std::string& hex)
{
    int value = 0;

    int a = 0;
    int b = static_cast<int>(hex.length()) - 1;
    for (; b >= 0; a++, b--)
    {
        if (hex[b] >= '0' && hex[b] <= '9')
        {
            value += (hex[b] - '0') * (1 << (a * 4));
        }
        else
        {
            switch (hex[b])
            {
                case 'A':
                case 'a':
                    value += 10 * (1 << (a * 4));
                    break;

                case 'B':
                case 'b':
                    value += 11 * (1 << (a * 4));
                    break;

                case 'C':
                case 'c':
                    value += 12 * (1 << (a * 4));
                    break;

                case 'D':
                case 'd':
                    value += 13 * (1 << (a * 4));
                    break;

                case 'E':
                case 'e':
                    value += 14 * (1 << (a * 4));
                    break;

                case 'F':
                case 'f':
                    value += 15 * (1 << (a * 4));
                    break;

                default:
                    m_ok = false;
                    //cout << "Error: invalid character '" << hex[b] << "' in hex number" << endl;
                    break;
            }
        }
    }

    return static_cast<Int8u>(value);
}

//---------------------------------------------------------------------------------------
Color& ImoColorDto::get_from_rgb_string(const std::string& rgb)
{
    m_ok = true;

    if (rgb[0] == '#')
    {
        m_color.r = convert_from_hex( rgb.substr(1, 2) );
        m_color.g = convert_from_hex( rgb.substr(3, 2) );
        m_color.b = convert_from_hex( rgb.substr(5, 2) );
        m_color.a = 255;
    }

    if (!m_ok)
        m_color = Color(0,0,0,255);

    return m_color;
}

//---------------------------------------------------------------------------------------
Color& ImoColorDto::get_from_rgba_string(const std::string& rgba)
{
    m_ok = true;

    if (rgba[0] == '#')
    {
        m_color.r = convert_from_hex( rgba.substr(1, 2) );
        m_color.g = convert_from_hex( rgba.substr(3, 2) );
        m_color.b = convert_from_hex( rgba.substr(5, 2) );
        m_color.a = convert_from_hex( rgba.substr(7, 2) );
    }

    if (!m_ok)
        m_color = Color(0,0,0,255);

    return m_color;
}

//---------------------------------------------------------------------------------------
Color& ImoColorDto::get_from_string(const std::string& hex)
{
    if (hex.length() == 7)
        return get_from_rgb_string(hex);
    else if (hex.length() == 9)
        return get_from_rgba_string(hex);
    else
    {
        m_ok = false;
        m_color = Color(0,0,0,255);
        return m_color;
    }
}


//---------------------------------------------------------------------------------------
// ImoAttachments implementation
//---------------------------------------------------------------------------------------
ImoAttachments::~ImoAttachments()
{
    std::list<ImoAuxObj*>::iterator it;
    for(it = m_attachments.begin(); it != m_attachments.end(); ++it)
        delete *it;
    m_attachments.clear();
}

//---------------------------------------------------------------------------------------
ImoAuxObj* ImoAttachments::get_item(int iItem)
{
    std::list<ImoAuxObj*>::iterator it;
    for(it = m_attachments.begin(); it != m_attachments.end() && iItem > 0; ++it, --iItem);
    if (it != m_attachments.end())
        return *it;
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
void ImoAttachments::remove(ImoAuxObj* pAO)
{
    m_attachments.remove(pAO);
}


//---------------------------------------------------------------------------------------
// ImoDocObj implementation
//---------------------------------------------------------------------------------------

ImoDocObj::ImoDocObj(int objtype)
    : ImoObj(objtype)
    , m_txUserLocation(0.0f)
    , m_tyUserLocation(0.0f)
{
}

//---------------------------------------------------------------------------------------
ImoDocObj::ImoDocObj(long id, int objtype)
    : ImoObj(id, objtype)
    , m_txUserLocation(0.0f)
    , m_tyUserLocation(0.0f)
{
}

//---------------------------------------------------------------------------------------
ImoDocObj::ImoDocObj(int objtype, DtoDocObj& dto)
    : ImoObj(objtype, dto)
    , m_txUserLocation( dto.get_user_location_x() )
    , m_tyUserLocation( dto.get_user_location_y() )
{
}

//---------------------------------------------------------------------------------------
ImoDocObj::~ImoDocObj()
{
}

//---------------------------------------------------------------------------------------
void ImoDocObj::attach(ImoAuxObj* pAO)
{
    ImoAttachments* pAttachments = get_attachments();
    if (!pAttachments)
    {
        pAttachments = new ImoAttachments();
        append_child(pAttachments);
    }
    pAttachments->add(pAO);
}

//---------------------------------------------------------------------------------------
ImoAuxObj* ImoDocObj::get_attachment(int i)
{
    ImoAttachments* pAttachments = get_attachments();
    return pAttachments->get_item(i);
}

//---------------------------------------------------------------------------------------
bool ImoDocObj::has_attachments()
{
    ImoAttachments* pAttachments = get_attachments();
    if (pAttachments)
        return pAttachments->get_num_items() > 0;
    else
        return false;
}

//---------------------------------------------------------------------------------------
int ImoDocObj::get_num_attachments()
{
    ImoAttachments* pAttachments = get_attachments();
    if (pAttachments)
        return pAttachments->get_num_items();
    else
        return 0;
}

//---------------------------------------------------------------------------------------
ImoAttachments* ImoDocObj::get_attachments()
{
    return dynamic_cast<ImoAttachments*>( get_child_of_type(ImoObj::k_attachments) );
}

//---------------------------------------------------------------------------------------
void ImoDocObj::remove_attachment(ImoAuxObj* pAO)
{
    ImoAttachments* pAttachments = get_attachments();
    pAttachments->remove(pAO);
}


//---------------------------------------------------------------------------------------
// ImoDocument implementation
//---------------------------------------------------------------------------------------

ImoDocument::ImoDocument(const std::string& version)
    : ImoContainerObj(ImoObj::k_document)
    , m_version(version)
    , m_pageInfo()
{
}

//---------------------------------------------------------------------------------------
ImoDocument::~ImoDocument()
{
}

//---------------------------------------------------------------------------------------
int ImoDocument::get_num_content_items()
{
    if (has_children())
    {
        ImoObj* pChild = get_first_child();
        int num = pChild->get_num_children();
        return num;
    }
    else
        return 0;
}

//---------------------------------------------------------------------------------------
ImoDocObj* ImoDocument::get_content_item(int iItem)
{
    ImoContent* pContent = get_content();
    if (iItem < pContent->get_num_children())
        return dynamic_cast<ImoDocObj*>( pContent->get_child(iItem) );
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
ImoContent* ImoDocument::get_content()
{
    return dynamic_cast<ImoContent*>( get_first_child() );
}

//---------------------------------------------------------------------------------------
void ImoDocument::add_page_info(ImoPageInfo* pPI)
{
    m_pageInfo = *pPI;
}


//---------------------------------------------------------------------------------------
// ImoFermata implementation
//---------------------------------------------------------------------------------------

ImoFermata::ImoFermata(DtoFermata& dto)
    : ImoAuxObj(ImoObj::k_fermata, dto)
    , m_placement( dto.get_placement() )
    , m_symbol( dto.get_symbol() )
{
}


//---------------------------------------------------------------------------------------
// ImoGoBackFwd implementation
//---------------------------------------------------------------------------------------

ImoGoBackFwd::ImoGoBackFwd(DtoGoBackFwd& dto)
    : ImoStaffObj(ImoObj::k_go_back_fwd, dto)
    , m_fFwd( dto.is_forward() )
    , m_rTimeShift( dto.get_time_shift() )
    , SHIFT_START_END(100000000.0f)
{
}


//---------------------------------------------------------------------------------------
// ImoInstrument implementation
//---------------------------------------------------------------------------------------
ImoInstrument::ImoInstrument()
    : ImoContainerObj(ImoObj::k_instrument)
    , m_name("")
    , m_abbrev("")
    , m_midi()
    , m_pGroup(NULL)
{
    add_staff();
//	m_midiChannel = g_pMidi->DefaultVoiceChannel();
//	m_midiInstr = g_pMidi->DefaultVoiceInstr();
}

//---------------------------------------------------------------------------------------
ImoInstrument::~ImoInstrument()
{
    std::list<ImoStaffInfo*>::iterator it;
    for (it = m_staves.begin(); it != m_staves.end(); ++it)
        delete *it;
    m_staves.clear();
}

//---------------------------------------------------------------------------------------
void ImoInstrument::add_staff()
{
    ImoStaffInfo* pStaff = new ImoStaffInfo();
    m_staves.push_back(pStaff);
}

//---------------------------------------------------------------------------------------
void ImoInstrument::replace_staff_info(ImoStaffInfo* pInfo)
{
    int iStaff = pInfo->get_staff_number();
    std::list<ImoStaffInfo*>::iterator it = m_staves.begin();
    for (; it != m_staves.end() && iStaff > 0; ++it, --iStaff);

    if (it != m_staves.end())
    {
        ImoStaffInfo* pOld = *it;
        it = m_staves.erase(it);
        delete pOld;
        m_staves.insert(it, new ImoStaffInfo(*pInfo));
    }
    delete pInfo;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_name(ImoScoreText* pText)
{
    m_name = *pText;
    delete pText;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_abbrev(ImoScoreText* pText)
{
    m_abbrev = *pText;
    delete pText;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_midi_info(ImoMidiInfo* pInfo)
{
    m_midi = *pInfo;
    delete pInfo;
}

//---------------------------------------------------------------------------------------
ImoMusicData* ImoInstrument::get_musicdata()
{
    return dynamic_cast<ImoMusicData*>( get_child_of_type(ImoObj::k_music_data) );
}

//---------------------------------------------------------------------------------------
ImoStaffInfo* ImoInstrument::get_staff(int iStaff)
{
    std::list<ImoStaffInfo*>::iterator it = m_staves.begin();
    for (; it != m_staves.end() && iStaff > 0; ++it, --iStaff);
    return *it;
}

//---------------------------------------------------------------------------------------
LUnits ImoInstrument::get_line_spacing_for_staff(int iStaff)
{
    return get_staff(iStaff)->get_line_spacing();
}

//---------------------------------------------------------------------------------------
// ImoInstrGroup implementation
//---------------------------------------------------------------------------------------

ImoInstrGroup::ImoInstrGroup()
    : ImoSimpleObj(ImoObj::k_instr_group)
    , m_fJoinBarlines(true)
    , m_symbol(k_brace)
    , m_name("")
    , m_abbrev("")
{
}

//---------------------------------------------------------------------------------------
ImoInstrGroup::~ImoInstrGroup()
{
    //AWARE: instruments MUST NOT be deleted. Thay are nodes in the tree and
    //will be deleted when deleting the tree. Here, in the ImoGroup, we just
    //keep pointers to locate them

    m_instruments.clear();
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::set_name(ImoScoreText* pText)
{
    m_name = *pText;
    delete pText;
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::set_abbrev(ImoScoreText* pText)
{
    m_abbrev = *pText;
    delete pText;
}

//---------------------------------------------------------------------------------------
ImoInstrument* ImoInstrGroup::get_instrument(int iInstr)    //iInstr = 0..n-1
{
    std::list<ImoInstrument*>::iterator it;
    int i = 0;
    for (it = m_instruments.begin(); it != m_instruments.end() && i < iInstr; ++it, ++i);
    if (i == iInstr)
        return *it;
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::add_instrument(ImoInstrument* pInstr)
{
    m_instruments.push_back(pInstr);
    pInstr->set_in_group(this);
}

//---------------------------------------------------------------------------------------
int ImoInstrGroup::get_num_instruments()
{
    return static_cast<int>( m_instruments.size() );
}



//---------------------------------------------------------------------------------------
// ImoKeySignature implementation
//---------------------------------------------------------------------------------------

ImoKeySignature::ImoKeySignature(DtoKeySignature& dto)
    : ImoStaffObj(ImoObj::k_key_signature, dto)
    , m_keyType( dto.get_key_type() )
{
}

//---------------------------------------------------------------------------------------
// ImoLine implementation
//---------------------------------------------------------------------------------------

ImoLine::ImoLine(ImoLineInfo& info)
    : ImoAuxObj(ImoObj::k_line)
    , m_lineInfo(info)
{
}

//---------------------------------------------------------------------------------------
// ImoMetronomeMark implementation
//---------------------------------------------------------------------------------------

ImoMetronomeMark::ImoMetronomeMark(DtoMetronomeMark& dto)
    : ImoStaffObj(ImoObj::k_metronome_mark, dto)
    , m_markType( dto.get_mark_type() )
    , m_ticksPerMinute( dto.get_ticks_per_minute() )
    , m_leftNoteType( dto.get_left_note_type() )
    , m_leftDots( dto.get_left_dots() )
    , m_rightNoteType( dto.get_right_note_type() )
    , m_rightDots( dto.get_right_dots() )
    , m_fParenthesis( dto.has_parenthesis() )
{
}

//---------------------------------------------------------------------------------------
// ImoMidiInfo implementation
//---------------------------------------------------------------------------------------

ImoMidiInfo::ImoMidiInfo()
    : ImoSimpleObj(ImoObj::k_midi_info)
    , m_instr(0)
    , m_channel(0)
{
}

//---------------------------------------------------------------------------------------
ImoMidiInfo::ImoMidiInfo(ImoMidiInfo& dto)
    : ImoSimpleObj(ImoObj::k_midi_info)
    , m_instr( dto.get_instrument() )
    , m_channel( dto.get_channel() )
{
}

//---------------------------------------------------------------------------------------
// ImoScore implementation
//---------------------------------------------------------------------------------------

//tables with default values for options
typedef struct
{
    string  sOptName;
    bool    fBoolValue;
}
BoolOption;

typedef struct
{
    string  sOptName;
    string  sStringValue;
}
StringOption;

typedef struct
{
    string  sOptName;
    float   rFloatValue;
}
FloatOption;

typedef struct
{
    string  sOptName;
    long    nLongValue;
}
LongOption;

static BoolOption m_BoolOptions[] =
{
    {"Score.FillPageWithEmptyStaves", false },
    {"StaffLines.StopAtFinalBarline", true },
    {"Score.JustifyFinalBarline", false },
    {"StaffLines.Hide", false },
    {"Staff.DrawLeftBarline", true },
};

//static StringOption m_StringOptions[] = {};

static FloatOption m_FloatOptions[] =
{
    {"Render.SpacingFactor", 0.547f },
        // Note spacing is proportional to duration.
        // As the duration of quarter note is 64 (duration units), I am
        // going to map it to 35 tenths. This gives a conversion factor
        // of 35/64 = 0.547
};

static LongOption m_LongOptions[] =
{
    {"Staff.UpperLegerLines.Displacement", 0L },
    {"Render.SpacingMethod", long(k_spacing_proportional) },
    {"Render.SpacingValue", 15L },       // 15 tenths (1.5 lines)
};


ImoScore::ImoScore()
    : ImoContainerObj(ImoObj::k_score)
    , m_version("")
    , m_pColStaffObjs(NULL)
    , m_systemInfoFirst()
    , m_systemInfoOther()
    , m_pageInfo()
{
    append_child( new ImoOptions() );
    append_child( new ImoInstruments() );
    set_defaults_for_system_info();
    set_defaults_for_options();
}

//---------------------------------------------------------------------------------------
ImoScore::~ImoScore()
{
    delete_staffobjs_collection();
    delete_text_styles();
}

//---------------------------------------------------------------------------------------
void ImoScore::set_defaults_for_system_info()
{
    m_systemInfoFirst.set_first(true);
    m_systemInfoFirst.set_system_distance(0.0f);
    m_systemInfoFirst.set_top_system_distance(0.0f);

    m_systemInfoOther.set_first(true);
    m_systemInfoOther.set_system_distance(3000.0f);         //3 cm
    m_systemInfoOther.set_top_system_distance(2000.0f);     //2 cm
}

//---------------------------------------------------------------------------------------
void ImoScore::set_defaults_for_options()
{
    //bool
    for (unsigned i=0; i < sizeof(m_BoolOptions)/sizeof(BoolOption); i++)
    {
        ImoOptionInfo* pOpt = new ImoOptionInfo(m_BoolOptions[i].sOptName);
        pOpt->set_type(ImoOptionInfo::k_boolean);
        pOpt->set_bool_value( m_BoolOptions[i].fBoolValue );
        add_option(pOpt);
    }

    //long
    for (unsigned i=0; i < sizeof(m_LongOptions)/sizeof(LongOption); i++)
    {
        ImoOptionInfo* pOpt = new ImoOptionInfo(m_LongOptions[i].sOptName);
        pOpt->set_type(ImoOptionInfo::k_number_long);
        pOpt->set_long_value( m_LongOptions[i].nLongValue );
        add_option(pOpt);
    }

    //double
    for (unsigned i=0; i < sizeof(m_FloatOptions)/sizeof(FloatOption); i++)
    {
        ImoOptionInfo* pOpt = new ImoOptionInfo(m_FloatOptions[i].sOptName);
        pOpt->set_type(ImoOptionInfo::k_number_float);
        pOpt->set_float_value( m_FloatOptions[i].rFloatValue );
        add_option(pOpt);
    }
}

//---------------------------------------------------------------------------------------
void ImoScore::delete_staffobjs_collection()
{
    if (m_pColStaffObjs)
        delete m_pColStaffObjs;
}

//---------------------------------------------------------------------------------------
void ImoScore::delete_text_styles()
{
    map<std::string, ImoTextStyleInfo*>::const_iterator it;
    for (it = m_nameToStyle.begin(); it != m_nameToStyle.end(); ++it)
        delete it->second;

    m_nameToStyle.clear();
}

//---------------------------------------------------------------------------------------
void ImoScore::set_float_option(const std::string& name, float value)
{
     ImoOptionInfo* pOpt = get_option(name);
     if (pOpt)
     {
         pOpt->set_float_value(value);
     }
     else
     {
        pOpt = new ImoOptionInfo(name);
        pOpt->set_type(ImoOptionInfo::k_number_float);
        pOpt->set_float_value(value);
        add_option(pOpt);
     }
}

//---------------------------------------------------------------------------------------
void ImoScore::set_bool_option(const std::string& name, bool value)
{
     ImoOptionInfo* pOpt = get_option(name);
     if (pOpt)
     {
         pOpt->set_bool_value(value);
     }
     else
     {
        pOpt = new ImoOptionInfo(name);
        pOpt->set_type(ImoOptionInfo::k_boolean);
        pOpt->set_bool_value(value);
        add_option(pOpt);
     }
}

//---------------------------------------------------------------------------------------
void ImoScore::set_long_option(const std::string& name, long value)
{
     ImoOptionInfo* pOpt = get_option(name);
     if (pOpt)
     {
         pOpt->set_long_value(value);
     }
     else
     {
        pOpt = new ImoOptionInfo(name);
        pOpt->set_type(ImoOptionInfo::k_number_long);
        pOpt->set_long_value(value);
        add_option(pOpt);
     }
}

//---------------------------------------------------------------------------------------
ImoInstruments* ImoScore::get_instruments()
{
    return dynamic_cast<ImoInstruments*>( get_child_of_type(ImoObj::k_instruments) );
}

//---------------------------------------------------------------------------------------
ImoInstrument* ImoScore::get_instrument(int iInstr)    //iInstr = 0..n-1
{
    ImoInstruments* pColInstr = get_instruments();
    return dynamic_cast<ImoInstrument*>( pColInstr->get_child(iInstr) );
}

//---------------------------------------------------------------------------------------
void ImoScore::add_instrument(ImoInstrument* pInstr)
{
    ImoInstruments* pColInstr = get_instruments();
    return pColInstr->append_child(pInstr);
}

//---------------------------------------------------------------------------------------
int ImoScore::get_num_instruments()
{
    ImoInstruments* pColInstr = get_instruments();
    return pColInstr->get_num_children();
}

//---------------------------------------------------------------------------------------
ImoOptionInfo* ImoScore::get_option(const std::string& name)
{
    ImoOptions* pColOpts = get_options();
    ImoObj::children_iterator it;
    for (it= pColOpts->begin(); it != pColOpts->end(); ++it)
    {
        ImoOptionInfo* pOpt = dynamic_cast<ImoOptionInfo*>(*it);
        if (pOpt->get_name() == name)
            return pOpt;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
ImoOptions* ImoScore::get_options()
{
    return dynamic_cast<ImoOptions*>( get_child_of_type(ImoObj::k_options) );
}

//---------------------------------------------------------------------------------------
void ImoScore::add_option(ImoOptionInfo* pOpt)
{
    ImoOptions* pColOpts = get_options();
    return pColOpts->append_child(pOpt);
}

//---------------------------------------------------------------------------------------
void ImoScore::set_option(ImoOptionInfo* pOpt)
{
    if (pOpt->is_bool_option())
        set_bool_option(pOpt->get_name(), pOpt->get_bool_value());
    else if (pOpt->is_long_option())
        set_long_option(pOpt->get_name(), pOpt->get_long_value());
    else if (pOpt->is_float_option())
        set_float_option(pOpt->get_name(), pOpt->get_float_value());
}

//---------------------------------------------------------------------------------------
bool ImoScore::has_options()
{
    ImoOptions* pColOpts = get_options();
    return pColOpts->get_num_children() > 0;
}

//---------------------------------------------------------------------------------------
void ImoScore::add_sytem_info(ImoSystemInfo* pSL)
{
    if (pSL->is_first())
        m_systemInfoFirst = *pSL;
    else
        m_systemInfoOther = *pSL;
}

//---------------------------------------------------------------------------------------
void ImoScore::add_page_info(ImoPageInfo* pPI)
{
    m_pageInfo = *pPI;
}

//---------------------------------------------------------------------------------------
ImoInstrGroups* ImoScore::get_instrument_groups()
{
    return dynamic_cast<ImoInstrGroups*>( get_child_of_type(ImoObj::k_instrument_groups) );
}

//---------------------------------------------------------------------------------------
void ImoScore::add_instruments_group(ImoInstrGroup* pGroup)
{
    ImoInstrGroups* pGroups = get_instrument_groups();
    if (!pGroups)
    {
        pGroups = new ImoInstrGroups();
        append_child(pGroups);
    }
    pGroups->append_child(pGroup);

    for (int i=0; i < pGroup->get_num_instruments(); i++)
        add_instrument(pGroup->get_instrument(i));
}

//---------------------------------------------------------------------------------------
void ImoScore::add_title(ImoScoreTitle* pTitle)
{
    m_titles.push_back(pTitle);
    append_child(pTitle);
}

//---------------------------------------------------------------------------------------
void ImoScore::add_style_info(ImoTextStyleInfo* pStyle)
{
    m_nameToStyle[pStyle->get_name()] = pStyle;
}

//---------------------------------------------------------------------------------------
ImoTextStyleInfo* ImoScore::get_style_info(const std::string& name)
{
	map<std::string, ImoTextStyleInfo*>::const_iterator it
        = m_nameToStyle.find(name);
	if (it != m_nameToStyle.end())
		return it->second;
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
ImoTextStyleInfo* ImoScore::get_style_info_or_defaults(const std::string& name)
{
    ImoTextStyleInfo* pStyle = get_style_info(name);
	if (pStyle)
		return pStyle;
    else
        return get_default_style_info();
}

//---------------------------------------------------------------------------------------
ImoTextStyleInfo* ImoScore::get_default_style_info()
{
    ImoTextStyleInfo* pStyle = get_style_info("Default style");
    if (pStyle)
		return pStyle;
    else
        return create_default_style();
}

//---------------------------------------------------------------------------------------
ImoTextStyleInfo* ImoScore::create_default_style()
{
	ImoTextStyleInfo* pStyle = new ImoTextStyleInfo();
	pStyle->set_name("Default style");
    pStyle->set_font_name("Liberation serif");
    pStyle->set_font_size(10.0f);
    pStyle->set_font_style(ImoFontInfo::k_normal);
    pStyle->set_font_weight(ImoFontInfo::k_normal);
    m_nameToStyle["Default style"] = pStyle;
    return pStyle;
}

//---------------------------------------------------------------------------------------
void ImoScore::add_required_text_styles()
{
    //For tuplets numbers
    if (get_style_info("Tuplet numbers") == NULL)
    {
	    ImoTextStyleInfo* pStyle = new ImoTextStyleInfo();
        pStyle->set_name("Tuplet numbers");
        pStyle->set_font_name("Liberation serif");
        pStyle->set_font_size(11.0f);
        pStyle->set_font_style(ImoFontInfo::k_italic);
        pStyle->set_font_weight(ImoFontInfo::k_normal);
        add_style_info(pStyle);
    }

    //For instrument and group names and abbreviations
    if (get_style_info("Instrument names") == NULL)
    {
	    ImoTextStyleInfo* pStyle = new ImoTextStyleInfo();
        pStyle->set_name("Instrument names");
        pStyle->set_font_name("Liberation serif");
        pStyle->set_font_size(14.0f);
        pStyle->set_font_style(ImoFontInfo::k_normal);
        pStyle->set_font_weight(ImoFontInfo::k_normal);
        add_style_info(pStyle);
    }

    //"Lyrics" - for lyrics
}

//---------------------------------------------------------------------------------------
// ImoPageInfo implementation
//---------------------------------------------------------------------------------------

ImoPageInfo::ImoPageInfo()
    : ImoSimpleObj(ImoObj::k_page_info)
    , m_uLeftMargin(1500.0f)
    , m_uRightMargin(1500.0f)
    , m_uTopMargin(2000.0f)
    , m_uBottomMargin(2000.0f)
    , m_uBindingMargin(0.0f)
    , m_uPageSize(21000.0f, 29700.0f)
    , m_fPortrait(true)
{
    //defaults: DIN A4 (210.0 x 297.0 mm), portrait
}

//---------------------------------------------------------------------------------------
ImoPageInfo::ImoPageInfo(ImoPageInfo& dto)
    : ImoSimpleObj(ImoObj::k_page_info)
    , m_uLeftMargin( dto.get_left_margin() )
    , m_uRightMargin( dto.get_right_margin() )
    , m_uTopMargin( dto.get_top_margin() )
    , m_uBottomMargin( dto.get_bottom_margin() )
    , m_uBindingMargin( dto.get_binding_margin() )
    , m_uPageSize( dto.get_page_size() )
    , m_fPortrait( dto.is_portrait() )
{
}

//---------------------------------------------------------------------------------------
// ImoSpacer implementation
//---------------------------------------------------------------------------------------

ImoSpacer::ImoSpacer(DtoSpacer& dto)
    : ImoStaffObj(ImoObj::k_spacer, dto)
    , m_space( dto.get_width() )
{
}


//---------------------------------------------------------------------------------------
// ImoSystemInfo implementation
//---------------------------------------------------------------------------------------

ImoSystemInfo::ImoSystemInfo()
    : ImoSimpleObj(ImoObj::k_system_info)
    , m_fFirst(true)
    , m_leftMargin(0.0f)
    , m_rightMargin(0.0f)
    , m_systemDistance(0.0f)
    , m_topSystemDistance(0.0f)
{
}

//---------------------------------------------------------------------------------------
ImoSystemInfo::ImoSystemInfo(ImoSystemInfo& dto)
    : ImoSimpleObj(ImoObj::k_system_info)
    , m_fFirst( dto.is_first() )
    , m_leftMargin( dto.get_left_margin() )
    , m_rightMargin( dto.get_right_margin() )
    , m_systemDistance( dto.get_system_distance() )
    , m_topSystemDistance( dto.get_top_system_distance() )
{
}

//---------------------------------------------------------------------------------------
// ImoTextInfo implementation
//---------------------------------------------------------------------------------------

const std::string& ImoTextInfo::get_font_name()
{
    return m_pStyle->get_font_name();
}

//---------------------------------------------------------------------------------------
float ImoTextInfo::get_font_size()
{
    return m_pStyle->get_font_size();
}

//---------------------------------------------------------------------------------------
int ImoTextInfo::get_font_style()
{
    return m_pStyle->get_font_style();
}

//---------------------------------------------------------------------------------------
int ImoTextInfo::get_font_weight()
{
    return m_pStyle->get_font_weight();
}

//---------------------------------------------------------------------------------------
Color ImoTextInfo::get_color()
{
    return m_pStyle->get_color();
}


//=======================================================================================
// ImoTie implementation
//=======================================================================================
ImoTie::~ImoTie()
{
    if (m_pStartBezier)
        delete m_pStartBezier;
    if (m_pEndBezier)
        delete m_pEndBezier;
}

//---------------------------------------------------------------------------------------
ImoNote* ImoTie::get_start_note() 
{ 
    return static_cast<ImoNote*>(m_pStartSO);
}

//---------------------------------------------------------------------------------------
ImoNote* ImoTie::get_end_note() 
{ 
    return static_cast<ImoNote*>(m_pEndSO); 
}

//---------------------------------------------------------------------------------------
void ImoTie::set_start_note(ImoNote* pNote) 
{ 
    m_pStartSO = pNote; 
}

//---------------------------------------------------------------------------------------
void ImoTie::set_end_note(ImoNote* pNote) 
{ 
    m_pEndSO = pNote; 
}


//=======================================================================================
// ImoTieDto implementation
//=======================================================================================
ImoTieDto::~ImoTieDto()
{
    if (m_pBezier)
        delete m_pBezier;
}

//---------------------------------------------------------------------------------------
int ImoTieDto::get_line_number()
{
    if (m_pTieElm)
        return m_pTieElm->get_line_number();
    else
        return 0;
}


//---------------------------------------------------------------------------------------
// ImoTimeSignature implementation
//---------------------------------------------------------------------------------------

ImoTimeSignature::ImoTimeSignature(DtoTimeSignature& dto)
    : ImoStaffObj(ImoObj::k_time_signature, dto)
    , m_beats( dto.get_beats() )
    , m_beatType( dto.get_beat_type() )
{
}


//---------------------------------------------------------------------------------------
// ImoTupletDto implementation
//---------------------------------------------------------------------------------------

ImoTupletDto::ImoTupletDto()
    : ImoSimpleObj(ImoObj::k_tuplet_info)
    , m_fStartOfTuplet(true)
    , m_nActualNum(0)
    , m_nNormalNum(0)
    , m_nShowBracket(k_yesno_default)
    , m_nPlacement(k_placement_default)
    , m_nShowNormalNum(k_yesno_default)
    , m_pTupletElm(NULL)
    , m_pNR(NULL)
{
}

//---------------------------------------------------------------------------------------
ImoTupletDto::ImoTupletDto(LdpElement* pTupletElm)
    : ImoSimpleObj(ImoObj::k_tuplet_info)
    , m_fStartOfTuplet(true)
    , m_nActualNum(0)
    , m_nNormalNum(0)
    , m_nShowBracket(k_yesno_default)
    , m_nPlacement(k_placement_default)
    , m_nShowNormalNum(k_yesno_default)
    , m_pTupletElm(pTupletElm)
    , m_pNR(NULL)
{
}

//---------------------------------------------------------------------------------------
int ImoTupletDto::get_line_number()
{
    if (m_pTupletElm)
        return m_pTupletElm->get_line_number();
    else
        return 0;
}


//=======================================================================================
// ImoTuplet implementation
//=======================================================================================
ImoTuplet::ImoTuplet(ImoTupletDto* dto)
    : ImoMultiRelObj(ImoObj::k_tuplet)
    , m_nActualNum(dto->get_actual_number())
    , m_nNormalNum(dto->get_normal_number())
    , m_nShowBracket(dto->get_show_bracket())
    , m_nShowNormalNum(dto->get_show_normal_num())
    , m_nPlacement(dto->get_placement())
{
}


//---------------------------------------------------------------------------------------
// global functions related to notes
//---------------------------------------------------------------------------------------

int to_step(const char& letter)
{
	switch (letter)
    {
		case 'a':	return k_step_A;
		case 'b':	return k_step_B;
		case 'c':	return k_step_C;
		case 'd':	return k_step_D;
		case 'e':	return k_step_E;
		case 'f':	return k_step_F;
		case 'g':	return k_step_G;
	}
	return -1;
}

//---------------------------------------------------------------------------------------
int to_octave(const char& letter)
{
	switch (letter)
    {
		case '0':	return 0;
		case '1':	return 1;
		case '2':	return 2;
		case '3':	return 3;
		case '4':	return 4;
		case '5':	return 5;
		case '6':	return 6;
		case '7':	return 7;
		case '8':	return 8;
		case '9':	return 9;
	}
	return -1;
}

//---------------------------------------------------------------------------------------
int to_accidentals(const std::string& accidentals)
{
    switch (accidentals.length())
    {
        case 0:
            return ImoNote::k_no_accidentals;
            break;

        case 1:
            if (accidentals[0] == '+')
                return ImoNote::k_sharp;
            else if (accidentals[0] == '-')
                return ImoNote::k_flat;
            else if (accidentals[0] == '=')
                return ImoNote::k_natural;
            else if (accidentals[0] == 'x')
                return ImoNote::k_double_sharp;
            else
                return -1;
            break;

        case 2:
            if (accidentals.compare(0, 2, "++") == 0)
                return ImoNote::k_sharp_sharp;
            else if (accidentals.compare(0, 2, "--") == 0)
                return ImoNote::k_flat_flat;
            else if (accidentals.compare(0, 2, "=-") == 0)
                return ImoNote::k_natural_flat;
            else
                return -1;
            break;

        default:
            return -1;
    }
}

//---------------------------------------------------------------------------------------
int to_note_type(const char& letter)
{
    //  USA           UK                      ESP               LDP     NoteType
    //  -----------   --------------------    -------------     ---     ---------
    //  long          longa                   longa             l       k_longa = 0
    //  double whole  breve                   cuadrada, breve   b       k_breve = 1
    //  whole         semibreve               redonda           w       k_whole = 2
    //  half          minim                   blanca            h       k_half = 3
    //  quarter       crochet                 negra             q       k_quarter = 4
    //  eighth        quaver                  corchea           e       k_eighth = 5
    //  sixteenth     semiquaver              semicorchea       s       k_16th = 6
    //  32nd          demisemiquaver          fusa              t       k_32th = 7
    //  64th          hemidemisemiquaver      semifusa          i       k_64th = 8
    //  128th         semihemidemisemiquaver  garrapatea        o       k_128th = 9
    //  256th         ???                     semigarrapatea    f       k_256th = 10

    switch (letter)
    {
        case 'l':     return ImoNote::k_longa;
        case 'b':     return ImoNote::k_breve;
        case 'w':     return ImoNote::k_whole;
        case 'h':     return ImoNote::k_half;
        case 'q':     return ImoNote::k_quarter;
        case 'e':     return ImoNote::k_eighth;
        case 's':     return ImoNote::k_16th;
        case 't':     return ImoNote::k_32th;
        case 'i':     return ImoNote::k_64th;
        case 'o':     return ImoNote::k_128th;
        case 'f':     return ImoNote::k_256th;
        default:
            return -1;
    }
}

//---------------------------------------------------------------------------------------
bool ldp_pitch_to_components(const string& pitch, int *step, int* octave, int* accidentals)
{
    //    Analyzes string pitch (LDP format), extracts its parts (step, octave and
    //    accidentals) and stores them in the corresponding parameters.
    //    Returns true if error (pitch is not a valid pitch name)
    //
    //    In LDP pitch is represented as a combination of the step of the diatonic scale, the
    //    chromatic alteration, and the octave.
    //      - The accidentals parameter represents chromatic alteration (does not include tonal
    //        key alterations)
    //      - The octave element is represented by the numbers 0 to 9, where 4 indicates
    //        the octave started by middle C.
    //
    //    pitch must be trimed (no spaces before or after real data) and lower case

    size_t i = pitch.length() - 1;
    if (i < 1)
        return true;   //error

    *octave = to_octave(pitch[i--]);
    if (*octave == -1)
        return true;   //error

    *step = to_step(pitch[i--]);
    if (*step == -1)
        return true;   //error

    if (++i == 0)
    {
        *accidentals = ImoNote::k_no_accidentals;
        return false;   //no error
    }
    else
        *accidentals = to_accidentals(pitch.substr(0, i));
    if (*accidentals == -1)
        return true;   //error

    return false;  //no error
}

//---------------------------------------------------------------------------------------
NoteTypeAndDots ldp_duration_to_components(const string& duration)
{
    // Return struct with noteType and dots.
    // If error, noteType is set to unknown and dots to zero

    size_t size = duration.length();
    if (size == 0)
        return NoteTypeAndDots(ImoNoteRest::k_unknown, 0);   //error

    //duration
    int noteType = to_note_type(duration[0]);
    if (noteType == -1)
        return NoteTypeAndDots(ImoNoteRest::k_unknown, 0);   //error

    //dots
    int dots = 0;
    for (size_t i=1; i < size; i++)
    {
        if (duration[i] == '.')
            dots++;
        else
            return NoteTypeAndDots(ImoNoteRest::k_unknown, 0);   //error
    }

    return NoteTypeAndDots(noteType, dots);   //no error
}

//---------------------------------------------------------------------------------------
float to_duration(int nNoteType, int nDots)
{
    //compute duration without modifiers
    //float rDuration = pow(2.0f, (10 - nNoteType));
    //Removed: pow not safe
    //      Valgrind: Conditional jump or move depends on uninitialised value(s)
    //                ==8126==    at 0x4140BBF: __ieee754_pow (e_pow.S:118)
    float rDuration = 1;
    switch (nNoteType)
    {
        case ImoNote::k_longa:   rDuration=1024; break;    //  0
        case ImoNote::k_breve:   rDuration=512;  break;    //  1
        case ImoNote::k_whole:   rDuration=256;  break;    //  2
        case ImoNote::k_half:    rDuration=128;  break;    //  3
        case ImoNote::k_quarter: rDuration=64;   break;    //  4
        case ImoNote::k_eighth:  rDuration=32;   break;    //  5
        case ImoNote::k_16th:    rDuration=16;   break;    //  6
        case ImoNote::k_32th:    rDuration=8;    break;    //  7
        case ImoNote::k_64th:    rDuration=4;    break;    //  8
        case ImoNote::k_128th:   rDuration=2;    break;    //  9
        case ImoNote::k_256th:   rDuration=1;    break;    //  10
        default:
            rDuration=64;
    }

    //take dots into account
    switch (nDots)
    {
        case 0:                             break;
        case 1: rDuration *= 1.5f;          break;
        case 2: rDuration *= 1.75f;         break;
        case 3: rDuration *= 1.875f;        break;
        case 4: rDuration *= 1.9375f;       break;
        case 5: rDuration *= 1.96875f;      break;
        case 6: rDuration *= 1.984375f;     break;
        case 7: rDuration *= 1.9921875f;    break;
        case 8: rDuration *= 1.99609375f;   break;
        case 9: rDuration *= 1.998046875f;  break;
        default:
            ;
            //wxLogMessage(_T("[to_duration] Program limit: do you really need more than nine dots?"));
    }

    return rDuration;
}


}  //namespace lomse

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include "lomse_internal_model.h"

#include <algorithm>
#include <math.h>                   //pow
#include "lomse_staffobjs_table.h"
#include "lomse_im_note.h"
#include "lomse_midi_table.h"
#include "lomse_dyn_generator.h"
#include "lomse_ldp_elements.h"
#include "lomse_im_factory.h"
#include "lomse_model_builder.h"
#include "lomse_control.h"
#include "lomse_score_player_ctrl.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// utility function to convert typographical points to LUnits
LUnits pt_to_LUnits(float pt)
{
    // 1pt = 1/72" = 25.4/72 mm = 2540/72 LU = 35.2777777777 LU
    // Wikipedia (http://en.wikipedia.org/wiki/Typographic_unit)
    return pt * 35.277777777f;
}

//---------------------------------------------------------------------------------------
// static variables to convert from ImoObj type to name
static std::map<int, std::string> m_TypeToName;
static bool m_fNamesRegistered;
static string m_unknown = "unknown";


//=======================================================================================
// InternalModel implementation
//=======================================================================================
InternalModel::~InternalModel()
{
    delete m_pRoot;
}


//=======================================================================================
// InlineLevelCreatorApi implementation
//=======================================================================================
ImoTextItem* InlineLevelCreatorApi::add_text_item(const string& text, ImoStyle* pStyle)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoTextItem* pImo = static_cast<ImoTextItem*>(
                            ImFactory::inject(k_imo_text_item, pDoc) );
    pImo->set_text(text);
    pImo->set_style(pStyle);
    m_pParent->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoButton* InlineLevelCreatorApi::add_button(const string& label, const USize& size,
                                    ImoStyle* pStyle)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoButton* pImo = static_cast<ImoButton*>(ImFactory::inject(k_imo_button, pDoc));
    pImo->set_label(label);
    pImo->set_size(size);
    pImo->set_style(pStyle);
    m_pParent->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoInlineWrapper* InlineLevelCreatorApi::add_inline_box(LUnits width, ImoStyle* pStyle)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                ImFactory::inject(k_imo_inline_wrapper, pDoc) );
    pImo->set_width(width);
    pImo->set_style(pStyle);
    m_pParent->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoLink* InlineLevelCreatorApi::add_link(const string& url, ImoStyle* pStyle)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoLink* pImo = static_cast<ImoLink*>( ImFactory::inject(k_imo_link, pDoc) );
    pImo->set_url(url);
    pImo->set_style(pStyle);
    m_pParent->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoImage* InlineLevelCreatorApi::add_image(unsigned char* imgbuf, VSize bmpSize,
                                           EPixelFormat format, USize imgSize,
                                           ImoStyle* pStyle)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoImage* pImo = ImFactory::inject_image(pDoc, imgbuf, bmpSize, format, imgSize);
    pImo->set_style(pStyle);
    m_pParent->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoControl* InlineLevelCreatorApi::add_control(Control* pCtrol)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoControl* pImo = ImFactory::inject_control(pDoc, pCtrol);
    m_pParent->append_child_imo(pImo);
    return pImo;
}


//=======================================================================================
// BlockLevelCreatorApi implementation
//=======================================================================================
ImoParagraph* BlockLevelCreatorApi::add_paragraph(ImoStyle* pStyle)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoParagraph* pImo = static_cast<ImoParagraph*>(
                                    ImFactory::inject(k_imo_para, pDoc) );
    add_to_model(pImo, pStyle);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoList* BlockLevelCreatorApi::add_list(int type, ImoStyle* pStyle)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoList* pImo = static_cast<ImoList*>(ImFactory::inject(k_imo_list, pDoc) );
    pImo->set_list_type(type);
    add_to_model(pImo, pStyle);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoContent* BlockLevelCreatorApi::add_content_wrapper(ImoStyle* pStyle)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoContent* pImo = static_cast<ImoContent*>(
                                    ImFactory::inject(k_imo_content, pDoc) );
    add_to_model(pImo, pStyle);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoMultiColumn* BlockLevelCreatorApi::add_multicolumn_wrapper(int numCols,
                                                              ImoStyle* pStyle)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoMultiColumn* pImo = ImFactory::inject_multicolumn(pDoc);
    add_to_model(pImo, pStyle);
    pImo->create_columns(numCols);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoScore* BlockLevelCreatorApi::add_score(ImoStyle* pStyle)
{
    Document* pDoc = m_pParent->get_the_document();
    ImoScore* pImo = static_cast<ImoScore*>(
                                ImFactory::inject(k_imo_score, pDoc) );
    add_to_model(pImo, pStyle);
    return pImo;
}

//---------------------------------------------------------------------------------------
void BlockLevelCreatorApi::add_to_model(ImoBoxLevelObj* pImo, ImoStyle* pStyle)
{
    pImo->set_style(pStyle);
    ImoObj* pNode = m_pParent->is_document() ?
                    static_cast<ImoDocument*>(m_pParent)->get_content()
                    : m_pParent;
    pNode->append_child_imo(pImo);
}



//=======================================================================================
// ImoObj implementation
//=======================================================================================
ImoObj::ImoObj(int objtype, long id)
    : m_id(id)
    , m_objtype(objtype)
    , m_flags(k_dirty)
{
    //Register all IM objects
    if (!m_fNamesRegistered)
    {
        // ImoStaffObj (A)
        m_TypeToName[k_imo_barline] = "barline";
        m_TypeToName[k_imo_clef] = "clef";
        m_TypeToName[k_imo_key_signature] = "keySignature";
        m_TypeToName[k_imo_time_signature] = "time";
        m_TypeToName[k_imo_note] = "n";
        m_TypeToName[k_imo_rest] = "r";
        m_TypeToName[k_imo_go_back_fwd] = "go_back_fwd";
        m_TypeToName[k_imo_metronome_mark] = "metronome";
        m_TypeToName[k_imo_system_break] = "system-break";
        m_TypeToName[k_imo_spacer] = "spacer";
        m_TypeToName[k_imo_figured_bass] = "figuredBass";

        // ImoBoxContainer (A)
        m_TypeToName[k_imo_content] = "content";
        m_TypeToName[k_imo_document] = "lenmusdoc";
        m_TypeToName[k_imo_score] = "score";

        // ImoBoxContent (A)
        m_TypeToName[k_imo_dynamic] = "dynamic";
        m_TypeToName[k_imo_heading] = "heading";
        m_TypeToName[k_imo_para] = "para";

        // ImoInlineObj
        m_TypeToName[k_imo_button] = "buttom";
        m_TypeToName[k_imo_text_item] = "txt";

        // ImoBoxInline (A)
        m_TypeToName[k_imo_inline_wrapper] = "wrapper";
        m_TypeToName[k_imo_link] = "link";

        // ImoDto, ImoSimpleObj (A)
        m_TypeToName[k_imo_beam_dto] = "beam";
        m_TypeToName[k_imo_bezier_info] = "bezier";
        m_TypeToName[k_imo_border_dto] = "border";
        m_TypeToName[k_imo_textblock_info] = "textblock";
        m_TypeToName[k_imo_color_dto] = "color";
        m_TypeToName[k_imo_cursor_info] = "cursor";
        m_TypeToName[k_imo_figured_bass_info] = "figured-bass";
        m_TypeToName[k_imo_font_style_dto] = "font-style";
        m_TypeToName[k_imo_instr_group] = "instr-group";
        m_TypeToName[k_imo_line_style] = "line-style";
        m_TypeToName[k_imo_midi_info] = "midi-info";
        m_TypeToName[k_imo_option] = "opt";
        m_TypeToName[k_imo_page_info] = "page-info";
        m_TypeToName[k_imo_param_info] = "param";
        m_TypeToName[k_imo_point_dto] = "point";
        m_TypeToName[k_imo_size_dto] = "size";
        m_TypeToName[k_imo_slur_dto] = "slur-dto";
        m_TypeToName[k_imo_staff_info] = "staff-info";
        m_TypeToName[k_imo_system_info] = "system-info";
        m_TypeToName[k_imo_text_info] = "text-info";
        m_TypeToName[k_imo_text_style] = "text-style";
        m_TypeToName[k_imo_style] = "style";
        m_TypeToName[k_imo_tie_dto] = "tie-dto";
        m_TypeToName[k_imo_tuplet_dto] = "tuplet-dto";

        // ImoRelDataObj (A)
        m_TypeToName[k_imo_beam_data] = "beam-data";
        m_TypeToName[k_imo_slur_data] = "slur-data";
        m_TypeToName[k_imo_tie_data] = "tie-data";
        m_TypeToName[k_imo_tuplet_data] = "tuplet-data";
//
        //ImoCollection(A)
        m_TypeToName[k_imo_instruments] = "instruments";
//            k_imo_instrument_groups,
        m_TypeToName[k_imo_music_data] = "musicData";
        m_TypeToName[k_imo_options] = "options";
        m_TypeToName[k_imo_styles] = "styles";

        // Special collections
        m_TypeToName[k_imo_attachments] = "attachments";

        // ImoContainerObj (A)
        m_TypeToName[k_imo_instrument] = "instrument";

        // ImoAuxObj (A)
        m_TypeToName[k_imo_fermata] = "fermata";
        m_TypeToName[k_imo_line] = "line";
        m_TypeToName[k_imo_score_text] = "score-text";
        m_TypeToName[k_imo_score_title] = "title";
        m_TypeToName[k_imo_text_box] = "text-box";

        // ImoRelObj (A)
        m_TypeToName[k_imo_beam] = "beam";
        m_TypeToName[k_imo_chord] = "chord";
        m_TypeToName[k_imo_slur] = "slur";
        m_TypeToName[k_imo_tie] = "tie";
        m_TypeToName[k_imo_tuplet] = "tuplet";

        m_fNamesRegistered = true;
    }
}

//---------------------------------------------------------------------------------------
ImoObj::~ImoObj()
{
    TreeNode<ImoObj>::children_iterator it(this);
    it = begin();
    while (it != end())
    {
        ImoObj* child = *it;
        ++it;
	    delete child;
    }
}

//---------------------------------------------------------------------------------------
const string& ImoObj::get_name(int type)
{
	map<int, std::string>::const_iterator it = m_TypeToName.find( type );
	if (it != m_TypeToName.end())
		return it->second;
    else
        return m_unknown;
        //throw std::runtime_error( "[ImoObj::get_name]. Invalid type" );
}

//---------------------------------------------------------------------------------------
const string& ImoObj::get_name() const
{
	return ImoObj::get_name( m_objtype );
}

//---------------------------------------------------------------------------------------
void ImoObj::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoObj>* p = dynamic_cast<Visitor<ImoObj>*>(&v);
    if (p)
        p->start_visit(this);

    visit_children(v);

    if (p)
        p->end_visit(this);
}

//---------------------------------------------------------------------------------------
void ImoObj::visit_children(BaseVisitor& v)
{
    TreeNode<ImoObj>::children_iterator it;
    for (it = this->begin(); it != this->end(); ++it)
    {
        (*it)->accept_visitor(v);
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
void ImoObj::set_dirty(bool dirty)
{
    if (dirty)
    {
        m_flags |= k_dirty;
        propagate_dirty();
    }
    else
        m_flags &= ~k_dirty;
}

//---------------------------------------------------------------------------------------
void ImoObj::set_children_dirty(bool value)
{
    value ? m_flags |= k_children_dirty : m_flags &= ~k_children_dirty;
}

//---------------------------------------------------------------------------------------
void ImoObj::propagate_dirty()
{
    ImoObj* pParent = get_parent_imo();
    if (pParent)
    {
        pParent->set_children_dirty(true);
        pParent->propagate_dirty();
    }

    if (this->is_document())
    {
        ImoDocument* pImoDoc = static_cast<ImoDocument*>(this);
        Document* pDoc = pImoDoc->get_the_document();
        pDoc->set_dirty();
    }
}

//---------------------------------------------------------------------------------------
void ImoObj::append_child_imo(ImoObj* pImo)
{
    append_child(pImo);
    set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ImoObj::remove_child_imo(ImoObj* pImo)
{
    remove_child(pImo);
    set_dirty(true);
}

//=======================================================================================
// ImoRelObj implementation
//=======================================================================================
ImoRelObj::~ImoRelObj()
{
    //should be empty. Unit test
    m_relatedObjects.clear();
}

//---------------------------------------------------------------------------------------
void ImoRelObj::push_back(ImoStaffObj* pSO, ImoRelDataObj* pData)
{
    m_relatedObjects.push_back( make_pair(pSO, pData));
}

//---------------------------------------------------------------------------------------
void ImoRelObj::remove(ImoStaffObj* pSO)
{
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    for (it = m_relatedObjects.begin(); it != m_relatedObjects.end(); ++it)
    {
        if ((*it).first == pSO)
        {
            delete (*it).second;
            m_relatedObjects.erase(it);
            return;
        }
    }
}

//---------------------------------------------------------------------------------------
void ImoRelObj::remove_all()
{
    //This is recursive. If there are objects, we delete the first one by
    //invoking "pSO->remove_but_not_delete_relation(this);" . And it, in turn,
    //invokes this method, until all items get deleted!
    while (m_relatedObjects.size() > 0)
    {
        ImoStaffObj* pSO = m_relatedObjects.front().first;
        pSO->remove_but_not_delete_relation(this);
    }
}

//---------------------------------------------------------------------------------------
ImoRelDataObj* ImoRelObj::get_data_for(ImoStaffObj* pSO)
{
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    for (it = m_relatedObjects.begin(); it != m_relatedObjects.end(); ++it)
    {
        if ((*it).first == pSO)
            return (*it).second;
    }
    return NULL;
}


//=======================================================================================
// ImoStaffObj implementation
//=======================================================================================
ImoStaffObj::~ImoStaffObj()
{
    ImoRelations* pRelObjs = get_relations();
    if (pRelObjs)
    {
        pRelObjs->remove_from_all_relations(this);
        this->remove_child_imo(pRelObjs);
        delete pRelObjs;
    }
}

//---------------------------------------------------------------------------------------
void ImoStaffObj::add_relation(Document* pDoc, ImoRelObj* pRO)
{
    ImoRelations* pRelObjs = get_relations();
    if (!pRelObjs)
    {
        pRelObjs = static_cast<ImoRelations*>(
                        ImFactory::inject(k_imo_relations, pDoc) );
        append_child_imo(pRelObjs);
    }
    pRelObjs->add(pRO);
}

//---------------------------------------------------------------------------------------
void ImoStaffObj::include_in_relation(Document* pDoc, ImoRelObj* pRelObj,
                                      ImoRelDataObj* pData)
{
    add_relation(pDoc, pRelObj);
    pRelObj->push_back(this, pData);
}

//---------------------------------------------------------------------------------------
void ImoStaffObj::remove_from_relation(ImoRelObj* pRelObj)
{
    remove_but_not_delete_relation(pRelObj);

    if (pRelObj->get_num_objects() < pRelObj->get_min_number_for_autodelete())
        pRelObj->remove_all();

    if (pRelObj->get_num_objects() == 0)
        delete pRelObj;
}

//---------------------------------------------------------------------------------------
void ImoStaffObj::remove_but_not_delete_relation(ImoRelObj* pRelObj)
{
    pRelObj->remove(this);

    ImoRelations* pRelObjs = get_relations();
    pRelObjs->remove(pRelObj);
}

//---------------------------------------------------------------------------------------
ImoRelObj* ImoStaffObj::get_relation(int i)
{
    ImoRelations* pRelObjs = get_relations();
    return pRelObjs->get_item(i);
}

//---------------------------------------------------------------------------------------
bool ImoStaffObj::has_relations()
{
    ImoRelations* pRelObjs = get_relations();
    if (pRelObjs)
        return pRelObjs->get_num_items() > 0;
    else
        return false;
}

//---------------------------------------------------------------------------------------
int ImoStaffObj::get_num_relations()
{
    ImoRelations* pRelObjs = get_relations();
    if (pRelObjs)
        return pRelObjs->get_num_items();
    else
        return 0;
}

//---------------------------------------------------------------------------------------
ImoRelations* ImoStaffObj::get_relations()
{
    return dynamic_cast<ImoRelations*>( get_child_of_type(k_imo_relations) );
}

//---------------------------------------------------------------------------------------
ImoRelObj* ImoStaffObj::find_relation(int type)
{
    ImoRelations* pRelObjs = get_relations();
    if (!pRelObjs)
        return NULL;
    else
        return pRelObjs->find_item_of_type(type);
}



//=======================================================================================
// ImoBeamData implementation
//=======================================================================================
ImoBeamData::ImoBeamData(ImoBeamDto* pDto)
    : ImoRelDataObj(k_imo_beam_data)
    , m_beamNum( pDto->get_beam_number() )
{
    for (int i=0; i < 6; ++i)
    {
        m_beamType[i] = pDto->get_beam_type(i);
        m_repeat[i] = pDto->get_repeat(i);
    }
}

//---------------------------------------------------------------------------------------
bool ImoBeamData::is_start_of_beam()
{
    //at least one is begin, forward or backward.

    bool fStart = false;
    for (int level=0; level < 6; level++)
    {
        if (m_beamType[level] == ImoBeam::k_end
            || m_beamType[level] == ImoBeam::k_continue
            )
            return false;
        if (m_beamType[level] != ImoBeam::k_none)
            fStart = true;
    }
    return fStart;
}

//---------------------------------------------------------------------------------------
bool ImoBeamData::is_end_of_beam()
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


//=======================================================================================
// ImoBeamDto implementation
//=======================================================================================
ImoBeamDto::ImoBeamDto()
    : ImoSimpleObj(k_imo_beam_dto)
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
ImoBeamDto::ImoBeamDto(LdpElement* pBeamElm)
    : ImoSimpleObj(k_imo_beam_dto)
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
int ImoBeamDto::get_line_number()
{
    if (m_pBeamElm)
        return m_pBeamElm->get_line_number();
    else
        return 0;
}

//---------------------------------------------------------------------------------------
void ImoBeamDto::set_beam_type(int level, int type)
{
    m_beamType[level] = type;
}

//---------------------------------------------------------------------------------------
void ImoBeamDto::set_beam_type(string& segments)
{
    if (segments.size() < 7)
    {
        for (int i=0; i < int(segments.size()); ++i)
        {
            if (segments[i] == '+')
                set_beam_type(i, ImoBeam::k_begin);
            else if (segments[i] == '=')
                set_beam_type(i, ImoBeam::k_continue);
            else if (segments[i] == '-')
                set_beam_type(i, ImoBeam::k_end);
            else if (segments[i] == 'f')
                set_beam_type(i, ImoBeam::k_forward);
            else if (segments[i] == 'b')
                set_beam_type(i, ImoBeam::k_backward);
            else
                return;   //error
        }
    }
}

//---------------------------------------------------------------------------------------
int ImoBeamDto::get_beam_type(int level)
{
    return m_beamType[level];
}

//---------------------------------------------------------------------------------------
bool ImoBeamDto::is_start_of_beam()
{
    //at least one is begin, forward or backward.

    bool fStart = false;
    for (int level=0; level < 6; level++)
    {
        if (m_beamType[level] == ImoBeam::k_end
            || m_beamType[level] == ImoBeam::k_continue
            )
            return false;
        if (m_beamType[level] != ImoBeam::k_none)
            fStart = true;
    }
    return fStart;
}

//---------------------------------------------------------------------------------------
bool ImoBeamDto::is_end_of_beam()
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
void ImoBeamDto::set_repeat(int level, bool value)
{
    m_repeat[level] = value;
}

//---------------------------------------------------------------------------------------
bool ImoBeamDto::get_repeat(int level)
{
    return m_repeat[level];
}


//=======================================================================================
// ImoBezierInfo implementation
//=======================================================================================
ImoBezierInfo::ImoBezierInfo(ImoBezierInfo* pBezier)
    : ImoSimpleObj(k_imo_bezier_info)
{
    if (pBezier)
    {
        for (int i=0; i < 4; ++i)
            m_tPoints[i] = pBezier->get_point(i);
    }
}


//=======================================================================================
// ImoBoxContainer implementation
//=======================================================================================
//ImoContentObj* ImoBoxContainer::get_container_node()
//{
//    return this;
//}
//
////---------------------------------------------------------------------------------------
//ImoParagraph* ImoBoxContainer::add_paragraph(ImoStyle* pStyle)
//{
//    Document* pDoc = get_the_document();
//    ImoParagraph* pPara = static_cast<ImoParagraph*>(
//                                    ImFactory::inject(k_imo_para, pDoc) );
//    pPara->set_style(pStyle);
//
//    ImoContentObj* node = get_container_node();
//    node->append_child_imo(pPara);
//    return pPara;
//}


//=======================================================================================
// ImoColorDto implementation
//=======================================================================================
ImoColorDto::ImoColorDto(Int8u r, Int8u g, Int8u b, Int8u a)
    : ImoDto(k_imo_color_dto)
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
Color& ImoColorDto::set_from_rgb_string(const std::string& rgb)
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
Color& ImoColorDto::set_from_rgba_string(const std::string& rgba)
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
Color& ImoColorDto::set_from_string(const std::string& hex)
{
    if (hex.length() == 7)
        return set_from_rgb_string(hex);
    else if (hex.length() == 9)
        return set_from_rgba_string(hex);
    else
    {
        m_ok = false;
        m_color = Color(0,0,0,255);
        return m_color;
    }
}


//=======================================================================================
// ImoRelations implementation
//=======================================================================================
ImoRelations::~ImoRelations()
{
    std::list<ImoRelObj*>::iterator it;
    for(it = m_relations.begin(); it != m_relations.end(); ++it)
        delete *it;
    m_relations.clear();
}

//---------------------------------------------------------------------------------------
void ImoRelations::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoObj>* vObj = NULL;

    vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
    if (vObj)
        vObj->start_visit(this);

    //visit_children
    std::list<ImoRelObj*>::iterator it;
    for(it = m_relations.begin(); it != m_relations.end(); ++it)
        (*it)->accept_visitor(v);

    if (vObj)
        vObj->end_visit(this);
}

//---------------------------------------------------------------------------------------
ImoRelObj* ImoRelations::get_item(int iItem)
{
    std::list<ImoRelObj*>::iterator it;
    for(it = m_relations.begin(); it != m_relations.end() && iItem > 0; ++it, --iItem);
    if (it != m_relations.end())
        return *it;
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
void ImoRelations::remove(ImoRelObj* pRO)
{
    m_relations.remove(pRO);
}

//---------------------------------------------------------------------------------------
void ImoRelations::add(ImoRelObj* pRO)
{
    //attachments must be ordered by renderization priority

    int priority = get_priority( pRO->get_obj_type() );
    if (priority > 1000)
        m_relations.push_back(pRO);
    else
    {
        std::list<ImoRelObj*>::iterator it;
        for(it = m_relations.begin(); it != m_relations.end(); ++it)
        {
            if (get_priority((*it)->get_obj_type()) > priority)
                break;
        }

        if (it == m_relations.end())
            m_relations.push_back(pRO);
        else
            m_relations.insert(it, pRO);
    }
}

//---------------------------------------------------------------------------------------
int ImoRelations::get_priority(int type)
{
    //not listed objects are low priority (order not important, added at end)
    static bool fMapInitialized = false;
    static map<int, int> priority;

    if (!fMapInitialized)
    {
        priority[k_imo_tie] = 0;
        priority[k_imo_beam] = 1;
        priority[k_imo_chord] = 2;
        priority[k_imo_tuplet] = 3;
        priority[k_imo_slur] = 4;
        priority[k_imo_fermata] = 5;

        fMapInitialized = true;
    };

	map<int, int>::const_iterator it = priority.find( type );
	if (it !=  priority.end())
		return it->second;

    return 5000;
}

//---------------------------------------------------------------------------------------
ImoRelObj* ImoRelations::find_item_of_type(int type)
{
    std::list<ImoRelObj*>::iterator it;
    for(it = m_relations.begin(); it != m_relations.end(); ++it)
    {
        if ((*it)->get_obj_type() == type)
            return *it;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
void ImoRelations::remove_from_all_relations(ImoStaffObj* pSO)
{
    std::list<ImoRelObj*>::iterator it = m_relations.begin();
    while (it != m_relations.end())
    {
        if ((*it)->is_relobj())
        {
            ImoRelObj* pRO = static_cast<ImoRelObj*>( *it );
            ++it;
            pSO->remove_from_relation(pRO);
            //m_relations.erase(it++);
        }
        else
        {
            ImoRelObj* pRO = *it;
            ++it;
            delete pRO;
        }
    }
    m_relations.clear();
}

////---------------------------------------------------------------------------------------
//void ImoRelations::remove_all_attachments()
//{
//    std::list<ImoRelObj*>::iterator it = m_relations.begin();
//    while (it != m_relations.end())
//    {
//        ImoRelObj* pRO = *it;
//        ++it;
//        delete pRO;
//    }
//    m_relations.clear();
//}


//=======================================================================================
// ImoContentObj implementation
//=======================================================================================
ImoContentObj::ImoContentObj(int objtype)
    : ImoObj(objtype)
    , Observable()
    , m_pStyle(NULL)
    , m_txUserLocation(0.0f)
    , m_tyUserLocation(0.0f)
    , m_fVisible(true)
{
}

//---------------------------------------------------------------------------------------
ImoContentObj::ImoContentObj(long id, int objtype)
    : ImoObj(id, objtype)
    , Observable()
    , m_pStyle(NULL)
    , m_txUserLocation(0.0f)
    , m_tyUserLocation(0.0f)
    , m_fVisible(true)
{
}

//---------------------------------------------------------------------------------------
ImoContentObj::~ImoContentObj()
{
}

//---------------------------------------------------------------------------------------
void ImoContentObj::add_attachment(Document* pDoc, ImoAuxObj* pAO)
{
    ImoAttachments* pAuxObjs = get_attachments();
    if (!pAuxObjs)
    {
        pAuxObjs = static_cast<ImoAttachments*>(
                        ImFactory::inject(k_imo_attachments, pDoc) );
        append_child_imo(pAuxObjs);
    }
    pAuxObjs->add(pAO);
}

//---------------------------------------------------------------------------------------
ImoAuxObj* ImoContentObj::get_attachment(int i)
{
    ImoAttachments* pAuxObjs = get_attachments();
    return static_cast<ImoAuxObj*>( pAuxObjs->get_item(i) );
}

//---------------------------------------------------------------------------------------
bool ImoContentObj::has_attachments()
{
    ImoAttachments* pAuxObjs = get_attachments();
    if (pAuxObjs)
        return pAuxObjs->get_num_items() > 0;
    else
        return false;
}

//---------------------------------------------------------------------------------------
int ImoContentObj::get_num_attachments()
{
    ImoAttachments* pAuxObjs = get_attachments();
    if (pAuxObjs)
        return pAuxObjs->get_num_items();
    else
        return 0;
}

//---------------------------------------------------------------------------------------
ImoAttachments* ImoContentObj::get_attachments()
{
    return dynamic_cast<ImoAttachments*>( get_child_of_type(k_imo_attachments) );
}

//---------------------------------------------------------------------------------------
void ImoContentObj::remove_attachment(ImoAuxObj* pAO)
{
    ImoAttachments* pAuxObjs = get_attachments();
    pAuxObjs->remove(pAO);
    delete pAO;
}

//---------------------------------------------------------------------------------------
ImoAuxObj* ImoContentObj::find_attachment(int type)
{
    ImoAttachments* pAuxObjs = get_attachments();
    if (!pAuxObjs)
        return NULL;
    else
        return pAuxObjs->find_item_of_type(type);
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoContentObj::get_style()
{
    if (m_pStyle)
        return m_pStyle;
    else
    {
        ImoContentObj* pParent = dynamic_cast<ImoContentObj*>( get_parent() );
        if (pParent)
            return pParent->get_style();
        else if (this->is_document())
            return (static_cast<ImoDocument*>(this))->get_default_style();
        else
            return NULL;
    }
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoContentObj::copy_style_as(const std::string& name)
{
    Document* pDoc = get_the_document();
    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, pDoc));
    pStyle->set_name(name);
    pStyle->set_parent_style(m_pStyle);
    return pStyle;
}

//---------------------------------------------------------------------------------------
void ImoContentObj::set_style(ImoStyle* pStyle)
{
    m_pStyle = pStyle;
}

//---------------------------------------------------------------------------------------
ImoDocument* ImoContentObj::get_document()
{
    if (is_document())
        return static_cast<ImoDocument*>(this);
    else
    {
        ImoObj* pParent = get_parent();
        if (pParent && pParent->is_contentobj())
            return (static_cast<ImoContentObj*>(pParent))->get_document();
        else
            throw std::runtime_error("[ImoContentObj::get_document] No parent!");
    }
}

//---------------------------------------------------------------------------------------
Document* ImoContentObj::get_the_document()
{
    if (is_document())
        return static_cast<ImoDocument*>(this)->get_owner();
    else if (is_content())
        return static_cast<ImoContent*>(this)->get_owner();
    else
    {
        ImoDocument* pDoc = get_document();
        if (pDoc)
            return pDoc->get_owner();
        else
            throw std::runtime_error("[ImoContentObj::get_the_document] No owner Document.");
    }
}

////---------------------------------------------------------------------------------------
//void ImoContentObj::add_event_handler(int eventType, EventHandler* pHandler)
//{
//    Document* pDoc = get_the_document();
//    pDoc->add_event_handler(this, eventType, pHandler);
//}
//
////---------------------------------------------------------------------------------------
//void ImoContentObj::add_event_handler(int eventType, void* pThis,
//                                      void (*pt2Func)(void* pObj, SpEventInfo event) )
//{
//    Document* pDoc = get_the_document();
//    pDoc->add_event_handler(this, eventType, pThis, pt2Func);
//}

//---------------------------------------------------------------------------------------
EventNotifier* ImoContentObj::get_event_notifier()
{
    return get_the_document();
}


//=======================================================================================
// ImoDocument implementation
//=======================================================================================
ImoDocument::ImoDocument(Document* owner, const std::string& version)
    : ImoBoxContainer(k_imo_document)
    , m_pOwner(owner)
    , m_version(version)
    , m_pageInfo()
{
}

//---------------------------------------------------------------------------------------
ImoDocument::~ImoDocument()
{
    std::list<ImoStyle*>::iterator it;
    for (it = m_privateStyles.begin(); it != m_privateStyles.end(); ++it)
        delete *it;
    m_privateStyles.clear();
}

//---------------------------------------------------------------------------------------
int ImoDocument::get_num_content_items()
{
    ImoContent* pContent = get_content();
    if (pContent)
        return pContent->get_num_children();
    else
        return 0;
}

//---------------------------------------------------------------------------------------
ImoContentObj* ImoDocument::get_content_item(int iItem)
{
    ImoContent* pContent = get_content();
    if (iItem < pContent->get_num_children())
        return dynamic_cast<ImoContentObj*>( pContent->get_child(iItem) );
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
ImoContent* ImoDocument::get_content()
{
    return dynamic_cast<ImoContent*>( get_child_of_type(k_imo_content) );
}

//---------------------------------------------------------------------------------------
void ImoDocument::add_page_info(ImoPageInfo* pPI)
{
    m_pageInfo = *pPI;
}

//---------------------------------------------------------------------------------------
ImoStyles* ImoDocument::get_styles()
{
    return dynamic_cast<ImoStyles*>( this->get_child_of_type(k_imo_styles) );
}

//---------------------------------------------------------------------------------------
void ImoDocument::add_style(ImoStyle* pStyle)
{
    get_styles()->add_style(pStyle);
}

//---------------------------------------------------------------------------------------
void ImoDocument::add_private_style(ImoStyle* pStyle)
{
    m_privateStyles.push_back(pStyle);
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoDocument::find_style(const std::string& name)
{
    return get_styles()->find_style(name);
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoDocument::get_default_style()
{
    return get_styles()->get_default_style();
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoDocument::get_style_or_default(const std::string& name)
{
    return get_styles()->get_style_or_default(name);
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoDocument::create_style(const string& name, const string& parent)
{
    Document* pDoc = get_the_document();
    ImoStyle* pParent = find_style(parent);
    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, pDoc));
    pStyle->set_name(name);
    pStyle->set_parent_style(pParent);
    add_style(pStyle);
    return pStyle;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoDocument::create_private_style(const string& parent)
{
    Document* pDoc = get_the_document();
    ImoStyle* pParent = get_style_or_default(parent);
    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, pDoc));
    pStyle->set_name("");
    pStyle->set_parent_style(pParent);
    add_private_style(pStyle);
    return pStyle;
}

//---------------------------------------------------------------------------------------
void ImoDocument::append_content_item(ImoContentObj* pItem)
{
    get_content()->append_child_imo(pItem);
}


//=======================================================================================
// ImoDynamic implementation
//=======================================================================================
ImoDynamic::~ImoDynamic()
{
    std::list<ImoParamInfo*>::iterator it;
    for (it = m_params.begin(); it != m_params.end(); ++it)
        delete *it;
    m_params.clear();

    delete m_generator;
}

//---------------------------------------------------------------------------------------
void ImoDynamic::set_generator(DynGenerator* pGenerator)
{
    delete m_generator;
    m_generator = pGenerator;
}


//=======================================================================================
// ImoHeading implementation
//=======================================================================================
void ImoHeading::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoHeading>* vHeading = NULL;
    Visitor<ImoObj>* vObj = NULL;

    vHeading = dynamic_cast<Visitor<ImoHeading>*>(&v);
    if (vHeading)
        vHeading->start_visit(this);
    else
    {
        vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
        if (vObj)
            vObj->start_visit(this);
    }

    visit_children(v);

    if (vHeading)
        vHeading->end_visit(this);
    else if (vObj)
        vObj->end_visit(this);
}



//=======================================================================================
// ImoInstrument implementation
//=======================================================================================
ImoInstrument::ImoInstrument(Document* pDoc)
    : ImoContainerObj(k_imo_instrument)
    , m_pDoc(pDoc)
    , m_name()
    , m_abbrev()
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
ImoStaffInfo* ImoInstrument::add_staff()
{
    ImoStaffInfo* pStaff = LOMSE_NEW ImoStaffInfo();
    m_staves.push_back(pStaff);
    return pStaff;
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
        m_staves.insert(it, LOMSE_NEW ImoStaffInfo(*pInfo));
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
void ImoInstrument::set_name(const string& value)
{
    m_name.set_text(value);
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_abbrev(const string& value)
{
    m_abbrev.set_text(value);
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_midi_info(ImoMidiInfo* pInfo)
{
    m_midi = *pInfo;
    delete pInfo;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_midi_instrument(int instr)
{
    m_midi.set_instrument(instr);
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_midi_channel(int channel)
{
    m_midi.set_channel(channel);
}

//---------------------------------------------------------------------------------------
ImoMusicData* ImoInstrument::get_musicdata()
{
    return dynamic_cast<ImoMusicData*>( get_child_of_type(k_imo_music_data) );
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
LUnits ImoInstrument::tenths_to_logical(Tenths value, int iStaff)
{
	return (value * get_line_spacing_for_staff(iStaff)) / 10.0f;
}

//---------------------------------------------------------------------------------------
// Instrument API
//---------------------------------------------------------------------------------------
ImoBarline* ImoInstrument::add_barline(int type, bool fVisible)
{
    ImoMusicData* pMD = get_musicdata();
    ImoBarline* pImo = static_cast<ImoBarline*>(
                                ImFactory::inject(k_imo_barline, m_pDoc) );
    pImo->set_type(type);
    pImo->set_visible(fVisible);
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoClef* ImoInstrument::add_clef(int type, int nStaff)
{
    ImoMusicData* pMD = get_musicdata();
    ImoClef* pImo = static_cast<ImoClef*>( ImFactory::inject(k_imo_clef, m_pDoc) );
    pImo->set_clef_type(type);
    pImo->set_staff(nStaff - 1);
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoKeySignature* ImoInstrument::add_key_signature(int type)
{
    ImoMusicData* pMD = get_musicdata();
    ImoKeySignature* pImo = static_cast<ImoKeySignature*>(
                                ImFactory::inject(k_imo_key_signature, m_pDoc) );
    pImo->set_key_type(type);
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoSpacer* ImoInstrument::add_spacer(Tenths space)
{
    ImoMusicData* pMD = get_musicdata();
    ImoSpacer* pImo = static_cast<ImoSpacer*>( ImFactory::inject(k_imo_spacer, m_pDoc) );
    pImo->set_width(space);
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoTimeSignature* ImoInstrument::add_time_signature(int beats, int beatType,
                                                    bool fVisible)
{
    ImoMusicData* pMD = get_musicdata();
    ImoTimeSignature* pImo = static_cast<ImoTimeSignature*>(
                                ImFactory::inject(k_imo_time_signature, m_pDoc) );
    pImo->set_beats(beats);
    pImo->set_beat_type(beatType);
    pImo->set_visible(fVisible);
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoObj* ImoInstrument::add_object(const string& ldpsource)
{
    ImoObj* pImo = m_pDoc->create_object(ldpsource);
    ImoMusicData* pMD = get_musicdata();
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::add_staff_objects(const string& ldpsource)
{
    ImoMusicData* pMD = get_musicdata();
    m_pDoc->add_staff_objects(ldpsource, pMD);
}



//=======================================================================================
// ImoInstrGroup implementation
//=======================================================================================
ImoInstrGroup::ImoInstrGroup()
    : ImoSimpleObj(k_imo_instr_group)
    , m_fJoinBarlines(true)
    , m_symbol(k_brace)
    , m_name()
    , m_abbrev()
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


//=======================================================================================
// ImoList implementation
//=======================================================================================
ImoListItem* ImoList::add_listitem(ImoStyle* pStyle)
{
    Document* pDoc = get_the_document();
    ImoListItem* pImo = static_cast<ImoListItem*>(
                            ImFactory::inject(k_imo_listitem, pDoc) );
    pImo->set_style(pStyle);
    add_item(pImo);
    return pImo;
}


//=======================================================================================
// ImoListItem implementation
//=======================================================================================
void ImoListItem::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoListItem>* vLi = NULL;
    Visitor<ImoObj>* vObj = NULL;

    vLi = dynamic_cast<Visitor<ImoListItem>*>(&v);
    if (vLi)
        vLi->start_visit(this);
    else
    {
        vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
        if (vObj)
            vObj->start_visit(this);
    }

    visit_children(v);

    if (vLi)
        vLi->end_visit(this);
    else if (vObj)
        vObj->end_visit(this);
}


//=======================================================================================
// ImoMultiColumn implementation
//=======================================================================================
void ImoMultiColumn::create_columns(int numCols)
{
    m_widths.reserve(numCols);
    float width = 100.0f / float(numCols);
    for (int i=numCols; i > 0; i--)
    {
        add_content_wrapper();
        //ImoContent* pCol = LOMSE_NEW ImoContent(k_imo_content);
        //append_child_imo(pCol);
        m_widths.push_back(width);
    }
}

//---------------------------------------------------------------------------------------
void ImoMultiColumn::set_column_width(int iCol, float percentage)
{
    m_widths[iCol] = percentage;
}

//---------------------------------------------------------------------------------------
float ImoMultiColumn::get_column_width(int iCol)
{
    return m_widths[iCol];
}


//=======================================================================================
// ImoParagraph implementation
//=======================================================================================
void ImoParagraph::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoParagraph>* vPara = NULL;
    Visitor<ImoObj>* vObj = NULL;

    vPara = dynamic_cast<Visitor<ImoParagraph>*>(&v);
    if (vPara)
        vPara->start_visit(this);
    else
    {
        vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
        if (vObj)
            vObj->start_visit(this);
    }

    visit_children(v);

    if (vPara)
        vPara->end_visit(this);
    else if (vObj)
        vObj->end_visit(this);
}


//=======================================================================================
// ImoParamInfo implementation
//=======================================================================================
bool ImoParamInfo::get_value_as_int(int* pNumber)
{
    //http://www.codeguru.com/forum/showthread.php?t=231054
    std::istringstream iss(m_value);
    if ((iss >> std::dec >> *pNumber).fail())
        return false;   //error
    else
        return true;    //ok
}

//=======================================================================================
// ImoScore implementation
//=======================================================================================

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

//---------------------------------------------------------------------------------------
static const BoolOption m_BoolOptions[] =
{
    {"Score.FillPageWithEmptyStaves", false },
    {"StaffLines.StopAtFinalBarline", true },
    {"Score.JustifyFinalBarline", false },
    {"StaffLines.Hide", false },
    {"Staff.DrawLeftBarline", true },
};

//static StringOption m_StringOptions[] = {};

//---------------------------------------------------------------------------------------
static const FloatOption m_FloatOptions[] =
{
    {"Render.SpacingFactor", 0.547f },
        // Note spacing is proportional to duration.
        // As the duration of quarter note is 64 (duration units), I am
        // going to map it to 35 tenths. This gives a conversion factor
        // of 35/64 = 0.547
};

//---------------------------------------------------------------------------------------
static const LongOption m_LongOptions[] =
{
    {"Staff.UpperLegerLines.Displacement", 0L },
    {"Render.SpacingMethod", long(k_spacing_proportional) },
    {"Render.SpacingValue", 35L },       // 15 tenths (1.5 lines) [add 20 to desired value]
};

//---------------------------------------------------------------------------------------
ImoScore::ImoScore(Document* pDoc)
    : ImoBoxLevelObj(k_imo_score)
    , m_version("")
    , m_pDoc(pDoc)
    , m_pColStaffObjs(NULL)
    , m_pMidiTable(NULL)
    , m_systemInfoFirst()
    , m_systemInfoOther()
    , m_pageInfo()
{
    append_child_imo( ImFactory::inject(k_imo_options, m_pDoc) );
    append_child_imo( ImFactory::inject(k_imo_instruments, m_pDoc) );
    set_defaults_for_system_info();
    set_defaults_for_options();
}

//---------------------------------------------------------------------------------------
ImoScore::~ImoScore()
{
    delete m_pColStaffObjs;
    delete_text_styles();
    delete m_pMidiTable;
}

//---------------------------------------------------------------------------------------
void ImoScore::set_defaults_for_system_info()
{
    m_systemInfoFirst.set_first(true);
    m_systemInfoFirst.set_top_system_distance(1000.0f);     //half system distance
    m_systemInfoFirst.set_system_distance(2000.0f);         //2 cm

    m_systemInfoOther.set_first(true);
    m_systemInfoOther.set_top_system_distance(1500.0f);     //1.5 cm
    m_systemInfoOther.set_system_distance(2000.0f);         //2 cm
}

//---------------------------------------------------------------------------------------
void ImoScore::set_defaults_for_options()
{
    //bool
    for (unsigned i=0; i < sizeof(m_BoolOptions)/sizeof(BoolOption); i++)
    {
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
                                    ImFactory::inject(k_imo_option, m_pDoc) );
        pOpt->set_name(m_BoolOptions[i].sOptName);
        pOpt->set_type(ImoOptionInfo::k_boolean);
        pOpt->set_bool_value( m_BoolOptions[i].fBoolValue );
        add_option(pOpt);
    }

    //long
    for (unsigned i=0; i < sizeof(m_LongOptions)/sizeof(LongOption); i++)
    {
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
                                    ImFactory::inject(k_imo_option, m_pDoc) );
        pOpt->set_name(m_LongOptions[i].sOptName);
        pOpt->set_type(ImoOptionInfo::k_number_long);
        pOpt->set_long_value( m_LongOptions[i].nLongValue );
        add_option(pOpt);
    }

    //double
    for (unsigned i=0; i < sizeof(m_FloatOptions)/sizeof(FloatOption); i++)
    {
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
                                    ImFactory::inject(k_imo_option, m_pDoc) );
        pOpt->set_name(m_FloatOptions[i].sOptName);
        pOpt->set_type(ImoOptionInfo::k_number_float);
        pOpt->set_float_value( m_FloatOptions[i].rFloatValue );
        add_option(pOpt);
    }
}
//---------------------------------------------------------------------------------------
void ImoScore::set_staffobjs_table(ColStaffObjs* pColStaffObjs)
{
    delete m_pColStaffObjs;;
    m_pColStaffObjs = pColStaffObjs;
}

//---------------------------------------------------------------------------------------
void ImoScore::delete_text_styles()
{
    map<std::string, ImoStyle*>::const_iterator it;
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
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
                                    ImFactory::inject(k_imo_option, m_pDoc) );
        pOpt->set_name(name);
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
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
                                    ImFactory::inject(k_imo_option, m_pDoc) );
        pOpt->set_name(name);
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
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
                                    ImFactory::inject(k_imo_option, m_pDoc) );
        pOpt->set_name(name);
        pOpt->set_type(ImoOptionInfo::k_number_long);
        pOpt->set_long_value(value);
        add_option(pOpt);
     }
}

//---------------------------------------------------------------------------------------
ImoInstruments* ImoScore::get_instruments()
{
    return dynamic_cast<ImoInstruments*>( get_child_of_type(k_imo_instruments) );
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
    return pColInstr->append_child_imo(pInstr);
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
    return dynamic_cast<ImoOptions*>( get_child_of_type(k_imo_options) );
}

//---------------------------------------------------------------------------------------
void ImoScore::add_option(ImoOptionInfo* pOpt)
{
    ImoOptions* pColOpts = get_options();
    return pColOpts->append_child_imo(pOpt);
}

//---------------------------------------------------------------------------------------
void ImoScore::add_or_replace_option(ImoOptionInfo* pOpt)
{
    ImoOptionInfo* pOldOpt = get_option( pOpt->get_name() );
    if (pOldOpt)
    {
        ImoOptions* pColOpts = get_options();
        pColOpts->remove_child_imo(pOldOpt);
        delete pOldOpt;
    }
    add_option(pOpt);
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
    return dynamic_cast<ImoInstrGroups*>( get_child_of_type(k_imo_instrument_groups) );
}

//---------------------------------------------------------------------------------------
void ImoScore::add_instruments_group(ImoInstrGroup* pGroup)
{
    ImoInstrGroups* pGroups = get_instrument_groups();
    if (!pGroups)
    {
        pGroups = static_cast<ImoInstrGroups*>(
                        ImFactory::inject(k_imo_instrument_groups, m_pDoc) );
        append_child_imo(pGroups);
    }
    pGroups->append_child_imo(pGroup);

    for (int i=0; i < pGroup->get_num_instruments(); i++)
        add_instrument(pGroup->get_instrument(i));
}

//---------------------------------------------------------------------------------------
void ImoScore::add_title(ImoScoreTitle* pTitle)
{
    m_titles.push_back(pTitle);
    append_child_imo(pTitle);
}

//---------------------------------------------------------------------------------------
void ImoScore::add_style(ImoStyle* pStyle)
{
    m_nameToStyle[pStyle->get_name()] = pStyle;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoScore::find_style(const std::string& name)
{
	map<std::string, ImoStyle*>::const_iterator it
        = m_nameToStyle.find(name);
	if (it != m_nameToStyle.end())
		return it->second;
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoScore::get_style_or_default(const std::string& name)
{
    ImoStyle* pStyle = find_style(name);
	if (pStyle)
		return pStyle;
    else
        return get_default_style();
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoScore::get_default_style()
{
    ImoStyle* pStyle = find_style("Default style");
    if (pStyle)
		return pStyle;
    else
        return create_default_style();
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoScore::create_default_style()
{
    //TODO: all code related to creating/storing/findig styles should be
    //removed and use Document Styles instead
	ImoStyle* pDefStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDoc));
    pDefStyle->set_name("Default style");

        //font properties
    pDefStyle->set_string_property(ImoStyle::k_font_name, "Liberation serif");
    pDefStyle->set_float_property(ImoStyle::k_font_size, 12.0f);
    pDefStyle->set_int_property(ImoStyle::k_font_style, ImoStyle::k_font_normal);
    pDefStyle->set_int_property(ImoStyle::k_font_weight, ImoStyle::k_font_normal);
        //text
    pDefStyle->set_int_property(ImoStyle::k_word_spacing, ImoStyle::k_spacing_normal);
    pDefStyle->set_int_property(ImoStyle::k_text_decoration, ImoStyle::k_decoration_none);
    pDefStyle->set_int_property(ImoStyle::k_vertical_align, ImoStyle::k_valign_baseline);
    pDefStyle->set_int_property(ImoStyle::k_text_align, ImoStyle::k_align_left);
    pDefStyle->set_lunits_property(ImoStyle::k_text_indent_length, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_word_spacing_length, 0.0f);   //not applicable
    pDefStyle->set_float_property(ImoStyle::k_line_height, 1.5f);
        //color and background
    pDefStyle->set_color_property(ImoStyle::k_color, Color(0,0,0));
    pDefStyle->set_color_property(ImoStyle::k_background_color, Color(255,255,255));
        //margin
    pDefStyle->set_lunits_property(ImoStyle::k_margin_top, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_margin_bottom, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_margin_left, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_margin_right, 0.0f);
        //padding
    pDefStyle->set_lunits_property(ImoStyle::k_padding_top, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_padding_bottom, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_padding_left, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_padding_right, 0.0f);
        ////border
    //pDefStyle->set_lunits_property(ImoStyle::k_border_top, 0.0f);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_bottom, 0.0f);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_left, 0.0f);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_right, 0.0f);
        //border width
    pDefStyle->set_lunits_property(ImoStyle::k_border_width_top, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_border_width_bottom, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_border_width_left, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_border_width_right, 0.0f);
    m_nameToStyle[pDefStyle->get_name()] = pDefStyle;
    add_style(pDefStyle);
    return pDefStyle;
}

//---------------------------------------------------------------------------------------
void ImoScore::add_required_text_styles()
{
    ImoStyle* pDefStyle = get_default_style();

    //For tuplets numbers
    if (find_style("Tuplet numbers") == NULL)
    {
	    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDoc));
        pStyle->set_name("Tuplet numbers");
        pStyle->set_parent_style(pDefStyle);
	    pStyle->set_string_property(ImoStyle::k_font_name, "Liberation serif");
	    pStyle->set_float_property(ImoStyle::k_font_size, 11.0f);
        pStyle->set_int_property(ImoStyle::k_font_style, ImoStyle::k_italic);
        pStyle->set_int_property(ImoStyle::k_font_weight, ImoStyle::k_font_normal);
        add_style(pStyle);
    }

    //For instrument and group names and abbreviations
    if (find_style("Instrument names") == NULL)
    {
	    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDoc));
        pStyle->set_name("Instrument names");
        pStyle->set_parent_style(pDefStyle);
	    pStyle->set_string_property(ImoStyle::k_font_name, "Liberation serif");
	    pStyle->set_float_property(ImoStyle::k_font_size, 14.0f);
        add_style(pStyle);
    }

    //"Lyrics" - for lyrics
}

//---------------------------------------------------------------------------------------
SoundEventsTable* ImoScore::get_midi_table()
{
    if (!m_pMidiTable)
    {
        m_pMidiTable = LOMSE_NEW SoundEventsTable(this);
        m_pMidiTable->create_table();
    }
    return m_pMidiTable;
}

//---------------------------------------------------------------------------------------
// Score API
//---------------------------------------------------------------------------------------
ImoInstrument* ImoScore::add_instrument()
{
    ImoInstrument* pInstr = static_cast<ImoInstrument*>(
                                ImFactory::inject(k_imo_instrument, m_pDoc) );
    add_instrument(pInstr);
    ImoMusicData* pMD = static_cast<ImoMusicData*>(
                                ImFactory::inject(k_imo_music_data, m_pDoc));
    pInstr->append_child_imo(pMD);
    return pInstr;
}

//---------------------------------------------------------------------------------------
void ImoScore::close()
{
    //ColStaffObjsBuilder builder;
    //builder.build(this);
    ModelBuilder builder;
    builder.structurize(this);
}



//=======================================================================================
// ImoScoreLine implementation
//=======================================================================================

//ImoScoreLine::ImoScoreLine(lmScoreObj* pOwner, long nID, lmTenths xStart, lmTenths yStart,
//                         lmTenths xEnd, lmTenths yEnd, lmTenths nWidth,
//                         lmELineCap nStartCap, lmELineCap nEndCap, lmELineStyle nStyle,
//                         wxColour nColor)
//    : lmAuxObj(pOwner, nID, lm_eSO_Line, lmDRAGGABLE)
//    , m_txStart(xStart)
//    , m_tyStart(yStart)
//    , m_txEnd(xEnd)
//    , m_tyEnd(yEnd)
//    , m_tWidth(nWidth)
//	, m_nColor(nColor)
//	, m_nEdge(lm_eEdgeNormal)
//    , m_nStyle(nStyle)
//    , m_nStartCap(nStartCap)
//    , m_nEndCap(nEndCap)
//{
//}



//=======================================================================================
// ImoScorePlayer implementation
//=======================================================================================
ImoScorePlayer::ImoScorePlayer()
    : ImoControl(k_imo_score_player)
    , m_pPlayer(NULL)
    , m_pScore(NULL)
{
}

//---------------------------------------------------------------------------------------
ImoScorePlayer::~ImoScorePlayer()
{
    delete m_pPlayer;
}

//---------------------------------------------------------------------------------------
void ImoScorePlayer::attach_player(ScorePlayerCtrl* pPlayer)
{
    m_pPlayer = pPlayer;
    ImoControl::attach_control(m_pPlayer);
}



//=======================================================================================
// ImoStyles implementation
//=======================================================================================
ImoStyles::ImoStyles(Document* pDoc)
    : ImoCollection(k_imo_styles)
    , m_pDoc(pDoc)
{
    create_default_styles();
}

//---------------------------------------------------------------------------------------
ImoStyles::~ImoStyles()
{
    delete_text_styles();
}

//---------------------------------------------------------------------------------------
void ImoStyles::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoObj>* vObj = NULL;

    vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
    if (vObj)
        vObj->start_visit(this);

    //visit_children
	map<std::string, ImoStyle*>::iterator it;
    for(it = m_nameToStyle.begin(); it != m_nameToStyle.end(); ++it)
        (it->second)->accept_visitor(v);

    if (vObj)
        vObj->end_visit(this);
}

//---------------------------------------------------------------------------------------
void ImoStyles::add_style(ImoStyle* pStyle)
{
    m_nameToStyle[pStyle->get_name()] = pStyle;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoStyles::find_style(const std::string& name)
{
	map<std::string, ImoStyle*>::const_iterator it
        = m_nameToStyle.find(name);
	if (it != m_nameToStyle.end())
		return it->second;
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoStyles::get_style_or_default(const std::string& name)
{
    ImoStyle* pStyle = find_style(name);
	if (pStyle)
		return pStyle;
    else
        return get_default_style();
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoStyles::get_default_style()
{
    ImoStyle* pStyle = find_style("Default style");
    if (pStyle)
		return pStyle;
    else
        return create_default_styles();
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoStyles::create_default_styles()
{
    // Default style
	ImoStyle* pDefStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDoc));
    pDefStyle->set_name("Default style");

        //font properties
    pDefStyle->set_string_property(ImoStyle::k_font_name, "Liberation serif");
    pDefStyle->set_float_property(ImoStyle::k_font_size, 12.0f);
    pDefStyle->set_int_property(ImoStyle::k_font_style, ImoStyle::k_font_normal);
    pDefStyle->set_int_property(ImoStyle::k_font_weight, ImoStyle::k_font_normal);
        //text
    pDefStyle->set_int_property(ImoStyle::k_word_spacing, ImoStyle::k_spacing_normal);
    pDefStyle->set_int_property(ImoStyle::k_text_decoration, ImoStyle::k_decoration_none);
    pDefStyle->set_int_property(ImoStyle::k_vertical_align, ImoStyle::k_valign_baseline);
    pDefStyle->set_int_property(ImoStyle::k_text_align, ImoStyle::k_align_left);
    pDefStyle->set_lunits_property(ImoStyle::k_text_indent_length, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_word_spacing_length, 0.0f);   //not applicable
    pDefStyle->set_float_property(ImoStyle::k_line_height, 1.5f);
        //color and background
    pDefStyle->set_color_property(ImoStyle::k_color, Color(0,0,0));
    pDefStyle->set_color_property(ImoStyle::k_background_color, Color(255,255,255));
        //margin
    pDefStyle->set_lunits_property(ImoStyle::k_margin_top, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_margin_bottom, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_margin_left, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_margin_right, 0.0f);
        //padding
    pDefStyle->set_lunits_property(ImoStyle::k_padding_top, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_padding_bottom, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_padding_left, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_padding_right, 0.0f);
        ////border
    //pDefStyle->set_lunits_property(ImoStyle::k_border_top, 0.0f);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_bottom, 0.0f);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_left, 0.0f);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_right, 0.0f);
        //border width
    pDefStyle->set_lunits_property(ImoStyle::k_border_width_top, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_border_width_bottom, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_border_width_left, 0.0f);
    pDefStyle->set_lunits_property(ImoStyle::k_border_width_right, 0.0f);

    m_nameToStyle[pDefStyle->get_name()] = pDefStyle;

    return pDefStyle;
}

//---------------------------------------------------------------------------------------
void ImoStyles::delete_text_styles()
{
    map<std::string, ImoStyle*>::const_iterator it;
    for (it = m_nameToStyle.begin(); it != m_nameToStyle.end(); ++it)
        delete it->second;

    m_nameToStyle.clear();
}



//=======================================================================================
// ImoPageInfo implementation
//=======================================================================================
ImoPageInfo::ImoPageInfo()
    : ImoSimpleObj(k_imo_page_info)
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
    : ImoSimpleObj(k_imo_page_info)
    , m_uLeftMargin( dto.get_left_margin() )
    , m_uRightMargin( dto.get_right_margin() )
    , m_uTopMargin( dto.get_top_margin() )
    , m_uBottomMargin( dto.get_bottom_margin() )
    , m_uBindingMargin( dto.get_binding_margin() )
    , m_uPageSize( dto.get_page_size() )
    , m_fPortrait( dto.is_portrait() )
{
}


//=======================================================================================
// ImoSlur implementation
//=======================================================================================
ImoSlur::~ImoSlur()
{
}

//---------------------------------------------------------------------------------------
ImoNote* ImoSlur::get_start_note()
{
    return static_cast<ImoNote*>( get_start_object() );
}

//---------------------------------------------------------------------------------------
ImoNote* ImoSlur::get_end_note()
{
    return static_cast<ImoNote*>( get_end_object() );
}


//=======================================================================================
// ImoSlurData implementation
//=======================================================================================
ImoSlurData::ImoSlurData(ImoSlurDto* pDto)
    : ImoRelDataObj(k_imo_slur_data)
    , m_slurType( pDto->get_slur_type() )
    , m_slurNum( pDto->get_slur_number() )
    , m_pBezier( pDto->get_bezier() )
    , m_color( pDto->get_color() )
{
}



//=======================================================================================
// ImoSlurDto implementation
//=======================================================================================
ImoSlurDto::~ImoSlurDto()
{
    delete m_pBezier;
}

//---------------------------------------------------------------------------------------
int ImoSlurDto::get_line_number()
{
    if (m_pSlurElm)
        return m_pSlurElm->get_line_number();
    else
        return 0;
}


//=======================================================================================
// ImoSystemInfo implementation
//=======================================================================================
ImoSystemInfo::ImoSystemInfo()
    : ImoSimpleObj(k_imo_system_info)
    , m_fFirst(true)
    , m_leftMargin(0.0f)
    , m_rightMargin(0.0f)
    , m_systemDistance(0.0f)
    , m_topSystemDistance(0.0f)
{
}

//---------------------------------------------------------------------------------------
ImoSystemInfo::ImoSystemInfo(ImoSystemInfo& dto)
    : ImoSimpleObj(k_imo_system_info)
    , m_fFirst( dto.is_first() )
    , m_leftMargin( dto.get_left_margin() )
    , m_rightMargin( dto.get_right_margin() )
    , m_systemDistance( dto.get_system_distance() )
    , m_topSystemDistance( dto.get_top_system_distance() )
{
}

//=======================================================================================
// ImoTextInfo implementation
//=======================================================================================
const std::string& ImoTextInfo::get_font_name()
{
    return m_pStyle->get_string_property(ImoStyle::k_font_name);
}

//---------------------------------------------------------------------------------------
float ImoTextInfo::get_font_size()
{
    return m_pStyle->get_float_property(ImoStyle::k_font_size);
}

//---------------------------------------------------------------------------------------
int ImoTextInfo::get_font_style()
{
    return m_pStyle->get_int_property(ImoStyle::k_font_style);
}

//---------------------------------------------------------------------------------------
int ImoTextInfo::get_font_weight()
{
    return m_pStyle->get_int_property(ImoStyle::k_font_weight);
}

//---------------------------------------------------------------------------------------
Color ImoTextInfo::get_color()
{
    return m_pStyle->get_color_property(ImoStyle::k_color);
}


//=======================================================================================
// ImoTie implementation
//=======================================================================================
ImoNote* ImoTie::get_start_note()
{
    return static_cast<ImoNote*>( get_start_object() );
}

//---------------------------------------------------------------------------------------
ImoNote* ImoTie::get_end_note()
{
    return static_cast<ImoNote*>( get_end_object() );
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoTie::get_start_bezier()
{
    ImoTieData* pData = static_cast<ImoTieData*>( get_start_data() );
    return pData->get_bezier();
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoTie::get_stop_bezier()
{
    ImoTieData* pData = static_cast<ImoTieData*>( get_end_data() );
    return pData->get_bezier();
}



//=======================================================================================
// ImoTieData implementation
//=======================================================================================
ImoTieData::ImoTieData(ImoTieDto* pDto)
    : ImoRelDataObj(k_imo_tie_data)
    , m_fStart( pDto->is_start() )
    , m_tieNum( pDto->get_tie_number() )
    , m_pBezier(NULL)
{
    if (pDto->get_bezier())
        m_pBezier = LOMSE_NEW ImoBezierInfo( pDto->get_bezier() );
}

//---------------------------------------------------------------------------------------
ImoTieData::~ImoTieData()
{
    delete m_pBezier;
}


//=======================================================================================
// ImoTieDto implementation
//=======================================================================================
ImoTieDto::~ImoTieDto()
{
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


//=======================================================================================
// ImoTimeSignature implementation
//=======================================================================================
int ImoTimeSignature::get_num_pulses()
{
    //returns the number of pulses (metronome pulses) implied by this TS

    return (is_compound_meter() ? m_beats / 3 : m_beats);
}

//---------------------------------------------------------------------------------------
float ImoTimeSignature::get_beat_duration()
{
    // returns beat duration (in LDP notes duration units)

    switch(m_beatType)
    {
        case 1:
            return pow(2.0f, (10 - k_whole));
        case 2:
            return pow(2.0f, (10 - k_half));
        case 4:
            return pow(2.0f, (10 - k_quarter));
        case 8:
            return pow(2.0f, (10 - k_eighth));
        case 16:
            return pow(2.0f, (10 - k_16th));
        default:
            return 64.0f;     //compiler happy
    }
}

//---------------------------------------------------------------------------------------
float ImoTimeSignature::get_measure_duration()
{
    return m_beats * get_beat_duration();
}



//=======================================================================================
// ImoTupletData implementation
//=======================================================================================
ImoTupletData::ImoTupletData(ImoTupletDto* pDto)
    : ImoRelDataObj(k_imo_tuplet_data)
{
}


//=======================================================================================
// ImoTupletDto implementation
//=======================================================================================
ImoTupletDto::ImoTupletDto()
    : ImoSimpleObj(k_imo_tuplet_dto)
    , m_tupletType(ImoTupletDto::k_unknown)
    , m_nActualNum(0)
    , m_nNormalNum(0)
    , m_nShowBracket(k_yesno_default)
    , m_nPlacement(k_placement_default)
    , m_nShowNumber(ImoTuplet::k_number_actual)
    , m_pTupletElm(NULL)
    , m_pNR(NULL)
{
}

//---------------------------------------------------------------------------------------
ImoTupletDto::ImoTupletDto(LdpElement* pTupletElm)
    : ImoSimpleObj(k_imo_tuplet_dto)
    , m_tupletType(ImoTupletDto::k_unknown)
    , m_nActualNum(0)
    , m_nNormalNum(0)
    , m_nShowBracket(k_yesno_default)
    , m_nPlacement(k_placement_default)
    , m_nShowNumber(ImoTuplet::k_number_actual)
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
    : ImoRelObj(k_imo_tuplet)
    , m_nActualNum(dto->get_actual_number())
    , m_nNormalNum(dto->get_normal_number())
    , m_nShowBracket(dto->get_show_bracket())
    , m_nShowNumber(dto->get_show_number())
    , m_nPlacement(dto->get_placement())
{
}


//=======================================================================================
// ImoControl implementation
//=======================================================================================
GmoBoxControl* ImoControl::layout(LibraryScope& libraryScope, UPoint pos)
{
    return m_ctrol->layout(libraryScope, pos);
}

//---------------------------------------------------------------------------------------
USize ImoControl::measure()
{
    return m_ctrol->measure();
}



//=======================================================================================
// global functions related to notes
//=======================================================================================
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
        case 'l':     return k_longa;
        case 'b':     return k_breve;
        case 'w':     return k_whole;
        case 'h':     return k_half;
        case 'q':     return k_quarter;
        case 'e':     return k_eighth;
        case 's':     return k_16th;
        case 't':     return k_32th;
        case 'i':     return k_64th;
        case 'o':     return k_128th;
        case 'f':     return k_256th;
        default:
            return -1;
    }
}

//---------------------------------------------------------------------------------------
NoteTypeAndDots ldp_duration_to_components(const string& duration)
{
    // Return struct with noteType and dots.
    // If error, noteType is set to unknown and dots to zero

    size_t size = duration.length();
    if (size == 0)
        return NoteTypeAndDots(k_unknown_notetype, 0);   //error

    //duration
    int noteType = to_note_type(duration[0]);
    if (noteType == -1)
        return NoteTypeAndDots(k_unknown_notetype, 0);   //error

    //dots
    int dots = 0;
    for (size_t i=1; i < size; i++)
    {
        if (duration[i] == '.')
            dots++;
        else
            return NoteTypeAndDots(k_unknown_notetype, 0);   //error
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
        case k_longa:   rDuration=1024; break;    //  0
        case k_breve:   rDuration=512;  break;    //  1
        case k_whole:   rDuration=256;  break;    //  2
        case k_half:    rDuration=128;  break;    //  3
        case k_quarter: rDuration=64;   break;    //  4
        case k_eighth:  rDuration=32;   break;    //  5
        case k_16th:    rDuration=16;   break;    //  6
        case k_32th:    rDuration=8;    break;    //  7
        case k_64th:    rDuration=4;    break;    //  8
        case k_128th:   rDuration=2;    break;    //  9
        case k_256th:   rDuration=1;    break;    //  10
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

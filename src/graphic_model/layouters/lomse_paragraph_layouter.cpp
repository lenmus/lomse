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

#include "lomse_paragraph_layouter.h"
#include "lomse_build_options.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_text_engraver.h"
#include "lomse_shape_text.h"
#include "lomse_shapes.h"
#include "lomse_calligrapher.h"

//other
#include <boost/format.hpp>

namespace lomse
{

//=======================================================================================
// ParagraphLayouter implementation
//=======================================================================================
ParagraphLayouter::ParagraphLayouter(ImoContentObj* pItem, Layouter* pParent,
                                     GraphicModel* pGModel, LibraryScope& libraryScope,
                                     ImoStyles* pStyles)
    : Layouter(pItem, pParent, pGModel, libraryScope, pStyles)
    , m_libraryScope(libraryScope)
    , m_pPara( dynamic_cast<ImoBoxContent*>(pItem) )
{
}

//---------------------------------------------------------------------------------------
ParagraphLayouter::~ParagraphLayouter()
{
    std::list<Engrouter*>::iterator it;
    for (it = m_engrouters.begin(); it != m_engrouters.end(); ++it)
        delete *it;
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::prepare_to_start_layout()
{
    Layouter::prepare_to_start_layout();
    create_engrouters();
    point_to_first_engrouter();
    initialize_lines();
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::layout_in_box()
{
    //AWARE: This method is invoked to layout a page. If there are more pages to
    //layout, it will be invoked more times. Therefore, this method must not initialize
    //anything. All initializations must be done in 'prepare_to_start_layout()'.
    //layout_in_box() method must always continue layouting from current state.

    set_cursor_and_available_space(m_pItemMainBox);
    page_initializations(m_pItemMainBox);

    if (!is_line_ready())
        prepare_line();

    if (!is_line_ready())
    {
        //empty paragraph
        advance_current_line_space(m_pageCursor.x);
    }

    while(is_line_ready() && enough_space_in_box())
    {
        add_line();
        prepare_line();
    }

    bool fMoreText = is_line_ready();
    if (!fMoreText)
        m_pItemMainBox->add_shapes_to_tables();

    set_layout_is_finished( !fMoreText );
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::create_main_box(GmoBox* pParentBox, UPoint pos,
                                        LUnits width, LUnits height)
{
    m_pItemMainBox = LOMSE_NEW GmoBoxParagraph(m_pPara);
    pParentBox->add_child_box(m_pItemMainBox);

    m_pItemMainBox->set_origin(pos);
    m_pItemMainBox->set_width(width);
    m_pItemMainBox->set_height(height);
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::create_engrouters()
{
    EngroutersCreator creator(m_engrouters, m_libraryScope);
    TreeNode<ImoObj>::children_iterator it;
    for (it = m_pPara->begin(); it != m_pPara->end(); ++it)
        creator.create_engrouters( dynamic_cast<ImoInlineObj*>( *it ) );
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::prepare_line()
{
    //After execution:
    //  m_itStart - will point to first engrouter to include or m_engrouters.end()
    //              if no more engrouters
    //  m_itEnd - will point to after last engrouter to include
    //  m_lineHeight and other reference points will be updated to final data

    m_itStart = more_engrouters() ? m_itEngrouters :  m_engrouters.end();
    m_itEnd = m_engrouters.end();
    UPoint pos = m_pageCursor;
    m_lineWidth = 0.0f;

    initialize_line_references();

    while(more_engrouters() && space_in_line())
    {
        m_lineWidth += add_engrouter_to_line();
        next_engrouter();
    }
    m_itEnd = m_itEngrouters;

    m_pageCursor = pos;     //restore cursor
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::add_line()
{
    LUnits left = m_pageCursor.x;       //save left margin

    //horizontal alignment
    ImoStyle* pStyle = m_pPara->get_style();
    switch (pStyle->get_int_property(ImoStyle::k_text_align))
    {
        case ImoStyle::k_align_left:
            break;

        case ImoStyle::k_align_right:
            m_pageCursor.x += m_availableWidth - m_lineWidth;
            break;

        case ImoStyle::k_align_center:
            m_pageCursor.x += (m_availableWidth - m_lineWidth) / 2.0f;
            break;

        case ImoStyle::k_align_justify:
            //TODO: For now just align left
            break;
    }

    std::list<Engrouter*>::iterator it;
    for(it = m_itStart; it != m_itEnd; ++it)
    {
        add_engrouter_shape(*it, m_lineRefs.lineHeight);
    }
    advance_current_line_space(left);
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::advance_current_line_space(LUnits left)
{
    m_pageCursor.x = left;
    m_pageCursor.y += m_lineRefs.lineHeight;
    m_availableSpace = m_availableWidth;

    m_pItemMainBox->set_height( m_pItemMainBox->get_height() + m_lineRefs.lineHeight );
}

//---------------------------------------------------------------------------------------
LUnits ParagraphLayouter::add_engrouter_to_line()
{
    //Current engrouter is going to be included in current line. Here we proceed to do
    //vertical alignement of the engrouter.
    //This could imply changes in the reference lines for the line. But as engrouters
    //are not yet engraved, these changes doesn't matter: we are only computing the
    //final reference lines for the line. Later, when the engrouters are engraved, they will
    //be properlly positioned on final reference lines.
    //Returns: width of added item

    Engrouter* pEngrouter = get_current_engrouter();

    LUnits shift = pEngrouter->shift_for_vertical_alignment(m_lineRefs);
    bool fUpdateText = dynamic_cast<WordEngrouter*>(pEngrouter) != NULL;
    if (fUpdateText)
    {
        int valign = pEngrouter->get_style()->get_int_property(ImoStyle::k_vertical_align);
        fUpdateText = valign == ImoStyle::k_valign_baseline;
    }
    update_line_references(pEngrouter->get_reference_lines(), shift, fUpdateText);

    LUnits width = pEngrouter->get_width();
    m_pageCursor.x += width;
    m_availableSpace -= width;
    return width;
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::initialize_line_references()
{
    //line height: 'strut' line height
    ImoStyle* pStyle = m_pPara->get_style();

    TextMeter meter(m_libraryScope);
    meter.select_font(pStyle->get_string_property(ImoStyle::k_font_name)
                      , pStyle->get_float_property(ImoStyle::k_font_size)
                      , pStyle->is_bold()
                      , pStyle->is_italic() );
    LUnits fontHeight = meter.get_font_height();

    float lineHeight = pStyle->get_float_property(ImoStyle::k_line_height);
    m_lineRefs.lineHeight = fontHeight * LUnits(lineHeight);

    //half-leading
    LUnits halfLeading = (m_lineRefs.lineHeight - fontHeight) / 2.0f;

    m_lineRefs.textTop = halfLeading;
    m_lineRefs.baseline = halfLeading + meter.get_ascender();

    //text-bottom
    m_lineRefs.textBottom = m_lineRefs.baseline - meter.get_descender();

    //middle line (center of 'x' glyph)
    URect rect = meter.bounding_rectangle('x');
    m_lineRefs.middleline = m_lineRefs.baseline - rect.height / 2.0f;
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::update_line_references(LineReferences& engr, LUnits shift,
                                               bool fUpdateText)
{
    if (fUpdateText)
    {
        if (shift < 0.0f)
        {
            m_lineRefs.textTop = (m_lineRefs.textTop - shift + engr.textTop) / 2.0f;
            m_lineRefs.middleline = (m_lineRefs.middleline - shift + engr.middleline) / 2.0f;
            m_lineRefs.baseline = max(m_lineRefs.baseline - shift, engr.baseline);
            m_lineRefs.textBottom = (m_lineRefs.textBottom - shift + engr.textBottom) / 2.0f;
            m_lineRefs.lineHeight = max(m_lineRefs.lineHeight - shift, engr.lineHeight);
            m_lineRefs.supperLine = engr.supperLine;
            m_lineRefs.subLine = engr.subLine;
        }
        else
        {
            m_lineRefs.textTop = (m_lineRefs.textTop + shift + engr.textTop) / 2.0f;
            m_lineRefs.middleline = (m_lineRefs.middleline + shift + engr.middleline) / 2.0f;
            m_lineRefs.baseline = max(m_lineRefs.baseline, engr.baseline + shift);
            m_lineRefs.textBottom = (m_lineRefs.textBottom + shift + engr.textBottom) / 2.0f;
            m_lineRefs.lineHeight = max(m_lineRefs.lineHeight, engr.lineHeight + shift);
            m_lineRefs.supperLine = engr.supperLine + shift;
            m_lineRefs.subLine = engr.subLine + shift;
        }
    }
    else
    {
        if (shift < 0.0f)
        {
            //m_lineRefs.textTop        //no change
            m_lineRefs.middleline = (m_lineRefs.middleline - shift + engr.middleline) / 2.0f;
            //m_lineRefs.baseline       //no change
            //m_lineRefs.textBottom     //no change
            m_lineRefs.lineHeight = max(m_lineRefs.lineHeight - shift, engr.lineHeight);
            //m_lineRefs.supperLine     //no change
            //m_lineRefs.subLine        //no change
        }
        else
        {
            //m_lineRefs.textTop        no change
            m_lineRefs.middleline = (m_lineRefs.middleline + shift + engr.middleline) / 2.0f;
            //m_lineRefs.baseline       no change
            //m_lineRefs.textBottom     no change
            m_lineRefs.lineHeight = max(m_lineRefs.lineHeight, engr.lineHeight + shift);
            //m_lineRefs.supperLine     no change
            //m_lineRefs.subLine        no change
        }
    }

}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::add_engrouter_shape(Engrouter* pEngrouter, LUnits lineHeight)
{
    GmoObj* pGmo = pEngrouter->create_gm_object(m_pageCursor, get_line_refs());

    if (pGmo->is_shape())
    {
        GmoShape* pShape = dynamic_cast<GmoShape*>(pGmo);
        m_pItemMainBox->add_shape(pShape, GmoShape::k_layer_staff);
        m_pageCursor.x += pShape->get_width();
    }
    else if (pGmo->is_box())
    {
        GmoBox* pBox = dynamic_cast<GmoBox*>(pGmo);
        m_pItemMainBox->add_child_box(pBox);
        m_pageCursor.x += pBox->get_width();
    }
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::page_initializations(GmoBox* pMainBox)
{
    m_pItemMainBox = pMainBox;

    //no height until content added
    ImoStyle* pStyle = m_pPara->get_style();
    if (pStyle)
        m_pItemMainBox->set_height( pStyle->get_lunits_property(ImoStyle::k_margin_top) + pStyle->get_lunits_property(ImoStyle::k_border_width_top)
                                    + pStyle->get_lunits_property(ImoStyle::k_padding_top) );
    else
        m_pItemMainBox->set_height(0.0f);

    m_availableSpace = m_availableWidth;
}

//---------------------------------------------------------------------------------------
bool ParagraphLayouter::enough_space_in_box()
{
    return m_availableHeight >= m_pItemMainBox->get_height() + m_lineRefs.lineHeight;
}



//=======================================================================================
// EngroutersCreator implementation
//=======================================================================================
EngroutersCreator::EngroutersCreator(std::list<Engrouter*>& engrouters, LibraryScope& libraryScope)
    : m_engrouters(engrouters)
    , m_libraryScope(libraryScope)
{
}

//---------------------------------------------------------------------------------------
EngroutersCreator::~EngroutersCreator()
{
}

//---------------------------------------------------------------------------------------
void EngroutersCreator::create_engrouters(ImoInlineObj* pImo)
{
    //factory method to create Cells and measure the ocupied space

    if (pImo->is_text_item())
    {
        create_text_item_engrouters(dynamic_cast<ImoTextItem*>(pImo));
        measure_engrouters();
    }
    else if (pImo->is_button())
    {
        Engrouter* pEngrouter = LOMSE_NEW ButtonEngrouter(pImo, m_libraryScope);
        pEngrouter->measure();
        m_engrouters.push_back(pEngrouter);
    }
    else if (pImo->is_image())
    {
        Engrouter* pEngrouter = LOMSE_NEW ImageEngrouter(pImo, m_libraryScope);
        pEngrouter->measure();
        m_engrouters.push_back(pEngrouter);
    }
    else if (pImo->is_control())
    {
        Engrouter* pEngrouter = LOMSE_NEW ControlEngrouter(pImo, m_libraryScope);
        pEngrouter->measure();
        m_engrouters.push_back(pEngrouter);
    }
    else if (pImo->is_box_inline())
    {
        ImoBoxInline* pIB = static_cast<ImoBoxInline*>(pImo);
        BoxEngrouter* pBox = create_wrapper_engrouterbox_for(pIB);
        create_and_measure_engrouters(pIB, pBox);
        layout_engrouters_in_engrouterbox_and_measure(pBox);
        m_engrouters.push_back(pBox);
    }
    else
    {
        string msg = str( boost::format(
                            "[EngroutersCreator::create_engrouters] invalid object %d")
                            % pImo->get_obj_type() );
        throw std::runtime_error(msg);
    }
}

//---------------------------------------------------------------------------------------
void EngroutersCreator::create_and_measure_engrouters(ImoBoxInline* pImo,
                                                      BoxEngrouter* pBox)
{
    EngroutersCreator creator(pBox->get_engrouters(), m_libraryScope);

    TreeNode<ImoObj>::children_iterator it;
    for (it = pImo->begin(); it != pImo->end(); ++it)
    {
        creator.create_engrouters( dynamic_cast<ImoInlineObj*>(*it) );
    }
}

//---------------------------------------------------------------------------------------
BoxEngrouter* EngroutersCreator::create_wrapper_engrouterbox_for(ImoBoxInline* pImo)
{
    BoxEngrouter* pBox = LOMSE_NEW BoxEngrouter(pImo, m_libraryScope);
    pBox->measure();
    return pBox;
}

//---------------------------------------------------------------------------------------
void EngroutersCreator::layout_engrouters_in_engrouterbox_and_measure(BoxEngrouter* pBox)
{
    UPoint cursor = pBox->get_content_org();
    LUnits left = cursor.x;
    LUnits lineWidth = pBox->get_content_width();
    LUnits lineHeight = 0.0f;
    LUnits availableWidth = lineWidth;

    std::list<Engrouter*>& engrouters = pBox->get_engrouters();
    std::list<Engrouter*>::iterator it;
    for (it = engrouters.begin(); it != engrouters.end(); ++it)
    {
        Engrouter* pEngrouter = *it;
        if (pEngrouter)
        {
            LUnits width = pEngrouter->get_width();
            if (availableWidth < width)
            {
                availableWidth = lineWidth;
                cursor.x = left;
                cursor.y += lineHeight;
                lineHeight = 0.0f;

                if (availableWidth < width)
                    break; // engrouter doesn't fit
            }

            pEngrouter->set_position(cursor);
            lineHeight = max(lineHeight, pEngrouter->get_line_height());

            cursor.x += width;
            availableWidth -= width;
        }
    }
    cursor.y += lineHeight;
    cursor.y += pBox->get_total_bottom_spacing();
    cursor.x += pBox->get_total_right_spacing();

    //set box size
    if (pBox->get_width() > 0.0f)
        pBox->set_height(cursor.y);
    else
        pBox->set_size(cursor.x, cursor.y);

    pBox->update_measures(cursor.y);
}

//---------------------------------------------------------------------------------------
void EngroutersCreator::create_text_item_engrouters(ImoTextItem* pText)
{
    string& text = pText->get_text();
    size_t n = text.length();
    if (n == 0)
        return;

    //initial spaces
    size_t start = 0;
    size_t length = 1;
    size_t spaces = 0;
    if (text[0] == ' ')
    {
        m_engrouters.push_back( LOMSE_NEW WordEngrouter(pText, m_libraryScope, " "));
        start = text.find_first_not_of(" ");
    }

    //words, including trailing spaces
    while (start >= 0 && start < n)
    {
        size_t stop = text.find_first_of(" ", start);
        if (stop < 0 || stop > n)
            length = n;
        else
        {
            spaces = text.find_first_not_of(" ", stop) - stop;
            if (spaces < 0)
                spaces = n;
            length = (spaces >= 1 ? stop - start + 1 : stop - start);
        }
        m_engrouters.push_back( LOMSE_NEW WordEngrouter(pText, m_libraryScope,
                                        text.substr(start, length)) );
        start += length + (spaces > 1 ? spaces - 1 : 0);
    }
}

////---------------------------------------------------------------------------------------
//void EngroutersCreator::create_item_engrouter(ImoObj* pImo)
//{
//    if (pImo->is_button())
//        m_engrouters.push_back( LOMSE_NEW ButtonEngrouter(pImo, m_libraryScope) );
//    else if (pImo->is_box_inline())
//        m_engrouters.push_back( LOMSE_NEW InlineBoxEngrouter(pImo, m_libraryScope) );
//}

//---------------------------------------------------------------------------------------
void EngroutersCreator::measure_engrouters()
{
    std::list<Engrouter*>::iterator it;
    for (it = m_engrouters.begin(); it != m_engrouters.end(); ++it)
    {
        (*it)->measure();
    }
}


//=======================================================================================
// Engrouter implementation
//=======================================================================================
Engrouter::Engrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope)
    : m_pCreatorImo(pCreatorImo)
    , m_libraryScope(libraryScope)
    , m_pStyle( pCreatorImo->get_style() )
    , m_refLines()
{
}

//---------------------------------------------------------------------------------------
LUnits Engrouter::shift_for_vertical_alignment(LineReferences& refs)
{
    if (refs.lineHeight == 0.0f)
        return 0.0f;

    int valign = k_valign_top;  //m_pStyle->get_int_property(ImoStyle::k_vertical_align);
    switch (valign)
    {
        case ImoStyle::k_valign_baseline:
            return refs.baseline - m_refLines.baseline;

        case ImoStyle::k_valign_sub:
            return 0.0f;    //TODO

        case ImoStyle::k_valign_super:
            return 0.0f;    //TODO

        case ImoStyle::k_valign_top:
            return 0.0f;

        case ImoStyle::k_valign_text_top:
            return 0.0f;    //TODO

        case ImoStyle::k_valign_middle:
            return 0.0f;    //TODO

        case ImoStyle::k_valign_bottom:
            return 0.0f;    //TODO

        case ImoStyle::k_valign_text_bottom:
            return 0.0f;    //TODO

        default:
            return 0.0f;
    }
}



//=======================================================================================
// BoxEngrouter implementation
//=======================================================================================
BoxEngrouter::~BoxEngrouter()
{
    std::list<Engrouter*>::iterator it;
    for (it = m_engrouters.begin(); it != m_engrouters.end(); ++it)
        delete *it;
    m_engrouters.clear();
}

//---------------------------------------------------------------------------------------
void BoxEngrouter::measure()
{
    ImoBoxInline* pWrapper = dynamic_cast<ImoBoxInline*>(m_pCreatorImo);
    set_size( pWrapper->get_size() );

    m_refLines.lineHeight = m_size.height;
    m_refLines.baseline = m_size.height;
    m_refLines.textTop = 0.0f;
    m_refLines.textBottom = m_size.height;
    m_refLines.middleline = m_size.height / 2.0f;
    m_refLines.supperLine = m_refLines.textTop;
    m_refLines.subLine = m_refLines.baseline;
}

//---------------------------------------------------------------------------------------
void BoxEngrouter::update_measures(LUnits lineHeight)
{
    m_refLines.lineHeight = lineHeight;
    m_refLines.baseline = lineHeight;
    m_refLines.textBottom = lineHeight;
    m_refLines.middleline = lineHeight / 2.0f;
    m_refLines.subLine = lineHeight;
}

//---------------------------------------------------------------------------------------
GmoObj* BoxEngrouter::create_gm_object(UPoint pos, LineReferences& refs)
{
    //create box
    pos.y += shift_for_vertical_alignment(refs);

    GmoBox* pBox;
    if (m_pCreatorImo->is_link())
        pBox = LOMSE_NEW GmoBoxLink(m_pCreatorImo);
    else
        pBox = LOMSE_NEW GmoBoxInline(m_pCreatorImo);

    pBox->set_origin(pos);
    pBox->set_height( m_size.height );
    pBox->set_width( m_size.width );

    //create shapes
    std::list<Engrouter*>::iterator it;
    for (it = m_engrouters.begin(); it != m_engrouters.end(); ++it)
    {
        GmoObj* pGmo = (*it)->create_gm_object(pos, refs);
        add_engrouter_shape(pGmo, pBox);
        if (m_pCreatorImo->is_link())
            pGmo->set_in_link(true);
    }

    return pBox;
}

//---------------------------------------------------------------------------------------
UPoint BoxEngrouter::get_content_org()
{
    UPoint org(0.0f, 0.0f);
    if (m_pStyle)
    {
        org.x = m_pStyle->get_lunits_property(ImoStyle::k_margin_left);
        org.x += m_pStyle->get_lunits_property(ImoStyle::k_border_width_left);
        org.x += m_pStyle->get_lunits_property(ImoStyle::k_padding_left);

        org.y = m_pStyle->get_lunits_property(ImoStyle::k_margin_top);
        org.y += m_pStyle->get_lunits_property(ImoStyle::k_border_width_top);
        org.y += m_pStyle->get_lunits_property(ImoStyle::k_padding_top);
    }
    return org;
}

//---------------------------------------------------------------------------------------
LUnits BoxEngrouter::get_content_width()
{
    LUnits lineWidth = get_width() > 0.0f ? get_width() : 10000000.0f;

    if (m_pStyle)
    {
        lineWidth -= m_pStyle->get_lunits_property(ImoStyle::k_border_width_left);
        lineWidth -= m_pStyle->get_lunits_property(ImoStyle::k_padding_left);
        lineWidth -= m_pStyle->get_lunits_property(ImoStyle::k_border_width_right);
        lineWidth -= m_pStyle->get_lunits_property(ImoStyle::k_padding_right);
    }
    return lineWidth;
}

//---------------------------------------------------------------------------------------
LUnits BoxEngrouter::get_total_bottom_spacing()
{
    LUnits space = 0.0f;

    if (m_pStyle)
    {
        space = m_pStyle->get_lunits_property(ImoStyle::k_margin_bottom);
        space += m_pStyle->get_lunits_property(ImoStyle::k_border_width_bottom);
        space += m_pStyle->get_lunits_property(ImoStyle::k_padding_bottom);
    }
    return space;
}

//---------------------------------------------------------------------------------------
LUnits BoxEngrouter::get_total_right_spacing()
{
    LUnits space = 0.0f;

    if (m_pStyle)
    {
        space = m_pStyle->get_lunits_property(ImoStyle::k_margin_right);
        space += m_pStyle->get_lunits_property(ImoStyle::k_border_width_right);
        space += m_pStyle->get_lunits_property(ImoStyle::k_padding_right);
    }
    return space;
}

////---------------------------------------------------------------------------------------
//LUnits BoxEngrouter::add_engrouters_to_box(GmoBox* pBox)
//{
//    LUnits lineHeight = 0.0f;
//    LUnits availableWidth = m_size.width;
//    UPoint cursor;
//
//    std::list<Engrouter*>::iterator it;
//    for (it = m_engrouters.begin(); it != m_engrouters.end(); ++it)
//    {
//        Engrouter* pEngrouter = *it;
//        if (pEngrouter)
//        {
//            LUnits width = pEngrouter->get_width();
//            if (availableWidth < width)
//            {
//                availableWidth = m_size.width;
//                cursor.x = 0.0f;
//                cursor.y += lineHeight;
//                lineHeight = 0.0f;
//
//                if (availableWidth < width)
//                    break; // engrouter doesn't fit
//            }
//
//            GmoObj* pGmo = pEngrouter->create_gm_object(cursor, lineHeight);
//            add_engrouter_shape(pGmo, pBox);
//            lineHeight = max(lineHeight, pEngrouter->get_height());
//
//            cursor.x += width;
//            availableWidth -= width;
//        }
//    }
//
//    m_size.height = cursor.y + lineHeight;
//    return m_size.height;
//}

//---------------------------------------------------------------------------------------
void BoxEngrouter::add_engrouter_shape(GmoObj* pGmo, GmoBox* pBox)
{
    if (pGmo->is_shape())
    {
        GmoShape* pShape = dynamic_cast<GmoShape*>(pGmo);
        pBox->add_shape(pShape, GmoShape::k_layer_staff);
    }
    else if (pGmo->is_box())
    {
        GmoBox* pChildBox = dynamic_cast<GmoBox*>(pGmo);
        pBox->add_child_box(pChildBox);
    }
}



//=======================================================================================
// ButtonEngrouter implementation
//=======================================================================================
GmoObj* ButtonEngrouter::create_gm_object(UPoint pos, LineReferences& refs)
{
    pos.y += shift_for_vertical_alignment(refs);

    ////add engrouter origin
    //pos.x += m_org.x;
    //pos.y += m_org.y;

    return LOMSE_NEW GmoShapeButton(m_pCreatorImo, pos, m_size, m_libraryScope);
}

//---------------------------------------------------------------------------------------
void ButtonEngrouter::measure()
{
    ImoButton* pButton = static_cast<ImoButton*>(m_pCreatorImo);
    m_size = pButton->get_size();

    m_refLines.lineHeight = m_size.height;
    m_refLines.baseline = m_size.height;
    m_refLines.textTop = 0.0f;
    m_refLines.textBottom = m_size.height;
    m_refLines.middleline = m_size.height / 2.0f;
    m_refLines.supperLine = m_refLines.textTop;
    m_refLines.subLine = m_refLines.baseline;
}



//=======================================================================================
// ImageEngrouter implementation
//=======================================================================================
GmoObj* ImageEngrouter::create_gm_object(UPoint pos, LineReferences& refs)
{
    pos.y += shift_for_vertical_alignment(refs);
    ImoImage* pImage = dynamic_cast<ImoImage*>(m_pCreatorImo);
    SpImage image = pImage->get_image();
    return LOMSE_NEW GmoShapeImage(m_pCreatorImo, image, pos, m_size);
}

//---------------------------------------------------------------------------------------
void ImageEngrouter::measure()
{
    ImoImage* pImg = static_cast<ImoImage*>(m_pCreatorImo);
    m_size = pImg->get_image_size();

    m_refLines.lineHeight = m_size.height;
    m_refLines.baseline = m_size.height;
    m_refLines.textTop = 0.0f;
    m_refLines.textBottom = m_size.height;
    m_refLines.middleline = m_size.height / 2.0f;
    m_refLines.supperLine = m_refLines.textTop;
    m_refLines.subLine = m_refLines.baseline;
}



//=======================================================================================
// WordEngrouter implementation
//=======================================================================================
void WordEngrouter::measure()
{
    TextMeter meter(m_libraryScope);
    meter.select_font(m_pStyle->get_string_property(ImoStyle::k_font_name)
                      , m_pStyle->get_float_property(ImoStyle::k_font_size)
                      , m_pStyle->is_bold()
                      , m_pStyle->is_italic() );

    LUnits fontHeight = meter.get_font_height();

    float lineHeight = m_pStyle->get_float_property(ImoStyle::k_line_height);
    m_refLines.lineHeight = (lineHeight == 0.0f ?
                             fontHeight : fontHeight * LUnits(lineHeight));

    m_halfLeading = (m_refLines.lineHeight - fontHeight) / 2.0f;
    m_refLines.textTop = m_halfLeading;
    m_refLines.baseline = m_refLines.textTop + meter.get_ascender();
    m_refLines.textBottom = m_refLines.textTop + fontHeight;

    //middle line (center of 'x' glyph)
    URect rect = meter.bounding_rectangle('x');
    m_refLines.middleline = m_refLines.baseline - rect.height / 2.0f;

    m_refLines.supperLine = m_refLines.textTop;
    m_refLines.subLine = m_refLines.baseline;

    m_size.height = m_refLines.lineHeight;
    m_size.width = meter.measure_width(m_word);

    m_descent = meter.get_descender();
    m_ascent = meter.get_ascender();
}

//---------------------------------------------------------------------------------------
GmoObj* WordEngrouter::create_gm_object(UPoint pos, LineReferences& refs)
{
    pos.y += shift_for_vertical_alignment(refs);

    //add engrouter origin
    pos.x += m_org.x;
    pos.y += m_org.y;

    return LOMSE_NEW GmoShapeWord(m_pCreatorImo, 0, m_word, m_pStyle,
                            pos.x, pos.y, m_halfLeading, m_libraryScope);
}



//=======================================================================================
// InlineWrapperEngrouter implementation
//=======================================================================================
InlineWrapperEngrouter::InlineWrapperEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope)
    : Engrouter(pCreatorImo, libraryScope)
{
    m_pWrapper = dynamic_cast<ImoInlineObj*>( pCreatorImo );
}

//---------------------------------------------------------------------------------------
GmoObj* InlineWrapperEngrouter::create_gm_object(UPoint pos, LineReferences& refs)
{
    //create box
    GmoBoxInline* pBox = LOMSE_NEW GmoBoxInline(m_pCreatorImo);

    ////create shapes and add them to box
    //EngroutersCreator creator(m_engrouters, m_libraryScope);
    //std::list<ImoInlineObj*>& items = m_pWrapper->get_items();
    //std::list<ImoInlineObj*>::iterator it;
    //for (it = items.begin(); it != items.end(); ++it)
    //    creator.create_engrouters(*it);

    //LUnits boxHeight = add_engrouters_to_box(pBox);
    //pBox->set_height( boxHeight );
    //pBox->set_width( m_size.width );

    //return box
    return pBox;
}

////---------------------------------------------------------------------------------------
//void InlineWrapperEngrouter::measure()
//{
//    //EngroutersCreator creator(m_engrouters, m_libraryScope);
//    //creator.measure_engrouters();
//
//    m_size = m_pWrapper->get_size();
//}

//---------------------------------------------------------------------------------------
LUnits InlineWrapperEngrouter::add_engrouters_to_box(GmoBox* pBox, LineReferences& refs)
{
    LUnits lineHeight = 0.0f;
    LUnits availableWidth = m_size.width;
    UPoint cursor;

    std::list<Engrouter*>::iterator it;
    for (it = m_engrouters.begin(); it != m_engrouters.end(); ++it)
    {
        Engrouter* pEngrouter = *it;
        if (pEngrouter)
        {
            LUnits width = pEngrouter->get_width();
            if (availableWidth < width)
            {
                availableWidth = m_size.width;
                cursor.x = 0.0f;
                cursor.y += lineHeight;
                lineHeight = 0.0f;

                if (availableWidth < width)
                    break; // engrouter doesn't fit
            }

            GmoObj* pGmo = pEngrouter->create_gm_object(cursor, refs);
            add_engrouter_shape(pGmo, pBox);
            lineHeight = max(lineHeight, pEngrouter->get_height());

            cursor.x += width;
            availableWidth -= width;
        }
    }

    m_size.height = cursor.y + lineHeight;
    return m_size.height;
}

//---------------------------------------------------------------------------------------
void InlineWrapperEngrouter::add_engrouter_shape(GmoObj* pGmo, GmoBox* pBox)
{
    if (pGmo->is_shape())
    {
        GmoShape* pShape = dynamic_cast<GmoShape*>(pGmo);
        pBox->add_shape(pShape, GmoShape::k_layer_staff);
    }
    else if (pGmo->is_box())
    {
        GmoBox* pBox = dynamic_cast<GmoBox*>(pGmo);
        pBox->add_child_box(pBox);
    }
}


//=======================================================================================
// InlineBoxEngrouter implementation
//=======================================================================================
InlineBoxEngrouter::InlineBoxEngrouter(ImoContentObj* pCreatorImo,
                                       LibraryScope& libraryScope)
    : InlineWrapperEngrouter(pCreatorImo, libraryScope)
{
    m_pBox = dynamic_cast<ImoInlineWrapper*>( pCreatorImo );
}

//---------------------------------------------------------------------------------------
GmoObj* InlineBoxEngrouter::create_gm_object(UPoint pos, LineReferences& refs)
{
    //create box
    GmoBoxInline* pBox = LOMSE_NEW GmoBoxInline(m_pCreatorImo);

    //create shapes
    EngroutersCreator creator(m_engrouters, m_libraryScope);
    TreeNode<ImoObj>::children_iterator it;
    for (it = m_pBox->begin(); it != m_pBox->end(); ++it)
        creator.create_engrouters( dynamic_cast<ImoInlineObj*>(*it) );

    //add shapes to box
    LUnits boxHeight = add_engrouters_to_box(pBox, refs);
    pBox->set_height( boxHeight );
    pBox->set_width( m_size.width );

    //return box
    return pBox;
}

//---------------------------------------------------------------------------------------
void InlineBoxEngrouter::measure()
{
    m_size = m_pBox->get_size();

    m_refLines.lineHeight = m_size.height;
    m_refLines.baseline = m_size.height;
    m_refLines.textTop = 0.0f;
    m_refLines.textBottom = m_size.height;
    m_refLines.middleline = m_size.height / 2.0f;
    m_refLines.supperLine = m_refLines.textTop;
    m_refLines.subLine = m_refLines.baseline;
}



//=======================================================================================
// ControlEngrouter implementation
//=======================================================================================
ControlEngrouter::ControlEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope)
    : Engrouter(pCreatorImo, libraryScope)
    , m_pControl( dynamic_cast<ImoControl*>(pCreatorImo) )
{
}

//---------------------------------------------------------------------------------------
void ControlEngrouter::measure()
{
    m_size = m_pControl->measure();

    m_refLines.lineHeight = m_size.height;
    m_refLines.baseline = m_size.height;
    m_refLines.textTop = 0.0f;
    m_refLines.textBottom = m_size.height;
    m_refLines.middleline = m_size.height / 2.0f;
    m_refLines.supperLine = m_refLines.textTop;
    m_refLines.subLine = m_refLines.baseline;
}

//---------------------------------------------------------------------------------------
GmoObj* ControlEngrouter::create_gm_object(UPoint pos, LineReferences& refs)
{
    pos.y += shift_for_vertical_alignment(refs);
    return m_pControl->layout(m_libraryScope, pos);
}


}  //namespace lomse

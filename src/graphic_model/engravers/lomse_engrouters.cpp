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

#include "lomse_engrouters.h"
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
void EngroutersCreator::create_prefix_engrouter(ImoBoxContent* pBoxContent,
                                                const string& prefix)
{
    m_engrouters.push_back( LOMSE_NEW WordEngrouter(pBoxContent, m_libraryScope, prefix) );
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
    //set size if forced by user
    ImoBoxInline* pWrapper = dynamic_cast<ImoBoxInline*>(m_pCreatorImo);
    if (pWrapper->get_size().width > 0.0)
        m_size.width = pWrapper->get_size().width;
    if (pWrapper->get_size().height > 0.0)
        m_size.height = pWrapper->get_size().height;

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

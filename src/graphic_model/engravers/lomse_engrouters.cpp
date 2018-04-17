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

#include "lomse_engrouters.h"
#include "lomse_build_options.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_text_engraver.h"
#include "lomse_shape_text.h"
#include "lomse_shapes.h"
#include "lomse_calligrapher.h"
#include "lomse_text_splitter.h"
#include "lomse_logger.h"

//other
#include "utf8.h"

namespace lomse
{

//=======================================================================================
// EngroutersCreator implementation
//=======================================================================================
EngroutersCreator::EngroutersCreator(LibraryScope& libraryScope,
                                     TreeNode<ImoObj>::children_iterator itStart,
                                     TreeNode<ImoObj>::children_iterator itEnd)
    : m_libraryScope(libraryScope)
    , m_itCurContent(itStart)
    , m_itEndContent(itEnd)
    , m_pTextSplitter(nullptr)
    , m_pCurText(nullptr)
    , m_pPendingEngr(nullptr)
{
}

//---------------------------------------------------------------------------------------
EngroutersCreator::~EngroutersCreator()
{
    delete m_pPendingEngr;
    delete m_pTextSplitter;
}

//---------------------------------------------------------------------------------------
bool EngroutersCreator::more_content()
{
    return m_itCurContent != m_itEndContent || m_pPendingEngr != nullptr;
}

//---------------------------------------------------------------------------------------
Engrouter* EngroutersCreator::create_next_engrouter(LUnits maxSpace, bool fFirstOfLine)
{
    if (!more_content())
    {
        LOMSE_LOG_DEBUG(Logger::k_layout, "No more content");
        return nullptr;
    }

    LOMSE_LOG_DEBUG(Logger::k_layout, "");
    Engrouter* pEngr = nullptr;
    if (!is_there_a_pending_engrouter())
    {
        ImoInlineLevelObj* pImo = static_cast<ImoInlineLevelObj*>( *m_itCurContent );
        LOMSE_LOG_TRACE(Logger::k_layout,
            "Trying to create the EngroutersCreator for Imo id %d %s",
            pImo->get_id(), pImo->get_name().c_str() );

        //composite content objects
        if (pImo->is_text_item())
        {
            ImoTextItem* pText = static_cast<ImoTextItem*>(pImo);
            pEngr = create_next_text_engrouter_for(pText, maxSpace, fFirstOfLine);
            LOMSE_LOG_TRACE(Logger::k_layout,
                "Text item [%s]", pText->get_text().c_str() );
        }
        else if (pImo->is_box_inline())
        {
            ImoBoxInline* pIB = static_cast<ImoBoxInline*>(pImo);
            pEngr = create_wrapper_engrouter_for(pIB, maxSpace);
        }

        //atomic content objects
        else
        {
            pEngr = create_engrouter_for(pImo);
            ++m_itCurContent;
        }

        if (pEngr)
            pEngr->measure();
    }
    else
    {
        LOMSE_LOG_TRACE(Logger::k_layout, "A pending engrouter exists.");
        pEngr = get_pending_engrouter();
    }

    if (pEngr)
    {
        LUnits width = pEngr->get_width();

        if (width <= maxSpace)
            return pEngr;
        else
        {
            LOMSE_LOG_TRACE(Logger::k_layout,
                "Not enough space for engrouter. Needed=%.02f, available=%.02f",
                width, maxSpace );
            save_engrouter_for_next_call(pEngr);
            return nullptr;
        }
    }
    else
    {
        LOMSE_LOG_TRACE(Logger::k_layout, "Error: Engrouter not created!");
        return nullptr;
    }
}

//---------------------------------------------------------------------------------------
Engrouter* EngroutersCreator::create_engrouter_for(ImoInlineLevelObj* pImo)
{
    //factory method to create engrouters and measure the ocupied space

    if (pImo->is_image())
    {
        Engrouter* pEngrouter = LOMSE_NEW ImageEngrouter(pImo, m_libraryScope);
        pEngrouter->measure();
        return pEngrouter;
    }
    else if (pImo->is_control())
    {
        Engrouter* pEngrouter = LOMSE_NEW ControlEngrouter(pImo, m_libraryScope);
        pEngrouter->measure();
        return pEngrouter;
    }
    else
    {
        stringstream msg;
        msg << "[EngroutersCreator::create_engrouter_for] invalid object " <<
               pImo->get_obj_type();
        cout << "Throw: " << msg.str() << endl;
        LOMSE_LOG_ERROR(msg.str());
        throw std::runtime_error(msg.str());
    }
}

//---------------------------------------------------------------------------------------
Engrouter* EngroutersCreator::create_wrapper_engrouter_for(ImoBoxInline* pIB,
                                                           LUnits UNUSED(maxSpace))
{
    BoxEngrouter* pBoxEngr = LOMSE_NEW BoxEngrouter(pIB, m_libraryScope);

    //create engrouters for its content. Assume all contents fits in box
    create_engrouters_for_box_content(pIB, pBoxEngr);

    pBoxEngr->measure();
    ++m_itCurContent;
    return pBoxEngr;
}

//---------------------------------------------------------------------------------------
void EngroutersCreator::create_engrouters_for_box_content(ImoBoxInline* pImo,
                                                          BoxEngrouter* pBoxEngr)
{
    //recursive. Need to create a new EngroutersCreator

    EngroutersCreator creator(m_libraryScope, pImo->begin(), pImo->end());

    while (creator.more_content())
    {
        Engrouter* pEngr = creator.create_next_engrouter(10000000.0f /*a lot of available space so that all contents fit in box */, false);
        if (pEngr)
            pBoxEngr->add_engrouter(pEngr);
    }

    pBoxEngr->layout_and_measure();
}

//---------------------------------------------------------------------------------------
Engrouter* EngroutersCreator::create_next_text_engrouter_for(ImoTextItem* pText,
                                                             LUnits maxSpace,
                                                             bool fFirstOfLine)
{
    if (m_pCurText != pText)
        return first_text_engrouter_for(pText, maxSpace, fFirstOfLine);
    else
        return next_text_engouter(maxSpace, fFirstOfLine);
}

//---------------------------------------------------------------------------------------
Engrouter* EngroutersCreator::first_text_engrouter_for(ImoTextItem* pText,
                                                       LUnits maxSpace,
                                                       bool fFirstOfLine)
{
    m_pCurText = pText;
    delete m_pTextSplitter;
    m_pTextSplitter = create_text_splitter_for(pText);
    return next_text_engouter(maxSpace, fFirstOfLine);
}

//---------------------------------------------------------------------------------------
Engrouter* EngroutersCreator::next_text_engouter(LUnits maxSpace, bool fFirstOfLine)
{
    Engrouter* pEngr = m_pTextSplitter->get_next_text_engrouter(maxSpace, fFirstOfLine);
    if (!m_pTextSplitter->more_text())
        ++m_itCurContent;
    return pEngr;
}

//---------------------------------------------------------------------------------------
TextSplitter* EngroutersCreator::create_text_splitter_for(ImoTextItem* pText)
{
    //factory method to create a TextSplitter suitable for current language

    string& lang = pText->get_language();
    if (lang == "zh_CN")   //Chinese
        return LOMSE_NEW ChineseTextSplitter(pText, m_libraryScope);
    else
        return LOMSE_NEW DefaultTextSplitter(pText, m_libraryScope);
}

//---------------------------------------------------------------------------------------
Engrouter* EngroutersCreator::create_prefix_engrouter(ImoInlinesContainer* pBoxContent,
                                                      const wstring& prefix)
{
    Engrouter* pEngr = LOMSE_NEW WordEngrouter(pBoxContent, m_libraryScope, prefix);
    pEngr->measure();
    return pEngr;
}



//=======================================================================================
// Engrouter implementation
//=======================================================================================
Engrouter::Engrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope)
    : m_pCreatorImo(pCreatorImo)
    , m_libraryScope(libraryScope)
    , m_pStyle( pCreatorImo->get_style() )
    , m_refLines()
    , m_fBreakRequested(false)
{
}

//---------------------------------------------------------------------------------------
LUnits Engrouter::shift_for_vertical_alignment(LineReferences& refs)
{
    if (refs.lineHeight == 0.0f)
        return 0.0f;

    int valign = k_valign_top;  //m_pStyle->vertical_align();
    switch (valign)
    {
        case ImoStyle::k_valign_baseline:
            return refs.baseline - m_refLines.baseline;

        case ImoStyle::k_valign_sub:
            return 0.0f;    //TODO: Engrouter::shift_for_vertical_alignment

        case ImoStyle::k_valign_super:
            return 0.0f;    //TODO: Engrouter::shift_for_vertical_alignment

        case ImoStyle::k_valign_top:
            return 0.0f;

        case ImoStyle::k_valign_text_top:
            return 0.0f;    //TODO: Engrouter::shift_for_vertical_alignment

        case ImoStyle::k_valign_middle:
            return 0.0f;    //TODO: Engrouter::shift_for_vertical_alignment

        case ImoStyle::k_valign_bottom:
            return 0.0f;    //TODO: Engrouter::shift_for_vertical_alignment

        case ImoStyle::k_valign_text_bottom:
            return 0.0f;    //TODO: Engrouter::shift_for_vertical_alignment

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
        org.x = m_pStyle->margin_left();
        org.x += m_pStyle->border_width_left();
        org.x += m_pStyle->padding_left();

        org.y = m_pStyle->margin_top();
        org.y += m_pStyle->border_width_top();
        org.y += m_pStyle->padding_top();
    }
    return org;
}

//---------------------------------------------------------------------------------------
LUnits BoxEngrouter::get_content_width()
{
    LUnits lineWidth = get_width() > 0.0f ? get_width() : 10000000.0f;

    if (m_pStyle)
    {
        lineWidth -= m_pStyle->border_width_left();
        lineWidth -= m_pStyle->padding_left();
        lineWidth -= m_pStyle->border_width_right();
        lineWidth -= m_pStyle->padding_right();
    }
    return lineWidth;
}

//---------------------------------------------------------------------------------------
LUnits BoxEngrouter::get_total_bottom_spacing()
{
    LUnits space = 0.0f;

    if (m_pStyle)
    {
        space = m_pStyle->margin_bottom();
        space += m_pStyle->border_width_bottom();
        space += m_pStyle->padding_bottom();
    }
    return space;
}

//---------------------------------------------------------------------------------------
LUnits BoxEngrouter::get_total_right_spacing()
{
    LUnits space = 0.0f;

    if (m_pStyle)
    {
        space = m_pStyle->margin_right();
        space += m_pStyle->border_width_right();
        space += m_pStyle->padding_right();
    }
    return space;
}

//---------------------------------------------------------------------------------------
void BoxEngrouter::layout_and_measure()
{
    UPoint cursor = get_content_org();
    LUnits lineHeight = 0.0f;

    std::list<Engrouter*>::iterator it;
    for (it = m_engrouters.begin(); it != m_engrouters.end(); ++it)
    {
        Engrouter* pEngrouter = *it;
//        if (pEngrouter)
        {
            LUnits width = pEngrouter->get_width();
            pEngrouter->set_position(cursor);
            lineHeight = max(lineHeight, pEngrouter->get_line_height());

            cursor.x += width;
        }
    }
    cursor.y += lineHeight;
    cursor.y += get_total_bottom_spacing();
    cursor.x += get_total_right_spacing();

    //set box size
    if (get_width() > 0.0f)
        set_height(cursor.y);
    else
        set_size(cursor.x, cursor.y);

    update_measures(cursor.y);
}

//---------------------------------------------------------------------------------------
void BoxEngrouter::add_engrouter_shape(GmoObj* pGmo, GmoBox* pBox)
{
    if (pGmo)
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
WordEngrouter::WordEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope,
                             const wstring& text)
    : Engrouter(pCreatorImo, libraryScope)
    , m_text(text)
{
    ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pCreatorImo );
    if (pText)
        m_language = pText->get_language();
    else
    {
        ImoDocument* pDoc = pCreatorImo->get_document();
        m_language = pDoc->get_language();
    }
}

//---------------------------------------------------------------------------------------
void WordEngrouter::measure()
{
    TextMeter meter(m_libraryScope);
    meter.select_font(m_language,
                      m_pStyle->font_file(),
                      m_pStyle->font_name(),
                      m_pStyle->font_size(),
                      m_pStyle->is_bold(),
                      m_pStyle->is_italic() );

    LUnits fontHeight = meter.get_font_height();

    float lineHeight = m_pStyle->line_height();
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
    m_size.width = meter.measure_width(m_text);

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

    return LOMSE_NEW GmoShapeWord(m_pCreatorImo, 0, m_text, m_pStyle,
                            m_language, pos.x, pos.y, m_halfLeading, m_libraryScope);
}

//---------------------------------------------------------------------------------------
bool WordEngrouter::text_has_space_at_end()
{
    return L' ' == *(m_text.rbegin());      //use m_text.back() in C++11
}



//=======================================================================================
// InlineWrapperEngrouter implementation
//=======================================================================================
InlineWrapperEngrouter::InlineWrapperEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope)
    : Engrouter(pCreatorImo, libraryScope)
{
    m_pWrapper = dynamic_cast<ImoInlineLevelObj*>( pCreatorImo );
}

//---------------------------------------------------------------------------------------
GmoObj* InlineWrapperEngrouter::create_gm_object(UPoint UNUSED(pos),
                                                 LineReferences& UNUSED(refs))
{
    //create box
    GmoBoxInline* pBox = LOMSE_NEW GmoBoxInline(m_pCreatorImo);
    return pBox;
}

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


////=======================================================================================
//// InlineBoxEngrouter implementation
////=======================================================================================
//InlineBoxEngrouter::InlineBoxEngrouter(ImoContentObj* pCreatorImo,
//                                       LibraryScope& libraryScope)
//    : InlineWrapperEngrouter(pCreatorImo, libraryScope)
//{
//    m_pBox = dynamic_cast<ImoInlineWrapper*>( pCreatorImo );
//}
//
////---------------------------------------------------------------------------------------
//GmoObj* InlineBoxEngrouter::create_gm_object(UPoint pos, LineReferences& refs)
//{
//    //create box
//    GmoBoxInline* pBox = LOMSE_NEW GmoBoxInline(m_pCreatorImo);
//    return pBox;
//}
//
////---------------------------------------------------------------------------------------
//void InlineBoxEngrouter::measure()
//{
//    m_size = m_pBox->get_size();
//
//    m_refLines.lineHeight = m_size.height;
//    m_refLines.baseline = m_size.height;
//    m_refLines.textTop = 0.0f;
//    m_refLines.textBottom = m_size.height;
//    m_refLines.middleline = m_size.height / 2.0f;
//    m_refLines.supperLine = m_refLines.textTop;
//    m_refLines.subLine = m_refLines.baseline;
//}



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

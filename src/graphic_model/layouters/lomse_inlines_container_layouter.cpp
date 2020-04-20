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

#include "lomse_inlines_container_layouter.h"
#include "lomse_build_options.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_text_engraver.h"
#include "lomse_shape_text.h"
#include "lomse_shapes.h"
#include "lomse_calligrapher.h"
#include "lomse_blocks_container_layouter.h"
#include "lomse_logger.h"

namespace lomse
{

//=======================================================================================
// InlinesContainerLayouter implementation
//=======================================================================================
InlinesContainerLayouter::InlinesContainerLayouter(ImoContentObj* pItem, Layouter* pParent,
                                     GraphicModel* pGModel, LibraryScope& libraryScope,
                                     ImoStyles* pStyles, bool fAddShapesToModel)
    : Layouter(pItem, pParent, pGModel, libraryScope, pStyles, fAddShapesToModel)
    , m_libraryScope(libraryScope)
    , m_pPara( dynamic_cast<ImoInlinesContainer*>(pItem) )
    , m_fFirstLine(true)
    , m_xLineStart(0.0f)
    , m_pEngrCreator(nullptr)
    , m_firstLineIndent(0.0f)
    , m_firstLinePrefix(L"")
    , m_availableSpace(0.0f)
    , m_lineWidth(0.0f)
{
}

//---------------------------------------------------------------------------------------
InlinesContainerLayouter::~InlinesContainerLayouter()
{
    delete m_pEngrCreator;
//    std::list<Engrouter*>::iterator it;
//    for (it = m_engrouters.begin(); it != m_engrouters.end(); ++it)
//        delete *it;
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::prepare_to_start_layout()
{
    Layouter::prepare_to_start_layout();
    m_pEngrCreator =
        LOMSE_NEW EngroutersCreator(m_libraryScope, m_pPara->begin(), m_pPara->end());
    get_indent_and_bullet_info();
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::get_indent_and_bullet_info()
{
    //TODO: Appart from bullet points, first line indent is a property of
    //      the paragraph style

    //Bullet and indent is only set if this InlinesContainer is a ListItem
    ListItemLayouter* pList = dynamic_cast<ListItemLayouter*>(m_pParentLayouter);
    if (pList && pList->is_first_content_item(m_pItem))
    {
        m_firstLineIndent = -500.0f;
        m_firstLinePrefix = L"*  ";
    }
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::layout_in_box()
{
    LOMSE_LOG_DEBUG(Logger::k_layout, string(""));

    //AWARE: This method is invoked to layout a page. If there are more pages to
    //layout, it will be invoked more times. Therefore, this method must not initialize
    //anything. All initializations must be done in 'prepare_to_start_layout()'.
    //layout_in_box() method must always continue laying out from current state.

    set_cursor_and_available_space();
    page_initializations(m_pItemMainBox);

    if (!is_line_ready())
        prepare_line();

    if (!is_line_ready())
    {
        //empty paragraph
        advance_current_line_space(m_pageCursor.x);
    }
    else
    {
        while(is_line_ready() && enough_space_in_box())
        {
            add_line();
            prepare_line();
        }
    }

    bool fMoreText = more_content() || is_line_ready();
    if (m_fAddShapesToModel && !fMoreText)
        m_pItemMainBox->add_shapes_to_tables();

    set_layout_result(fMoreText ? k_layout_not_finished : k_layout_success);
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::prepare_line()
{
    LOMSE_LOG_DEBUG(Logger::k_layout, string(""));

    set_line_pos_and_width();
    initialize_line_references();

    if (m_fFirstLine)
        add_bullet();

    bool fBreak = false;

    LOMSE_LOG_TRACE(Logger::k_layout, "available space=%.02f", m_availableSpace);

    bool fSomethingAdded = false;
    bool fFirstEngrouterOfLine = true;
    Engrouter* pEngr = nullptr;
    while (space_in_line() && !fBreak)
    {
        bool fRemoveLeftSpace = fFirstEngrouterOfLine;
        if (pEngr)
        {
            WordEngrouter* pWE = dynamic_cast<WordEngrouter*>(pEngr);
            if (pWE)
                fRemoveLeftSpace |= pWE->text_has_space_at_end();
        }

        pEngr = create_next_engrouter(fRemoveLeftSpace);

        if (pEngr)
        {
            add_engrouter_to_line(pEngr);
            fBreak = pEngr->break_requested();
            fSomethingAdded = true;
            fFirstEngrouterOfLine = false;
        }
        else
        {
            if (!fSomethingAdded)
            {
                LOMSE_LOG_TRACE(Logger::k_layout, "Line empty! No engrouter created.");
            }
            break;
        }
    }
}

//---------------------------------------------------------------------------------------
Engrouter* InlinesContainerLayouter::create_next_engrouter(bool fRemoveLeftSpace)
{
    if (logger.debug_mode_enabled() || logger.trace_mode_enabled())
    {
        Engrouter* pEngr = m_pEngrCreator->create_next_engrouter(m_availableSpace,
                                                                 fRemoveLeftSpace);
        if (pEngr)
        {
            LOMSE_LOG_DEBUG(Logger::k_layout, "Engrouter created");
        }
        else
        {
            LOMSE_LOG_DEBUG(Logger::k_layout, "Engrouter is nullptr");
        }
        return pEngr;
    }
    else
        return m_pEngrCreator->create_next_engrouter(m_availableSpace, fRemoveLeftSpace);
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::create_main_box(GmoBox* pParentBox, UPoint pos,
                                        LUnits width, LUnits height)
{
    m_pItemMainBox = LOMSE_NEW GmoBoxParagraph(m_pPara);
    pParentBox->add_child_box(m_pItemMainBox);

    m_pItemMainBox->set_origin(pos);
    m_pItemMainBox->set_width(width);
    m_pItemMainBox->set_height(height);
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::add_bullet()
{
    wstring prefix = get_first_line_prefix();
    if (!prefix.empty())
    {
        Engrouter* pEngr = m_pEngrCreator->create_prefix_engrouter(m_pPara, prefix);
        add_engrouter_to_line(pEngr);
    }
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::set_line_pos_and_width()
{
    //TODO: lineStart will depend on block-progression property
    m_xLineStart = m_pItemMainBox->get_content_left();
    m_lineWidth = 0.0f;

    if (is_first_line())
    {
        LUnits indent = get_first_line_indent();
        m_xLineStart += indent;
        m_lineWidth = indent;
        m_availableSpace -= indent;
    }


    //TODO: For left-to-right (ltr) and right-to-left (rtl) m_availableSpace is
    //the available width. But for top-to-bottom (ttb) and bottom-to-top (btt) it
    //should be the available height.

}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::add_line()
{
    LOMSE_LOG_DEBUG(Logger::k_layout, string(""));

    m_pageCursor.x = m_xLineStart;
    LUnits left = m_pageCursor.x;       //save left margin

    //horizontal alignment
    ImoStyle* pStyle = m_pPara->get_style();
    switch (pStyle->text_align())
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
    for(it = m_engrouters.begin(); it != m_engrouters.end(); ++it)
    {
        add_engrouter_shape(*it, m_lineRefs.lineHeight);
        delete *it;
    }
    m_engrouters.clear();

    advance_current_line_space(left);
    m_fFirstLine = false;
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::advance_current_line_space(LUnits left)
{
    LOMSE_LOG_DEBUG(Logger::k_layout, string(""));

    m_pageCursor.x = left;
    m_pageCursor.y += m_lineRefs.lineHeight;
    m_availableSpace = m_availableWidth;

    m_pItemMainBox->set_height( m_pItemMainBox->get_height() + m_lineRefs.lineHeight );
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::add_engrouter_to_line(Engrouter* pEngrouter)
{
    LOMSE_LOG_DEBUG(Logger::k_layout, string(""));

    //add engrouter to the list of engrouters for current line.
    m_engrouters.push_back( pEngrouter );

    //do  vertical alignement of the engrouter.
    LUnits shift = pEngrouter->shift_for_vertical_alignment(m_lineRefs);

    //if it is a WordEngrouter, vertival alignment could imply changes in the
    //reference lines for the line.
    bool fUpdateText = dynamic_cast<WordEngrouter*>(pEngrouter) != nullptr;
    if (fUpdateText)
    {
        int valign = pEngrouter->get_style()->vertical_align();
        fUpdateText = valign == ImoStyle::k_valign_baseline;
    }
    update_line_references(pEngrouter->get_reference_lines(), shift, fUpdateText);

    //discount space ocuppied by the engrouter's shape
    LUnits width = pEngrouter->get_width();
    m_pageCursor.x += width;
    m_availableSpace -= width;
    m_lineWidth += width;
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::initialize_line_references()
{
    //line height: 'strut' line height
    ImoStyle* pStyle = m_pPara->get_style();

    TextMeter meter(m_libraryScope);
    meter.select_font( "",      //no particular language
                       pStyle->font_file(),
                       pStyle->font_name(),
                       pStyle->font_size(),
                       pStyle->is_bold(),
                       pStyle->is_italic() );
    LUnits fontHeight = meter.get_font_height();

    float lineHeight = pStyle->line_height();
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
void InlinesContainerLayouter::update_line_references(LineReferences& engr, LUnits shift,
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
void InlinesContainerLayouter::add_engrouter_shape(Engrouter* pEngrouter,
                                                   LUnits UNUSED(lineHeight))
{
    GmoObj* pGmo = pEngrouter->create_gm_object(m_pageCursor, get_line_refs());

    if (pGmo)
    {
        if (pGmo->is_shape())
        {
            GmoShape* pShape = static_cast<GmoShape*>(pGmo);
            m_pItemMainBox->add_shape(pShape, GmoShape::k_layer_staff);
            m_pageCursor.x += pShape->get_width();
        }
        else if (pGmo->is_box())
        {
            GmoBox* pBox = static_cast<GmoBox*>(pGmo);
            m_pItemMainBox->add_child_box(pBox);
            m_pageCursor.x += pBox->get_width();
        }
    }
}

//---------------------------------------------------------------------------------------
void InlinesContainerLayouter::page_initializations(GmoBox* pMainBox)
{
    m_pItemMainBox = pMainBox;

    //no height until content added
    ImoStyle* pStyle = m_pPara->get_style();
    if (pStyle)
        m_pItemMainBox->set_height( pStyle->margin_top() + pStyle->border_width_top()
                                    + pStyle->padding_top() );
    else
        m_pItemMainBox->set_height(0.0f);

    m_availableSpace = m_availableWidth;
}

//---------------------------------------------------------------------------------------
bool InlinesContainerLayouter::enough_space_in_box()
{
    return m_availableHeight >= m_pItemMainBox->get_height() + m_lineRefs.lineHeight;
}


}  //namespace lomse

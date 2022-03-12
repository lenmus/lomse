//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_text_splitter.h"

#include "lomse_engrouters.h"
#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"
#include "lomse_logger.h"

#include "utf8.h"


namespace lomse
{


//=======================================================================================
// TextSplitter implementation
//=======================================================================================
TextSplitter::TextSplitter(ImoTextItem* pText, LibraryScope& libraryScope)
    : m_pText(pText)
    , m_language( pText->get_language() )
    , m_libraryScope(libraryScope)
{
    //convert string to utf-32 and store characters as unsigned int
    const char* utf8str = pText->get_text().c_str();
    utf8::utf8to32(utf8str, utf8str + strlen(utf8str), std::back_inserter(m_glyphs));

    m_totalGlyphs = m_glyphs.size();

    measure_glyphs();
}

//---------------------------------------------------------------------------------------
void TextSplitter::measure_glyphs()
{
    TextMeter meter(m_libraryScope);
    ImoStyle* pStyle = m_pText->get_style();
    meter.select_font( m_language,
                       pStyle->font_file(),
                       pStyle->font_name(),
                       pStyle->font_size(),
                       pStyle->is_bold(),
                       pStyle->is_italic() );

    meter.measure_glyphs(&m_glyphs, m_glyphWidths);
}



//=======================================================================================
// DefaultTextSplitter implementation
//=======================================================================================
DefaultTextSplitter::DefaultTextSplitter(ImoTextItem* pText, LibraryScope& libraryScope)
    : TextSplitter(pText, libraryScope)
    , m_start(0)
    , m_length(0)
    , m_spaces(0)
{
}

//---------------------------------------------------------------------------------------
bool DefaultTextSplitter::more_text()
{
    return m_totalGlyphs > 0 && m_start < m_totalGlyphs;
}

//---------------------------------------------------------------------------------------
Engrouter* DefaultTextSplitter::get_next_text_engrouter(LUnits maxSpace,
                                                        bool fRemoveLeftSpaces)
{
    // Returns WordEngrouter with a text chunk of size <= maxSize. This chunk
    // always contains full words (splits at spaces).
    // If not enough space for a word, returns nullptr
    // If no more text, returns NullEngrouter

    if (!more_text())
        return LOMSE_NEW NullEngrouter(m_pText, m_libraryScope);

    if (fRemoveLeftSpaces)
    {
        //skip initial spaces
        while (m_start < m_totalGlyphs && m_glyphs[m_start] == L' ')
            ++m_start;
    }

    LUnits width = 0.0f;
    size_t i = m_start;
    size_t breakPoint = m_start;
    LUnits nextWidth = m_glyphWidths[i];
    bool fSpaces = false;

    while (i < m_totalGlyphs && width + nextWidth < maxSpace)
    {
        if (m_glyphs[i] == L' ')
        {
            if (!fSpaces)
            {
                breakPoint = i;
                fSpaces = true;
            }
        }
        else
            fSpaces = false;

        width += nextWidth;
        if (++i < m_totalGlyphs)
            nextWidth = m_glyphWidths[i];
    }
    if (i == m_totalGlyphs || (m_glyphs[i] == L' ' && !fSpaces))
        breakPoint = i;

    size_t length = breakPoint - m_start;

    if (length > 0)
    {
        Engrouter* pEngr = LOMSE_NEW WordEngrouter(m_pText, m_libraryScope,
                                         m_glyphs.substr(m_start, length));
        if (i < m_totalGlyphs)
            pEngr->set_break_requested();
        pEngr->measure();
        m_start += length;
        //skip initial spaces in next chunk
        while (m_start < m_totalGlyphs && m_glyphs[m_start] == L' ')
            ++m_start;
        return pEngr;
    }
    else
    {
        LOMSE_LOG_ERROR("Text without spaces is longer than line size");
        //TODO: If a text without spaces is longer than line size, returning nullptr
        //will enter the program in an infinite loop, as TextSplitter will be invoked
        //again. Therefore, it is necessary to deal with this case. A possibility is
        //to save information about maxSize and a flag signaling that nullptr was returned
        //in previous invocation. Then, when nullptr is going to be returned, if nullptr
        //was previously returned and current maxSize == saved maxSize, it is clear
        //that text must be split in non-spaces position.
        return nullptr;
    }
}



//=======================================================================================
// ChineseTextSplitter implementation
//=======================================================================================
ChineseTextSplitter::ChineseTextSplitter(ImoTextItem* pText, LibraryScope& libraryScope)
    : TextSplitter(pText, libraryScope)
    , m_start(0)
    , m_length(0)
    , m_spaces(0)
{
}

//---------------------------------------------------------------------------------------
bool ChineseTextSplitter::more_text()
{
    return m_totalGlyphs > 0 && m_start < m_totalGlyphs;
}

//---------------------------------------------------------------------------------------
Engrouter* ChineseTextSplitter::get_next_text_engrouter(LUnits maxSpace,
                                                        bool UNUSED(fRemoveLeftSpaces))
{
    if (!more_text())
        return nullptr;

    LUnits width = 0.0f;
    size_t i = m_start;
    LUnits nextWidth = m_glyphWidths[i];
    while (i < m_totalGlyphs && width + nextWidth < maxSpace)
    {
        width += nextWidth;
        if (++i < m_totalGlyphs)
            nextWidth = m_glyphWidths[i];
    }
    size_t length = i - m_start;

    if (length > 0)
    {
        Engrouter* pEngr = LOMSE_NEW WordEngrouter(m_pText, m_libraryScope,
                                                   m_glyphs.substr(m_start, length));
        if (i < m_totalGlyphs)
            pEngr->set_break_requested();
        pEngr->measure();
        m_start += length;
        return pEngr;
    }
    else
    {
        LOMSE_LOG_ERROR("Text without spaces is longer than line size");
        //TODO: If a text without spaces is longer than line size, returning nullptr
        //will enter the program in an infinite loop, as TextSplitter will be invoked
        //again. Therefore, it is necessary to deal with this case. A possibility is
        //to save information about maxSize and a flag signaling that nullptr was returned
        //in previous invocation. Then, when nullptr is going to be returned, if nullptr
        //was previously returned and current maxSize == saved maxSize, it is clear
        //that text must be split in non-spaces position.
        return nullptr;
    }
}


}  //namespace lomse

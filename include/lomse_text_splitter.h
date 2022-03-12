//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_TEXT_SPLITTER_H__
#define __LOMSE_TEXT_SPLITTER_H__

#include "lomse_injectors.h"
#include "lomse_basic.h"


namespace lomse
{

//forward declarations
class Engrouter;
class ImoTextItem;


//---------------------------------------------------------------------------------------
// TextSplitter: encapsulates the algorithms for text hyphenation and splitting.
//    This is an abstract class. Specific splitters have to be created for each
//    language.
class TextSplitter
{
protected:
    ImoTextItem* m_pText;
    string m_language;
    LibraryScope& m_libraryScope;
    wstring m_glyphs;
    std::vector<LUnits> m_glyphWidths;
    size_t m_totalGlyphs;

    TextSplitter(ImoTextItem* pText, LibraryScope& libraryScope);

public:
    virtual ~TextSplitter() {}

    virtual Engrouter* get_next_text_engrouter(LUnits maxSpace, bool fRemoveLeftSpaces) = 0;
    virtual bool more_text() = 0;

protected:
    void measure_glyphs();

};

//---------------------------------------------------------------------------------------
// DefaultTextSplitter: splits at spaces, no hyphenation
class DefaultTextSplitter : public TextSplitter
{
protected:
    size_t m_start;
    size_t m_length;
    size_t m_spaces;

public:
    DefaultTextSplitter(ImoTextItem* pText, LibraryScope& libraryScope);
    ~DefaultTextSplitter() {}

    Engrouter* get_next_text_engrouter(LUnits maxSpace, bool fRemoveLeftSpaces) override;
    bool more_text() override;

protected:

};

//---------------------------------------------------------------------------------------
// ChineseTextSplitter:
class ChineseTextSplitter : public TextSplitter
{
protected:
    size_t m_start;
    size_t m_length;
    size_t m_spaces;

public:
    ChineseTextSplitter(ImoTextItem* pText, LibraryScope& libraryScope);
    ~ChineseTextSplitter() {}

    Engrouter* get_next_text_engrouter(LUnits maxSpace, bool fRemoveLeftSpaces) override;
    bool more_text() override;

};


}   //namespace lomse

#endif      //__LOMSE_TEXT_SPLITTER_H__

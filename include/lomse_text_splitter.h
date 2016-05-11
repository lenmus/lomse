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

    Engrouter* get_next_text_engrouter(LUnits maxSpace, bool fRemoveLeftSpaces);
    bool more_text();

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

    Engrouter* get_next_text_engrouter(LUnits maxSpace, bool fRemoveLeftSpaces);
    bool more_text();

};


}   //namespace lomse

#endif      //__LOMSE_TEXT_SPLITTER_H__

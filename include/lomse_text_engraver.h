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

#ifndef __LOMSE_TEXT_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_TEXT_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class FontStorage;
class ImoScore;
class GmoBox;
class GmoShapeText;
class GmoShapeTextBox;
class ImoStyle;
class ImoScoreText;
class VerticalProfile;

//---------------------------------------------------------------------------------------
class TextEngraver : public Engraver
{
protected:
    const string& m_text;
    ImoStyle* m_pStyle;
    FontStorage* m_pFontStorage;
    string m_language;
    int m_idxStaff;
    VerticalProfile* m_pVProfile;

public:
    TextEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 const string& text, const string& language, ImoStyle* pStyle);
    ~TextEngraver();

    GmoShapeText* create_shape(ImoObj* pCreatorImo, LUnits xLeft, LUnits yTop);
    LUnits measure_width();
    LUnits measure_height();
};

//---------------------------------------------------------------------------------------
class TextBoxEngraver : public Engraver
{
protected:
    const string& m_text;
    ImoStyle* m_pStyle;
    FontStorage* m_pFontStorage;
    string m_language;

public:
    TextBoxEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 const string& text, const string& language, ImoStyle* pStyle);
    ~TextBoxEngraver();

    GmoShapeTextBox* create_shape(ImoObj* pCreatorImo, LUnits xLeft, LUnits yTop);
    LUnits measure_width();
    LUnits measure_height();
};

//---------------------------------------------------------------------------------------
class MeasureNumberEngraver : public Engraver
{
protected:
    const string& m_text;
    ImoStyle* m_pStyle;
    FontStorage* m_pFontStorage;

public:
    MeasureNumberEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                          const string& text);
    ~MeasureNumberEngraver();

    GmoShapeText* create_shape(ImoObj* pCreator, LUnits xLeft, LUnits yTop);
    LUnits measure_width();
    LUnits measure_height();
};


}   //namespace lomse

#endif    // __LOMSE_TEXT_ENGRAVER_H__


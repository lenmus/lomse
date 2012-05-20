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

#include "lomse_text_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_text.h"


namespace lomse
{

//=======================================================================================
// TextEngraver implementation
//=======================================================================================
TextEngraver::TextEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           const string& text, ImoStyle* pStyle)
    : Engraver(libraryScope, pScoreMeter)
    , m_text(text)
    , m_pStyle(pStyle)
    , m_pFontStorage( libraryScope.font_storage() )
{
}

//---------------------------------------------------------------------------------------
TextEngraver::TextEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           ImoScoreText* pText, ImoScore* pScore)
    : Engraver(libraryScope, pScoreMeter)
    , m_text(pText->get_text())
    , m_pFontStorage( libraryScope.font_storage() )
{
    m_pStyle = pText->get_style();
    if (!m_pStyle)
        m_pStyle = pScore->get_default_style();
}

//---------------------------------------------------------------------------------------
TextEngraver::~TextEngraver()
{
}

//---------------------------------------------------------------------------------------
LUnits TextEngraver::measure_width()
{
    TextMeter meter(m_libraryScope);
    meter.select_font( m_pStyle->get_string_property(ImoStyle::k_font_name),
                       m_pStyle->get_float_property(ImoStyle::k_font_size) );
    return meter.measure_width(m_text);
}

//---------------------------------------------------------------------------------------
LUnits TextEngraver::measure_height()
{
    TextMeter meter(m_libraryScope);
    meter.select_font( m_pStyle->get_string_property(ImoStyle::k_font_name),
                       m_pStyle->get_float_property(ImoStyle::k_font_size) );
    return meter.get_font_height();
}

//---------------------------------------------------------------------------------------
GmoShapeText* TextEngraver::create_shape(ImoObj* pCreatorImo, LUnits xLeft, LUnits yTop)
{
    UPoint pos(xLeft, yTop);
    if (pCreatorImo && pCreatorImo->is_score_text())
        add_user_shift(static_cast<ImoScoreText*>(pCreatorImo), &pos);

    //TODO-LOG
    //if (valign == k_center)
    {
        TextMeter meter(m_libraryScope);
        yTop -= meter.get_descender();
    }

    int idx = 0;
    return LOMSE_NEW GmoShapeText(pCreatorImo, idx, m_text, m_pStyle, pos.x, pos.y,
                            m_libraryScope);
}


}  //namespace lomse

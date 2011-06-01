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
                           const string& text, ImoTextStyleInfo* pStyle)
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
        m_pStyle = pScore->get_default_style_info();
}

//---------------------------------------------------------------------------------------
TextEngraver::~TextEngraver()
{
}

//---------------------------------------------------------------------------------------
LUnits TextEngraver::measure_width()
{
    TextMeter meter(m_libraryScope);
    meter.select_font( m_pStyle->get_font_name(), m_pStyle->get_font_size() );
    return meter.measure_width(m_text);
}

//---------------------------------------------------------------------------------------
LUnits TextEngraver::measure_height()
{
    TextMeter meter(m_libraryScope);
    meter.select_font( m_pStyle->get_font_name(), m_pStyle->get_font_size() );
    return meter.get_font_height();
}

//---------------------------------------------------------------------------------------
GmoShapeText* TextEngraver::create_shape(ImoObj* pCreatorImo, LUnits xLeft,
                                         LUnits yTop, int valign)
{
    //TODO-LOG
    //if (valign == k_center)
    {
        TextMeter meter(m_libraryScope);
        yTop -= meter.get_descender();
    }

    int idx = 0;
    return new GmoShapeText(pCreatorImo, idx, m_text, m_pStyle, xLeft, yTop,
                            m_libraryScope);
}


}  //namespace lomse

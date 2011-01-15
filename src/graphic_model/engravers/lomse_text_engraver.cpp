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

//---------------------------------------------------------------------------------------
// TextEngraver implementation
//---------------------------------------------------------------------------------------
TextEngraver::TextEngraver(LibraryScope& libraryScope, ImoScoreText& text,
                           ImoScore* pScore)
    : m_text(text)
    , m_pScore(pScore)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_libraryScope(libraryScope)
{
}

//---------------------------------------------------------------------------------------
TextEngraver::~TextEngraver()
{
}

//---------------------------------------------------------------------------------------
LUnits TextEngraver::measure_width()
{
    ImoTextStyleInfo* pStyle = m_text.get_style();
    if (!pStyle)
        pStyle = m_pScore->get_default_style_info();

    TextMeter meter(m_libraryScope);
    meter.select_font( pStyle->get_font_name(), pStyle->get_font_size() );
    return meter.measure_width(m_text.get_text());
}

//---------------------------------------------------------------------------------------
void TextEngraver::add_shape(GmoBox* pBox, LUnits xLeft, LUnits yTop, int valign)
{
    //TODO-LOG
    //if (valign == k_center)
    {
        TextMeter meter(m_libraryScope);
        yTop -= meter.get_descender();
    }

    ImoTextStyleInfo* pStyle = m_text.get_style();
    if (!pStyle)
        pStyle = m_pScore->get_default_style_info();

    int idx = 0;
    GmoShape* pShape = new GmoShapeText(idx, m_text.get_text(), pStyle,
                                        xLeft, yTop, m_libraryScope);
    pBox->add_shape(pShape, GmoShape::k_layer_staff);
}


}  //namespace lomse

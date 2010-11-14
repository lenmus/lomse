//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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

#include "lomse_instrument_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"
#include "lomse_engraving_options.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_brace_bracket.h"
#include "lomse_shape_staff.h"
#include "lomse_shape_text.h"
#include "lomse_box_system.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// InstrumentEngraver implementation
//---------------------------------------------------------------------------------------

InstrumentEngraver::InstrumentEngraver(ImoInstrument* pInstr, TextMeter* pMeter)
    : m_pInstr(pInstr)
    , m_pTextMeter(pMeter)
{
}

//---------------------------------------------------------------------------------------
InstrumentEngraver::~InstrumentEngraver()
{
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::tenths_to_logical(Tenths value, int iStaff)
{
    return (value * m_pInstr->get_staff(iStaff)->get_line_spacing()) / 10.0f;
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::measure_indents()
{
    m_uIndentFirst = 0.0f;
    m_uIndentOther = 0.0f;
    measure_name_abbrev();
    measure_brace_or_bracket();
}

////---------------------------------------------------------------------------------------
void InstrumentEngraver::measure_name_abbrev()
{
//    //if this instrument is in a group get group names width
//    if (m_pGroup)
//    {
//        if (m_pGroup->GetFirstInstrument() == this)
//        {
//            m_pGroup->MeasureNames(pPaper);
//        }
//        m_uIndentFirst += m_pGroup->GetIndentFirst();
//        m_uIndentOther += m_pGroup->GetIndentOther();
//    }

    LUnits uSpaceAfterName = tenths_to_logical(LOMSE_INSTR_SPACE_AFTER_NAME);

    const std::string name = m_pInstr->get_name();
    if (name != "")
        m_uIndentFirst += m_pTextMeter->measure_width(name) + uSpaceAfterName;

    const std::string abbrev =  m_pInstr->get_abbrev();
    if (abbrev != "")
        m_uIndentOther += m_pTextMeter->measure_width(abbrev) + uSpaceAfterName;
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::measure_brace_or_bracket()
{
    if (has_brace_or_bracket())
    {
        m_uBracketWidth = tenths_to_logical(LOMSE_GRP_BRACKET_WIDTH);
        m_uBracketGap = tenths_to_logical(LOMSE_GRP_BRACKET_GAP);

        m_uIndentOther += m_uBracketWidth + m_uBracketGap;
        m_uIndentFirst += m_uBracketWidth + m_uBracketGap;
    }
}

//---------------------------------------------------------------------------------------
bool InstrumentEngraver::has_brace_or_bracket()
{
    return (m_pInstr->get_num_staves() > 1);

    //return (m_pInstr->get_num_staves() > 1 && m_bracketSymbol == ImoInstrGroup::k_default ||
    //        m_bracketSymbol == ImoInstrGroup::k_bracket ||
    //        m_bracketSymbol == ImoInstrGroup::k_brace );
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::add_name_abbrev(GmoBox* pBox, int nSystem)
{
    const std::string& name
        = (nSystem == 1 ? m_pInstr->get_name() : m_pInstr->get_abbrev());
    if (name != "")
        add_text_shape(pBox, name);
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::add_brace_bracket(GmoBox* pBox)
{
    if (has_brace_or_bracket())
    {
        LUnits xLeft = m_staffLeft - m_uBracketWidth - m_uBracketGap;
        LUnits xRight = m_staffLeft - m_uBracketGap;
        LUnits yTop = m_staffTop;
        LUnits yBottom = m_staffBottom;
        //int nSymbol = ImoInstrGroup::k_bracket;
        //int nSymbol = (m_bracketSymbol ==
        //    (ImoInstrGroup::k_default ? ImoInstrGroup::k_bracket : m_bracketSymbol));

        GmoShape* pShape;
//        if (nSymbol == lm_eBracket)
            pShape = new GmoShapeBracket(pBox, xLeft, yTop, xRight, yBottom);
//        else
//        {
//            LUnits dyHook = tenths_to_logical(6.0f);
//            pShape = new GmoShapeBrace(this, xLeft, yTop, xRight, yBottom,
//                                      dyHook, *wxBLACK);
//        }
        pBox->add_shape(pShape, GmoShape::k_layer_staff);
    }
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::add_staff_lines(GmoBoxSystem* pBox, LUnits x, LUnits y,
                                         LUnits indent)
{
 //   bool fVisible = !HideStaffLines();
    m_staffLeft = x + indent;
    m_staffTop = y;
    for (int iStaff=0; iStaff < m_pInstr->get_num_staves(); iStaff++)
	{
        ImoStaffInfo* pStaff = m_pInstr->get_staff(iStaff);
        if (iStaff > 0)
            y += pStaff->get_staff_margin();
        GmoShapeStaff* pShape = new GmoShapeStaff(pBox, pStaff, iStaff, indent);
        pShape->set_origin(m_staffLeft, y);
        pBox->add_staff_shape(pShape);
 //       pShape->SetVisible(fVisible);
        y = pShape->get_bottom();
    }
    m_staffBottom = y;
}

////---------------------------------------------------------------------------------------
//void InstrumentEngraver::AddNameAndBracket(GmoBox* pBSystem, GmoBox* pBSliceInstr, lmPaper* pPaper,
//                                     int nSystem)
//{
//    //Layout.
//    // invoked when layouting first measure in system, to add instrument name and bracket/brace.
//    // This method is also responsible for managing group name and bracket/brace layout
//    // When reaching this point, BoxSystem and BoxSliceInstr have their bounds correctly
//    // set (except xRight)
//
//	if (nSystem == 1)
//        add_name_abbrev_shape(pBSliceInstr, pPaper, m_pName);
//	else
//        add_name_abbrev_shape(pBSliceInstr, pPaper, m_pAbbreviation);
//
//    // if first instrument in group, save yTop position for group
//    static LUnits yTopGroup;
//	if (IsFirstOfGroup())
//		yTopGroup = pBSliceInstr->GetYTop();
//
//    // if last instrument of a group, add group name and bracket/brace
//	if (IsLastOfGroup())
//        m_pGroup->AddNameAndBracket(pBSystem, pPaper, nSystem, pBSliceInstr->GetXLeft(),
//                                    yTopGroup, pBSliceInstr->GetYBottom() );
//}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::add_text_shape(GmoBox* pBox, const std::string& text)
{
    LUnits xLeft = pBox->get_left();
    LUnits yTop = m_staffTop + (m_staffBottom - m_staffTop) / 2.0f
                  - m_pTextMeter->get_descender();

//    GmoShape* pShape = new GmoShapeText(pBox, text, xLeft, yTop);
//    pBox->add_shape(pShape, GmoShape::k_layer_staff);
}


}  //namespace lomse

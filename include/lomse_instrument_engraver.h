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

#ifndef __LOMSE_INSTRUMENT_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_INSTRUMENT_ENGRAVER_H__

#include "lomse_basic.h"
#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class ImoInstrument;
class TextMeter;
class GmoBox;
class GmoBoxSystem;

//---------------------------------------------------------------------------------------
class InstrumentEngraver
{
protected:
    ImoInstrument* m_pInstr;
    TextMeter* m_pTextMeter;
    LUnits m_uIndentFirst;
    LUnits m_uIndentOther;
    LUnits m_uBracketWidth;
    LUnits m_uBracketGap;
    //int m_bracketSymbol;      

    LUnits m_staffTop; 
    LUnits m_staffBottom; 
    LUnits m_staffLeft;

public:
    InstrumentEngraver(ImoInstrument* pInstr, TextMeter* pMeter);
    ~InstrumentEngraver();

    //indents
    void measure_indents();
    LUnits get_indent_first() { return m_uIndentFirst; }
    LUnits get_indent_other() { return m_uIndentOther; }

    //shapes
    void add_staff_lines(GmoBoxSystem* pBox, LUnits x, LUnits y, LUnits indent);
    void add_name_abbrev(GmoBox* pBox, int nSystem);
    void add_brace_bracket(GmoBox* pBox);
    LUnits get_staff_bottom() { return m_staffBottom; }

    //helper
    LUnits tenths_to_logical(Tenths value, int iStaff=0);

protected:
    void measure_name_abbrev();
    void measure_brace_or_bracket();
    bool has_brace_or_bracket();
    void add_text_shape(GmoBox* pBox, const std::string& text);

};


}   //namespace lomse

#endif    // __LOMSE_INSTRUMENT_ENGRAVER_H__


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

#ifndef __LOMSE_INSTRUMENT_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_INSTRUMENT_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include <vector>
#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class ImoInstrument;
class ImoScore;
class FontStorage;
class GmoBox;
class GmoBoxSystem;

//---------------------------------------------------------------------------------------
class InstrumentEngraver
{
protected:
    ImoInstrument* m_pInstr;
    ImoScore* m_pScore;
    FontStorage* m_pFontStorage;
    LibraryScope& m_libraryScope;
    LUnits m_uIndentFirst;
    LUnits m_uIndentOther;
    LUnits m_uBracketWidth;
    LUnits m_uBracketGap;
    //int m_bracketSymbol;

    LUnits m_stavesTop;
    LUnits m_stavesBottom;
    LUnits m_stavesLeft;
    LUnits m_stavesWidth;

    std::vector<LUnits> m_staffTop;

public:
    InstrumentEngraver(ImoInstrument* pInstr, ImoScore* pScore, LibraryScope& libraryScope);
    ~InstrumentEngraver();

    //indents
    void measure_indents();
    LUnits get_indent_first() { return m_uIndentFirst; }
    LUnits get_indent_other() { return m_uIndentOther; }

    //shapes
    void add_staff_lines(GmoBoxSystem* pBox, LUnits x, LUnits y, LUnits indent);
    void add_name_abbrev(GmoBox* pBox, int nSystem);
    void add_brace_bracket(GmoBox* pBox);
    LUnits get_staves_bottom() { return m_stavesBottom; }
    LUnits get_staves_top() { return m_stavesTop; }
    LUnits get_top_of_staff(int iStaff);
    LUnits get_staves_width() { return m_stavesWidth; }
    LUnits get_staves_left() { return m_stavesLeft; }

    //helper
    LUnits tenths_to_logical(Tenths value, int iStaff=0);

protected:
    void measure_name_abbrev();
    void measure_brace_or_bracket();
    bool has_brace_or_bracket();

};


}   //namespace lomse

#endif    // __LOMSE_INSTRUMENT_ENGRAVER_H__


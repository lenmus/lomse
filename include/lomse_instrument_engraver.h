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

#ifndef __LOMSE_INSTRUMENT_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_INSTRUMENT_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

#include <vector>
#include <list>
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
class GmoShapeStaff;

//---------------------------------------------------------------------------------------
class InstrumentEngraver : public Engraver
{
protected:
    ImoInstrument* m_pInstr;
    ImoScore* m_pScore;
    FontStorage* m_pFontStorage;
    UPoint m_org;
    LUnits m_uIndentFirst;
    LUnits m_uIndentOther;
    LUnits m_uBracketWidth;
    LUnits m_uBracketGap;
    //int m_bracketSymbol;

    //vertical positions are relative to SystemBox origin
    LUnits m_stavesTop;
    LUnits m_stavesBottom;
    LUnits m_stavesLeft;
    LUnits m_stavesWidth;

    std::vector<LUnits> m_staffTop;
    std::vector<LUnits> m_staffTopLine;
    std::vector<LUnits> m_lineThickness;

public:
    InstrumentEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                       ImoInstrument* pInstr, ImoScore* pScore);
    ~InstrumentEngraver();

    void set_staves_horizontal_position(LUnits x, LUnits width, LUnits indent);
    void set_staves_vertical_position(LUnits y);
    inline void set_slice_instr_origin(UPoint org) { m_org = org; }

    //indents
    void measure_indents();
    LUnits get_indent_first() { return m_uIndentFirst; }
    LUnits get_indent_other() { return m_uIndentOther; }

    //shapes
    void add_staff_lines(GmoBoxSystem* pBox);
    void add_name_abbrev(GmoBoxSystem* pBox, int iSystem);
    void add_brace_bracket(GmoBoxSystem* pBox);
    inline LUnits get_staves_bottom() { return m_stavesBottom + m_org.y; }
    LUnits get_staves_top_line();
    LUnits get_staves_bottom_line();
    LUnits get_top_line_of_staff(int iStaff);
    inline LUnits get_staves_width() { return m_stavesWidth; }
    inline LUnits get_staves_left() { return m_stavesLeft; }
    inline LUnits get_staves_right() { return m_stavesLeft + m_stavesWidth; }

    //helper
    LUnits tenths_to_logical(Tenths value, int iStaff=0);

protected:
    void measure_name_abbrev();
    void measure_brace_or_bracket();
    bool has_brace_or_bracket();

};


}   //namespace lomse

#endif    // __LOMSE_INSTRUMENT_ENGRAVER_H__


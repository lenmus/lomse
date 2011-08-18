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
class ImoStyle;
class ImoScoreText;

//---------------------------------------------------------------------------------------
class TextEngraver : public Engraver
{
protected:
    const string& m_text;
    ImoStyle* m_pStyle;
    FontStorage* m_pFontStorage;

public:
    TextEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 const string& text, ImoStyle* pStyle);
    TextEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 ImoScoreText* pText, ImoScore* pScore);
    ~TextEngraver();

    GmoShapeText* create_shape(ImoObj* pCreatorImo, LUnits xLeft, LUnits yTop,
                               int valign);
    LUnits measure_width();
    LUnits measure_height();

};


}   //namespace lomse

#endif    // __LOMSE_TEXT_ENGRAVER_H__


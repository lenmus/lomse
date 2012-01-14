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

#ifndef __LOMSE_IM_FACTORY_H__        //to avoid nested includes
#define __LOMSE_IM_FACTORY_H__

#include "lomse_injectors.h"
#include "lomse_basic.h"
#include "lomse_im_note.h"

#include <string>
#include <map>
using namespace std;


namespace lomse
{

//forward declarations
class ImoObj;


//---------------------------------------------------------------------------------------
// ImFactory: Factory object to create ImoObj objects
class ImFactory
{
public:
    ImFactory() {}
    ~ImFactory() {}

    //factory injector, from type
    static ImoObj* inject(int type, Document* pDoc);

    //factory injector, from LDP source code
    static ImoObj* inject(Document* pDoc, const std::string& ldpSource);

    //specific injectors, to simplify testing
    static ImoNote* inject_note(Document* pDoc, int step, int octave,
                                int noteType, EAccidentals accidentals=k_no_accidentals,
                                int dots=0, int staff=0, int voice=0,
                                int stem=k_stem_default);

    static ImoBeamData* inject_beam_data(Document* pDoc, ImoBeamDto* pDto);
    static ImoTieData* inject_tie_data(Document* pDoc, ImoTieDto* pDto);
    static ImoTupletData* inject_tuplet_data(Document* pDoc, ImoTupletDto* pDto);
    static ImoSlurData* inject_slur_data(Document* pDoc, ImoSlurDto* pDto);
    static ImoTuplet* inject_tuplet(Document* pDoc, ImoTupletDto* pDto);
    static ImoTextBox* inject_text_box(Document* pDoc, ImoTextBlockInfo& dto);
    static ImoMultiColumn* inject_multicolumn(Document* pDoc);
    static ImoImage* inject_image(Document* pDoc, unsigned char* imgbuf,
                                  VSize bmpSize, EPixelFormat format, USize imgSize);
    static ImoControl* inject_control(Document* pDoc, Control* ctrol);

};


}   //namespace lomse

#endif    // __LOMSE_IM_FACTORY_H__


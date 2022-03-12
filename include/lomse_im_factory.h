//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
    static ImoObj* inject(int type, Document* pDoc, ImoId id=k_no_imoid);

    //factory injector, from LDP source code
    static ImoObj* inject(Document* pDoc, const std::string& ldpSource);

    //specific injectors, to simplify testing
    static ImoNote* inject_note(Document* pDoc, int step, int octave,
                                int noteType, EAccidentals accidentals=k_no_accidentals,
                                int dots=0, int staff=0, int voice=0,
                                int stem=k_stem_default);

    static ImoBeamData* inject_beam_data(Document* pDoc, ImoBeamDto* pDto);
    static ImoTieData* inject_tie_data(Document* pDoc, ImoTieDto* pDto);
    static ImoSlurData* inject_slur_data(Document* pDoc, ImoSlurDto* pDto);
    static ImoTuplet* inject_tuplet(Document* pDoc, ImoTupletDto* pDto);
    static ImoTextBox* inject_text_box(Document* pDoc, ImoTextBlockInfo& dto, ImoId id=k_no_imoid);
    static ImoMultiColumn* inject_multicolumn(Document* pDoc);
    static ImoImage* inject_image(Document* pDoc, unsigned char* imgbuf,
                                  VSize bmpSize, EPixelFormat format, USize imgSize);
    static ImoControl* inject_control(Document* pDoc, Control* ctrol);

};


}   //namespace lomse

#endif    // __LOMSE_IM_FACTORY_H__


//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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


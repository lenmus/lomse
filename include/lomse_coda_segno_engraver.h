//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2017. All rights reserved.
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

#ifndef __LOMSE_SEGNO_CODA_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_SEGNO_CODA_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoSymbolRepetitionMark;
class GmoShape;
class ScoreMeter;
class GmoShapeCodaSegno;

//---------------------------------------------------------------------------------------
class CodaSegnoEngraver : public Engraver
{
protected:
    ImoSymbolRepetitionMark* m_pRepetitionMark;
    GmoShape* m_pParentShape;
    GmoShapeCodaSegno* m_pCodaSegnoShape;

public:
    CodaSegnoEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                      int iInstr, int iStaff);
    ~CodaSegnoEngraver() {}

    GmoShapeCodaSegno* create_shape(ImoSymbolRepetitionMark* pRepetitionMark, UPoint pos,
                                    Color color=Color(0,0,0),
                                    GmoShape* pParentShape=nullptr);

protected:
    UPoint compute_location(UPoint pos);
    void center_on_parent();
    int find_glyph();

};


}   //namespace lomse

#endif    // __LOMSE_SEGNO_CODA_ENGRAVER_H__


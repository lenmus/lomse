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

#ifndef __LOMSE_NOTEREST_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_NOTEREST_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_im_note.h"

namespace lomse
{

//forward declarations
class GmoShape;
class FontStorage;
class GmoShapeBeam;
class GmoShapeTuplet;
class ScoreMeter;
class ShapesStorage;
class BeamEngraver;
class TupletEngraver;

//---------------------------------------------------------------------------------------
class NoterestEngraver : public Engraver
{
protected:
    ShapesStorage* m_pShapesStorage;
    LUnits m_lineSpacing;
    Color m_color;
    double m_fontSize;
    ImoNoteRest* m_pNoteRest;
    GmoShape* m_pNoteRestShape;

public:
    virtual ~NoterestEngraver() {}


protected:
    NoterestEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 ShapesStorage* pShapesStorage, int iInstr, int iStaff);

//    void layout_tuplet();

    inline bool is_beamed() { return m_pNoteRest->is_beamed(); }

    //helper
    inline bool has_stem() { return m_pNoteRest->get_note_type() >= k_half; }
    inline bool has_flag() { return m_pNoteRest->get_note_type() >= k_eighth; }
//    Tenths get_glyph_offset(int iGlyph);

    LUnits m_uyStaffTopLine;
    LUnits m_uxLeft, m_uyTop;       //current position
    GmoShapeBeam* m_pStartBeamShape;
    GmoShapeBeam* m_pEndBeamShape;
    GmoShapeTuplet* m_pTupletShape;
};


}   //namespace lomse

#endif    // __LOMSE_NOTEREST_ENGRAVER_H__


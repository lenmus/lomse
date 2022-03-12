//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_ACCIDENTALS_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_ACCIDENTALS_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_pitch.h"

namespace lomse
{

//forward declarations
class GmoShapeAccidentals;
class GmoShapeAccidental;
class GmoShape;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class AccidentalsEngraver : public StaffSymbolEngraver
{
protected:
    EAccidentals m_accidentals;
    bool m_fCautionary;
    GmoShapeAccidentals* m_pContainer;
    double m_fontSize;
    ImoNote* m_pNote;
    Color m_color;

public:
    AccidentalsEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                        int iInstr, int iStaff);
    ~AccidentalsEngraver() {}

    GmoShapeAccidentals* create_shape(ImoNote* pNote, UPoint uPos,
                                      EAccidentals accidentals, double fontSize,
                                      bool fCautionary=false, Color color=Color(0,0,0));

    //helper, for KeyEngraver or other
    GmoShapeAccidental* create_shape_for_glyph(int iGlyph, UPoint pos, Color color,
                                               double fontSize, ImoObj* pCreatorImo,
                                               ShapeId idx);
    static int get_glyph_for(int accidental);


protected:
    void find_glyphs();
    void create_container_shape(UPoint pos);
    void add_glyphs_to_container_shape(UPoint pos);
    void add_voice(VoiceRelatedShape* pVRS);
    LUnits glyph_offset(int iGlyph);

    int m_glyphs[4];
    int m_numGlyphs;
};


}   //namespace lomse

#endif    // __LOMSE_ACCIDENTALS_ENGRAVER_H__


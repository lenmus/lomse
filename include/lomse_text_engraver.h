//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
class GmoShapeTextBox;
class ImoStyle;
class ImoScoreText;
class VerticalProfile;

//---------------------------------------------------------------------------------------
class TextEngraver : public Engraver
{
protected:
    const string& m_text;
    ImoStyle* m_pStyle;
    FontStorage* m_pFontStorage;
    string m_language;
    int m_idxStaff;
    int m_classid;          //text class, enum E
    VerticalProfile* m_pVProfile;

public:
    TextEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 const string& text, const string& language, ImoStyle* pStyle,
                 int classid);
    ~TextEngraver();

    enum ETextClass
    {
        // 'id' attribute IS NOT added for these classes:
        k_class_lyric_elision,
        k_class_lyric_hyphenation,
        k_class_lyric_syllable,
        k_class_metronome_text,
        k_class_pedal_text,
        k_class_score_text,
        k_class_tuplet_text,
        k_class_volta_text,

        k_classes_with_id,     //to mark texts that will have 'id' attribute

        // 'id' attribute IS added for these classes:
        k_class_group_name,
        k_class_group_abbrev,
        k_class_instr_name,
        k_class_instr_abbrev,
        k_class_measure_number,
        k_class_score_title,
        k_class_repetition_mark,
    };

    GmoShapeText* create_shape(ImoObj* pCreatorImo, ShapeId idx, LUnits xLeft, LUnits yTop);
    LUnits measure_width();
    LUnits measure_height();
};

//---------------------------------------------------------------------------------------
class TextBoxEngraver : public Engraver
{
protected:
    const string& m_text;
    ImoStyle* m_pStyle;
    FontStorage* m_pFontStorage;
    string m_language;

public:
    TextBoxEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 const string& text, const string& language, ImoStyle* pStyle);
    ~TextBoxEngraver();

    GmoShapeTextBox* create_shape(ImoObj* pCreatorImo, LUnits xLeft, LUnits yTop);
    LUnits measure_width();
    LUnits measure_height();
};

//---------------------------------------------------------------------------------------
class MeasureNumberEngraver : public Engraver
{
protected:
    const string& m_text;
    ImoStyle* m_pStyle;
    FontStorage* m_pFontStorage;

public:
    MeasureNumberEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                          const string& text);
    ~MeasureNumberEngraver();

    GmoShapeText* create_shape(ImoObj* pCreator, ShapeId idx, LUnits xLeft, LUnits yTop);
    LUnits measure_width();
    LUnits measure_height();
};


}   //namespace lomse

#endif    // __LOMSE_TEXT_ENGRAVER_H__


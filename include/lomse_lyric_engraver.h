//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_LYRIC_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_LYRIC_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

#include <list>
#include <vector>

namespace lomse
{

//forward declarations
class ImoObj;
class GmoShapeLyrics;
class ScoreMeter;
class ImoLyric;
class ImoNote;
class GmoShapeNote;
class GmoShape;
class InstrumentEngraver;


//---------------------------------------------------------------------------------------
class LyricEngraver : public AuxRelObjEngraver
{
protected:
    GmoShapeLyrics* m_pLyricsShape;
    InstrumentEngraver* m_pInstrEngrv;
    std::list< std::pair<ImoLyric*, GmoShape*> > m_lyrics;
    std::vector<ShapeBoxInfo*> m_shapesInfo;
    UPoint m_origin;
    USize m_size;
    bool m_fLyricAbove;
    int m_numShapes;
    LUnits m_uStaffTop;             //top line of current staff
    LUnits m_uStaffLeft;
    LUnits m_uStaffRight;

    LUnits m_fontAscender;
    LUnits m_fontDescender;
    LUnits m_fontHeight;
    LUnits m_fontBase;      //measured from top border

public:
    LyricEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                  InstrumentEngraver* pInstrEngrv);
    ~LyricEngraver();

    //implementation of virtual methods from AuxRelObjEngraver
    void set_start_staffobj(ImoAuxRelObj* pARO, const AuxObjContext& aoc) override;
    void set_middle_staffobj(ImoAuxRelObj* pARO, const AuxObjContext& aoc) override;
    void set_end_staffobj(ImoAuxRelObj* pARO, const AuxObjContext& aoc) override;

    int create_shapes(const RelObjEngravingContext& ctx) override;
    int get_num_shapes() override { return m_numShapes; }
    ShapeBoxInfo* get_shape_box_info(int i) override { return m_shapesInfo[i]; }

    //specific methods for this engraver
    void prepare_for_next_system();


protected:

    void create_shape(int note, GmoShapeNote* pNoteShape, ImoLyric* pLyric,
                      GmoShapeNote* pNextNoteShape, LUnits xLeft, LUnits xRight);
    void decide_placement();
    void measure_text_height(ImoLyric* pLyric);

};


}   //namespace lomse

#endif    // __LOMSE_LYRIC_ENGRAVER_H__


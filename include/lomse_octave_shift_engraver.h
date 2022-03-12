//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_OCTAVE_SHIFT_H__        //to avoid nested includes
#define __LOMSE_OCTAVE_SHIFT_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoOctaveShiftData;
class ImoOctaveShift;
class ImoNote;
class GmoShapeOctaveShift;
class GmoShapeNote;
class ScoreMeter;
class InstrumentEngraver;

//---------------------------------------------------------------------------------------
class OctaveShiftEngraver : public RelObjEngraver
{
protected:
    int m_numShapes;
    ImoOctaveShift* m_pOctaveShift;
    GmoShape* m_pShapeNumeral;
    GmoShapeOctaveShift* m_pMainShape;

    ImoNote* m_pStartNote;
    ImoNote* m_pEndNote;
    GmoShapeNote* m_pStartNoteShape;
    GmoShapeNote* m_pEndNoteShape;

    bool m_fPlaceAtTop;
    UPoint m_points[2];  //points for a shape (top-left, bottom-left).

public:
    OctaveShiftEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~OctaveShiftEngraver() {}

    void set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;
    void set_end_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;

    //RelObjEngraver mandatory overrides
    GmoShape* create_first_or_intermediate_shape(const RelObjEngravingContext& ctx) override;
    GmoShape* create_last_shape(const RelObjEngravingContext& ctx) override;

protected:
    void decide_placement();
    inline bool is_end_point_set() { return m_pEndNote != nullptr; }
    GmoShape* create_single_shape();
    GmoShape* create_first_shape();
    GmoShape* create_intermediate_shape();
    GmoShape* create_final_shape();

    inline bool octave_shift_at_top() { return m_fPlaceAtTop; }

    void create_main_container_shape();
    void add_shape_numeral();
    void add_line_info();
    int find_glyph(int shift);

    void compute_first_shape_position();
    void compute_second_shape_position();
    void compute_intermediate_shape_position();
    //void add_user_displacements(int iOctaveShift, UPoint* points);
    LUnits determine_top_line_of_shape();

};


}   //namespace lomse

#endif    // __LOMSE_OCTAVE_SHIFT_H__


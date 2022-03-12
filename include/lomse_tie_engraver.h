//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_TIE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_TIE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoTie;
class ImoNote;
class GmoShapeTie;
class GmoShapeNote;
class ScoreMeter;
class VoiceRelatedShape;

//---------------------------------------------------------------------------------------
class TieEngraver : public RelObjEngraver
{
protected:
    ImoTie* m_pTie;
    int m_numShapes;
    ImoNote* m_pStartNote;
    ImoNote* m_pEndNote;
    GmoShapeNote* m_pStartNoteShape;
    GmoShapeNote* m_pEndNoteShape;
    UPoint m_points[4];    //bezier points for an arch
    LUnits m_thickness;
    bool m_fTieBelowNote;

public:
    TieEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~TieEngraver() {}

    void set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;
    void set_end_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;

    //RelObjEngraver mandatory overrides
    GmoShape* create_first_or_intermediate_shape(const RelObjEngravingContext& ctx) override;
    GmoShape* create_last_shape(const RelObjEngravingContext& ctx) override;


protected:
    void decide_placement();
    inline bool is_end_point_set() { return m_pEndNoteShape != nullptr; }
    GmoShape* create_single_shape();
    GmoShape* create_first_shape();
    GmoShape* create_intermediate_shape();
    GmoShape* create_final_shape();


    void decide_if_one_or_two_arches();

    void add_voice(VoiceRelatedShape* pVRS);
    void compute_start_point();
    void compute_end_point(UPoint* point);
    void compute_start_of_staff_point();
    void compute_end_of_staff_point();
    void compute_default_control_points(UPoint* points);
    void add_user_displacements(int iTie, UPoint* points);

};


}   //namespace lomse

#endif    // __LOMSE_TIE_ENGRAVER_H__


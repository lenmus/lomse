//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#ifndef __LOMSE_PEDAL_ENGRAVER_H__
#define __LOMSE_PEDAL_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoPedalMark;
class ImoPedalLine;
class ImoDirection;
class GmoShapeInvisible;
class GmoShapePedalLine;
class GmoShapePedalGlyph;
class ScoreMeter;
class InstrumentEngraver;
class VerticalProfile;

//---------------------------------------------------------------------------------------
class PedalMarkEngraver : public Engraver
{
    int m_idxStaff;
    VerticalProfile* m_pVProfile;

public:
    PedalMarkEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                      int iInstr, int iStaff, int idxStaff, VerticalProfile* pVProfile);

    GmoShapePedalGlyph* create_shape(ImoPedalMark* pPedalMark, UPoint pos,
                                     const Color& color,
                                     GmoShape* pParentShape);

protected:
    double determine_font_size() override;
    static int find_glyph(const ImoPedalMark* pPedalMark);
    LUnits determine_y_pos(LUnits xLeft, LUnits xRight, LUnits yCurrent, LUnits yBaselineOffset, bool fAbove) const;
};

//---------------------------------------------------------------------------------------
class PedalLineEngraver : public RelObjEngraver
{
protected:

    InstrumentEngraver* m_pInstrEngrv = nullptr;
    LUnits m_uStaffTop = 0.0f;  //top line of current system
    LUnits m_uStaffLeft = 0.0f;

    int m_numShapes = 0;
    ImoPedalLine* m_pPedal = nullptr;
    bool m_fPedalAbove = false;
    LUnits m_uPrologWidth = 0.0f;

    ImoDirection* m_pStartDirection = nullptr;
    GmoShapeInvisible* m_pStartDirectionShape = nullptr;
    GmoShapeInvisible* m_pEndDirectionShape = nullptr;
    std::vector<GmoShapeInvisible*> m_pedalChangeDirectionShapes;

    const GmoShapePedalGlyph* m_pPedalStartShape = nullptr;
    const GmoShapePedalGlyph* m_pPedalEndShape = nullptr;

    LUnits m_xStart = 0;
    LUnits m_xEnd = 0;
    LUnits m_lineY = 0;

public:
    PedalLineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 InstrumentEngraver* pInstrEngrv);

    void set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol,
                            LUnits xStaffLeft, LUnits xStaffRight, LUnits yTop,
                            int idxStaff, VerticalProfile* pVProfile) override;
    void set_middle_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                             GmoShape* pStaffObjShape, int iInstr, int iStaff,
                             int iSystem, int iCol,
                             LUnits xStaffLeft, LUnits xStaffRight, LUnits yTop,
                             int idxStaff, VerticalProfile* pVProfile) override;
    void set_end_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol,
                          LUnits xStaffLeft, LUnits xStaffRight, LUnits yTop,
                          int idxStaff, VerticalProfile* pVProfile) override;

    //RelObjEngraver mandatory overrides
    void set_prolog_width(LUnits width) override;
    GmoShape* create_first_or_intermediate_shape(LUnits xStaffLeft, LUnits xStaffRight,
                                                 LUnits yStaffTop, LUnits prologWidth,
                                                 VerticalProfile* pVProfile,
                                                 Color color=Color(0,0,0)) override;
    GmoShape* create_last_shape(Color color=Color(0,0,0)) override;

protected:
    double determine_font_size() override;

    void decide_placement();

    GmoShape* create_shape();
    void compute_shape_position(bool first);
    LUnits determine_shape_position_left(bool first) const;
    LUnits determine_shape_position_right() const;
    LUnits determine_default_base_line_y() const;
    void adjust_vertical_position(LUnits xLeft, LUnits xRight, LUnits height, GmoShapePedalLine* pMainShape = nullptr);

    void add_pedal_changes(GmoShapePedalLine* pMainShape);
    void add_pedal_continuation_text(GmoShapePedalLine* pMainShape);
    void add_pedal_continuation_part_shape(GmoShapePedalLine* pMainShape, GmoShape* pAddedShape);

    void add_line_info(GmoShapePedalLine* pMainShape, const GmoShape* pPedalStartShape, const GmoShape* pPedalEndShape, bool start);
};


}   //namespace lomse

#endif    // __LOMSE_PEDAL_ENGRAVER_H__


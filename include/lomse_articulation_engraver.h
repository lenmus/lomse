//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#ifndef __LOMSE_ARTICULATION_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_ARTICULATION_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoArticulation;
class GmoShape;
class ScoreMeter;
class GmoShapeArticulation;
class VerticalProfile;

//---------------------------------------------------------------------------------------
class ArticulationEngraver : public Engraver
{
protected:
    ImoArticulation* m_pArticulation;
    int m_placement;
    bool m_fAbove;
    bool m_fEnableShiftWhenCollision;
    GmoShape* m_pParentShape;
    GmoShapeArticulation* m_pArticulationShape;
    int m_idxStaff;
    VerticalProfile* m_pVProfile;

public:
    ArticulationEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                    int iInstr, int iStaff, int idxStaff, VerticalProfile* pVProfile);
    ~ArticulationEngraver() {}

    GmoShapeArticulation* create_shape(ImoArticulation* pArticulation, UPoint pos,
                                       Color color=Color(0,0,0),
                                       GmoShape* pParentShape=nullptr);

protected:
    bool determine_if_above();
    UPoint compute_location(UPoint pos);
    void center_on_parent();
    void add_voice();
    int find_glyph();
    bool must_be_placed_outside_staff();
    bool is_accent_articulation();
    void shift_shape_if_collision();

};


}   //namespace lomse

#endif    // __LOMSE_ARTICULATION_ENGRAVER_H__


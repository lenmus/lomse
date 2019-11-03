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

#ifndef __LOMSE_TUPLET_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_TUPLET_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class GmoShapeTuplet;
class GmoShapeBeam;
class ScoreMeter;
class ImoTuplet;
class ImoNote;
class GmoShapeNote;
class ImoNoteRest;
class GmoShape;
class ImoStyle;


//---------------------------------------------------------------------------------------
class TupletEngraver : public RelObjEngraver
{
protected:
    int m_numShapes;
    GmoShapeTuplet* m_pTupletShape;
    string m_label;
    ImoStyle* m_pStyle;
    ImoTuplet* m_pTuplet;
    std::list< pair<ImoNoteRest*, GmoShape*> > m_noteRests;
    bool m_fDrawBracket;
    bool m_fDrawNumber;
    bool m_fAbove;

public:
    TupletEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~TupletEngraver();

    //implementation of virtual methods from RelObjEngraver
    void set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol,
                            LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                            int idxStaff, VerticalProfile* pVProfile) override;
    void set_middle_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                             GmoShape* pStaffObjShape, int iInstr, int iStaff,
                             int iSystem, int iCol,
                             LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                             int idxStaff, VerticalProfile* pVProfile) override;
    void set_end_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol,
                          LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                          int idxStaff, VerticalProfile* pVProfile) override;

    //RelObjEngraver mandatory overrides
    void set_prolog_width(LUnits UNUSED(width)) override {}
    GmoShape* create_first_or_intermediate_shape(Color color=Color(0,0,0)) override;
    GmoShape* create_last_shape(Color color=Color(0,0,0)) override;

protected:
    void add_text_shape();
    bool check_if_single_beamed_group();

    ImoNoteRest* get_start_noterest() { return m_noteRests.front().first; }
    ImoNoteRest* get_end_noterest() { return m_noteRests.back().first; }

    void compute_y_coordinates();
    GmoShapeNote* get_first_note();
    GmoShapeNote* get_last_note();

    GmoShape* get_start_noterest_shape() { return m_noteRests.front().second; }
    GmoShape* get_end_noterest_shape() { return m_noteRests.back().second; }

    LUnits m_yStart, m_yEnd;

    void decide_if_show_bracket();
    void decide_tuplet_placement();
    void determine_tuplet_text();
    void create_shape();
    void set_shape_details();
    int count_nested_tuplets();

};


}   //namespace lomse

#endif    // __LOMSE_TUPLET_ENGRAVER_H__


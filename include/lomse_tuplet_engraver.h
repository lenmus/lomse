//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
class TupletEngraver : public RelAuxObjEngraver
{
protected:
    int m_iInstr;
    int m_iStaff;
    int m_numShapes;
    UPoint m_pos;
    GmoShapeTuplet* m_pTupletShape;
    string m_label;
    ImoStyle* m_pStyle;
    ImoTuplet* m_pTuplet;
    std::list< pair<ImoNoteRest*, GmoShape*> > m_noteRests;
    ShapeBoxInfo m_shapesInfo[1];
    bool m_fDrawBracket;
    bool m_fDrawNumber;

public:
    TupletEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~TupletEngraver();

    //implementation of virtual methods from RelAuxObjEngraver
    void set_start_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol, UPoint pos);
    void set_middle_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                             GmoShape* pStaffObjShape, int iInstr, int iStaff,
                             int iSystem, int iCol);
    void set_end_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol);
    int create_shapes();
    int get_num_shapes() { return m_numShapes; }
    ShapeBoxInfo* get_shape_box_info(int i) { return &m_shapesInfo[0]; }

protected:
    void add_text_shape();
    LUnits tenths_to_logical(Tenths tenths);
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
    bool decide_if_tuplet_placement_above();
    void determine_tuplet_text();
    void create_shape();
    void set_shape_details();

};


}   //namespace lomse

#endif    // __LOMSE_TUPLET_ENGRAVER_H__


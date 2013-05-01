//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_CARET_POSITIONER_H__
#define __LOMSE_CARET_POSITIONER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_document_cursor.h"
#include "lomse_score_meter.h"

//other
#include <iostream>
using namespace std;


namespace lomse
{

//forward declarations
class Caret;
class GmoBox;
class GraphicModel;
class InnerLevelCaretPositioner;


//---------------------------------------------------------------------------------------
// CaretPositioner:
// responsible for translating cursor state into graphical model position
class CaretPositioner
{
protected:
    DocCursor* m_pCursor;

public:
    CaretPositioner(DocCursor* pCursor);
    virtual ~CaretPositioner() {}

    //operations
    void prepare_caret(Caret* pCaret, GraphicModel* pGModel);


protected:
    InnerLevelCaretPositioner* new_positioner(DocCursor* pCursor,
                                              GraphicModel* pGModel);

};

//---------------------------------------------------------------------------------------
// TopLevelCaretPositioner
// responsible for positioning caret at start of current top level element:
class TopLevelCaretPositioner
{
protected:
    DocCursor* m_pCursor;
    GraphicModel* m_pGModel;
    DocCursorState m_state;

public:
    TopLevelCaretPositioner(DocCursor* pCursor, GraphicModel* pGModel);
    virtual ~TopLevelCaretPositioner() {}

    void prepare_caret(Caret* pCaret);

protected:
    GmoBox* get_box_for_last_element();

};


//---------------------------------------------------------------------------------------
// InnerLevelPositioner. Base class for any object responsible for positioning caret
// at current place in an inner level element
class InnerLevelCaretPositioner
{
protected:
    GraphicModel* m_pGModel;

    InnerLevelCaretPositioner(GraphicModel* pGModel);

public:
    virtual ~InnerLevelCaretPositioner() {}

    virtual void prepare_caret(Caret* pCaret) = 0;
};


//---------------------------------------------------------------------------------------
// ScoreCaretPositioner.
// responsible for positioning caret at current place in a score
class ScoreCaretPositioner : public InnerLevelCaretPositioner
{
protected:
    DocCursor* m_pDocCursor;
    ScoreCursor* m_pScoreCursor;
    ScoreCursorState* m_pState;
    Document* m_pDoc;
    ImoScore* m_pScore;
    ScoreMeter* m_pMeter;

public:
    ScoreCaretPositioner(DocCursor* pCursor, GraphicModel* pGModel);
    virtual ~ScoreCaretPositioner();

    void prepare_caret(Caret* pCaret);

protected:
    void caret_on_pointed_object(Caret* pCaret);
    void caret_on_empty_timepos(Caret* pCaret);
    void caret_at_start_of_score(Caret* pCaret);
    void caret_at_end_of_staff(Caret* pCaret);

    URect get_bounds_for_imo(ImoId id);
    LUnits tenths_to_logical(Tenths value);
    void set_caret_timecode(Caret* pCaret);
    void set_caret_y_pos_and_height(URect* pBounds, ImoId id);

};


}   //namespace lomse

#endif      //__LOMSE_CARET_POSITIONER_H__
//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
class GmoBoxSystem;
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
    CaretPositioner();
    virtual ~CaretPositioner() {}

    //operations
    void layout_caret(Caret* pCaret, DocCursor* pCursor, GraphicModel* pGModel);
    DocCursorState click_point_to_cursor_state(GraphicModel* pGModel, int iPage, LUnits x,
                                         LUnits y, ImoObj* pImo, GmoObj* pGmo);


protected:
    InnerLevelCaretPositioner* new_positioner(ImoObj* pTopLevel, GraphicModel* pGModel);

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
    TopLevelCaretPositioner(GraphicModel* pGModel);
    virtual ~TopLevelCaretPositioner() {}

    void layout_caret(Caret* pCaret, DocCursor* pCursor);

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

    virtual void layout_caret(Caret* pCaret, DocCursor* pCursor) = 0;
    virtual SpElementCursorState click_point_to_cursor_state(int iPage, LUnits x, LUnits y,
                                                 ImoObj* pImo, GmoObj* pGmo) = 0;
};


//---------------------------------------------------------------------------------------
// ScoreCaretPositioner.
// responsible for positioning caret at current place in a score
class ScoreCaretPositioner : public InnerLevelCaretPositioner
{
protected:
    DocCursor* m_pDocCursor;
    ScoreCursor* m_pScoreCursor;
    SpScoreCursorState m_spState;
    Document* m_pDoc;
    ImoScore* m_pScore;
    ScoreMeter* m_pMeter;
    GmoBoxSystem* m_pBoxSystem;

public:
    ScoreCaretPositioner(GraphicModel* pGModel);
    virtual ~ScoreCaretPositioner();

    void layout_caret(Caret* pCaret, DocCursor* pCursor) override;
    SpElementCursorState click_point_to_cursor_state(int iPage, LUnits x, LUnits y,
                                               ImoObj* pImo, GmoObj* pGmo) override;

protected:
    void caret_on_pointed_object(Caret* pCaret);
    void caret_on_empty_timepos(Caret* pCaret);
    void caret_at_start_of_score(Caret* pCaret);
    void caret_at_end_of_staff(Caret* pCaret);

    URect get_bounds_for_imo(ImoId id, int iStaff);
    LUnits tenths_to_logical(Tenths value);
    void set_caret_timecode(Caret* pCaret);
    void set_caret_y_pos_and_height(URect* pBounds, ImoId id, int iStaff);
    GmoShape* get_shape_for_imo(ImoId id, int iStaff);

};


}   //namespace lomse

#endif      //__LOMSE_CARET_POSITIONER_H__

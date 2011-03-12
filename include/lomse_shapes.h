//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPES_H__        //to avoid nested includes
#define __LOMSE_SHAPES_H__

#include "lomse_shape_base.h"
#include "lomse_injectors.h"
//#include <sstream>
//using namespace std;

namespace lomse
{

//forward declarations
class FontStorage;


//---------------------------------------------------------------------------------------
// a shape drawn by using a single glyph from LenMus font
class GmoShapeGlyph : public GmoSimpleShape
{
protected:
    unsigned int m_glyph;
    USize m_shiftToDraw;
    FontStorage* m_pFontStorage;
    LibraryScope& m_libraryScope;
    double m_fontHeight;

public:
    virtual ~GmoShapeGlyph() {}

    //implementation of virtual methods from base class
    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt);
//    void RenderHighlighted(wxDC* pDC, Color colorC);

//    void Shift(LUnits xIncr, LUnits yIncr);
//	virtual wxBitmap* OnBeginDrag(double rScale, wxDC* pDC);
//    virtual UPoint OnDrag(lmPaper* pPaper, const UPoint& uPos);
//	virtual void OnEndDrag(lmPaper* pPaper, lmInteractor* pCanvas, const UPoint& uPos);
//	UPoint GetObjectOrigin();

protected:
    GmoShapeGlyph(ImoObj* pCreatorImo, int type, int nShapeIdx, unsigned int nGlyph,
                  UPoint pos, Color color, LibraryScope& libraryScope,
                  double fontHeight);

//    wxBitmap* GetBitmapFromShape(double rScale, Color colorF, Color colorB = *wxWHITE);
//    virtual double GetPointSize();

};

//---------------------------------------------------------------------------------------
class GmoShapeClef : public GmoShapeGlyph
{
public:
    GmoShapeClef(ImoObj* pCreatorImo, int nShapeIdx, int nGlyph, UPoint pos, Color color,
                 LibraryScope& libraryScope, double fontSize);
    ~GmoShapeClef() {}

//	//overrides
//    UPoint OnDrag(lmPaper* pPaper, const UPoint& uPos);
//    void OnEndDrag(lmPaper* pPaper, lmInteractor* pCanvas, const UPoint& uPos);
//    double GetPointSize();
//
};

//---------------------------------------------------------------------------------------
class GmoShapeFermata : public GmoShapeGlyph
{
public:
    GmoShapeFermata(ImoObj* pCreatorImo, int nShapeIdx, int nGlyph, UPoint pos,
                    Color color, LibraryScope& libraryScope, double fontSize);
    ~GmoShapeFermata() {}
};

//---------------------------------------------------------------------------------------
class GmoShapeSimpleLine : public GmoSimpleShape
{
protected:
    LUnits		m_uWidth;
	LUnits		m_uBoundsExtraWidth;
	ELineEdge	m_nEdge;

public:
    virtual ~GmoShapeSimpleLine() {}

    //implementation of virtual methods from base class
    void on_draw(Drawer* pDrawer, RenderOptions& opt);

protected:
    GmoShapeSimpleLine(ImoObj* pCreatorImo, int type, LUnits xStart, LUnits yStart,
                       LUnits xEnd, LUnits yEnd, LUnits uWidth, LUnits uBoundsExtraWidth,
                       Color color, ELineEdge nEdge = k_edge_normal);
    void set_new_values(LUnits xStart, LUnits yStart, LUnits xEnd, LUnits yEnd,
                        LUnits uWidth, LUnits uBoundsExtraWidth,
                        Color color, ELineEdge nEdge);

};

////---------------------------------------------------------------------------------------
//class GmoShapeRectangle : public GmoSimpleShape
//{
//public:
//    //TODO: remove this backwards compatibility constructor
//    GmoShapeRectangle(GmoBox* owner, LUnits xLeft, LUnits yTop,
//                     LUnits xRight, LUnits yBottom, LUnits uWidth,
//                     Color color = *wxBLACK, wxString sName = _T("Rectangle"),
//				     bool fDraggable = true, bool fSelectable = true,
//                     bool fVisible = true);
//
//    //new rectangle constructor
//    GmoShapeRectangle(GmoBox* owner,
//                     //position and size
//                     LUnits uxLeft, LUnits uyTop, LUnits uxRight, LUnits uyBottom,
//                     //border
//                     LUnits uBorderWidth, Color nBorderColor,
//                     //content
//                     Color nBgColor = *wxWHITE,
//                     //other
//                     int nShapeIdx = 0, wxString sName = _T("Rectangle"),
//				     bool fDraggable = true, bool fSelectable = true,
//                     bool fVisible = true);
//
//    virtual ~GmoShapeRectangle();
//
//    //implementation of virtual methods from base class
//    void on_draw(Drawer* pDrawer, RenderOptions& opt);
//    void RenderNormal(lmPaper* pPaper, Color color);
//    void RenderWithHandlers(lmPaper* pPaper);
//    void Shift(LUnits uxIncr, LUnits uyIncr);
//
//    //Handler IDs. AWARE: Used also as array indexes
//    enum
//    {
//        lmID_TOP_LEFT = 0,
//        lmID_TOP_RIGHT,
//        lmID_BOTTOM_RIGHT,
//        lmID_BOTTOM_LEFT,
//        //
//        lmID_LEFT_CENTER,
//        lmID_TOP_CENTER,
//        lmID_RIGHT_CENTER,
//        lmID_BOTTOM_CENTER,
//        //
//        lmID_NUM_HANDLERS
//    };
//
//    //settings
//    void SetCornerRadius(LUnits uRadius);
//    inline void SetBorderStyle(ELineStyle nBorderStyle) { m_nBorderStyle = nBorderStyle; }
//
//    //shape dragging
//    wxBitmap* OnBeginDrag(double rScale, wxDC* pDC);
//	UPoint OnDrag(lmPaper* pPaper, const UPoint& uPos);
//	void OnEndDrag(lmPaper* pPaper, lmInteractor* pCanvas, const UPoint& uPos);
//
//    //handlers dragging
//    UPoint OnHandlerDrag(lmPaper* pPaper, const UPoint& uPos, long nHandlerID);
//    void OnHandlerEndDrag(lmInteractor* pCanvas, const UPoint& uPos, long nHandlerID);
//
//    //call backs
//    void MovePoints(int nNumPoints, int nShapeIdx, UPoint* pShifts, bool fAddShifts);
//
//
//protected:
//    void Create(LUnits xLeft, LUnits yTop, LUnits xRight, LUnits yBottom,
//                LUnits uBorderWidth, Color nBorderColor, Color nBgColor);
//    void DrawRectangle(lmPaper* pPaper, Color colorC, bool fSketch);
//    void ComputeNewPointsAndHandlersPositions(const UPoint& uPos, long nHandlerID);
//    void ComputeCenterPoints();
//    void UpdateBounds();
//    void SavePoints();
//
//    //rectangle
//    Color        m_nBgColor;
//    LUnits        m_uCornerRadius;
//
//    //border
//    LUnits        m_uBorderWidth;
//    Color        m_nBorderColor;
//    ELineStyle    m_nBorderStyle;
//
//    //rectangle points and handlers
//    UPoint            m_uPoint[lmID_NUM_HANDLERS];       //four corners + anchor point + centers of rectangle sides
//    UPoint            m_uSavePoint[lmID_NUM_HANDLERS];   //to save start and end points when dragging/moving
//    lmHandlerSquare*    m_pHandler[lmID_NUM_HANDLERS];     //handlers
//
//};

//---------------------------------------------------------------------------------------
class GmoShapeInvisible : public GmoSimpleShape
{
public:
    GmoShapeInvisible(ImoObj* pCreatorImo, int idx, UPoint uPos, USize uSize);
    ~GmoShapeInvisible() {}

//	//overrides
//	void on_draw(Drawer* pDrawer, RenderOptions& opt);
};

//---------------------------------------------------------------------------------------
class GmoShapeStem : public GmoShapeSimpleLine
{
private:
	bool m_fStemDown;
    LUnits m_uExtraLength;

public:
    GmoShapeStem(ImoObj* pCreatorImo, LUnits xPos, LUnits yStart, LUnits uExtraLength,
                 LUnits yEnd, bool fStemDown, LUnits uWidth, Color color);
    ~GmoShapeStem() {}

    void change_length(LUnits length);
	inline bool is_stem_down() const { return m_fStemDown; }
    void set_stem_up(LUnits xRight, LUnits yNote);
    void set_stem_down(LUnits xLeft, LUnits yNote);
	void adjust(LUnits xLeft, LUnits yTop, LUnits height, bool fStemDown);
    inline LUnits get_extra_length() { return m_uExtraLength; }
    LUnits get_y_note();
    LUnits get_y_flag();

};

////---------------------------------------------------------------------------------------
//class GmoShapeFiguredBass : public lmCompositeShape
//{
//public:
//    GmoShapeFiguredBass(GmoBox* owner, int nShapeIdx, Color nColor)
//        : lmCompositeShape(pOwner, nShapeIdx, nColor, _T("Figured bass"), true)  //true= lmDRAGGABLE
//        { m_nType = GmoObj::k_shape_FiguredBass; }
//    ~GmoShapeFiguredBass() {}
//
//	//info about related shapes
//    inline void OnFBLineAttached(int nLine, GmoShapeLine* pShapeFBLine)
//                    { m_pFBLineShape[nLine] = pShapeFBLine; }
//
//    //overrides
//    void Shift(LUnits uxIncr, LUnits uyIncr);
//
//private:
//    GmoShapeLine*  m_pFBLineShape[2];     //The two lines of a FB line. This is the end FB of the line
//
//};
//
////---------------------------------------------------------------------------------------
//class GmoShapeWindow : public GmoShapeRectangle
//{
//public:
//    GmoShapeWindow(GmoBox* owner, int nShapeIdx,
//                  //position and size
//                  LUnits uxLeft, LUnits uyTop, LUnits uxRight, LUnits uyBottom,
//                  //border
//                  LUnits uBorderWidth, Color nBorderColor,
//                  //content
//                  Color nBgColor = *wxWHITE,
//                  //other
//                  wxString sName = _T("Window"),
//				  bool fDraggable = true, bool fSelectable = true,
//                  bool fVisible = true);
//    virtual ~GmoShapeWindow() {}
//
//    //renderization
//    void on_draw(Drawer* pDrawer, RenderOptions& opt);
//
//	//specific methods
//
//protected:
//
//    wxWindow*       m_pWidget;      //the window to embbed
//};
//
////global functions defined in this module
//extern wxBitmap* GetBitmapForGlyph(double rScale, int nGlyph, double rPointSize,
//                                   Color colorF, Color colorB);
//

//---------------------------------------------------------------------------------------
class GmoShapeAccidentals : public GmoCompositeShape
{
public:
    GmoShapeAccidentals(ImoObj* pCreatorImo, int idx, UPoint pos, Color color)
        : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_accidentals, idx, color)
    {
    }
};

//---------------------------------------------------------------------------------------
class GmoShapeAccidental : public GmoShapeGlyph
{
public:
    GmoShapeAccidental(ImoObj* pCreatorImo, int idx, unsigned int iGlyph, UPoint pos,
                       Color color, LibraryScope& libraryScope, double fontSize)
        : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_accidental_sign, idx, iGlyph,
                        pos, color, libraryScope, fontSize)
    {
    }

};

//---------------------------------------------------------------------------------------
class GmoShapeDigit : public GmoShapeGlyph
{
public:
    GmoShapeDigit(ImoObj* pCreatorImo, int idx, unsigned int iGlyph, UPoint pos,
                  Color color, LibraryScope& libraryScope, double fontSize)
        : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_time_signature_digit, idx, iGlyph,
                        pos, color, libraryScope, fontSize)
    {
    }

};

//---------------------------------------------------------------------------------------
class GmoShapeKeySignature : public GmoCompositeShape
{
protected:
    LibraryScope& m_libraryScope;

public:
    GmoShapeKeySignature(ImoObj* pCreatorImo, int idx, UPoint pos, Color color,
                         LibraryScope& libraryScope)
        : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_key_signature, idx, color)
        , m_libraryScope(libraryScope)
    {
    }
};

//---------------------------------------------------------------------------------------
class GmoShapeTimeSignature : public GmoCompositeShape
{
protected:
    LibraryScope& m_libraryScope;

public:
    GmoShapeTimeSignature(ImoObj* pCreatorImo, int idx, UPoint pos, Color color, 
                          LibraryScope& libraryScope)
        : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_time_signature, idx, color)
        , m_libraryScope(libraryScope)
    {
    }
};


}   //namespace lomse

#endif    // __LOMSE_SHAPES_H__


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

namespace lomse
{

//forward declarations
class FontStorage;
class ImoButton;
class ImoStyle;

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

    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt);

protected:
    GmoShapeGlyph(ImoObj* pCreatorImo, int type, int nShapeIdx, unsigned int nGlyph,
                  UPoint pos, Color color, LibraryScope& libraryScope,
                  double fontHeight);

    void compute_size_origin(double fontHeight, UPoint pos);

};

//---------------------------------------------------------------------------------------
class GmoShapeClef : public GmoShapeGlyph
{
public:
    GmoShapeClef(ImoObj* pCreatorImo, int nShapeIdx, int nGlyph, UPoint pos, Color color,
                 LibraryScope& libraryScope, double fontSize);
    ~GmoShapeClef() {}
};

//---------------------------------------------------------------------------------------
class GmoShapeButton : public GmoSimpleShape
{
protected:
    FontStorage* m_pFontStorage;
    LibraryScope& m_libraryScope;
    ImoButton* m_pButton;
    LUnits m_xLabel;
    LUnits m_yLabel;

public:
    GmoShapeButton(ImoObj* pCreatorImo, UPoint pos, USize size,
                   LibraryScope& libraryScope);
    ~GmoShapeButton() {}

    void on_draw(Drawer* pDrawer, RenderOptions& opt);

protected:
    void select_font();
    void center_text();

    //Color get_normal_color();
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

//---------------------------------------------------------------------------------------
class GmoShapeRectangle : public GmoSimpleShape
{
protected:
    LUnits m_radius;

public:
    GmoShapeRectangle(ImoObj* pCreatorImo
                      , int type = GmoObj::k_shape_rectangle
                      , int idx = 0
                      , const UPoint& pos = UPoint(0.0f, 0.0f)    //top-left corner
                      , const USize& size = USize(0.0f, 0.0f)     //rectangle size
                      , LUnits radius = 0.0f                      //for rounded corners
                      , ImoStyle* pStyle = NULL       //for line style & background color
                     );
    virtual ~GmoShapeRectangle() {}


    //implementation of virtual methods from base class
    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt);

    //settings
    inline void set_radius(LUnits radius) { m_radius = radius; }

};

//---------------------------------------------------------------------------------------
class GmoShapeInvisible : public GmoSimpleShape
{
public:
    GmoShapeInvisible(ImoObj* pCreatorImo, int idx, UPoint uPos, USize uSize);
    ~GmoShapeInvisible() {}
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


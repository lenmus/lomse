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

#ifndef __LOMSE_SHAPE_NOTE_H__        //to avoid nested includes
#define __LOMSE_SHAPE_NOTE_H__

#include "lomse_shape_base.h"
#include "lomse_shapes.h"
#include "lomse_basic.h"
#include "lomse_injectors.h"
#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class GmoBox;
class GmoShapeNotehead;
class GmoShapeStem;
class GmoShapeFlag;
class FontStorage;

//---------------------------------------------------------------------------------------
class GmoShapeNote : public GmoCompositeShape
{
protected:
    FontStorage* m_pFontStorage;
    LibraryScope& m_libraryScope;
    GmoShapeNotehead* m_pNoteheadShape;
	GmoShapeStem* m_pStemShape;

public:
//    GmoShapeNote(lmNoteRest* pOwner, LUnits xPos, LUnits yTop, Color color);
    GmoShapeNote(LUnits x, LUnits y, Color color, LibraryScope& libraryScope);
    ~GmoShapeNote();


//	//overrides of virtual methods in base class
//	void Shift(LUnits xIncr, LUnits yIncr);
    void on_draw(Drawer* pDrawer, RenderOptions& opt);

//	//specific methods
	void add_stem(GmoShapeStem* pShape);
	void add_notehead(GmoShapeNotehead* pShape);
	void add_flag(GmoShapeFlag* pShape);
//	void AddAccidental(GmoShape* pShape);
	void add_note_in_block(GmoShape* pShape);
//	void AddLegerLinesInfo(int nPosOnStaff, LUnits uyStaffTopLine);
//    void ApplyUserShiftsToTieShape();
//
//	//info about related shapes
//	inline void SetBeamShape(GmoShapeBeam* pBeamShape) { m_pBeamShape = pBeamShape; }
//	inline GmoShapeBeam* GetBeamShape() const { return m_pBeamShape; }
//	inline void SetStemShape(GmoShapeStem* pStemShape) { m_pStemShape = pStemShape; }
//    inline void OnTieAttached(int nTie, GmoShapeTie* pShapeTie) { m_pTieShape[nTie] = pShapeTie; }

	//access to constituent shapes
	//GmoShape* get_notehead();
	LUnits get_notehead_width();
//	inline GmoShapeStem* GetStem() const { return m_pStemShape; }
//
//	//access to info
//	inline LUnits GetXEnd() const { return m_uxLeft + m_uWidth; }
//	LUnits GetStemThickness();
//	bool StemGoesDown();
//
//	//layout related
//	void SetStemLength(LUnits uLength);
//	void DrawLegerLines(int nPosOnStaff, LUnits uxLine, lmPaper* pPaper, Color color);
//
//	//dragging
//    wxBitmap* OnBeginDrag(double rScale, wxDC* pDC);
//	UPoint OnDrag(lmPaper* pPaper, const UPoint& uPos);
//	void OnEndDrag(lmPaper* pPaper, lmController* pCanvas, const UPoint& uPos);
//
//
//
//protected:
//    //position
//    LUnits		m_uxLeft;
//    LUnits		m_uyTop;
//
//	LUnits		m_uWidth;
//
//	//related shapes
//	GmoShapeBeam*	m_pBeamShape;
//    GmoShapeTie*     m_pTieShape[2];     //The two archs of a tie. This is the end note of the tie
//
//	//info to render leger lines
//	int				m_nPosOnStaff;		//line/space on staff on which this note is placed
//	LUnits		m_uyStaffTopLine;	//y pos. of top staff line (5th line)
//
//    //temporary data to be used during dragging
//    int             m_nOldSteps;		//to clear leger lines while dragging
//    LUnits        m_uxOldPos;

};

//---------------------------------------------------------------------------------------
class GmoShapeNotehead : public GmoShapeGlyph
{
public:
    GmoShapeNotehead(int idx, unsigned int iGlyph, UPoint pos, Color color,
                     LibraryScope& libraryScope)
        : GmoShapeGlyph(GmoObj::k_shape_notehead, idx, iGlyph,
                        pos, color, libraryScope)
    {
    }

    ~GmoShapeNotehead() {}

};

//---------------------------------------------------------------------------------------
class GmoShapeFlag : public GmoShapeGlyph
{
public:
    GmoShapeFlag(int idx, unsigned int iGlyph, UPoint pos, Color color,
                     LibraryScope& libraryScope)
        : GmoShapeGlyph(GmoObj::k_shape_flag, idx, iGlyph,
                        pos, color, libraryScope)
    {
    }

    ~GmoShapeFlag() {}

};

//---------------------------------------------------------------------------------------
class GmoShapeDot : public GmoShapeGlyph
{
public:
    GmoShapeDot(int idx, unsigned int iGlyph, UPoint pos, Color color,
                     LibraryScope& libraryScope)
        : GmoShapeGlyph(GmoObj::k_shape_dot, idx, iGlyph,
                        pos, color, libraryScope)
    {
    }

    ~GmoShapeDot() {}

};

////global functions defined in this module
//
//class lmVStaff;
//
//extern void lmDrawLegerLines(int nPosOnStaff, LUnits uxLine, lmVStaff* pVStaff, int nStaff,
//                             LUnits uLineLength, LUnits uyStaffTopLine, lmPaper* pPaper,
//                             Color color);
//


}   //namespace lomse

#endif    // __LOMSE_SHAPE_NOTE_H__


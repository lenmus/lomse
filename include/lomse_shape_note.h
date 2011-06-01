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
class GmoShapeAccidentals;
class GmoShapeFlag;
class FontStorage;
class GmoShapeBeam;


//---------------------------------------------------------------------------------------
class GmoShapeNote : public GmoCompositeShape
{
protected:
    FontStorage* m_pFontStorage;
    LibraryScope& m_libraryScope;
    GmoShapeNotehead* m_pNoteheadShape;
	GmoShapeStem* m_pStemShape;
    GmoShapeAccidentals* m_pAccidentalsShape;
    GmoShapeFlag* m_pFlagShape;
    LUnits m_uAnchorOffset;
    bool m_fUpOriented;     //explicit info. about note orientation: up (stem up) or down

    //for leger lines
    int m_nPosOnStaff;
    LUnits m_uyStaffTopLine;
    LUnits m_uLineOutgoing;
    LUnits m_uLineThickness;
    LUnits m_lineSpacing;

    //required by the beam engraver
    int m_posOnStaff;

public:
    GmoShapeNote(ImoObj* pCreatorImo, LUnits x, LUnits y, Color color,
                 LibraryScope& libraryScope);
    ~GmoShapeNote();


	//overrides
    void on_draw(Drawer* pDrawer, RenderOptions& opt);
    LUnits get_anchor_offset() { return m_uAnchorOffset; }

	//specific methods
	void add_stem(GmoShapeStem* pShape);
	void add_notehead(GmoShapeNotehead* pShape);
	void add_flag(GmoShapeFlag* pShape);
	void add_accidentals(GmoShapeAccidentals* pShape);
	void add_note_in_block(GmoShape* pShape);
    void add_leger_lines_info(int posOnStaff, LUnits yStaffTopLine, LUnits lineLength,
                              LUnits lineThickness, LUnits lineSpacing);
    inline void set_anchor_offset(LUnits offset) { m_uAnchorOffset = offset; }

	//access to constituent shapes
    inline GmoShapeNotehead* get_notehead_shape() const { return m_pNoteheadShape; }
	inline GmoShapeStem* get_stem_shape() const { return m_pStemShape; }
    inline GmoShapeAccidentals* get_accidentals_shape() const { return m_pAccidentalsShape; }
    LUnits get_notehead_width() const;
	LUnits get_notehead_left() const;
	LUnits get_notehead_right() const;
	LUnits get_notehead_height() const;
	LUnits get_notehead_top() const;
	LUnits get_notehead_bottom() const;
    LUnits get_stem_height() const;
    LUnits get_stem_width() const;
    LUnits get_stem_left() const;
    LUnits get_stem_y_flag() const;
    LUnits get_stem_y_note() const;
    LUnits get_stem_extra_length() const;

    //re-shaping
    void set_stem_down(bool down);
    void set_stem_length(LUnits length);

    //required by beam engraver
    inline void set_pos_on_staff(int posOnStaff) { m_posOnStaff = posOnStaff; }
    inline int get_pos_on_staff() { return m_posOnStaff; }

    //info
    inline bool is_up() { return m_fUpOriented; }
    inline void set_up_oriented(bool value) { m_fUpOriented = value; }

protected:
    void draw_leger_lines(Drawer* pDrawer);

};

//---------------------------------------------------------------------------------------
class GmoShapeNotehead : public GmoShapeGlyph
{
public:
    GmoShapeNotehead(ImoObj* pCreatorImo, int idx, unsigned int iGlyph, UPoint pos,
                     Color color, LibraryScope& libraryScope, double fontSize)
        : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_notehead, idx, iGlyph,
                        pos, color, libraryScope, fontSize)
    {
    }

    ~GmoShapeNotehead() {}

};

//---------------------------------------------------------------------------------------
class GmoShapeFlag : public GmoShapeGlyph
{
public:
    GmoShapeFlag(ImoObj* pCreatorImo, int idx, unsigned int iGlyph, UPoint pos,
                 Color color, LibraryScope& libraryScope, double fontSize)
        : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_flag, idx, iGlyph,
                        pos, color, libraryScope, fontSize)
    {
    }

    ~GmoShapeFlag() {}

};

//---------------------------------------------------------------------------------------
class GmoShapeDot : public GmoShapeGlyph
{
public:
    GmoShapeDot(ImoObj* pCreatorImo, int idx, unsigned int iGlyph, UPoint pos,
                Color color, LibraryScope& libraryScope, double fontSize)
        : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_dot, idx, iGlyph,
                        pos, color, libraryScope, fontSize)
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


//---------------------------------------------------------------------------------------
class GmoShapeRest : public GmoCompositeShape
{
protected:
    LibraryScope& m_libraryScope;
	GmoShapeBeam* m_pBeamShape;

public:
    GmoShapeRest(ImoObj* pCreatorImo, int idx, LUnits x, LUnits y, Color color,
                 LibraryScope& libraryScope);
	virtual ~GmoShapeRest();

	//overrides of virtual methods in base class
    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt);
};

//---------------------------------------------------------------------------------------
class GmoShapeRestGlyph : public GmoShapeGlyph
{
public:
    GmoShapeRestGlyph(ImoObj* pCreatorImo, int idx, unsigned int iGlyph, UPoint pos,
                      Color color, LibraryScope& libraryScope, double fontSize)
        : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_rest_glyph, idx, iGlyph,
                        pos, color, libraryScope, fontSize)
    {
    }

    ~GmoShapeRestGlyph() {}

};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_NOTE_H__


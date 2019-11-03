//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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
class GmoShapeNote : public GmoCompositeShape, public VoiceRelatedShape
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
    int m_nPosOnStaff;      //required by the beam engraver and ledger lines

    //for leger lines
    LUnits m_uyStaffTopLine;
    LUnits m_uLineOutgoing;
    LUnits m_uLineThickness;
    LUnits m_lineSpacing;

 public:    //TO_FIX: constructor used in tests
    //friend class NoteEngraver;
    GmoShapeNote(ImoObj* pCreatorImo, LUnits x, LUnits y, Color color,
                 LibraryScope& libraryScope);

public:
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
    inline GmoShapeFlag* get_flag_shape() const { return m_pFlagShape; }
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
    void increment_stem_length(LUnits yIncrement);

    //required by beam engraver
    inline int get_pos_on_staff() { return m_nPosOnStaff; }

    //info
    inline bool is_up() { return m_fUpOriented; }
    inline void set_up_oriented(bool value) { m_fUpOriented = value; }

    //info from parent ImoNote
    bool has_beam();
    bool is_in_chord();

    //used for debug
    void set_color(Color color);

protected:
    void draw_leger_lines(Drawer* pDrawer);

};

//---------------------------------------------------------------------------------------
class GmoShapeChordBaseNote : public GmoShapeNote
{
protected:
    GmoShapeNote* m_pFlagNote;  //note containing the fixed segment for the stem

public:
    GmoShapeChordBaseNote(ImoObj* pCreatorImo, LUnits x, LUnits y, Color color,
                          LibraryScope& libraryScope)
        : GmoShapeNote(pCreatorImo, x, y, color, libraryScope)
        , m_pFlagNote(nullptr)
    {
        m_objtype = GmoObj::k_shape_chord_base_note;
    }

    inline GmoShapeNote* get_flag_note() { return m_pFlagNote; }

protected:
    friend class StemFlagEngraver;
    inline void set_flag_note(GmoShapeNote* pNote) { m_pFlagNote = pNote; }

};

//---------------------------------------------------------------------------------------
class GmoShapeNotehead : public GmoShapeGlyph, public VoiceRelatedShape
{
//protected:
//    friend class NoteEngraver;
public:     //TO_FIX: Constructor used in tests
    GmoShapeNotehead(ImoObj* pCreatorImo, ShapeId idx, unsigned int iGlyph, UPoint pos,
                     Color color, LibraryScope& libraryScope, double fontSize)
        : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_notehead, idx, iGlyph,
                        pos, color, libraryScope, fontSize)
        , VoiceRelatedShape()
    {
    }
};

//---------------------------------------------------------------------------------------
class GmoShapeFlag : public GmoShapeGlyph, public VoiceRelatedShape
{
protected:
    friend class StemFlagEngraver;
    GmoShapeFlag(ImoObj* pCreatorImo, ShapeId idx, unsigned int iGlyph, UPoint pos,
                 Color color, LibraryScope& libraryScope, double fontSize)
        : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_flag, idx, iGlyph,
                        pos, color, libraryScope, fontSize)
        , VoiceRelatedShape()
{
    }
};

//---------------------------------------------------------------------------------------
class GmoShapeDot : public GmoShapeGlyph, public VoiceRelatedShape
{
protected:
    friend class NoteEngraver;
    friend class RestEngraver;
    GmoShapeDot(ImoObj* pCreatorImo, ShapeId idx, unsigned int iGlyph, UPoint pos,
                Color color, LibraryScope& libraryScope, double fontSize)
        : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_dot, idx, iGlyph,
                        pos, color, libraryScope, fontSize)
        , VoiceRelatedShape()
    {
    }
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
class GmoShapeRest : public GmoCompositeShape, public VoiceRelatedShape
{
protected:
    LibraryScope& m_libraryScope;
	GmoShapeBeam* m_pBeamShape;

public:     //TO_FIX: Constructor used in tests
//    friend class RestEngraver;
    GmoShapeRest(ImoObj* pCreatorImo, ShapeId idx, LUnits x, LUnits y, Color color,
                 LibraryScope& libraryScope);

public:
    void on_draw(Drawer* pDrawer, RenderOptions& opt);
};

//---------------------------------------------------------------------------------------
class GmoShapeRestGlyph : public GmoShapeGlyph, public VoiceRelatedShape
{
protected:
    friend class RestEngraver;
    GmoShapeRestGlyph(ImoObj* pCreatorImo, ShapeId idx, unsigned int iGlyph, UPoint pos,
                      Color color, LibraryScope& libraryScope, double fontSize)
        : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_rest_glyph, idx, iGlyph,
                        pos, color, libraryScope, fontSize)
        , VoiceRelatedShape()
    {
    }
};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_NOTE_H__


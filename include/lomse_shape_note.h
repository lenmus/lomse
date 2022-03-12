//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
class GmoShapeChordBaseNote;


//---------------------------------------------------------------------------------------
class GmoShapeNote : public GmoCompositeShape, public VoiceRelatedShape
{
protected:
    GmoShapeNotehead* m_pNoteheadShape;
	GmoShapeStem* m_pStemShape;
    GmoShapeAccidentals* m_pAccidentalsShape;
    GmoShapeFlag* m_pFlagShape;
    LUnits m_uAnchorOffset;
    bool m_fUpOriented;     //explicit info. about note orientation: up (stem up) or down
    int m_nPosOnStaff;      //required by the beam engraver and ledger lines
    int m_nTopPosOnStaff;       //pos on staff for 1st ledger line above
    int m_nBottomPosOnStaff;    //pos on staff for 1st ledger line below

    //for leger lines
    LUnits m_uyStaffTopLine;
    LUnits m_uLineOutgoing;
    LUnits m_uLineThickness;
    LUnits m_lineSpacing;

    //for notes in chord
    enum {
        k_chord_note_no = 0,
        k_chord_note_flag,
        k_chord_note_link,
        k_chord_note_start,
    };

    int m_chordNoteType;                        //chord note type, from enum
    GmoShapeChordBaseNote* m_pBaseNoteShape;    //ptr to base note shape



 public:    //TO_FIX: constructor used in tests
    //friend class NoteEngraver;
    GmoShapeNote(ImoObj* pCreatorImo, LUnits x, LUnits y, Color color,
                 LibraryScope& libraryScope);

public:
    ~GmoShapeNote();


	//overrides
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;
    LUnits get_anchor_offset() override { return m_uAnchorOffset; }

	//specific methods
	void add_stem(GmoShapeStem* pShape);
	void add_notehead(GmoShapeNotehead* pShape);
	void add_flag(GmoShapeFlag* pShape);
	void add_accidentals(GmoShapeAccidentals* pShape);
	void add_note_in_block(GmoShape* pShape);
    void add_leger_lines_info(int posOnStaff, int topPosOnStaff, int bottomPosOnStaff,
                              LUnits yStaffTopLine, LUnits lineLength,
                              LUnits lineThickness, LUnits lineSpacing);
    inline void set_anchor_offset(LUnits offset) { m_uAnchorOffset = offset; }
    inline void increment_anchor_offset(LUnits incr) { m_uAnchorOffset += incr; }

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
    LUnits get_stem_right() const;
    LUnits get_stem_y_flag() const;
    LUnits get_stem_y_note() const;

    //re-shaping
    void set_stem_down(bool down);
    void set_stem_length(LUnits length);
    void increment_stem_length(LUnits yIncrement);

    //required by beam engraver
    inline int get_pos_on_staff() { return m_nPosOnStaff; }

    //info
    inline bool is_up() { return m_fUpOriented; }
    inline void set_up_oriented(bool value) { m_fUpOriented = value; }
    inline bool is_chord_flag_note() { return m_chordNoteType == k_chord_note_flag; }
    inline bool is_chord_link_note() { return m_chordNoteType == k_chord_note_link; }
    inline bool is_chord_start_note() { return m_chordNoteType == k_chord_note_start; }
    inline bool is_chord_note() { return m_chordNoteType != k_chord_note_no; }
    inline bool has_stem() { return m_pStemShape != nullptr; }

    //info from parent ImoNote
    bool has_beam();
    bool is_in_chord();
    bool is_cross_staff_chord();

    //for chords
    inline GmoShapeChordBaseNote* get_base_note_shape() { return m_pBaseNoteShape; }
    inline void set_base_note_shape(GmoShapeChordBaseNote* pShape) { m_pBaseNoteShape = pShape; }

    //used for debug
    void set_notehead_color(Color color);
    void dump(ostream& outStream, int level) override;


protected:
    void draw_leger_lines(Drawer* pDrawer);

    //for chords
    friend class GmoShapeChordBaseNote;
    inline void set_chord_note_type(int type) { m_chordNoteType = type; }

};

//---------------------------------------------------------------------------------------
class GmoShapeChordBaseNote : public GmoShapeNote
{
protected:
    GmoShapeNote* m_pFlagNote;  //note containing the fixed segment for the stem
    GmoShapeNote* m_pLinkNote;  //note containing the link segment for the stem
    GmoShapeNote* m_pStartNote; //note containing the extensible segment for the stem

    GmoShapeArpeggio* m_pArpeggio; //arpeggio, if a chord has any

public:
    GmoShapeChordBaseNote(ImoObj* pCreatorImo, LUnits x, LUnits y, Color color,
                          LibraryScope& libraryScope)
        : GmoShapeNote(pCreatorImo, x, y, color, libraryScope)
        , m_pFlagNote(nullptr)
        , m_pLinkNote(nullptr)
        , m_pStartNote(nullptr)
        , m_pArpeggio(nullptr)
    {
        m_objtype = GmoObj::k_shape_chord_base_note;
    }

    inline GmoShapeNote* get_flag_note() { return m_pFlagNote; }
    inline GmoShapeNote* get_link_note() { return m_pLinkNote; }
    inline GmoShapeNote* get_start_note() { return m_pStartNote; }
    GmoShapeNote* get_top_note();
    GmoShapeNote* get_bottom_note();

    GmoShapeArpeggio* get_arpeggio() { return m_pArpeggio; }

protected:
    friend class ChordEngraver;
    void set_flag_note(GmoShapeNote* pNote);
    void set_link_note(GmoShapeNote* pNote);
    void set_start_note(GmoShapeNote* pNote);
    void set_arpeggio(GmoShapeArpeggio* pArp) { m_pArpeggio = pArp; }

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
	GmoShapeBeam* m_pBeamShape = nullptr;

	//required for fixing overlaps
    int m_nPosOnStaff = 0;
    LUnits m_uAnchorOffset = 0.0f;

public:     //TO_FIX: Constructor used in tests
//    friend class RestEngraver;
    GmoShapeRest(ImoObj* pCreatorImo, ShapeId idx, LUnits x, LUnits y, Color color,
                 LibraryScope& libraryScope);

public:
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //required for fixing overlaps
    inline void set_pos_on_staff(int pos) { m_nPosOnStaff = pos; }
    inline int get_pos_on_staff() { return m_nPosOnStaff; }
    LUnits get_anchor_offset() override { return m_uAnchorOffset; }
    inline void increment_anchor_offset(LUnits incr) { m_uAnchorOffset += incr; }
    inline void set_anchor_offset(LUnits offset) { m_uAnchorOffset = offset; }
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


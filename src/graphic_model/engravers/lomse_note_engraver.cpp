//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_note_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_gm_basic.h"
#include "lomse_engraving_options.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"
#include "lomse_font_storage.h"
#include "lomse_shapes.h"
#include "lomse_pitch.h"
#include "lomse_score_meter.h"
#include "lomse_engravers_map.h"
#include "lomse_accidentals_engraver.h"
#include "lomse_chord_engraver.h"
#include "lomse_internal_model.h"

#include <sstream>
using namespace std;

namespace lomse
{

//=======================================================================================
// NoteEngraver implementation
//=======================================================================================
NoteEngraver::NoteEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           EngraversMap* pEngravers, int iInstr, int iStaff)
    : Engraver(libraryScope, pScoreMeter, iInstr, iStaff)
    , m_pNote(nullptr)
    , m_clefType(k_clef_undefined)
    , m_octaveShift(0)
    , m_pEngravers(pEngravers)
    , m_fStemDown(false)
    , m_nPosOnStaff(0)
    , m_uyStaffTopLine(0.0f)
    , m_uxLeft(0.0f)
    , m_uyTop(0.0f)
    , m_pNoteShape(nullptr)
    , m_pNoteheadShape(nullptr)
    , m_pAccidentalsShape(nullptr)
    , m_nDots(0)
    , m_noteType(k_quarter)
    , m_acc(k_no_accidentals)
    , m_lineSpacing(0.0f)
    , m_fontSize(0.0)
{
    m_idxStaff = pScoreMeter->staff_index(iInstr, iStaff);
}

//---------------------------------------------------------------------------------------
GmoShape* NoteEngraver::create_shape(ImoNote* pNote, int clefType, int octaveShift,
                                     UPoint uPos, Color color)
{
    //save data and initialize
    m_pNote = pNote;
    m_clefType = clefType;
    m_octaveShift = octaveShift;
    m_lineSpacing = m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff);
    m_color = color;
    m_pNoteShape = nullptr;
    m_pNoteheadShape = nullptr;
    m_fontSize = determine_font_size();

    //compute y pos for note pitch
    m_uyStaffTopLine = uPos.y;
    m_nPosOnStaff = get_pos_on_staff();
    m_uyTop = m_uyStaffTopLine - get_pitch_shift();
    m_uxLeft = uPos.x;

    //get note required data
    m_nDots = m_pNote->get_dots();
    m_noteType = m_pNote->get_note_type();
    m_acc = m_pNote->get_notated_accidentals();
    determine_stem_direction();

    create_shape();

    return m_pNoteShape;
}

//---------------------------------------------------------------------------------------
GmoShape* NoteEngraver::create_tool_dragged_shape(int noteType, EAccidentals acc,
                                                  int dots)
{
    //initialize
    m_pNote = nullptr;
    m_clefType = k_clef_G2;
    m_lineSpacing = 72.0;
    m_color = Color(255,0,0);       //TODO: options/configuration
    m_pNoteShape = nullptr;
    m_pNoteheadShape = nullptr;
    m_fontSize = 21.0;

    //y pos for note pitch
    m_uyStaffTopLine = 0.0;
    m_nPosOnStaff = 2;          //2 = on first line
    m_uyTop = 0.0;
    m_uxLeft = 0.0;

    //set note required data
    m_nDots = dots;
    m_noteType = noteType;
    m_acc = acc;
    m_fStemDown = false;

    create_shape();

    return m_pNoteShape;
}

//---------------------------------------------------------------------------------------
void NoteEngraver::create_shape()
{
	//create the note container shape
	if (m_pNote->is_start_of_chord())
        m_pNoteShape = LOMSE_NEW GmoShapeChordBaseNote(m_pNote, m_uxLeft, m_uyTop,
                                                       m_color, m_libraryScope);
    else
        m_pNoteShape = LOMSE_NEW GmoShapeNote(m_pNote, m_uxLeft, m_uyTop, m_color,
                                              m_libraryScope);

    add_voice(m_pNoteShape);

    //create component shapes: accidentals, notehead, dots, stem, flag, ledger lines
    add_shapes_for_accidentals_if_required();
    add_notehead_shape();
    add_shapes_for_dots_if_required();
    add_stem_and_flag_if_required();
    add_leger_lines_if_necessary();
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_voice(VoiceRelatedShape* pVRS)
{
    if (m_pNote)
        pVRS->set_voice(m_pNote->get_voice());
}

//---------------------------------------------------------------------------------------
UPoint NoteEngraver::get_drag_offset()
{
    //return center of notehead
    URect total = m_pNoteShape->get_bounds();
    URect head = m_pNoteheadShape->get_bounds();
    return UPoint(head.get_x() - total.get_x() + head.get_width() / 2.0f,
                  head.get_y() - total.get_y() + head.get_height() / 2.0f );
}

//---------------------------------------------------------------------------------------
void NoteEngraver::determine_stem_direction()
{
	switch (m_pNote->get_stem_direction())
	{
        case k_stem_default:
            m_fStemDown = (m_nPosOnStaff >= 6);
            break;
        case k_stem_double:
//            TODO: NoteEngraver stem_double
//            I understand that "stem double" means two stems: one up and one down.
//            This is not yet implemented and is treated as stem default.
            m_fStemDown = (m_nPosOnStaff >= 6);
            break;
        case k_stem_up:
            m_fStemDown = false;
            break;
        case k_stem_down:
            m_fStemDown = true;
            break;
        case k_stem_none:
            m_fStemDown = false;       //false or true. The value doesn't matter.
            break;
    }
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_shapes_for_dots_if_required()
{
    if (m_nDots > 0)
    {
        LUnits uSpaceBeforeDot = tenths_to_logical(LOMSE_SPACE_BEFORE_DOT);
        LUnits uyPos = m_uyTop;
        if (m_nPosOnStaff % 2 == 0)
        {
            // notehead is placed on a line. Shift up/down the dots by half line
            if (m_fStemDown)
                uyPos -= tenths_to_logical(5.0f);
            else
                uyPos += tenths_to_logical(5.0f);
        }

        for (int i = 0; i < m_nDots; i++)
        {
            m_uxLeft += uSpaceBeforeDot;
            m_uxLeft += add_dot_shape(m_uxLeft, uyPos, m_color);
        }
    }
}

//---------------------------------------------------------------------------------------
LUnits NoteEngraver::add_dot_shape(LUnits x, LUnits y, Color color)
{
    y += tenths_to_logical(get_glyph_offset(k_glyph_dot));
    GmoShapeDot* pShape = LOMSE_NEW GmoShapeDot(m_pNote, 0, k_glyph_dot, UPoint(x, y),
                                          color, m_libraryScope, m_fontSize);
    add_voice(pShape);
	m_pNoteShape->add(pShape);
    return pShape->get_width();
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_stem_and_flag_if_required()
{
    //When the note is in a chord the stem is added by the chord engraver

    if (has_stem() && !is_in_chord())
    {
        bool fHasFlag = (!is_beamed() && has_flag());
        Tenths length = get_standard_stem_length(m_nPosOnStaff, m_fStemDown);
        if (fHasFlag && length < 35.0f && m_noteType > k_eighth)
            length = 35.0f;     // 3.5 spaces
        LUnits stemLength = tenths_to_logical(length);
        bool fShortFlag = (length < 35.0f);
        StemFlagEngraver engrv(m_libraryScope, m_pMeter, m_pNote, m_iInstr, m_iStaff);
        engrv.add_stem_flag_to_note(m_pNoteShape, m_noteType, m_fStemDown, fHasFlag,
                                    fShortFlag, stemLength, m_color);
        m_pNoteShape->set_up_oriented(!m_fStemDown);
    }
    else
        m_pNoteShape->set_up_oriented(true);
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_shapes_for_accidentals_if_required()
{
    if (m_acc != k_no_accidentals)
    {
        AccidentalsEngraver engrv(m_libraryScope, m_pMeter, m_iInstr, m_iStaff);
        m_pAccidentalsShape = engrv.create_shape(m_pNote, UPoint(m_uxLeft, m_uyTop),
                                                 m_acc, false /*cautionary accidentals*/,
                                                 m_color);
        m_pNoteShape->add_accidentals(m_pAccidentalsShape);
        m_uxLeft += m_pAccidentalsShape->get_width();
        m_uxLeft += tenths_to_logical(LOMSE_SPACE_AFTER_ACCIDENTALS);
    }
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_notehead_shape()
{
    int notehead = decide_notehead_type();
    int iGlyph = get_glyph_for_notehead(notehead);
    LUnits y = m_uyTop + tenths_to_logical(get_glyph_offset(iGlyph));
    m_pNoteheadShape = LOMSE_NEW GmoShapeNotehead(m_pNote, 0, iGlyph, UPoint(m_uxLeft, y),
                                            m_color, m_libraryScope, m_fontSize);
    add_voice(m_pNoteheadShape);
    m_pNoteShape->add_notehead(m_pNoteheadShape);
    m_pNoteShape->set_anchor_offset(m_pNoteShape->get_left() - m_uxLeft);
    m_uxLeft += m_pNoteheadShape->get_width();
    m_pNoteShape->set_up_oriented(!m_fStemDown);
}

//---------------------------------------------------------------------------------------
int NoteEngraver::decide_notehead_type()
{
    //TODO: Notehead cross

    //if (! m_fCabezaX)
    {
        if (m_noteType > k_half) {
            return k_notehead_quarter;
        } else if (m_noteType == k_half) {
            return k_notehead_half;
        } else if (m_noteType == k_whole) {
            return k_notehead_whole;
        } else if (m_noteType == k_breve) {
            return k_notehead_breve;
        } else if (m_noteType == k_longa) {
            return k_notehead_longa;
        } else {
            //LogMessage("NoteEngraver::decide_notehead_type", "Unknown note type.");
            return k_notehead_quarter;
        }
    }
//    else
//        return k_notehead_cross;
}

//---------------------------------------------------------------------------------------
int NoteEngraver::get_glyph_for_notehead(int noteheadType)
{
    switch (noteheadType)
    {
        case k_notehead_longa:
            return k_glyph_longa_note;
        case k_notehead_breve:
            return k_glyph_breve_note;
        case k_notehead_whole:
            return k_glyph_whole_note;
        case k_notehead_half:
            return k_glyph_notehead_half;
        case k_notehead_quarter:
            return k_glyph_notehead_quarter;
        case k_notehead_cross:
            return k_glyph_notehead_cross;
        default:
            //LogMessage("NoteEngraver::get_glyph_for_notehead]", "Invalid value for notehead type");
            return k_glyph_notehead_quarter;
    }
}

//---------------------------------------------------------------------------------------
int NoteEngraver::get_pos_on_staff()
{
    // Returns the position on the staff (line/space) referred to the first ledger
    // line of the staff. Depends on clef:
    //        0 - on first ledger line (C note in G clef)
    //        1 - on next space (D in G clef)
    //        2 - on first line (E not in G clef)
    //        3 - on first space
    //        4 - on second line
    //        5 - on second space
    //        etc.

    if (!m_pNote->is_pitch_defined())
        return 0;   //first bottom ledger line
    else
        return pitch_to_pos_on_staff(m_clefType);
}

//---------------------------------------------------------------------------------------
int NoteEngraver::pitch_to_pos_on_staff(int clefType)
{
    // Returns the position on the staff (line/space) referred to the first ledger line of
    // the staff. Depends on clef:
    //        0 - on first ledger line (C note in G clef)
    //        1 - on next space (D in G clef)
    //        2 - on first line (E not in G clef)
    //        3 - on first space
    //        4 - on second line
    //        5 - on second space
    //        etc.

    DiatonicPitch dpitch(m_pNote->get_step(), m_pNote->get_octave());
    dpitch += m_octaveShift;

	// pitch is defined. Position will depend on key
    switch (clefType)
    {
        case k_clef_undefined:
        case k_clef_G2:
            return dpitch - C4_DPITCH;
        case k_clef_F4:
            return dpitch - C4_DPITCH + 12;
        case k_clef_F3:
            return dpitch - C4_DPITCH + 10;
        case k_clef_C1:
            return dpitch - C4_DPITCH + 2;
        case k_clef_C2:
            return dpitch - C4_DPITCH + 4;
        case k_clef_C3:
            return dpitch - C4_DPITCH + 6;
        case k_clef_C4:
            return dpitch - C4_DPITCH + 8;
        case k_clef_percussion:
            return 5;       //on 2nd space
        case k_clef_C5:
            return dpitch - C4_DPITCH + 10;
        case k_clef_F5:
            return dpitch - C4_DPITCH + 14;
        case k_clef_G1:
            return dpitch - C4_DPITCH - 2;
        case k_clef_8_G2:        //8 above
        case k_clef_G2_8:        //8 below
        case k_clef_8_F4:        //8 above
        case k_clef_F4_8:        //8 below
        case k_clef_15_G2:       //15 above
        case k_clef_G2_15:       //15 below
        case k_clef_15_F4:       //15 above
        case k_clef_F4_15:       //15 below
            //TODO: NoteEngraver::pitch_to_pos_on_staff. clefs with 8ve
            return 2;
        default:
            //LogMessage("NoteEngraver::pitch_to_pos_on_staff", "Case %d not treated in switch statement", nClef);
            return dpitch - C4_DPITCH;     //assume G clef
    }
}

//---------------------------------------------------------------------------------------
LUnits NoteEngraver::get_pitch_shift()
{
    LUnits uShift = tenths_to_logical(Tenths(m_nPosOnStaff * 5));
    if (m_nPosOnStaff > 0 && m_nPosOnStaff < 12)
    {
        //on staff lines/spaces
        return uShift;
    }
    else
    {
        //leger lines required
        if (m_nPosOnStaff > 11)
        {
            //leger lines at top
            Tenths dsplz = m_pMeter->get_upper_ledger_lines_displacement();
            return uShift + tenths_to_logical(dsplz);
        }
        else
        {
            //leger lines at bottom
            return uShift;
        }
    }
}

//---------------------------------------------------------------------------------------
Tenths NoteEngraver::get_standard_stem_length(int nPosOnStaff, bool fStemDown)
{
    // Returns the stem length that this note should have, according to engraving
    // rules. It takes into account the position of the note on the staff.
    //
    // a1 - Normal length is one octave (3.5 spaces), but only for notes between the spaces
    //      previous to first ledger lines (b3 and b5, in G key, both inclusive).
    //
    // a2 - Notes with stems upwards from c5 inclusive, or with stems downwards from
    //      g4 inclusive have a length of 2.5 spaces.
    //
    // a3 - If a note is on or above the second ledger line above the staff, or
    //      on or below the second ledger line below the staff: the end of stem
    //      have to touch the middle staff line.


    Tenths length;

    // rule a3
    if (nPosOnStaff >= 14 && fStemDown)
    {
        length = Tenths(5 * (nPosOnStaff-6));     // touch middle line
    }
    else if (nPosOnStaff <= -2 && !fStemDown)
    {
        length = Tenths(5 *(6-nPosOnStaff));     // touch middle line
    }

    // rule a2
    else if ((nPosOnStaff >= 7 && !fStemDown) || (nPosOnStaff <= 4 && fStemDown))
    {
        length = 25.0f;     // 2.5 spaces
    }

    // rule a1 and any other case not covered (I did not analyze completeness)
    else
    {
        length = 35.0f;     // 3.5 spaces
    }

    return length;
}

//---------------------------------------------------------------------------------------
Tenths NoteEngraver::get_glyph_offset(int iGlyph)
{
    //AWARE: notehead registration is as follows:
    // * Vertically centered on the baseline.
    // * Noteheads should be positioned as if on the bottom line of the staff.
    // * The leftmost point coincides with x = 0.

    return m_libraryScope.get_glyphs_table()->glyph_offset(iGlyph) + 50.0f;
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_leger_lines_if_necessary()
{
    LUnits lineOutgoing = tenths_to_logical(LOMSE_LEGER_LINE_OUTGOING);
    LUnits lineThickness = tenths_to_logical(LOMSE_STEM_THICKNESS);
    LUnits lineSpacing = tenths_to_logical(10.0f);

    //leger lines at top
    Tenths dsplz = 0.0f;
    if (m_nPosOnStaff > 11)
        dsplz = m_pMeter->get_upper_ledger_lines_displacement();

    //AWARE: yStart is relative to notehead top
    LUnits yStart =  m_uyStaffTopLine - m_pNoteShape->get_notehead_top()
                     - tenths_to_logical(dsplz);

    m_pNoteShape->add_leger_lines_info(m_nPosOnStaff, yStart, lineOutgoing,
                                       lineThickness, lineSpacing);
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_to_chord_if_in_chord()
{
    if (is_in_chord())
    {
        ImoChord* pChord = m_pNote->get_chord();
        ChordEngraver* pEngrv
            = dynamic_cast<ChordEngraver*>(m_pEngravers->get_engraver(pChord));

        if (!pEngrv)
            create_chord();
        else if (pEngrv->notes_missing() == 1)
            layout_chord();
        else
            add_to_chord();
    }
}

//---------------------------------------------------------------------------------------
void NoteEngraver::create_chord()
{
    ImoChord* pChord = m_pNote->get_chord();
    int numNotes = pChord->get_num_objects();
    ChordEngraver* pEngrv =
        LOMSE_NEW ChordEngraver(m_libraryScope, m_pMeter, numNotes);
    m_pEngravers->save_engraver(pEngrv, pChord);

    pEngrv->set_start_staffobj(pChord, m_pNote, m_pNoteShape, m_iInstr, m_iStaff,
                               0, 0, 0.0f, 0.0f, 0.0f, m_idxStaff, nullptr);
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_to_chord()
{
    ImoChord* pChord = m_pNote->get_chord();
    ChordEngraver* pEngrv
        = static_cast<ChordEngraver*>(m_pEngravers->get_engraver(pChord));

    pEngrv->set_middle_staffobj(pChord, m_pNote, m_pNoteShape, 0, 0, 0, 0,
                                0.0f, 0.0f, 0.0f, m_idxStaff, nullptr);
}

//---------------------------------------------------------------------------------------
void NoteEngraver::layout_chord()
{
    ImoChord* pChord = m_pNote->get_chord();
    ChordEngraver* pEngrv
        = static_cast<ChordEngraver*>(m_pEngravers->get_engraver(pChord));
    pEngrv->set_end_staffobj(pChord, m_pNote, m_pNoteShape, 0, 0, 0, 0,
                             0.0f, 0.0f, 0.0f, m_idxStaff, nullptr);

    pEngrv->create_shapes(pChord->get_color());

    m_pEngravers->remove_engraver(pChord);
    delete pEngrv;
}



//=======================================================================================
// StemFlagEngraver implementation
//=======================================================================================
StemFlagEngraver::StemFlagEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                                   ImoObj* pCreatorImo, int iInstr, int iStaff)
    : Engraver(libraryScope, pScoreMeter)
    , m_iInstr(iInstr)
    , m_iStaff(iStaff)
    , m_noteType(0)
    , m_fStemDown(false)
    , m_fWithFlag(false)
    , m_fShortFlag(false)
    , m_fCrossStaffChord(false)
    , m_uStemLength(0.0f)
    , m_fontSize(0.0)
    , m_pCreatorImo(pCreatorImo)
    , m_pFlagNoteShape(nullptr)
    , m_pRefNoteShape(nullptr)
    , m_pBaseNoteShape(nullptr)

    , m_uStemThickness(0.0f)
    , m_uxStem(0.0f)
    , m_yStemTop(0.0f)
    , m_yStemFlag(0.0f)
    , m_yStemBottom(0.0f)
    , m_pRefNoteheadShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::add_stem_flag_to_note(GmoShapeNote* pNoteShape, int noteType,
                                     bool fStemDown, bool fWithFlag, bool fShortFlag,
                                     LUnits stemLength, Color color)
{
    m_pRefNoteShape = pNoteShape;
    m_pFlagNoteShape = pNoteShape;
    m_pBaseNoteShape = pNoteShape;
    m_pRefNoteheadShape = pNoteShape->get_notehead_shape();
    m_noteType = noteType;
    m_fStemDown = fStemDown;
    m_fWithFlag = fWithFlag;
    m_fShortFlag = fShortFlag;
    m_fCrossStaffChord = false;
    m_uStemLength = stemLength;
    m_color = color;
    m_fontSize = determine_font_size();

    add_stem_and_flag();
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::add_stem_flag_to_chord(GmoShapeNote* pMinNoteShape,
                                     GmoShapeNote* pMaxNoteShape,
                                     GmoShapeNote* pBaseNoteShape, int noteType,
                                     bool fStemDown, bool fWithFlag, bool fShortFlag,
                                     bool fCrossStaffChord, LUnits stemLength, Color color)
{
    m_pRefNoteShape = (fStemDown ? pMaxNoteShape : pMinNoteShape);
    m_pFlagNoteShape = (fStemDown ? pMinNoteShape : pMaxNoteShape);
    m_pBaseNoteShape = pBaseNoteShape;
    m_pRefNoteheadShape = m_pRefNoteShape->get_notehead_shape();
    m_noteType = noteType;
    m_fStemDown = fStemDown;
    m_fWithFlag = fWithFlag;
    m_fShortFlag = fShortFlag;
    m_fCrossStaffChord = fCrossStaffChord;
    m_uStemLength = stemLength;
    m_color = color;
    m_fontSize = determine_font_size();

    add_stem_and_flag();
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::add_stem_and_flag()
{
    determine_stem_x_left();
    determine_stem_y_pos();
    add_stem_shape();
    add_flag_shape_if_required();
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::add_stem_shape()
{
    if (m_fCrossStaffChord)
    {
        //For chords across two or more staves it is required to split the stem into
        //two segments, one for the flag (the 'fixed segment' or 'flag segment') and
        //anoter segment joining all noteheads (the 'extensible segment').
        //The flag segment will be attached to the lowest pitch note when stem down or
        //to the highest pith note when stem up.
        //Base note will store a ptr. to the note shape receiving the flag segment.

        //flag segment. Fixed
        LUnits yTop = (m_fStemDown ? m_yStemFlag : m_yStemTop);
        LUnits yBottom = (m_fStemDown ? m_yStemBottom : m_yStemFlag);
        GmoShapeStem* pShape = LOMSE_NEW GmoShapeStem(m_pCreatorImo, m_uxStem, yTop,
                                                      0.0f, yBottom, m_fStemDown,
                                                      m_uStemThickness, m_color);
        add_voice(pShape);
        m_pFlagNoteShape->add_stem(pShape);

        //extensible segment.
        if (m_pRefNoteShape != m_pFlagNoteShape)
        {
            LUnits yTop = (m_fStemDown ? m_yStemTop : m_yStemFlag);
            LUnits yBottom = (m_fStemDown ? m_yStemFlag : m_yStemBottom);
            GmoShapeStem* pShape = LOMSE_NEW GmoShapeStem(m_pCreatorImo, m_uxStem, yTop,
                                                          0.0f, yBottom, m_fStemDown,
                                                          m_uStemThickness, m_color);
            add_voice(pShape);
            m_pRefNoteShape->add_stem(pShape);
        }

        //set pointer to flag segment in chord base note shape
        static_cast<GmoShapeChordBaseNote*>(m_pBaseNoteShape)->set_flag_note(m_pFlagNoteShape);
    }
    else
    {
        //Just the fixed segment
        GmoShapeStem* pShape = LOMSE_NEW GmoShapeStem(m_pCreatorImo, m_uxStem, m_yStemTop,
                                                      0.0f, m_yStemBottom, m_fStemDown,
                                                      m_uStemThickness, m_color);
        add_voice(pShape);
        m_pBaseNoteShape->add_stem(pShape);

        //set pointer to flag segment in chord base note shape
        if (m_pBaseNoteShape->is_shape_chord_base_note())
            static_cast<GmoShapeChordBaseNote*>(m_pBaseNoteShape)->set_flag_note(m_pBaseNoteShape);
    }

}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::add_flag_shape_if_required()
{
    //AWARE: Flags registration is as follows:
    // * y=0 corresponds to the end of a stem of normal length
    // * x=0 corresponds to the left-hand side of the stem.

    if (m_fWithFlag)
    {
        int iGlyph = get_glyph_for_flag();
        LUnits x = m_uxStem;
        LUnits y = (m_fStemDown ? m_yStemBottom + get_glyph_offset(iGlyph)
                                : m_yStemTop - get_glyph_offset(iGlyph));

        GmoShapeFlag* pShape = LOMSE_NEW GmoShapeFlag(m_pCreatorImo, 0, iGlyph,
                                                      UPoint(x, y), m_color,
                                                      m_libraryScope, m_fontSize);
        add_voice(pShape);
        m_pFlagNoteShape->add_flag(pShape);
    }
}
//---------------------------------------------------------------------------------------
void StemFlagEngraver::determine_stem_x_left()
{
    m_uStemThickness = tenths_to_logical(LOMSE_STEM_THICKNESS);

    if (m_fStemDown)
		m_uxStem = m_pRefNoteheadShape->get_left();
    else
		m_uxStem = m_pRefNoteheadShape->get_right() - m_uStemThickness;
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::determine_stem_y_pos()
{
    GmoShape* pTopNotehead = (m_fStemDown ? m_pRefNoteShape : m_pFlagNoteShape)->get_notehead_shape();
    GmoShape* pBottomNotehead = (m_fStemDown ? m_pFlagNoteShape : m_pRefNoteShape)->get_notehead_shape();
    LUnits halfNotehead = pTopNotehead->get_height() / 2.0f;

    //top of fixed/extensible when up/down, respectively
    m_yStemTop = pTopNotehead->get_top() + halfNotehead;

    //bottom of extensible/fixed when up/down, respectively
    m_yStemBottom = pBottomNotehead->get_top() + halfNotehead;

    //re-arrange from top to bottom
    if (m_fStemDown)
    {
        m_yStemFlag = m_yStemBottom;
        m_yStemBottom += m_uStemLength;
    }
    else
    {
        m_yStemFlag = m_yStemTop;
        m_yStemTop -= m_uStemLength;
    }
}

//---------------------------------------------------------------------------------------
int StemFlagEngraver::get_glyph_for_flag()
{
    switch (m_noteType)
	{
        case k_eighth :
        {
            if (m_fShortFlag)
                return (m_fStemDown ? k_glyph_internal_flag_down : k_glyph_internal_flag_up);
            else
                return (m_fStemDown ? k_glyph_eighth_flag_down : k_glyph_eighth_flag_up);
        }
        case k_16th :
            return  (m_fStemDown ? k_glyph_16th_flag_down : k_glyph_16th_flag_up);
        case k_32nd :
            return  (m_fStemDown ? k_glyph_32nd_flag_down : k_glyph_32nd_flag_up);
        case k_64th :
            return  (m_fStemDown ? k_glyph_64th_flag_down : k_glyph_64th_flag_up);
        case k_128th :
            return  (m_fStemDown ? k_glyph_128th_flag_down : k_glyph_128th_flag_up);
        case k_256th :
            return  (m_fStemDown ? k_glyph_256th_flag_down : k_glyph_256th_flag_up);
        default:
        {
            stringstream msg;
            msg << "Error: invalid note type " << m_noteType;
            LOMSE_LOG_ERROR(msg.str());
            return  k_glyph_eighth_flag_down;
        }
    }
}

//---------------------------------------------------------------------------------------
LUnits StemFlagEngraver::get_glyph_offset(int iGlyph)
{
    Tenths tenths = m_libraryScope.get_glyphs_table()->glyph_offset(iGlyph);
    return m_pMeter->tenths_to_logical(tenths, m_iInstr, m_iStaff);
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::add_voice(VoiceRelatedShape* pVRS)
{
    if (m_pRefNoteShape)
    {
        VoiceRelatedShape* pNote = static_cast<VoiceRelatedShape*>(m_pRefNoteShape);
        pVRS->set_voice(pNote->get_voice());
    }
}



}  //namespace lomse

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
#include "lomse_shapes_storage.h"
#include "lomse_accidentals_engraver.h"
#include "lomse_chord_engraver.h"
#include "lomse_internal_model.h"

using namespace std;

namespace lomse
{

//=======================================================================================
// NoteEngraver implementation
//=======================================================================================
NoteEngraver::NoteEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           ShapesStorage* pShapesStorage)
    : NoterestEngraver(libraryScope, pScoreMeter, pShapesStorage)
    , m_pNote(NULL)
{
}

//---------------------------------------------------------------------------------------
GmoShape* NoteEngraver::create_shape(ImoNote* pNote, int iInstr, int iStaff,
                                     int clefType, UPoint uPos)
{
    //save data and initialize
    m_pNote = pNote;
    m_pNoteRest = pNote;
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_clefType = clefType;
    m_lineSpacing = m_pMeter->line_spacing_for_instr_staff(iInstr, iStaff);
    m_color = m_pNote->get_color();
    m_pNoteShape = NULL;
    m_pNoteheadShape = NULL;
    m_fontSize = determine_font_size();

    //compute y pos for note pitch
    m_uyStaffTopLine = uPos.y;
    m_nPosOnStaff = get_pos_on_staff();
    m_uyTop = m_uyStaffTopLine - get_pitch_shift();
    m_uxLeft = uPos.x;

	//create the note container shape
    m_pNoteShape = LOMSE_NEW GmoShapeNote(pNote, m_uxLeft, m_uyTop, m_color, m_libraryScope);
    m_pNoteShape->set_pos_on_staff(m_nPosOnStaff);
    m_pNoteRestShape = m_pNoteShape;

    //create component shapes: accidentals, notehead, dots, stem, flag, ledger lines
    determine_stem_direction();
    add_shapes_for_accidentals_if_required();
    add_notehead_shape();
    add_shapes_for_dots_if_required();
    add_stem_and_flag_if_required();
    add_leger_lines_if_necessary();

    return m_pNoteShape;
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
//            TODO
//            I understand that "STEM_double" means two stems: one up and one down.
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
    int nDots = m_pNote->get_dots();
    if (nDots > 0)
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

        for (int i = 0; i < nDots; i++)
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
	m_pNoteShape->add(pShape);
    return pShape->get_width();
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_stem_and_flag_if_required()
{
    //When the note is in a chord the stem is added by the chord engraver

    if (has_stem() && !is_in_chord())
    {
        int noteType = m_pNote->get_note_type();
        bool fHasFlag = (!is_beamed() && has_flag());
        LUnits stemLength = tenths_to_logical(
                                get_standard_stem_length(m_nPosOnStaff, m_fStemDown) );
        StemFlagEngraver engrv(m_libraryScope, m_pMeter, m_pNote, m_iInstr, m_iStaff);
        engrv.add_stem_flag(m_pNoteShape, m_pNoteShape, noteType, m_fStemDown,
                            fHasFlag, stemLength, m_color);
        m_pNoteShape->set_up_oriented(!m_fStemDown);
    }
    else
        m_pNoteShape->set_up_oriented(true);
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_shapes_for_accidentals_if_required()
{
    EAccidentals acc = m_pNote->get_notated_accidentals();
    if (acc != k_no_accidentals)
    {
        AccidentalsEngraver engrv(m_libraryScope, m_pMeter);
        m_pAccidentalsShape = engrv.create_shape(m_pNote, m_iInstr, m_iStaff,
                                    UPoint(m_uxLeft, m_uyTop), acc, false);
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
    m_pNoteShape->add_notehead(m_pNoteheadShape);
    m_pNoteShape->set_anchor_offset(m_pNoteShape->get_left() - m_uxLeft);
    m_uxLeft += m_pNoteheadShape->get_width();
    m_pNoteShape->set_up_oriented(!m_fStemDown);
}

//---------------------------------------------------------------------------------------
int NoteEngraver::decide_notehead_type()
{
    //TODO: Notehead cross

    int noteType = m_pNote->get_note_type();
    //if (! m_fCabezaX)
    {
        if (noteType > k_half) {
            return k_notehead_quarter;
        } else if (noteType == k_half) {
            return k_notehead_half;
        } else if (noteType == k_whole) {
            return k_notehead_whole;
        } else if (noteType == k_breve) {
            return k_notehead_breve;
        } else if (noteType == k_longa) {
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
            //TODO
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
            //TODO
//            lmTenths nDsplz = (lmTenths) m_pVStaff->GetOptionLong(_T("Staff.UpperLegerLines.Displacement"));
//            lmLUnits uyDsplz = tenths_to_logical(nDsplz);
            return uShift ; //+ uyDsplz;
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
    // rules. It takes into account the `posiotion of the note on the staff.
    //
    // a1 - Normal length is one octave (3.5 spaces), but only for notes between the spaces
    //      previous to first leger lines (b3 and b5, in Sol key, both inclusive).
    //
    // a2 - Notes with stems upwards from c5 inclusive, or with stems downwards from
    //      g4 inclusive have a legth of 2.5 spaces.
    //
    // a3 - If a note is on or above the second leger line above the staff, or
    //      on or below the second leger line below the staff: the end of stem
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

    // rule a1 and any other case not covered (I did not analyze completness)
    else
    {
        length = 35.0f;     // 3.5 spaces
    }

    return length;
}

//---------------------------------------------------------------------------------------
Tenths NoteEngraver::get_glyph_offset(int iGlyph)
{
    return glyphs_lmbasic2[iGlyph].GlyphOffset;
}

//---------------------------------------------------------------------------------------
LUnits NoteEngraver::tenths_to_logical(Tenths tenths)
{
    return m_pMeter->tenths_to_logical(tenths, m_iInstr, m_iStaff);
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_leger_lines_if_necessary()
{
    LUnits lineOutgoing = tenths_to_logical(LOMSE_LEGER_LINE_OUTGOING);
    LUnits lineThickness = tenths_to_logical(LOMSE_STEM_THICKNESS);
    LUnits lineSpacing = tenths_to_logical(10.0f);

    //TODO
    //ImoOptionInfo* pOpt = get_score(0)->get_option("Staff.UpperLegerLines.Displacement");
    //Tenths dsplz = Tenths( pOpt->get_long_value() );

    //AWARE: yStart is relative to notehead top
    LUnits yStart =  m_uyStaffTopLine - m_pNoteShape->get_notehead_top();   // - tenths_to_logical(dsplz);

    m_pNoteShape->add_leger_lines_info(m_nPosOnStaff, yStart, lineOutgoing,
                                       lineThickness, lineSpacing);
}

//---------------------------------------------------------------------------------------
double NoteEngraver::determine_font_size()
{
    //TODO
    return 21.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_to_chord_if_in_chord()
{
    if (is_in_chord())
    {
        ImoChord* pChord = m_pNote->get_chord();
        ChordEngraver* pEngrv
            = dynamic_cast<ChordEngraver*>(m_pShapesStorage->get_engraver(pChord));

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
    ChordEngraver* pEngrv = LOMSE_NEW ChordEngraver(m_libraryScope, m_pMeter, numNotes);
    m_pShapesStorage->save_engraver(pEngrv, pChord);

    pEngrv->set_start_staffobj(pChord, m_pNote, m_pNoteShape, m_iInstr, m_iStaff,
                               0, 0, UPoint(0.0f, 0.0f));
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_to_chord()
{
    ImoChord* pChord = m_pNote->get_chord();
    ChordEngraver* pEngrv
        = dynamic_cast<ChordEngraver*>(m_pShapesStorage->get_engraver(pChord));

    //pEngrv->add_note(m_pNote, m_pNoteShape, m_nPosOnStaff, m_iInstr);
    pEngrv->set_middle_staffobj(pChord, m_pNote, m_pNoteShape, 0, 0, 0, 0);
}

//---------------------------------------------------------------------------------------
void NoteEngraver::layout_chord()
{
    ImoChord* pChord = m_pNote->get_chord();
    ChordEngraver* pEngrv
        = dynamic_cast<ChordEngraver*>(m_pShapesStorage->get_engraver(pChord));
    //pEngrv->layout_chord();
//    pEngrv->set_end_staffobj(pAO, pSO, pStaffObjShape, iInstr, iStaff, iSystem, iCol);
    pEngrv->set_end_staffobj(pChord, m_pNote, m_pNoteShape, 0, 0, 0, 0);
//    pEngrv->set_prolog_width( prologWidth );
//
    pEngrv->create_shapes();

    m_pShapesStorage->remove_engraver(pChord);
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
    , m_pCreatorImo(pCreatorImo)
{
}
//---------------------------------------------------------------------------------------
void StemFlagEngraver::add_stem_flag(GmoShapeNote* pNoteShape,
                                     GmoShapeNote* pBaseNoteShape, int noteType,
                                     bool fStemDown, bool fWithFlag,
                                     LUnits stemLength, Color color)
{
    m_pNoteShape = pNoteShape;
    m_pNoteheadShape = pNoteShape->get_notehead_shape();
    m_pBaseNoteShape = pBaseNoteShape;
    m_noteType = noteType;
    m_fStemDown = fStemDown;
    m_fWithFlag = fWithFlag;
    m_uStemLength = stemLength;
    m_color = color;
    m_fontSize = determine_font_size();

    determine_position_and_size_of_stem();
    add_stem_shape();
    add_flag_shape_if_required();
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::add_stem_shape()
{
    m_uyStemFlag = m_uyStemNote + (m_fStemDown ? m_uStemLength : -m_uStemLength);
    LUnits yTop = (m_fStemDown ? m_uyStemNote : m_uyStemFlag);
    LUnits yBottom = (m_fStemDown ? m_uyStemFlag : m_uyStemNote);
    m_pBaseNoteShape->add_stem(
        LOMSE_NEW GmoShapeStem(m_pCreatorImo, m_uxStem, yTop, 0.0f, yBottom, m_fStemDown,
                         m_uStemThickness, m_color) );
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::add_flag_shape_if_required()
{
    if (m_fWithFlag)
    {
        int iGlyph = get_glyph_for_flag();
        LUnits x = (m_fStemDown ? m_uxStem : m_uxStem + m_uStemThickness);
        LUnits y = m_uyStemFlag + get_glyph_offset(iGlyph);
        m_pBaseNoteShape->add_flag( new GmoShapeFlag(m_pCreatorImo, 0, iGlyph,
                                                     UPoint(x, y), m_color,
                                                     m_libraryScope, m_fontSize) );
    }
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::determine_position_and_size_of_stem()
{
    determine_stem_x_left();
    determine_stem_y_note();
    if (m_fWithFlag)
        cut_stem_size_to_add_flag();
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::cut_stem_size_to_add_flag()
{
    int iGlyph = get_glyph_for_flag();
    // Glyph data is in FUnits. As 512 FU are 1 line (10 tenths), to convert
    // FUnits into tenths just divide FU by 51.2
    Tenths rFlag, rMinStem;
    if (m_fStemDown)
    {
        rFlag = fabs((2048.0f - glyphs_lmbasic2[iGlyph].Bottom) / 51.2f );
        rMinStem = (glyphs_lmbasic2[iGlyph].Top - 2048.0f + 128.0f) / 51.2f ;
    }
    else
    {
        if (m_noteType == k_eighth)
            rFlag = (glyphs_lmbasic2[iGlyph].Top) / 51.2f ;
        else if (m_noteType == k_16th)
            rFlag = (glyphs_lmbasic2[iGlyph].Top + 128.0f) / 51.2f;
        else
            rFlag = (glyphs_lmbasic2[iGlyph].Top + 512.0f) / 51.2f;

        rMinStem = fabs(glyphs_lmbasic2[iGlyph].Bottom / 51.2f);
    }
    LUnits uFlag = tenths_to_logical(rFlag);
    LUnits uMinStem = tenths_to_logical(rMinStem);

    m_uStemLength = max((m_uStemLength > uFlag ? m_uStemLength-uFlag : 0), uMinStem);
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::determine_stem_x_left()
{
    m_uStemThickness = tenths_to_logical(LOMSE_STEM_THICKNESS);

    if (m_fStemDown)
		m_uxStem = m_pNoteheadShape->get_left();
    else
		m_uxStem = m_pNoteheadShape->get_right() - m_uStemThickness;
}

//---------------------------------------------------------------------------------------
void StemFlagEngraver::determine_stem_y_note()
{
    m_uyStemNote = m_pNoteheadShape->get_top();
    if (m_fStemDown)
        m_uyStemNote += tenths_to_logical(6.1f);    //on the left of the notehead
    else
        m_uyStemNote += tenths_to_logical(3.9f);    //on the right of the notehead
}

//---------------------------------------------------------------------------------------
int StemFlagEngraver::get_glyph_for_flag()
{
    switch (m_noteType)
	{
        case k_eighth :
            return (m_fStemDown ? k_glyph_eighth_flag_down : k_glyph_eighth_flag_up);
        case k_16th :
            return  (m_fStemDown ? k_glyph_16th_flag_down : k_glyph_16th_flag_up);
        case k_32th :
            return  (m_fStemDown ? k_glyph_32nd_flag_down : k_glyph_32nd_flag_up);
        case k_64th :
            return  (m_fStemDown ? k_glyph_64th_flag_down : k_glyph_64th_flag_up);
        case k_128th :
            return  (m_fStemDown ? k_glyph_128th_flag_down : k_glyph_128th_flag_up);
        case k_256th :
            return  (m_fStemDown ? k_glyph_256th_flag_down : k_glyph_256th_flag_up);
        default:
            //LogMessage("StemFlagEngraver::get_glyph_for_flag", "Error: invalid note type %d.", m_pNote->get_note_type());
            return  k_glyph_eighth_flag_down;
    }
}

//---------------------------------------------------------------------------------------
LUnits StemFlagEngraver::get_glyph_offset(int iGlyph)
{
    Tenths tenths = glyphs_lmbasic2[iGlyph].GlyphOffset;
    return m_pMeter->tenths_to_logical(tenths, m_iInstr, m_iStaff);
}

//---------------------------------------------------------------------------------------
LUnits StemFlagEngraver::tenths_to_logical(Tenths value)
{
    return m_pMeter->tenths_to_logical(value, m_iInstr, m_iStaff);
}

//---------------------------------------------------------------------------------------
double StemFlagEngraver::determine_font_size()
{
    //TODO
    return 21.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;
}



}  //namespace lomse

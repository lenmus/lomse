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

#include "lomse_note_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"
#include "lomse_box_slice_instr.h"
#include "lomse_font_storage.h"
#include "lomse_shapes.h"
#include "lomse_pitch.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// NoteEngraver implementation
//---------------------------------------------------------------------------------------
NoteEngraver::NoteEngraver(LibraryScope& libraryScope)
    : m_libraryScope(libraryScope)
    , m_pNote(NULL)
{
}

//---------------------------------------------------------------------------------------
GmoShape* NoteEngraver::create_shape(ImoNote* pNote, int clefType, UPoint uPos,
                                     LUnits lineSpacing)
                                     //GmoBoxSliceInstr* pBox,
{
    m_pNote = pNote;
    m_clefType = clefType;
    m_lineSpacing = lineSpacing;
    m_color = m_pNote->get_color();
    m_pNoteShape = NULL;
    m_pStemShape = NULL;
    m_pNoteheadShape = NULL;


    LUnits uyStaffTopLine = uPos.y;
    m_nPosOnStaff = get_pos_on_staff();
    m_uyTop = uyStaffTopLine - get_pitch_shift();
    m_uxLeft = uPos.x;

    determine_stem_direction();

//	// create the shape for the stem because it must be measured even if it must not be
//	// drawn. This is necessary, for example, to have information for positioning
//	// tuplets' bracket. Or to align notes in a chord.
//	// If the shape is finally not needed (i.e. the note doesn't have stem)
//	// the shape will be deleted. Otherwise, it will be added to the note shape.
//    //-----------------------------------------------------------------------------------
    m_uStemThickness = tenths_to_logical(LOMSE_STEM_THICKNESS);
	bool fStemAdded = false;
	m_pStemShape = new GmoShapeStem(0.0f, 0.0f, 0.0f, 0.0f,
                                    m_fStemDown, m_uStemThickness, m_color);


//    //if this is the first note of a chord, give lmChordLayout the responsibility for
//	//layouting the chord (only note heads, accidentals and rests). If it is
//    //any other note of a chord, do nothing and mark the note as 'already layouted'.
    m_fNoteEngraved = false;
//    if (IsBaseOfChord())
//	{
//        m_pChord->LayoutNoteHeads(pBox, pPaper, uPos, color);
//        m_fNoteEngraved = true;
//    }
//    else if (is_in_chord())
//	{
//        m_fNoteEngraved = true;
//    }
//	else
//	{
//		//note is not in chord. create the container shape
//		CreateContainerShape(pBox, m_uxLeft, m_uyTop, color);
//	}
//	GmoShapeNote* m_pNoteShape = (GmoShapeNote*)GetShape();
    m_pNoteShape = new GmoShapeNote(m_uxLeft, m_uyTop, m_color, m_libraryScope);

//
//    //if this is the first note/rest of a beam, create the beam shape
//    //AWARE This must be done before using stem information, as the beam could
//    //change stem direction if it is not determined for some/all the notes in the beam
//    if (is_beamed() && (NoteEngraver*)m_pBeam->GetStartNoteRest() == this )
//    {
//        if (m_pBeam->NeedsSetUp())
//            m_pBeam->AutoSetUp();
//        m_pBeam->CreateShape();
//    }
//
//
    add_shapes_for_accidentals_if_required();
    add_space_before_note();
    bool fDrawStem = add_notehead_shape();
    add_shapes_for_dots_if_required();
    set_position_and_size_of_stem();

//
//        //if not in a chord add the shape for the stem. When the note is in a chord
//		//the stem will be created later, when layouting the last note of the chord
        if (fDrawStem && ! is_in_chord())
		{
//			// if beamed, the stem shape will be owned by the beam; otherwise by the note
//			if (is_beamed())
//				m_pBeam->AddNoteAndStem(m_pStemShape, m_pNoteShape, &m_BeamInfo);
//			else
				m_pNoteShape->add_stem(m_pStemShape);

            fStemAdded = true;
        }
//    }
//
//    //if this is the last note of a chord draw the stem of the chord
//    //-----------------------------------------------------------------------------
//    if (is_in_chord() && IsLastOfChord() && has_stem())
//    {
//        m_pChord->AddStemShape(pPaper, color, GetSuitableFont(pPaper), m_pVStaff,
//							   m_nStaffNum);
//        //AWARE: m_pShapeStem created in all the notes that form the chord at start
//        //of layout method will be deleted in previous method lmChordLayout::AddStemShape
//    }
//
//
    add_shape_for_flag_if_required();
//
//    // add shapes for leger lines if necessary
//    //--------------------------------------------
//	m_pNoteShape->AddLegerLinesInfo(m_nPosOnStaff, uyStaffTopLine);
//
//
//	//if this is the last note of a multi-attached AuxObj add the shape for the aux obj
//    //----------------------------------------------------------------------------------
//
//    // beam lines
//    if (is_in_chord() && IsLastOfChord())
//	{
//		NoteEngraver* pBaseNote = m_pChord->GetBaseNote();
//		if (pBaseNote->IsBeamed() && pBaseNote->GetBeamType(0) == ImoBeam::k_end)
//			pBaseNote->GetBeam()->LayoutObject(pBox, pPaper, UPoint(), color);
//    }
//    else if (!is_in_chord() && is_beamed() && m_BeamInfo.get_beam_type(0) == ImoBeam::k_end)
//	{
//        m_pBeam->LayoutObject(pBox, pPaper, UPoint(), color);
//    }
//
//    // tuplet bracket
//    if (m_pTuplet && m_pTuplet->GetEndNoteRest() == this)
//		m_pTuplet->LayoutObject(pBox, pPaper, color);

	//if not used, delete stem shape created at start of this method.
    //For chords, stem will be deleted when invoking lmChordLayout::AddStemShape in last note
    //of the chord, so we cannot delete the stem here.
	if (!fStemAdded && (!is_in_chord() || !has_stem()))
        delete m_pStemShape;        //DeleteStemShape();

//    add_note_afterspace();
//
//	//for chords it must return the maximum width
//    #define NOTE_AFTERSPACE     0      //TODO user options
//    LUnits uAfterSpace = m_pVStaff->TenthsToLogical(NOTE_AFTERSPACE, m_nStaffNum);
//
//	if (is_in_chord())
//		return m_pChord->GetXRight() - uPos.x + uAfterSpace;
//	else
//		return GetShape()->GetXRight() - uPos.x + uAfterSpace;
//
    return m_pNoteShape;
}

//---------------------------------------------------------------------------------------
void NoteEngraver::determine_stem_direction()
{
	switch (m_pNote->get_stem_direction())
	{
        case ImoNote::k_default:
            m_fStemDown = (m_nPosOnStaff >= 6);
            break;
        case ImoNote::k_double:
//            TODO
//            I understand that "STEM_DOUBLE" means two stems: one up and one down.
//            This is not yet implemented and is treated as stem default.
            m_fStemDown = (m_nPosOnStaff >= 6);
            break;
        case ImoNote::k_up:
            m_fStemDown = false;
            break;
        case ImoNote::k_down:
            m_fStemDown = true;
            break;
        case ImoNote::k_none:
            m_fStemDown = false;       //false or true. The value doesn't matter.
            break;
    }
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_space_before_note()
{
    //TODO
    //m_uxLeft += m_uSpacePrev;
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
    GmoShapeDot* pShape = new GmoShapeDot(0, k_glyph_dot, UPoint(x, y), color,
                                          m_libraryScope);
	m_pNoteShape->add(pShape);
    return pShape->get_width();
}

//---------------------------------------------------------------------------------------
void NoteEngraver::determine_stem_x_pos()
{
    //TODO: stem position for reversed noteheads

    if (m_fStemDown)
    {
        //stem down: line down on the left of the notehead unless notehead reversed
        //line is centered on stem x position, so we must add half stem thickness
		m_uxStem = m_pNoteheadShape->get_left() + m_uStemThickness / 2.0f;
//		if (m_fNoteheadReversed)
//            m_uxStem += m_pNoteheadShape->GetWidth();
    }
    else
    {
        //stem up: line up on the right of the notehead unless notehead reversed
        //line is centered on stem x position, so we must substract half stem thickness
		m_uxStem = m_pNoteheadShape->get_right() - m_uStemThickness / 2.0f;
//        if (m_fNoteheadReversed)
//            m_uxStem -= m_pNoteheadShape->GetWidth();
    }
}

//---------------------------------------------------------------------------------------
void NoteEngraver::determine_stem_y_start()
{
    if (m_fStemDown)
        m_uyTop += tenths_to_logical(51.0f);    //on the left of the notehead
    else
        m_uyTop += tenths_to_logical(49.0f);    //on the right of the notehead

    m_uyStemStart = m_uyTop;
}

//---------------------------------------------------------------------------------------
void NoteEngraver::set_position_and_size_of_stem()
{
    m_uyFlag = 0.0f;                //y pos for flag
    determine_stem_x_pos();
    determine_stem_y_start();

    if (!is_beamed() && has_stem())
    {
        m_uStemLength = get_standard_stem_length();
        if (has_flag())
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
                if (m_pNote->get_note_type() == ImoNoteRest::k_eighth)
                    rFlag = (glyphs_lmbasic2[iGlyph].Top) / 51.2f ;
                else if (m_pNote->get_note_type() == ImoNoteRest::k_16th)
                    rFlag = (glyphs_lmbasic2[iGlyph].Top + 128.0f) / 51.2f;
                else
                    rFlag = (glyphs_lmbasic2[iGlyph].Top + 512.0f) / 51.2f;

                rMinStem = fabs(glyphs_lmbasic2[iGlyph].Bottom / 51.2f);
            }
            LUnits uFlag = tenths_to_logical(rFlag);
            LUnits uMinStem = tenths_to_logical(rMinStem);
            m_uStemLength = max((m_uStemLength > uFlag ? m_uStemLength-uFlag : 0), uMinStem);
            m_uyFlag = m_uyStemStart + (m_fStemDown ? m_uStemLength : -m_uStemLength);
//            SetStemLength(uStemLength + uFlag);
        }
    }

    //adjust the position and size of the stem shape
    m_uyStemEnd = m_uyStemStart + (m_fStemDown ? m_uStemLength : -m_uStemLength);
    m_pStemShape->adjust(m_uxStem, m_uyStemStart, m_uyStemEnd, m_fStemDown);
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_shape_for_flag_if_required()
{
    if (!is_beamed() && has_flag() && !is_in_chord())
    {
        LUnits x = (m_fStemDown ? get_stem_x_left() : get_stem_x_right());
        add_flag_shape(UPoint(x, m_uyFlag), m_color);
    }
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_shapes_for_accidentals_if_required()
{
    //add shapes for accidental signs, if they exist and note not in chord

    int acc = m_pNote->get_accidentals();
    if (acc != ImoNote::k_no_accidentals)
    {
        if (!m_fNoteEngraved)
        {
//            m_pAccidentals->Layout(m_uxLeft, m_uyTop);
//			m_pNoteShape->add_accidental(m_pAccidentals->GetShape());
        }
//        m_uxLeft += m_pAccidentals->get_width();
    }
}

//---------------------------------------------------------------------------------------
bool NoteEngraver::add_notehead_shape()
{
    bool fDrawStem = false;
    if (!m_fNoteEngraved)
	{
        int notehead = decide_notehead_type();
        int iGlyph = get_glyph_for_notehead(notehead);
        LUnits y = m_uyTop + tenths_to_logical(get_glyph_offset(iGlyph));
        m_pNoteheadShape =
            new GmoShapeNotehead(0, iGlyph, UPoint(m_uxLeft, y), m_color, m_libraryScope);
        m_pNoteShape->add_notehead(m_pNoteheadShape);
        fDrawStem = has_stem();
    }
    m_uxLeft += m_pNoteheadShape->get_width();
    return fDrawStem;
}

//---------------------------------------------------------------------------------------
int NoteEngraver::decide_notehead_type()
{
    //TODO: Notehead cross

    int noteType = m_pNote->get_note_type();
    //if (! m_fCabezaX)
    {
        if (noteType > ImoNoteRest::k_half) {
            return k_notehead_quarter;
        } else if (noteType == ImoNoteRest::k_half) {
            return k_notehead_half;
        } else if (noteType == ImoNoteRest::k_whole) {
            return k_notehead_whole;
        } else if (noteType == ImoNoteRest::k_breve) {
            return k_notehead_breve;
        } else if (noteType == ImoNoteRest::k_longa) {
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
int NoteEngraver::get_glyph_for_flag()
{
    switch (m_pNote->get_note_type())
	{
        case ImoNoteRest::k_eighth :
            return (m_fStemDown ? k_glyph_eighth_flag_down : k_glyph_eighth_flag_up);
        case ImoNoteRest::k_16th :
            return  (m_fStemDown ? k_glyph_16th_flag_down : k_glyph_16th_flag_up);
        case ImoNoteRest::k_32th :
            return  (m_fStemDown ? k_glyph_32nd_flag_down : k_glyph_32nd_flag_up);
        case ImoNoteRest::k_64th :
            return  (m_fStemDown ? k_glyph_64th_flag_down : k_glyph_64th_flag_up);
        case ImoNoteRest::k_128th :
            return  (m_fStemDown ? k_glyph_128th_flag_down : k_glyph_128th_flag_up);
        case ImoNoteRest::k_256th :
            return  (m_fStemDown ? k_glyph_256th_flag_down : k_glyph_256th_flag_up);
        default:
            //LogMessage("NoteEngraver::get_glyph_for_flag", "Error: invalid note type %d.", m_pNote->get_note_type());
            return  k_glyph_eighth_flag_down;
    }
}

//---------------------------------------------------------------------------------------
void NoteEngraver::add_flag_shape(UPoint uPos, Color color)
{
    int iGlyph = get_glyph_for_flag();
    LUnits y = uPos.y + tenths_to_logical(get_glyph_offset(iGlyph));
    GmoShapeFlag* pShape = new GmoShapeFlag(0, iGlyph, UPoint(uPos.x, y),
                                            color, m_libraryScope);
	m_pNoteShape->add_flag(pShape);
}

//---------------------------------------------------------------------------------------
Tenths NoteEngraver::get_glyph_offset(int iGlyph)
{
    return glyphs_lmbasic2[iGlyph].GlyphOffset;
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
        case ImoClef::k_undefined:
        case ImoClef::k_G2:
            return dpitch - LOMSE_C4_DPITCH;
        case ImoClef::k_F4:
            return dpitch - LOMSE_C4_DPITCH + 12;
        case ImoClef::k_F3:
            return dpitch - LOMSE_C4_DPITCH + 10;
        case ImoClef::k_C1:
            return dpitch - LOMSE_C4_DPITCH + 2;
        case ImoClef::k_C2:
            return dpitch - LOMSE_C4_DPITCH + 4;
        case ImoClef::k_C3:
            return dpitch - LOMSE_C4_DPITCH + 6;
        case ImoClef::k_C4:
            return dpitch - LOMSE_C4_DPITCH + 8;
        case ImoClef::k_Percussion:
            return 5;       //on 2nd space
        case ImoClef::k_C5:
            return dpitch - LOMSE_C4_DPITCH + 10;
        case ImoClef::k_F5:
            return dpitch - LOMSE_C4_DPITCH + 14;
        case ImoClef::k_G1:
            return dpitch - LOMSE_C4_DPITCH - 2;
        case ImoClef::k_8_G2:        //8 above
        case ImoClef::k_G2_8:        //8 below
        case ImoClef::k_8_F4:        //8 above
        case ImoClef::k_F4_8:        //8 below
        case ImoClef::k_15_G2:       //15 above
        case ImoClef::k_G2_15:       //15 below
        case ImoClef::k_15_F4:       //15 above
        case ImoClef::k_F4_15:       //15 below
            //TODO
            return 2;
        default:
            //LogMessage("NoteEngraver::pitch_to_pos_on_staff", "Case %d not treated in switch statement", nClef);
            return dpitch - LOMSE_C4_DPITCH;     //assume G clef
    }
}

//---------------------------------------------------------------------------------------
LUnits NoteEngraver::get_stem_x_left()
{
	//if (!m_pStemShape) return 0.0f;
	return m_pStemShape->get_left();
}

//---------------------------------------------------------------------------------------
LUnits NoteEngraver::get_stem_x_right()
{
	//if (!m_pStemShape) return 0.0f;
	return m_pStemShape->get_right();
}

////---------------------------------------------------------------------------------------
//LUnits NoteEngraver::GetXStemCenter()
//{
//	if (!m_pStemShape) return 0.0;
//	return m_pStemShape->GetXCenterStem();
//}
//
//LUnits NoteEngraver::GetYStartStem()
//{
//	//Start of stem is the nearest position to the notehead
//	if (!m_pStemShape) return 0.0;
//
//	return m_pStemShape->GetYStartStem();
//	//return m_uyStem + m_uPaperPos.y;
//}
//
//LUnits NoteEngraver::GetYEndStem()
//{
//	//End of stem is the farthest position from the notehead
//	if (!m_pStemShape) return 0.0;
//
//	return m_pStemShape->GetYEndStem();
//	//return GetYStartStem() + (m_fStemDown ? m_uStemLength : -m_uStemLength);
//}

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
LUnits NoteEngraver::get_standard_stem_length()
{
    // Returns the stem lenght that this note should have, according to engraving
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
    if (m_nPosOnStaff >= 14 && m_fStemDown)
    {
        length = Tenths(5 * (m_nPosOnStaff-6));     // touch middle line
    }
    else if (m_nPosOnStaff <= -2 && !m_fStemDown)
    {
        length = Tenths(5 *(6-m_nPosOnStaff));     // touch middle line
    }

    // rule a2
    else if ((m_nPosOnStaff >= 7 && !m_fStemDown) || (m_nPosOnStaff <= 4 && m_fStemDown))
    {
        length = 25.0f;     // 2.5 spaces
    }

    // rule a1 and any other case not covered (I did not analyze completness)
    else
    {
        length = 35.0f;     // 3.5 spaces
    }

    return tenths_to_logical(length);
}



}  //namespace lomse

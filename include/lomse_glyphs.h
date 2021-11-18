//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#ifndef __LOMSE_GLYPHS_H__        //to avoid nested includes
#define __LOMSE_GLYPHS_H__

#include "lomse_basic.h"

namespace lomse
{

//forward declarations
class LibraryScope;


//---------------------------------------------------------------------------------------
//   Glyphs info: an entry of the Glyphs table
//---------------------------------------------------------------------------------------
struct GlyphData
{
    // all measurements in tenths
    unsigned int GlyphChar;
    std::string GlyphName;

    GlyphData(const unsigned int glyph, std::string name="")
        : GlyphChar(glyph)
        , GlyphName(name)
    {
    }

};

//---------------------------------------------------------------------------------------
//indexes for the table
enum EGlyphIndex
{
    k_glyph_none = -1,            //special value meaning 'No glyph'

    //Repeats (U+E040 - U+E04F)
    k_glyph_dacapo = 0,
    k_glyph_dalsegno,
    k_glyph_coda,
    k_glyph_segno,

    //clefs (U+E050 - U+E07F)
    k_glyph_g_clef,
    k_glyph_f_clef,
    k_glyph_c_clef,
    k_glyph_percussion_clef_block,
    k_glyph_g_clef_ottava_bassa,
    k_glyph_g_clef_ottava_alta,
    k_glyph_g_clef_quindicesima_bassa,
    k_glyph_g_clef_quindicesima_alta,
    k_glyph_f_clef_ottava_bassa,
    k_glyph_f_clef_ottava_alta,
    k_glyph_f_clef_quindicesima_bassa,
    k_glyph_f_clef_quindicesima_alta,
    k_glyph_TAB_clef,

    //time signatures (U+E080 - U+E09F)
    k_glyph_number_0,
    k_glyph_number_1,
    k_glyph_number_2,
    k_glyph_number_3,
    k_glyph_number_4,
    k_glyph_number_5,
    k_glyph_number_6,
    k_glyph_number_7,
    k_glyph_number_8,
    k_glyph_number_9,
    k_glyph_common_time,
    k_glyph_cut_time,

    //Notheads (U+E0A0 - U+E0FF)
    k_glyph_longa_note,           //longa
    k_glyph_breve_note,           //breve, cuadrada
    k_glyph_whole_note,           //whole, redonda
    k_glyph_notehead_half,        //half, blanca
    k_glyph_notehead_quarter,     //quarter, negra
    k_glyph_notehead_cross,       //cross, aspa

    //Individual notes with stem and flag (U+E1D0 - U+E1EF)
    k_glyph_half_note_down,       //half, blanca
    k_glyph_half_note_up,
    k_glyph_quarter_note_down,    //quarter, negra
    k_glyph_quarter_note_up,
    k_glyph_eighth_note_down,     //eighth, corchea
    k_glyph_eighth_note_up,
    k_glyph_16th_note_down,       //16th, semicorchea
    k_glyph_16th_note_up,
    k_glyph_32nd_note_down,       //32nd, fusa
    k_glyph_32nd_note_up,
    k_glyph_64th_note_down,       //64th, semifusa
    k_glyph_64th_note_up,
    k_glyph_128th_note_down,      //128th garrapatea
    k_glyph_128th_note_up,
    k_glyph_256th_note_down,      //256th semigarrapatea
    k_glyph_256th_note_up,
    k_glyph_dot,                  //augmentation dot, for dotted notes

    //Tremolos (U+E220 - U+E23F)
    k_glyph_tremolo_1,     // Combining tremolo 1
    k_glyph_tremolo_2,     // Combining tremolo 2
    k_glyph_tremolo_3,     // Combining tremolo 3
    k_glyph_tremolo_4,     // Combining tremolo 4
    k_glyph_tremolo_5,     // Combining tremolo 5

    //flags for notes (U+E240 - U+E25F)
    k_glyph_eighth_flag_down,     //eighth, corchea
    k_glyph_16th_flag_down,       //16th, semicorchea
    k_glyph_32nd_flag_down,       //32nd, fusa
    k_glyph_64th_flag_down,       //64th, semifusa
    k_glyph_128th_flag_down,      //128th, garrapatea
    k_glyph_256th_flag_down,      //256th, semigarrapatea
    k_glyph_512th_flag_down,
    k_glyph_1024th_flag_down,
    k_glyph_eighth_flag_up,
    k_glyph_16th_flag_up,
    k_glyph_32nd_flag_up,
    k_glyph_64th_flag_up,
    k_glyph_128th_flag_up,
    k_glyph_256th_flag_up,
    k_glyph_512th_flag_up,
    k_glyph_1024th_flag_up,
    k_glyph_internal_flag_up,
    k_glyph_internal_flag_down,

    // Standard accidentals (12-EDO) (U+E260xU+E26F)
    k_glyph_flat_accidental,         // Flat"
    k_glyph_natural_accidental, 	    // Natural"
    k_glyph_sharp_accidental, 	    // Sharp"
    k_glyph_double_sharp_accidental, 	// Double sharp"
    k_glyph_double_flat_accidental, 	// Double flat"
    k_glyph_accidentalTripleSharp, 	// Triple sharp
    k_glyph_accidentalTripleFlat, 	// Triple flat
    k_glyph_accidentalNaturalFlat, 	// Natural flat
    k_glyph_accidentalNaturalSharp, // Natural sharp
    k_glyph_accidentalSharpSharp, 	// Sharp sharp
    k_glyph_open_cautionary_accidental, 	// open_cautionary_accidental,   Accidental parenthesis, left
    k_glyph_close_cautionary_accidental, 	// close_cautionary_accidental   Accidental parenthesis, right
    k_glyph_accidentalBracketLeft, 	// Accidental bracket, left
    k_glyph_accidentalBracketRight, // Accidental bracket, right

    // Gould arrow quartertone accidentals (24-EDO) (U+E270xU+E27F)
    k_glyph_accidentalQuarterToneFlatArrowUp, 	        // Quarter-tone flat"
    k_glyph_accidentalThreeQuarterTonesFlatArrowDown, 	// Three-quarter-tones flat"
    k_glyph_accidentalQuarterToneSharpNaturalArrowUp, 	// Quarter-tone sharp"
    k_glyph_accidentalQuarterToneFlatNaturalArrowDown, 	// Quarter-tone flat"
    k_glyph_accidentalThreeQuarterTonesSharpArrowUp, 	// Three-quarter-tones sharp"
    k_glyph_accidentalQuarterToneSharpArrowDown, 	    // Quarter-tone sharp"
    k_glyph_accidentalFiveQuarterTonesSharpArrowUp, 	// Five-quarter-tones sharp
    k_glyph_accidentalThreeQuarterTonesSharpArrowDown, 	// Three-quarter-tones sharp
    k_glyph_accidentalThreeQuarterTonesFlatArrowUp, 	// Three-quarter-tones flat
    k_glyph_accidentalFiveQuarterTonesFlatArrowDown, 	// Five-quarter-tones flat
    k_glyph_accidentalArrowUp, 	                        // Arrow up (raise by one quarter-tone)
    k_glyph_accidentalArrowDown, 	                    // Arrow down (lower by one quarter-tone)

    // Stein-Zimmermann accidentals (24-EDO) (U+E280xU+E28F)
    k_glyph_accidentalQuarterToneFlatStein, 	        // Reversed flat (quarter-tone flat) (Stein)
    k_glyph_accidentalThreeQuarterTonesFlatZimmermann, 	// Reversed flat and flat (three-quarter-tones flat) (Zimmermann)
    k_glyph_accidentalQuarterToneSharpStein, 	        // Half sharp (quarter-tone sharp) (Stein)
    k_glyph_accidentalThreeQuarterTonesSharpStein, 	    // One and a half sharps (three-quarter-tones sharp) (Stein)
    k_glyph_accidentalNarrowReversedFlat, 	            // Narrow reversed flat(quarter-tone flat)
    k_glyph_accidentalNarrowReversedFlatAndFlat, 	    // Narrow reversed flat and flat(three-quarter-tones flat)

    // Extended Stein-Zimmermann accidentals (U+E290xU+E29F)
    k_glyph_accidentalReversedFlatArrowUp, 	            // Reversed flat with arrow up
    k_glyph_accidentalReversedFlatArrowDown, 	        // Reversed flat with arrow down
    k_glyph_accidentalFilledReversedFlatArrowUp, 	    // Filled reversed flat with arrow up
    k_glyph_accidentalFilledReversedFlatArrowDown,      // Filled reversed flat with arrow down
    k_glyph_accidentalReversedFlatAndFlatArrowUp,       // Reversed flat and flat with arrow up
    k_glyph_accidentalReversedFlatAndFlatArrowDown, 	// Reversed flat and flat with arrow down
    k_glyph_accidentalFilledReversedFlatAndFlat, 	    // Filled reversed flat and flat
    k_glyph_accidentalFilledReversedFlatAndFlatArrowUp, 	// Filled reversed flat and flat with arrow up
    k_glyph_accidentalFilledReversedFlatAndFlatArrowDown, 	// Filled reversed flat and flat with arrow down
    k_glyph_accidentalHalfSharpArrowUp, 	            // Half sharp with arrow up
    k_glyph_accidentalHalfSharpArrowDown, 	            // Half sharp with arrow down
    k_glyph_accidentalOneAndAHalfSharpsArrowUp, 	    // One and a half sharps with arrow up
    k_glyph_accidentalOneAndAHalfSharpsArrowDown, 	    // One and a half sharps with arrow down

    // Sims accidentals (72-EDO) (U+E2A0xU+E2AF)
    k_glyph_accidentalSims12Down, 	// 1/12 tone low
    k_glyph_accidentalSims6Down, 	// 1/6 tone low
    k_glyph_accidentalSims4Down, 	// 1/4 tone low
    k_glyph_accidentalSims12Up, 	// 1/12 tone high
    k_glyph_accidentalSims6Up, 	    // 1/6 tone high
    k_glyph_accidentalSims4Up, 	    // 1/4 tone high

    // Johnston accidentals (just intonation) (U+E2B0xU+E2BF)
    k_glyph_accidentalJohnstonPlus, 	// Plus (raise by 81:80x
    k_glyph_accidentalJohnstonMinus, 	// Minus (lower by 81:80x
    k_glyph_accidentalJohnstonEl, 	    // Inverted seven (raise by 36:35)
    k_glyph_accidentalJohnstonSeven, 	// Seven (lower by 36:35)
    k_glyph_accidentalJohnstonUp, 	    // Up arrow (raise by 33:32)
    k_glyph_accidentalJohnstonDown, 	// Down arrow (lower by 33:32)
    k_glyph_accidentalJohnston13, 	    // Thirteen (raise by 65:64)
    k_glyph_accidentalJohnston31, 	    // Inverted 13 (lower by 65:64)

    // Extended Helmholtz-Ellis accidentals (just intonation) (U+E2C0xU+E2FF)
    k_glyph_accidentalDoubleFlatOneArrowDown, 	// Double flat lowered by one syntonic comma
    k_glyph_accidentalFlatOneArrowDown, 	    // Flat lowered by one syntonic comma
    k_glyph_accidentalNaturalOneArrowDown, 	    // Natural lowered by one syntonic comma
    k_glyph_accidentalSharpOneArrowDown, 	    // Sharp lowered by one syntonic comma
    k_glyph_accidentalDoubleSharpOneArrowDown, 	// Double sharp lowered by one syntonic comma
    k_glyph_accidentalDoubleFlatOneArrowUp, 	// Double flat raised by one syntonic comma
    k_glyph_accidentalFlatOneArrowUp, 	        // Flat raised by one syntonic comma
    k_glyph_accidentalNaturalOneArrowUp, 	    // Natural raised by one syntonic comma
    k_glyph_accidentalSharpOneArrowUp, 	        // Sharp raised by one syntonic comma
    k_glyph_accidentalDoubleSharpOneArrowUp, 	// Double sharp raised by one syntonic comma
    k_glyph_accidentalDoubleFlatTwoArrowsDown, 	// Double flat lowered by two syntonic commas
    k_glyph_accidentalFlatTwoArrowsDown, 	    // Flat lowered by two syntonic commas
    k_glyph_accidentalNaturalTwoArrowsDown, 	// Natural lowered by two syntonic commas
    k_glyph_accidentalSharpTwoArrowsDown, 	    // Sharp lowered by two syntonic commas
    k_glyph_accidentalDoubleSharpTwoArrowsDown, // Double sharp lowered by two syntonic commas
    k_glyph_accidentalDoubleFlatTwoArrowsUp, 	// Double flat raised by two syntonic commas
    k_glyph_accidentalFlatTwoArrowsUp, 	        // Flat raised by two syntonic commas
    k_glyph_accidentalNaturalTwoArrowsUp, 	    // Natural raised by two syntonic commas
    k_glyph_accidentalSharpTwoArrowsUp, 	    // Sharp raised by two syntonic commas
    k_glyph_accidentalDoubleSharpTwoArrowsUp, 	// Double sharp raised by two syntonic commas
    k_glyph_accidentalDoubleFlatThreeArrowsDown, 	// Double flat lowered by three syntonic commas
    k_glyph_accidentalFlatThreeArrowsDown, 	    // Flat lowered by three syntonic commas
    k_glyph_accidentalNaturalThreeArrowsDown, 	// Natural lowered by three syntonic commas
    k_glyph_accidentalSharpThreeArrowsDown, 	// Sharp lowered by three syntonic commas
    k_glyph_accidentalDoubleSharpThreeArrowsDown, 	// Double sharp lowered by three syntonic commas
    k_glyph_accidentalDoubleFlatThreeArrowsUp, 	// Double flat raised by three syntonic commas
    k_glyph_accidentalFlatThreeArrowsUp, 	    // Flat raised by three syntonic commas
    k_glyph_accidentalNaturalThreeArrowsUp, 	// Natural raised by three syntonic commas
    k_glyph_accidentalSharpThreeArrowsUp, 	    // Sharp raised by three syntonic commas
    k_glyph_accidentalDoubleSharpThreeArrowsUp, 	// Double sharp raised by three syntonic commas
    k_glyph_accidentalLowerOneSeptimalComma, 	// Lower by one septimal comma
    k_glyph_accidentalRaiseOneSeptimalComma, 	// Raise by one septimal comma
    k_glyph_accidentalLowerTwoSeptimalCommas, 	// Lower by two septimal commas
    k_glyph_accidentalRaiseTwoSeptimalCommas, 	// Raise by two septimal commas
    k_glyph_accidentalLowerOneUndecimalQuartertone, 	// Lower by one undecimal quartertone
    k_glyph_accidentalRaiseOneUndecimalQuartertone, 	// Raise by one undecimal quartertone
    k_glyph_accidentalLowerOneTridecimalQuartertone, 	// Lower by one tridecimal quartertone
    k_glyph_accidentalRaiseOneTridecimalQuartertone, 	// Raise by one tridecimal quartertone
    k_glyph_accidentalCombiningLower17Schisma, 	// Combining lower by one 17-limit schisma
    k_glyph_accidentalCombiningRaise17Schisma, 	// Combining raise by one 17-limit schisma
    k_glyph_accidentalCombiningLower19Schisma, 	// Combining lower by one 19-limit schisma
    k_glyph_accidentalCombiningRaise19Schisma, 	// Combining raise by one 19-limit schisma
    k_glyph_accidentalCombiningLower23Limit29LimitComma, 	// Combining lower by one 23-limit comma or 29-limit comma
    k_glyph_accidentalCombiningRaise23Limit29LimitComma, 	// Combining raise by one 23-limit comma or 29-limit comma
    k_glyph_accidentalCombiningLower31Schisma, 	// Combining lower by one 31-limit schisma
    k_glyph_accidentalCombiningRaise31Schisma, 	// Combining raise by one 31-limit schisma
    k_glyph_accidentalCombiningOpenCurlyBrace, 	// Combining open curly brace
    k_glyph_accidentalCombiningCloseCurlyBrace, // Combining close curly brace
    k_glyph_accidentalDoubleFlatEqualTempered, 	// Double flat equal tempered semitone
    k_glyph_accidentalFlatEqualTempered, 	    // Flat equal tempered semitone
    k_glyph_accidentalNaturalEqualTempered, 	// Natural equal tempered semitone
    k_glyph_accidentalSharpEqualTempered, 	    // Sharp equal tempered semitone
    k_glyph_accidentalDoubleSharpEqualTempered, // Double sharp equal tempered semitone

    // Spartan Sagittal single-shaft accidentals (U+E30x–U+E30x)
    k_glyph_accSagittal5v7KleismaUp, 	    // 5:7 kleisma up
    k_glyph_accSagittal5v7KleismaDown, 	    // 5:7 kleisma down
    k_glyph_accSagittal5CommaUp, 	        // 5 comma up
    k_glyph_accSagittal5CommaDown, 	        // 5 comma down
    k_glyph_accSagittal7CommaUp, 	        // 7 comma up
    k_glyph_accSagittal7CommaDown, 	        // 7 comma down
    k_glyph_accSagittal25SmallDiesisUp, 	// 25 small diesis up
    k_glyph_accSagittal25SmallDiesisDown, 	// 25 small diesis down
    k_glyph_accSagittal35MediumDiesisUp, 	// 35 medium diesis up
    k_glyph_accSagittal35MediumDiesisDown, 	// 35 medium diesis down
    k_glyph_accSagittal11MediumDiesisUp, 	// 11 medium diesis up
    k_glyph_accSagittal11MediumDiesisDown, 	// 11 medium diesis down
    k_glyph_accSagittal11LargeDiesisUp, 	// 11 large diesis up
    k_glyph_accSagittal11LargeDiesisDown, 	// 11 large diesis down
    k_glyph_accSagittal35LargeDiesisUp, 	// 35 large diesis up
    k_glyph_accSagittal35LargeDiesisDown, 	// 35 large diesis down

    // Spartan Sagittal multi-shaft accidentals (U+E310xU+E33F)
    k_glyph_accSagittalSharp25SDown, 	// Sharp 25S-down
    k_glyph_accSagittalFlat25SUp, 	    // Flat 25S-up
    k_glyph_accSagittalSharp7CDown, 	// Sharp 7C-down
    k_glyph_accSagittalFlat7CUp, 	    // Flat 7C-up
    k_glyph_accSagittalSharp5CDown, 	// Sharp 5C-down
    k_glyph_accSagittalFlat5CUp, 	    // Flat 5C-up
    k_glyph_accSagittalSharp5v7kDown, 	// Sharp 5:7k-down
    k_glyph_accSagittalFlat5v7kUp, 	    // Flat 5:7k-up
    k_glyph_accSagittalSharp, 	        // Sharp
    k_glyph_accSagittalFlat, 	        // Flat
    k_glyph_accSagittalUnused1, 	    // Unused
    k_glyph_accSagittalUnused2, 	    // Unused
    k_glyph_accSagittalSharp5v7kUp, 	// Sharp 5:7k-up
    k_glyph_accSagittalFlat5v7kDown, 	// Flat 5:7k-down
    k_glyph_accSagittalSharp5CUp, 	    // Sharp 5C-up
    k_glyph_accSagittalFlat5CDown, 	    // Flat 5C-down
    k_glyph_accSagittalSharp7CUp, 	    // Sharp 7C-up
    k_glyph_accSagittalFlat7CDown, 	    // Flat 7C-down
    k_glyph_accSagittalSharp25SUp, 	    // Sharp 25S-up
    k_glyph_accSagittalFlat25SDown, 	// Flat 25S-down
    k_glyph_accSagittalSharp35MUp, 	    // Sharp 35M-up
    k_glyph_accSagittalFlat35MDown, 	// Flat 35M-down
    k_glyph_accSagittalSharp11MUp, 	    // Sharp 11M-up
    k_glyph_accSagittalFlat11MDown, 	// Flat 11M-down
    k_glyph_accSagittalSharp11LUp, 	    // Sharp 11L-up
    k_glyph_accSagittalFlat11LDown, 	// Flat 11L-down
    k_glyph_accSagittalSharp35LUp, 	    // Sharp 35L-up
    k_glyph_accSagittalFlat35LDown, 	    // Flat 35L-down
    k_glyph_accSagittalDoubleSharp25SDown, 	// Double sharp 25S-down
    k_glyph_accSagittalDoubleFlat25SUp, 	// Double flat 25S-up
    k_glyph_accSagittalDoubleSharp7CDown, 	// Double sharp 7C-down
    k_glyph_accSagittalDoubleFlat7CUp, 	    // Double flat 7C-up
    k_glyph_accSagittalDoubleSharp5CDown, 	// Double sharp 5C-down
    k_glyph_accSagittalDoubleFlat5CUp, 	    // Double flat 5C-up
    k_glyph_accSagittalDoubleSharp5v7kDown, // Double sharp 5:7k-down
    k_glyph_accSagittalDoubleFlat5v7kUp, 	// Double flat 5:7k-up
    k_glyph_accSagittalDoubleSharp, 	    // Double sharp
    k_glyph_accSagittalDoubleFlat, 	        // Double flat

    // Athenian Sagittal extension (medium precision) accidentals (U+E340xU+E36F)
    k_glyph_accSagittal7v11KleismaUp, 	// 7:11 kleisma up
    k_glyph_accSagittal7v11KleismaDown, // 7:11 kleisma down
    k_glyph_accSagittal17CommaUp, 	    // 17 comma up
    k_glyph_accSagittal17CommaDown, 	// 17 comma down
    k_glyph_accSagittal55CommaUp, 	    // 55 comma up
    k_glyph_accSagittal55CommaDown, 	// 55 comma down
    k_glyph_accSagittal7v11CommaUp, 	// 7:11 comma up
    k_glyph_accSagittal7v11CommaDown, 	// 7:11 comma down
    k_glyph_accSagittal5v11SmallDiesisUp, 	// 5:11 small diesis up
    k_glyph_accSagittal5v11SmallDiesisDown, // 5:11 small diesis down
    k_glyph_accSagittalSharp5v11SDown, 	// Sharp 5:11S-down
    k_glyph_accSagittalFlat5v11SUp, 	// Flat 5:11S-up
    k_glyph_accSagittalSharp7v11CDown, 	// Sharp 7:11C-down
    k_glyph_accSagittalFlat7v11CUp, 	// Flat 7:11C-up
    k_glyph_accSagittalSharp55CDown, 	// Sharp 55C-down
    k_glyph_accSagittalFlat55CUp, 	    // Flat 55C-up
    k_glyph_accSagittalSharp17CDown, 	// Sharp 17C-down
    k_glyph_accSagittalFlat17CUp, 	    // Flat 17C-up
    k_glyph_accSagittalSharp7v11kDown, 	// Sharp 7:11k-down
    k_glyph_accSagittalFlat7v11kUp, 	// Flat 7:11k-up
    k_glyph_accSagittalSharp7v11kUp, 	// Sharp 7:11k-up
    k_glyph_accSagittalFlat7v11kDown, 	// Flat 7:11k-down
    k_glyph_accSagittalSharp17CUp, 	    // Sharp 17C-up
    k_glyph_accSagittalFlat17CDown, 	// Flat 17C-down
    k_glyph_accSagittalSharp55CUp, 	    // Sharp 55C-up
    k_glyph_accSagittalFlat55CDown, 	// Flat 55C-down
    k_glyph_accSagittalSharp7v11CUp, 	// Sharp 7:11C-up
    k_glyph_accSagittalFlat7v11CDown, 	// Flat 7:11C-down
    k_glyph_accSagittalSharp5v11SUp, 	// Sharp 5:11S-up
    k_glyph_accSagittalFlat5v11SDown, 	// Flat 5:11S-down
    k_glyph_accSagittalDoubleSharp5v11SDown, 	// Double sharp 5:11S-down
    k_glyph_accSagittalDoubleFlat5v11SUp, 	    // Double flat 5:11S-up
    k_glyph_accSagittalDoubleSharp7v11CDown, 	// Double sharp 7:11C-down
    k_glyph_accSagittalDoubleFlat7v11CUp, 	    // Double flat 7:11C-up
    k_glyph_accSagittalDoubleSharp55CDown, 	    // Double sharp 55C-down
    k_glyph_accSagittalDoubleFlat55CUp, 	    // Double flat 55C-up
    k_glyph_accSagittalDoubleSharp17CDown, 	    // Double sharp 17C-down
    k_glyph_accSagittalDoubleFlat17CUp, 	    // Double flat 17C-up
    k_glyph_accSagittalDoubleSharp7v11kDown, 	// Double sharp 7:11k-down
    k_glyph_accSagittalDoubleFlat7v11kUp, 	    // Double flat 7:11k-up

    // Trojan Sagittal extension (12-EDO relative) accidentals (U+E370xU+E38F)
    k_glyph_accSagittal23CommaUp, 	        // 23 comma up
    k_glyph_accSagittal23CommaDown, 	    // 23 comma down
    k_glyph_accSagittal5v19CommaUp, 	    // 5:19 comma up
    k_glyph_accSagittal5v19CommaDown, 	    // 5:19 comma down
    k_glyph_accSagittal5v23SmallDiesisUp, 	// 5:23 small diesis up
    k_glyph_accSagittal5v23SmallDiesisDown, // 5:23 small diesis down
    k_glyph_accSagittalSharp5v23SDown, 	    // Sharp 5:23S-down
    k_glyph_accSagittalFlat5v23SUp, 	    // Flat 5:23S-up
    k_glyph_accSagittalSharp5v19CDown, 	    // Sharp 5:19C-down
    k_glyph_accSagittalFlat5v19CUp, 	    // Flat 5:19C-up
    k_glyph_accSagittalSharp23CDown, 	    // Sharp 23C-down
    k_glyph_accSagittalFlat23CUp, 	        // Flat 23C-up
    k_glyph_accSagittalSharp23CUp, 	        // Sharp 23C-up
    k_glyph_accSagittalFlat23CDown, 	    // Flat 23C-down
    k_glyph_accSagittalSharp5v19CUp, 	    // Sharp 5:19C-up
    k_glyph_accSagittalFlat5v19CDown, 	    // Flat 5:19C-down
    k_glyph_accSagittalSharp5v23SUp, 	    // Sharp 5:23S-up
    k_glyph_accSagittalFlat5v23SDown, 	    // Flat 5:23S-down
    k_glyph_accSagittalDoubleSharp5v23SDown, 	// Double sharp 5:23S-down
    k_glyph_accSagittalDoubleFlat5v23SUp, 	// Double flat 5:23S-up
    k_glyph_accSagittalDoubleSharp5v19CDown, 	// Double sharp 5:19C-down
    k_glyph_accSagittalDoubleFlat5v19CUp, 	// Double flat 5:19C-up
    k_glyph_accSagittalDoubleSharp23CDown, 	// Double sharp 23C-down
    k_glyph_accSagittalDoubleFlat23CUp, 	// Double flat 23C-up

    // Promethean Sagittal extension (high precision) single-shaft accidentals (U+E390xU+E3AF)
    k_glyph_accSagittal19SchismaUp, 	    // 19 schisma up
    k_glyph_accSagittal19SchismaDown, 	    // 19 schisma down
    k_glyph_accSagittal17KleismaUp, 	    // 17 kleisma up
    k_glyph_accSagittal17KleismaDown, 	    // 17 kleisma down
    k_glyph_accSagittal143CommaUp, 	        // 143 comma up
    k_glyph_accSagittal143CommaDown, 	    // 143 comma down
    k_glyph_accSagittal11v49CommaUp, 	    // 11:49 comma up
    k_glyph_accSagittal11v49CommaDown, 	    // 11:49 comma down
    k_glyph_accSagittal19CommaUp, 	        // 19 comma up
    k_glyph_accSagittal19CommaDown, 	    // 19 comma down
    k_glyph_accSagittal7v19CommaUp, 	    // 7:19 comma up
    k_glyph_accSagittal7v19CommaDown, 	    // 7:19 comma down
    k_glyph_accSagittal49SmallDiesisUp, 	// 49 small diesis up
    k_glyph_accSagittal49SmallDiesisDown, 	// 49 small diesis down
    k_glyph_accSagittal23SmallDiesisUp, 	// 23 small diesis up
    k_glyph_accSagittal23SmallDiesisDown, 	// 23 small diesis down
    k_glyph_accSagittal5v13MediumDiesisUp, 	// 5:13 medium diesis up
    k_glyph_accSagittal5v13MediumDiesisDown, 	// 5:13 medium diesis down
    k_glyph_accSagittal11v19MediumDiesisUp, 	// 11:19 medium diesis up
    k_glyph_accSagittal11v19MediumDiesisDown, 	// 11:19 medium diesis down
    k_glyph_accSagittal49MediumDiesisUp, 	    // 49 medium diesis up
    k_glyph_accSagittal49MediumDiesisDown, 	    // 49 medium diesis down
    k_glyph_accSagittal5v49MediumDiesisUp, 	    // 5:49 medium diesis up
    k_glyph_accSagittal5v49MediumDiesisDown, 	// 5:49 medium diesis down
    k_glyph_accSagittal49LargeDiesisUp, 	    // 49 large diesis up
    k_glyph_accSagittal49LargeDiesisDown, 	    // 49 large diesis down
    k_glyph_accSagittal11v19LargeDiesisUp, 	    // 11:19 large diesis up
    k_glyph_accSagittal11v19LargeDiesisDown, 	// 11:19 large diesis down
    k_glyph_accSagittal5v13LargeDiesisUp, 	    // 5:13 large diesis up
    k_glyph_accSagittal5v13LargeDiesisDown, 	// 5:13 large diesis down

    // Promethean Sagittal extension (high precision) multi-shaft accidentals (U+E3B0xU+E3EF)
    k_glyph_accSagittalSharp23SDown, 	// Sharp 23S-down
    k_glyph_accSagittalFlat23SUp, 	    // Flat 23S-up
    k_glyph_accSagittalSharp49SDown, 	// Sharp 49S-down
    k_glyph_accSagittalFlat49SUp, 	    // Flat 49S-up
    k_glyph_accSagittalSharp7v19CDown, 	// Sharp 7:19C-down
    k_glyph_accSagittalFlat7v19CUp, 	// Flat 7:19C-up
    k_glyph_accSagittalSharp19CDown, 	// Sharp 19C-down
    k_glyph_accSagittalFlat19CUp, 	    // Flat 19C-up
    k_glyph_accSagittalSharp11v49CDown, // Sharp 11:49C-down
    k_glyph_accSagittalFlat11v49CUp, 	// Flat 11:49C-up
    k_glyph_accSagittalSharp143CDown, 	// Sharp 143C-down
    k_glyph_accSagittalFlat143CUp, 	    // Flat 143C-up
    k_glyph_accSagittalSharp17kDown, 	// Sharp 17k-down
    k_glyph_accSagittalFlat17kUp, 	    // Flat 17k-up
    k_glyph_accSagittalSharp19sDown, 	// Sharp 19s-down
    k_glyph_accSagittalFlat19sUp, 	    // Flat 19s-up
    k_glyph_accSagittalSharp19sUp, 	    // Sharp 19s-up
    k_glyph_accSagittalFlat19sDown, 	// Flat 19s-down
    k_glyph_accSagittalSharp17kUp, 	    // Sharp 17k-up
    k_glyph_accSagittalFlat17kDown, 	// Flat 17k-down
    k_glyph_accSagittalSharp143CUp, 	// Sharp 143C-up
    k_glyph_accSagittalFlat143CDown, 	// Flat 143C-down
    k_glyph_accSagittalSharp11v49CUp, 	// Sharp 11:49C-up
    k_glyph_accSagittalFlat11v49CDown, 	// Flat 11:49C-down
    k_glyph_accSagittalSharp19CUp, 	    // Sharp 19C-up
    k_glyph_accSagittalFlat19CDown, 	// Flat 19C-down
    k_glyph_accSagittalSharp7v19CUp, 	// Sharp 7:19C-up
    k_glyph_accSagittalFlat7v19CDown, 	// Flat 7:19C-down
    k_glyph_accSagittalSharp49SUp, 	    // Sharp 49S-up
    k_glyph_accSagittalFlat49SDown, 	// Flat 49S-down
    k_glyph_accSagittalSharp23SUp, 	    // Sharp 23S-up
    k_glyph_accSagittalFlat23SDown, 	// Flat 23S-down
    k_glyph_accSagittalSharp5v13MUp, 	// Sharp 5:13M-up
    k_glyph_accSagittalFlat5v13MDown, 	// Flat 5:13M-down
    k_glyph_accSagittalSharp11v19MUp, 	// Sharp 11:19M-up
    k_glyph_accSagittalFlat11v19MDown, 	// Flat 11:19M-down
    k_glyph_accSagittalSharp49MUp, 	    // Sharp 49M-up
    k_glyph_accSagittalFlat49MDown, 	// Flat 49M-down
    k_glyph_accSagittalSharp5v49MUp, 	// Sharp 5:49M-up
    k_glyph_accSagittalFlat5v49MDown, 	// Flat 5:49M-down
    k_glyph_accSagittalSharp49LUp, 	    // Sharp 49L-up
    k_glyph_accSagittalFlat49LDown, 	// Flat 49L-down
    k_glyph_accSagittalSharp11v19LUp, 	// Sharp 11:19L-up
    k_glyph_accSagittalFlat11v19LDown, 	// Flat 11:19L-down
    k_glyph_accSagittalSharp5v13LUp, 	// Sharp 5:13L-up
    k_glyph_accSagittalFlat5v13LDown, 	// Flat 5:13L-down
    k_glyph_accSagittalUnused3, 	        // Unused
    k_glyph_accSagittalUnused4, 	        // Unused
    k_glyph_accSagittalDoubleSharp23SDown, 	    // Double sharp 23S-down
    k_glyph_accSagittalDoubleFlat23SUp, 	    // Double flat 23S-up
    k_glyph_accSagittalDoubleSharp49SDown, 	    // Double sharp 49S-down
    k_glyph_accSagittalDoubleFlat49SUp, 	    // Double flat 49S-up
    k_glyph_accSagittalDoubleSharp7v19CDown, 	// Double sharp 7:19C-down
    k_glyph_accSagittalDoubleFlat7v19CUp, 	    // Double flat 7:19C-up
    k_glyph_accSagittalDoubleSharp19CDown, 	    // Double sharp 19C-down
    k_glyph_accSagittalDoubleFlat19CUp, 	    // Double flat 19C-up
    k_glyph_accSagittalDoubleSharp11v49CDown, 	// Double sharp 11:49C-down
    k_glyph_accSagittalDoubleFlat11v49CUp, 	    // Double flat 11:49C-up
    k_glyph_accSagittalDoubleSharp143CDown, 	// Double sharp 143C-down
    k_glyph_accSagittalDoubleFlat143CUp, 	    // Double flat 143C-up
    k_glyph_accSagittalDoubleSharp17kDown, 	    // Double sharp 17k-down
    k_glyph_accSagittalDoubleFlat17kUp, 	    // Double flat 17k-up
    k_glyph_accSagittalDoubleSharp19sDown, 	    // Double sharp 19s-down
    k_glyph_accSagittalDoubleFlat19sUp, 	    // Double flat 19s-up

    // Wyschnegradsky accidentals (72-EDO) (U+E420xU+E43F)
    k_glyph_accidentalWyschnegradsky1TwelfthsSharp,     // 1/12 tone sharp
    k_glyph_accidentalWyschnegradsky2TwelfthsSharp,     // 1/6 tone sharp
    k_glyph_accidentalWyschnegradsky3TwelfthsSharp,     // 1/4 tone sharp
    k_glyph_accidentalWyschnegradsky4TwelfthsSharp,     // 1/3 tone sharp
    k_glyph_accidentalWyschnegradsky5TwelfthsSharp,     // 5/12 tone sharp
    k_glyph_accidentalWyschnegradsky6TwelfthsSharp,     // 1/2 tone sharp
    k_glyph_accidentalWyschnegradsky7TwelfthsSharp,     // 7/12 tone sharp
    k_glyph_accidentalWyschnegradsky8TwelfthsSharp,     // 2/3 tone sharp
    k_glyph_accidentalWyschnegradsky9TwelfthsSharp,     // 3/4 tone sharp
    k_glyph_accidentalWyschnegradsky10xwelfthsSharp, 	// 5/6 tone sharp
    k_glyph_accidentalWyschnegradsky11TwelfthsSharp, 	// 11/12 tone sharp
    k_glyph_accidentalWyschnegradsky1TwelfthsFlat, 	    // 1/12 tone flat
    k_glyph_accidentalWyschnegradsky2TwelfthsFlat, 	    // 1/6 tone flat
    k_glyph_accidentalWyschnegradsky3TwelfthsFlat, 	    // 1/4 tone flat
    k_glyph_accidentalWyschnegradsky4TwelfthsFlat, 	    // 1/3 tone flat
    k_glyph_accidentalWyschnegradsky5TwelfthsFlat, 	    // 5/12 tone flat
    k_glyph_accidentalWyschnegradsky6TwelfthsFlat, 	    // 1/2 tone flat
    k_glyph_accidentalWyschnegradsky7TwelfthsFlat, 	    // 7/12 tone flat
    k_glyph_accidentalWyschnegradsky8TwelfthsFlat, 	    // 2/3 tone flat
    k_glyph_accidentalWyschnegradsky9TwelfthsFlat, 	    // 3/4 tone flat
    k_glyph_accidentalWyschnegradsky10xwelfthsFlat,     // 5/6 tone flat
    k_glyph_accidentalWyschnegradsky11TwelfthsFlat,     // 11/12 tone flat

    // Arel-Ezgi-Uzdilek (AEU) accidentals (U+E440xU+E44F)
    k_glyph_accidentalBuyukMucennebFlat, 	// Büyük mücenneb (flat)
    k_glyph_accidentalKucukMucennebFlat, 	// Küçük mücenneb (flat)
    k_glyph_accidentalBakiyeFlat, 	        // Bakiye (flat)
    k_glyph_accidentalKomaFlat, 	        // Koma (flat)
    k_glyph_accidentalKomaSharp, 	        // Koma (sharp)
    k_glyph_accidentalBakiyeSharp, 	        // Bakiye (sharp)
    k_glyph_accidentalKucukMucennebSharp, 	// Küçük mücenneb (sharp)
    k_glyph_accidentalBuyukMucennebSharp, 	// Büyük mücenneb (sharp)

    // Turkish folk music accidentals (U+E450xU+E45F)
    k_glyph_accidental1CommaSharp, 	// 1-comma sharp
    k_glyph_accidental2CommaSharp, 	// 2-comma sharp
    k_glyph_accidental3CommaSharp, 	// 3-comma sharp
    k_glyph_accidental5CommaSharp, 	// 5-comma sharp
    k_glyph_accidental1CommaFlat, 	// 1-comma flat
    k_glyph_accidental2CommaFlat, 	// 2-comma flat
    k_glyph_accidental3CommaFlat, 	// 3-comma flat
    k_glyph_accidental4CommaFlat, 	// 4-comma flat

    // Persian accidentals (U+E460xU+E46F)
    k_glyph_accidentalKoron, 	    // Koron (quarter tone flat)
    k_glyph_accidentalSori, 	    // Sori (quarter tone sharp)

    //Other accidentals (U+E470xU+E49F)
    k_glyph_accidentalXenakisOneThirdToneSharp,     // One-third-tone sharp (Xenakis)
    k_glyph_accidentalXenakisTwoThirdTonesSharp, 	// Two-third-tones sharp (Xenakis)
    k_glyph_accidentalQuarterToneSharpBusotti, 	    // Quarter tone sharp (Bussotti)
    k_glyph_accidentalSharpOneHorizontalStroke,     // One or three quarter tones sharp
    k_glyph_accidentalThreeQuarterTonesSharpBusotti, 	// Three quarter tones sharp (Bussotti)
    k_glyph_accidentalQuarterToneSharpWiggle, 	    // Quarter tone sharp with wiggly tail
    k_glyph_accidentalTavenerSharp, 	            // Byzantine-style Bu\u0x,yu\u0x,k mu\u0x,cenneb sharp (Tavener)
    k_glyph_accidentalTavenerFlat, 	                // Byzantine-style Bakiye flat (Tavener)
    k_glyph_accidentalQuarterToneFlatPenderecki, 	// Quarter tone flat (Penderecki)
    k_glyph_accidentalCommaSlashUp, 	            // Syntonic/Didymus comma (80x1) up (Bosanquet)
    k_glyph_accidentalCommaSlashDown, 	            // Syntonic/Didymus comma (80x1) down (Bosanquet)
    k_glyph_accidentalWilsonPlus, 	                // Wilson plus (5 comma up)
    k_glyph_accidentalWilsonMinus, 	                // Wilson minus (5 comma down)
    k_glyph_accidentalLargeDoubleSharp, 	        // Large double sharp
    k_glyph_accidentalQuarterToneSharp4, 	        // Quarter-tone sharp"
    k_glyph_accidentalQuarterToneFlat4, 	            // Quarter-tone flat"
    k_glyph_accidentalQuarterToneFlatFilledReversed, 	// Filled reversed flat (quarter-tone flat)
    k_glyph_accidentalSharpReversed, 	            // Reversed sharp
    k_glyph_accidentalNaturalReversed, 	            // Reversed natural
    k_glyph_accidentalDoubleFlatReversed, 	        // Reversed double flat
    k_glyph_accidentalFlatTurned, 	                // Turned flat
    k_glyph_accidentalDoubleFlatTurned, 	            // Turned double flat
    k_glyph_accidentalThreeQuarterTonesFlatGrisey, 	    // Three-quarter-tones flat (Grisey)
    k_glyph_accidentalThreeQuarterTonesFlatTartini, 	// Three-quarter-tones flat (Tartini)
    k_glyph_accidentalQuarterToneFlatVanBlankenburg, 	// Quarter-tone flat (van Blankenburg)
    k_glyph_accidentalThreeQuarterTonesFlatCouper, 	    // Three-quarter-tones flat (Couper)
    k_glyph_accidentalOneThirdToneSharpFerneyhough, 	// One-third-tone sharp (Ferneyhough)
    k_glyph_accidentalOneThirdToneFlatFerneyhough, 	    // One-third-tone flat (Ferneyhough)
    k_glyph_accidentalTwoThirdTonesSharpFerneyhough, 	// Two-third-tones sharp (Ferneyhough)
    k_glyph_accidentalTwoThirdTonesFlatFerneyhough, 	// Two-third-tones flat (Ferneyhough)

    //Articulation (U+E4A0 - U+E4BF)
    k_glyph_accent_above,
    k_glyph_accent_below,
    k_glyph_staccato_above,
    k_glyph_staccato_below,
    k_glyph_tenuto_above,
    k_glyph_tenuto_below,
    k_glyph_staccatissimo_above,
    k_glyph_staccatissimo_below,
    k_glyph_staccatissimo_wedge_above,
    k_glyph_staccatissimo_wedge_below,
    k_glyph_staccatissimo_stroke_above,
    k_glyph_staccatissimo_stroke_below,
    k_glyph_marcato_above,
    k_glyph_marcato_below,
    k_glyph_marcato_staccato_above,
    k_glyph_marcato_staccato_below,
    k_glyph_accent_staccato_above,
    k_glyph_accent_staccato_below,
    k_glyph_tenuto_staccato_above,
    k_glyph_tenuto_staccato_below,
    k_glyph_tenuto_accent_above,
    k_glyph_tenuto_accent_below,
    k_glyph_stress_above,
    k_glyph_stress_below,
    k_glyph_unstress_above,
    k_glyph_unstress_below,
    k_glyph_laissez_vibrer_above,
    k_glyph_laissez_vibrer_below,
    k_glyph_marcato_tenuto_above,
    k_glyph_marcato_tenuto_below,

    //Holds and pauses (U+E4C0 - U+E4DF)
    k_glyph_fermata_above,
    k_glyph_fermata_below,
    k_glyph_fermata_very_short_above,
    k_glyph_fermata_very_short_below,
    k_glyph_fermata_above_angle,
    k_glyph_fermata_below_angle,
    k_glyph_fermata_above_square,
    k_glyph_fermata_below_square,
    k_glyph_fermata_very_long_above,
    k_glyph_fermata_very_long_below,
    k_glyph_fermata_long_above,
    k_glyph_fermata_long_below,
    k_glyph_fermata_short_above,
    k_glyph_fermata_short_below,
    k_glyph_breath_mark_comma,      //breath-mark (comma)
    k_glyph_breath_mark_tick,       //breath-mark (tick)
    k_glyph_breath_mark_v,          //breath-mark (V)
    k_glyph_caesura,
    k_glyph_caesura_thick,
    k_glyph_caesura_short,
    k_glyph_caesura_curved,
    k_glyph_breath_mark_salzedo,    //breath-mark (salcedo)
    k_glyph_curlew_mark,            //curlew mark

    //Rests (U+E4E0 - U+E4FF)
    k_glyph_longa_rest,       //longa
    k_glyph_breve_rest,       //breve, cuadrada
    k_glyph_whole_rest,       //whole, redonda
    k_glyph_half_rest,        //half, blanca
    k_glyph_quarter_rest,     //quarter, negra
    k_glyph_eighth_rest,      //eighth, corchea
    k_glyph_16th_rest,        //16th, semicorchea
    k_glyph_32nd_rest,        //32nd, fusa
    k_glyph_64th_rest,        //64th, semifusa
    k_glyph_128th_rest,       //128th, garrapatea
    k_glyph_256th_rest,       //256th, semigarrapatea
    k_glyph_512th_rest,
    k_glyph_1024th_rest,

    //Octaves (U+E510 - U+E51F)
    k_glyph_ottava,                 //Ottava: 8
    k_glyph_ottavaAlta,             //Ottava alta: 8va ('va' at top)
    k_glyph_ottavaBassa,            //Ottava bassa: 8va ('va' at bottom)
    k_glyph_ottavaBassaBa,          //Ottava bassa: 8ba
    k_glyph_quindicesima,           //Quindicesima: 15
    k_glyph_quindicesimaAlta,       //Quindicesima alta: 15ma ('ma' at top)
    k_glyph_quindicesimaBassa,      //Quindicesima bassa: 15ma ('ma' at bottom)
    k_glyph_ventiduesima,           //Ventiduesima: 22
    k_glyph_ventiduesimaAlta,       //Ventiduesima alta: 22ma ('ma' at top)
    k_glyph_ventiduesimaBassa,      //Ventiduesima bassa: 22ma ('ma' at bottom)
    k_glyph_octaveParensLeft,       //Left parenthesis for octave signs
    k_glyph_octaveParensRight,      //Right parenthesis for octave signs
    k_glyph_ottavaBassaVb,          //Ottava bassa: 8vb
    k_glyph_quindicesimaBassaMb,    //Quindicesima bassa: 15mb
    k_glyph_ventiduesimaBassaMb,    //Ventiduesima bassa: 22mb
    k_glyph_octaveBassa,            //'bassa' word

    //Dynamics (U+E520 - U+E54F)
    k_glyph_dynamic_p,          //Piano
    k_glyph_dynamic_m,          //Mezzo
    k_glyph_dynamic_f,          //Forte
    k_glyph_dynamic_r,          //Rinforzando
    k_glyph_dynamic_s,          //Sforzando
    k_glyph_dynamic_z,          //Z
    k_glyph_dynamic_n,          //Niente
    k_glyph_dynamic_6p,         //pppppp
    k_glyph_dynamic_5p,         //ppppp
    k_glyph_dynamic_4p,         //pppp
    k_glyph_dynamic_3p,         //ppp
    k_glyph_dynamic_2p,         //pp
    k_glyph_dynamic_mp,         //mp
    k_glyph_dynamic_mf,         //mf
    k_glyph_dynamic_pf,         //pf
    k_glyph_dynamic_2f,         //ff
    k_glyph_dynamic_3f,         //fff
    k_glyph_dynamic_4f,         //ffff
    k_glyph_dynamic_5f,         //fffff
    k_glyph_dynamic_6f,         //ffffff
    k_glyph_dynamic_fp,         //FortePiano
    k_glyph_dynamic_fz,         //Forzando
    k_glyph_dynamic_sf,         //Sforzando1
    k_glyph_dynamic_sfp,        //SforzandoPiano
    k_glyph_dynamic_sfpp,       //SforzandoPianissimo
    k_glyph_dynamic_sfz,        //Sforzato
    k_glyph_dynamic_sfzp,       //SforzatoPiano
    k_glyph_dynamic_sffz,       //SforzatoFF
    k_glyph_dynamic_rf,         //Rinforzando1
    k_glyph_dynamic_rfz,        //Rinforzando2

    //Common ornaments (U+E560 - U+E56F)
    k_glyph_slashed_grace_note_stem_up,     // Slashed grace note stem up
    k_glyph_slashed_grace_note_stem_down,   // Slashed grace note stem down
    k_glyph_grace_note_stem_up,             // Grace note stem up
    k_glyph_grace_note_stem_down,           // Grace note stem down
    k_glyph_slash_for_stem_up_grace_note,   // Slash for stem up grace note
    k_glyph_slash_for_stem_down_grace_note, // Slash for stem down grace note
    k_glyph_trill,                          // Trill
    k_glyph_turn,                           // Turn
    k_glyph_inverted_turn,                  // Inverted Turn
    k_glyph_turn_with_slash,                // Turn with slash
    k_glyph_turn_up,                        // Turn up
    k_glyph_inverted_turn_up,               // Inverted turn up
    k_glyph_mordent,                        // Mordent
    k_glyph_inverted_mordent,               // Inverted mordent
    k_glyph_tremblement,                    // Tremblement
    k_glyph_haydn_ornament,                 // Haydn ornament

    //Other baroque ornaments (U+E570 - U+E58F)
    k_glyph_schleifer_ornament,             // Schleifer ornament

    //Brass techniques (U+E5D0 - U+E5EF)
    k_glyph_brass_scoop,                        //Scoop
    k_glyph_brass_lift_short,                   //Lift, short
    k_glyph_brass_lift_medium,                  //Lift, medium
    k_glyph_brass_lift_long,                    //Lift, long
    k_glyph_brass_doit_short,                   //Doit, short
    k_glyph_brass_doit_medium,                  //Doit, medium
    k_glyph_brass_doit_long,                    //Doit, long
    k_glyph_brass_fall_lip_short,               //Lip fall, short
    k_glyph_brass_fall_lip_medium,              //Lip fall, medium
    k_glyph_brass_fall_lip_long,                //Lip fall, long
    k_glyph_brass_fall_smooth_short,            //Smooth fall, short
    k_glyph_brass_fall_smooth_medium,           //Smooth fall, medium
    k_glyph_brass_fall_smooth_long,             //Smooth fall, long
    k_glyph_brass_fall_rough_short,             //Rough fall, short
    k_glyph_brass_fall_rough_medium,            //Rough fall, medium
    k_glyph_brass_fall_rough_long,              //Rough fall, long
    k_glyph_brass_plop,                         //Plop
    k_glyph_brass_flip,                         //Flip
    k_glyph_brass_smear,                        //Smear
    k_glyph_brass_bend,                         //Bend
    k_glyph_brass_jazz_turn,                    //Jazz turn
    k_glyph_brass_mute_closed,                  //Muted (closed)
    k_glyph_brass_mute_half_closed,             //Half-muted (half-closed)
    k_glyph_brass_mute_open,                    //Open
    k_glyph_brass_harmon_mute_closed,           //Harmon mute, stem in
    k_glyph_brass_harmon_mute_stem_half_left,   //Harmon mute, stem extended, left
    k_glyph_brass_harmon_mute_stem_half_right,  //Harmon mute, stem extended, right
    k_glyph_brass_harmon_mute_stem_open,        //Harmon mute, stem out
    k_glyph_brass_lift_smooth_short,            //Smooth lift, short
    k_glyph_brass_lift_smooth_medium,           //Smooth lift, medium
    k_glyph_brass_lift_smooth_long,             //Smooth lift, long

    //Wind techniques (U+E5F0 –U+E60F)
    k_glyph_double_tongue_above,                //Double-tongue above
    k_glyph_double_tongue_below,                //Double-tongue below
    k_glyph_triple_tongue_above,                //Triple-tongue above
    k_glyph_triple_tongue_below,                //Triple-tongue below
    k_glyph_wind_closed_hole,                   //Closed hole
    k_glyph_wind_three_quarters_closed_hole,    //Three-quarters closed hole
    k_glyph_wind_half_closed_hole1,             //Half-closed hole
    k_glyph_wind_half_closed_hole2,             //Half-closed hole 2
    k_glyph_wind_half_closed_hole3,             //Half-open hole
    k_glyph_wind_open_hole,                     //Open hole
    k_glyph_wind_trill_key,                     //Trill key
    k_glyph_wind_flat_embouchure,               //Flatter embouchure
    k_glyph_wind_sharp_embouchure,              //Sharper embouchure
    k_glyph_wind_relaxed_embouchure,            //Relaxed embouchure
    k_glyph_wind_less_relaxed_embouchure,       //Somewhat relaxed embouchure
    k_glyph_wind_tight_embouchure,              //Tight embouchure
    k_glyph_wind_less_tight_embouchure,         //Somewhat tight embouchure
    k_glyph_wind_very_tight_embouchure,         //Very tight embouchure
    k_glyph_wind_weak_air_pressure,             //Very relaxed embouchure / weak air-pressure
    k_glyph_wind_strong_air_pressure,           //Very tight embouchure / strong air pressure
    k_glyph_wind_reed_position_normal,          //Normal reed position
    k_glyph_wind_reed_position_out,             //Very little reed (pull outwards)
    k_glyph_wind_reed_position_in,              //Much more reed (push inwards)
    k_glyph_wind_multiphonics_black_stem,       //Combining multiphonics (black) for stem
    k_glyph_wind_multiphonics_white_stem,       //Combining multiphonics (white) for stem
    k_glyph_wind_multiphonics_black_white_stem, //Combining multiphonics (black and white) for stem

    //String techniques (U+E610 - U+E62F)
    k_glyph_string_down_bow,        // Down bow
    k_glyph_string_up_bow,          // Up bow
    k_glyph_string_harmonic,        // Harmonic
    k_glyph_string_half_harmonic,   // Half-harmonic

    //Keyboard techniques (U+E650 - U+E67F)
    k_glyph_pedal_mark,         // Pedal mark
    k_glyph_pedal_p,            // Pedal P
    k_glyph_pedal_up_mark,      // Pedal up mark
    k_glyph_half_pedal_mark,    // Half-pedal mark
    k_glyph_pedal_up_notch,     // Pedal up notch
    k_glyph_pedal_sostenuto,    // Sostenuto pedal mark
    k_glyph_pedal_s,            // Pedal S

    //Handbells (U+E810 - U+E82F)
    k_glyph_bells_martellato,               // Martellato
    k_glyph_bells_martellato_lift,          // Martellato lift
    k_glyph_bells_hand_martellato,          // Hand martellato
    k_glyph_bells_muted_martellato,         // Muted martellato
    k_glyph_mallet_bell_suspended,          // Mallet, bell suspended
    k_glyph_mallet_bell_on_table,           // Mallet, bell on table
    k_glyph_mallet_lift,                    // Mallet lift
    k_glyph_pluck_lift,                     // Pluck lift
    k_glyph_swing_up,                       // Swing up
    k_glyph_swing_down,                     // Swing down
    k_glyph_swing,                          // Swing
    k_glyph_echo,                           // Echo
    k_glyph_echo_2,                         // Echo 2
    k_glyph_gyro,                           // Gyro
    k_glyph_damp_3,                         // Damp 3
    k_glyph_belltree,                       // Belltree
    k_glyph_table_single_handbell,          // Table single handbell
    k_glyph_table_pair_of_handbells,        // Table pair of handbells

    //figured bass (U+EA50 - U+EA6F)
    k_glyph_figured_bass_0,                   //number 0
    k_glyph_figured_bass_1,                   //number 1
    k_glyph_figured_bass_2,                   //number 2
    k_glyph_figured_bass_3,                   //number 3
    k_glyph_figured_bass_4,                   //number 4
    k_glyph_figured_bass_5,                   //number 5
    k_glyph_figured_bass_6,                   //number 6
    k_glyph_figured_bass_7,                   //number 7
    k_glyph_figured_bass_8,                   //number 8
    k_glyph_figured_bass_9,                   //number 9
    k_glyph_figured_bass_sharp,               //Sharp symbol
    k_glyph_figured_bass_flat,                //Flat symbol
    k_glyph_figured_bass_natural,             //Natural symbol
    k_glyph_figured_bass_plus,                // +
    k_glyph_figured_bass_minus,               // -
    k_glyph_figured_bass_open_parenthesis,    // (
    k_glyph_figured_bass_close_parenthesis,   // )
    k_glyph_figured_bass_7_striked,           // 7 with overlayered /

    //Multi-segment lines (U+EAA0 - U+EB0F)
    k_glyph_trill_wiggle_segment,           // Trill wiggle segment
    k_glyph_arpeggiato_wiggle_segment_up,   // Arpeggiato wiggle segment, upwards
    k_glyph_arpeggiato_wiggle_segment_down, // Arpeggiato wiggle segment, downwards
    k_glyph_arpeggiato_arrow_up,            // Arpeggiato arrowhead up
    k_glyph_arpeggiato_arrow_down,          // Arpeggiato arrowhead down

    //Electronic music pictograms (U+EB10 - U+EB5F)
    k_glyph_stop_button,                    // Stop button, used for errors

    //Metronome marks (U+ECA0 - U+ECBF)
    k_glyph_small_longa_note,
    k_glyph_small_whole_note,
    k_glyph_small_half_note,
    k_glyph_small_quarter_note,
    k_glyph_small_eighth_note,
    k_glyph_small_16th_note,
    k_glyph_small_32nd_note,
    k_glyph_small_64th_note,
    k_glyph_small_128th_note,
    k_glyph_small_256th_note,
    k_glyph_metronome_augmentation_dot,

    k_glyph_error,          // stop play button

};

//---------------------------------------------------------------------------------------
// Encapsulate access to glyphs table. A singleton with library scope
class MusicGlyphs
{
protected:
    LibraryScope* m_pLibScope;
    const GlyphData* m_glyphs;

public:
    MusicGlyphs(LibraryScope* pLibScope);
    ~MusicGlyphs() {}

    void update();

    inline unsigned int glyph_code(int iGlyph) { return (*(m_glyphs+iGlyph)).GlyphChar; }
    inline std::string glyph_name(int iGlyph) { return (*(m_glyphs+iGlyph)).GlyphName; }
    inline LUnits glyph_offset(int UNUSED(iGlyph)) { return 0.0f; }
    inline const GlyphData& get_glyph_data(int iGlyph) { return *(m_glyphs+iGlyph); }
};



}   //namespace lomse

#endif    // __LOMSE_GLYPHS_H__


//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_glyphs.h"

#include "lomse_injectors.h"


namespace lomse
{


//---------------------------------------------------------------------------------------
//the glyphs table for SMuFL compliant fonts
//IMPORTANT: The table inicialization values MUST be ordered following the
//          enum EGlyphIndex, defined in lomse_glyphs.h
//---------------------------------------------------------------------------------------
const GlyphData m_glyphs_smufl[] =
{

//Repeats (U+E040 - U+E04F)
    GlyphData(0xE046),  // Da Capo sign
    GlyphData(0xE045),  // Dal Segno sign
    GlyphData(0xE048),  // Coda sign
    GlyphData(0xE047),  // Segno sign

//clefs (U+E050 - U+E07F)
    GlyphData(0xE050),  // G clef
    GlyphData(0xE062),  // F clef
    GlyphData(0xE05C),  // C clef
    GlyphData(0xE06A),  // Percussion clef, block
    GlyphData(0xE052),  // G clef ottava bassa
    GlyphData(0xE053),  // G clef ottava alta
    GlyphData(0xE051),  // G clef quindicesima_bassa,
    GlyphData(0xE054),  // G clef quindicesima_alta
    GlyphData(0xE064),  // F clef ottava bassa
    GlyphData(0xE065),  // F clef ottava alta
    GlyphData(0xE063),  // F clef quindicesima_bassa,
    GlyphData(0xE066),  // F clef quindicesima_alta,
    GlyphData(0xE06D),  // 6-string TAB clef

//time signatures (U+E080 - U+E09F)
    GlyphData(0xE080),  // Number 0
    GlyphData(0xE081),  // Number 1
    GlyphData(0xE082),  // Number 2
    GlyphData(0xE083),  // Number 3
    GlyphData(0xE084),  // Number 4
    GlyphData(0xE085),  // Number 5
    GlyphData(0xE086),  // Number 6
    GlyphData(0xE087),  // Number 7
    GlyphData(0xE088),  // Number 8
    GlyphData(0xE089),  // Number 9
    GlyphData(0xE08A),  // COMMON_TIME
    GlyphData(0xE08B),  // CUT_TIME

//Notheads (U+E0A0 - U+E0FF)
    GlyphData(0xE95D),  // Longa note
    GlyphData(0xE0A0),  // Breve note, Double whole
    GlyphData(0xE0A2),  // Whole note
    GlyphData(0xE0A3),  // Half note
    GlyphData(0xE0A4),  // Quarter note notehead
    GlyphData(0xE0A9),  // Cross notehead

//Individual notes with stem and flag (U+E1D0 - U+E1EF)
    GlyphData(0xE1D4),  // Half note, stem down
    GlyphData(0xE1D3),  // Half note, stem up
    GlyphData(0xE1D6),  // Quarter note, stem down
    GlyphData(0xE1D5),  // Quarter note, stem up
    GlyphData(0xE1D8),  // Eight note, flag bottom
    GlyphData(0xE1D7),  // Eight note, flag top
    GlyphData(0xE1DA),  // 16th note, flag bottom
    GlyphData(0xE1D9),  // 16th note, flag top
    GlyphData(0xE1DC),  // 32nd note, flag bottom
    GlyphData(0xE1DB),  // 32nd note, flag top
    GlyphData(0xE1DE),  // 64th note, flag bottom
    GlyphData(0xE1DD),  // 64th note, flag top)),
    GlyphData(0xE1E0),  // 128th note, flag bottom
    GlyphData(0xE1DF),  // 128th note, flag top)),
    GlyphData(0xE1E2),  // 256th note, flag bottom
    GlyphData(0xE1E1),  // 256th note, flag top)),
    GlyphData(0xE1E7),  // Dot (for dotted notes)

//Tremolos (U+E220 - U+E23F)
    GlyphData(0xE220),  // Combining tremolo 1
    GlyphData(0xE221),  // Combining tremolo 2
    GlyphData(0xE222),  // Combining tremolo 3
    GlyphData(0xE223),  // Combining tremolo 4
    GlyphData(0xE224),  // Combining tremolo 5

//flags for notes (U+E240 - U+E25F)
    GlyphData(0xE241),  // Eight note flag down
    GlyphData(0xE243),  // 16th note flag down
    GlyphData(0xE245),  // 32nd note flag down
    GlyphData(0xE247),  // 64th note flag down
    GlyphData(0xE249),  // 128th note flag down
    GlyphData(0xE24B),  // 256th note flag down
    GlyphData(0xE24D),  // 512th note flag down
    GlyphData(0xE24F),  // 1024th note flag down
    GlyphData(0xE240),  // Eight note flag up
    GlyphData(0xE242),  // 16th note flag up
    GlyphData(0xE244),  // 32nd note flag up
    GlyphData(0xE246),  // 64th note flag up
    GlyphData(0xE248),  // 128th note flag up
    GlyphData(0xE24A),  // 256th note flag up
    GlyphData(0xE24C),  // 512th note flag up
    GlyphData(0xE24E),  // 1024th note flag up
    GlyphData(0xE250),  // flag internal up
    GlyphData(0xE251),  // flag internal down

// Standard accidentals (12-EDO) (U+E260–U+E26F)
    GlyphData(0xE260, "accidentalFlat"),         // Flat"
    GlyphData(0xE261, "accidentalNatural"),      // Natural"
    GlyphData(0xE262, "accidentalSharp"),        // Sharp"
    GlyphData(0xE263, "accidentalDoubleSharp"),  // Double sharp"
    GlyphData(0xE264, "accidentalDoubleFlat"),   // Double flat"
    GlyphData(0xE265, "accidentalTripleSharp"),  // Triple sharp
    GlyphData(0xE266, "accidentalTripleFlat"),   // Triple flat
    GlyphData(0xE267, "accidentalNaturalFlat"),  // Natural flat
    GlyphData(0xE268, "accidentalNaturalSharp"), // Natural sharp
    GlyphData(0xE269, "accidentalSharpSharp"),   // Sharp sharp
    GlyphData(0xE26A, "accidentalParensLeft"),   // "accidental parenthesis
    GlyphData(0xE26B, "accidentalParensRight"),  // "accidental parenthesis
    GlyphData(0xE26C, "accidentalBracketLeft"),  // Accidental bracket, left
    GlyphData(0xE26D, "accidentalBracketRight"), // Accidental bracket, right

// Gould arrow quartertone accidentals (24-EDO) (U+E270–U+E27F)
    GlyphData(0xE270, "accidentalQuarterToneFlatArrowUp"),          // Quarter-tone flat"
    GlyphData(0xE271, "accidentalThreeQuarterTonesFlatArrowDown"),  // Three-quarter-tones flat"
    GlyphData(0xE272, "accidentalQuarterToneSharpNaturalArrowUp"),  // Quarter-tone sharp"
    GlyphData(0xE273, "accidentalQuarterToneFlatNaturalArrowDown"), // Quarter-tone flat"
    GlyphData(0xE274, "accidentalThreeQuarterTonesSharpArrowUp"),   // Three-quarter-tones sharp"
    GlyphData(0xE275, "accidentalQuarterToneSharpArrowDown"),       // Quarter-tone sharp"
    GlyphData(0xE276, "accidentalFiveQuarterTonesSharpArrowUp"),    // Five-quarter-tones sharp
    GlyphData(0xE277, "accidentalThreeQuarterTonesSharpArrowDown"), // Three-quarter-tones sharp
    GlyphData(0xE278, "accidentalThreeQuarterTonesFlatArrowUp"),    // Three-quarter-tones flat
    GlyphData(0xE279, "accidentalFiveQuarterTonesFlatArrowDown"),   // Five-quarter-tones flat
    GlyphData(0xE27A, "accidentalArrowUp"),                         // Arrow up (raise by one quarter-tone)
    GlyphData(0xE27B, "accidentalArrowDown"),                       // Arrow down (lower by one quarter-tone)

// Stein-Zimmermann accidentals (24-EDO) (U+E280–U+E28F)
    GlyphData(0xE280, "accidentalQuarterToneFlatStein"),            // Reversed flat (quarter-tone flat) (Stein)
    GlyphData(0xE281, "accidentalThreeQuarterTonesFlatZimmermann"), // Reversed flat and flat (three-quarter-tones flat) (Zimmermann)
    GlyphData(0xE282, "accidentalQuarterToneSharpStein"),           // Half sharp (quarter-tone sharp) (Stein)
    GlyphData(0xE283, "accidentalThreeQuarterTonesSharpStein"),     // One and a half sharps (three-quarter-tones sharp) (Stein)
    GlyphData(0xE284, "accidentalNarrowReversedFlat"),              // Narrow reversed flat(quarter-tone flat)
    GlyphData(0xE285, "accidentalNarrowReversedFlatAndFlat"),       // Narrow reversed flat and flat(three-quarter-tones flat)

// Extended Stein-Zimmermann accidentals (U+E290–U+E29F)
    GlyphData(0xE290, "accidentalReversedFlatArrowUp"),             // Reversed flat with arrow up
    GlyphData(0xE291, "accidentalReversedFlatArrowDown"),           // Reversed flat with arrow down
    GlyphData(0xE292, "accidentalFilledReversedFlatArrowUp"),       // Filled reversed flat with arrow up
    GlyphData(0xE293, "accidentalFilledReversedFlatArrowDown"),     // Filled reversed flat with arrow down
    GlyphData(0xE294, "accidentalReversedFlatAndFlatArrowUp"),      // Reversed flat and flat with arrow up
    GlyphData(0xE295, "accidentalReversedFlatAndFlatArrowDown"),    // Reversed flat and flat with arrow down
    GlyphData(0xE296, "accidentalFilledReversedFlatAndFlat"),       // Filled reversed flat and flat
    GlyphData(0xE297, "accidentalFilledReversedFlatAndFlatArrowUp"),   // Filled reversed flat and flat with arrow up
    GlyphData(0xE298, "accidentalFilledReversedFlatAndFlatArrowDown"), // Filled reversed flat and flat with arrow down
    GlyphData(0xE299, "accidentalHalfSharpArrowUp"),                   // Half sharp with arrow up
    GlyphData(0xE29A, "accidentalHalfSharpArrowDown"),              // Half sharp with arrow down
    GlyphData(0xE29B, "accidentalOneAndAHalfSharpsArrowUp"),        // One and a half sharps with arrow up
    GlyphData(0xE29C, "accidentalOneAndAHalfSharpsArrowDown"),      // One and a half sharps with arrow down

// Sims accidentals (72-EDO) (U+E2A0–U+E2AF)
    GlyphData(0xE2A0, "accidentalSims12Down"),   // 1/12 tone low
    GlyphData(0xE2A1, "accidentalSims6Down"),    // 1/6 tone low
    GlyphData(0xE2A2, "accidentalSims4Down"),    // 1/4 tone low
    GlyphData(0xE2A3, "accidentalSims12Up"),     // 1/12 tone high
    GlyphData(0xE2A4, "accidentalSims6Up"),      // 1/6 tone high
    GlyphData(0xE2A5, "accidentalSims4Up"),      // 1/4 tone high

// Johnston accidentals (just intonation) (U+E2B0–U+E2BF)
    GlyphData(0xE2B0, "accidentalJohnstonPlus"),   // Plus (raise by 81:80)
    GlyphData(0xE2B1, "accidentalJohnstonMinus"),  // Minus (lower by 81:80)
    GlyphData(0xE2B2, "accidentalJohnstonEl"),     // Inverted seven (raise by 36:35)
    GlyphData(0xE2B3, "accidentalJohnstonSeven"),  // Seven (lower by 36:35)
    GlyphData(0xE2B4, "accidentalJohnstonUp"),     // Up arrow (raise by 33:32)
    GlyphData(0xE2B5, "accidentalJohnstonDown"),   // Down arrow (lower by 33:32)
    GlyphData(0xE2B6, "accidentalJohnston13"),     // Thirteen (raise by 65:64)
    GlyphData(0xE2B7, "accidentalJohnston31"),     // Inverted 13 (lower by 65:64)

// Extended Helmholtz-Ellis accidentals (just intonation) (U+E2C0–U+E2FF)
    GlyphData(0xE2C0, "accidentalDoubleFlatOneArrowDown"),   // Double flat lowered by one syntonic comma
    GlyphData(0xE2C1, "accidentalFlatOneArrowDown"),         // Flat lowered by one syntonic comma
    GlyphData(0xE2C2, "accidentalNaturalOneArrowDown"),      // Natural lowered by one syntonic comma
    GlyphData(0xE2C3, "accidentalSharpOneArrowDown"),        // Sharp lowered by one syntonic comma
    GlyphData(0xE2C4, "accidentalDoubleSharpOneArrowDown"),  // Double sharp lowered by one syntonic comma
    GlyphData(0xE2C5, "accidentalDoubleFlatOneArrowUp"),     // Double flat raised by one syntonic comma
    GlyphData(0xE2C6, "accidentalFlatOneArrowUp"),           // Flat raised by one syntonic comma
    GlyphData(0xE2C7, "accidentalNaturalOneArrowUp"),        // Natural raised by one syntonic comma
    GlyphData(0xE2C8, "accidentalSharpOneArrowUp"),          // Sharp raised by one syntonic comma
    GlyphData(0xE2C9, "accidentalDoubleSharpOneArrowUp"),    // Double sharp raised by one syntonic comma
    GlyphData(0xE2CA, "accidentalDoubleFlatTwoArrowsDown"),  // Double flat lowered by two syntonic commas
    GlyphData(0xE2CB, "accidentalFlatTwoArrowsDown"),        // Flat lowered by two syntonic commas
    GlyphData(0xE2CC, "accidentalNaturalTwoArrowsDown"),     // Natural lowered by two syntonic commas
    GlyphData(0xE2CD, "accidentalSharpTwoArrowsDown"),       // Sharp lowered by two syntonic commas
    GlyphData(0xE2CE, "accidentalDoubleSharpTwoArrowsDown"), // Double sharp lowered by two syntonic commas
    GlyphData(0xE2CF, "accidentalDoubleFlatTwoArrowsUp"),    // Double flat raised by two syntonic commas
    GlyphData(0xE2D0, "accidentalFlatTwoArrowsUp"),          // Flat raised by two syntonic commas
    GlyphData(0xE2D1, "accidentalNaturalTwoArrowsUp"),       // Natural raised by two syntonic commas
    GlyphData(0xE2D2, "accidentalSharpTwoArrowsUp"),         // Sharp raised by two syntonic commas
    GlyphData(0xE2D3, "accidentalDoubleSharpTwoArrowsUp"),   // Double sharp raised by two syntonic commas
    GlyphData(0xE2D4, "accidentalDoubleFlatThreeArrowsDown"),  // Double flat lowered by three syntonic commas
    GlyphData(0xE2D5, "accidentalFlatThreeArrowsDown"),        // Flat lowered by three syntonic commas
    GlyphData(0xE2D6, "accidentalNaturalThreeArrowsDown"),     // Natural lowered by three syntonic commas
    GlyphData(0xE2D7, "accidentalSharpThreeArrowsDown"),       // Sharp lowered by three syntonic commas
    GlyphData(0xE2D8, "accidentalDoubleSharpThreeArrowsDown"), // Double sharp lowered by three syntonic commas
    GlyphData(0xE2D9, "accidentalDoubleFlatThreeArrowsUp"),    // Double flat raised by three syntonic commas
    GlyphData(0xE2DA, "accidentalFlatThreeArrowsUp"),          // Flat raised by three syntonic commas
    GlyphData(0xE2DB, "accidentalNaturalThreeArrowsUp"),       // Natural raised by three syntonic commas
    GlyphData(0xE2DC, "accidentalSharpThreeArrowsUp"),         // Sharp raised by three syntonic commas
    GlyphData(0xE2DD, "accidentalDoubleSharpThreeArrowsUp"),   // Double sharp raised by three syntonic commas
    GlyphData(0xE2DE, "accidentalLowerOneSeptimalComma"),      // Lower by one septimal comma
    GlyphData(0xE2DF, "accidentalRaiseOneSeptimalComma"),      // Raise by one septimal comma
    GlyphData(0xE2E0, "accidentalLowerTwoSeptimalCommas"),     // Lower by two septimal commas
    GlyphData(0xE2E1, "accidentalRaiseTwoSeptimalCommas"),     // Raise by two septimal commas
    GlyphData(0xE2E2, "accidentalLowerOneUndecimalQuartertone"),  // Lower by one undecimal quartertone
    GlyphData(0xE2E3, "accidentalRaiseOneUndecimalQuartertone"),  // Raise by one undecimal quartertone
    GlyphData(0xE2E4, "accidentalLowerOneTridecimalQuartertone"), // Lower by one tridecimal quartertone
    GlyphData(0xE2E5, "accidentalRaiseOneTridecimalQuartertone"), // Raise by one tridecimal quartertone
    GlyphData(0xE2E6, "accidentalCombiningLower17Schisma"),    // Combining lower by one 17-limit schisma
    GlyphData(0xE2E7, "accidentalCombiningRaise17Schisma"),    // Combining raise by one 17-limit schisma
    GlyphData(0xE2E8, "accidentalCombiningLower19Schisma"),    // Combining lower by one 19-limit schisma
    GlyphData(0xE2E9, "accidentalCombiningRaise19Schisma"),    // Combining raise by one 19-limit schisma
    GlyphData(0xE2EA, "accidentalCombiningLower23Limit29LimitComma"), // Combining lower by one 23-limit comma or 29-limit comma
    GlyphData(0xE2EB, "accidentalCombiningRaise23Limit29LimitComma"), // Combining raise by one 23-limit comma or 29-limit comma
    GlyphData(0xE2EC, "accidentalCombiningLower31Schisma"),    // Combining lower by one 31-limit schisma
    GlyphData(0xE2ED, "accidentalCombiningRaise31Schisma"),    // Combining raise by one 31-limit schisma
    GlyphData(0xE2EE, "accidentalCombiningOpenCurlyBrace"),    // Combining open curly brace
    GlyphData(0xE2EF, "accidentalCombiningCloseCurlyBrace"),   // Combining close curly brace
    GlyphData(0xE2F0, "accidentalDoubleFlatEqualTempered"),    // Double flat equal tempered semitone
    GlyphData(0xE2F1, "accidentalFlatEqualTempered"),          // Flat equal tempered semitone
    GlyphData(0xE2F2, "accidentalNaturalEqualTempered"),       // Natural equal tempered semitone
    GlyphData(0xE2F3, "accidentalSharpEqualTempered"),         // Sharp equal tempered semitone
    GlyphData(0xE2F4, "accidentalDoubleSharpEqualTempered"),   // Double sharp equal tempered semitone

// Spartan Sagittal single-shaft accidentals (U+E300–U+E30F)
    GlyphData(0xE300, "accSagittal5v7KleismaUp"),        // 5:7 kleisma up
    GlyphData(0xE301, "accSagittal5v7KleismaDown"),      // 5:7 kleisma down
    GlyphData(0xE302, "accSagittal5CommaUp"),            // 5 comma up
    GlyphData(0xE303, "accSagittal5CommaDown"),          // 5 comma down
    GlyphData(0xE304, "accSagittal7CommaUp"),            // 7 comma up
    GlyphData(0xE305, "accSagittal7CommaDown"),          // 7 comma down
    GlyphData(0xE306, "accSagittal25SmallDiesisUp"),     // 25 small diesis up
    GlyphData(0xE307, "accSagittal25SmallDiesisDown"),   // 25 small diesis down
    GlyphData(0xE308, "accSagittal35MediumDiesisUp"),    // 35 medium diesis up
    GlyphData(0xE309, "accSagittal35MediumDiesisDown"),  // 35 medium diesis down
    GlyphData(0xE30A, "accSagittal11MediumDiesisUp"),    // 11 medium diesis up
    GlyphData(0xE30B, "accSagittal11MediumDiesisDown"),  // 11 medium diesis down
    GlyphData(0xE30C, "accSagittal11LargeDiesisUp"),     // 11 large diesis up
    GlyphData(0xE30D, "accSagittal11LargeDiesisDown"),   // 11 large diesis down
    GlyphData(0xE30E, "accSagittal35LargeDiesisUp"),     // 35 large diesis up
    GlyphData(0xE30F, "accSagittal35LargeDiesisDown"),   // 35 large diesis down

// Spartan Sagittal multi-shaft accidentals (U+E310–U+E33F)
    GlyphData(0xE310, "accSagittalSharp25SDown"),        // Sharp 25S-down
    GlyphData(0xE311, "accSagittalFlat25SUp"),           // Flat 25S-up
    GlyphData(0xE312, "accSagittalSharp7CDown"),         // Sharp 7C-down
    GlyphData(0xE313, "accSagittalFlat7CUp"),            // Flat 7C-up
    GlyphData(0xE314, "accSagittalSharp5CDown"),         // Sharp 5C-down
    GlyphData(0xE315, "accSagittalFlat5CUp"),            // Flat 5C-up
    GlyphData(0xE316, "accSagittalSharp5v7kDown"),       // Sharp 5:7k-down
    GlyphData(0xE317, "accSagittalFlat5v7kUp"),          // Flat 5:7k-up
    GlyphData(0xE318, "accSagittalSharp"),               // Sharp
    GlyphData(0xE319, "accSagittalFlat"),                // Flat
    GlyphData(0xE31A, "accSagittalUnused1"),             // Unused
    GlyphData(0xE31B, "accSagittalUnused2"),             // Unused
    GlyphData(0xE31C, "accSagittalSharp5v7kUp"),         // Sharp 5:7k-up
    GlyphData(0xE31D, "accSagittalFlat5v7kDown"),        // Flat 5:7k-down
    GlyphData(0xE31E, "accSagittalSharp5CUp"),           // Sharp 5C-up
    GlyphData(0xE31F, "accSagittalFlat5CDown"),          // Flat 5C-down
    GlyphData(0xE320, "accSagittalSharp7CUp"),           // Sharp 7C-up
    GlyphData(0xE321, "accSagittalFlat7CDown"),          // Flat 7C-down
    GlyphData(0xE322, "accSagittalSharp25SUp"),          // Sharp 25S-up
    GlyphData(0xE323, "accSagittalFlat25SDown"),         // Flat 25S-down
    GlyphData(0xE324, "accSagittalSharp35MUp"),          // Sharp 35M-up
    GlyphData(0xE325, "accSagittalFlat35MDown"),         // Flat 35M-down
    GlyphData(0xE326, "accSagittalSharp11MUp"),          // Sharp 11M-up
    GlyphData(0xE327, "accSagittalFlat11MDown"),         // Flat 11M-down
    GlyphData(0xE328, "accSagittalSharp11LUp"),          // Sharp 11L-up
    GlyphData(0xE329, "accSagittalFlat11LDown"),         // Flat 11L-down
    GlyphData(0xE32A, "accSagittalSharp35LUp"),          // Sharp 35L-up
    GlyphData(0xE32B, "accSagittalFlat35LDown"),         // Flat 35L-down
    GlyphData(0xE32C, "accSagittalDoubleSharp25SDown"),  // Double sharp 25S-down
    GlyphData(0xE32D, "accSagittalDoubleFlat25SUp"),     // Double flat 25S-up
    GlyphData(0xE32E, "accSagittalDoubleSharp7CDown"),   // Double sharp 7C-down
    GlyphData(0xE32F, "accSagittalDoubleFlat7CUp"),      // Double flat 7C-up
    GlyphData(0xE330, "accSagittalDoubleSharp5CDown"),   // Double sharp 5C-down
    GlyphData(0xE331, "accSagittalDoubleFlat5CUp"),      // Double flat 5C-up
    GlyphData(0xE332, "accSagittalDoubleSharp5v7kDown"), // Double sharp 5:7k-down
    GlyphData(0xE333, "accSagittalDoubleFlat5v7kUp"),    // Double flat 5:7k-up
    GlyphData(0xE334, "accSagittalDoubleSharp"),         // Double sharp
    GlyphData(0xE335, "accSagittalDoubleFlat"),          // Double flat

// Athenian Sagittal extension (medium precision) accidentals (U+E340–U+E36F)
    GlyphData(0xE340, "accSagittal7v11KleismaUp"),       // 7:11 kleisma up
    GlyphData(0xE341, "accSagittal7v11KleismaDown"),     // 7:11 kleisma down
    GlyphData(0xE342, "accSagittal17CommaUp"),           // 17 comma up
    GlyphData(0xE343, "accSagittal17CommaDown"),         // 17 comma down
    GlyphData(0xE344, "accSagittal55CommaUp"),           // 55 comma up
    GlyphData(0xE345, "accSagittal55CommaDown"),         // 55 comma down
    GlyphData(0xE346, "accSagittal7v11CommaUp"),         // 7:11 comma up
    GlyphData(0xE347, "accSagittal7v11CommaDown"),       // 7:11 comma down
    GlyphData(0xE348, "accSagittal5v11SmallDiesisUp"),   // 5:11 small diesis up
    GlyphData(0xE349, "accSagittal5v11SmallDiesisDown"), // 5:11 small diesis down
    GlyphData(0xE34A, "accSagittalSharp5v11SDown"),      // Sharp 5:11S-down
    GlyphData(0xE34B, "accSagittalFlat5v11SUp"),         // Flat 5:11S-up
    GlyphData(0xE34C, "accSagittalSharp7v11CDown"),      // Sharp 7:11C-down
    GlyphData(0xE34D, "accSagittalFlat7v11CUp"),         // Flat 7:11C-up
    GlyphData(0xE34E, "accSagittalSharp55CDown"),        // Sharp 55C-down
    GlyphData(0xE34F, "accSagittalFlat55CUp"),           // Flat 55C-up
    GlyphData(0xE350, "accSagittalSharp17CDown"),        // Sharp 17C-down
    GlyphData(0xE351, "accSagittalFlat17CUp"),           // Flat 17C-up
    GlyphData(0xE352, "accSagittalSharp7v11kDown"),      // Sharp 7:11k-down
    GlyphData(0xE353, "accSagittalFlat7v11kUp"),         // Flat 7:11k-up
    GlyphData(0xE354, "accSagittalSharp7v11kUp"),        // Sharp 7:11k-up
    GlyphData(0xE355, "accSagittalFlat7v11kDown"),       // Flat 7:11k-down
    GlyphData(0xE356, "accSagittalSharp17CUp"),          // Sharp 17C-up
    GlyphData(0xE357, "accSagittalFlat17CDown"),         // Flat 17C-down
    GlyphData(0xE358, "accSagittalSharp55CUp"),          // Sharp 55C-up
    GlyphData(0xE359, "accSagittalFlat55CDown"),         // Flat 55C-down
    GlyphData(0xE35A, "accSagittalSharp7v11CUp"),        // Sharp 7:11C-up
    GlyphData(0xE35B, "accSagittalFlat7v11CDown"),       // Flat 7:11C-down
    GlyphData(0xE35C, "accSagittalSharp5v11SUp"),        // Sharp 5:11S-up
    GlyphData(0xE35D, "accSagittalFlat5v11SDown"),       // Flat 5:11S-down
    GlyphData(0xE35E, "accSagittalDoubleSharp5v11SDown"), // Double sharp 5:11S-down
    GlyphData(0xE35F, "accSagittalDoubleFlat5v11SUp"),    // Double flat 5:11S-up
    GlyphData(0xE360, "accSagittalDoubleSharp7v11CDown"), // Double sharp 7:11C-down
    GlyphData(0xE361, "accSagittalDoubleFlat7v11CUp"),    // Double flat 7:11C-up
    GlyphData(0xE362, "accSagittalDoubleSharp55CDown"),   // Double sharp 55C-down
    GlyphData(0xE363, "accSagittalDoubleFlat55CUp"),      // Double flat 55C-up
    GlyphData(0xE364, "accSagittalDoubleSharp17CDown"),   // Double sharp 17C-down
    GlyphData(0xE365, "accSagittalDoubleFlat17CUp"),      // Double flat 17C-up
    GlyphData(0xE366, "accSagittalDoubleSharp7v11kDown"), // Double sharp 7:11k-down
    GlyphData(0xE367, "accSagittalDoubleFlat7v11kUp"),    // Double flat 7:11k-up

// Trojan Sagittal extension (12-EDO relative) accidentals (U+E370–U+E38F)
    GlyphData(0xE370, "accSagittal23CommaUp"),            // 23 comma up
    GlyphData(0xE371, "accSagittal23CommaDown"),          // 23 comma down
    GlyphData(0xE372, "accSagittal5v19CommaUp"),          // 5:19 comma up
    GlyphData(0xE373, "accSagittal5v19CommaDown"),        // 5:19 comma down
    GlyphData(0xE374, "accSagittal5v23SmallDiesisUp"),    // 5:23 small diesis up
    GlyphData(0xE375, "accSagittal5v23SmallDiesisDown"),  // 5:23 small diesis down
    GlyphData(0xE376, "accSagittalSharp5v23SDown"),       // Sharp 5:23S-down
    GlyphData(0xE377, "accSagittalFlat5v23SUp"),          // Flat 5:23S-up
    GlyphData(0xE378, "accSagittalSharp5v19CDown"),       // Sharp 5:19C-down
    GlyphData(0xE379, "accSagittalFlat5v19CUp"),          // Flat 5:19C-up
    GlyphData(0xE37A, "accSagittalSharp23CDown"),         // Sharp 23C-down
    GlyphData(0xE37B, "accSagittalFlat23CUp"),            // Flat 23C-up
    GlyphData(0xE37C, "accSagittalSharp23CUp"),           // Sharp 23C-up
    GlyphData(0xE37D, "accSagittalFlat23CDown"),          // Flat 23C-down
    GlyphData(0xE37E, "accSagittalSharp5v19CUp"),         // Sharp 5:19C-up
    GlyphData(0xE37F, "accSagittalFlat5v19CDown"),        // Flat 5:19C-down
    GlyphData(0xE380, "accSagittalSharp5v23SUp"),         // Sharp 5:23S-up
    GlyphData(0xE381, "accSagittalFlat5v23SDown"),        // Flat 5:23S-down
    GlyphData(0xE382, "accSagittalDoubleSharp5v23SDown"), // Double sharp 5:23S-down
    GlyphData(0xE383, "accSagittalDoubleFlat5v23SUp"),    // Double flat 5:23S-up
    GlyphData(0xE384, "accSagittalDoubleSharp5v19CDown"), // Double sharp 5:19C-down
    GlyphData(0xE385, "accSagittalDoubleFlat5v19CUp"),    // Double flat 5:19C-up
    GlyphData(0xE386, "accSagittalDoubleSharp23CDown"),   // Double sharp 23C-down
    GlyphData(0xE387, "accSagittalDoubleFlat23CUp"),      // Double flat 23C-up

// Promethean Sagittal extension (high precision) single-shaft accidentals (U+E390–U+E3AF)
    GlyphData(0xE390, "accSagittal19SchismaUp"),           // 19 schisma up
    GlyphData(0xE391, "accSagittal19SchismaDown"),         // 19 schisma down
    GlyphData(0xE392, "accSagittal17KleismaUp"),           // 17 kleisma up
    GlyphData(0xE393, "accSagittal17KleismaDown"),         // 17 kleisma down
    GlyphData(0xE394, "accSagittal143CommaUp"),            // 143 comma up
    GlyphData(0xE395, "accSagittal143CommaDown"),          // 143 comma down
    GlyphData(0xE396, "accSagittal11v49CommaUp"),          // 11:49 comma up
    GlyphData(0xE397, "accSagittal11v49CommaDown"),        // 11:49 comma down
    GlyphData(0xE398, "accSagittal19CommaUp"),             // 19 comma up
    GlyphData(0xE399, "accSagittal19CommaDown"),           // 19 comma down
    GlyphData(0xE39A, "accSagittal7v19CommaUp"),           // 7:19 comma up
    GlyphData(0xE39B, "accSagittal7v19CommaDown"),         // 7:19 comma down
    GlyphData(0xE39C, "accSagittal49SmallDiesisUp"),       // 49 small diesis up
    GlyphData(0xE39D, "accSagittal49SmallDiesisDown"),     // 49 small diesis down
    GlyphData(0xE39E, "accSagittal23SmallDiesisUp"),       // 23 small diesis up
    GlyphData(0xE39F, "accSagittal23SmallDiesisDown"),     // 23 small diesis down
    GlyphData(0xE3A0, "accSagittal5v13MediumDiesisUp"),    // 5:13 medium diesis up
    GlyphData(0xE3A1, "accSagittal5v13MediumDiesisDown"),  // 5:13 medium diesis down
    GlyphData(0xE3A2, "accSagittal11v19MediumDiesisUp"),   // 11:19 medium diesis up
    GlyphData(0xE3A3, "accSagittal11v19MediumDiesisDown"), // 11:19 medium diesis down
    GlyphData(0xE3A4, "accSagittal49MediumDiesisUp"),      // 49 medium diesis up
    GlyphData(0xE3A5, "accSagittal49MediumDiesisDown"),    // 49 medium diesis down
    GlyphData(0xE3A6, "accSagittal5v49MediumDiesisUp"),    // 5:49 medium diesis up
    GlyphData(0xE3A7, "accSagittal5v49MediumDiesisDown"),  // 5:49 medium diesis down
    GlyphData(0xE3A8, "accSagittal49LargeDiesisUp"),       // 49 large diesis up
    GlyphData(0xE3A9, "accSagittal49LargeDiesisDown"),     // 49 large diesis down
    GlyphData(0xE3AA, "accSagittal11v19LargeDiesisUp"),    // 11:19 large diesis up
    GlyphData(0xE3AB, "accSagittal11v19LargeDiesisDown"),  // 11:19 large diesis down
    GlyphData(0xE3AC, "accSagittal5v13LargeDiesisUp"),     // 5:13 large diesis up
    GlyphData(0xE3AD, "accSagittal5v13LargeDiesisDown"),   // 5:13 large diesis down

// Promethean Sagittal extension (high precision) multi-shaft accidentals (U+E3B0–U+E3EF)
    GlyphData(0xE3B0, "accSagittalSharp23SDown"),    // Sharp 23S-down
    GlyphData(0xE3B1, "accSagittalFlat23SUp"),       // Flat 23S-up
    GlyphData(0xE3B2, "accSagittalSharp49SDown"),    // Sharp 49S-down
    GlyphData(0xE3B3, "accSagittalFlat49SUp"),       // Flat 49S-up
    GlyphData(0xE3B4, "accSagittalSharp7v19CDown"),  // Sharp 7:19C-down
    GlyphData(0xE3B5, "accSagittalFlat7v19CUp"),     // Flat 7:19C-up
    GlyphData(0xE3B6, "accSagittalSharp19CDown"),    // Sharp 19C-down
    GlyphData(0xE3B7, "accSagittalFlat19CUp"),       // Flat 19C-up
    GlyphData(0xE3B8, "accSagittalSharp11v49CDown"), // Sharp 11:49C-down
    GlyphData(0xE3B9, "accSagittalFlat11v49CUp"),    // Flat 11:49C-up
    GlyphData(0xE3BA, "accSagittalSharp143CDown"),   // Sharp 143C-down
    GlyphData(0xE3BB, "accSagittalFlat143CUp"),      // Flat 143C-up
    GlyphData(0xE3BC, "accSagittalSharp17kDown"),    // Sharp 17k-down
    GlyphData(0xE3BD, "accSagittalFlat17kUp"),       // Flat 17k-up
    GlyphData(0xE3BE, "accSagittalSharp19sDown"),    // Sharp 19s-down
    GlyphData(0xE3BF, "accSagittalFlat19sUp"),       // Flat 19s-up
    GlyphData(0xE3C0, "accSagittalSharp19sUp"),      // Sharp 19s-up
    GlyphData(0xE3C1, "accSagittalFlat19sDown"),     // Flat 19s-down
    GlyphData(0xE3C2, "accSagittalSharp17kUp"),      // Sharp 17k-up
    GlyphData(0xE3C3, "accSagittalFlat17kDown"),     // Flat 17k-down
    GlyphData(0xE3C4, "accSagittalSharp143CUp"),     // Sharp 143C-up
    GlyphData(0xE3C5, "accSagittalFlat143CDown"),    // Flat 143C-down
    GlyphData(0xE3C6, "accSagittalSharp11v49CUp"),   // Sharp 11:49C-up
    GlyphData(0xE3C7, "accSagittalFlat11v49CDown"),  // Flat 11:49C-down
    GlyphData(0xE3C8, "accSagittalSharp19CUp"),      // Sharp 19C-up
    GlyphData(0xE3C9, "accSagittalFlat19CDown"),     // Flat 19C-down
    GlyphData(0xE3CA, "accSagittalSharp7v19CUp"),    // Sharp 7:19C-up
    GlyphData(0xE3CB, "accSagittalFlat7v19CDown"),   // Flat 7:19C-down
    GlyphData(0xE3CC, "accSagittalSharp49SUp"),      // Sharp 49S-up
    GlyphData(0xE3CD, "accSagittalFlat49SDown"),     // Flat 49S-down
    GlyphData(0xE3CE, "accSagittalSharp23SUp"),      // Sharp 23S-up
    GlyphData(0xE3CF, "accSagittalFlat23SDown"),     // Flat 23S-down
    GlyphData(0xE3D0, "accSagittalSharp5v13MUp"),    // Sharp 5:13M-up
    GlyphData(0xE3D1, "accSagittalFlat5v13MDown"),   // Flat 5:13M-down
    GlyphData(0xE3D2, "accSagittalSharp11v19MUp"),   // Sharp 11:19M-up
    GlyphData(0xE3D3, "accSagittalFlat11v19MDown"),  // Flat 11:19M-down
    GlyphData(0xE3D4, "accSagittalSharp49MUp"),      // Sharp 49M-up
    GlyphData(0xE3D5, "accSagittalFlat49MDown"),     // Flat 49M-down
    GlyphData(0xE3D6, "accSagittalSharp5v49MUp"),    // Sharp 5:49M-up
    GlyphData(0xE3D7, "accSagittalFlat5v49MDown"),   // Flat 5:49M-down
    GlyphData(0xE3D8, "accSagittalSharp49LUp"),      // Sharp 49L-up
    GlyphData(0xE3D9, "accSagittalFlat49LDown"),     // Flat 49L-down
    GlyphData(0xE3DA, "accSagittalSharp11v19LUp"),   // Sharp 11:19L-up
    GlyphData(0xE3DB, "accSagittalFlat11v19LDown"),  // Flat 11:19L-down
    GlyphData(0xE3DC, "accSagittalSharp5v13LUp"),    // Sharp 5:13L-up
    GlyphData(0xE3DD, "accSagittalFlat5v13LDown"),   // Flat 5:13L-down
    GlyphData(0xE3DE, "accSagittalUnused3"),         // Unused
    GlyphData(0xE3DF, "accSagittalUnused4"),         // Unused
    GlyphData(0xE3E0, "accSagittalDoubleSharp23SDown"),    // Double sharp 23S-down
    GlyphData(0xE3E1, "accSagittalDoubleFlat23SUp"),       // Double flat 23S-up
    GlyphData(0xE3E2, "accSagittalDoubleSharp49SDown"),    // Double sharp 49S-down
    GlyphData(0xE3E3, "accSagittalDoubleFlat49SUp"),       // Double flat 49S-up
    GlyphData(0xE3E4, "accSagittalDoubleSharp7v19CDown"),  // Double sharp 7:19C-down
    GlyphData(0xE3E5, "accSagittalDoubleFlat7v19CUp"),     // Double flat 7:19C-up
    GlyphData(0xE3E6, "accSagittalDoubleSharp19CDown"),    // Double sharp 19C-down
    GlyphData(0xE3E7, "accSagittalDoubleFlat19CUp"),       // Double flat 19C-up
    GlyphData(0xE3E8, "accSagittalDoubleSharp11v49CDown"), // Double sharp 11:49C-down
    GlyphData(0xE3E9, "accSagittalDoubleFlat11v49CUp"),    // Double flat 11:49C-up
    GlyphData(0xE3EA, "accSagittalDoubleSharp143CDown"),   // Double sharp 143C-down
    GlyphData(0xE3EB, "accSagittalDoubleFlat143CUp"),      // Double flat 143C-up
    GlyphData(0xE3EC, "accSagittalDoubleSharp17kDown"),    // Double sharp 17k-down
    GlyphData(0xE3ED, "accSagittalDoubleFlat17kUp"),       // Double flat 17k-up
    GlyphData(0xE3EE, "accSagittalDoubleSharp19sDown"),    // Double sharp 19s-down
    GlyphData(0xE3EF, "accSagittalDoubleFlat19sUp"),       // Double flat 19s-up

// Wyschnegradsky accidentals (72-EDO) (U+E420–U+E43F)
    GlyphData(0xE420, "accidentalWyschnegradsky1TwelfthsSharp"),    // 1/12 tone sharp
    GlyphData(0xE421, "accidentalWyschnegradsky2TwelfthsSharp"),    // 1/6 tone sharp
    GlyphData(0xE422, "accidentalWyschnegradsky3TwelfthsSharp"),    // 1/4 tone sharp
    GlyphData(0xE423, "accidentalWyschnegradsky4TwelfthsSharp"),    // 1/3 tone sharp
    GlyphData(0xE424, "accidentalWyschnegradsky5TwelfthsSharp"),    // 5/12 tone sharp
    GlyphData(0xE425, "accidentalWyschnegradsky6TwelfthsSharp"),    // 1/2 tone sharp
    GlyphData(0xE426, "accidentalWyschnegradsky7TwelfthsSharp"),    // 7/12 tone sharp
    GlyphData(0xE427, "accidentalWyschnegradsky8TwelfthsSharp"),    // 2/3 tone sharp
    GlyphData(0xE428, "accidentalWyschnegradsky9TwelfthsSharp"),    // 3/4 tone sharp
    GlyphData(0xE429, "accidentalWyschnegradsky10TwelfthsSharp"),   // 5/6 tone sharp
    GlyphData(0xE42A, "accidentalWyschnegradsky11TwelfthsSharp"),   // 11/12 tone sharp
    GlyphData(0xE42B, "accidentalWyschnegradsky1TwelfthsFlat"),     // 1/12 tone flat
    GlyphData(0xE42C, "accidentalWyschnegradsky2TwelfthsFlat"),     // 1/6 tone flat
    GlyphData(0xE42D, "accidentalWyschnegradsky3TwelfthsFlat"),     // 1/4 tone flat
    GlyphData(0xE42E, "accidentalWyschnegradsky4TwelfthsFlat"),     // 1/3 tone flat
    GlyphData(0xE42F, "accidentalWyschnegradsky5TwelfthsFlat"),     // 5/12 tone flat
    GlyphData(0xE430, "accidentalWyschnegradsky6TwelfthsFlat"),     // 1/2 tone flat
    GlyphData(0xE431, "accidentalWyschnegradsky7TwelfthsFlat"),     // 7/12 tone flat
    GlyphData(0xE432, "accidentalWyschnegradsky8TwelfthsFlat"),     // 2/3 tone flat
    GlyphData(0xE433, "accidentalWyschnegradsky9TwelfthsFlat"),     // 3/4 tone flat
    GlyphData(0xE434, "accidentalWyschnegradsky10TwelfthsFlat"),    // 5/6 tone flat
    GlyphData(0xE435, "accidentalWyschnegradsky11TwelfthsFlat"),    // 11/12 tone flat

// Arel-Ezgi-Uzdilek (AEU) accidentals (U+E440–U+E44F)
    GlyphData(0xE440, "accidentalBuyukMucennebFlat"),    // Büyük mücenneb (flat)
    GlyphData(0xE441, "accidentalKucukMucennebFlat"),    // Küçük mücenneb (flat)
    GlyphData(0xE442, "accidentalBakiyeFlat"),           // Bakiye (flat)
    GlyphData(0xE443, "accidentalKomaFlat"),             // Koma (flat)
    GlyphData(0xE444, "accidentalKomaSharp"),            // Koma (sharp)
    GlyphData(0xE445, "accidentalBakiyeSharp"),          // Bakiye (sharp)
    GlyphData(0xE446, "accidentalKucukMucennebSharp"),   // Küçük mücenneb (sharp)
    GlyphData(0xE447, "accidentalBuyukMucennebSharp"),   // Büyük mücenneb (sharp)

// Turkish folk music accidentals (U+E450–U+E45F)
    GlyphData(0xE450, "accidental1CommaSharp"),   // 1-comma sharp
    GlyphData(0xE451, "accidental2CommaSharp"),   // 2-comma sharp
    GlyphData(0xE452, "accidental3CommaSharp"),   // 3-comma sharp
    GlyphData(0xE453, "accidental5CommaSharp"),   // 5-comma sharp
    GlyphData(0xE454, "accidental1CommaFlat"),    // 1-comma flat
    GlyphData(0xE455, "accidental2CommaFlat"),    // 2-comma flat
    GlyphData(0xE456, "accidental3CommaFlat"),    // 3-comma flat
    GlyphData(0xE457, "accidental4CommaFlat"),    // 4-comma flat

// Persian accidentals (U+E460–U+E46F)
    GlyphData(0xE460, "accidentalKoron"),   // Koron (quarter tone flat)
    GlyphData(0xE461, "accidentalSori"),    // Sori (quarter tone sharp)

//Other accidentals (U+E470–U+E49F)
    GlyphData(0xE470, "accidentalXenakisOneThirdToneSharp"),      // One-third-tone sharp (Xenakis)
    GlyphData(0xE471, "accidentalXenakisTwoThirdTonesSharp"),     // Two-third-tones sharp (Xenakis)
    GlyphData(0xE472, "accidentalQuarterToneSharpBusotti"),       // Quarter tone sharp (Bussotti)
    GlyphData(0xE473, "accidentalSharpOneHorizontalStroke"),      // One or three quarter tones sharp
    GlyphData(0xE474, "accidentalThreeQuarterTonesSharpBusotti"), // Three quarter tones sharp (Bussotti)
    GlyphData(0xE475, "accidentalQuarterToneSharpWiggle"),        // Quarter tone sharp with wiggly tail
    GlyphData(0xE476, "accidentalTavenerSharp"),                  // Byzantine-style Bu\u0308yu\u0308k mu\u0308cenneb sharp (Tavener)
    GlyphData(0xE477, "accidentalTavenerFlat"),                   // Byzantine-style Bakiye flat (Tavener)
    GlyphData(0xE478, "accidentalQuarterToneFlatPenderecki"),     // Quarter tone flat (Penderecki)
    GlyphData(0xE479, "accidentalCommaSlashUp"),                  // Syntonic/Didymus comma (80:81) up (Bosanquet)
    GlyphData(0xE47A, "accidentalCommaSlashDown"),                // Syntonic/Didymus comma (80:81) down (Bosanquet)
    GlyphData(0xE47B, "accidentalWilsonPlus"),                    // Wilson plus (5 comma up)
    GlyphData(0xE47C, "accidentalWilsonMinus"),                   // Wilson minus (5 comma down)
    GlyphData(0xE47D, "accidentalLargeDoubleSharp"),              // Large double sharp
    GlyphData(0xE47E, "accidentalQuarterToneSharp4"),             // Quarter-tone sharp"
    GlyphData(0xE47F, "accidentalQuarterToneFlat4"),              // Quarter-tone flat"
    GlyphData(0xE480, "accidentalQuarterToneFlatFilledReversed"), // Filled reversed flat (quarter-tone flat)
    GlyphData(0xE481, "accidentalSharpReversed"),                 // Reversed sharp
    GlyphData(0xE482, "accidentalNaturalReversed"),               // Reversed natural
    GlyphData(0xE483, "accidentalDoubleFlatReversed"),            // Reversed double flat
    GlyphData(0xE484, "accidentalFlatTurned"),                    // Turned flat
    GlyphData(0xE485, "accidentalDoubleFlatTurned"),              // Turned double flat
    GlyphData(0xE486, "accidentalThreeQuarterTonesFlatGrisey"),   // Three-quarter-tones flat (Grisey)
    GlyphData(0xE487, "accidentalThreeQuarterTonesFlatTartini"),  // Three-quarter-tones flat (Tartini)
    GlyphData(0xE488, "accidentalQuarterToneFlatVanBlankenburg"), // Quarter-tone flat (van Blankenburg)
    GlyphData(0xE489, "accidentalThreeQuarterTonesFlatCouper"),   // Three-quarter-tones flat (Couper)
    GlyphData(0xE48A, "accidentalOneThirdToneSharpFerneyhough"),  // One-third-tone sharp (Ferneyhough)
    GlyphData(0xE48B, "accidentalOneThirdToneFlatFerneyhough"),   // One-third-tone flat (Ferneyhough)
    GlyphData(0xE48C, "accidentalTwoThirdTonesSharpFerneyhough"), // Two-third-tones sharp (Ferneyhough)
    GlyphData(0xE48D, "accidentalTwoThirdTonesFlatFerneyhough"),  // Two-third-tones flat (Ferneyhough)

//Articulation (U+E4A0 - U+E4BF)
    GlyphData(0xE4A0),  // accent_above,
    GlyphData(0xE4A1),  // accent_below,
    GlyphData(0xE4A2),  // staccato_above,
    GlyphData(0xE4A3),  // staccato_below,
    GlyphData(0xE4A4),  // tenuto_above,
    GlyphData(0xE4A5),  // tenuto_below,
    GlyphData(0xE4A6),  // staccatissimo_above,
    GlyphData(0xE4A7),  // staccatissimo_below,
    GlyphData(0xE4A8),  // staccatissimo_wedge_above,
    GlyphData(0xE4A9),  // staccatissimo_wedge_below,
    GlyphData(0xE4AA),  // staccatissimo_stroke_above,
    GlyphData(0xE4AB),  // staccatissimo_stroke_below,
    GlyphData(0xE4AC),  // marcato_above,
    GlyphData(0xE4AD),  // marcato_below,
    GlyphData(0xE4AE),  // marcato_staccato_above,
    GlyphData(0xE4AF),  // marcato_staccato_below,
    GlyphData(0xE4B0),  // accent_staccato_above,
    GlyphData(0xE4B1),  // accent_staccato_below,
    GlyphData(0xE4B2),  // tenuto_staccato_above,
    GlyphData(0xE4B3),  // tenuto_staccato_below,
    GlyphData(0xE4B4),  // tenuto_accent_above,
    GlyphData(0xE4B5),  // tenuto_accent_below,
    GlyphData(0xE4B6),  // stress_above,
    GlyphData(0xE4B7),  // stress_below,
    GlyphData(0xE4B8),  // unstress_above,
    GlyphData(0xE4B9),  // unstress_below,
    GlyphData(0xE4BA),  // laissez_vibrer_above,
    GlyphData(0xE4BB),  // laissez_vibrer_below,
    GlyphData(0xE4BC),  // marcato_tenuto_above,
    GlyphData(0xE4BD),  // marcato_tenuto_below,

//Holds and pauses (U+E4C0 - U+E4DF)
    GlyphData(0xE4C0),  // Fermata above (arch)
    GlyphData(0xE4C1),  // Fermata below (arch)
    GlyphData(0xE4C2),  // Very short fermata above
    GlyphData(0xE4C3),  // Very short fermata below
    GlyphData(0xE4C4),  // Fermata above (angled)
    GlyphData(0xE4C5),  // Fermata below (angled)
    GlyphData(0xE4C6),  // Fermata above (square)
    GlyphData(0xE4C7),  // Fermata below (square)
    GlyphData(0xE4C8),  // Very long fermata above
    GlyphData(0xE4C9),  // Very long fermata below
    GlyphData(0xE4CA),  // Long fermata (Henze) above
    GlyphData(0xE4CB),  // Long fermata (Henze) below
    GlyphData(0xE4CC),  // Short fermata (Henze) above
    GlyphData(0xE4CD),  // Short fermata (Henze) below
    GlyphData(0xE4CE),  // breath-mark (comma)
    GlyphData(0xE4CF),  // breath-mark (tick)
    GlyphData(0xE4D0),  // breath-mark (V)
    GlyphData(0xE4D1),  // caesura
    GlyphData(0xE4D2),  // Thick caesura
    GlyphData(0xE4D3),  // Short caesura
    GlyphData(0xE4D4),  // Curved caesura
    GlyphData(0xE4D5),  // Breath mark (Salzedo)
    GlyphData(0xE4D6),  // Curlew (Britten)

//Rests (U+E4E0 - U+E4FF)
    GlyphData(0xE4E1),  // Longa rest     //larga
    GlyphData(0xE4E2),  // Breve rest, Double Whole   //breve, cuadrada
    GlyphData(0xE4E3),  // Whole rest    //whole redonda
    GlyphData(0xE4E4),  // Half rest     //half blanca
    GlyphData(0xE4E5),  // Quarter rest  //quarter negra
    GlyphData(0xE4E6),  // Eight rest    //eighth corchea
    GlyphData(0xE4E7),  // 16th rest     //16th semicorchea
    GlyphData(0xE4E8),  // 32nd rest     //32nd fusa
    GlyphData(0xE4E9),  // 64th rest     //64th semifusa
    GlyphData(0xE4EA),  // 128th rest    //128th garrapatea
    GlyphData(0xE4EB),  // 256th rest    //256th semigarrapatea
    GlyphData(0xE4EC),  // 512th rest
    GlyphData(0xE4ED),  // 1024th rest

//Octaves (U+E510 - U+E51F)
    GlyphData(0xE510),  //Ottava: 8
    GlyphData(0xE511),  //Ottava alta: 8va ('va' at top)
    GlyphData(0xE512),  //Ottava bassa: 8va ('va' at bottom)
    GlyphData(0xE513),  //Ottava bassa: 8ba
    GlyphData(0xE514),  //Quindicesima: 15
    GlyphData(0xE515),  //Quindicesima alta: 15ma ('ma' at top)
    GlyphData(0xE516),  //Quindicesima bassa: 15ma ('ma' at bottom)
    GlyphData(0xE517),  //Ventiduesima: 22
    GlyphData(0xE518),  //Ventiduesima alta: 22ma ('ma' at top)
    GlyphData(0xE519),  //Ventiduesima bassa: 22ma ('ma' at bottom)
    GlyphData(0xE51A),  //Left parenthesis for octave signs
    GlyphData(0xE51B),  //Right parenthesis for octave signs
    GlyphData(0xE51C),  //Ottava bassa: 8vb
    GlyphData(0xE51D),  //Quindicesima bassa: 15mb
    GlyphData(0xE51E),  //Ventiduesima bassa: 22mb
    GlyphData(0xE51F),  //'bassa' word

//Dynamics (U+E520 - U+E54F)
    GlyphData(0xE520),  // dynamic Piano                p
    GlyphData(0xE521),  // dynamic Mezzo                m
    GlyphData(0xE522),  // dynamic Forte                f
    GlyphData(0xE523),  // dynamic Rinforzando          r
    GlyphData(0xE524),  // dynamic Sforzando            s
    GlyphData(0xE525),  // dynamic Z                    z
    GlyphData(0xE526),  // dynamic Niente               n
    GlyphData(0xE527),  // dynamic PPPPPP               pppppp
    GlyphData(0xE528),  // dynamic PPPPP                ppppp
    GlyphData(0xE529),  // dynamic PPPP                 pppp
    GlyphData(0xE52A),  // dynamic PPP                  ppp
    GlyphData(0xE52B),  // dynamic PP                   pp
    GlyphData(0xE52C),  // dynamic MP                   mp
    GlyphData(0xE52D),  // dynamic MF                   mf
    GlyphData(0xE52E),  // dynamic PF                   pf
    GlyphData(0xE52F),  // dynamic FF                   ff
    GlyphData(0xE530),  // dynamic FFF                  fff
    GlyphData(0xE531),  // dynamic FFFF                 ffff
    GlyphData(0xE532),  // dynamic FFFFF                fffff
    GlyphData(0xE533),  // dynamic FFFFFF               ffffff
    GlyphData(0xE534),  // dynamic FortePiano           fp
    GlyphData(0xE535),  // dynamic Forzando             fz
    GlyphData(0xE536),  // dynamic Sforzando1           sf
    GlyphData(0xE537),  // dynamic SforzandoPiano       sfp
    GlyphData(0xE538),  // dynamic SforzandoPianissimo  sfpp
    GlyphData(0xE539),  // dynamic Sforzato             sfz
    GlyphData(0xE53A),  // dynamic SforzatoPiano        sfzp
    GlyphData(0xE53B),  // dynamic SforzatoFF           sffz
    GlyphData(0xE53C),  // dynamic Rinforzando1         rf
    GlyphData(0xE53D),  // dynamic Rinforzando2         rfz

//Common ornaments (U+E560 - U+E56F)
    GlyphData(0xE560),  // Slashed grace note stem up
    GlyphData(0xE561),  // Slashed grace note stem down
    GlyphData(0xE562),  // Grace note stem up
    GlyphData(0xE563),  // Grace note stem down
    GlyphData(0xE564),  // Slash for stem up grace note
    GlyphData(0xE565),  // Slash for stem down grace note
    GlyphData(0xE566),  // Trill
    GlyphData(0xE567),  // Turn
    GlyphData(0xE568),  // Inverted Turn
    GlyphData(0xE569),  // Turn with slash
    GlyphData(0xE56A),  // Turn up
    GlyphData(0xE56B),  // Inverted turn up
    GlyphData(0xE56C),  // Mordent
    GlyphData(0xE56D),  // Inverted mordent
    GlyphData(0xE56E),  // Tremblement
    GlyphData(0xE56F),  // Haydn ornament

//Other baroque ornaments (U+E570 - U+E58F)
    GlyphData(0xE587),  // Schleifer

//Brass techniques (U+E5D0 - U+E5EF)
    GlyphData(0xE5D0),  // Scoop
    GlyphData(0xE5D1),  // Lift, short
    GlyphData(0xE5D2),  // Lift, medium
    GlyphData(0xE5D3),  // Lift, long
    GlyphData(0xE5D4),  // Doit, short
    GlyphData(0xE5D5),  // Doit, medium
    GlyphData(0xE5D6),  // Doit, long
    GlyphData(0xE5D7),  // Lip fall, short
    GlyphData(0xE5D8),  // Lip fall, medium
    GlyphData(0xE5D9),  // Lip fall, long
    GlyphData(0xE5DA),  // Smooth fall, short
    GlyphData(0xE5DB),  // Smooth fall, medium
    GlyphData(0xE5DC),  // Smooth fall, long
    GlyphData(0xE5DD),  // Rough fall, short
    GlyphData(0xE5DE),  // Rough fall, medium
    GlyphData(0xE5DF),  // Rough fall, long
    GlyphData(0xE5E0),  // Plop
    GlyphData(0xE5E1),  // Flip
    GlyphData(0xE5E2),  // Smear
    GlyphData(0xE5E3),  // Bend
    GlyphData(0xE5E4),  // Jazz turn
    GlyphData(0xE5E5),  // Muted (closed)
    GlyphData(0xE5E6),  // Half-muted (half-closed)
    GlyphData(0xE5E7),  // Open
    GlyphData(0xE5E8),  // Harmon mute, stem in
    GlyphData(0xE5E9),  // Harmon mute, stem extended, left
    GlyphData(0xE5EA),  // Harmon mute, stem extended, right
    GlyphData(0xE5EB),  // Harmon mute, stem out
    GlyphData(0xE5EC),  // Smooth lift, short
    GlyphData(0xE5ED),  // Smooth lift, medium
    GlyphData(0xE5EE),  // Smooth lift, long

//Wind techniques (U+E5F0 –U+E60F)
    GlyphData(0xE5F0),  // Double-tongue above
    GlyphData(0xE5F1),  // Double-tongue below
    GlyphData(0xE5F2),  // Triple-tongue above
    GlyphData(0xE5F3),  // Triple-tongue below
    GlyphData(0xE5F4),  // Closed hole
    GlyphData(0xE5F5),  // Three-quarters closed hole
    GlyphData(0xE5F6),  // Half-closed hole
    GlyphData(0xE5F7),  // Half-closed hole 2
    GlyphData(0xE5F8),  // Half-open hole
    GlyphData(0xE5F9),  // Open hole
    GlyphData(0xE5FA),  // Trill key
    GlyphData(0xE5FB),  // Flatter embouchure
    GlyphData(0xE5FC),  // Sharper embouchure
    GlyphData(0xE5FD),  // Relaxed embouchure
    GlyphData(0xE5FE),  // Somewhat relaxed embouchure
    GlyphData(0xE5FF),  // Tight embouchure
    GlyphData(0xE600),  // Somewhat tight embouchure
    GlyphData(0xE601),  // Very tight embouchure
    GlyphData(0xE602),  // Very relaxed embouchure / weak air-pressure
    GlyphData(0xE603),  // Very tight embouchure / strong air pressure
    GlyphData(0xE604),  // Normal reed position
    GlyphData(0xE605),  // Very little reed (pull outwards)
    GlyphData(0xE606),  // Much more reed (push inwards)
    GlyphData(0xE607),  // Combining multiphonics (black) for stem
    GlyphData(0xE608),  // Combining multiphonics (white) for stem
    GlyphData(0xE609),  // Combining multiphonics (black and white) for stem

//String techniques (U+E610 - U+E62F)
    GlyphData(0xE610),  // Down bow
    GlyphData(0xE612),  // Up bow
    GlyphData(0xE614),  // Harmonic
    GlyphData(0xE615),  // Half-harmonic

//Keyboard techniques (U+E650 - U+E67F)
    GlyphData(0xE650),  // Pedal mark
    GlyphData(0xE651),  // Pedal P
    GlyphData(0xE655),  // Pedal up mark
    GlyphData(0xE656),  // Half-pedal mark
    GlyphData(0xE657),  // Pedal up notch
    GlyphData(0xE659),  // Sostenuto pedal mark
    GlyphData(0xE65A),  // Pedal S

//Handbells (U+E810 - U+E82F)
    GlyphData(0xE810),  // Martellato
    GlyphData(0xE811),  // Martellato lift
    GlyphData(0xE812),  // Hand martellato
    GlyphData(0xE813),  // Muted martellato
    GlyphData(0xE814),  // Mallet, bell suspended
    GlyphData(0xE815),  // Mallet, bell on table
    GlyphData(0xE816),  // Mallet lift
    GlyphData(0xE817),  // Pluck lift
    GlyphData(0xE818),  // Swing up
    GlyphData(0xE819),  // Swing down
    GlyphData(0xE81A),  // Swing
    GlyphData(0xE81B),  // Echo
    GlyphData(0xE81C),  // Echo 2
    GlyphData(0xE81D),  // Gyro
    GlyphData(0xE81E),  // Damp 3
    GlyphData(0xE81F),  // Belltree
    GlyphData(0xE820),  // Table single handbell
    GlyphData(0xE821),  // Table pair of handbells

//figured bass (U+EA50 - U+EA6F)
    GlyphData(0xEA50),  // Figured bass. Number 0
    GlyphData(0xEA51),  // Figured bass. Number 1
    GlyphData(0xEA52),  // Figured bass. Number 2
    GlyphData(0xEA54),  // Figured bass. Number 3
    GlyphData(0xEA55),  // Figured bass. Number 4
    GlyphData(0xEA57),  // Figured bass. Number 5
    GlyphData(0xEA5B),  // Figured bass. Number 6
    GlyphData(0xEA5D),  // Figured bass. Number 7
    GlyphData(0xEA60),  // Figured bass. Number 8
    GlyphData(0xEA61),  // Figured bass. Number 9
    GlyphData(0xEA66),  // Figured bass. Sharp symbol
    GlyphData(0xEA64),  // GLYPH_FIGURED_BASS_FLAT,                //Flat symbol
    GlyphData(0xEA65),  // GLYPH_FIGURED_BASS_NATURAL,             //Natural symbol
    GlyphData(0xEA6C),  // GLYPH_FIGURED_BASS_PLUS,                // +
    GlyphData(0xEA6D),  // GLYPH_FIGURED_BASS_MINUS,               // -
    GlyphData(0xEA6A),  // GLYPH_FIGURED_BASS_OPEN_PARENTHESIS,    // (
    GlyphData(0xEA6B),  // GLYPH_FIGURED_BASS_CLOSE_PARENTHESIS,   // )
    GlyphData(0xEA5F),  // GLYPH_FIGURED_BASS_7_STRIKED,           // 7 with overlayered /

//Multi-segment lines (U+EAA0 - U+EB0F)
    GlyphData(0xEAA4),  // Trill wiggle segment
    GlyphData(0xEAA9),  // Arpeggiato wiggle segment, upwards
    GlyphData(0xEAAA),  // Arpeggiato wiggle segment, downwards
    GlyphData(0xEAAD),  // Arpeggiato arrowhead up
    GlyphData(0xEAAE),  // Arpeggiato arrowhead down

//Electronic music pictograms (U+EB10 - U+EB5F)
    GlyphData(0xEB1D),  // stop button, used for errors

//Metronome marks (U+ECA0 - U+ECBF)
    GlyphData(0xECA0),  // small longa note
    GlyphData(0xECA2),  // small whole note
    GlyphData(0xECA3),  // small half note up
    GlyphData(0xECA5),  // small quarter note up
    GlyphData(0xECA7),  // small eighth note up
    GlyphData(0xECA9),  // small 16th note up
    GlyphData(0xECAB),  // small 32nd note up
    GlyphData(0xECAD),  // small 64th note up
    GlyphData(0xECAF),  // small 128th note up
    GlyphData(0xECB1),  // small 256th note up
    GlyphData(0xECB7),  // augmentation dot

// Fingering (U+ED10–U+ED2F)
	GlyphData(0xED10, "fingering0"),		    //Fingering 0 (open string)
	GlyphData(0xED11, "fingering1"),		    //Fingering 1 (thumb)
	GlyphData(0xED12, "fingering2"),		    //Fingering 2 (index finger)
	GlyphData(0xED13, "fingering3"),		    //Fingering 3 (middle finger)
	GlyphData(0xED14, "fingering4"),		    //Fingering 4 (ring finger)
	GlyphData(0xED15, "fingering5"),		    //Fingering 5 (little finger)
	GlyphData(0xED16, "fingeringTUpper"),    //Fingering T (left-hand thumb for guitar)
	GlyphData(0xED17, "fingeringPLower"),    //Fingering p (pulgar; right-hand thumb for guitar)
	GlyphData(0xED18, "fingeringTLower"),    //Fingering t (right-hand thumb for guitar)
	GlyphData(0xED19, "fingeringILower"),    //Fingering i (indicio; right-hand index finger for guitar)
	GlyphData(0xED1A, "fingeringMLower"),    //Fingering m (medio; right-hand middle finger for guitar)
	GlyphData(0xED1B, "fingeringALower"),    //Fingering a (anular; right-hand ring finger for guitar)
	GlyphData(0xED1C, "fingeringCLower"),    //Fingering c (right-hand little finger for guitar)
	GlyphData(0xED1D, "fingeringXLower"),    //Fingering x (right-hand little finger for guitar)
	GlyphData(0xED1E, "fingeringELower"),    //Fingering e (right-hand little finger for guitar)
	GlyphData(0xED1F, "fingeringOLower"),    //Fingering o (right-hand little finger for guitar)
	GlyphData(0xED20, "fingeringSubstitutionAbove"),		//Finger substitution above
	GlyphData(0xED21, "fingeringSubstitutionBelow"),		//Finger substitution below
	GlyphData(0xED22, "fingeringSubstitutionDash"),		//Finger substitution dash
	GlyphData(0xED23, "fingeringMultipleNotes"),		    //Multiple notes played by thumb or single finger
	GlyphData(0xED24, "fingering6"),		                //Fingering 6
	GlyphData(0xED25, "fingering7"),		                //Fingering 7
	GlyphData(0xED26, "fingering8"),		                //Fingering 8
	GlyphData(0xED27, "fingering9"),		                //Fingering 9
	GlyphData(0xED28, "fingeringLeftParenthesis"),		//Fingering left parenthesis
	GlyphData(0xED29, "fingeringRightParenthesis"),		//Fingering right parenthesis
	GlyphData(0xED2A, "fingeringLeftBracket"),		    //Fingering left bracket
	GlyphData(0xED2B, "fingeringRightBracket"),		    //Fingering right bracket
	GlyphData(0xED2C, "fingeringSeparatorMiddleDot"),    //Fingering middle dot separator
	GlyphData(0xED2D, "fingeringSeparatorMiddleDotWhite"),   //Fingering white middle dot separator
	GlyphData(0xED2E, "fingeringSeparatorSlash"),		//Fingering forward slash separator 	  	};

};

//=======================================================================================
// MusicGlyphs implementation

MusicGlyphs::MusicGlyphs(LibraryScope* pLibScope)
    : m_pLibScope(pLibScope)
    , m_glyphs(nullptr)
{
    update();
}

//---------------------------------------------------------------------------------------
void MusicGlyphs::update()
{
    if (m_pLibScope->is_music_font_smufl_compliant())
        m_glyphs = &m_glyphs_smufl[0];
}

}  //namespace lomse

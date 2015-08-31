//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2015 Cecilio Salmeron. All rights reserved.
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
//Notheads (U+E0A0 - U+E0FF)
    GlyphData(0xE95D),  // Longa note
    GlyphData(0xE0A0),  // Breve note, Double whole
    GlyphData(0xE0A2),  // Whole note
    GlyphData(0xE0A3),  // Half note
    GlyphData(0xE0A4),  // Quarter note notehead
    GlyphData(0xE0A9),  // Cross notehead

//Individual notes (U+E1D0 - U+E1EF)
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
    GlyphData(0xE4C0),  // Fermata over (arch)
    GlyphData(0xE4C1),  // Fermata under (arch)
    GlyphData(0xE4C6),  // long_fermata_above,
    GlyphData(0xE4C7),  // long_fermata_below,
    GlyphData(0xE4D0),  // V breath-mark

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

//flags for notes (U+E240 - U+E25F)
    GlyphData(0xE241),  // Eight note flag down
    GlyphData(0xE243),  // 16th note flag down
    GlyphData(0xE245),  // 32nd note flag down
    GlyphData(0xE247),  // 64th note flag down
    GlyphData(0xE249),  // 128th note flag down
    GlyphData(0xE24B),  // 256th note flag down
    GlyphData(0xE240),  // Eight note flag up
    GlyphData(0xE242),  // 16th note flag up
    GlyphData(0xE244),  // 32nd note flag up
    GlyphData(0xE246),  // 64th note flag up)),
    GlyphData(0xE248),  // 128th note flag up
    GlyphData(0xE24A),  // 256th note flag up)),

//standard accidentals (U+E260 - U+E26F)
    GlyphData(0xE261),  // Natural accidental
    GlyphData(0xE262),  // Sharp accidental
    GlyphData(0xE260),  // Flat accidental
    GlyphData(0xE263),  // Double sharp accidental
    GlyphData(0xE264),  // Double flat accidental
    GlyphData(0xE26A),  // open_cautionary_accidental,   Accidental parenthesis, left
    GlyphData(0xE26B),  // close_cautionary_accidental   Accidental parenthesis, right

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

//Metronome marks (U+ECA0 - U+ECBF)
    GlyphData(0xECA2),  // small whole note
    GlyphData(0xECA3),  // small half note up
    GlyphData(0xECA5),  // small quarter note up
    GlyphData(0xECA7),  // small eighth note up
    GlyphData(0xECA9),  // small 16th note up
    GlyphData(0xECAB),  // small 32th note up
    GlyphData(0xECAD),  // small 64th note up
    GlyphData(0xECAF),  // small 128th note up
    GlyphData(0xECB1),  // small 256th note up
    GlyphData(0xECB7),  // augmentation dot

//Repeats (U+E040 - U+E04F)
    GlyphData(0xE046),  // Da Capo sign
    GlyphData(0xE045),  // Dal Segno sign
    GlyphData(0xE048),  // Coda sign
    GlyphData(0xE047),  // Segno sign

//Octaves (U+E510 - U+E51F)
    GlyphData(0xE510),  // Ottava alta sign

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
        //TODO: Figured bass minus sign does not exists!
    GlyphData(0xEA6D),  // GLYPH_FIGURED_BASS_MINUS,               // -
    GlyphData(0xEA6A),  // GLYPH_FIGURED_BASS_OPEN_PARENTHESIS,    // (
    GlyphData(0xEA6B),  // GLYPH_FIGURED_BASS_CLOSE_PARENTHESIS,   // )
    GlyphData(0xEA5F),  // GLYPH_FIGURED_BASS_7_STRIKED,           // 7 with overlayered /
};


//=======================================================================================
// MusicGlyphs implementation

MusicGlyphs::MusicGlyphs(LibraryScope* pLibScope)
    : m_pLibScope(pLibScope)
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

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
    Tenths GlyphOffset;
    Tenths Top;                 //for rests flags. Only in StemFlagEngraver::cut_stem_size_to_add_flag()
    Tenths Bottom;              //for rests flags. Only in StemFlagEngraver::cut_stem_size_to_add_flag()

    GlyphData(const unsigned int glyph, Tenths yOffset=0.0f, int top=0.0f, int bottom=0.0f)
        : GlyphChar(glyph)
        , GlyphOffset(Tenths(yOffset))
        , Top(Tenths(top))
        , Bottom(Tenths(bottom))
    {
    }

};

//---------------------------------------------------------------------------------------
//indexes for the table
enum EGlyphIndex
{
    k_glyph_none = -1,            //special value meaning 'No glyph'

    //noteheads
    k_glyph_longa_note = 0,       //longa
    k_glyph_breve_note,           //breve, cuadrada
    k_glyph_whole_note,           //whole, redonda
    k_glyph_notehead_half,        //half, blanca
    k_glyph_notehead_quarter,     //quarter, negra
    k_glyph_notehead_cross,       //cross, aspa

    //notes with stem and flag, in single char
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
    k_glyph_long_fermata_above,
    k_glyph_long_fermata_below,
	k_glyph_breath_mark_v,      //breath-mark  V

    // rests
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

    //note flags
    k_glyph_eighth_flag_down,     //eighth, corchea
    k_glyph_16th_flag_down,       //16th, semicorchea
    k_glyph_32nd_flag_down,       //32nd, fusa
    k_glyph_64th_flag_down,       //64th, semifusa
    k_glyph_128th_flag_down,      //128th, garrapatea
    k_glyph_256th_flag_down,      //256th, semigarrapatea
    k_glyph_eighth_flag_up,
    k_glyph_16th_flag_up,
    k_glyph_32nd_flag_up,
    k_glyph_64th_flag_up,
    k_glyph_128th_flag_up,
    k_glyph_256th_flag_up,

    //accidentals
    k_glyph_natural_accidental,
    k_glyph_sharp_accidental,
    k_glyph_flat_accidental,
    k_glyph_double_sharp_accidental,
    k_glyph_double_flat_accidental,
    k_glyph_open_cautionary_accidental,
    k_glyph_close_cautionary_accidental,

    //clefs
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

    //numbers for time signatures
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

    //other for time signatures
    k_glyph_common_time,
    k_glyph_cut_time,

    //Metronome marks
    k_glyph_small_whole_note,
    k_glyph_small_half_note,
    k_glyph_small_quarter_note,
    k_glyph_small_eighth_note,
    k_glyph_small_16th_note,
    k_glyph_small_32th_note,
    k_glyph_small_64th_note,
    k_glyph_small_128th_note,
    k_glyph_small_256th_note,
    k_glyph_metronome_augmentation_dot,

    //Repeats (U+E040 - U+E04F)
    k_glyph_dacapo,
    k_glyph_dalsegno,
    k_glyph_coda,
    k_glyph_segno,

    //Octaves (U+E510 - U+E51F)
    k_glyph_octava,

    //figured bass. Numbers and other symbols
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

    inline const unsigned int glyph_code(int iGlyph) { return (*(m_glyphs+iGlyph)).GlyphChar; }
    inline const Tenths glyph_offset(int iGlyph) { return (*(m_glyphs+iGlyph)).GlyphOffset; }
    inline const Tenths glyph_top(int iGlyph) { return (*(m_glyphs+iGlyph)).Top; }
    inline const Tenths glyph_bottom(int iGlyph) { return (*(m_glyphs+iGlyph)).Bottom; }
    inline const GlyphData& get_glyph_data(int iGlyph) { return *(m_glyphs+iGlyph); }
};



}   //namespace lomse

#endif    // __LOMSE_GLYPHS_H__


//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

    //Rests
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

    //Note flags
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

    //Accidentals
    k_glyph_natural_accidental,
    k_glyph_sharp_accidental,
    k_glyph_flat_accidental,
    k_glyph_double_sharp_accidental,
    k_glyph_double_flat_accidental,
    k_glyph_open_cautionary_accidental,
    k_glyph_close_cautionary_accidental,

    //Clefs
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

    //Numbers for time signatures
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

    //Other for time signatures
    k_glyph_common_time,
    k_glyph_cut_time,

    //Metronome marks
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

    //Repeats (U+E040 - U+E04F)
    k_glyph_dacapo,
    k_glyph_dalsegno,
    k_glyph_coda,
    k_glyph_segno,

    //Figured bass. Numbers and other symbols
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

    //Dynamics (0xE520 - 0xE54F)
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

    //Multi-segment lines (U+EAA0 - U+EB0F)
    k_glyph_trill_wiggle_segment,           // Trill wiggle segment

    //Tremolos (U+E220 - U+E23F)
    k_glyph_tremolo_1,     // Combining tremolo 1
    k_glyph_tremolo_2,     // Combining tremolo 2
    k_glyph_tremolo_3,     // Combining tremolo 3
    k_glyph_tremolo_4,     // Combining tremolo 4
    k_glyph_tremolo_5,     // Combining tremolo 5

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

    //Brass techniques (U+E5D0 –U+E5EF)
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
    k_glyph_pedal_up_mark,      // Pedal up mark
    k_glyph_half_pedal_mark,    // Half-pedal mark

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

    //Electronic music pictograms (U+EB10 - U+EB5F)
    k_glyph_stop_button,                    // Stop button, used for errors

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
    inline Tenths glyph_offset(int iGlyph) { return (*(m_glyphs+iGlyph)).GlyphOffset; }
    inline Tenths glyph_top(int iGlyph) { return (*(m_glyphs+iGlyph)).Top; }
    inline Tenths glyph_bottom(int iGlyph) { return (*(m_glyphs+iGlyph)).Bottom; }
    inline const GlyphData& get_glyph_data(int iGlyph) { return *(m_glyphs+iGlyph); }
};



}   //namespace lomse

#endif    // __LOMSE_GLYPHS_H__


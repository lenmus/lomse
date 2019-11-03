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

#ifndef __LOMSE_INTERNAL_MODEL_H__        //to avoid nested includes
#define __LOMSE_INTERNAL_MODEL_H__

#include <string>
#include <list>
#include <vector>
#include <map>
#include <sstream>
#include "lomse_visitor.h"
#include "lomse_tree.h"
#include "lomse_basic.h"
#include "lomse_score_enums.h"
#include "lomse_shape_base.h"
#include "lomse_events.h"
#include "lomse_pixel_formats.h"
#include "lomse_injectors.h"
#include "lomse_image.h"
#include "lomse_logger.h"
typedef int TIntAttribute;

using namespace std;

///@cond INTERNALS
namespace lomse
{
///@endcond

//---------------------------------------------------------------------------------------
//forward declarations
class ColStaffObjs;
class LdpElement;
class SoundEventsTable;
class Document;
class EventHandler;
class Control;
class ScorePlayerCtrl;
class ButtonCtrl;
class AttribsContainer;
class ImMeasuresTable;
class ImMeasuresTableEntry;

class ImoAttachments;
class ImoAuxObj;
class ImoBarline;
class ImoBeamData;
class ImoBeamDto;
class ImoBeam;
class ImoBlocksContainer;
class ImoInlinesContainer;
class ImoBoxInline;
class ImoBlockLevelObj;
class ImoButton;
class ImoControl;
class ImoChord;
class ImoContent;
class ImoContentObj;
class ImoDirection;
class ImoDocument;
class ImoDynamic;
class ImoFontStyleDto;
class ImoHeading;
class ImoImage;
class ImoInlineLevelObj;
class ImoInlineWrapper;
class ImoInstrument;
class ImoKeySignature;
class ImoLineStyle;
class ImoLink;
class ImoList;
class ImoListItem;
class ImoLyrics;
class ImoLyricsTextInfo;
class ImoMultiColumn;
class ImoMusicData;
class ImoNote;
class ImoNoteRest;
class ImoObj;
class ImoOptionInfo;
class ImoParagraph;
class ImoParamInfo;
class ImoRelations;
class ImoRelDataObj;
class ImoRelObj;
class ImoScore;
class ImoScoreLine;
class ImoScorePlayer;
class ImoScoreText;
class ImoSimpleObj;
class ImoSlurDto;
class ImoSoundInfo;
class ImoSounds;
class ImoStaffInfo;
class ImoStaffObj;
class ImoStyle;
class ImoStyles;
class ImoTable;
class ImoTableCell;
class ImoTableBody;
class ImoTableHead;
class ImoTableRow;
class ImoTextInfo;
class ImoTextItem;
class ImoTextStyle;
class ImoTieData;
class ImoTieDto;
class ImoTimeModificationDto;
class ImoTimeSignature;
class ImoTupletDto;
class ImoTuplet;
class ImoWedge;
class ImoWedgeDto;
class ImoWrapperBox;


//---------------------------------------------------------------------------------------
// some helper defines
#define k_no_visible    false

//---------------------------------------------------------------------------------------
//limits
#define k_max_voices    64      //number of voices per instrument

//---------------------------------------------------------------------------------------
// enums for common values

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes values for placement attribute.
    The placement attribute indicates whether something is above or below another
    element, such as a note or a notation.

    @#include <lomse_internal_model.h>
*/
enum EPlacement
{
    k_placement_default = 0,    ///< Use default placement
    k_placement_above,          ///< Place it above
    k_placement_below,          ///< Place it below
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes values for orientation attribute.
    The orientation attribute indicates whether slurs and ties are overhand (tips
    down) or underhand (tips up). This is distinct from the placement attribute:
    placement is relative, one object with respect to another object. But
    orientation is referred to the object itself: turned up or down.

    @#include <lomse_internal_model.h>
*/
enum EOrientation
{
    k_orientation_default = 0,      ///< Default orientation
    k_orientation_over,             ///< Over (tips down)
    k_orientation_under,            ///< Under (tips up)
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes values for line type.

    @#include <lomse_internal_model.h>
*/
enum ELineType
{
    k_line_type_solid = 0,  ///< Solid line
    k_line_type_dashed,     ///< Dashed line
    k_line_type_dotted,     ///< Dotted line
    k_line_type_wavy,       ///< Wavy line
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes values for tye line-shape attribute. This attribute is used
    to distinguish between straight and curved lines.

    @#include <lomse_internal_model.h>
*/
enum ELineShape
{
    k_line_shape_straight = 0,  ///< Strait line
    k_line_shape_curved,        ///< Curved line
};


//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid values for clefs.

    @#include <lomse_internal_model.h>
*/
enum EClef
{
    k_clef_undefined=-1,    ///< Undefined clef
    k_clef_G2 = 0,          ///< G clef on second line (treble clef)
    k_clef_F4,              ///< F clef on fourth line (bass clef)
    k_clef_F3,              ///< F clef on third line (baritone clef)
    k_clef_C1,              ///< C clef on first line (soprano clef)
    k_clef_C2,              ///< C clef on second line (mezzo-soprano clef)
    k_clef_C3,              ///< C clef on third line (alto clef)
    k_clef_C4,              ///< C clef on fourth line (tenor clef)
    k_clef_percussion,      ///< Percussion clef
    // other clefs not available for exercises
    k_clef_C5,              ///< C clef on fifth line (baritone clef)
    k_clef_F5,              ///< F clef on fifth line (subbass clef)
    k_clef_G1,              ///< G clef on first line (French violin)
    k_clef_8_G2,            ///< G clef on second line, 8ve. above
    k_clef_G2_8,            ///< G clef on second line, 8ve. below
    k_clef_8_F4,            ///< F clef on fourth line, 8ve. above
    k_clef_F4_8,            ///< F clef on fourth line, 8ve below
    k_clef_15_G2,           ///< G clef on second line, 15 above
    k_clef_G2_15,           ///< G clef on second line, 15 below
    k_clef_15_F4,           ///< F clef on fourth line, 15 above
    k_clef_F4_15,           ///< F clef on fourth line, 15 below

    k_max_clef,             ///< Last element, for loops and checks
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid values for standard key signatures.

    @#include <lomse_internal_model.h>
*/
enum EKeySignature
{
    k_key_undefined=-1,         ///< Undefined key signature

    k_key_C=0,                  ///< C major key signature
    k_min_key = k_key_C,        ///< Minimum valid value = C major key signature
    k_min_major_key = k_key_C,  ///< Minimum value for major keys = C major key signature
    k_key_G,                    ///< G major key signature
    k_key_D,                    ///< D major key signature
    k_key_A,                    ///< A major key signature
    k_key_E,                    ///< E major key signature
    k_key_B,                    ///< B major key signature
    k_key_Fs,                   ///< F-sharp major key signature
    k_key_Cs,                   ///< C-sharp major key signature
    k_key_Cf,                   ///< C-flat major key signature
    k_key_Gf,                   ///< G-flat major key signature
    k_key_Df,                   ///< D-flat major key signature
    k_key_Af,                   ///< A-flat major key signature
    k_key_Ef,                   ///< E-flat major key signature
    k_key_Bf,                   ///< B-flat major key signature
    k_key_F,                    ///< F major key signature
    k_max_major_key = k_key_F,  ///< Maximum value for valid major key signatures = F major

    k_key_a,                    ///< A minor key signature
    k_min_minor_key = k_key_a,  ///< Minimum value for minor keys = A minor key signature
    k_key_e,                    ///< E minor key signature
    k_key_b,                    ///< B minor key signature
    k_key_fs,                   ///< F-sharp minor key signature
    k_key_cs,                   ///< C-sharp minor key signature
    k_key_gs,                   ///< G-sharp minor key signature
    k_key_ds,                   ///< D-sharp minor key signature
    k_key_as,                   ///< A-sharp minor key signature
    k_key_af,                   ///< A-flat minor key signature
    k_key_ef,                   ///< E-flat minor key signature
    k_key_bf,                   ///< B-flat minor key signature
    k_key_f,                    ///< F minor key signature
    k_key_c,                    ///< C minor key signature
    k_key_g,                    ///< G minor key signature
    k_key_d,                    ///< D minor key signature
    k_max_minor_key = k_key_d,  ///< Maximum value for valid minor key signatures = D minor
    k_max_key = k_key_d,        ///< Maximum value for valid key signatures = D minor
};
#define k_num_keys k_max_key - k_min_key + 1

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid modes for standard key signatures.

    @#include <lomse_internal_model.h>
*/
enum EKeyMode
{
    k_key_mode_none = 0,
    k_key_mode_major,
    k_key_mode_minor,
    k_key_mode_dorian,
    k_key_mode_phrygian,
    k_key_mode_lydian,
    k_key_mode_mixolydian,
    k_key_mode_aeolian,
    k_key_mode_ionian,
    k_key_mode_locrian,
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid values for the number of fifths in standard key signatures.

    @#include <lomse_internal_model.h>
*/
enum EKeyFifths
{
    k_fifths_Cf = -7,              ///< C-flat
    k_fifths_Gf,                   ///< G-flat
    k_fifths_Df,                   ///< D-flat
    k_fifths_Af,                   ///< A-flat
    k_fifths_Ef,                   ///< E-flat
    k_fifths_Bf,                   ///< B-flat
    k_fifths_F,                    ///< F
    k_fifths_C,                    ///< C
    k_fifths_G,                    ///< G
    k_fifths_D,                    ///< D
    k_fifths_A,                    ///< A
    k_fifths_E,                    ///< E
    k_fifths_B,                    ///< B
    k_fifths_Fs,                   ///< F-sharp
    k_fifths_Cs,                   ///< C-sharp
};


//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid note/rest types.

    @#include <lomse_internal_model.h>
*/
enum ENoteType
{
    k_unknown_notetype = -1,    ///< Unknown note type
    k_longa = 0,                ///< Longa note
    k_breve = 1,                ///< Double note (breve)
    k_whole = 2,                ///< Whole note (semibreve)
    k_half = 3,                 ///< Half note (minim)
    k_quarter = 4,              ///< Quarter note (crotchet)
    k_eighth = 5,               ///< Eighth note (quaver)
    k_16th = 6,                 ///< Sexteenth note (semiquaver)
    k_32nd = 7,                 ///< Thirty-second note (demisemiquaver)
    k_64th = 8,                 ///< Sixty-fourth note (hemidemisemiquaver)
    k_128th = 9,                ///< Hundred twenty-eighth note (semihemidemisemiquaver)
    k_256th = 10,               ///< Two hundred fifty-six note

    k_max_note_type,            ///< Last element, for loops and checks
};


//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes note/rest durations (in Lomse Time Units). It is
    useful for comparisons.

    @#include <lomse_internal_model.h>
*/
enum ENoteDuration
{
    k_duration_longa_dotted = 1536,     ///< Duration of a dotted longa note
    k_duration_longa = 1024,            ///< Duration of a longa note
    k_duration_breve_dotted = 768,      ///< Duration of a dotted breve note
    k_duration_breve = 512,             ///< Duration of a breve note
    k_duration_whole_dotted = 384,      ///< Duration of a dotted whole note
    k_duration_whole = 256,             ///< Duration of a whole note
    k_duration_half_dotted = 192,       ///< Duration of a dotted half note
    k_duration_half = 128,              ///< Duration of a half note
    k_duration_quarter_dotted = 96,     ///< Duration of a dotted quarter note
    k_duration_quarter = 64,            ///< Duration of a quarter note
    k_duration_eighth_dotted = 48,      ///< Duration of a dotted eighth note
    k_duration_eighth = 32,             ///< Duration of a eighth note
    k_duration_16th_dotted = 24,        ///< Duration of a dotted 16th note
    k_duration_16th = 16,               ///< Duration of a 16th note
    k_duration_32nd_dotted = 12,        ///< Duration of a dotted 32nd note
    k_duration_32nd = 8,                ///< Duration of a 32nd note
    k_duration_64th_dotted = 6,         ///< Duration of a dotted 64th note
    k_duration_64th = 4,                ///< Duration of a 64th note
    k_duration_128th_dotted = 3,        ///< Duration of a dotted 128th note
    k_duration_128th = 2,               ///< Duration of a 128th note
    k_duration_256th = 1,               ///< Duration of a 256th note
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid values for barlines.

    @#include <lomse_internal_model.h>
*/
enum EBarline
{
    k_barline_unknown = -1,             ///< Unknown barline type
    k_barline_none,						///< No barline
    k_barline_simple,                   ///< Simple barline
    k_barline_double,                   ///< Double barline
    k_barline_start,                    ///< Start barline
    k_barline_end,                      ///< End barline
    k_barline_start_repetition,         ///< Start of repetition barline
    k_barline_end_repetition,           ///< End of repetition barline
    k_barline_double_repetition,        ///< Double repetition barline
    k_barline_double_repetition_alt,    ///< Double repetition barline alternative design
                                        //for heavy-heavy in MusicXML. See E.Gould, p.234

    k_max_barline,                      ///< Last element, for loops and checks
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid values for the winged attribute. It indicates
	whether a repeat barline has winged extensions that appear above and below
	the barline. The straight and curved values represent single wings,
	while the double-straight and double-curved values represent double wings.
	The none value indicates no wings.

    @#include <lomse_internal_model.h>
*/
enum EBarlineWings
{
    k_wings_none = 0,				///< No wings. Default value
    k_wings_straight,				///< Strait wings
    k_wings_curved,					///< Curved wings
    k_wings_double_straight,		///< Double strait wings
    k_wings_double_curved,			///< Doube curved wings
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid values for articulations.

    @#include <lomse_internal_model.h>
*/
enum EArticulations
{
    k_articulation_unknown = -1,        ///< Unknown articulation

    //accents
    k_articulation_accent,					///< Accent
    k_articulation_strong_accent,       	///< Strong accent
    k_articulation_detached_legato,     	///< Detached legato
    k_articulation_legato_duro,				///< Legato duro
    k_articulation_marccato,				///< Marccato
    k_articulation_marccato_legato,			///< Marccato-legato
    k_articulation_marccato_staccato,		///< Marccato-staccato
    k_articulation_marccato_staccatissimo,	///< Marccato-staccatissimo
    k_articulation_mezzo_staccato,			///< Mezzo-staccato
    k_articulation_mezzo_staccatissimo,		///< Mezzo-staccatissimo
    k_articulation_staccato,				///< Staccato
    k_articulation_staccato_duro,			///< Staccato-duro
    k_articulation_staccatissimo_duro,		///< Staccatissimo-duro
    k_articulation_staccatissimo,			///< Staccatissimo
    k_articulation_tenuto,					///< Tenuto

    //jazz pitch articulations
    k_articulation_scoop,					///< Jazz pitch: Scoop
    k_articulation_plop,					///< Jazz pitch: Plop
    k_articulation_doit,					///< Jazz pitch: Doit
    k_articulation_falloff,					///< Jazz pitch: Fall off

    //stress articulations
    k_articulation_stress,              ///< Stress
    k_articulation_unstress,             ///< Unstress

    //other in MusicXML
    k_articulation_spiccato,            ///< Spicato

    //breath marks
    k_articulation_breath_mark,         ///< Breath mark
    k_articulation_caesura,             ///< Caesura

    k_max_articulation,						///< Last element, for loops and checks
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid values for ornaments.

    @#include <lomse_internal_model.h>
*/
enum EOrnaments
{
    k_ornament_unknown = -1,            ///< Unknown ornament
    k_ornament_trill_mark,              ///< Trill mark
    k_ornament_vertical_turn,           ///< Vertical turn
    k_ornament_shake,                   ///< Shake
    k_ornament_turn,                    ///< Turn
    k_ornament_delayed_turn,            ///< Delayed turn
    k_ornament_inverted_turn,           ///< Inverted turn
    k_ornament_delayed_inverted_turn,   ///< Delayed inverted turn
    k_ornament_mordent,                 ///< Mordent
    k_ornament_inverted_mordent,        ///< Inverted mordent
    k_ornament_wavy_line,               ///< Wavy line
    k_ornament_schleifer,               ///< Schleifer
    k_ornament_tremolo,                 ///< Tremolo
    k_ornament_other,                   ///< Other ornaments (for MusicXML)

    k_max_ornament,                         ///< Last element, for loops and checks
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid values for technical marks.

    @#include <lomse_internal_model.h>
*/
enum ETechnical
{
    k_technical_unknown = -1,       ///< Unknown technical mark
    k_technical_up_bow,             ///< Bow up
    k_technical_down_bow,           ///< Bow down
    k_technical_harmonic,           ///< Harmonic
    k_technical_fingering,          ///< Fingering
    k_technical_double_tongue,      ///< Double tongue
    k_technical_triple_tongue,      ///< Triple tongue
    k_technical_hole,               ///< Hole
    k_technical_handbell,           ///< Handbell

    k_max_technical,				///< Last element, for loops and checks
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid values for repetition marks and sound directions.

    @#include <lomse_internal_model.h>
*/
enum ERepeatMark
{
    k_repeat_none = 0,				///< No mark
    k_repeat_segno,					///< Segno
    k_repeat_coda,					///< Coda
    k_repeat_fine,					///< Fine
    k_repeat_da_capo,				///< D.C.
    k_repeat_da_capo_al_fine,		///< D.C. al Fine
    k_repeat_da_capo_al_coda,		///< D.C. al Coda
    k_repeat_dal_segno,				///< Dal Segno
    k_repeat_dal_segno_al_fine,		///< Dal Segno al Fine
    k_repeat_dal_segno_al_coda,		///< Dal Segno al Coda
    k_repeat_to_coda,				///< To Coda
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid types for internal model objects.

    @#include <lomse_internal_model.h>

    @todo Document other ImoObj type values
*/
enum EImoObjType
{
    // ImoObj (A)
    k_imo_obj=0,

    //ImoDto (A)
    k_imo_dto,
    k_imo_beam_dto,
    k_imo_border_dto,
    k_imo_color_dto,
    k_imo_font_style_dto,
    k_imo_point_dto,
    k_imo_octave_shift_dto,
    k_imo_size_dto,
    k_imo_slur_dto,
    k_imo_tie_dto,
    k_imo_time_modification_dto,
    k_imo_tuplet_dto,
    k_imo_volta_bracket_dto,
    k_imo_wedge_dto,
    k_imo_dto_last,

    // ImoSimpleObj (A)
    k_imo_simpleobj,

    // value objects, never nodes in tree
    k_imo_bezier_info,
    k_imo_cursor_info,
    k_imo_figured_bass_info,
    k_imo_instr_group,
    k_imo_line_style,
    k_imo_lyrics_text_info,
    k_imo_midi_info,
    k_imo_page_info,
    k_imo_sound_info,
    k_imo_staff_info,
    k_imo_system_info,
    k_imo_text_info,
    k_imo_textblock_info,
    k_imo_text_style,

    // nodes in tree
    k_imo_option,
    k_imo_param_info,
    k_imo_style,

    // ImoRelDataObj (A)
    k_imo_reldataobj,
    k_imo_beam_data,
    k_imo_slur_data,
    k_imo_tie_data,
    k_imo_reldataobj_last,

    //ImoCollection(A)
    k_imo_collection,
    k_imo_attachments,
    k_imo_instruments,
    k_imo_instrument_groups,
    k_imo_music_data,
    k_imo_options,
    k_imo_sounds,
    k_imo_table_head,
    k_imo_table_body,
    k_imo_collection_last,

    // Special collections
    k_imo_styles,
    k_imo_relations,

    // ImoContainerObj (A)
    k_imo_containerobj,
    k_imo_instrument,
    k_imo_containerobj_last,

    k_imo_simpleobj_last,


    // ImoContentObj (A)
    k_imo_contentobj,

    // ImoScoreObj (A) content only for scores
    k_imo_scoreobj,             ///< <b>Score objects: content only valid for music scores. Any of the following:</b>

    // ImoStaffObj (A)
    k_imo_staffobj,             ///< &nbsp;&nbsp;&nbsp;&nbsp; <b>Staff object. Any of the following:</b>
    k_imo_barline,          ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Barline
    k_imo_clef,             ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Clef
    k_imo_direction,        ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Direction: a musical indication that is not attached to a specific note.
    k_imo_figured_bass,     ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Figured bass mark
    k_imo_go_back_fwd,      ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; GoBackFwd
    k_imo_key_signature,    ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Key signature
    k_imo_note,             ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Note
    k_imo_rest,             ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Rest
    k_imo_sound_change,     ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Playback parameters
    k_imo_system_break,     ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; System break
    k_imo_time_signature,   ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Time signature
    k_imo_staffobj_last,

    // ImoAuxObj (A)
    k_imo_auxobj,               ///< &nbsp;&nbsp;&nbsp;&nbsp; <b>Auxiliary object. Any of the following:</b>
    k_imo_dynamics_mark,    ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Dynamics mark
    k_imo_fermata,          ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Fermata
    k_imo_metronome_mark,   ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Metronome mark
    k_imo_ornament,         ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Ornament
    k_imo_symbol_repetition_mark,   ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Symbol for repetition mark
    k_imo_technical,        ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Technical mark
    k_imo_text_repetition_mark, ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Text for repetition mark

    // ImoArticulation (A)
    k_imo_articulation,
    k_imo_articulation_symbol,  ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Articulation symbol
    k_imo_articulation_line,    ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Articulation line
    k_imo_articulation_last,

    k_imo_score_text,
    k_imo_score_title,
    k_imo_line,
    k_imo_score_line,
    k_imo_text_box,

    // ImoAuxReloObj (A)
    k_imo_auxrelobj,
    k_imo_lyric,        ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Lyrics

    k_imo_auxobj_last,

    // ImoRelObj (A)
    k_imo_relobj,           ///< &nbsp;&nbsp;&nbsp;&nbsp; <b>Relation objects. Any of the following:</b>
    k_imo_beam,             ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Beam
    k_imo_chord,            ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Chord
    k_imo_octave_shift,     ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Octave-shift line
    k_imo_slur,             ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Slur
    k_imo_tie,              ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Tie
    k_imo_tuplet,           ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Tuplet
    k_imo_volta_bracket,    ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Volta bracket
    k_imo_wedge,            ///< &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Wedge
    k_imo_relobj_last,

    k_imo_scoreobj_last,

    // ImoBlockLevelObj (A)
    k_imo_block_level_obj,	///< <b>Block level objects: Any of the following:</b>
    k_imo_score,		    ///< &nbsp;&nbsp;&nbsp;&nbsp; Music score

    // ImoBlocksContainer (A)
    k_imo_blocks_container,
    k_imo_content,
    k_imo_dynamic,
    k_imo_document,
    k_imo_list,         ///< &nbsp;&nbsp;&nbsp;&nbsp; Text list
    k_imo_listitem,
    k_imo_multicolumn,
    k_imo_table,        ///< &nbsp;&nbsp;&nbsp;&nbsp; Table
    k_imo_table_cell,
    k_imo_table_row,
    k_imo_blocks_container_last,

    // ImoInlinesContainer (A)
    k_imo_inlines_container,
    k_imo_anonymous_block,
    k_imo_heading,      ///< &nbsp;&nbsp;&nbsp;&nbsp; Text header
    k_imo_para,         ///< &nbsp;&nbsp;&nbsp;&nbsp; Paragraph
    k_imo_inlines_container_last,

    k_imo_block_level_obj_last,

    // ImoInlineLevelObj
    k_imo_inline_level_obj,  ///< <b>Inline level objects: Any of the following:</b>
    k_imo_image,                ///< &nbsp;&nbsp;&nbsp;&nbsp; Image
    k_imo_text_item,            ///< &nbsp;&nbsp;&nbsp;&nbsp; Text item
    k_imo_control,
    k_imo_score_player,     ///< &nbsp;&nbsp;&nbsp;&nbsp; Score player control
    k_imo_button,           ///< &nbsp;&nbsp;&nbsp;&nbsp; Button control
    k_imo_control_end,

    // ImoBoxInline (A)
    k_imo_box_inline,
    k_imo_inline_wrapper,
    k_imo_link,             ///< &nbsp;&nbsp;&nbsp;&nbsp; Link
    k_imo_box_inline_last,

    k_imo_inline_level_obj_last,

    k_imo_contentobj_last,
    k_imo_last,

};

//---------------------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid values for score option "Score.JustifyLastSystem". This
    option controls the justification of the last system.

    @#include <lomse_internal_model.h>
*/
enum EJustifyLastSystemOpts
{
	k_justify_never = 0,			///< Never justify last system
    k_justify_barline_final = 1,	///< Justify it only if ends in barline of type final
    k_justify_barline_any = 2,		///< Justify it only if ends in barline of any type
    k_justify_always = 3,			///< Justify it in any case
};

//---------------------------------------------------------------------------------------
/** @ingroup enumerations

    When a system is not justified, the staff lines should always run until right margin
	of the score, but this behavior can be controlled by the score option
	"StaffLines.Truncate". This option only works when a system is not justified and
	the valid values are described by this enum.

    Option 'k_truncate_barline_final' is the default behavior and it can be useful
	for score editors: staff lines will run always to right margin until a barline of
	type final is entered.

    Option 'k_truncate_always' truncates staff lines after last staff object. It can
	be useful for creating score samples (i.e. for ebooks).

    @#include <lomse_internal_model.h>
*/
enum ETruncateStaffLinesOpts
{

    k_truncate_never = 0,			///< Never truncate. Staff lines will always run to right margin.
    k_truncate_barline_final = 1,	///< Truncate only if last object is a barline of type final
    k_truncate_barline_any = 2,		///< Truncate only if last object is a barline of any type
    k_truncate_always = 3,			///< Truncate always, in any case, after last object
};

//---------------------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid flags for score option "Render.SpacingOptions". This
    flags control the algorithms for laying out scores.

    @#include <lomse_internal_model.h>
*/
enum ERenderSpacingOpts
{
    // Lines breaker algorithm
    k_render_opt_breaker_simple     = 0x0001,   ///< use LinesBreakerSimple.
    k_render_opt_breaker_optimal    = 0x0002,   ///< use LinesBreakerOptimal
    k_render_opt_breaker_no_shrink  = 0x0004,   ///< do not shrink lines

    // Spacing algorithm
    k_render_opt_dmin_fixed         = 0x0100,   ///< use fixed value for Dmin
    k_render_opt_dmin_global        = 0x0200,   ///< use min note in score for Dmin
};



///@cond INTERNALS
//---------------------------------------------------------------------------------------
// a struct to contain note/rest figure and dots
struct NoteTypeAndDots
{
    NoteTypeAndDots(int nt, int d) : noteType(nt), dots(d) {}

    int noteType;   //ImoNoteRest enum
    int dots;       //0..n
};


//---------------------------------------------------------------------------------------
// utility function to convert typographical points to LUnits
extern LUnits pt_to_LUnits(float pt);


//---------------------------------------------------------------------------------------
// API for objects that are allowed to create inline-level objects
class InlineLevelCreatorApi
{
protected:
    ImoContentObj* m_pParent;

    inline void set_inline_level_creator_api_parent(ImoContentObj* parent)
    {
        m_pParent = parent;
    }

public:
    InlineLevelCreatorApi() : m_pParent(nullptr) {}
    virtual ~InlineLevelCreatorApi() {}

    //API
    ImoTextItem* add_text_item(const string& text, ImoStyle* pStyle=nullptr);
    ButtonCtrl* add_button(LibraryScope& libScope, const string& label,
                           const USize& size, ImoStyle* pStyle=nullptr);
    ImoInlineWrapper* add_inline_box(LUnits width=0.0f, ImoStyle* pStyle=nullptr);
    ImoLink* add_link(const string& url, ImoStyle* pStyle=nullptr);
    ImoImage* add_image(unsigned char* imgbuf, VSize bmpSize, EPixelFormat format,
                        USize imgSize, ImoStyle* pStyle=nullptr);
    ImoControl* add_control(Control* pCtrol);

};


//---------------------------------------------------------------------------------------
// API for objects that are allowed to create block-level objects
class BlockLevelCreatorApi
{
protected:
    ImoContentObj* m_pParent;

    inline void set_box_level_creator_api_parent(ImoContentObj* parent)
    {
        m_pParent = parent;
    }

public:
    BlockLevelCreatorApi() : m_pParent(nullptr) {}
    virtual ~BlockLevelCreatorApi() {}

    //API
    ImoContent* add_content_wrapper(ImoStyle* pStyle=nullptr);
    ImoList* add_list(int type, ImoStyle* pStyle=nullptr);
    ImoMultiColumn* add_multicolumn_wrapper(int numCols, ImoStyle* pStyle=nullptr);
    ImoParagraph* add_paragraph(ImoStyle* pStyle=nullptr);
    ImoScore* add_score(ImoStyle* pStyle=nullptr);

private:
    void add_to_model(ImoBlockLevelObj* pImo, ImoStyle* pStyle);

};

//---------------------------------------------------------------------------------------
// Auxiliary class: An attribute

// AttribValue can hold one of <int, float, double, bool, string, Color>.
// In C++17 it could be declared as simple as:
//     typedef std::variant<int, float, double, bool, string, Color> AttribValue;
// But we implement it manually to support older compilers.
class AttribValue
{
private:
    union
    {
        int intValue;
        string stringValue;
        bool boolValue;
        float floatValue;
        double doubleValue;
        Color colorValue;
    };

    enum AttribType { vt_empty, vt_int, vt_string, vt_bool, vt_float, vt_double, vt_color };
    AttribType m_type = vt_empty;

public:
    AttribValue() {}
    ~AttribValue() { Cleanup(); }

    explicit operator int() const
    {
        CheckType(vt_int);
        return intValue;
    }
    explicit operator float() const
    {
        CheckType(vt_float);
        return floatValue;
    }
    explicit operator double() const
    {
        CheckType(vt_double);
        return doubleValue;
    }
    explicit operator bool() const
    {
        CheckType(vt_bool);
        return boolValue;
    }
    explicit operator string() const
    {
        CheckType(vt_string);
        return stringValue;
    }
    explicit operator Color() const
    {
        CheckType(vt_color);
        return colorValue;
    }

    void operator=(int value)
    {
        Cleanup();
        intValue = value;
        m_type = vt_int;
    }
    void operator=(float value)
    {
        Cleanup();
        floatValue = value;
        m_type = vt_float;
    }
    void operator=(double value)
    {
        Cleanup();
        doubleValue = value;
        m_type = vt_double;
    }
    void operator=(bool value)
    {
        Cleanup();
        boolValue = value;
        m_type = vt_bool;
    }
    void operator=(const string& value)
    {
        Cleanup();
        // placement new (http://en.cppreference.com/w/cpp/language/union)
        new (&stringValue) string(value);
        m_type = vt_string;
    }
    void operator=(const Color& value)
    {
        Cleanup();
        // placement new (http://en.cppreference.com/w/cpp/language/union)
        new (&colorValue) Color(value);
        m_type = vt_color;
    }

private:
    void Cleanup()
    {
        if (m_type == vt_string)
        {
            // explicit destructor call (http://en.cppreference.com/w/cpp/language/union)
            stringValue.~string();
        }
        else if (m_type == vt_color)
        {
            // explicit destructor call (http://en.cppreference.com/w/cpp/language/union)
            colorValue.~Color();
        }
        m_type = vt_empty;
    }

    void CheckType(AttribType type) const
    {
        if (type != m_type)
        {
            stringstream ss;
            ss << "[AttribValue::operator(<type>)]. Type mismatch. Expected type=" <<
                type << ", real type=" << m_type;
            LOMSE_LOG_ERROR(ss.str());
            throw std::runtime_error(ss.str());
        }
    }
};

class ImoAttr
{
protected:
    int m_attrbIdx;
    AttribValue m_value;
    ImoAttr* m_next;

public:
    ImoAttr(int idx) : m_attrbIdx(idx), m_next(nullptr) {}
    ImoAttr(int idx, const string& value);
    ImoAttr(int idx, int value);
    ImoAttr(int idx, float value);
    ImoAttr(int idx, double value);
    ImoAttr(int idx, bool value);
    ImoAttr(int idx, Color value);
    ~ImoAttr() {}

    const string get_name();

    inline int get_int_value()
    {
        return static_cast<int>(m_value);
    }
    inline double get_double_value()
    {
        return static_cast<double>(m_value);
    }
    inline string get_string_value()
    {
        return static_cast<string>(m_value);
    }
    inline bool get_bool_value()
    {
        return static_cast<bool>(m_value);
    }
    inline float get_float_value()
    {
        return static_cast<float>(m_value);
    }
    inline Color get_color_value()
    {
        return static_cast<Color>(m_value);
    }

    inline void set_string_value(const string& value)
    {
        m_value = value;
    }
    inline void set_int_value(int value)
    {
        m_value = value;
    }
    inline void set_double_value(double value)
    {
        m_value = value;
    }
    inline void set_float_value(float value)
    {
        m_value = value;
    }
    inline void set_bool_value(bool value)
    {
        m_value = value;
    }
    inline void set_color_value(Color value)
    {
        m_value = value;
    }

    inline int get_attrib_idx()
    {
        return m_attrbIdx;
    }
    inline ImoAttr* get_next_attrib()
    {
        return m_next;
    }

    static const string get_name(int idx);

protected:
    friend class ImoObj;
    inline void set_next_attrib(ImoAttr* pAttr)
    {
        m_next = pAttr;
    }

};


//=======================================================================================
// Auxiliary data containers. They are composite data types used to encapsulate data
// for the IM, but they are not nodes in the IM tree.
//
// They are just passive data structures (= Plain old data (POD), = Pasive Data Structure
// (PDS), = record), that is a collections of field values, without using object-oriented
// features. The C++ compiler guarantees that there will be no "magic" going on in the
// structure: for example hidden pointers to vtables, offsets that get applied to
// the address when it is cast to other types (at least if the target's Type too),
// constructors, or destructors. Roughly speaking, a type is a Type when the only things
// in it are built-in types and combinations of them. The result is something that
// "acts like" a C type.
//
// This 'Type' classes are auxiliary and they are not nodes in the IM tree.
//=======================================================================================

//---------------------------------------------------------------------------------------
/** %TypeMeasureInfo contains the data associated to a measure, such as its
    displayed number.

    %TypeMeasureInfo objects are not part of the IM tree. Each %TypeMeasureInfo object
    is directly stored in the ImoBarline object that closes the measure. And
    in ImoInstrument if last measure is not closed or if the score has no metric.
*/
class TypeMeasureInfo
{
public:
    int index;      ///< An optional integer index for the measure, as defined in NMX.
                    ///< The first measure has an index of 1.
    int count;      ///< sequential integer index for the measure. The first measure
                    ///< is counted as 1, even if anacrusis start.
    string number;  ///< An optional textual number to be displayed for the measure.
    bool fHideNumber;   ///< Override measures number policy for preventing to
                        ///< display the number in this measure.


    TypeMeasureInfo()
        : index(0)
        , count(0)
        , number("")
        , fHideNumber(false)
    {
    }

    ~TypeMeasureInfo() {}
};


//=======================================================================================
// standard interface for ImoObj objects containing an ImoSounds child
//=======================================================================================
#define LOMSE_DECLARE_IMOSOUNDS_INTERFACE                               \
    ImoSounds* get_sounds();                                            \
    void add_sound_info(ImoSoundInfo* pInfo);                           \
    int get_num_sounds();                                               \
    ImoSoundInfo* get_sound_info(const string& soundId);                \
    ImoSoundInfo* get_sound_info(int iSound);    //iSound = 0..n-1


//************************************************************
// Objects that form the content of the internal classes
//************************************************************

//===================================================
// Abstract objects hierachy
//===================================================


//---------------------------------------------------------------------------------------
// the root. Any object must derive from it
class ImoObj : public Visitable, public TreeNode<ImoObj>
{
protected:
    Document* m_pDoc;
    ImoId m_id;
    int m_objtype;
    unsigned int m_flags;
    ImoAttr* m_pAttribs;

protected:
    ImoObj(int objtype, ImoId id=k_no_imoid);

    friend class ImFactory;
    inline void set_owner_document(Document* pDoc)
    {
        m_pDoc = pDoc;
    }
    virtual void initialize_object(Document* pDoc);

public:
    virtual ~ImoObj();

    //flag values
    enum
    {
        k_dirty             = 0x0001,   //dirty: modified since last "clear_dirty()" ==> need to rebuild GModel
        k_children_dirty    = 0x0002,   //this is not dirty but some children are dirty
        k_edit_terminal     = 0x0004,   //terminal node for edition
        k_editable          = 0x0008,   //in edition, this node can be edited
        k_deletable         = 0x0010,   //if editable, this node can be also deleted
        k_expandable        = 0x0020,   //if editable, more children can be added/inserted
    };

    //dirty
    inline bool is_dirty()
    {
        return (m_flags & k_dirty) != 0;
    }
    void set_dirty(bool value);
    inline bool are_children_dirty()
    {
        return (m_flags & k_children_dirty) != 0;
    }
    void set_children_dirty(bool value);

    //edition flags
    inline bool is_edit_terminal()
    {
        return (m_flags & k_edit_terminal) != 0;
    }
    inline void set_edit_terminal(bool value)
    {
        value ? m_flags |= k_edit_terminal
                           : m_flags &= ~k_edit_terminal;
    }
    inline bool is_editable()
    {
        return (m_flags & k_editable) != 0;
    }
    inline void set_editable(bool value)
    {
        value ? m_flags |= k_editable
                           : m_flags &= ~k_editable;
    }
    inline bool is_deletable()
    {
        return (m_flags & k_deletable) != 0;
    }
    inline void set_deletable(bool value)
    {
        value ? m_flags |= k_deletable
                           : m_flags &= ~k_deletable;
    }
    inline bool is_expandable()
    {
        return (m_flags & k_expandable) != 0;
    }
    inline void set_expandable(bool value)
    {
        value ? m_flags |= k_expandable
                           : m_flags &= ~k_expandable;
    }

    //getters
    inline ImoId get_id()
    {
        return m_id;
    }

    //setters
    inline void set_id(ImoId id)
    {
        m_id = id;
    }

    //required by Visitable parent class
    virtual void accept_visitor(BaseVisitor& v);
    virtual bool has_visitable_children()
    {
        return has_children();
    }

    //parent / children
    ImoObj* get_child_of_type(int objtype);
    virtual ImoObj* get_parent_imo()
    {
        return static_cast<ImoObj*>(get_parent());
    }
    void append_child_imo(ImoObj* pImo);
    void remove_child_imo(ImoObj* pImo);
    Document* get_the_document();
    ImoDocument* get_document();
    Observable* get_observable_parent();
    ImoContentObj* get_contentobj_parent();
    ImoBlockLevelObj* find_block_level_parent();

    //Get the name and source code
    static const string& get_name(int type);
    const string& get_name() const;
    string to_string(bool fWithIds=false);
    inline string to_string_with_ids()
    {
        return to_string(true);
    }

    //properties
    virtual bool can_generate_secondary_shapes()
    {
        return false;
    }

    //IM attributes interface
    //set attribute value
    virtual void set_int_attribute(TIntAttribute idx, int value);
    virtual void set_color_attribute(TIntAttribute idx, Color value);
    virtual void set_bool_attribute(TIntAttribute idx, bool value);
    virtual void set_float_attribute(TIntAttribute idx, float value);
    virtual void set_double_attribute(TIntAttribute idx, double value);
    virtual void set_string_attribute(TIntAttribute idx, const string& value);
    //get attribute value
    virtual int get_int_attribute(TIntAttribute idx);
    virtual float get_float_attribute(TIntAttribute idx);
    virtual double get_double_attribute(TIntAttribute idx);
    virtual Color get_color_attribute(TIntAttribute idx);
    virtual bool get_bool_attribute(TIntAttribute idx);
    virtual string get_string_attribute(TIntAttribute idx);
    //attribute nodes
    ImoAttr* get_attribute_node(TIntAttribute idx);
    //miscellaneous
    virtual int get_num_attributes();
    inline ImoAttr* get_first_attribute()
    {
        return m_pAttribs;
    }
    virtual list<TIntAttribute> get_supported_attributes()
    {
        list<TIntAttribute> supported;
        return supported;
    }

    //object classification
    inline int get_obj_type()
    {
        return m_objtype;
    }
    inline bool has_children()
    {
        return !is_terminal();
    }

    //groups
    inline bool is_dto() { return m_objtype > k_imo_dto
                                  && m_objtype < k_imo_dto_last; }
    inline bool is_simpleobj() { return m_objtype > k_imo_simpleobj
                                        && m_objtype < k_imo_simpleobj_last; }
    inline bool is_collection() { return m_objtype > k_imo_collection
                                         && m_objtype < k_imo_collection_last; }
    inline bool is_reldataobj() { return m_objtype > k_imo_reldataobj
                                         && m_objtype < k_imo_reldataobj_last; }
    inline bool is_containerobj() { return m_objtype > k_imo_containerobj
                                           && m_objtype < k_imo_containerobj_last; }
    inline bool is_contentobj() { return m_objtype > k_imo_contentobj
                                         && m_objtype < k_imo_contentobj_last; }
    inline bool is_scoreobj() { return m_objtype > k_imo_scoreobj
                                       && m_objtype < k_imo_scoreobj_last; }
    inline bool is_staffobj() { return m_objtype > k_imo_staffobj
                                       && m_objtype < k_imo_staffobj_last; }
    inline bool is_auxobj() { return m_objtype > k_imo_auxobj
                                     && m_objtype < k_imo_auxobj_last; }
    inline bool is_relobj() { return m_objtype > k_imo_relobj
                                     && m_objtype < k_imo_relobj_last; }
    inline bool is_auxrelobj() { return m_objtype > k_imo_auxrelobj
                                        && m_objtype < k_imo_auxobj_last; }
    inline bool is_block_level_obj() { return m_objtype > k_imo_block_level_obj
                                              && m_objtype < k_imo_block_level_obj_last; }
    inline bool is_blocks_container() { return m_objtype > k_imo_blocks_container
                                               && m_objtype < k_imo_blocks_container_last; }
    inline bool is_inlines_container() { return m_objtype > k_imo_inlines_container
                                                && m_objtype < k_imo_inlines_container_last; }
    inline bool is_box_inline() { return m_objtype > k_imo_box_inline
                                         && m_objtype < k_imo_box_inline_last; }
    inline bool is_inline_level_obj() { return m_objtype > k_imo_inline_level_obj
                                               && m_objtype < k_imo_inline_level_obj_last; }
    inline bool is_articulation() { return m_objtype > k_imo_articulation
                                           && m_objtype < k_imo_articulation_last; }

    //items
    inline bool is_anonymous_block() { return m_objtype == k_imo_anonymous_block; }
    inline bool is_articulation_symbol() { return m_objtype == k_imo_articulation_symbol; }
    inline bool is_articulation_line() { return m_objtype == k_imo_articulation_line; }
    inline bool is_attachments() { return m_objtype == k_imo_attachments; }
    inline bool is_barline() { return m_objtype == k_imo_barline; }
    inline bool is_beam() { return m_objtype == k_imo_beam; }
    inline bool is_beam_data() { return m_objtype == k_imo_beam_data; }
    inline bool is_beam_dto() { return m_objtype == k_imo_beam_dto; }
    inline bool is_bezier_info() { return m_objtype == k_imo_bezier_info; }
    inline bool is_border_dto() { return m_objtype == k_imo_border_dto; }
    inline bool is_button() { return m_objtype == k_imo_button; }
    inline bool is_chord() { return m_objtype == k_imo_chord; }
    inline bool is_clef() { return m_objtype == k_imo_clef; }
    inline bool is_color_dto() { return m_objtype == k_imo_color_dto; }
    inline bool is_content() { return m_objtype == k_imo_content; }
    inline bool is_control() { return m_objtype >= k_imo_control
                                      && m_objtype < k_imo_control_end; }
    inline bool is_cursor_info() { return m_objtype == k_imo_cursor_info; }
    inline bool is_direction() { return m_objtype == k_imo_direction; }
    inline bool is_document() { return m_objtype == k_imo_document; }
    inline bool is_dynamic() { return m_objtype == k_imo_dynamic; }
    inline bool is_dynamics_mark() { return m_objtype == k_imo_dynamics_mark; }
    inline bool is_fermata() { return m_objtype == k_imo_fermata; }
    inline bool is_figured_bass() { return m_objtype == k_imo_figured_bass; }
    inline bool is_figured_bass_info() { return m_objtype == k_imo_figured_bass_info; }
    inline bool is_font_style_dto() { return m_objtype == k_imo_font_style_dto; }
    inline bool is_go_back_fwd() { return m_objtype == k_imo_go_back_fwd; }
    bool is_gap();      ///a rest representing a goFwd element
    inline bool is_heading() { return m_objtype == k_imo_heading; }
    inline bool is_image() { return m_objtype == k_imo_image; }
    inline bool is_inline_wrapper() { return m_objtype == k_imo_inline_wrapper; }
    inline bool is_instrument() { return m_objtype == k_imo_instrument; }
    inline bool is_instr_group() { return m_objtype == k_imo_instr_group; }
    inline bool is_key_signature() { return m_objtype == k_imo_key_signature; }
    inline bool is_line() { return m_objtype == k_imo_line; }
    inline bool is_line_style() { return m_objtype == k_imo_line_style; }
    inline bool is_link() { return m_objtype == k_imo_link; }
    inline bool is_list() { return m_objtype == k_imo_list; }
    inline bool is_listitem() { return m_objtype == k_imo_listitem; }
    inline bool is_lyric() { return m_objtype == k_imo_lyric; }
    inline bool is_lyrics_text_info() { return m_objtype == k_imo_lyrics_text_info; }
    inline bool is_metronome_mark() { return m_objtype == k_imo_metronome_mark; }
    inline bool is_midi_info() { return m_objtype == k_imo_midi_info; }
    inline bool is_multicolumn() { return m_objtype == k_imo_multicolumn; }
    inline bool is_music_data() { return m_objtype == k_imo_music_data; }
    inline bool is_note() { return m_objtype == k_imo_note; }
    inline bool is_note_rest() { return m_objtype == k_imo_note
               || m_objtype == k_imo_rest; }
    inline bool is_octave_shift() { return m_objtype == k_imo_octave_shift; }
    inline bool is_octave_shift_dto() { return m_objtype == k_imo_octave_shift_dto; }
    inline bool is_option() { return m_objtype == k_imo_option; }
    inline bool is_ornament() { return m_objtype == k_imo_ornament; }
    inline bool is_page_info() { return m_objtype == k_imo_page_info; }
    inline bool is_paragraph() { return m_objtype == k_imo_para; }
    inline bool is_param_info() { return m_objtype == k_imo_param_info; }
    inline bool is_point_dto() { return m_objtype == k_imo_point_dto; }
    inline bool is_rest() { return m_objtype == k_imo_rest; }
    inline bool is_relations() { return m_objtype == k_imo_relations; }
    inline bool is_repetition_mark() { return m_objtype == k_imo_text_repetition_mark
               || m_objtype == k_imo_symbol_repetition_mark; }
    inline bool is_score() { return m_objtype == k_imo_score; }
    inline bool is_score_line() { return m_objtype == k_imo_score_line; }
    inline bool is_score_player() { return m_objtype == k_imo_score_player; }
    inline bool is_score_text() { return m_objtype == k_imo_score_text; }
    inline bool is_score_title() { return m_objtype == k_imo_score_title; }
    inline bool is_size_info() { return m_objtype == k_imo_size_dto; }
    inline bool is_slur() { return m_objtype == k_imo_slur; }
    inline bool is_slur_data() { return m_objtype == k_imo_slur_data; }
    inline bool is_slur_dto() { return m_objtype == k_imo_slur_dto; }
    inline bool is_sound_change() { return m_objtype == k_imo_sound_change; }
    inline bool is_sound_info() { return m_objtype == k_imo_sound_info; }
    inline bool is_spacer() { return m_objtype == k_imo_direction; }
    inline bool is_staff_info() { return m_objtype == k_imo_staff_info; }
    inline bool is_style() { return m_objtype == k_imo_style; }
    inline bool is_styles() { return m_objtype == k_imo_styles; }
    inline bool is_symbol_repetition_mark() { return m_objtype == k_imo_symbol_repetition_mark; }
    inline bool is_system_break() { return m_objtype == k_imo_system_break; }
    inline bool is_system_info() { return m_objtype == k_imo_system_info; }
    inline bool is_table() { return m_objtype == k_imo_table; }
    inline bool is_table_cell() { return m_objtype == k_imo_table_cell; }
    inline bool is_table_body() { return m_objtype == k_imo_table_body; }
    inline bool is_table_head() { return m_objtype == k_imo_table_head; }
    inline bool is_table_row() { return m_objtype == k_imo_table_row; }
    inline bool is_technical() { return m_objtype == k_imo_technical; }
    inline bool is_text_info() { return m_objtype == k_imo_text_info; }
    inline bool is_text_item() { return m_objtype == k_imo_text_item; }
    inline bool is_text_style() { return m_objtype == k_imo_text_style; }
    inline bool is_textblock_info() { return m_objtype == k_imo_textblock_info; }
    inline bool is_text_box() { return m_objtype == k_imo_text_box; }
    inline bool is_text_repetition_mark() { return m_objtype == k_imo_text_repetition_mark; }
    inline bool is_tie() { return m_objtype == k_imo_tie; }
    inline bool is_tie_data() { return m_objtype == k_imo_tie_data; }
    inline bool is_tie_dto() { return m_objtype == k_imo_tie_dto; }
    inline bool is_time_signature() { return m_objtype == k_imo_time_signature; }
    inline bool is_time_modification_dto() { return m_objtype == k_imo_time_modification_dto; }
    inline bool is_tuplet() { return m_objtype == k_imo_tuplet; }
    inline bool is_tuplet_dto() { return m_objtype == k_imo_tuplet_dto; }
    inline bool is_volta_bracket() { return m_objtype == k_imo_volta_bracket; }
    inline bool is_volta_bracket_dto() { return m_objtype == k_imo_volta_bracket_dto; }
    inline bool is_wedge() { return m_objtype == k_imo_wedge; }
    inline bool is_wedge_dto() { return m_objtype == k_imo_wedge_dto; }

    //special checkers
    inline bool is_mouse_over_generator() { return   m_objtype == k_imo_link
                  || m_objtype == k_imo_button
                  ; }

protected:
    void visit_children(BaseVisitor& v);
    void propagate_dirty();
    void remove_id();
    void delete_attributes();

    //attributes
    ImoAttr* set_attribute_node(ImoAttr* newAttr);
    void remove_attribute(TIntAttribute idx);
    ImoAttr* get_last_attribute();

};

//---------------------------------------------------------------------------------------
// base class for DTOs
class ImoDto : public ImoObj
{
protected:
    ImoDto(int objtype) : ImoObj(objtype) {}

public:
    virtual ~ImoDto() {}
};

//---------------------------------------------------------------------------------------
class ImoFontStyleDto : public ImoDto
{
public:
    string name;
    float size;       // in points
    int style;        // k_font_style_normal, k_font_style_italic
    int weight;       // k_font_weight_normal, k_font_weight_bold

    ImoFontStyleDto()
        : ImoDto(k_imo_font_style_dto)
        , name("Liberation serif")
        , size(12)
        , style(k_font_style_normal)
        , weight(k_font_weight_normal)
    {
    }

    enum { k_font_weight_normal=0, k_font_weight_bold,
           k_font_style_normal, k_font_style_italic
         };
};

//---------------------------------------------------------------------------------------
class ImoColorDto : public ImoDto
{
protected:
    Color m_color;
    bool m_ok;

public:
    ImoColorDto() : ImoDto(k_imo_color_dto), m_color(0, 0, 0, 255), m_ok(true) {}
    ImoColorDto(Int8u r, Int8u g, Int8u b, Int8u a = 255);
    virtual ~ImoColorDto() {}

    Color& set_from_rgb_string(const std::string& rgb);
    Color& set_from_rgba_string(const std::string& rgba);
    Color& set_from_argb_string(const std::string& argb);
    Color& set_from_string(const std::string& hex);
    inline bool is_ok()
    {
        return m_ok;
    }

    inline Int8u red()
    {
        return m_color.r;
    }
    inline Int8u blue()
    {
        return m_color.b;
    }
    inline Int8u green()
    {
        return m_color.g;
    }
    inline Int8u alpha()
    {
        return m_color.a;
    }
    inline Color& get_color()
    {
        return m_color;
    }


protected:
    Int8u convert_from_hex(const std::string& hex);

};



// just auxiliary objects or data transfer objects
//---------------------------------------------------------------------------------------
class ImoSimpleObj : public ImoObj
{
protected:
    ImoSimpleObj(int objtype, ImoId id) : ImoObj(objtype, id) {}
    ImoSimpleObj(int objtype) : ImoObj(objtype) {}

public:
    virtual ~ImoSimpleObj() {}
};

//---------------------------------------------------------------------------------------
class ImoTextStyle : public ImoSimpleObj
{
public:
    int word_spacing;
    int text_decoration;
    int vertical_align;
    int text_align;
    LUnits text_indent_length;
    LUnits word_spacing_length;     //for case k_length

    ImoTextStyle()
        : ImoSimpleObj(k_imo_text_style)
        , word_spacing(k_normal)
        , text_decoration(k_decoration_none)
        , vertical_align(k_valign_baseline)
        , text_align(k_align_left)
        , text_indent_length(0.0f)
        , word_spacing_length(0.0f) //not applicable
    {
    }

    enum { k_normal=0, k_length, };
    enum { k_decoration_none=0, k_decoration_underline, k_decoration_overline,
           k_decoration_line_through,
         };
    enum { k_valign_baseline, k_valign_sub, k_valign_super, k_valign_top,
           k_valign_text_top, k_valign_middle, k_valign_bottom,
           k_valign_text_bottom
         };
    enum { k_align_left, k_align_right, k_align_center, k_align_justify };
};

//---------------------------------------------------------------------------------------
class ImoStyle : public ImoSimpleObj
{
protected:
    string m_name;
    ImoStyle* m_pParent;
    std::map<int, LUnits> m_lunitsProps;
    std::map<int, float> m_floatProps;
    std::map<int, string> m_stringProps;
    std::map<int, int> m_intProps;
    std::map<int, Color> m_colorProps;

    friend class ImFactory;
    ImoStyle() : ImoSimpleObj(k_imo_style), m_name(), m_pParent(nullptr) {}

public:
    virtual ~ImoStyle() {}

    //text style
    enum { k_spacing_normal=0, k_length, };
    enum { k_decoration_none=0, k_decoration_underline, k_decoration_overline,
           k_decoration_line_through,
         };
    enum { k_valign_baseline, k_valign_sub, k_valign_super, k_valign_top,
           k_valign_text_top, k_valign_middle, k_valign_bottom,
           k_valign_text_bottom
         };
    enum { k_align_left, k_align_right, k_align_center, k_align_justify };

    //font style/weight
    enum { k_font_style_normal=0, k_font_style_italic };
    enum { k_font_weight_normal=0, k_font_weight_bold, };

    //style properties
    enum
    {
        //font
        k_font_file,
        k_font_name,
        k_font_size,
        k_font_style,
        k_font_weight,
        //text
        k_word_spacing,
        k_text_decoration,
        k_vertical_align,
        k_text_align,
        k_text_indent_length,
        k_word_spacing_length,
        k_line_height,
        //color and background
        k_color,
        k_background_color,
        //margin
        k_margin_top,
        k_margin_bottom,
        k_margin_left,
        k_margin_right,
        //padding
        k_padding_top,
        k_padding_bottom,
        k_padding_left,
        k_padding_right,

//    ////border
//    k_border_top,
//    k_border_bottom,
//    k_border_left,
//    k_border_right,

        //border width
        k_border_width_top,
        k_border_width_bottom,
        k_border_width_left,
        k_border_width_right,

        //size
        k_width,
        k_height,
        k_min_width,
        k_min_height,
        k_max_width,
        k_max_height,

        //table
        k_table_col_width,
    };

    //general
    inline const std::string& get_name()
    {
        return m_name;
    }
    inline void set_name(const std::string& value)
    {
        m_name = value;
    }
    inline void set_parent_style(ImoStyle* pStyle)
    {
        m_pParent = pStyle;
    }

    //utility
    LUnits em_to_LUnits(float em)
    {
        return pt_to_LUnits( get_float_property(ImoStyle::k_font_size) * em );
    }
    inline bool is_bold()
    {
        return get_int_property(ImoStyle::k_font_weight) == k_font_weight_bold;
    }
    inline bool is_italic()
    {
        return get_int_property(ImoStyle::k_font_style) == k_font_style_italic;
    }
    bool is_default_style_with_default_values();

    //utility getters/setters to avoid stupid mistakes and to simplify source code
    //font
    inline const string& font_file()
    {
        return get_string_property(ImoStyle::k_font_file);
    }
    inline ImoStyle* font_file(const string& filename)
    {
        set_string_property(ImoStyle::k_font_file, filename);
        return this;
    }
    inline const string& font_name()
    {
        return get_string_property(ImoStyle::k_font_name);
    }
    inline ImoStyle* font_name(const string& name)
    {
        set_string_property(ImoStyle::k_font_name, name);
        return this;
    }
    inline float font_size()
    {
        return get_float_property(ImoStyle::k_font_size);
    }
    inline ImoStyle* font_size(float value)
    {
        set_float_property(ImoStyle::k_font_size, value);
        return this;
    }
    inline int font_style()
    {
        return get_int_property(ImoStyle::k_font_style);
    }
    inline ImoStyle* font_style(int value)
    {
        set_int_property(ImoStyle::k_font_style, value);
        return this;
    }
    inline int font_weight()
    {
        return get_int_property(ImoStyle::k_font_weight);
    }
    inline ImoStyle* font_weight(int value)
    {
        set_int_property(ImoStyle::k_font_weight, value);
        return this;
    }
    //text
    inline int word_spacing()
    {
        return get_int_property(ImoStyle::k_word_spacing);
    }
    inline ImoStyle* word_spacing(int value)
    {
        set_int_property(ImoStyle::k_word_spacing, value);
        return this;
    }
    inline int text_decoration()
    {
        return get_int_property(ImoStyle::k_text_decoration);
    }
    inline ImoStyle* text_decoration(int value)
    {
        set_int_property(ImoStyle::k_text_decoration, value);
        return this;
    }
    inline int vertical_align()
    {
        return get_int_property(ImoStyle::k_vertical_align);
    }
    inline ImoStyle* vertical_align(int value)
    {
        set_int_property(ImoStyle::k_vertical_align, value);
        return this;
    }
    inline int text_align()
    {
        return get_int_property(ImoStyle::k_text_align);
    }
    inline ImoStyle* text_align(int value)
    {
        set_int_property(ImoStyle::k_text_align, value);
        return this;
    }
    inline LUnits text_indent_length()
    {
        return get_lunits_property(ImoStyle::k_text_indent_length);
    }
    inline ImoStyle* text_indent_length(LUnits value)
    {
        set_lunits_property(ImoStyle::k_text_indent_length, value);
        return this;
    }
    inline LUnits word_spacing_length()
    {
        return get_lunits_property(ImoStyle::k_word_spacing_length);
    }
    inline ImoStyle* word_spacing_length(LUnits value)
    {
        set_lunits_property(ImoStyle::k_word_spacing_length, value);
        return this;
    }
    inline float line_height()
    {
        return get_float_property(ImoStyle::k_line_height);
    }
    inline ImoStyle* line_height(float value)
    {
        set_float_property(ImoStyle::k_line_height, value);
        return this;
    }
    //color and background
    inline Color color()
    {
        return get_color_property(ImoStyle::k_color);
    }
    inline ImoStyle* color(Color color)
    {
        set_color_property(ImoStyle::k_color, color);
        return this;
    }
    inline Color background_color()
    {
        return get_color_property(ImoStyle::k_background_color);
    }
    inline ImoStyle* background_color(Color color)
    {
        set_color_property(ImoStyle::k_background_color, color);
        return this;
    }
    //border-width
    inline ImoStyle* border_width(LUnits value)
    {
        set_border_width_property(value);
        return this;
    }
    inline LUnits border_width_top()
    {
        return get_lunits_property(ImoStyle::k_border_width_top);
    }
    inline ImoStyle* border_width_top(LUnits value)
    {
        set_lunits_property(ImoStyle::k_border_width_top, value);
        return this;
    }
    inline LUnits border_width_bottom()
    {
        return get_lunits_property(ImoStyle::k_border_width_bottom);
    }
    inline ImoStyle* border_width_bottom(LUnits value)
    {
        set_lunits_property(ImoStyle::k_border_width_bottom, value);
        return this;
    }
    inline LUnits border_width_left()
    {
        return get_lunits_property(ImoStyle::k_border_width_left);
    }
    inline ImoStyle* border_width_left(LUnits value)
    {
        set_lunits_property(ImoStyle::k_border_width_left, value);
        return this;
    }
    inline LUnits border_width_right()
    {
        return get_lunits_property(ImoStyle::k_border_width_right);
    }
    inline ImoStyle* border_width_right(LUnits value)
    {
        set_lunits_property(ImoStyle::k_border_width_right, value);
        return this;
    }
    //padding
    inline ImoStyle* padding(LUnits value)
    {
        set_padding_property(value);
        return this;
    }
    inline LUnits padding_top()
    {
        return get_lunits_property(ImoStyle::k_padding_top);
    }
    inline ImoStyle* padding_top(LUnits value)
    {
        set_lunits_property(ImoStyle::k_padding_top, value);
        return this;
    }
    inline LUnits padding_bottom()
    {
        return get_lunits_property(ImoStyle::k_padding_bottom);
    }
    inline ImoStyle* padding_bottom(LUnits value)
    {
        set_lunits_property(ImoStyle::k_padding_bottom, value);
        return this;
    }
    inline LUnits padding_left()
    {
        return get_lunits_property(ImoStyle::k_padding_left);
    }
    inline ImoStyle* padding_left(LUnits value)
    {
        set_lunits_property(ImoStyle::k_padding_left, value);
        return this;
    }
    inline LUnits padding_right()
    {
        return get_lunits_property(ImoStyle::k_padding_right);
    }
    inline ImoStyle* padding_right(LUnits value)
    {
        set_lunits_property(ImoStyle::k_padding_right, value);
        return this;
    }
    //margin
    inline ImoStyle* margin(LUnits value)
    {
        set_margin_property(value);
        return this;
    }
    inline LUnits margin_top()
    {
        return get_lunits_property(ImoStyle::k_margin_top);
    }
    inline ImoStyle* margin_top(LUnits value)
    {
        set_lunits_property(ImoStyle::k_margin_top, value);
        return this;
    }
    inline LUnits margin_bottom()
    {
        return get_lunits_property(ImoStyle::k_margin_bottom);
    }
    inline ImoStyle* margin_bottom(LUnits value)
    {
        set_lunits_property(ImoStyle::k_margin_bottom, value);
        return this;
    }
    inline LUnits margin_left()
    {
        return get_lunits_property(ImoStyle::k_margin_left);
    }
    inline ImoStyle* margin_left(LUnits value)
    {
        set_lunits_property(ImoStyle::k_margin_left, value);
        return this;
    }
    inline LUnits margin_right()
    {
        return get_lunits_property(ImoStyle::k_margin_right);
    }
    inline ImoStyle* margin_right(LUnits value)
    {
        set_lunits_property(ImoStyle::k_margin_right, value);
        return this;
    }
    //size
    inline LUnits width()
    {
        return get_lunits_property(ImoStyle::k_width);
    }
    inline ImoStyle* width(LUnits value)
    {
        set_lunits_property(ImoStyle::k_width, value);
        return this;
    }
    inline LUnits height()
    {
        return get_lunits_property(ImoStyle::k_height);
    }
    inline ImoStyle* height(LUnits value)
    {
        set_lunits_property(ImoStyle::k_height, value);
        return this;
    }
    inline LUnits min_width()
    {
        return get_lunits_property(ImoStyle::k_min_width);
    }
    inline ImoStyle* min_width(LUnits value)
    {
        set_lunits_property(ImoStyle::k_min_width, value);
        return this;
    }
    inline LUnits min_height()
    {
        return get_lunits_property(ImoStyle::k_min_height);
    }
    inline ImoStyle* min_height(LUnits value)
    {
        set_lunits_property(ImoStyle::k_min_height, value);
        return this;
    }
    inline LUnits max_width()
    {
        return get_lunits_property(ImoStyle::k_max_width);
    }
    inline ImoStyle* max_width(LUnits value)
    {
        set_lunits_property(ImoStyle::k_max_width, value);
        return this;
    }
    inline LUnits max_height()
    {
        return get_lunits_property(ImoStyle::k_max_height);
    }
    inline ImoStyle* max_height(LUnits value)
    {
        set_lunits_property(ImoStyle::k_max_height, value);
        return this;
    }

    //table
    inline LUnits table_col_width()
    {
        return get_lunits_property(ImoStyle::k_table_col_width);
    }
    inline ImoStyle* table_col_width(LUnits value)
    {
        set_lunits_property(ImoStyle::k_table_col_width, value);
        return this;
    }

protected:

    //setters
    void set_string_property(int prop, const string& value)
    {
        m_stringProps[prop] = value;
    }
    void set_float_property(int prop, float value)
    {
        m_floatProps[prop] = value;
    }
    void set_int_property(int prop, int value)
    {
        m_intProps[prop] = value;
    }
    void set_lunits_property(int prop, LUnits value)
    {
        m_lunitsProps[prop] = value;
    }
    void set_color_property(int prop, Color value)
    {
        m_colorProps[prop] = value;
    }

    //special setters
    void set_margin_property(LUnits value)
    {
        set_lunits_property(ImoStyle::k_margin_left, value);
        set_lunits_property(ImoStyle::k_margin_top, value);
        set_lunits_property(ImoStyle::k_margin_right, value);
        set_lunits_property(ImoStyle::k_margin_bottom, value);
    }
    void set_padding_property(LUnits value)
    {
        set_lunits_property(ImoStyle::k_padding_left, value);
        set_lunits_property(ImoStyle::k_padding_top, value);
        set_lunits_property(ImoStyle::k_padding_right, value);
        set_lunits_property(ImoStyle::k_padding_bottom, value);
    }
    void set_border_width_property(LUnits value)
    {
        set_lunits_property(ImoStyle::k_border_width_left, value);
        set_lunits_property(ImoStyle::k_border_width_top, value);
        set_lunits_property(ImoStyle::k_border_width_right, value);
        set_lunits_property(ImoStyle::k_border_width_bottom, value);
    }

    friend class DefineStyleLmdGenerator;
    friend class DefineStyleLdpGenerator;
    friend class DefineStyleMnxGenerator;

    //getters non-inheriting from parent. Returns true if property exists
    bool get_float_property(int prop, float* value)
    {
        map<int, float>::const_iterator it = m_floatProps.find(prop);
        if (it != m_floatProps.end())
        {
            *value = it->second;
            return true;
        }
        else
            return false;
    }

    bool get_lunits_property(int prop, LUnits* value)
    {
        map<int, LUnits>::const_iterator it = m_lunitsProps.find(prop);
        if (it != m_lunitsProps.end())
        {
            *value = it->second;
            return true;
        }
        else
            return false;
    }

    bool get_string_property(int prop, string* value)
    {
        map<int, string>::const_iterator it = m_stringProps.find(prop);
        if (it != m_stringProps.end())
        {
            *value = it->second;
            return true;
        }
        else
            return false;
    }

    bool get_int_property(int prop, int* value)
    {
        map<int, int>::const_iterator it = m_intProps.find(prop);
        if (it != m_intProps.end())
        {
            *value = it->second;
            return true;
        }
        else
            return false;
    }

    bool get_color_property(int prop, Color* value)
    {
        map<int, Color>::const_iterator it = m_colorProps.find(prop);
        if (it != m_colorProps.end())
        {
            *value = it->second;
            return true;
        }
        else
            return false;
    }

    //getters. If value not stored, inherites from parent
    float get_float_property(int prop)
    {
        map<int, float>::const_iterator it = m_floatProps.find(prop);
        if (it != m_floatProps.end())
            return it->second;
        else if (m_pParent)
            return m_pParent->get_float_property(prop);
        else
        {
            LOMSE_LOG_ERROR("Aborting. Style has no parent.");
            throw std::runtime_error( "[ImoStyle::get_float_property]. No parent" );
        }
    }

    LUnits get_lunits_property(int prop)
    {
        map<int, LUnits>::const_iterator it = m_lunitsProps.find(prop);
        if (it != m_lunitsProps.end())
            return it->second;
        else if (m_pParent)
            return m_pParent->get_lunits_property(prop);
        else
        {
            LOMSE_LOG_ERROR("Aborting. Style has no parent.");
            throw std::runtime_error( "[ImoStyle::get_lunits_property]. No parent" );
        }
    }

    const string& get_string_property(int prop)
    {
        map<int, string>::const_iterator it = m_stringProps.find(prop);
        if (it != m_stringProps.end())
            return it->second;
        else if (m_pParent)
            return m_pParent->get_string_property(prop);
        else
        {
            LOMSE_LOG_ERROR("Aborting. Style has no parent.");
            throw std::runtime_error( "[ImoStyle::get_string_property]. No parent" );
        }
    }

    int get_int_property(int prop)
    {
        map<int, int>::const_iterator it = m_intProps.find(prop);
        if (it != m_intProps.end())
            return it->second;
        else if (m_pParent)
            return m_pParent->get_int_property(prop);
        else
        {
            LOMSE_LOG_ERROR("Aborting. Style has no parent.");
            throw std::runtime_error( "[ImoStyle::get_int_property]. No parent" );
        }
    }

    Color get_color_property(int prop)
    {
        map<int, Color>::const_iterator it = m_colorProps.find(prop);
        if (it != m_colorProps.end())
            return it->second;
        else if (m_pParent)
            return m_pParent->get_color_property(prop);
        else
            throw std::runtime_error( "[ImoStyle::get_color_property]. No parent" );
    }

};

//---------------------------------------------------------------------------------------
/** %ImoContentObj is the base class from which any object for the renderizable content
    of a document must derive.

    %ImoContentObj objects have style, location, visibility and can
    have attachments.
*/
class ImoContentObj : public ImoObj
    , public Observable
{
protected:
    ImoStyle* m_pStyle;
    Tenths m_txUserLocation;
    Tenths m_tyUserLocation;
    Tenths m_txUserRefPoint;
    Tenths m_tyUserRefPoint;
    bool m_fVisible;

    ImoContentObj(int objtype);
    ImoContentObj(ImoId id, int objtype);

public:
    virtual ~ImoContentObj();

    //getters
    inline Tenths get_user_location_x()
    {
        return m_txUserLocation;
    }
    inline Tenths get_user_location_y()
    {
        return m_tyUserLocation;
    }
    inline Tenths get_user_ref_point_x()
    {
        return m_txUserRefPoint;
    }
    inline Tenths get_user_ref_point_y()
    {
        return m_tyUserRefPoint;
    }
    inline bool is_visible()
    {
        return m_fVisible;
    }

    //setters
    inline void set_user_location_x(Tenths tx)
    {
        m_txUserLocation = tx;
    }
    inline void set_user_location_y(Tenths ty)
    {
        m_tyUserLocation = ty;
    }
    inline void set_user_ref_point_x(Tenths tx)
    {
        m_txUserRefPoint = tx;
    }
    inline void set_user_ref_point_y(Tenths ty)
    {
        m_tyUserRefPoint = ty;
    }
    inline void set_visible(bool visible)
    {
        m_fVisible = visible;
    }

    //attachments (first child)
    ImoAttachments* get_attachments();
    bool has_attachments();
    int get_num_attachments();
    ImoAuxObj* get_attachment(int i);
    void add_attachment(Document* pDoc, ImoAuxObj* pAO);
    void remove_attachment(ImoAuxObj* pAO);
    ImoAuxObj* find_attachment(int type);

    //overrides for Observable children
    EventNotifier* get_event_notifier();
    void add_event_handler(int eventType, EventHandler* pHandler);
    void add_event_handler(int eventType, void* pThis,
                           void (*pt2Func)(void* pObj, SpEventInfo event) );
    void add_event_handler(int eventType,
                           void (*pt2Func)(SpEventInfo event) );

    //style
    virtual ImoStyle* get_style(bool fInherit=true);
    ImoStyle* get_inherited_style();
    ImoStyle* copy_style_as(const std::string& name);
    void set_style(ImoStyle* pStyle);

    //font
    inline const std::string font_name()
    {
        ImoStyle* pStyle = get_style();
        return pStyle ? pStyle->font_name() : "";
    }

    //margin
    inline LUnits margin_top()
    {
        ImoStyle* pStyle = get_style();
        return pStyle ? pStyle->margin_top() : 0.0f;
    }
    inline LUnits margin_bottom()
    {
        ImoStyle* pStyle = get_style();
        return pStyle ? pStyle->margin_bottom() : 0.0f;
    }

    //IM attributes interface
    void set_int_attribute(TIntAttribute attrib, int value);
    int get_int_attribute(TIntAttribute attrib);
    void set_bool_attribute(TIntAttribute attrib, bool value);
    bool get_bool_attribute(TIntAttribute attrib);
    void set_double_attribute(TIntAttribute attrib, double value);
    double get_double_attribute(TIntAttribute attrib);
    void set_string_attribute(TIntAttribute attrib, const string& value);
    string get_string_attribute(TIntAttribute attrib);
    list<TIntAttribute> get_supported_attributes();

};

//---------------------------------------------------------------------------------------
class ImoCollection : public ImoSimpleObj
{
public:
    virtual ~ImoCollection() {}

    //contents
    inline ImoObj* get_item(int iItem)
    {
        return get_child(iItem);    //iItem = 0..n-1
    }
    inline int get_num_items()
    {
        return get_num_children();
    }
    inline void remove_item(ImoContentObj* pItem)
    {
        remove_child_imo(pItem);
    }

protected:
    ImoCollection(int objtype) : ImoSimpleObj(objtype) {}
};

//---------------------------------------------------------------------------------------
class ImoRelations : public ImoSimpleObj
{
protected:
    std::list<ImoRelObj*> m_relations;

    friend class ImFactory;
    friend class ImoContentObj;
    ImoRelations() : ImoSimpleObj(k_imo_relations) {}

public:
    virtual ~ImoRelations();

    //overrides, to traverse this special node
    void accept_visitor(BaseVisitor& v);
    bool has_visitable_children()
    {
        return get_num_items() > 0;
    }

    //contents
    ImoRelObj* get_item(int iItem);   //iItem = 0..n-1
    inline int get_num_items()
    {
        return int(m_relations.size());
    }
    inline std::list<ImoRelObj*>& get_relations()
    {
        return m_relations;
    }
    void remove_relation(ImoRelObj* pRO);
    void add_relation(ImoRelObj* pRO);
    ImoRelObj* find_item_of_type(int type);
    void remove_from_all_relations(ImoStaffObj* pSO);

protected:
    static int get_priority(int type);
};

//---------------------------------------------------------------------------------------
// ContainerObj: A collection of containers and contained objs.
class ImoContainerObj : public ImoObj
{
protected:
    ImoContainerObj(int objtype) : ImoObj(objtype) {}

public:
    virtual ~ImoContainerObj() {}

};

//---------------------------------------------------------------------------------------
// ImoBlockLevelObj: abstract class for all block-level objects
class ImoBlockLevelObj : public ImoContentObj
{
protected:
    ImoBlockLevelObj(ImoId id, int objtype) : ImoContentObj(id, objtype) {}
    ImoBlockLevelObj(int objtype) : ImoContentObj(objtype) {}

public:
    virtual ~ImoBlockLevelObj() {}

};

//---------------------------------------------------------------------------------------
// ImoBlocksContainer: A block-level container for block-level objects
class ImoBlocksContainer : public ImoBlockLevelObj, public BlockLevelCreatorApi
{
protected:
    ImoBlocksContainer(ImoId id, int objtype)
        : ImoBlockLevelObj(id, objtype)
        , BlockLevelCreatorApi()
    {
        set_box_level_creator_api_parent(this);
    }
    ImoBlocksContainer(int objtype)
        : ImoBlockLevelObj(objtype)
        , BlockLevelCreatorApi()
    {
        set_box_level_creator_api_parent(this);
    }

public:
    virtual ~ImoBlocksContainer() {}

    //support for storing content inside an ImoContent node.
    //Container ImoContent node will be automatically created when storing the
    //first content item (first invocation of append_content_item() )
    int get_num_content_items();
    ImoContent* get_content();
    ImoContentObj* get_content_item(int iItem);
    ImoContentObj* get_first_content_item();
    ImoContentObj* get_last_content_item();
    void append_content_item(ImoContentObj* pItem);

//    //API
//    ImoParagraph* add_paragraph(ImoStyle* pStyle=nullptr);

protected:
//    virtual ImoContentObj* get_container_node()=0;
    void create_content_container(Document* pDoc);

};

//---------------------------------------------------------------------------------------
// ImoInlinesContainer: A block-level container for ImoInlineLevelObj objs.
class ImoInlinesContainer : public ImoBlockLevelObj, public InlineLevelCreatorApi
{
protected:
    ImoInlinesContainer(int objtype)
        : ImoBlockLevelObj(objtype)
        , InlineLevelCreatorApi()
    {
        set_inline_level_creator_api_parent(this);
    }

public:
    virtual ~ImoInlinesContainer() {}

    //contents
    inline int get_num_items()
    {
        return get_num_children();
    }
    inline void remove_item(ImoContentObj* pItem)
    {
        remove_child_imo(pItem);
    }
    inline void add_item(ImoContentObj* pItem)
    {
        append_child_imo(pItem);
    }
    inline ImoContentObj* get_first_item()
    {
        return dynamic_cast<ImoContentObj*>( get_first_child() );
    }

};

////---------------------------------------------------------------------------------------
//class ImoTextBlock : public ImoInlinesContainer, public InlineLevelCreatorApi
//{
//protected:
//    ImoTextBlock(int objtype)
//        : ImoInlinesContainer(objtype)
//        , InlineLevelCreatorApi()
//    {
//        set_inline_level_creator_api_parent(this);
//    }
//
//public:
//    virtual ~ImoTextBlock() {}
//
//    //contents
//    inline int get_num_items() { return get_num_children(); }
//    inline void remove_item(ImoContentObj* pItem) { remove_child_imo(pItem); }
//    inline void add_item(ImoContentObj* pItem) { append_child_imo(pItem); }
//    inline ImoContentObj* get_first_item() {
//        return dynamic_cast<ImoContentObj*>( get_first_child() );
//    }
//
//};

//---------------------------------------------------------------------------------------
// ImoInlineLevelObj: Abstract class from which any ImoInlinesContainer content object must derive
class ImoInlineLevelObj : public ImoContentObj
{
protected:
    ImoInlineLevelObj(int objtype) : ImoContentObj(objtype) {}

public:
    virtual ~ImoInlineLevelObj() {}

};

//---------------------------------------------------------------------------------------
class ImoBoxInline : public ImoInlineLevelObj, public InlineLevelCreatorApi
{
protected:
    USize m_size;

    ImoBoxInline(int objtype)
        : ImoInlineLevelObj(objtype), InlineLevelCreatorApi(), m_size(0.0f, 0.0f)
    {
        set_inline_level_creator_api_parent(this);
    }
    ImoBoxInline(int objtype, const USize& size)
        : ImoInlineLevelObj(objtype), InlineLevelCreatorApi(), m_size(size)
    {
        set_inline_level_creator_api_parent(this);
    }
    ImoBoxInline(int objtype, LUnits width, LUnits height)
        : ImoInlineLevelObj(objtype), InlineLevelCreatorApi(), m_size(width, height)
    {
        set_inline_level_creator_api_parent(this);
    }

public:
    virtual ~ImoBoxInline() {}

    //content
    inline int get_num_items()
    {
        return get_num_children();
    }
    inline void add_item(ImoInlineLevelObj* pItem)
    {
        append_child_imo(pItem);
    }
    inline void remove_item(ImoContentObj* pItem)
    {
        remove_child_imo(pItem);
    }
    inline ImoInlineLevelObj* get_first_item()
    {
        return dynamic_cast<ImoInlineLevelObj*>( get_first_child() );
    }

    //size
    inline USize& get_size()
    {
        return m_size;
    }
    inline LUnits get_width()
    {
        return m_size.width;
    }
    inline LUnits get_height()
    {
        return m_size.height;
    }
    inline void set_size(const USize& size)
    {
        m_size = size;
    }
    inline void set_width(LUnits value)
    {
        m_size.width = value;
    }
    inline void set_height(LUnits value)
    {
        m_size.height = value;
    }

};

//---------------------------------------------------------------------------------------
// An inline-block container for inline objs
class ImoInlineWrapper : public ImoBoxInline
{
protected:
    friend class ImFactory;
    ImoInlineWrapper() : ImoBoxInline(k_imo_inline_wrapper) {}

public:
    virtual ~ImoInlineWrapper() {}
};

//---------------------------------------------------------------------------------------
// An inline-block wrapper to add link properties to wrapped objs
class ImoLink : public ImoBoxInline
{
private:
    string m_url;
    string m_language;

    friend class ImFactory;
    ImoLink() : ImoBoxInline(k_imo_link) {}

public:
    virtual ~ImoLink() {}

    //url
    inline string& get_url()
    {
        return m_url;
    }
    inline void set_url(const string& url)
    {
        m_url = url;
    }
    string& get_language();
    void set_language(const string& value)
    {
        m_language = value;
    }

};

//---------------------------------------------------------------------------------------
// Any atomic displayable object for a Score
class ImoScoreObj : public ImoContentObj
{
protected:
    Color m_color;

    ImoScoreObj(ImoId id, int objtype) : ImoContentObj(id, objtype), m_color(0,0,0) {}
    ImoScoreObj(int objtype) : ImoContentObj(objtype), m_color(0,0,0) {}

public:
    virtual ~ImoScoreObj() {}

    //getters
    inline Color& get_color()
    {
        return m_color;
    }

    //setters
    inline void set_color(Color color)
    {
        m_color = color;
    }

    //IM attributes interface
    void set_int_attribute(TIntAttribute attrib, int value);
    int get_int_attribute(TIntAttribute attrib);
    void set_color_attribute(TIntAttribute attrib, Color value);
    Color get_color_attribute(TIntAttribute attrib);
    list<TIntAttribute> get_supported_attributes();


};

//---------------------------------------------------------------------------------------
// StaffObj: An object attached to an Staff. Consume time
class ImoStaffObj : public ImoScoreObj
{
protected:
    int m_staff;
    int m_measure;
    TimeUnits m_time;

    ImoStaffObj(int objtype)
        : ImoScoreObj(objtype), m_staff(0), m_measure(0), m_time(0.0f) {}
    ImoStaffObj(ImoId id, int objtype)
        : ImoScoreObj(id, objtype), m_staff(0), m_measure(0), m_time(0.0f) {}

public:
    virtual ~ImoStaffObj();

    //relations
    void include_in_relation(Document* pDoc, ImoRelObj* pRelObj,
                             ImoRelDataObj* pData=nullptr);
    void remove_from_relation(ImoRelObj* pRelObj);
    void remove_but_not_delete_relation(ImoRelObj* pRelObj);

    void add_relation(Document* pDoc, ImoRelObj* pRO);
    ImoRelations* get_relations();
    bool has_relations();
    int get_num_relations();
    ImoRelObj* get_relation(int i);
    ImoRelObj* find_relation(int type);

    //getters
    inline TimeUnits get_time()
    {
        return m_time;
    }
    virtual TimeUnits get_duration()
    {
        return 0.0;
    }
    inline int get_staff()
    {
        return m_staff;
    }
    inline int get_measure()
    {
        return m_measure;
    }

    //setters
    virtual void set_staff(int staff)
    {
        m_staff = staff;
    }
    inline void set_time(TimeUnits rTime)
    {
        m_time = rTime;
    }
    inline void set_measure(int measure)
    {
        m_measure = measure;
    }

    //other
    ImoInstrument* get_instrument();
    ImoScore* get_score();

    //IM attributes interface
    virtual void set_int_attribute(TIntAttribute attrib, int value);
    virtual int get_int_attribute(TIntAttribute attrib);
    virtual list<TIntAttribute> get_supported_attributes();

};

//---------------------------------------------------------------------------------------
// AuxObj: a ScoreObj that must be attached to other objects but not
//         directly to an staff. Do not consume time
class ImoAuxObj : public ImoScoreObj
{
public:
    virtual ~ImoAuxObj() {}

protected:
    ImoAuxObj(int objtype) : ImoScoreObj(objtype) {}
    ImoAuxObj(ImoId id, int objtype)
        : ImoScoreObj(id, objtype)
    {
    }

};

//---------------------------------------------------------------------------------------
// AuxRelObj: a type of AuxObj that is related to other AuxObjs of the same type.
class ImoAuxRelObj : public ImoAuxObj
{
protected:
    ImoAuxRelObj* m_prevARO;
    ImoAuxRelObj* m_nextARO;

    ImoAuxRelObj(int objtype)
        : ImoAuxObj(objtype)
        , m_prevARO(nullptr)
        , m_nextARO(nullptr)
    {
    }
    ImoAuxRelObj(ImoId id, int objtype)
        : ImoAuxObj(id, objtype)
        , m_prevARO(nullptr)
        , m_nextARO(nullptr)
    {
    }


public:
    virtual ~ImoAuxRelObj();

    //information
    inline bool is_start_of_relation()
    {
        return m_prevARO == nullptr;
    }
    inline bool is_end_of_relation()
    {
        return m_nextARO == nullptr;
    }

protected:

    void link_to_next_ARO(ImoAuxRelObj* pNext);
    void set_prev_ARO(ImoAuxRelObj* pPrev);


};

//---------------------------------------------------------------------------------------
//An abstract object containing the specific data for one node in a relation
class ImoRelDataObj : public ImoSimpleObj
{
public:
    virtual ~ImoRelDataObj() {}

protected:
    ImoRelDataObj(int objtype) : ImoSimpleObj(objtype) {}

};

//---------------------------------------------------------------------------------------
//An abstract object relating two or more StaffObjs
class ImoRelObj : public ImoScoreObj
{
protected:
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> > m_relatedObjects;

public:
    virtual ~ImoRelObj();

    void push_back(ImoStaffObj* pSO, ImoRelDataObj* pData);
    void remove(ImoStaffObj* pSO);
    void remove_all();
    inline int get_num_objects()
    {
        return int( m_relatedObjects.size() );
    }
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& get_related_objects()
    {
        return m_relatedObjects;
    }
    ImoStaffObj* get_start_object()
    {
        return m_relatedObjects.front().first;
    }
    ImoStaffObj* get_end_object()
    {
        return m_relatedObjects.back().first;
    }
    ImoRelDataObj* get_start_data()
    {
        return m_relatedObjects.front().second;
    }
    ImoRelDataObj* get_end_data()
    {
        return m_relatedObjects.back().second;
    }
    ImoRelDataObj* get_data_for(ImoStaffObj* pSO);

    virtual void reorganize_after_object_deletion()=0;
    virtual int get_min_number_for_autodelete()
    {
        return 2;
    }

protected:
    ImoRelObj(int objtype) : ImoScoreObj(objtype) {}

};

//---------------------------------------------------------------------------------------
class ImoAttachments : public ImoCollection
{
protected:
    friend class ImFactory;
    friend class ImoContentObj;
    ImoAttachments() : ImoCollection(k_imo_attachments) {}

public:
    virtual ~ImoAttachments() {}

    void remove(ImoAuxObj* pAO)
    {
        remove_child_imo(pAO);
    }
    void add(ImoAuxObj* pAO)
    {
        append_child_imo(pAO);
    }
    ImoAuxObj* find_item_of_type(int type)
    {
        return static_cast<ImoAuxObj*>( get_child_of_type(type) );
    }
};


//=======================================================================================
// Simple objects
//=======================================================================================

//---------------------------------------------------------------------------------------
class ImoBeamData : public ImoRelDataObj
{
protected:
    int m_beamNum;
    int m_beamType[6];
    bool m_repeat[6];

    friend class ImFactory;
    ImoBeamData(ImoBeamDto* pDto);

public:
    virtual ~ImoBeamData() {}

    //getters
    inline int get_beam_number()
    {
        return m_beamNum;
    }
    inline int get_beam_type(int level)
    {
        return m_beamType[level];
    }
    inline bool get_repeat(int level)
    {
        return m_repeat[level];
    }

    //setters
    inline void set_beam_type(int level, int type)
    {
        m_beamType[level] = type;
    }

    //properties
    bool is_end_of_beam();
    bool is_start_of_beam();
};

//---------------------------------------------------------------------------------------
class ImoBeamDto : public ImoSimpleObj
{
protected:
    int m_beamType[6];
    int m_beamNum;
    bool m_repeat[6];
    LdpElement* m_pBeamElm;
    ImoNoteRest* m_pNR;
    int m_lineNum;

public:
    ImoBeamDto();
    virtual ~ImoBeamDto() {}

    //getters
    inline int get_beam_number()
    {
        return m_beamNum;
    }
    inline ImoNoteRest* get_note_rest()
    {
        return m_pNR;
    }
    int get_line_number()
    {
        return m_lineNum;
    }
    int get_beam_type(int level);
    bool get_repeat(int level);

    //setters
    inline void set_beam_number(int num)
    {
        m_beamNum = num;
    }
    inline void set_note_rest(ImoNoteRest* pNR)
    {
        m_pNR = pNR;
    }
    void set_beam_type(int level, int type);
    void set_beam_type(string& segments);
    void set_repeat(int level, bool value);
    inline void set_line_number(int value)
    {
        m_lineNum = value;
    }

    //properties
    bool is_end_of_beam();
    bool is_start_of_beam();

    //required by RelationBuilder
    int get_item_number()
    {
        return get_beam_number();
    }
    bool is_start_of_relation()
    {
        return is_start_of_beam();
    }
    bool is_end_of_relation()
    {
        return is_end_of_beam();
    }
    ImoStaffObj* get_staffobj();

};

//---------------------------------------------------------------------------------------
class ImoBezierInfo : public ImoSimpleObj
{
protected:
    TPoint m_tPoints[4];   //start, end, ctrol1, ctrol2

    friend class ImFactory;
    ImoBezierInfo() : ImoSimpleObj(k_imo_bezier_info) {}

public:
    ImoBezierInfo(ImoBezierInfo* pBezier);

    virtual ~ImoBezierInfo() {}

    enum { k_start=0, k_end, k_ctrol1, k_ctrol2, k_max};     // point number

    //points
    inline void set_point(int i, TPoint& value)
    {
        m_tPoints[i] = value;
    }
    inline TPoint& get_point(int i)
    {
        return m_tPoints[i];
    }

};

//---------------------------------------------------------------------------------------
class ImoBorderDto : public ImoSimpleObj
{
    Color         m_color;
    Tenths        m_width;
    ELineStyle    m_style;

public:
    ImoBorderDto() : ImoSimpleObj(k_imo_border_dto)
        , m_color(Color(0,0,0,255)), m_width(1.0f), m_style(k_line_solid) {}
    virtual ~ImoBorderDto() {}

    //getters
    inline Color get_color()
    {
        return m_color;
    }
    inline Tenths get_width()
    {
        return m_width;
    }
    inline ELineStyle get_style()
    {
        return m_style;
    }

    //setters
    inline void set_color(Color value)
    {
        m_color = value;
    }
    inline void set_width(Tenths value)
    {
        m_width = value;
    }
    inline void set_style(ELineStyle value)
    {
        m_style = value;
    }

};

//---------------------------------------------------------------------------------------
class ImoChord : public ImoRelObj
{
protected:
    friend class ImFactory;
    ImoChord() : ImoRelObj(k_imo_chord) {}

public:
    virtual ~ImoChord() {}

    void reorganize_after_object_deletion() {}
};

//---------------------------------------------------------------------------------------
class ImoCursorInfo : public ImoSimpleObj
{
protected:
    int m_instrument;
    int m_staff;
    TimeUnits m_time;
    ImoId m_id;

    friend class ImFactory;
    ImoCursorInfo() : ImoSimpleObj(k_imo_cursor_info)
        , m_instrument(0), m_staff(0), m_time(0.0), m_id(k_no_imoid) {}

public:
    virtual ~ImoCursorInfo() {}

    //getters
    inline int get_instrument()
    {
        return m_instrument;
    }
    inline int get_staff()
    {
        return m_staff;
    }
    inline TimeUnits get_time()
    {
        return m_time;
    }
    inline ImoId get_id()
    {
        return m_id;
    }

    //setters
    inline void set_instrument(int value)
    {
        m_instrument = value;
    }
    inline void set_staff(int value)
    {
        m_staff = value;
    }
    inline void set_time(TimeUnits value)
    {
        m_time = value;
    }
    inline void set_id(ImoId value)
    {
        m_id = value;
    }
};

//---------------------------------------------------------------------------------------
class ImoLineStyle : public ImoSimpleObj
{
protected:
    ELineStyle  m_lineStyle;
    ELineEdge   m_startEdge;
    ELineEdge   m_endEdge;
    ELineCap    m_startStyle;
    ELineCap    m_endStyle;
    Color       m_color;
    Tenths      m_width;
    TPoint      m_startPoint;
    TPoint      m_endPoint;

    friend class ImFactory;
    friend class ImoTextBox;
    friend class ImoLine;
    friend class ImoScoreLine;
    ImoLineStyle()
        : ImoSimpleObj(k_imo_line_style)
        , m_lineStyle(k_line_solid)
        , m_startEdge(k_edge_normal)
        , m_endEdge(k_edge_normal)
        , m_startStyle(k_cap_none)
        , m_endStyle(k_cap_none)
        , m_color(Color(0,0,0,255))
        , m_width(1.0f)
        , m_startPoint(0.0f, 0.0f)
        , m_endPoint(0.0f, 0.0f)
    {
    }

    ImoLineStyle(ImoLineStyle& info)
        : ImoSimpleObj(k_imo_line_style)
        , m_lineStyle( info.get_line_style() )
        , m_startEdge( info.get_start_edge() )
        , m_endEdge( info.get_end_edge() )
        , m_startStyle( info.get_start_cap() )
        , m_endStyle( info.get_end_cap() )
        , m_color( info.get_color() )
        , m_width( info.get_width() )
        , m_startPoint( info.get_start_point() )
        , m_endPoint( info.get_end_point() )
    {
    }

public:
    virtual ~ImoLineStyle() {}

    //getters
    inline ELineStyle get_line_style() const
    {
        return m_lineStyle;
    }
    inline ELineEdge get_start_edge() const
    {
        return m_startEdge;
    }
    inline ELineEdge get_end_edge() const
    {
        return m_endEdge;
    }
    inline ELineCap get_start_cap() const
    {
        return m_startStyle;
    }
    inline ELineCap get_end_cap() const
    {
        return m_endStyle;
    }
    inline Color get_color() const
    {
        return m_color;
    }
    inline Tenths get_width() const
    {
        return m_width;
    }
    inline TPoint get_start_point() const
    {
        return m_startPoint;
    }
    inline TPoint get_end_point() const
    {
        return m_endPoint;
    }

    //setters
    inline void set_line_style(ELineStyle value)
    {
        m_lineStyle = value;
    }
    inline void set_start_edge(ELineEdge value)
    {
        m_startEdge = value;
    }
    inline void set_end_edge(ELineEdge value)
    {
        m_endEdge = value;
    }
    inline void set_start_cap(ELineCap value)
    {
        m_startStyle = value;
    }
    inline void set_end_cap(ELineCap value)
    {
        m_endStyle = value;
    }
    inline void set_color(Color value)
    {
        m_color = value;
    }
    inline void set_width(Tenths value)
    {
        m_width = value;
    }
    inline void set_start_point(TPoint point)
    {
        m_startPoint = point;
    }
    inline void set_end_point(TPoint point)
    {
        m_endPoint = point;
    }

};

//---------------------------------------------------------------------------------------
//The <score-part> element allows for multiple score-instruments per
//score-part. ImoMidiInfo contains the MIDI information for one score-instrument
class ImoMidiInfo : public ImoSimpleObj
{
protected:
    string	m_soundId;          //id of the score-instrument

    //midi device
    int     m_port;			    //port: 0-15 (MIDI 1-16)
    string  m_midiDeviceName;   //DeviceName meta event

    //midi instrument
    string m_midiName;  //name: ProgramName meta-events
    int m_bank;			//bank: 0-16383 (MIDI 1-16,384)
    int m_channel;		//channel: 0-15 (MIDI 1-16)
    int m_program;		//patch: 0-127 (MIDI 1-128)
    int m_unpitched;	//pitch number for percussion: 1-128

    //mixer information
    float m_volume;		//channel volume: 0.0 (muted) to 1.0
    int m_pan;			//degrees: -180 to 180.
    // 0 = in front of the listener, centered
    //-90 = hard left, 90 is hard right
    //-180 and 180 = behind the listener, centered
    int m_elevation;	//degrees: -90 to 90.
    // 0 = listener level
    // 90 = directly above
    //-90 = directly below.

    friend class ImFactory;
    friend class ImoInstrument;
    ImoMidiInfo()
        : ImoSimpleObj(k_imo_midi_info)
        , m_port(-1)
        , m_midiDeviceName("")
        , m_midiName("")
        , m_bank(0)
        , m_channel(-1)
        , m_program(0)
        , m_unpitched(0)
        , m_volume(1.0)
        , m_pan(0)
        , m_elevation(0)
    {
    }

public:
    virtual ~ImoMidiInfo() {}

    //getters
    inline string& get_score_instr_id()
    {
        return 	m_soundId;
    }
    inline int get_midi_port()
    {
        return m_port;
    }
    inline string& get_midi_device_name()
    {
        return m_midiDeviceName;
    }
    inline string& get_midi_name()
    {
        return m_midiName;
    }
    inline int get_midi_bank()
    {
        return m_bank;
    }
    inline int get_midi_channel()
    {
        return m_channel;
    }
    inline int get_midi_program()
    {
        return m_program;
    }
    inline int get_midi_unpitched()
    {
        return m_unpitched;
    }
    inline float get_midi_volume()
    {
        return m_volume;
    }
    inline int get_midi_pan()
    {
        return m_pan;
    }
    inline int get_midi_elevation()
    {
        return m_elevation;
    }

    //setters
    inline void set_score_instr_id(const string& value)
    {
        m_soundId = value;
    }
    inline void set_midi_port(int value)
    {
        m_port = value;
    }
    inline void set_midi_device_name(const string& value)
    {
        m_midiDeviceName = value;
    }
    inline void set_midi_name(const string& value)
    {
        m_midiName = value;
    }
    inline void set_midi_bank(int value)
    {
        m_bank = value;
    }
    inline void set_midi_channel(int value)
    {
        m_channel = value;
    }
    inline void set_midi_program(int value)
    {
        m_program = value;
    }
    inline void set_midi_unpitched(int value)
    {
        m_unpitched = value;
    }
    inline void set_midi_volume(float value)
    {
        m_volume = value;
    }
    inline void set_midi_pan(int value)
    {
        m_pan = value;
    }
    inline void set_midi_elevation(int value)
    {
        m_elevation = value;
    }

};

//---------------------------------------------------------------------------------------
class ImoTextBlockInfo : public ImoSimpleObj
{
//<location>[<size>][<color>][<border>]
protected:
    //block position and size
    TSize     m_size;
    TPoint    m_topLeftPoint;

    ////block position within a possible parent block
    //EHAlign       m_hAlign;
    //EVAlign       m_vAlign;

    //block looking
    Color        m_bgColor;
    Color        m_borderColor;
    Tenths        m_borderWidth;
    ELineStyle    m_borderStyle;

public:
    ImoTextBlockInfo()
        : ImoSimpleObj(k_imo_textblock_info)
        , m_size(TSize(160.0f, 100.0f))
        , m_topLeftPoint(TPoint(0.0f, 0.0f))
        , m_bgColor( Color(255, 255, 255, 255) )
        , m_borderColor( Color(0, 0, 0, 255) )
        , m_borderWidth(1.0f)
        , m_borderStyle(k_line_solid)
    {
    }

    virtual ~ImoTextBlockInfo() {}

    //getters
    inline Tenths get_height()
    {
        return m_size.height;
    }
    inline Tenths get_width()
    {
        return m_size.width;
    }
    inline TPoint get_position()
    {
        return m_topLeftPoint;
    }
    inline Color get_bg_color()
    {
        return m_bgColor;
    }
    inline Color get_border_color()
    {
        return m_borderColor;
    }
    inline Tenths get_border_width()
    {
        return m_borderWidth;
    }
    inline ELineStyle get_border_style()
    {
        return m_borderStyle;
    }

    //setters
    inline void set_position_x(Tenths value)
    {
        m_topLeftPoint.x = value;
    }
    inline void set_position_y(Tenths value)
    {
        m_topLeftPoint.y = value;
    }
    inline void set_size(TSize size)
    {
        m_size = size;
    }
    inline void set_bg_color(Color color)
    {
        m_bgColor = color;
    }
    inline void set_border(ImoBorderDto* pBorder)
    {
        m_borderColor = pBorder->get_color();
        m_borderWidth = pBorder->get_width();
        m_borderStyle = pBorder->get_style();
    }
};

//---------------------------------------------------------------------------------------
//The score-instrument element allows for multiple instruments per
//score-part. Contains the information for an instrument in an score-part
class ImoSoundInfo : public ImoSimpleObj
{
protected:
    string	m_soundId;          //id of the score-instrument
    string	m_instrName;		//not oriented to appearing printed on the score
    string	m_instrAbbrev;		//not oriented to appearing printed on the score
    string	m_instrSound;		//describes the default timbre of the score-instrument
    //Allows playback to be shared more easily between
    //applications and libraries.

    //performance data: a solo instrument? or an ensemble?
    bool	m_fSolo;			//performance by a solo instrument
    bool	m_fEnsemble;		//performance by an ensemble
    int		m_ensembleSize;		//if ensemble, the size of the section, or
    //0 if the ensemble size is not specified.

    //defines the specific virtual instrument used for sound.
    string	m_virtualLibrary;		//virtual library name
    string	m_virtualName;			//virtual instrument to use

    //defines play technique to use for all notes played back with this instrument
    int     m_playTechnique;


    friend class ImFactory;
    friend class ImoInstrument;
    ImoSoundInfo();
    void initialize_object(Document* pDoc) override;

public:
    virtual ~ImoSoundInfo() {}

    //getters
    inline string& get_score_instr_id()
    {
        return m_soundId;
    }
    inline string& get_score_instr_name()
    {
        return m_instrName;
    }
    inline string& get_score_instr_abbrev()
    {
        return m_instrAbbrev;
    }
    inline string& get_score_instr_sound()
    {
        return m_instrSound;
    }
    inline bool	get_score_instr_solo()
    {
        return m_fSolo;
    }
    inline bool	get_score_instr_ensemble()
    {
        return m_fEnsemble;
    }
    inline int get_score_instr_ensemble_size()
    {
        return m_ensembleSize;
    }
    inline string& get_score_instr_virtual_library()
    {
        return m_virtualLibrary;
    }
    inline string& get_score_instr_virtual_name()
    {
        return m_virtualName;
    }

    //setters
    void set_score_instr_id(const string& value);
    inline void set_score_instr_name(const string& value)
    {
        m_instrName = value;
    }
    inline void set_score_instr_abbrev(const string& value)
    {
        m_instrAbbrev = value;
    }
    inline void set_score_instr_sound(const string& value)
    {
        m_instrSound = value;
    }
    inline void set_score_instr_solo(bool value)
    {
        m_fSolo = value;
    }
    inline void set_score_instr_ensemble(bool value)
    {
        m_fEnsemble = value;
    }
    inline void set_score_instr_ensemble_size(int value)
    {
        m_ensembleSize = value;
    }
    inline void set_score_instr_virtual_library(const string& value)
    {
        m_virtualLibrary = value;
    }
    inline void set_score_instr_virtual_name(const string& value)
    {
        m_virtualName = value;
    }

    //access to ImoMidiInfo child
    ImoMidiInfo* get_midi_info();

};

//---------------------------------------------------------------------------------------
class ImoTextInfo : public ImoSimpleObj
{
protected:
    string m_text;
    string m_language;
    ImoStyle* m_pStyle;

    friend class ImFactory;
    friend class ImoTextBox;
    friend class ImoButton;
    friend class ImoScoreText;
    friend class ImoTextItem;
    friend class ImoLyricsTextInfo;
    friend class ImoTextRepetitionMark;
    ImoTextInfo() : ImoSimpleObj(k_imo_text_info), m_text(""), m_language(""), m_pStyle(nullptr) {}

public:
    virtual ~ImoTextInfo() {}

    //getters
    inline string& get_text()
    {
        return m_text;
    }
    inline string& get_language()
    {
        return m_language;
    }
    inline ImoStyle* get_style()
    {
        return m_pStyle;
    }
    const std::string& get_font_name();
    float get_font_size();
    int get_font_style();
    int get_font_weight();
    Color get_color();

    //setters
    inline void set_text(const string& text)
    {
        m_text = text;
    }
    inline void set_style(ImoStyle* pStyle)
    {
        m_pStyle = pStyle;
    }
    inline void set_language(const string& language)
    {
        m_language = language;
    }
};

//---------------------------------------------------------------------------------------
class ImoPageInfo : public ImoSimpleObj
{
protected:
    LUnits  m_uLeftMargin;
    LUnits  m_uRightMargin;
    LUnits  m_uTopMargin;
    LUnits  m_uBottomMargin;
    LUnits  m_uBindingMargin;
    USize   m_uPageSize;
    bool    m_fPortrait;

    friend class ImFactory;
    friend class ImoDocument;
    friend class ImoScore;
    ImoPageInfo();
    ImoPageInfo(ImoPageInfo& dto);

public:
    virtual ~ImoPageInfo() {}

    //getters
    inline LUnits get_left_margin()
    {
        return m_uLeftMargin;
    }
    inline LUnits get_right_margin()
    {
        return m_uRightMargin;
    }
    inline LUnits get_top_margin()
    {
        return m_uTopMargin;
    }
    inline LUnits get_bottom_margin()
    {
        return m_uBottomMargin;
    }
    inline LUnits get_binding_margin()
    {
        return m_uBindingMargin;
    }
    inline bool is_portrait()
    {
        return m_fPortrait;
    }
    inline USize get_page_size()
    {
        return m_uPageSize;
    }
    inline LUnits get_page_width()
    {
        return m_uPageSize.width;
    }
    inline LUnits get_page_height()
    {
        return m_uPageSize.height;
    }

    //setters
    inline void set_left_margin(LUnits value)
    {
        m_uLeftMargin = value;
    }
    inline void set_right_margin(LUnits value)
    {
        m_uRightMargin = value;
    }
    inline void set_top_margin(LUnits value)
    {
        m_uTopMargin = value;
    }
    inline void set_bottom_margin(LUnits value)
    {
        m_uBottomMargin = value;
    }
    inline void set_binding_margin(LUnits value)
    {
        m_uBindingMargin = value;
    }
    inline void set_portrait(bool value)
    {
        m_fPortrait = value;
    }
    inline void set_page_size(USize uPageSize)
    {
        m_uPageSize = uPageSize;
    }
    inline void set_page_width(LUnits value)
    {
        m_uPageSize.width = value;
    }
    inline void set_page_height(LUnits value)
    {
        m_uPageSize.height = value;
    }
};


//=======================================================================================
// Real objects
//=======================================================================================

//---------------------------------------------------------------------------------------
class ImoAnonymousBlock : public ImoInlinesContainer
{
protected:
    friend class ImoBlocksContainer;
    friend class Document;
    friend class ImFactory;
    ImoAnonymousBlock() : ImoInlinesContainer(k_imo_anonymous_block) {}

public:
    virtual ~ImoAnonymousBlock() {}

    //required by Visitable parent class
    virtual void accept_visitor(BaseVisitor& v);
};

//---------------------------------------------------------------------------------------
/** %ImoBarline represents a barline for an instrument.
*/
class ImoBarline : public ImoStaffObj
{
protected:
    int m_barlineType;
    bool m_fMiddle;


    int m_times;        //for k_barline_end_repetition and k_barline_double_repetition,
    //indicates the number of times the repeated section must be
    //played. If there are volta brackets m_times is updated with
    //the number of times implied by the volta bracket.

    int m_winged;       //indicates whether the barline has winged extensions above and
    //below. The k_winged_none value indicates no wings.

    TypeMeasureInfo* m_pMeasureInfo;    //ptr to info for measure ending with this barline.
                                        //nullptr when middle barline


    friend class ImFactory;
    ImoBarline()
        : ImoStaffObj(k_imo_barline)
        , m_barlineType(k_barline_simple)
        , m_fMiddle(false)
        , m_times(0)
        , m_winged(k_wings_none)
        , m_pMeasureInfo(nullptr)
    {
    }

public:
    virtual ~ImoBarline();

    //getters
    inline int get_type() { return m_barlineType; }
    inline bool is_middle() { return m_fMiddle; }
    inline int get_num_repeats() { return m_times; }
    inline int get_winged() { return m_winged; }
    inline TypeMeasureInfo* get_measure_info() { return m_pMeasureInfo; }


    //setters
    inline void set_type(int barlineType) { m_barlineType = barlineType; }
    inline void set_middle(bool value) { m_fMiddle = value; }
    inline void set_num_repeats(int times) { m_times = times; }
    inline void set_winged(int wingsType) { m_winged = wingsType; }
    inline void set_measure_info(TypeMeasureInfo* pInfo) { m_pMeasureInfo = pInfo; }

    //overrides: barlines always in staff 0
    void set_staff(int UNUSED(staff)) { m_staff = 0; }

    //IM attributes interface
    virtual void set_int_attribute(TIntAttribute attrib, int value);
    virtual int get_int_attribute(TIntAttribute attrib);
    virtual list<TIntAttribute> get_supported_attributes();

};

//---------------------------------------------------------------------------------------
class ImoBeam : public ImoRelObj
{
protected:
    friend class ImFactory;
    ImoBeam() : ImoRelObj(k_imo_beam) {}

public:
    virtual ~ImoBeam() {}

    //type of beam
    enum { k_none = 0, k_begin, k_continue, k_end, k_forward, k_backward, };

    void reorganize_after_object_deletion();
    int get_max_staff();
    int get_min_staff();
};

//---------------------------------------------------------------------------------------
class ImoBlock : public ImoAuxObj
{
protected:
    ImoTextBlockInfo m_box;

    ImoBlock(int objtype) : ImoAuxObj(objtype) {}
    ImoBlock(int objtype, ImoTextBlockInfo& box) : ImoAuxObj(objtype), m_box(box) {}

public:
    virtual ~ImoBlock() {}

};

//---------------------------------------------------------------------------------------
class ImoTextBox : public ImoBlock
{
protected:
    string m_text;
    ImoLineStyle m_line;
    bool m_fHasAnchorLine;
    //TPoint m_anchorJoinPoint;     //point on the box rectangle

    friend class ImFactory;
    ImoTextBox() : ImoBlock(k_imo_text_box), m_fHasAnchorLine(false) {}
    ImoTextBox(ImoTextBlockInfo& box) : ImoBlock(k_imo_text_box, box)
        , m_fHasAnchorLine(false) {}

public:
    virtual ~ImoTextBox() {}

    inline ImoTextBlockInfo* get_box_info() { return &m_box;
    }
    inline ImoLineStyle* get_anchor_line_info() { return &m_line;
    }
    inline bool has_anchor_line()
    {
        return m_fHasAnchorLine;
    }

    inline const std::string& get_text()
    {
        return m_text;
    }
    inline void set_text(ImoTextInfo* pTI)
    {
        m_text = pTI->get_text();
    }
    inline void set_anchor_line(ImoLineStyle* pLine)
    {
        m_line = *pLine;
        m_fHasAnchorLine = true;
    }
};

//---------------------------------------------------------------------------------------
// ImoControl: An inline wrapper for defining GUI controls (GUI interactive component).
class ImoControl : public ImoInlineLevelObj
{
protected:
    Control* m_ctrol;

    friend class ImFactory;
    ImoControl(Control* ctrol) : ImoInlineLevelObj(k_imo_control), m_ctrol(ctrol) {}
    ImoControl(int type) : ImoInlineLevelObj(type), m_ctrol(nullptr) {}

    friend class InlineLevelCreatorApi;
    void attach_control(Control* ctrol);

public:
    virtual ~ImoControl() {}

    //delegates on its associated Control for determining its size
    USize measure();

    //delegates on its associated Control for generating its graphical model
    GmoBoxControl* layout(LibraryScope& libraryScope, UPoint pos);

    //other
    inline Control* get_control()
    {
        return m_ctrol;
    }

};

//---------------------------------------------------------------------------------------
class ImoButton : public ImoControl
{
protected:
    string m_text;
    string m_language;
    USize m_size;
    Color m_bgColor;
    bool m_fEnabled;

    friend class ImFactory;
    ImoButton();

public:
    virtual ~ImoButton() {}

    //getters
    inline string& get_label()
    {
        return m_text;
    }
    inline string& get_language()
    {
        return m_language;
    }
    inline USize get_size()
    {
        return m_size;
    }
    inline LUnits get_width()
    {
        return m_size.width;
    }
    inline LUnits get_height()
    {
        return m_size.height;
    }
    inline Color get_bg_color()
    {
        return m_bgColor;
    }

    //setters
    inline void set_style(ImoStyle* pStyle)
    {
        ImoContentObj::set_style(pStyle);
        if (pStyle)
            m_bgColor = pStyle->background_color();
    }
    inline void set_bg_color(Color color)
    {
        m_bgColor = color;
    }
    inline void set_label(const string& text)
    {
        m_text = text;
    }
    inline void set_language(const string& value)
    {
        m_language = value;
    }
    inline void set_size(const USize& size)
    {
        m_size = size;
    }
    inline void set_width(LUnits value)
    {
        m_size.width = value;
    }
    inline void set_height(LUnits value)
    {
        m_size.height = value;
    }

    //getters
    inline bool is_enabled()
    {
        return m_fEnabled;
    }

    //setters
    inline void enable(bool value)
    {
        m_fEnabled = value;
    }
};

//---------------------------------------------------------------------------------------
class ImoClef : public ImoStaffObj
{
protected:
    int m_clefType;
    int m_symbolSize;

    friend class ImFactory;
    ImoClef()
        : ImoStaffObj(k_imo_clef)
        , m_clefType(k_clef_G2)
        , m_symbolSize(k_size_default)
    {
    }

public:
    virtual ~ImoClef() {}

    //getters and setters
    inline int get_clef_type()
    {
        return m_clefType;
    }
    inline void set_clef_type(int type)
    {
        m_clefType = type;
    }
    inline int get_symbol_size()
    {
        return m_symbolSize;
    }
    inline void set_symbol_size(int symbolSize)
    {
        m_symbolSize = symbolSize;
    }

    //properties
    bool can_generate_secondary_shapes()
    {
        return true;
    }

};

//---------------------------------------------------------------------------------------
/** %ImoContent object is a generic block-level container, that is a wrapper container
    for ImoBlockLevelObj objects.

    %ImoContent object can be compared to a @<div@> element in HTML. It is used for
    wrapping content.

    In a well formed document this object is a child of ImoDocument, must always
    exist, and there must be only one %ImoContent child.

    The children of %ImoContent are ImoBlockLevelObj objects, representing the main
    blocks of the document content, such as paragraphs, headings, scores, lists or
    tables.

    For example:
@verbatim

                             ImoDocument
                                  |
                             ImoContent
                                  |
         +----------------+----------------+------------------+
         |                |                |                  |
     ImoHeading     ImoParagraph       ImoScore         ImoParagraph

.
@endverbatim
*/
class ImoContent : public ImoBlocksContainer
{
protected:
    friend class ImFactory;
    ImoContent() : ImoBlocksContainer(k_imo_content) {}
    ImoContent(int objtype) : ImoBlocksContainer(objtype) {}

public:
    virtual ~ImoContent() {}

    inline Document* get_owner()
    {
        return m_pDoc;
    }

    //contents
    ImoContentObj* get_item(int iItem)     //iItem = 0..n-1
    {
        return dynamic_cast<ImoContentObj*>( get_child(iItem) );
    }
    inline int get_num_items()
    {
        return get_num_children();
    }
    inline void remove_item(ImoContentObj* pItem)
    {
        remove_child_imo(pItem);
    }

};

//---------------------------------------------------------------------------------------
/** ImoDirection represents a discrete instruction in the score that applies to
    notated events.
*/
class ImoDirection : public ImoStaffObj
{
protected:
    Tenths  m_space;
    int     m_placement;

    // When the <direction> element contains a <sound> children related to repetition
    // marks or a <direction-type> children of type <segno>/<coda>, or a <words>
    // related to repetition marks, these members contains the type of mark.
    // Otherwise, contains k_repeat_none.
    int	    m_displayRepeat;
    int     m_soundRepeat;


    friend class ImFactory;
    ImoDirection()
        : ImoStaffObj(k_imo_direction)
        , m_space(0.0f)
        , m_placement(k_placement_default)
        , m_displayRepeat(k_repeat_none)
        , m_soundRepeat(k_repeat_none)
    {
    }

public:
    virtual ~ImoDirection() {}

    //getters
    inline Tenths get_width() { return m_space; }
    inline int get_placement() { return m_placement; }
    inline int get_display_repeat() { return m_displayRepeat; }
    inline int get_sound_repeat() { return m_soundRepeat; }

    //setters
    inline void set_width(Tenths space) { m_space = space; }
    inline void set_placement(int placement) { m_placement = placement; }
    inline void set_display_repeat(int repeat) { m_displayRepeat = repeat; }
    inline void set_sound_repeat(int repeat) { m_soundRepeat = repeat; }

    //info
    inline bool is_display_repeat() { return m_displayRepeat != k_repeat_none; }
    inline bool is_sound_repeat() { return m_soundRepeat != k_repeat_none; }
};

////---------------------------------------------------------------------------------------
//class ImoDirectionSymbol : public ImoAuxObj
//{
//protected:
//    ImoTextInfo m_text;
//
//    friend class ImFactory;
//    ImoDirectionSymbol() : ImoAuxObj(k_imo_score_text), m_text() {}
//    ImoDirectionSymbol(int objtype) : ImoAuxObj(objtype), m_text() {}
//
//public:
//    virtual ~ImoDirectionSymbol() {}
//
//    //getters
//    inline string& get_text() { return m_text.get_text(); }
//
//    //setters
//    inline void set_text(const std::string& value) { m_text.set_text(value); }
//
//};

//---------------------------------------------------------------------------------------
class ImoSymbolRepetitionMark : public ImoAuxObj
{
protected:
    int m_symbol;       //a value from enum ESymbolRepetitionMark

    friend class ImFactory;
    ImoSymbolRepetitionMark()
        : ImoAuxObj(k_imo_symbol_repetition_mark)
        , m_symbol(ImoSymbolRepetitionMark::k_undefined)
    {
    }

public:
    virtual ~ImoSymbolRepetitionMark() {}

    enum ESymbolRepetitionMark
    {
        k_undefined = 0,
        k_coda,
        k_segno,
    };

    //getters
    inline int get_symbol()
    {
        return m_symbol;
    }

    //setters
    inline void set_symbol(int value)
    {
        m_symbol = value;
    }

};

//---------------------------------------------------------------------------------------
class ImoDynamic : public ImoContent
{
protected:
    string m_classid;
    std::list<ImoParamInfo*> m_params;

    friend class ImFactory;
    ImoDynamic() : ImoContent(k_imo_dynamic), m_classid("") {}

public:
    virtual ~ImoDynamic();

    //construction
    inline void set_classid(const string& value)
    {
        m_classid = value;
    }
    inline void add_param(ImoParamInfo* pParam)
    {
        m_params.push_back(pParam);
    }

    //accessors
    inline string& get_classid()
    {
        return m_classid;
    }
    inline std::list<ImoParamInfo*>& get_params()
    {
        return m_params;
    }

};

//---------------------------------------------------------------------------------------
class ImoSystemBreak : public ImoStaffObj
{
protected:
    friend class ImFactory;
    ImoSystemBreak() : ImoStaffObj(k_imo_system_break) {}

public:
    virtual ~ImoSystemBreak() {}
};

//---------------------------------------------------------------------------------------
/** %ImoDocument is the root object of the internal model. It contains some data, such
    as the default language used in the document, the list of all styles defined
    in the document, and the information about the paper size with its margins.

    Its only child is an ImoContent object, wrapping all the document content.

    @verbatim
                             ImoDocument
                                  |
                             ImoContent
                                  |

    .
    @endverbatim
*/
class ImoDocument : public ImoBlocksContainer
{
protected:
    friend class Document;
    string m_version;
    string m_language;
    ImoPageInfo m_pageInfo;
    std::list<ImoStyle*> m_privateStyles;

    friend class ImFactory;
    ImoDocument(const std::string& version="");

public:
    virtual ~ImoDocument();

    //info
    inline std::string& get_version() { return m_version; }
    inline void set_version(const string& version) { m_version = version; }
    inline Document* get_owner() { return m_pDoc; }
    inline std::string& get_language() { return m_language; }
    inline void set_language(const string& language) { m_language = language; }

    //document intended paper size
    void add_page_info(ImoPageInfo* pPI);
    inline ImoPageInfo* get_page_info() { return &m_pageInfo; }
    inline LUnits get_paper_width() { return m_pageInfo.get_page_width(); }
    inline LUnits get_paper_height() { return m_pageInfo.get_page_height(); }

    //styles
    ImoStyles* get_styles();
    void add_style(ImoStyle* pStyle);
    ImoStyle* find_style(const std::string& name);
    ImoStyle* get_default_style();
    ImoStyle* get_style_or_default(const std::string& name);

    //user API
    ImoStyle* create_style(const string& name, const string& parent="Default style");
    ImoStyle* create_private_style(const string& parent="Default style");

    //support for edition commands
    void insert_block_level_obj(ImoBlockLevelObj* pAt, ImoBlockLevelObj* pImoNew);
    void delete_block_level_obj(ImoBlockLevelObj* pAt);

    //cursor
    //TODO: method add_cursor_info
    void add_cursor_info(ImoCursorInfo* UNUSED(pCursor)) {};

protected:
    void add_private_style(ImoStyle* pStyle);

};

//---------------------------------------------------------------------------------------
class ImoFermata : public ImoAuxObj
{
protected:
    int m_placement;
    int m_symbol;

    friend class ImFactory;
    ImoFermata()
        : ImoAuxObj(k_imo_fermata)
        , m_placement(k_placement_default)
        , m_symbol(k_normal)
    {
    }

public:
    virtual ~ImoFermata() {}

    enum { k_normal, k_short, k_long, k_henze_short, k_henze_long,
           k_very_short, k_very_long,
         };

    //getters
    inline int get_placement()
    {
        return m_placement;
    }
    inline int get_symbol()
    {
        return m_symbol;
    }

    //setters
    inline void set_placement(int placement)
    {
        m_placement = placement;
    }
    inline void set_symbol(int symbol)
    {
        m_symbol = symbol;
    }

};

//---------------------------------------------------------------------------------------
class ImoArticulation : public ImoAuxObj
{
protected:
    int m_articulationType;
    int m_placement;

    ImoArticulation(int objtype)
        : ImoAuxObj(objtype)
        , m_articulationType(k_articulation_unknown)
        , m_placement(k_placement_default)
    {
    }

public:
    virtual ~ImoArticulation() {}

    //getters
    inline int get_placement()
    {
        return m_placement;
    }
    inline int get_articulation_type()
    {
        return m_articulationType;
    }

    //setters
    inline void set_placement(int placement)
    {
        m_placement = placement;
    }
    inline void set_articulation_type(int articulationType)
    {
        m_articulationType = articulationType;
    }

};

//---------------------------------------------------------------------------------------
class ImoArticulationSymbol : public ImoArticulation
{
protected:
    bool m_fUp;     //only for k_articulation_marccato
    int m_symbol;   //symbol to use when alternatives. For now only for breath_mark

    friend class ImFactory;
    ImoArticulationSymbol()
        : ImoArticulation(k_imo_articulation_symbol)
        , m_fUp(true)
        , m_symbol(k_default)
    {
    }

public:
    virtual ~ImoArticulationSymbol() {}

    enum
    {
        k_default=0,
        k_breath_comma, k_breath_tick, k_breath_v, k_breath_salzedo,
        k_caesura_normal, k_caesura_thick, k_caesura_short, k_caesura_curved,
    };

    //getters
    inline bool is_up()
    {
        return m_fUp;
    }
    inline int get_symbol()
    {
        return m_symbol;
    }

    //setters
    inline void set_up(bool value)
    {
        m_fUp = value;
    }
    inline void set_symbol(int value)
    {
        m_symbol = value;
    }

    //info
    inline bool is_accent()
    {
        return m_articulationType >= k_articulation_accent
               && m_articulationType <= k_articulation_tenuto;
    }
    inline bool is_stress()
    {
        return m_articulationType >= k_articulation_stress
               && m_articulationType <= k_articulation_unstress;
    }
    inline bool is_breath_mark()
    {
        return m_articulationType >= k_articulation_breath_mark
               && m_articulationType <= k_articulation_caesura;
    }
    inline bool is_jazz_pitch()
    {
        return m_articulationType >= k_articulation_scoop
               && m_articulationType <= k_articulation_falloff;
    }

};

//---------------------------------------------------------------------------------------
class ImoArticulationLine : public ImoArticulation
{
protected:
    int m_lineShape;        //straight | curved
    int m_lineType;         //solid | dashed | dotted | wavy
    Tenths m_dashLength;    //only for dashed lines
    Tenths m_dashSpace;     //only for dashed lines

    friend class ImFactory;
    ImoArticulationLine()
        : ImoArticulation(k_imo_articulation_line)
        , m_lineShape(k_line_shape_straight)
        , m_lineType(k_line_type_solid)
        , m_dashLength(4.0)
        , m_dashSpace(2.0)
    {
    }

public:
    virtual ~ImoArticulationLine() {}

    //getters
    inline int get_line_shape()
    {
        return m_lineShape;
    }
    inline int get_line_type()
    {
        return m_lineType;
    }
    inline Tenths get_dash_length()
    {
        return m_dashLength;
    }
    inline Tenths get_dash_space()
    {
        return m_dashSpace;
    }

    //setters
    inline void set_line_shape(int lineShape)
    {
        m_lineShape = lineShape;
    }
    inline void set_line_type(int lineType)
    {
        m_lineType = lineType;
    }
    inline void set_dash_length(Tenths length)
    {
        m_dashLength = length;
    }
    inline void set_dash_space(Tenths space)
    {
        m_dashSpace = space;
    }

};

//---------------------------------------------------------------------------------------
class ImoDynamicsMark : public ImoAuxObj
{
protected:
    string m_markType;
    int m_placement;
//TODO
//    %text-decoration;
//    %enclosure;

    friend class ImFactory;
    ImoDynamicsMark()
        : ImoAuxObj(k_imo_dynamics_mark)
        , m_markType("")
        , m_placement(k_placement_default)
    {
    }

public:
    virtual ~ImoDynamicsMark() {}

    //getters
    inline int get_placement()
    {
        return m_placement;
    }
    inline string get_mark_type()
    {
        return m_markType;
    }

    //setters
    inline void set_placement(int placement)
    {
        m_placement = placement;
    }
    inline void set_mark_type(const string& markType)
    {
        m_markType = markType;
    }

};

//---------------------------------------------------------------------------------------
class ImoOrnament : public ImoAuxObj
{
protected:
    int m_ornamentType;
    int m_placement;
//TODO
//    %text-decoration;
//    %enclosure;

    friend class ImFactory;
    ImoOrnament()
        : ImoAuxObj(k_imo_ornament)
        , m_ornamentType(k_ornament_unknown)
        , m_placement(k_placement_default)
    {
    }

public:
    virtual ~ImoOrnament() {}

    //getters
    inline int get_placement()
    {
        return m_placement;
    }
    inline int get_ornament_type()
    {
        return m_ornamentType;
    }

    //setters
    inline void set_placement(int placement)
    {
        m_placement = placement;
    }
    inline void set_ornament_type(int ornamentType)
    {
        m_ornamentType = ornamentType;
    }

};

//---------------------------------------------------------------------------------------
class ImoTechnical : public ImoAuxObj
{
protected:
    int m_technicalType;
    int m_placement;

    friend class ImFactory;
    ImoTechnical()
        : ImoAuxObj(k_imo_technical)
        , m_technicalType(k_technical_unknown)
        , m_placement(k_placement_default)
    {
    }

public:
    virtual ~ImoTechnical() {}

    //getters
    inline int get_placement()
    {
        return m_placement;
    }
    inline int get_technical_type()
    {
        return m_technicalType;
    }

    //setters
    inline void set_placement(int placement)
    {
        m_placement = placement;
    }
    inline void set_technical_type(int technicalType)
    {
        m_technicalType = technicalType;
    }

};

//---------------------------------------------------------------------------------------
class ImoGoBackFwd : public ImoStaffObj
{
protected:
    bool        m_fFwd;
    TimeUnits   m_rTimeShift;

    const TimeUnits SHIFT_START_END;     //any too big value

    friend class ImFactory;
    ImoGoBackFwd()
        : ImoStaffObj(k_imo_go_back_fwd), m_fFwd(true), m_rTimeShift(0.0)
        , SHIFT_START_END(100000000.0)
    {}

public:
    virtual ~ImoGoBackFwd() {}

    //getters and setters
    inline bool is_forward()
    {
        return m_fFwd;
    }
    inline void set_forward(bool fFwd)
    {
        m_fFwd = fFwd;
    }
    inline bool is_to_start()
    {
        return !m_fFwd && (m_rTimeShift == -SHIFT_START_END);
    }
    inline bool is_to_end()
    {
        return m_fFwd && (m_rTimeShift == SHIFT_START_END);
    }
    inline TimeUnits get_time_shift()
    {
        return m_rTimeShift;
    }
    inline void set_to_start()
    {
        set_time_shift(SHIFT_START_END);
    }
    inline void set_to_end()
    {
        set_time_shift(SHIFT_START_END);
    }
    inline void set_time_shift(TimeUnits rTime)
    {
        m_rTimeShift = (m_fFwd ? rTime : -rTime);
    }
};

//---------------------------------------------------------------------------------------
class ImoScoreText : public ImoAuxObj
{
protected:
    ImoTextInfo m_text;

    friend class ImFactory;
    friend class ImoInstrument;
    friend class ImoInstrGroup;
    ImoScoreText() : ImoAuxObj(k_imo_score_text), m_text() {}
    ImoScoreText(int objtype) : ImoAuxObj(objtype), m_text() {}

public:
    virtual ~ImoScoreText() {}

    //getters
    inline string& get_text()
    {
        return m_text.get_text();
    }
    inline ImoTextInfo* get_text_info() { return &m_text;
    }
    string& get_language();
    inline void set_language(const string& lang)
    {
        m_text.set_language(lang);
    }
    inline bool has_language()
    {
        return !m_text.get_language().empty();
    }

    //setters
    inline void set_text(const std::string& value)
    {
        m_text.set_text(value);
    }

};

//---------------------------------------------------------------------------------------
class ImoScoreTitle : public ImoScoreText
{
protected:
    int m_hAlign;

    friend class ImFactory;
    ImoScoreTitle() : ImoScoreText(k_imo_score_title), m_hAlign(k_halign_center) {}

public:
    virtual ~ImoScoreTitle() {}

    inline int get_h_align()
    {
        return m_hAlign;
    }
    inline void set_h_align(int value)
    {
        m_hAlign = value;
    }
//    inline void set_style(ImoStyle* pStyle) { m_text.set_style(pStyle); }
//    inline ImoStyle* get_style() { return m_text.get_style(); }
};

//---------------------------------------------------------------------------------------
class ImoSoundChange : public ImoStaffObj
{
protected:

    friend class ImFactory;
    ImoSoundChange()
        : ImoStaffObj(k_imo_sound_change)
    {
    }

public:
    virtual ~ImoSoundChange() {}

    ImoMidiInfo* get_midi_info(const string& soundId);


};

//---------------------------------------------------------------------------------------
class ImoTextRepetitionMark : public ImoScoreText
{
protected:
    int m_repeatType;

    friend class ImFactory;
    ImoTextRepetitionMark()
        : ImoScoreText(k_imo_text_repetition_mark)
        , m_repeatType(0)
    {
    }

public:
    virtual ~ImoTextRepetitionMark() {}

    //getters
    inline int get_repeat_mark()
    {
        return m_repeatType;
    }

    //setters
    inline void set_repeat_mark(int repeat)
    {
        m_repeatType = repeat;
    }

};

//---------------------------------------------------------------------------------------
class ImoImage : public ImoInlineLevelObj
{
protected:
    SpImage m_image;

    friend class ImFactory;
    ImoImage() : ImoInlineLevelObj(k_imo_image), m_image( LOMSE_NEW Image() ) {}
    ImoImage(unsigned char* imgbuf, VSize bmpSize, EPixelFormat format, USize imgSize)
        : ImoInlineLevelObj(k_imo_image)
        , m_image( LOMSE_NEW Image(imgbuf, bmpSize, format, imgSize) )
    {
    }
    friend class ImageAnalyser;
    friend class ImageLmdAnalyser;
    friend class ImageMxlAnalyser;
    friend class ImageMnxAnalyser;
    inline void set_content(SpImage img)
    {
        m_image = img;
    }

public:
    virtual ~ImoImage() {}

    //accessors
    SpImage get_image()
    {
        return m_image;
    }
    inline unsigned char* get_buffer()
    {
        return m_image->get_buffer();
    }
    inline LUnits get_image_width()
    {
        return m_image->get_image_width();
    }
    inline LUnits get_image_height()
    {
        return m_image->get_image_height();
    }
    inline USize& get_image_size()
    {
        return m_image->get_image_size();
    }
    inline Pixels get_bitmap_width()
    {
        return m_image->get_bitmap_width();
    }
    inline Pixels get_bitmap_height()
    {
        return m_image->get_bitmap_height();
    }
    inline VSize& get_bitmap_size()
    {
        return m_image->get_bitmap_size();
    }
    inline int get_stride()
    {
        return m_image->get_stride();
    }
    inline int get_format()
    {
        return m_image->get_format();
    }

    inline int get_bits_per_pixel()
    {
        return m_image->get_bits_per_pixel();
    }
    inline bool has_alpha()
    {
        return m_image->has_alpha();
    }

protected:
    friend class InlineLevelCreatorApi;
    inline void load(unsigned char* imgbuf, VSize bmpSize, EPixelFormat format,
                     USize imgSize)
    {
        m_image->load(imgbuf, bmpSize, format, imgSize);
    }

};

//---------------------------------------------------------------------------------------
class ImoInstrGroup : public ImoSimpleObj
{
protected:
    ImoScore* m_pScore;
    int m_joinBarlines;     // enum k_no, k_standard, k_mensurstrich
    int m_symbol;           // enum k_none, k_default, k_brace, k_bracket, ...
    ImoScoreText m_name;
    ImoScoreText m_abbrev;
    std::list<ImoInstrument*> m_instruments;

    friend class ImFactory;
    ImoInstrGroup();

    friend class ImoScore;
    inline void set_owner_score(ImoScore* pScore)
    {
        m_pScore = pScore;
    }

public:
    virtual ~ImoInstrGroup();

    enum { k_none=0, k_brace, k_bracket, k_line, };
    enum { k_no=0, k_standard, k_mensurstrich, };

    //getters
    inline int join_barlines()
    {
        return m_joinBarlines;
    }
    inline int get_symbol()
    {
        return m_symbol;
    }
    inline const std::string& get_name_string()
    {
        return m_name.get_text();
    }
    inline const std::string& get_abbrev_string()
    {
        return m_abbrev.get_text();
    }
    inline ImoScoreText& get_name()
    {
        return m_name;
    }
    inline ImoScoreText& get_abbrev()
    {
        return m_abbrev;
    }

    //setters
    void set_name(ImoScoreText* pText);
    void set_abbrev(ImoScoreText* pText);
    inline void set_symbol(int symbol)
    {
        m_symbol = symbol;
    }
    inline void set_join_barlines(int value)
    {
        m_joinBarlines = value;
    }

    //instruments
    inline list<ImoInstrument*>& get_instruments()
    {
        return m_instruments;
    }
    void add_instrument(ImoInstrument* pInstr);
    ImoInstrument* get_instrument(int iInstr);   //0..n-1
    int get_num_instruments();
    ImoInstrument* get_first_instrument()
    {
        return m_instruments.front();
    }
    ImoInstrument* get_last_instrument()
    {
        return m_instruments.back();
    }

    //info
    inline ImoScore* get_score()
    {
        return m_pScore;
    }
    inline bool has_name()
    {
        return m_name.get_text() != "";
    }
    inline bool has_abbrev()
    {
        return m_abbrev.get_text() != "";
    }

};

//---------------------------------------------------------------------------------------
/** %ImoInstrument represents the musical content and attributes for an score part
    ('instrument', in lomse terminology).

    Layout options are modeled by specific member variables. Perhaps, in future, if
    there are many options, they could be modeled by an ImoOptions child
*/
class ImoInstrument : public ImoContainerObj
{
protected:
    ImoScore*       m_pScore;
    ImoScoreText    m_name;
    ImoScoreText    m_abbrev;
    string          m_partId;
    std::list<ImoStaffInfo*> m_staves;

    //layout options
    int     m_barlineLayout;        //from enum EBarlineLayout
    int     m_measuresNumbering;    //from enum EMeasuresNumbering

    //measures info
    ImMeasuresTable*  m_pMeasures;
    TypeMeasureInfo*  m_pLastMeasureInfo;   //for last measure if not closed or the score
                                            //has no metric. Otherwise it will be nullptr.

    friend class ImFactory;
    ImoInstrument();

    friend class ImoScore;
    friend class ImoInstrGroup;
    inline void set_owner_score(ImoScore* pScore) { m_pScore = pScore; }

public:
    virtual ~ImoInstrument();

    //methods for accessing ImoSounds child
    LOMSE_DECLARE_IMOSOUNDS_INTERFACE;

    //getters
    inline int get_num_staves() const { return static_cast<int>(m_staves.size()); }
    inline ImoScoreText& get_name() { return m_name; }
    inline ImoScoreText& get_abbrev() { return m_abbrev; }
    ImoMusicData* get_musicdata();
    ImoStaffInfo* get_staff(int iStaff);
    LUnits get_line_spacing_for_staff(int iStaff);
    inline const string& get_instr_id() const { return m_partId; }
    inline ImMeasuresTable* get_measures_table() const { return m_pMeasures; }
    inline TypeMeasureInfo* get_last_measure_info() { return m_pLastMeasureInfo; }

    //setters
    ImoStaffInfo* add_staff();
    void set_name(ImoScoreText* pText);
    void set_abbrev(ImoScoreText* pText);
    void set_name(const string& value);
    void set_abbrev(const string& value);
    void replace_staff_info(ImoStaffInfo* pInfo);
    inline void set_instr_id(const string& id) { m_partId = id; }
    inline void set_last_measure_info(TypeMeasureInfo* pInfo) { m_pLastMeasureInfo = pInfo; }

        //layout options

    /// Valid values for defining the policy to layout barlines.
    enum EBarlineLayout
    {
        k_isolated=0,       ///< Independent barlines for each score part
        k_joined,           ///< Barlines joined across all parts
        k_mensurstrich,     ///< Barlines only in the gaps between parts, but not on
                            ///< the staves of each part
        k_nothing,          ///< Do not display barlines (???)
    };

    /// Valid values for measure numbering global policy.
    enum EMeasuresNumbering
    {
        k_none=0,   ///< Measure numbers are never displayed
        k_system,   ///< Display measure numbers only at start of each system
        k_all,      ///< Display measure numbers in all measures
    };

    /** Barlines layout describes the policy for laying out barlines.
    */
    inline int get_barline_layout() const { return m_barlineLayout; }
    inline void set_barline_layout(int value) { m_barlineLayout = value; }

    /** Measures numbering describes the policy for displaying numbers in measures
        for this instrument. The value set in the policy can be overridden at each
        measure.
    */
    inline int get_measures_numbering() const { return m_measuresNumbering; }
    inline void set_measures_numbering(int value) { m_measuresNumbering = value; }


    //info
    inline bool has_name() { return m_name.get_text() != ""; }
    inline bool has_abbrev() { return m_abbrev.get_text() != ""; }
    LUnits tenths_to_logical(Tenths value, int iStaff=0);
    inline ImoScore* get_score() const { return m_pScore; }

    //direct creation API
    ImoBarline* add_barline(int type, bool fVisible=true);
    ImoClef* add_clef(int type, int nStaff=1, bool fVisible=true);
    ImoKeySignature* add_key_signature(int type, bool fVisible=true);
    ImoTimeSignature* add_time_signature(int top, int bottom, bool fVisible=true);
    ImoDirection* add_spacer(Tenths space);
    ImoObj* add_object(const string& ldpsource);
    void add_staff_objects(const string& ldpsource);

    //score edition API
    void delete_staffobj(ImoStaffObj* pImo);
    void insert_staffobj(ImoStaffObj* pPos, ImoStaffObj* pImo);
    void insert_staffobj_after(ImoStaffObj* pPos, ImoStaffObj* pImo);
    ImoStaffObj* insert_staffobj_at(ImoStaffObj* pAt, ImoStaffObj* pImo);
    ImoStaffObj* insert_staffobj_at(ImoStaffObj* pAt, const string& ldpsource,
                                    ostream& reporter=cout);
    list<ImoStaffObj*> insert_staff_objects_at(ImoStaffObj* pAt, ImoMusicData* pObjects);
    list<ImoStaffObj*> insert_staff_objects_at(ImoStaffObj* pAt, const string& ldpsource,
            ostream& reporter);

protected:
    friend class LdpAnalyser;
    friend class MxlAnalyser;
    friend class MnxAnalyser;
    friend class InstrumentAnalyser;

    //FIX: For lyrics space
    void reserve_space_for_lyrics(int iStaff, LUnits space);

    friend class MeasuresTableBuilder;
    inline void set_measures_table(ImMeasuresTable* pTable) { m_pMeasures = pTable; }

};

//---------------------------------------------------------------------------------------
class ImoInstruments : public ImoCollection
{
protected:
    friend class ImFactory;
    ImoInstruments() : ImoCollection(k_imo_instruments) {}

public:
    virtual ~ImoInstruments() {}

};

//---------------------------------------------------------------------------------------
class ImoInstrGroups : public ImoCollection
{
protected:
    friend class ImFactory;
    ImoInstrGroups() : ImoCollection(k_imo_instrument_groups) {}

public:
    virtual ~ImoInstrGroups() {}

};

//---------------------------------------------------------------------------------------
class ImoSounds : public ImoCollection
{
protected:
    friend class ImFactory;
    ImoSounds() : ImoCollection(k_imo_sounds) {}

public:
    virtual ~ImoSounds() {}

    void add_sound_info(ImoSoundInfo* pInfo);
    int get_num_sounds();
    ImoSoundInfo* get_sound_info(const string& soundId);
    ImoSoundInfo* get_sound_info(int iSound);

};

//---------------------------------------------------------------------------------------
class ImoKeySignature : public ImoStaffObj
{
protected:

    //Variable only valid for standard key signatures
    //Standard key signatures are represented by the number of flats and sharps, plus an
    //optional mode for major/minor/other distinctions.
    /** Number of flats or sharps (negative numbers are used for flats and positive
        numbers for sharps), reflecting the key's placement within the circle of fifths
        (hence the name of this variable).
    */
    int m_fifths;
    int m_keyMode;      ///< A value from EKeyModes

    friend class ImFactory;
    ImoKeySignature()
        : ImoStaffObj(k_imo_key_signature)
        , m_fifths(0)
        , m_keyMode(k_key_mode_major)
    {
    }

public:
    virtual ~ImoKeySignature() {}

    //getters and setters
    int get_key_type();
    void set_key_type(int type);
    inline int get_fifths() { return m_fifths; }
    inline void set_fifths(int fifths) { m_fifths = fifths; }

    //operations
    void transpose(const int semitones);

    //overrides: key signatures always in staff 0
    void set_staff(int UNUSED(staff))
    {
        m_staff = 0;
    }

    //properties
    bool can_generate_secondary_shapes()
    {
        return true;
    }

};

//---------------------------------------------------------------------------------------
class ImoLine : public ImoAuxObj
{
    ImoLineStyle* m_pStyle;

    friend class ImFactory;
    ImoLine() : ImoAuxObj(k_imo_line), m_pStyle(nullptr) {}

public:
    virtual ~ImoLine()
    {
        delete m_pStyle;
    }

    inline ImoLineStyle* get_line_info() { return m_pStyle;
    }
    inline void set_line_style(ImoLineStyle* pStyle)
    {
        m_pStyle = pStyle;
    }

};

//---------------------------------------------------------------------------------------
class ImoListItem : public ImoBlocksContainer
{
protected:
    friend class Document;
    friend class ImFactory;
    ImoListItem(Document* pDoc);    //pDoc is needed for unit tests

public:
    virtual ~ImoListItem() {}

//   //required by Visitable parent class
    //virtual void accept_visitor(BaseVisitor& v);
};

//---------------------------------------------------------------------------------------
class ImoList : public ImoBlocksContainer
{
protected:
    int m_listType;

    friend class ImFactory;
    ImoList(Document* pDoc);

public:
    virtual ~ImoList() {}

    enum { k_itemized=0, k_ordered, };

    inline void set_list_type(int type)
    {
        m_listType = type;
    }
    inline int get_list_type()
    {
        return m_listType;
    }

    //API
    ImoListItem* add_listitem(ImoStyle* pStyle=nullptr);

    //helper, to access content
    inline ImoListItem* get_list_item(int iItem)     //iItem = 0..n-1
    {
        return dynamic_cast<ImoListItem*>( get_content_item(iItem) );
    }

protected:
    friend class BlockLevelCreatorApi;

};

//---------------------------------------------------------------------------------------
class ImoMetronomeMark : public ImoAuxObj
{
protected:
    int     m_markType;
    int     m_ticksPerMinute;
    int     m_leftNoteType;
    int     m_leftDots;
    int     m_rightNoteType;
    int     m_rightDots;
    bool    m_fParenthesis;

    friend class ImFactory;
    ImoMetronomeMark()
        : ImoAuxObj(k_imo_metronome_mark), m_markType(k_value)
        , m_ticksPerMinute(60), m_leftNoteType(0), m_leftDots(0)
        , m_rightNoteType(0), m_rightDots(0), m_fParenthesis(false)
    {}

public:
    virtual ~ImoMetronomeMark() {}

    enum { k_note_value=0, k_note_note, k_value, };

    //getters
    inline int get_left_note_type()
    {
        return m_leftNoteType;
    }
    inline int get_right_note_type()
    {
        return m_rightNoteType;
    }
    inline int get_left_dots()
    {
        return m_leftDots;
    }
    inline int get_right_dots()
    {
        return m_rightDots;
    }
    inline int get_ticks_per_minute()
    {
        return m_ticksPerMinute;
    }
    inline int get_mark_type()
    {
        return m_markType;
    }
    inline bool has_parenthesis()
    {
        return m_fParenthesis;
    }

    //setters
    inline void set_left_note_type(int noteType)
    {
        m_leftNoteType = noteType;
    }
    inline void set_right_note_type(int noteType)
    {
        m_rightNoteType = noteType;
    }
    inline void set_left_dots(int dots)
    {
        m_leftDots = dots;
    }
    inline void set_right_dots(int dots)
    {
        m_rightDots = dots;
    }
    inline void set_ticks_per_minute(int ticks)
    {
        m_ticksPerMinute = ticks;
    }
    inline void set_mark_type(int type)
    {
        m_markType = type;
    }
    inline void set_parenthesis(bool fValue)
    {
        m_fParenthesis = fValue;
    }
};

//---------------------------------------------------------------------------------------
class ImoMultiColumn : public ImoBlocksContainer
{
protected:
    std::vector<float> m_widths;

    friend class ImFactory;
    ImoMultiColumn(Document* pDoc);

public:
    virtual ~ImoMultiColumn() {}

    //contents
    inline int get_num_columns()
    {
        return get_num_content_items();
    }
    inline ImoContent* get_column(int iCol)     //iCol = 0..n-1
    {
        return dynamic_cast<ImoContent*>( get_content_item(iCol) );
    }

    void set_column_width(int iCol, float percentage);
    float get_column_width(int iCol);

protected:
    friend class BlockLevelCreatorApi;
    void create_columns(int numCols, Document* pDoc);

};

//---------------------------------------------------------------------------------------
class ImoMusicData : public ImoCollection
{
protected:
    friend class ImFactory;
    ImoMusicData() : ImoCollection(k_imo_music_data) {}

public:
    virtual ~ImoMusicData() {}

    ImoInstrument* get_instrument();
};

//---------------------------------------------------------------------------------------
class ImoOptionInfo : public ImoSimpleObj
{
protected:
    int         m_type;
    string      m_name;
    string      m_sValue;
    bool        m_fValue;
    long        m_nValue;
    float       m_rValue;

    friend class ImFactory;
    ImoOptionInfo()
        : ImoSimpleObj(k_imo_option), m_type(k_boolean), m_name("")
        , m_fValue(false), m_nValue(0L), m_rValue(0.0f)  {}

public:
    virtual ~ImoOptionInfo() {}

    enum { k_boolean=0, k_number_long, k_number_float, k_string };

    //getters
    inline string get_name()
    {
        return m_name;
    }
    inline int get_type()
    {
        return m_type;
    }
    inline bool get_bool_value()
    {
        return m_fValue;
    }
    inline long get_long_value()
    {
        return m_nValue;
    }
    inline float get_float_value()
    {
        return m_rValue;
    }
    inline string& get_string_value()
    {
        return m_sValue;
    }
    inline bool is_bool_option()
    {
        return m_type == k_boolean;
    }
    inline bool is_long_option()
    {
        return m_type == k_number_long;
    }
    inline bool is_float_option()
    {
        return m_type == k_number_float;
    }

    //setters
    inline void set_name(const string& name)
    {
        m_name = name;
    }
    inline void set_type(int type)
    {
        m_type = type;
    }
    inline void set_bool_value(bool value)
    {
        m_fValue = value;
        m_type = k_boolean;
    }
    inline void set_long_value(long value)
    {
        m_nValue = value;
        m_type = k_number_long;
    }
    inline void set_float_value(float value)
    {
        m_rValue = value;
        m_type = k_number_float;
    }
    inline void set_string_value(const string& value)
    {
        m_sValue = value;
    }

};

//---------------------------------------------------------------------------------------
class ImoOptions : public ImoCollection
{
protected:
    friend class ImFactory;
    ImoOptions() : ImoCollection(k_imo_options) {}

public:
    virtual ~ImoOptions() {}

};

//---------------------------------------------------------------------------------------
class ImoParagraph : public ImoInlinesContainer
{
protected:
    friend class ImoBlocksContainer;
    friend class Document;
    friend class ImFactory;
    ImoParagraph()
        : ImoInlinesContainer(k_imo_para)
    {
        set_edit_terminal(true);
    }

public:
    virtual ~ImoParagraph() {}

    //required by Visitable parent class
    virtual void accept_visitor(BaseVisitor& v);
};

//---------------------------------------------------------------------------------------
class ImoParamInfo : public ImoSimpleObj
{
protected:
    string m_name;
    string m_value;

    friend class ImFactory;
    ImoParamInfo() : ImoSimpleObj(k_imo_param_info), m_name(), m_value() {}

public:
    virtual ~ImoParamInfo() {}

    //getters
    inline string& get_name()
    {
        return m_name;
    }
    inline string& get_value()
    {
        return m_value;
    }
    bool get_value_as_int(int* pNumber);

    //setters
    inline void set_name(const string& name)
    {
        m_name = name;
    }
    inline void set_value(const string& value)
    {
        m_value = value;
    }

};

//---------------------------------------------------------------------------------------
class ImoHeading : public ImoInlinesContainer
{
protected:
    int m_level;

    friend class ImFactory;
    ImoHeading()
        : ImoInlinesContainer(k_imo_heading)
        , m_level(1)
    {
        set_edit_terminal(true);
    }

public:
    virtual ~ImoHeading() {};

    //level
    inline int get_level()
    {
        return m_level;
    }
    inline void set_level(int level)
    {
        m_level = level;
    }

    //required by Visitable parent class
    virtual void accept_visitor(BaseVisitor& v);

};

//---------------------------------------------------------------------------------------
class ImoPointDto : public ImoSimpleObj
{
    TPoint m_point;

public:
    ImoPointDto() : ImoSimpleObj(k_imo_point_dto) {}
    virtual ~ImoPointDto() {}

    inline TPoint get_point()
    {
        return m_point;
    }

    inline void set_x(Tenths x)
    {
        m_point.x = x;
    }
    inline void set_y(Tenths y)
    {
        m_point.y = y;
    }

};

//---------------------------------------------------------------------------------------
// A graphical line
class ImoScoreLine : public ImoAuxObj
{
protected:
    TPoint m_startPoint;
    TPoint m_endPoint;
    ImoLineStyle m_style;

    friend class ImFactory;
    ImoScoreLine()
        : ImoAuxObj(k_imo_score_line)
        , m_startPoint(0.0f, 0.0f)
        , m_endPoint(0.0f, 0.0f)
        , m_style()
    {
        m_style.set_line_style(k_line_solid);
        m_style.set_start_edge(k_edge_normal);
        m_style.set_end_edge(k_edge_normal);
        m_style.set_start_cap(k_cap_none);
        m_style.set_end_cap(k_cap_none);
        m_style.set_color(Color(0,0,0));
        m_style.set_width(1.0f);
    }

public:
    virtual ~ImoScoreLine() {}

    //setters
    inline void set_start_point(TPoint point)
    {
        m_startPoint = point;
    }
    inline void set_end_point(TPoint point)
    {
        m_endPoint = point;
    }
    inline void set_x_start(Tenths value)
    {
        m_startPoint.x = value;
    }
    inline void set_y_start(Tenths value)
    {
        m_startPoint.y = value;
    }
    inline void set_x_end(Tenths value)
    {
        m_endPoint.x = value;
    }
    inline void set_y_end(Tenths value)
    {
        m_endPoint.y = value;
    }
    inline void set_line_style(ELineStyle value)
    {
        m_style.set_line_style(value);
    }
    inline void set_start_edge(ELineEdge value)
    {
        m_style.set_start_edge(value);
    }
    inline void set_end_edge(ELineEdge value)
    {
        m_style.set_end_edge(value);
    }
    inline void set_start_cap(ELineCap value)
    {
        m_style.set_start_cap(value);
    }
    inline void set_end_cap(ELineCap value)
    {
        m_style.set_end_cap(value);
    }
    inline void set_color(Color value)
    {
        m_style.set_color(value);
    }
    inline void set_width(Tenths value)
    {
        m_style.set_width(value);
    }

    //getters
    inline Tenths get_x_start() const
    {
        return m_startPoint.x;
    }
    inline Tenths get_y_start() const
    {
        return m_startPoint.y;
    }
    inline Tenths get_x_end() const
    {
        return m_endPoint.x;
    }
    inline Tenths get_y_end() const
    {
        return m_endPoint.y;
    }
    inline TPoint get_start_point() const
    {
        return m_startPoint;
    }
    inline TPoint get_end_point() const
    {
        return m_endPoint;
    }
    inline Tenths get_line_width() const
    {
        return m_style.get_width();
    }
    inline Color get_color() const
    {
        return m_style.get_color();
    }
    inline ELineEdge get_start_edge() const
    {
        return m_style.get_start_edge();
    }
    inline ELineEdge get_end_edge() const
    {
        return m_style.get_end_edge();
    }
    inline ELineStyle get_line_style() const
    {
        return m_style.get_line_style();
    }
    inline ELineCap get_start_cap() const
    {
        return m_style.get_start_cap();
    }
    inline ELineCap get_end_cap() const
    {
        return m_style.get_end_cap();
    }
};


//---------------------------------------------------------------------------------------
// ImoScorePlayer: A control for managing score playback
class ImoScorePlayer : public ImoControl
{
protected:
    ScorePlayerCtrl* m_pPlayer;
    ImoScore* m_pScore;
    string m_playLabel;
    string m_stopLabel;

    friend class ImFactory;
    ImoScorePlayer();

    friend class ScorePlayerAnalyser;
    friend class ScorePlayerLmdAnalyser;
    inline void attach_score(ImoScore* pScore)
    {
        m_pScore = pScore;
    }
    void attach_player(ScorePlayerCtrl* pPlayer);
    void set_metronome_mm(int value);
    void set_play_label(const string& value);
    inline void set_stop_label(const string& value)
    {
        m_stopLabel = value;
    }

public:
    virtual ~ImoScorePlayer();

    inline ImoScore* get_score()
    {
        return m_pScore;
    }
    inline ScorePlayerCtrl* get_player()
    {
        return m_pPlayer;
    }
    int get_metronome_mm();
    inline const string& get_play_label()
    {
        return m_playLabel;
    }
    inline const string& get_stop_label()
    {
        return m_stopLabel;
    }

};

//---------------------------------------------------------------------------------------
class ImoSizeDto : public ImoSimpleObj
{
    TSize m_size;

public:
    ImoSizeDto() : ImoSimpleObj(k_imo_size_dto) {}
    virtual ~ImoSizeDto() {}

    inline TSize get_size()
    {
        return m_size;
    }

    inline void set_width(Tenths w)
    {
        m_size.width = w;
    }
    inline void set_height(Tenths h)
    {
        m_size.height = h;
    }

};

//---------------------------------------------------------------------------------------
class ImoSystemInfo : public ImoSimpleObj
{
protected:
    bool    m_fFirst;   //true=first, false=other
    LUnits   m_leftMargin;
    LUnits   m_rightMargin;
    LUnits   m_systemDistance;
    LUnits   m_topSystemDistance;

    friend class ImFactory;
    friend class ImoScore;
    ImoSystemInfo();
    ImoSystemInfo(ImoSystemInfo& dto);

public:
    virtual ~ImoSystemInfo() {}

    //getters
    inline bool is_first()
    {
        return m_fFirst;
    }
    inline LUnits get_left_margin()
    {
        return m_leftMargin;
    }
    inline LUnits get_right_margin()
    {
        return m_rightMargin;
    }
    inline LUnits get_system_distance()
    {
        return m_systemDistance;
    }
    inline LUnits get_top_system_distance()
    {
        return m_topSystemDistance;
    }

    //setters
    inline void set_first(bool fValue)
    {
        m_fFirst = fValue;
    }
    inline void set_left_margin(LUnits rValue)
    {
        m_leftMargin = rValue;
    }
    inline void set_right_margin(LUnits rValue)
    {
        m_rightMargin = rValue;
    }
    inline void set_system_distance(LUnits rValue)
    {
        m_systemDistance = rValue;
    }
    inline void set_top_system_distance(LUnits rValue)
    {
        m_topSystemDistance = rValue;
    }
};

//---------------------------------------------------------------------------------------
class ImoScore : public ImoBlockLevelObj      //ImoBlocksContainer
{
protected:
    int             m_version;
    ColStaffObjs*   m_pColStaffObjs;
    SoundEventsTable* m_pMidiTable;
    ImoSystemInfo   m_systemInfoFirst;
    ImoSystemInfo   m_systemInfoOther;
    ImoPageInfo     m_pageInfo;
    list<ImoScoreTitle*> m_titles;
    map<string, ImoStyle*> m_nameToStyle;

    friend class ImFactory;
    ImoScore(Document* pDoc);
    void initialize();


public:
    virtual ~ImoScore();

    //getters and setters
    string get_version_string();
    inline int get_version_major()
    {
        return m_version/100;
    }
    inline int get_version_minor()
    {
        return m_version % 100;
    }
    inline int get_version_number()
    {
        return m_version;
    }
    inline void set_version(int version)
    {
        m_version = version;
    }
    inline ColStaffObjs* get_staffobjs_table()
    {
        return m_pColStaffObjs;
    }
    void set_staffobjs_table(ColStaffObjs* pColStaffObjs);
    SoundEventsTable* get_midi_table();

    //required by Visitable parent class
    void accept_visitor(BaseVisitor& v);

    //instruments
    void add_instrument(ImoInstrument* pInstr, const string& partId="");
    ImoInstrument* get_instrument(int iInstr);   //0..n-1
    ImoInstrument* get_instrument(const string& partId);
    int get_num_instruments();
    ImoInstruments* get_instruments();
    int get_instr_number_for(ImoInstrument* pInstr);

    //instrument groups
    void add_instruments_group(ImoInstrGroup* pGroup);
    ImoInstrGroups* get_instrument_groups();

    //options
    ImoOptions* get_options();
    bool has_options();
    ImoOptionInfo* get_option(const std::string& name);
    void set_float_option(const std::string& name, float value);
    void set_bool_option(const std::string& name, bool value);
    void set_long_option(const std::string& name, long value);
    void add_or_replace_option(ImoOptionInfo* pOpt);
    bool has_default_value(ImoOptionInfo* pOpt);

    //score layout
    void add_sytem_info(ImoSystemInfo* pSL);
    inline ImoSystemInfo* get_first_system_info() { return &m_systemInfoFirst;
    }
    inline ImoSystemInfo* get_other_system_info() { return &m_systemInfoOther;
    }
    void add_page_info(ImoPageInfo* pPI);
    inline ImoPageInfo* get_page_info() { return &m_pageInfo;
    }
    bool has_default_values(ImoSystemInfo* pInfo);


    //titles
    void add_title(ImoScoreTitle* pTitle);
    inline std::list<ImoScoreTitle*>& get_titles()
    {
        return m_titles;
    }

    //styles
    void add_style(ImoStyle* pStyle);
    void add_required_text_styles();
    ImoStyle* find_style(const std::string& name);
    ImoStyle* get_default_style();
    ImoStyle* get_style_or_default(const std::string& name);

    //low level edition API
    /** Append a new empty instrument (score part) to the score. Returns a pointer to
        the created instrument. */
    ImoInstrument* add_instrument();

    /** When you modify the content of an score it is necessary to update associated
        structures, such as the staffobjs collection. For this it is mandatory to
        invoke this method. Alternatively, you can invoke Document::end_of_changes(),
        that will invoke this method on all scores. */
    void end_of_changes();


protected:
    void add_option(ImoOptionInfo* pOpt);
    void delete_text_styles();
    ImoStyle* create_default_style();
    void set_defaults_for_system_info();
    void set_defaults_for_options();

    friend class ScoreLdpGenerator;
    inline map<std::string, ImoStyle*>& get_styles()
    {
        return m_nameToStyle;
    }

};


//---------------------------------------------------------------------------------------
class ImoSlur : public ImoRelObj
{
protected:
    int     m_slurNum;
    int     m_orientation;
    Color   m_color;

    friend class ImFactory;
    ImoSlur()
        : ImoRelObj(k_imo_slur), m_slurNum(0), m_orientation(k_orientation_default)
    {}
    ImoSlur(int num)
        : ImoRelObj(k_imo_slur), m_slurNum(num), m_orientation(k_orientation_default)
    {}

public:
    virtual ~ImoSlur() {}

    //getters
    inline int get_slur_number()
    {
        return m_slurNum;
    }
    inline int get_orientation()
    {
        return m_orientation;
    }
    ImoNote* get_start_note();
    ImoNote* get_end_note();

    //setters
    inline void set_slur_number(int num)
    {
        m_slurNum = num;
    }
    inline void set_orientation(int value)
    {
        m_orientation = value;
    }
    inline void set_color(Color value)
    {
        m_color = value;
    }

    //access to data objects
    ImoBezierInfo* get_start_bezier();
    ImoBezierInfo* get_stop_bezier() ;
    ImoBezierInfo* get_start_bezier_or_create();
    ImoBezierInfo* get_stop_bezier_or_create();

    void reorganize_after_object_deletion();
};

//---------------------------------------------------------------------------------------
class ImoSlurData : public ImoRelDataObj
{
protected:
    bool    m_fStart;
    int     m_slurNum;
    int     m_orientation;
    ImoBezierInfo* m_pBezier;

    friend class ImFactory;
    ImoSlurData(ImoSlurDto* pDto);

public:
    virtual ~ImoSlurData();

    //getters
    inline bool is_start()
    {
        return m_fStart;
    }
    inline int get_slur_number()
    {
        return m_slurNum;
    }
    inline int get_orientation()
    {
        return m_orientation;
    }
    inline ImoBezierInfo* get_bezier()
    {
        return m_pBezier;
    }

    //edition
    ImoBezierInfo* add_bezier();
};

// raw info about a pending slur
//---------------------------------------------------------------------------------------
class ImoSlurDto : public ImoSimpleObj
{
protected:
    bool m_fStart;
    int m_slurNum;
    int m_orientation;
    ImoNote* m_pNote;
    ImoBezierInfo* m_pBezier;
    Color m_color;
    int m_lineNum;

public:
    ImoSlurDto()
        : ImoSimpleObj(k_imo_slur_dto)
        , m_fStart(true)
        , m_slurNum(0)
        , m_orientation(k_orientation_default)
        , m_pNote(nullptr)
        , m_pBezier(nullptr)
        , m_lineNum(0)
    {
    }
    virtual ~ImoSlurDto();

    //getters
    inline bool is_start()
    {
        return m_fStart;
    }
    inline int get_slur_number()
    {
        return m_slurNum;
    }
    inline int get_orientation()
    {
        return m_orientation;
    }
    inline ImoNote* get_note()
    {
        return m_pNote;
    }
    inline ImoBezierInfo* get_bezier()
    {
        return m_pBezier;
    }
    int get_line_number()
    {
        return m_lineNum;
    }
    inline Color get_color()
    {
        return m_color;
    }

    //setters
    inline void set_start(bool value)
    {
        m_fStart = value;
    }
    inline void set_slur_number(int num)
    {
        m_slurNum = num;
    }
    inline void set_orientation(int value)
    {
        m_orientation = value;
    }
    inline void set_note(ImoNote* pNote)
    {
        m_pNote = pNote;
    }
    inline void set_bezier(ImoBezierInfo* pBezier)
    {
        m_pBezier = pBezier;
    }
    inline void set_color(Color value)
    {
        m_color = value;
    }
    inline void set_line_number(int value)
    {
        m_lineNum = value;
    }

    //required by RelationBuilder
    int get_item_number()
    {
        return get_slur_number();
    }
    bool is_start_of_relation()
    {
        return is_start();
    }
    bool is_end_of_relation()
    {
        return !is_start();
    }
    ImoStaffObj* get_staffobj();
};

//---------------------------------------------------------------------------------------
class ImoStaffInfo : public ImoSimpleObj
{
protected:
    int m_numStaff;
    int m_nNumLines;
    int m_staffType;
    LUnits m_uSpacing;      //between line centers
    LUnits m_uLineThickness;
    LUnits m_uMarging;      //distance from the bottom line of the previous staff

    friend class ImFactory;
    friend class ImoInstrument;
    //Default values for staff. Line spacing: 1.8 mm (staff height = 7.2 mm),
    //line thickness: 0.15 millimeters, top margin: 10 millimeters
    ImoStaffInfo(int numStaff=0, int lines=5, int type=k_staff_regular,
                 LUnits spacing=180.0f, LUnits thickness=15.0f, LUnits margin=1000.0f)
        : ImoSimpleObj(k_imo_staff_info)
        , m_numStaff(numStaff)
        , m_nNumLines(lines)
        , m_staffType(type)
        , m_uSpacing(spacing)
        , m_uLineThickness(thickness)
        , m_uMarging(margin)
    {
    }

public:
    virtual ~ImoStaffInfo() {}

    enum { k_staff_ossia=0, k_staff_cue, k_staff_editorial, k_staff_regular,
           k_staff_alternate,
         };

    //staff number
    inline int get_staff_number()
    {
        return m_numStaff;
    }
    inline void set_staff_number(int num)
    {
        m_numStaff = num;
    }

    //staff type
    inline int get_staff_type()
    {
        return m_staffType;
    }
    inline void set_staff_type(int type)
    {
        m_staffType = type;
    }

    //margins
    inline LUnits get_staff_margin()
    {
        return m_uMarging;
    }
    inline void set_staff_margin(LUnits uSpace)
    {
        m_uMarging = uSpace;
    }

    //spacing and size
    inline LUnits get_line_spacing()
    {
        return m_uSpacing;
    }
    inline void set_line_spacing(LUnits uSpacing)
    {
        m_uSpacing = uSpacing;
    }
    LUnits get_height()
    {
        if (m_nNumLines > 1)
            return (m_nNumLines - 1) * m_uSpacing + m_uLineThickness;
        else
            return m_uLineThickness;
    }

    //lines
    inline LUnits get_line_thickness()
    {
        return m_uLineThickness;
    }
    inline void set_line_thickness(LUnits uTickness)
    {
        m_uLineThickness = uTickness;
    }
    inline int get_num_lines()
    {
        return m_nNumLines;
    }
    inline void set_num_lines(int nLines)
    {
        m_nNumLines = nLines;
    }

protected:
    friend class MxlAnalyser;
    friend class LdpAnalyser;
    friend class MnxAnalyser;
    inline void add_space_for_lyrics(LUnits space)
    {
        m_uMarging += space;
    }

};

//---------------------------------------------------------------------------------------
class ImoStyles : public ImoSimpleObj
{
protected:
    std::map<std::string, ImoStyle*> m_nameToStyle;

    friend class ImFactory;
    ImoStyles(Document* pDoc);

public:
    virtual ~ImoStyles();

    //overrides, to traverse this special node
    void accept_visitor(BaseVisitor& v);
    bool has_visitable_children()
    {
        return m_nameToStyle.size() > 0;
    }

    //styles
    void add_style(ImoStyle* pStyle);
    //void add_required_text_styles();
    ImoStyle* find_style(const std::string& name);
    ImoStyle* get_default_style();
    ImoStyle* get_style_or_default(const std::string& name);

protected:
    void delete_text_styles();
    void create_default_styles();

    friend class StylesLmdGenerator;
    friend class StylesMnxGenerator;
    inline std::map<std::string, ImoStyle*>& get_styles_collection()
    {
        return m_nameToStyle;
    }

};

//---------------------------------------------------------------------------------------
class ImoTable : public ImoBlocksContainer
{
protected:
    std::list<ImoStyle*> m_colStyles;   //cannot be ImoCollection as ImoStyles are
    //deleted at document level.

    friend class TableColumnAnalyser;
    friend class TableColumnLmdAnalyser;
    inline void add_column_style(ImoStyle* pStyle)
    {
        m_colStyles.push_back(pStyle);
    }

    friend class ImFactory;
    ImoTable() : ImoBlocksContainer(k_imo_table) {}

public:
    virtual ~ImoTable() {}

    //contents
    ImoTableHead* get_head();
    ImoTableBody* get_body();
    inline std::list<ImoStyle*>& get_column_styles()
    {
        return m_colStyles;
    }
    inline int get_num_columns()
    {
        return int( m_colStyles.size() );
    }

protected:
    friend class BlockLevelCreatorApi;

};

//---------------------------------------------------------------------------------------
class ImoTableCell : public ImoBlocksContainer
{
protected:
    int m_rowspan;
    int m_colspan;

    friend class Document;
    friend class ImFactory;
    ImoTableCell(Document* pDoc);

public:
    virtual ~ImoTableCell() {}

    //accessors
    inline void set_rowspan(int value)
    {
        m_rowspan = value;
    }
    inline int get_rowspan()
    {
        return m_rowspan;
    }
    inline void set_colspan(int value)
    {
        m_colspan = value;
    }
    inline int get_colspan()
    {
        return m_colspan;
    }


//   //required by Visitable parent class
    //virtual void accept_visitor(BaseVisitor& v);
};

//---------------------------------------------------------------------------------------
class ImoTableRow : public ImoBlocksContainer
{
protected:

    friend class ImFactory;
    ImoTableRow(Document* pDoc);

public:
    virtual ~ImoTableRow() {}

    //contents
    inline int get_num_cells()
    {
        return get_num_content_items();
    }
    inline void add_cell(ImoTableCell* pCell)
    {
        append_content_item(pCell);
    }
    inline ImoTableCell* get_cell(int iItem)     //iItem = 0..n-1
    {
        return dynamic_cast<ImoTableCell*>( get_content_item(iItem) );
    }

    //overrides: as ImoTableRow is stored in an ImoCollection, some general methods
    //doesn't work and must be overrinden
    ImoStyle* get_style(bool fInherit=true) override;

protected:
    friend class BlockLevelCreatorApi;

};

//---------------------------------------------------------------------------------------
class ImoTableSection : public ImoCollection
{
protected:
    ImoTableSection(int type) : ImoCollection(type) {}

public:
    virtual ~ImoTableSection() {}

    void remove(ImoTableRow* pTR)
    {
        remove_child_imo(pTR);
    }
    void add(ImoTableRow* pTR)
    {
        append_child_imo(pTR);
    }
};

//---------------------------------------------------------------------------------------
class ImoTableBody : public ImoTableSection
{
protected:
    friend class ImFactory;
    friend class ImoContentObj;
    ImoTableBody() : ImoTableSection(k_imo_table_body) {}

public:
    virtual ~ImoTableBody() {}
};

//---------------------------------------------------------------------------------------
class ImoTableHead : public ImoTableSection
{
protected:
    friend class ImFactory;
    friend class ImoContentObj;
    ImoTableHead() : ImoTableSection(k_imo_table_head) {}

public:
    virtual ~ImoTableHead() {}
};

//---------------------------------------------------------------------------------------
class ImoTextItem : public ImoInlineLevelObj
{
private:
    string m_text;
    string m_language;

protected:
    friend class ImFactory;
    friend class TextItemAnalyser;
    friend class TextItemLmdAnalyser;

    ImoTextItem() : ImoInlineLevelObj(k_imo_text_item), m_text("") {}

public:
    virtual ~ImoTextItem() {}

    //getters
    inline string& get_text()
    {
        return m_text;
    }
    string& get_language();

    //setters
    inline void set_text(const string& text)
    {
        m_text = text;
    }
    inline void set_language(const string& value)
    {
        m_language = value;
    }

};

//---------------------------------------------------------------------------------------
class ImoTieData : public ImoRelDataObj
{
protected:
    bool    m_fStart;
    int     m_tieNum;
    int     m_orientation;
    ImoBezierInfo* m_pBezier;

    friend class ImFactory;
    ImoTieData(ImoTieDto* pDto);

public:
    virtual ~ImoTieData();

    //getters
    inline bool is_start()
    {
        return m_fStart;
    }
    inline int get_tie_number()
    {
        return m_tieNum;
    }
    inline int get_orientation()
    {
        return m_orientation;
    }
    inline ImoBezierInfo* get_bezier()
    {
        return m_pBezier;
    }

    //edition
    ImoBezierInfo* add_bezier();
};

//---------------------------------------------------------------------------------------
class ImoTie : public ImoRelObj
{
protected:
    int     m_tieNum;
    int     m_orientation;
    Color   m_color;

    friend class ImFactory;
    ImoTie()
        : ImoRelObj(k_imo_tie), m_tieNum(0), m_orientation(k_orientation_default)
    {}
    ImoTie(int num)
        : ImoRelObj(k_imo_tie), m_tieNum(num), m_orientation(k_orientation_default)
    {}

public:
    virtual ~ImoTie() {}

    //getters
    inline int get_tie_number()
    {
        return m_tieNum;
    }
    inline int get_orientation()
    {
        return m_orientation;
    }
    ImoNote* get_start_note();
    ImoNote* get_end_note();

    //setters
    inline void set_tie_number(int num)
    {
        m_tieNum = num;
    }
    inline void set_orientation(int value)
    {
        m_orientation = value;
    }
    inline void set_color(Color value)
    {
        m_color = value;
    }

    //access to data objects
    ImoBezierInfo* get_start_bezier();
    ImoBezierInfo* get_stop_bezier() ;
    ImoBezierInfo* get_start_bezier_or_create();
    ImoBezierInfo* get_stop_bezier_or_create();

    void reorganize_after_object_deletion();
};

// raw info about a pending tie
//---------------------------------------------------------------------------------------
class ImoTieDto : public ImoSimpleObj
{
protected:
    bool m_fStart;
    int m_tieNum;
    int m_orientation;
    ImoNote* m_pNote;
    ImoBezierInfo* m_pBezier;
    Color m_color;
    int m_lineNum;

public:
    ImoTieDto()
        : ImoSimpleObj(k_imo_tie_dto)
        , m_fStart(true)
        , m_tieNum(0)
        , m_orientation(k_orientation_default)
        , m_pNote(nullptr)
        , m_pBezier(nullptr)
        , m_lineNum(0)
    {
    }
    virtual ~ImoTieDto();

    //getters
    inline bool is_start()
    {
        return m_fStart;
    }
    inline int get_tie_number()
    {
        return m_tieNum;
    }
    inline int get_orientation()
    {
        return m_orientation;
    }
    inline ImoNote* get_note()
    {
        return m_pNote;
    }
    inline ImoBezierInfo* get_bezier()
    {
        return m_pBezier;
    }
    int get_line_number()
    {
        return m_lineNum;
    }
    inline Color get_color()
    {
        return m_color;
    }

    //setters
    inline void set_start(bool value)
    {
        m_fStart = value;
    }
    inline void set_tie_number(int num)
    {
        m_tieNum = num;
    }
    inline void set_orientation(int value)
    {
        m_orientation = value;
    }
    inline void set_note(ImoNote* pNote)
    {
        m_pNote = pNote;
    }
    inline void set_bezier(ImoBezierInfo* pBezier)
    {
        m_pBezier = pBezier;
    }
    inline void set_color(Color value)
    {
        m_color = value;
    }
    inline void set_line_number(int value)
    {
        m_lineNum = value;
    }

    //required by RelationBuilder
    int get_item_number()
    {
        return get_tie_number();
    }
    bool is_start_of_relation()
    {
        return is_start();
    }
    bool is_end_of_relation()
    {
        return !is_start();
    }
    ImoStaffObj* get_staffobj();
};

//---------------------------------------------------------------------------------------
class ImoTimeModificationDto : public ImoDto
{
protected:
    int m_top;
    int m_bottom;

public:
    ImoTimeModificationDto() : ImoDto(k_imo_time_modification_dto)
        , m_top(1), m_bottom(1) {}
    virtual ~ImoTimeModificationDto() {}

    //getters & setters
    inline int get_top_number()
    {
        return m_top;
    }
    inline void set_top_number(int num)
    {
        m_top = num;
    }
    inline int get_bottom_number()
    {
        return m_bottom;
    }
    inline void set_bottom_number(int num)
    {
        m_bottom = num;
    }
};

//---------------------------------------------------------------------------------------
class ImoTimeSignature : public ImoStaffObj
{
protected:
    int     m_top;
    int     m_bottom;
    int     m_type;

    friend class ImFactory;
    ImoTimeSignature()
        : ImoStaffObj(k_imo_time_signature)
        , m_top(2)
        , m_bottom(4)
        , m_type(ImoTimeSignature::k_normal)
    {
    }

public:
    virtual ~ImoTimeSignature() {}

    //getters and setters
    inline int get_top_number()
    {
        return m_top;
    }
    inline void set_top_number(int num)
    {
        m_top = num;
    }
    inline int get_bottom_number()
    {
        return m_bottom;
    }
    inline void set_bottom_number(int num)
    {
        m_bottom = num;
    }
    inline int get_type()
    {
        return m_type;
    }
    inline void set_type(int type)
    {
        m_type = type;
    }

    //overrides: time signatures always in staff 0
    void set_staff(int UNUSED(staff))
    {
        m_staff = 0;
    }

    //properties
    bool can_generate_secondary_shapes()
    {
        return true;
    }

    //other
    bool is_compound_meter();
    inline bool is_normal()
    {
        return m_type == k_normal;
    }
    inline bool is_senza_misura()
    {
        return m_type == k_senza_misura;
    }
    inline bool is_single_number()
    {
        return m_type == k_single_number;
    }
    inline bool is_common()
    {
        return m_type == k_common;
    }
    inline bool is_cut()
    {
        return m_type == k_cut;
    }
    int get_num_pulses();
    TimeUnits get_ref_note_duration();
    TimeUnits get_measure_duration();
    TimeUnits get_beat_duration();

    //time signature type
    enum { k_normal=0, k_common, k_cut, k_single_number, k_senza_misura, };

};

//---------------------------------------------------------------------------------------
// raw info about a tuplet
class ImoTupletDto : public ImoSimpleObj
{
protected:
    int m_tupletType;
    int m_tupletNum;
    int m_nActualNum;
    int m_nNormalNum;
    int m_nShowBracket;
    int m_nPlacement;
    int m_nShowNumber;
    int m_lineNum;
    bool m_fOnlyGraphical;
    ImoNoteRest* m_pNR;

public:
    ImoTupletDto();
    virtual ~ImoTupletDto() {}

    enum { k_unknown = 0, k_start, k_continue, k_stop, };

    //getters
    inline ImoNoteRest* get_note_rest()
    {
        return m_pNR;
    }
    inline bool is_start_of_tuplet()
    {
        return m_tupletType == ImoTupletDto::k_start;
    }
    inline bool is_end_of_tuplet()
    {
        return m_tupletType == ImoTupletDto::k_stop;;
    }
    inline int get_actual_number()
    {
        return m_nActualNum;
    }
    inline int get_normal_number()
    {
        return m_nNormalNum;
    }
    inline int get_show_bracket()
    {
        return m_nShowBracket;
    }
    inline int get_show_number()
    {
        return m_nShowNumber;
    }
    inline int get_placement()
    {
        return m_nPlacement;
    }
    int get_line_number()
    {
        return m_lineNum;
    };
    inline bool is_only_graphical()
    {
        return m_fOnlyGraphical;
    }

    //setters
    inline void set_note_rest(ImoNoteRest* pNR)
    {
        m_pNR = pNR;
    }
    inline void set_tuplet_type(int value)
    {
        m_tupletType = value;
    }
    inline void set_tuplet_number(int value)
    {
        m_tupletNum = value;
    }
    inline void set_actual_number(int value)
    {
        m_nActualNum = value;
    }
    inline void set_normal_number(int value)
    {
        m_nNormalNum = value;
    }
    inline void set_show_bracket(int value)
    {
        m_nShowBracket = value;
    }
    inline void set_show_number(int value)
    {
        m_nShowNumber = value;
    }
    inline void set_placement(int value)
    {
        m_nPlacement = value;
    }
    inline void set_only_graphical(bool value)
    {
        m_fOnlyGraphical = value;
    }
    inline void set_line_number(int value)
    {
        m_lineNum = value;
    }

    //required by RelationBuilder
    int get_item_number()
    {
        return m_tupletNum;
    }
    bool is_start_of_relation()
    {
        return is_start_of_tuplet();
    }
    bool is_end_of_relation()
    {
        return is_end_of_tuplet();
    }
    ImoStaffObj* get_staffobj();
};

//---------------------------------------------------------------------------------------
class ImoTuplet : public ImoRelObj
{
protected:
    int m_nActualNum;
    int m_nNormalNum;
    int m_nShowBracket;
    int m_nShowNumber;
    int m_nPlacement;

    friend class ImFactory;
    ImoTuplet()
        : ImoRelObj(k_imo_tuplet)
        , m_nActualNum(0)
        , m_nNormalNum(0)
        , m_nShowBracket(0)
        , m_nShowNumber(0)
        , m_nPlacement(0)
    {
    }
    ImoTuplet(ImoTupletDto* dto);

public:
    virtual ~ImoTuplet() {}

    enum { k_straight = 0, k_curved, k_slurred, };
    enum { k_number_actual=0, k_number_both, k_number_none, };

    //getters
    inline int get_actual_number()
    {
        return m_nActualNum;
    }
    inline int get_normal_number()
    {
        return m_nNormalNum;
    }
    inline int get_show_bracket()
    {
        return m_nShowBracket;
    }
    inline int get_show_number()
    {
        return m_nShowNumber;
    }
    inline int get_placement()
    {
        return m_nPlacement;
    }

    void reorganize_after_object_deletion();
};

//---------------------------------------------------------------------------------------
// ImoLyric represents the lyrics info for one note
class ImoLyric : public ImoAuxRelObj
{
protected:
    int m_number;
    int m_placement;
    int m_numTextItems;

    bool m_fLaughing;
    bool m_fHumming;
    bool m_fEndLine;
    bool m_fEndParagraph;
    bool m_fMelisma;
    bool m_fHyphenation;

    //children
    // ImoLyricsTextInfo[]

    friend class ImFactory;
    ImoLyric()
        : ImoAuxRelObj(k_imo_lyric)
        , m_number(0)
        , m_placement(k_placement_default)
        , m_numTextItems(0)
        , m_fLaughing(false)
        , m_fHumming(false)
        , m_fEndLine(false)
        , m_fEndParagraph(false)
        , m_fMelisma(false)
        , m_fHyphenation(false)
    {
    }

public:
    virtual ~ImoLyric();

    //getters
    inline int get_number()
    {
        return m_number;
    }
    inline int get_placement()
    {
        return m_placement;
    }
    inline bool is_laughing()
    {
        return m_fLaughing;
    }
    inline bool is_humming()
    {
        return m_fHumming;
    }
    inline bool is_end_line()
    {
        return m_fEndLine;
    }
    inline bool is_end_paragraph()
    {
        return m_fEndParagraph;
    }
    inline bool has_hyphenation()
    {
        return m_fHyphenation;
    }
    inline ImoLyric* get_prev_lyric()
    {
        return static_cast<ImoLyric*>(m_prevARO);
    }
    inline ImoLyric* get_next_lyric()
    {
        return static_cast<ImoLyric*>(m_nextARO);
    }

    //setters
    inline void set_number(int number)
    {
        m_number = number;
    }
    inline void set_placement(int placement)
    {
        m_placement = placement;
    }
    inline void set_laughing(bool value)
    {
        m_fLaughing = value;
    }
    inline void set_humming(bool value)
    {
        m_fHumming = value;
    }
    inline void set_end_line(bool value)
    {
        m_fEndLine = value;
    }
    inline void set_end_paragraph(bool value)
    {
        m_fEndParagraph = value;
    }
    inline void set_melisma(bool value)
    {
        m_fMelisma = value;
    }
    inline void set_hyphenation(bool value)
    {
        m_fHyphenation = value;
    }

    //information
    inline int get_num_text_items()
    {
        return m_numTextItems;
    }
    inline bool has_melisma()
    {
        return m_fMelisma;
    }

    //data
    ImoLyricsTextInfo* get_text_item(int i);

protected:

    friend class LyricMxlAnalyser;
    friend class LyricAnalyser;
    friend class LyricMnxAnalyser;
    friend class LdpAnalyser;
    friend class MxlAnalyser;
    friend class MnxAnalyser;
    void add_text_item(ImoLyricsTextInfo* pText);
    void link_to_next_lyric(ImoLyric* pNext)
    {
        link_to_next_ARO(pNext);
    }
    void set_prev_lyric(ImoLyric* pPrev)
    {
        set_prev_ARO(pPrev);
    }

};

//---------------------------------------------------------------------------------------
class ImoLyricsTextInfo : public ImoSimpleObj
{
protected:
    int m_syllableType;
    ImoTextInfo m_text;
    string m_elision;       //before this syllable
//    string m_elisionFont;
//    Color m_elisionColor;

    friend class ImFactory;
    ImoLyricsTextInfo()
        : ImoSimpleObj(k_imo_lyrics_text_info)
        , m_syllableType(k_single)
    {
    }


public:
    virtual ~ImoLyricsTextInfo() {}

    //syllable type
    enum { k_single, k_begin, k_end, k_middle, };

    //getters
    inline int get_syllable_type()
    {
        return m_syllableType;
    }
    inline string& get_syllable_text()
    {
        return m_text.get_text();
    }
    inline string& get_syllable_language()
    {
        return m_text.get_language();
    }
    ImoStyle* get_syllable_style();
    inline bool has_elision()
    {
        return !m_elision.empty();
    }
    inline string& get_elision_text()
    {
        return m_elision;
    }

    //setters
    inline void set_syllable_type(int value)
    {
        m_syllableType = value;
    }
    inline void set_syllable_text(const string& text)
    {
        m_text.set_text(text);
    }
    inline void set_syllable_style(ImoStyle* pStyle)
    {
        m_text.set_style(pStyle);
    }
    inline void set_syllable_language(const string& language)
    {
        m_text.set_language(language);
    }
    inline void set_elision_text(const string& text)
    {
        m_elision = text;
    }

};


//---------------------------------------------------------------------------------------
// Represents an octave-shift line
class ImoOctaveShift : public ImoRelObj
{
protected:
    int     m_steps;
    int     m_octaveShiftNum;

    friend class ImFactory;
    ImoOctaveShift(int num=0)
        : ImoRelObj(k_imo_octave_shift)
        , m_steps(0)
        , m_octaveShiftNum(num)
    {
    }

public:
    virtual ~ImoOctaveShift() {}

    //setters
    inline void set_shift_steps(int value) { m_steps = value; }
    inline void set_octave_shift_number(int value) { m_octaveShiftNum = value; }

    //getters
    inline int get_shift_steps() { return m_steps; }
    inline int get_octave_shift_number() { return m_octaveShiftNum; }

    void reorganize_after_object_deletion();
};

//---------------------------------------------------------------------------------------
// raw info about a pending octave-shift line
class ImoOctaveShiftDto : public ImoSimpleObj
{
protected:
    bool m_fStart;
    int m_steps;
    int m_octaveShiftNum;
    ImoNote* m_pNote;
    Color m_color;
    int m_lineNum;
    int m_iStaff;

public:
    ImoOctaveShiftDto()
        : ImoSimpleObj(k_imo_octave_shift_dto)
        , m_fStart(true)
        , m_steps(0)
        , m_octaveShiftNum(0)
        , m_pNote(nullptr)
        , m_color(Color(0,0,0))
        , m_lineNum(0)
        , m_iStaff(0)
    {
    }
    virtual ~ImoOctaveShiftDto() {}

    //setters
    inline void set_line_number(int value) { m_lineNum = value; }
    inline void set_staffobj(ImoNote* pNote) { m_pNote = pNote; }
    inline void set_shift_steps(int value) { m_steps = value; }
    inline void set_start(bool value) { m_fStart = value; }
    inline void set_octave_shift_number(int value) { m_octaveShiftNum = value; }
    inline void set_color(Color value) { m_color = value; }
    inline void set_staff(int value) { m_iStaff = value; }

    //getters
    inline int get_line_number() { return m_lineNum; }
    inline int get_shift_steps() { return m_steps; }
    inline bool is_start() { return m_fStart; }
    inline int get_octave_shift_number() { return m_octaveShiftNum; }
    inline Color get_color() { return m_color; }
    inline int get_staff() { return m_iStaff; }

    //required by RelationBuilder
    inline int get_item_number() { return get_octave_shift_number(); }
    bool is_start_of_relation() { return is_start(); }
    bool is_end_of_relation() { return !is_start(); }
    inline ImoNote* get_staffobj() { return m_pNote; }
};


//---------------------------------------------------------------------------------------
class ImoVoltaBracket : public ImoRelObj
{
protected:
    bool    m_fStopJog;
    //False when there is no downward jog, as is typical for
    //second endings that do not conclude a piece.
    string  m_voltaNum;
    //The numeric values of the repetitions for the measure associated to this volta.
    //Single values such as "1" or comma-separated multiple endings such as "1, 2"
    //may be used.
    string  m_voltaText;
    //If not empty, this text is used to be displayed in the volta bracket instead
    //the volta numbers, i.e.: "First time" instead of "1".
    vector<int> m_repetitions;      //repetition numbers extracted from m_voltaNum

    //data valid only in first volta of each set of voltas for a repetition
    int m_numVoltas;                //number of voltas in the set

    friend class ImFactory;
    ImoVoltaBracket()
        : ImoRelObj(k_imo_volta_bracket)
        , m_fStopJog(true)
        , m_voltaNum()
        , m_voltaText()
        , m_numVoltas(1)
    {
    }

public:
    virtual ~ImoVoltaBracket() {}

    //getters
    inline bool has_final_jog()
    {
        return m_fStopJog;
    }
    inline string& get_volta_number()
    {
        return m_voltaNum;
    }
    inline string& get_volta_text()
    {
        return m_voltaText;
    }
    ImoBarline* get_start_barline();
    ImoBarline* get_stop_barline();

    //setters
    inline void set_final_jog(bool value)
    {
        m_fStopJog = value;
    }
    inline void set_volta_number(const string& num)
    {
        m_voltaNum = num;
    }
    inline void set_volta_text(const string& text)
    {
        m_voltaText = text;
    }
    inline void increment_total_voltas()
    {
        ++m_numVoltas;
    }

    //info
    inline bool is_first_repeat()
    {
        return m_repetitions[0] == 1;
    }
    inline int get_number_of_repetitions()
    {
        return int(m_repetitions.size());
    }
    inline void set_repetitions(vector<int>& repetitions)
    {
        m_repetitions = repetitions;
    }
    inline int get_total_voltas()
    {
        return m_numVoltas;
    }


    //required override for ImoRelObj
    void reorganize_after_object_deletion();

};


//---------------------------------------------------------------------------------------
// raw info about a volta bracket
class ImoVoltaBracketDto : public ImoSimpleObj
{
protected:
    int         m_lineNum;
    int         m_voltaId;
    bool        m_fStopJog;
    int         m_type;
    string      m_voltaNum;
    string      m_voltaText;
    ImoBarline* m_pBarline;
    vector<int> m_repetitions;      //repetition numbers extracted from m_voltaNum
    //only for type k_start

public:
    ImoVoltaBracketDto()
        : ImoSimpleObj(k_imo_volta_bracket_dto)
        , m_lineNum(0)
        , m_voltaId(0)
        , m_fStopJog(true)
        , m_type(k_unknown)
        , m_voltaNum()
        , m_voltaText()
        , m_pBarline(nullptr)
    {
    }
    virtual ~ImoVoltaBracketDto() {}

    enum { k_unknown = 0, k_start, k_stop, k_discontinue,  };

    //getters
    inline int get_volta_type()
    {
        return m_type;
    }
    inline string& get_volta_number()
    {
        return m_voltaNum;
    }
    inline string& get_volta_text()
    {
        return m_voltaText;
    }
    inline ImoBarline* get_barline()
    {
        return m_pBarline;
    }
    inline bool get_final_jog()
    {
        return m_fStopJog;
    }
    inline int get_volta_id()
    {
        return m_voltaId;
    }
    int get_line_number()
    {
        return m_lineNum;
    };
    inline vector<int>& get_repetitions()
    {
        return m_repetitions;
    }

    //setters
    inline void set_volta_type(int value)
    {
        m_type = value;
    }
    inline void set_volta_number(const string& num)
    {
        m_voltaNum = num;
    }
    inline void set_volta_text(const string& text)
    {
        m_voltaText = text;
    }
    inline void set_barline(ImoBarline* pSO)
    {
        m_pBarline = pSO;
    }
    inline void set_final_jog(bool value)
    {
        m_fStopJog = value;
    }
    inline void set_volta_id(int value)
    {
        m_voltaId = value;
    }
    inline void set_line_number(int value)
    {
        m_lineNum = value;
    }
    inline void set_repetitions(vector<int>& repetitions)
    {
        m_repetitions = repetitions;
    }

    //required by RelationBuilder
    int get_item_number()
    {
        return get_volta_id();
    }
    bool is_start_of_relation()
    {
        return m_type == k_start;
    }
    bool is_end_of_relation()
    {
        return m_type == k_stop;
    }
    ImoStaffObj* get_staffobj()
    {
        return m_pBarline;
    }
};

//---------------------------------------------------------------------------------------
class ImoWedge : public ImoRelObj
{
protected:
    Tenths  m_startSpread;
    Tenths  m_endSpread;
    bool    m_fNiente;
    bool    m_fCrescendo;
    int     m_wedgeNum;
//    Color   m_color;

    friend class ImFactory;
    ImoWedge(int num=0)
        : ImoRelObj(k_imo_wedge)
        , m_startSpread(0.0f)
        , m_endSpread(0.0f)
        , m_fNiente(false)
        , m_fCrescendo(false)
        , m_wedgeNum(num)
    {
    }

public:
    virtual ~ImoWedge() {}

    //setters
    inline void set_start_spread(Tenths value) { m_startSpread = value; }
    inline void set_end_spread(Tenths value) { m_endSpread = value; }
    inline void set_niente(bool value) { m_fNiente = value; }
    inline void set_crescendo(bool value) { m_fCrescendo = value; }
    inline void set_wedge_number(int value) { m_wedgeNum = value; }
//    inline void set_color(Color value) { m_color = value; }

    //getters
    inline Tenths get_start_spread() { return m_startSpread; }
    inline Tenths get_end_spread() { return m_endSpread; }
    inline bool is_niente() { return m_fNiente; }
    inline bool is_crescendo() { return m_fCrescendo; }
    inline int get_wedge_number() { return m_wedgeNum; }
//    inline Color get_color() { return m_color; }

    void reorganize_after_object_deletion();
};

// raw info about a pending wedge
//---------------------------------------------------------------------------------------
class ImoWedgeDto : public ImoSimpleObj
{
protected:
    Tenths m_spread;
    bool m_fNiente;
    bool m_fStart;
    bool m_fCrescendo;
    int m_wedgeNum;
    ImoDirection* m_pDirection;
    Color m_color;
    int m_lineNum;

public:
    ImoWedgeDto()
        : ImoSimpleObj(k_imo_wedge_dto)
        , m_spread(0.0f)
        , m_fNiente(false)
        , m_fStart(true)
        , m_fCrescendo(false)
        , m_wedgeNum(0)
        , m_pDirection(nullptr)
        , m_color(Color(0,0,0))
        , m_lineNum(0)
    {
    }
    virtual ~ImoWedgeDto();

    //setters
    inline void set_line_number(int value) { m_lineNum = value; }
    inline void set_spread(Tenths value) { m_spread = value; }
    inline void set_niente(bool value) { m_fNiente = value; }
    inline void set_staffobj(ImoDirection* pDirection) { m_pDirection = pDirection; }
    inline void set_crescendo(bool value) { m_fCrescendo = value; }
    inline void set_start(bool value) { m_fStart = value; }
    inline void set_wedge_number(int value) { m_wedgeNum = value; }
    inline void set_color(Color value) { m_color = value; }

    //getters
    inline int get_line_number() { return m_lineNum; }
    inline Tenths get_spread() { return m_spread; }
    inline bool is_niente() { return m_fNiente; }
    inline bool is_crescendo() { return m_fCrescendo; }
    inline bool is_start() { return m_fStart; }
    inline int get_wedge_number() { return m_wedgeNum; }
    inline Color get_color() { return m_color; }

    //required by RelationBuilder
    inline int get_item_number() { return get_wedge_number(); }
    bool is_start_of_relation() { return is_start(); }
    bool is_end_of_relation() { return !is_start(); }
    inline ImoDirection* get_staffobj() { return m_pDirection; }
};


//---------------------------------------------------------------------------------------
// A tree of ImoObj objects
typedef Tree<ImoObj>          ImoTree;
typedef TreeNode<ImoObj>      ImoNode;

//---------------------------------------------------------------------------------------
// Visitors for ImoObj objects
typedef Tree<ImoObj>          ImoTree;
//typedef Visitor<ImoAttachments> ImVisitor;
//typedef Visitor<ImoAuxObj> ImVisitor;
//typedef Visitor<ImoBeamData> ImVisitor;
//typedef Visitor<ImoBeamDto> ImVisitor;
//typedef Visitor<ImoBeam> ImVisitor;
//typedef Visitor<ImoBlocksContainer> ImVisitor;
//typedef Visitor<ImoInlinesContainer> ImVisitor;
//typedef Visitor<ImoBoxInline> ImVisitor;
//typedef Visitor<ImoButton> ImVisitor;
//typedef Visitor<ImoChord> ImVisitor;
//typedef Visitor<ImoContentObj> ImVisitor;
//typedef Visitor<ImoDocument> ImVisitor;
//typedef Visitor<ImoDynamic> ImVisitor;
typedef Visitor<ImoHeading> ImHeadingVisitor;
//typedef Visitor<ImoInlineLevelObj> ImVisitor;
//typedef Visitor<ImoInstrument> ImVisitor;
//typedef Visitor<ImoLineStyle> ImVisitor;
//typedef Visitor<ImoMusicData> ImVisitor;
//typedef Visitor<ImoNote> ImVisitor;
//typedef Visitor<ImoNoteRest> ImVisitor;
typedef Visitor<ImoObj> ImObjVisitor;
//typedef Visitor<ImoOptionInfo> ImVisitor;
typedef Visitor<ImoParagraph> ImParagraphVisitor;
//typedef Visitor<ImoParamInfo> ImVisitor;
//typedef Visitor<ImoRelDataObj> ImVisitor;
//typedef Visitor<ImoRelObj> ImVisitor;
//typedef Visitor<ImoScoreText> ImVisitor;
//typedef Visitor<ImoSimpleObj> ImVisitor;
//typedef Visitor<ImoSlurDto> ImVisitor;
//typedef Visitor<ImoStaffInfo> ImVisitor;
//typedef Visitor<ImoStaffObj> ImVisitor;
//typedef Visitor<ImoStyles> ImVisitor;
//typedef Visitor<ImoTextInfo> ImVisitor;
//typedef Visitor<ImoTextItem> ImVisitor;
//typedef Visitor<ImoTextStyle> ImVisitor;
//typedef Visitor<ImoStyle> ImVisitor;
//typedef Visitor<ImoTieData> ImVisitor;
//typedef Visitor<ImoTieDto> ImVisitor;
//typedef Visitor<ImoTupletDto> ImVisitor;
//typedef Visitor<ImoTuplet> ImVisitor;
//typedef Visitor<ImoWrapperBox> ImVisitor;


//---------------------------------------------------------------------------------------
// global functions

extern int to_note_type(const char& letter);
extern NoteTypeAndDots ldp_duration_to_components(const string& duration);
extern TimeUnits to_duration(int nNoteType, int nDots);


}   //namespace lomse
///@endcond

#endif    // __LOMSE_INTERNAL_MODEL_H__


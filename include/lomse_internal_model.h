//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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

#include "lomse_internal_model_defs.h"

//std
#include <string>
#include <list>

#if (LOMSE_PLATFORM_WIN32 == 1)
    //Issue #253 points to a bug in Microsoft compiler with virtual inheritance.
    //To bypass the problem, for Windows the API definition is slightly different and
    //does not uses virtual inheritance.
    #define LOMSE_BYPASS_ISSUE_253     1
#else
    #define LOMSE_BYPASS_ISSUE_253     0
#endif


///@cond INTERNALS
namespace lomse
{

//forward declaration of the document API classes and the implementation classes
class ImoBlockLevelObj;
class ImoContentObj;
class ImoDocument;
class ImoDynamic;
class ImoInstrument;
class ImoInstrGroup;
class ImoLink;
class ImoList;
class ImoMidiInfo;
class ImoParagraph;
class ImoScore;
class ImoSoundInfo;
class ImoTextItem;

class ADocument;
class ADynamic;
class AObject;
class AInstrument;
class AInstrGroup;
class ALink;
class AList;
class AMidiInfo;
class AParagraph;
class AScore;
class ASoundInfo;
class ATextItem;

///@endcond


//=======================================================================================
// Enumerations defined in this module
//=======================================================================================

//---------------------------------------------------------------------------------------
/** @ingroup enumerations
    In a music score, when several instruments form a group, this enum indicates how
    the barlines for the instruments should be displayed.
*/
enum EJoinBarlines
{
    k_non_joined_barlines = 0,  ///< Independent barlines for each instrument in the group.
    k_joined_barlines,	        ///< Barlines joined across all instruments in the group.
    k_mensurstrich_barlines,    ///< Barlines only in the gaps between instruments in
                                ///< the group, but not on the staves of each instrument.
};

//---------------------------------------------------------------------------------------
/** @ingroup enumerations
    In a music score, when several instruments form a group, this enum indicates if a
    symbol for the group should be displayed in the score and what symbol to use.
*/
enum EGroupSymbol
{
    k_group_symbol_none = 0,    ///< Do not display a symbol.
    k_group_symbol_brace,       ///< Use a brace.
    k_group_symbol_bracket,     ///< Use a bracket.
    k_group_symbol_line,        ///< Use a vertical a line.
};

//-----------------------------------------------------------------------------
/** @ingroup enumerations
    This enum describes the API objects that compose a document
*/
enum EDocObject
{
    k_obj_anonymous_block,  ///< An structural block-level container (AAnonymousBlock)
    k_obj_button,           ///< A button control (AButton)
    k_obj_content,          ///< A generic block-level container (AContent)
    k_obj_control,          ///< A user defined GUI control object (AControl)
    k_obj_dynamic,          ///< A block of external content injected dynamically (ADynamic)
    k_obj_heading,          ///< A text header (AHeading)
    k_obj_image,            ///< An image (AImage)
    k_obj_inline_wrapper,   ///< A generic inline-box container (AInlineWrapper)
    k_obj_instrument,       ///< An instrument in an score (AInstrument)
    k_obj_instr_group,      ///< A grouping of instruments in an score (AInstrGroup)
    k_obj_link,             ///< A hyperlink (ALink)
    k_obj_list,             ///< A text list (AList)
    k_obj_list_item,        ///< An item in a text list (AListItem)
    k_obj_midi_info,        ///< A set of MIDI properties (AMidiInfo)
    k_obj_multicolumn,      ///< A blocks container subdivided in columns (AMultiColumn)
    k_obj_music_data,       ///< The musical content for an instrument (AMusicData)
    k_obj_paragraph,        ///< A paragraph (AParagraph)
    k_obj_score,	        ///< A music score (AScore)
    k_obj_score_player,     ///< A score player control (AScorePlayer)
    k_obj_sound_info,       ///< Sound information for an instrument (ASoundInfo)
    k_obj_table,            ///< A table (ATable)
    k_obj_table_cell,       ///< A cell in a table (ATableCell)
    k_obj_table_row,        ///< A row in a table (ATableRow)
    k_obj_text_item,        ///< A chunk of text (ATextItem)
};


//=======================================================================================
// Classes defined in this module
//=======================================================================================
///@cond INTERNALS


#if (LOMSE_BYPASS_ISSUE_253 == 1)

//---------------------------------------------------------------------------------------
// AObject: Abstract base class for all objects composing the document
class LOMSE_EXPORT AObject
{
protected:
    AObject downcast_to_content_obj();
    LOMSE_DECLARE_IM_API_ROOT_CLASS

public:
    //properties
    ImoId object_id() const;
    const std::string& object_name() const;
    ADocument owner_document() const;

    //downcast objects
    ADynamic downcast_to_dynamic() const;
    AInstrument downcast_to_instrument() const;
    AInstrGroup downcast_to_instr_group() const;
    ALink downcast_to_link() const;
    AList downcast_to_list() const;
    AParagraph downcast_to_paragraph() const;
    AScore downcast_to_score() const;
    ATextItem downcast_to_text_item() const;

    //check object type
    bool is_anonymous_block() const;
    bool is_button() const;
    bool is_content() const;
    bool is_control() const;
    bool is_dynamic() const;
    bool is_heading() const;
    bool is_image() const;
    bool is_inline_wrapper() const;
    bool is_instrument() const;
    bool is_instr_group() const;
    bool is_link() const;
    bool is_list() const;
    bool is_list_item() const;
    bool is_midi_info() const;
    bool is_multicolumn() const;
    bool is_music_data() const;
    bool is_paragraph() const;
    bool is_score() const;
    bool is_sound_info() const;
    bool is_table() const;
    bool is_table_cell() const;
    bool is_table_body() const;
    bool is_table_head() const;
    bool is_table_row() const;
    bool is_text_item() const;

    // Transitional, to facilitate migration to this new public API.
    ImoObj* internal_object() const;
};


//---------------------------------------------------------------------------------------
// AObjectWithSiblings: base class for all objects having siblings
class LOMSE_EXPORT AObjectWithSiblings : public AObject
{
    LOMSE_DECLARE_IM_API_CLASS(AObjectWithSiblings, ImoInstrument)

    //Document content traversal
    AObject previous_sibling() const;
    AObject next_sibling() const;
};


//---------------------------------------------------------------------------------------
// AObjectWithSiblingsAndChildren: base class for all objects having siblings and children
class LOMSE_EXPORT AObjectWithSiblingsAndChildren : public AObjectWithSiblings
{
    LOMSE_DECLARE_IM_API_CLASS(AObjectWithSiblingsAndChildren, ImoInstrument)

    //Document content traversal
    int num_children() const;
    AObject child_at(int iItem) const;
    AObject first_child() const;
    AObject last_child() const;

};


//---------------------------------------------------------------------------------------
// ADynamic represents external content that is injected dynamically into the document
class LOMSE_EXPORT ADynamic : public AObjectWithSiblingsAndChildren
{
    LOMSE_DECLARE_IM_API_CLASS(ADynamic, ImoDynamic)
    friend class RequestDynamic;

    std::string& classid();

    // Transitional, to facilitate migration to this new public API.
    ImoDynamic* internal_object() const;
};


//---------------------------------------------------------------------------------------
// AInstrument represents an instrument in the score.
class LOMSE_EXPORT AInstrument : public AObject
{
    LOMSE_DECLARE_IM_API_CLASS(AInstrument, ImoInstrument)
    friend class AScore;
    friend class AInstrGroup;

    // name and abbreviation
    std::string& name_string() const;
    std::string& abbreviation_string() const;
    void set_name_string(const std::string& name);
    void set_abbreviation_string(const std::string& abbrev);

    // sound information
    int num_sounds() const;
    ASoundInfo sound_info_at(int iSound) const;

    // Transitional, to facilitate migration to this new public API.
    ImoInstrument* internal_object() const;
};


//---------------------------------------------------------------------------------------
// AInstrGroup provides access to the information for an instruments group.
class LOMSE_EXPORT AInstrGroup : public AObject
{
    LOMSE_DECLARE_IM_API_CLASS(AInstrGroup, ImoInstrGroup)
    friend class AScore;

    //access to group properties
    EJoinBarlines barlines_mode() const;
    EGroupSymbol symbol() const;
    const std::string& name_string() const;
    const std::string& abbreviation_string() const;

    //group properties modification
    void set_name_string(const std::string& text);
    void set_abbreviation_string(const std::string& text);
    void set_symbol(EGroupSymbol symbol);
    void set_barlines_mode(EJoinBarlines value);

    //instruments in the group
    int num_instruments() const;
    AInstrument instrument_at(int iInstr) const;  //iInstr = 0..num-instrs-in-group - 1
    AInstrument first_instrument() const;
    AInstrument last_instrument() const;
    int index_to_first_instrument() const;
    int index_to_last_instrument() const;
    bool set_range(int iFirstInstr, int iLastInstr);

    // Transitional, to facilitate migration to this new public API.
    ImoInstrGroup* internal_object() const;
};


//---------------------------------------------------------------------------------------
// ALink is a container for inline objects, and reprensents a clickable 'link'
class LOMSE_EXPORT ALink : public AObjectWithSiblingsAndChildren
{
    LOMSE_DECLARE_IM_API_CLASS(ALink, ImoLink)

    // Transitional, to facilitate migration to this new public API.
    ImoLink* internal_object() const;
};


//---------------------------------------------------------------------------------------
// AList represents a list of items and it is a container for AListItem objects.
class LOMSE_EXPORT AList : public AObjectWithSiblingsAndChildren
{
    LOMSE_DECLARE_IM_API_CLASS(AList, ImoList)

    // Transitional, to facilitate migration to this new public API.
    ImoList* internal_object() const;
};


//---------------------------------------------------------------------------------------
// AMidiInfo provides access to the MIDI information associated to a ASoundInfo object
class LOMSE_EXPORT AMidiInfo : public AObject
{
    LOMSE_DECLARE_IM_API_CLASS(AMidiInfo, ImoMidiInfo)
    friend class ASoundInfo;

    //getters
    int port() const;
    std::string& device_name() const;
    std::string& program_name() const;
    int bank() const;
    int channel() const;
    int program() const;
    int unpitched() const;
    float volume() const;
    int pan() const;
    int elevation() const;

    //setters
    void set_port(int value);
    void set_device_name(const std::string& value);
    void set_program_name(const std::string& value);
    void set_bank(int value);
    void set_channel(int value);
    void set_program(int value);
    void set_unpitched(int value);
    void set_volume(float value);
    void set_pan(int value);
    void set_elevation(int value);

    // Transitional, to facilitate migration to this new public API.
    ImoMidiInfo* internal_object() const;
};



//---------------------------------------------------------------------------------------
// AParagraph represents a paragraph. It is a block-level container, similar to the
class LOMSE_EXPORT AParagraph : public AObjectWithSiblingsAndChildren
{
    LOMSE_DECLARE_IM_API_CLASS(AParagraph, ImoParagraph)

    // Transitional, to facilitate migration to this new public API.
    ImoScore* internal_object() const;

};



//---------------------------------------------------------------------------------------
// AScore is the API object for interacting with the internal model for a music score.
class LOMSE_EXPORT AScore : public AObjectWithSiblings
{
    LOMSE_DECLARE_IM_API_CLASS(AScore, ImoScore)

    //instruments: create/delete/move instruments and get information
    AInstrument append_new_instrument();
    void delete_instrument(ImoId instrId);
    void delete_instrument(AInstrument& instr);
    void move_up_instrument(ImoId instrId);
    void move_up_instrument(AInstrument& instr);
    void move_down_instrument(ImoId instrId);
    void move_down_instrument(AInstrument& instr);
    AInstrument instrument_at(int iInstr) const;   //0..n-1
    int num_instruments() const;

    //instrument groups
    int num_instruments_groups() const;
    AInstrGroup instruments_group_at(int iGroup) const;   //0..n-1
    AInstrGroup group_instruments(int iFirstInstr, int iLastInstr);
    bool delete_instruments_group_at(int iGroup);   //0..n-1
    bool delete_instruments_group(const AInstrGroup& group);
    void delete_all_instruments_groups();

    //Algorithms
    MeasureLocator locator_for(TimeUnits timepos, int iInstr=0);
    TimeUnits timepos_for(int iMeasure, int iBeat, int iInstr=0);
    TimeUnits timepos_for(const MeasureLocator& ml);

    //inform that you have finished modifying this score
    void end_of_changes();

    // Transitional, to facilitate migration to this new public API.
    ImoScore* internal_object() const;

};


//---------------------------------------------------------------------------------------
// ASoundInfo class contains and manages the information for one sound, such as its
// MIDI values. It always contains a AMidiInfo object.
class LOMSE_EXPORT ASoundInfo : public AObject
{
    LOMSE_DECLARE_IM_API_CLASS(ASoundInfo, ImoSoundInfo)
    friend class AInstrument;

    //access to MIDI info
    AMidiInfo midi_info() const;

    // Transitional, to facilitate migration to this new public API.
    ImoSoundInfo* internal_object() const;
};


//---------------------------------------------------------------------------------------
// ATextItem is an inline-level object containing a chunk of text with the same style.
class LOMSE_EXPORT ATextItem : public AObjectWithSiblings
{
    LOMSE_DECLARE_IM_API_CLASS(ATextItem, ImoTextItem)

    // Transitional, to facilitate migration to this new public API.
    ImoTextItem* internal_object() const;
};

#else
//---------------------------------------------------------------------------------------
// AObject: Abstract base class for all objects composing the document
// See: https://lenmus.github.io/lomse/classAObject.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT AObject
{
protected:
    AObject downcast_to_content_obj();
    LOMSE_DECLARE_IM_API_ROOT_CLASS

public:
    //properties
    ImoId object_id() const;
    const std::string& object_name() const;
    ADocument owner_document() const;

    //downcast objects
    ADynamic downcast_to_dynamic() const;
    AInstrument downcast_to_instrument() const;
    AInstrGroup downcast_to_instr_group() const;
    ALink downcast_to_link() const;
    AList downcast_to_list() const;
    AParagraph downcast_to_paragraph() const;
    AScore downcast_to_score() const;
    ATextItem downcast_to_text_item() const;

    //check object type
    bool is_anonymous_block() const;
    bool is_button() const;
    bool is_content() const;
    bool is_control() const;
    bool is_dynamic() const;
    bool is_heading() const;
    bool is_image() const;
    bool is_inline_wrapper() const;
    bool is_instrument() const;
    bool is_instr_group() const;
    bool is_link() const;
    bool is_list() const;
    bool is_list_item() const;
    bool is_midi_info() const;
    bool is_multicolumn() const;
    bool is_music_data() const;
    bool is_paragraph() const;
    bool is_score() const;
    bool is_sound_info() const;
    bool is_table() const;
    bool is_table_cell() const;
    bool is_table_body() const;
    bool is_table_head() const;
    bool is_table_row() const;
    bool is_text_item() const;

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoObj* internal_object() const;
};


//---------------------------------------------------------------------------------------
// ISiblings class provides sibling traversal method for objects supporting them
// See: https://lenmus.github.io/lomse/classISiblings.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT ISiblings : public virtual AObject
{
    LOMSE_DECLARE_IM_API_CLASS(ISiblings, ImoObj)

    //Document content traversal
    AObject previous_sibling() const;
    AObject next_sibling() const;
};


//---------------------------------------------------------------------------------------
// IChildren class provides child traversal method for objects supporting them
// See: https://lenmus.github.io/lomse/classIChildren.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IChildren : public virtual AObject
{
    LOMSE_DECLARE_IM_API_CLASS(IChildren, ImoObj)

    //Document content traversal
    int num_children() const;
    AObject child_at(int iItem) const;
    AObject first_child() const;
    AObject last_child() const;

};


//---------------------------------------------------------------------------------------
// ADynamic represents external content that is injected dynamically into the document
// by the user application. It is equivalent to the HTML \<object\> element.
// See: https://lenmus.github.io/lomse/classADynamic.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT ADynamic : public virtual AObject, public ISiblings, public IChildren
{
    LOMSE_DECLARE_IM_API_CLASS(ADynamic, ImoDynamic)
    friend class RequestDynamic;

    std::string& classid();

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoDynamic* internal_object() const;
};


//---------------------------------------------------------------------------------------
// AInstrument represents an instrument in the score.
// See: https://lenmus.github.io/lomse/classAInstrument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT AInstrument : public virtual AObject
{
    LOMSE_DECLARE_IM_API_CLASS(AInstrument, ImoInstrument)
    friend class AScore;
    friend class AInstrGroup;

    // name and abbreviation
    std::string& name_string() const;
    std::string& abbreviation_string() const;
    void set_name_string(const std::string& name);
    void set_abbreviation_string(const std::string& abbrev);

    // sound information
    int num_sounds() const;
    ASoundInfo sound_info_at(int iSound) const;

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoInstrument* internal_object() const;
};


//---------------------------------------------------------------------------------------
// AInstrGroup provides access to the information for an instruments group.
// See: https://lenmus.github.io/lomse/classAInstrGroup.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT AInstrGroup : public virtual AObject
{
    LOMSE_DECLARE_IM_API_CLASS(AInstrGroup, ImoInstrGroup)
    friend class AScore;

    //access to group properties
    EJoinBarlines barlines_mode() const;
    EGroupSymbol symbol() const;
    const std::string& name_string() const;
    const std::string& abbreviation_string() const;

    //group properties modification
    void set_name_string(const std::string& text);
    void set_abbreviation_string(const std::string& text);
    void set_symbol(EGroupSymbol symbol);
    void set_barlines_mode(EJoinBarlines value);

    //instruments in the group
    int num_instruments() const;
    AInstrument instrument_at(int iInstr) const;  //iInstr = 0..num-instrs-in-group - 1
    AInstrument first_instrument() const;
    AInstrument last_instrument() const;
    int index_to_first_instrument() const;
    int index_to_last_instrument() const;
    bool set_range(int iFirstInstr, int iLastInstr);

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoInstrGroup* internal_object() const;
};


//---------------------------------------------------------------------------------------
// ALink is a container for inline objects, and reprensents a clickable 'link'
// object that creates hyperlinks. It is similar to the HTML \<a\> element.
// See: https://lenmus.github.io/lomse/classALink.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT ALink : public virtual AObject, public ISiblings, public IChildren
{
    LOMSE_DECLARE_IM_API_CLASS(ALink, ImoLink)

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoLink* internal_object() const;
};


//---------------------------------------------------------------------------------------
// AList represents a list of items and it is a container for AListItem objects.
// It is equivalent to the HTML \<ol\> and \<ul\> elements.
// See: https://lenmus.github.io/lomse/classAList.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT AList : public virtual AObject, public ISiblings, public IChildren
{
    LOMSE_DECLARE_IM_API_CLASS(AList, ImoList)

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoList* internal_object() const;
};


//---------------------------------------------------------------------------------------
// AMidiInfo provides access to the MIDI information associated to a ASoundInfo object
// for an instrument. MIDI info always exists in the ASoundInfo object.
// See: https://lenmus.github.io/lomse/classAMidiInfo.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT AMidiInfo : public virtual AObject
{
    LOMSE_DECLARE_IM_API_CLASS(AMidiInfo, ImoMidiInfo)
    friend class ASoundInfo;

    //getters
    int port() const;
    std::string& device_name() const;
    std::string& program_name() const;
    int bank() const;
    int channel() const;
    int program() const;
    int unpitched() const;
    float volume() const;
    int pan() const;
    int elevation() const;

    //setters
    void set_port(int value);
    void set_device_name(const std::string& value);
    void set_program_name(const std::string& value);
    void set_bank(int value);
    void set_channel(int value);
    void set_program(int value);
    void set_unpitched(int value);
    void set_volume(float value);
    void set_pan(int value);
    void set_elevation(int value);

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoMidiInfo* internal_object() const;
};



//---------------------------------------------------------------------------------------
// AParagraph represents a paragraph. It is a block-level container, similar to the
// HTML <p> element.
// See: https://lenmus.github.io/lomse/classAParagraph.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT AParagraph : public virtual AObject, public ISiblings, public IChildren
{
    LOMSE_DECLARE_IM_API_CLASS(AParagraph, ImoParagraph)

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoScore* internal_object() const;

};



//---------------------------------------------------------------------------------------
// AScore is the API object for interacting with the internal model for a music score.
// See: https://lenmus.github.io/lomse/classAScore.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT AScore : public virtual AObject, public ISiblings
{
    LOMSE_DECLARE_IM_API_CLASS(AScore, ImoScore)

    //instruments: create/delete/move instruments and get information
    AInstrument append_new_instrument();
    void delete_instrument(ImoId instrId);
    void delete_instrument(AInstrument& instr);
    void move_up_instrument(ImoId instrId);
    void move_up_instrument(AInstrument& instr);
    void move_down_instrument(ImoId instrId);
    void move_down_instrument(AInstrument& instr);
    AInstrument instrument_at(int iInstr) const;   //0..n-1
    int num_instruments() const;

    //instrument groups
    int num_instruments_groups() const;
    AInstrGroup instruments_group_at(int iGroup) const;   //0..n-1
    AInstrGroup group_instruments(int iFirstInstr, int iLastInstr);
    bool delete_instruments_group_at(int iGroup);   //0..n-1
    bool delete_instruments_group(const AInstrGroup& group);
    void delete_all_instruments_groups();

    //Algorithms
    MeasureLocator locator_for(TimeUnits timepos, int iInstr=0);
    TimeUnits timepos_for(int iMeasure, int iBeat, int iInstr=0);
    TimeUnits timepos_for(const MeasureLocator& ml);

    //inform that you have finished modifying this score
    void end_of_changes();

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoScore* internal_object() const;

};


//---------------------------------------------------------------------------------------
// ASoundInfo class contains and manages the information for one sound, such as its
// MIDI values. It always contains a AMidiInfo object.
// An AInstrument always have at least one sound but can have more. For each sound there
// is a ASoundInfo object and its associated AMidiInfo object.
// See: https://lenmus.github.io/lomse/classASoundInfo.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT ASoundInfo : public virtual AObject
{
    LOMSE_DECLARE_IM_API_CLASS(ASoundInfo, ImoSoundInfo)
    friend class AInstrument;

    //access to MIDI info
    AMidiInfo midi_info() const;

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoSoundInfo* internal_object() const;
};


//---------------------------------------------------------------------------------------
// ATextItem is an inline-level object containing a chunk of text with the same style.
// See: https://lenmus.github.io/lomse/classATextItem.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT ATextItem : public virtual AObject, public ISiblings
{
    LOMSE_DECLARE_IM_API_CLASS(ATextItem, ImoTextItem)

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoTextItem* internal_object() const;
};

#endif      //if (LOMSE_BYPASS_ISSUE_253 == 1)


}   //namespace lomse
///@endcond

#endif    // __LOMSE_INTERNAL_MODEL_H__



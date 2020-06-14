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

#include "lomse_basic.h"
#include "lomse_build_options.h"
#include "lomse_api_definitions.h"
#include "private/lomse_internal_model_p.h"

//std
#include <string>
#include <list>

namespace lomse
{

//forward declaration of the API classes and the implementation classes
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

class IBlockLevelObj;
class IDocument;
class IDynamic;
class IObject;
class IInstrument;
class IInstrGroup;
class ILink;
class IList;
class IMidiInfo;
class IParagraph;
class IScore;
class ISoundInfo;
class ITextItem;

//---------------------------------------------------------------------------------------
// IObject: Abstract base class for all objects composing the document
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IObject
{
protected:
    IObject downcast_to_content_obj();
    LOMSE_DECLARE_IM_API_ROOT_CLASS(IObject, ImoObj)

public:
    //properties
    ImoId object_id() const;
    const std::string& object_name() const;
    IDocument owner_document() const;

    //downcast objects
    IDynamic downcast_to_dynamic() const;
    IInstrument downcast_to_instrument() const;
    IInstrGroup downcast_to_instr_group() const;
    ILink downcast_to_link() const;
    IList downcast_to_list() const;
    IParagraph downcast_to_paragraph() const;
    IScore downcast_to_score() const;
    ITextItem downcast_to_text_item() const;

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
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT ISiblings : public virtual IObject
{
    LOMSE_DECLARE_IM_API_CLASS(ISiblings, ImoObj)

    //Document content traversal
    IObject previous_sibling() const;
    IObject next_sibling() const;
};


//---------------------------------------------------------------------------------------
// IChildren class provides child traversal method for objects supporting them
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IChildren : public virtual IObject
{
    LOMSE_DECLARE_IM_API_CLASS(IChildren, ImoObj)

    //Document content traversal
    int num_children() const;
    IObject child_at(int iItem) const;
    IObject first_child() const;
    IObject last_child() const;

};


//---------------------------------------------------------------------------------------
// IDynamic represents external content that is injected dynamically into the document
// by the user application. It is equivalent to the HTML \<object\> element.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IDynamic : public virtual IObject, public ISiblings, public IChildren
{
    LOMSE_DECLARE_IM_API_CLASS(IDynamic, ImoDynamic)
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
// IInstrument represents an instrument in the score.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IInstrument : public virtual IObject
{
    LOMSE_DECLARE_IM_API_CLASS(IInstrument, ImoInstrument)
    friend class IScore;
    friend class IInstrGroup;

    // name and abbreviation
    std::string& name_string() const;
    std::string& abbreviation_string() const;
    void set_name_string(const std::string& name);
    void set_abbreviation_string(const std::string& abbrev);

    // sound information
    int num_sounds() const;
    ISoundInfo sound_info_at(int iSound) const;

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoInstrument* internal_object() const;
};


//---------------------------------------------------------------------------------------
// IInstrGroup provides access to the information for an instruments group.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IInstrGroup : public virtual IObject
{
    LOMSE_DECLARE_IM_API_CLASS(IInstrGroup, ImoInstrGroup)
    friend class IScore;

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
    IInstrument instrument_at(int iInstr) const;  //iInstr = 0..num-instrs-in-group - 1
    IInstrument first_instrument() const;
    IInstrument last_instrument() const;
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
// ILink is a container for inline objects, and reprensents a clickable 'link'
// object that creates hyperlinks. It is similar to the HTML \<a\> element.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT ILink : public virtual IObject, public ISiblings, public IChildren
{
    LOMSE_DECLARE_IM_API_CLASS(ILink, ImoLink)

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoLink* internal_object() const;
};


//---------------------------------------------------------------------------------------
// IList represents a list of items and it is a container for IListItem objects.
// It is equivalent to the HTML \<ol\> and \<ul\> elements.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IList : public virtual IObject, public ISiblings, public IChildren
{
    LOMSE_DECLARE_IM_API_CLASS(IList, ImoList)

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoList* internal_object() const;
};


//---------------------------------------------------------------------------------------
// IMidiInfo provides access to the MIDI information associated to a ISoundInfo object
// for an instrument. MIDI info always exists in the ISoundInfo object.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IMidiInfo : public virtual IObject
{
    LOMSE_DECLARE_IM_API_CLASS(IMidiInfo, ImoMidiInfo)
    friend class ISoundInfo;

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
// IParagraph represents a paragraph. It is a block-level container, similar to the
// HTML <p> element.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IParagraph : public virtual IObject, public ISiblings, public IChildren
{
    LOMSE_DECLARE_IM_API_CLASS(IParagraph, ImoParagraph)

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoScore* internal_object() const;

};



//---------------------------------------------------------------------------------------
// IScore is the API object for interacting with the internal model for a music score.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IScore : public virtual IObject, public ISiblings
{
    LOMSE_DECLARE_IM_API_CLASS(IScore, ImoScore)

    //instruments: create/delete/move instruments and get information
    IInstrument append_new_instrument();
    void delete_instrument(ImoId instrId);
    void delete_instrument(IInstrument& instr);
    void move_up_instrument(ImoId instrId);
    void move_up_instrument(IInstrument& instr);
    void move_down_instrument(ImoId instrId);
    void move_down_instrument(IInstrument& instr);
    IInstrument instrument_at(int iInstr) const;   //0..n-1
    int num_instruments() const;

    //instrument groups
    int num_instruments_groups() const;
    IInstrGroup instruments_group_at(int iGroup) const;   //0..n-1
    IInstrGroup create_instruments_group(int iFirstInstr, int iLastInstr);
    bool delete_instruments_group_at(int iGroup);   //0..n-1
    bool delete_instruments_group(const IInstrGroup& group);
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
// ISoundInfo class contains and manages the information for one sound, such as its
// MIDI values. It always contains a IMidiInfo object.
// An IInstrument always have at least one sound but can have more. For each sound there
// is a ISoundInfo object and its associated IMidiInfo object.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT ISoundInfo : public virtual IObject
{
    LOMSE_DECLARE_IM_API_CLASS(ISoundInfo, ImoSoundInfo)
    friend class IInstrument;

//    //getters
//    std::string& score_instr_name();
//    std::string& score_instr_abbrev();
//    std::string& score_instr_sound();
//    bool score_instr_solo();
//    bool score_instr_ensemble();
//    int score_instr_ensemble_size();
//    std::string& score_instr_virtual_library();
//    std::string& score_instr_virtual_name();
//
//    //setters
//    void set_score_instr_name(const std::string& value);
//    void set_score_instr_abbrev(const std::string& value);
//    void set_score_instr_sound(const std::string& value);
//    void set_score_instr_solo(bool value);
//    void set_score_instr_ensemble(bool value);
//    void set_score_instr_ensemble_size(int value);
//    void set_score_instr_virtual_library(const std::string& value);
//    void set_score_instr_virtual_name(const std::string& value),

    //access to MIDI info
    IMidiInfo midi_info() const;

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoSoundInfo* internal_object() const;
};


//---------------------------------------------------------------------------------------
// ITextItem is an inline-level object containing a chunk of text with the same style.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT ITextItem : public virtual IObject, public ISiblings
{
    LOMSE_DECLARE_IM_API_CLASS(ITextItem, ImoTextItem)

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoTextItem* internal_object() const;
};


////---------------------------------------------------------------------------------------
//// IXxxxx is the API object for interacting with
//// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
//// for all class members.
////
//class LOMSE_EXPORT IXxxxx : public virtual IObject
//{
//    LOMSE_DECLARE_IM_API_CLASS(IXxxxx, ImoXxxxx)
//    friend class IInstrument;
//
//    // put here the member methods ---------------------
//            //void do_something();
//    // end of member methods ---------------------------
//
//    // Transitional, to facilitate migration to this new public API.
//    // Notice that this method will be removed in future so, please, if you need to
//    // use this method open an issue at https://github.com/lenmus/lomse/issues
//    // explaining the need, so that the public API could be fixed and your app.
//    // would not be affected in future when this method is removed.
//    ImoXxxxx* internal_object() const;
//};


}   //namespace lomse

#endif    // __LOMSE_INTERNAL_MODEL_H__



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
class ImoInstrument;
class ImoInstrGroup;
class ImoMidiInfo;
class ImoParagraph;
class ImoScore;
class ImoSoundInfo;

class IBlockLevelObj;
class IDocument;
class IObject;
class IInstrument;
class IInstrGroup;
class IMidiInfo;
class IParagraph;
class IScore;
class ISoundInfo;

//---------------------------------------------------------------------------------------
// IObject: Abstract base class for all objects composing the document
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IObject
{
protected:
    std::unique_ptr<IObject> downcast_to_content_obj();
    LOMSE_DECLARE_IM_API_ROOT_CLASS(IObject, ImoObj)

public:
    //properties
    ImoId get_object_id() const;
    const std::string& get_object_name() const;
    std::unique_ptr<IDocument> get_owner_document() const;

    //downcast objects
    std::unique_ptr<IParagraph> downcast_to_paragraph() const;
    std::unique_ptr<IScore> downcast_to_score() const;

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
    bool is_listitem() const;
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
    ImoObj* get_internal_object() const;
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
    std::unique_ptr<IObject> get_previous_sibling() const;
    std::unique_ptr<IObject> get_next_sibling() const;
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
    int get_num_children() const;
    std::unique_ptr<IObject> get_child_at(int iItem) const;
    std::unique_ptr<IObject> get_first_child() const;
    std::unique_ptr<IObject> get_last_child() const;

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
    std::string& get_name_string() const;
    std::string& get_abbreviation_string() const;
    void set_name_string(const std::string& name);
    void set_abbreviation_string(const std::string& abbrev);

    // sound information
    int get_num_sounds() const;
    std::unique_ptr<ISoundInfo> get_sound_info_at(int iSound) const;

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoInstrument* get_internal_object() const;
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
    EJoinBarlines get_barlines_mode() const;
    EGroupSymbol get_symbol() const;
    const std::string& get_name_string() const;
    const std::string& get_abbreviation_string() const;

    //group properties modification
    void set_name_string(const std::string& text);
    void set_abbreviation_string(const std::string& text);
    void set_symbol(EGroupSymbol symbol);
    void set_barlines_mode(EJoinBarlines value);

    //instruments in the group
    int get_num_instruments() const;
    std::unique_ptr<IInstrument> get_instrument_at(int iInstr) const;  //iInstr = 0..num-instrs-in-group - 1
    std::unique_ptr<IInstrument> get_first_instrument() const;
    std::unique_ptr<IInstrument> get_last_instrument() const;
    int get_index_to_first_instrument() const;
    int get_index_to_last_instrument() const;
    bool set_range(int iFirstInstr, int iLastInstr);

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoInstrGroup* get_internal_object() const;
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
    int get_port() const;
    std::string& get_device_name() const;
    std::string& get_program_name() const;
    int get_bank() const;
    int get_channel() const;
    int get_program() const;
    int get_unpitched() const;
    float get_volume() const;
    int get_pan() const;
    int get_elevation() const;

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
    ImoMidiInfo* get_internal_object() const;
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
    ImoScore* get_internal_object() const;

};



//---------------------------------------------------------------------------------------
// IScore is the API object for interacting with the internal model for a music score.
// See: https://lenmus.github.io/lomse/classDocument.html  for details and documentation
// for all class members.
//
class LOMSE_EXPORT IScore : public virtual IObject, public ISiblings
{
    LOMSE_DECLARE_IM_API_CLASS(IScore, ImoScore)
    friend class IDocument;

    //instruments: create/delete/move instruments and get information
    std::unique_ptr<IInstrument> append_new_instrument();
    void delete_instrument(ImoId instrId);
    void delete_instrument(IInstrument& instr);
    void move_up_instrument(ImoId instrId);
    void move_up_instrument(IInstrument& instr);
    void move_down_instrument(ImoId instrId);
    void move_down_instrument(IInstrument& instr);
    std::unique_ptr<IInstrument> get_instrument_at(int iInstr) const;   //0..n-1
    int get_num_instruments() const;

    //instrument groups
    int get_num_instruments_groups() const;
    std::unique_ptr<IInstrGroup> get_instruments_group_at(int iGroup) const;   //0..n-1
    std::unique_ptr<IInstrGroup> create_instruments_group(int iFirstInstr, int iLastInstr);
    bool delete_instruments_group_at(int iGroup);   //0..n-1
    bool delete_instruments_group(const IInstrGroup& group);
    void delete_all_instruments_groups();

    //inform that you have finished modifying this score
    void end_of_changes();

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoScore* get_internal_object() const;

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
//    std::string& get_score_instr_name();
//    std::string& get_score_instr_abbrev();
//    std::string& get_score_instr_sound();
//    bool get_score_instr_solo();
//    bool get_score_instr_ensemble();
//    int get_score_instr_ensemble_size();
//    std::string& get_score_instr_virtual_library();
//    std::string& get_score_instr_virtual_name();
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
    std::unique_ptr<IMidiInfo> get_midi_info() const;

    // Transitional, to facilitate migration to this new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app.
    // would not be affected in future when this method is removed.
    ImoSoundInfo* get_internal_object() const;
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
//    ImoXxxxx* get_internal_object() const;
//};


}   //namespace lomse

#endif    // __LOMSE_INTERNAL_MODEL_H__



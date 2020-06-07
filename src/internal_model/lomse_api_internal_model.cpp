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

#include "lomse_internal_model.h"
#include "private/lomse_internal_model_p.h"

#include "lomse_document.h"
#include "lomse_model_builder.h"
#include "lomse_im_factory.h"


using namespace std;

namespace lomse
{

//=======================================================================================
// helper macro to simplify the implementation of internal model API objects

#define LOMSE_IMPLEMENT_IM_API_ROOT_CLASS(IXxxx, ImoXxxx) \
    IXxxx::IXxxx(ImoXxxx* impl, Document* doc, long imVers) \
        : m_pImpl(impl) \
        , m_pDoc(doc) \
        , m_id(impl->get_id()) \
        , m_imVersion(imVers) \
    { \
    } \
    IXxxx::IXxxx() \
        : m_pImpl(nullptr) \
        , m_pDoc(nullptr) \
        , m_id(-1L) \
        , m_imVersion(-1L) \
    { \
    } \
    void IXxxx::ensure_validity() const \
    { \
        if (!m_pDoc->is_valid_model(m_imVersion)) \
        { \
            m_imVersion = m_pDoc->get_model_ref(); \
            m_pImpl = static_cast<ImoXxxx*>(m_pDoc->get_pointer_to_imo(m_id)); \
        } \
    }


#define LOMSE_IMPLEMENT_IM_API_CLASS(IXxxx, ImoXxxx, IParentClass) \
    IXxxx::IXxxx(ImoXxxx* impl, Document* doc, long imVers) \
        : IParentClass(impl, doc, imVers) \
    { \
    } \
    IXxxx::IXxxx() \
        : IParentClass() \
    { \
    } \



//---------------------------------------------------------------------------------------
/** @enum EJoinBarlines

    Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod
    tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam,
    quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.

    Option 'k_truncate_barline_final' is the default behavior and it can be useful
    for score editors: staff lines will run always to right margin until a barline of
    type final is entered.

    Option 'k_truncate_always' truncates staff lines after last staff object. It can
    be useful for creating score samples (i.e. for ebooks).
        k_isolated=0,       Independent barlines for each score part
        k_joined,           Barlines joined across all parts
        k_mensurstrich,     Barlines only in the gaps between parts, but not on
                            the staves of each part

    @#include <lomse_api_definitions.h>
*/
//---------------------------------------------------------------------------------------
/** @enum EGroupSymbol

    When several instruments form a group this enum indicates if a symbol for
    the group should be displayed in the score and what symbol to use.

    @#include <lomse_api_definitions.h>
*/

/** @file lomse_api_definitions.h
*/
//---------------------------------------------------------------------------------------
//This does not work, but there are neither warnings nor errors !
/* * @enum EGroupSymbol
    @var k_group_symbol_brace
    @brief Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod
           tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam,
           quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo
           consequat.
*/
/* * @enum EGroupSymbol
    @var EGroupSymbol::k_group_symbol_brace
    @brief Lorem ipsum dolor sit amet, consectetur adipiscing elit,
*/
/* * @enum EGroupSymbol
    @var EGroupSymbol::k_group_symbol_brace
    @brief Lorem ipsum dolor sit amet,
*/



//=======================================================================================
/** @class INode
    Abstract base class for all objects composing the document
*/
LOMSE_IMPLEMENT_IM_API_ROOT_CLASS(INode, ImoObj)

//---------------------------------------------------------------------------------------
/** @memberof INode
    Returns the internal unique identifier (ID) for this object. It could be required
    by some methods in the libray.
*/
ImoId INode::get_object_id() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->get_id();
}



//=======================================================================================
/** @class IElement
    @extends INode
    Abstract base class for all elements composing the document
*/
LOMSE_IMPLEMENT_IM_API_CLASS(IElement, ImoContentObj, INode)

/// @name Content traversal
//@{

//---------------------------------------------------------------------------------------
/** @memberof IElement
    Returns
*/
int IElement::get_num_children() const
{
    ensure_validity();
//    ImoScoreText& text = const_cast<ImoInstrument*>(pimpl())->get_name();
//    return text.get_text();
    return 0;    //TODO
}

//---------------------------------------------------------------------------------------
/** @memberof IElement
    Returns
*/
std::unique_ptr<IElement> IElement::get_child_at(int iItem) const
{
    ensure_validity();
//    ImoScoreText& text = const_cast<ImoInstrument*>(pimpl())->get_name();
//    return text.get_text();
    return unique_ptr<IElement>();    //TODO
}

//---------------------------------------------------------------------------------------
/** @memberof IElement
    Returns
*/
std::unique_ptr<IElement> IElement::get_first_child() const
{
    ensure_validity();
//    ImoScoreText& text = const_cast<ImoInstrument*>(pimpl())->get_name();
//    return text.get_text();
    return unique_ptr<IElement>();    //TODO
}

//---------------------------------------------------------------------------------------
/** @memberof IElement
    Returns
*/
std::unique_ptr<IElement> IElement::get_last_child() const
{
    ensure_validity();
//    ImoScoreText& text = const_cast<ImoInstrument*>(pimpl())->get_name();
//    return text.get_text();
    return unique_ptr<IElement>();    //TODO
}

//---------------------------------------------------------------------------------------
/** @memberof IElement
    Returns
*/
std::unique_ptr<IElement> IElement::get_previous_sibling() const
{
    ensure_validity();
//    ImoScoreText& text = const_cast<ImoInstrument*>(pimpl())->get_name();
//    return text.get_text();
    return unique_ptr<IElement>();
}

//---------------------------------------------------------------------------------------
/** @memberof IElement
    Returns
*/
std::unique_ptr<IElement> IElement::get_next_sibling() const
{
    ensure_validity();
//    ImoScoreText& text = const_cast<ImoInstrument*>(pimpl())->get_name();
//    return text.get_text();
    return unique_ptr<IElement>();
}

//@}    //Content traversal

//---------------------------------------------------------------------------------------
/** @memberof IElement
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoContentObj* IElement::get_internal_object() const
{
    return const_cast<ImoContentObj*>(pimpl());
}



//=======================================================================================
/** @class IInstrument
    @extends INode
    %IInstrument represents an instrument on the score. For lomse, an instrument refers
    to a physical musical instrument, such as a violin, a flute, or a piano. It is
    represented by one or more staves and is modeled as the collection of all aspects
    pertaining to the visual display of the staff/staves as they appear on the printed
    page (name, transposition, musical content, etc) as well as the required information
    for audio playback (e.g. MIDI channel, MIDI program, etc.).

    But there are cases in which the staves for an instrument contain the music for
    more than one real instrument. For instance the bass and tenor voices in a choral
    can be placed in a single staff, or the different percussion instruments can share
    an staff.

    In theory, sharing  the same staff between several instruments should be forbidden,
    and each instrument should be modeled in its own staff. And for presentation, the
    user should be able to decide which instruments are going to form an score part,
    and how that part should be displayed: all instruments merged in a single staff or
    other layout. But for compatibility with MusicXML, lomse allows that several
    instruments share the same staves. As a consequence, a lomse instrument could
    represent several real instruments and, thus, could have several sound information
    objects (ISoundInfo objects) each of them containing the sound information for each
    real physical instrument that is sharing the staff.
*/
LOMSE_IMPLEMENT_IM_API_CLASS(IInstrument, ImoInstrument, INode)


/// @name Name and abbreviation
//@{

//---------------------------------------------------------------------------------------
/** @memberof IInstrument
    Returns the name of the instrument, that is the string that is placed at the start of
    the first system in the score.
*/
std::string& IInstrument::get_name_string() const
{
    ensure_validity();
    ImoScoreText& text = const_cast<ImoInstrument*>(pimpl())->get_name();
    return text.get_text();
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrument
    Returns the short, abbreviated name that appears on every system other than
    the first system.
*/
std::string& IInstrument::get_abbreviation_string() const
{
    ensure_validity();
    ImoScoreText& text = const_cast<ImoInstrument*>(pimpl())->get_abbrev();
    return text.get_text();
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrument
    Sets the name of the instrument, that is the string that is placed at the start of
    the first system in the score.

    It’s important to note that changing the name of an instrument doesn’t actually
    change any other instrument properties, such as transposition information or
    number of staves. For example, renaming "Flute" to "Piano" does not creates a
    second staff for the instrument. Or renaming  “Trumpets in Bb” to “Trumpets in C”
    does not change the transposition information.
*/
void IInstrument::set_name_string(const string& name)
{
    ensure_validity();
    pimpl()->set_name(name);
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrument
    Sets the short, abbreviated name that appears on every system other than
    the first system.
*/
void IInstrument::set_abbreviation_string(const string& abbrev)
{
    ensure_validity();
    pimpl()->set_abbrev(abbrev);
}

//@}    //Name and abbreviation


/// @name Sound information
//@{

//---------------------------------------------------------------------------------------
/** @memberof IInstrument
    Returns the number of ISoundInfo elements contained in this instrument, at least one.

    An %IInstruments always have at least one sound, represented by an ISoundInfo object.
    But there are cases in which the staves for an instrument contain the music for
    more than one real instrument. For instance the bass and tenor voices in a choral
    can be placed in a single staff, or the different percussion instruments can share
    an staff. In these cases in which a lomse instrument represents several real
    instruments there is aISoundInfo object for each real physical instrument that is
    sharing the staff.

    Method IInstrument::get_num_sounds() informs about the number of ISoundInfo objects
    that this instrument contains. Always at least one.
*/
int IInstrument::get_num_sounds() const
{
    ensure_validity();
    return const_cast<ImoInstrument*>(pimpl())->get_num_sounds();
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrument
    Returns the ISoundInfo object at a given position in the collection of sounds
    for this instrument.

    In cases in which a lomse instrument represents several real
    instruments there is a collection of ISoundInfo objects, one for each real physical
    instrument that is sharing the staff.

    IInstrument::get_sound_info_at(0) is always valid and returns the only ISoundInfo
    object for normal cases and the first ISoundInfo object when several real instruments
    are represented by a single IInstrument object.

    @param iSound   The index (0..n-1) to the requested sound info object.

*/
std::unique_ptr<ISoundInfo> IInstrument::get_sound_info_at(int iSound) const
{
    ensure_validity();
    return unique_ptr<ISoundInfo>(
                new ISoundInfo(const_cast<ImoInstrument*>(pimpl())->get_sound_info(iSound),
                               m_pDoc, m_imVersion) );
}

//@}    //Sound information


//---------------------------------------------------------------------------------------
/** @memberof IInstrument
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoInstrument* IInstrument::get_internal_object() const
{
    return const_cast<ImoInstrument*>(pimpl());
}


//=======================================================================================
/** @class IInstrGroup
    @extends INode
    %IInstrGroup is the API object for interacting with the internal model Bla,bla bla...
*/
LOMSE_IMPLEMENT_IM_API_CLASS(IInstrGroup, ImoInstrGroup, INode)

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoInstrGroup* IInstrGroup::get_internal_object() const
{
    return const_cast<ImoInstrGroup*>(pimpl());
}


/// @name Access to group properties
//@{

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Returns a value from enum EJoinBarlines indicating how the barlines for the
    instruments in the group will be displayed.
*/
EJoinBarlines IInstrGroup::get_barlines_mode() const
{
    ensure_validity();
    return static_cast<EJoinBarlines>(
        const_cast<ImoInstrGroup*>(pimpl())->join_barlines() );
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Returns a value from enum EGroupSymbol indicating what symbol will be displayed for
    marking the instruments that form the group.
*/
EGroupSymbol IInstrGroup::get_symbol() const
{
    ensure_validity();
    return static_cast<EGroupSymbol>(
        const_cast<ImoInstrGroup*>(pimpl())->get_symbol() );
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Returns the name of the group, that is the string for the group that is placed at
    the start of the first system in the score.
*/
const std::string& IInstrGroup::get_name_string() const
{
    ensure_validity();
    ImoScoreText& text = const_cast<ImoInstrGroup*>(pimpl())->get_name();
    return text.get_text();
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Returns the short, abbreviated name of the group. This is the string for the group
    that appears on every system other than the first system.
*/
const std::string& IInstrGroup::get_abbreviation_string() const
{
    ensure_validity();
    ImoScoreText& text = const_cast<ImoInstrGroup*>(pimpl())->get_abbrev();
    return text.get_text();
}

//@}    //Access to group properties


/// @name Group properties modification
//@{

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Sets the symbol that will be displayed for marking the instruments that form the
    group. Must be a value from enum EGroupSymbol.
*/
void IInstrGroup::set_symbol(EGroupSymbol symbol)
{
    ensure_validity();
    pimpl()->set_symbol(symbol);
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Defines how the barlines for the instruments in the group will be displayed.
    Parameter must be a value from enum EJoinBarlines.
*/
void IInstrGroup::set_barlines_mode(EJoinBarlines value)
{
    ensure_validity();
    pimpl()->set_join_barlines(value);
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Sets the name of the group, that is the string for the group that is placed at the
    start of the first system in the score.
*/
void IInstrGroup::set_name_string(const std::string& name)
{
    ensure_validity();
    pimpl()->set_name(name);
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Sets the short, abbreviated name for the group that appears on every system other
    than the first one.
*/
void IInstrGroup::set_abbreviation_string(const std::string& abbrev)
{
    ensure_validity();
    pimpl()->set_abbrev(abbrev);
}

//@}    //Group properties modification


/// @name Instruments in the group
//@{

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Returns the number of instruments included in the group.
*/
int IInstrGroup::get_num_instruments() const
{
    ensure_validity();
    return const_cast<ImoInstrGroup*>(pimpl())->get_num_instruments();
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Returns the instrument at position <i>pos</i> in the group. First instrument in the
    group is position 0.
*/
std::unique_ptr<IInstrument> IInstrGroup::get_instrument_at(int pos) const
{
    ensure_validity();
    ImoInstrGroup* pGrp = const_cast<ImoInstrGroup*>(pimpl());
    if (pos < 0 || pos >= pGrp->get_num_instruments())
        return unique_ptr<IInstrument>();

    return unique_ptr<IInstrument>(
                    new IInstrument(pGrp->get_instrument(pos), m_pDoc, m_imVersion));
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Returns the first instrument included in the group.
*/
std::unique_ptr<IInstrument> IInstrGroup::get_first_instrument() const
{
    ensure_validity();
    return unique_ptr<IInstrument>(
                new IInstrument(const_cast<ImoInstrGroup*>(pimpl())->get_first_instrument(),
                                m_pDoc, m_imVersion) );
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Returns the last instrument included in the group.
*/
std::unique_ptr<IInstrument> IInstrGroup::get_last_instrument() const
{
    ensure_validity();
    return unique_ptr<IInstrument>(
                new IInstrument(const_cast<ImoInstrGroup*>(pimpl())->get_last_instrument(),
                                m_pDoc, m_imVersion) );
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Returns the index, referred to the score, of the first instrument included in the
    group. The returned index is the position occupied by this instrument in the score
    (0 based: 0 .. num.instrs - 1)
*/
int IInstrGroup::get_index_to_first_instrument() const
{
    ensure_validity();
    return const_cast<ImoInstrGroup*>(pimpl())->get_index_to_first_instrument();
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Returns the index, referred to the score, of the last instrument included in the
    group. The returned index is the position occupied by this instrument in the score
    (0 based: 0 .. num.instrs - 1)
*/
int IInstrGroup::get_index_to_last_instrument() const
{
    ensure_validity();
    return const_cast<ImoInstrGroup*>(pimpl())->get_index_to_last_instrument();
}

//---------------------------------------------------------------------------------------
/** @memberof IInstrGroup
    Defines the instruments that will be included in the group. Returns @FALSE if
    any error.
    @param iFirstInstr  Position in the score of the first instrument to be included in
        the group: 0 .. num.instrs - 1
    @param iLastInstr  Position in the score of the last instrument to be included in
        the group: 0 .. num.instrs - 1

    Notice that <i>iFirstInstr</i> must be lower than <i>iLastInstr</i>. Otherwise
    this method will do nothing.
*/
bool IInstrGroup::set_range(int iFirstInstr, int iLastInstr)
{
    ensure_validity();
    ImoScore* pScore = pimpl()->get_score();
    int maxInstr = pScore->get_num_instruments();
    if ((iFirstInstr >= 0 && iFirstInstr < maxInstr)
        && (iLastInstr > iFirstInstr && iLastInstr < maxInstr) )
    {
        pimpl()->set_range(iFirstInstr, iLastInstr);
        return true;
    }
    else
    {
        return false;
    }
}

//@}    //Instruments in the group



//=======================================================================================
/** @class IMidiInfo
    @extends INode
    %IMidiInfo provides access to the MIDI information associated to a ISoundInfo object
    for an instrument. MIDI info always exists in the ISoundInfo object.
    By default, when no MIDI information is provided in the source file or when
    programatically building a score, MIDI information is initialized as follows:

        device name = ""
        program name = ""
        bank = 1
        port = 0        port 0 is invalid. Means "not initialized"
        channel = 0     channel 0 is invalid. Means "not initialized"
        program = 1
        unpitched = 0
        volume = 1.0    maximum volume
        pan = 0         in front, centered
        elevation = 0   at listener head level

    When MIDI information has been specified when building the score, notice that
    'port', 'channel' or both could have not been specified and, thus, any of them can
    still in "not initialized". In these cases, port and channel are automatically
    assigned right values when the score is finished, that is, when invoking
    IScore::end_of_changes() method. The algorithm, in method
    MidiAssigner::assign_port_and_channel(), ensures that
    each instrument is assigned a unique combination (port, channel) for all instruments
    with port or channel containig the "not initialized" value.

    Important. Please notice that currently, lomse sound API for playback generate play
    events that only uses the MIDI channel, program and volume information. All other
    MIDI information, such as bank, port or elevation is currently ignored.
*/
LOMSE_IMPLEMENT_IM_API_CLASS(IMidiInfo, ImoMidiInfo, INode)

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoMidiInfo* IMidiInfo::get_internal_object() const
{
    return const_cast<ImoMidiInfo*>(pimpl());
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Returns the MIDI port assigned to this sound.
    It is a number from 1 to 16 that can be used with the unofficial MIDI port
    (or cable) meta event.
*/
int IMidiInfo::get_port() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_port();
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Returns the MIDI device name assigned to this sound.
    It will be used in the DeviceName meta-event when exporting the score as a
    Standard MIDI File (not yet implemented).
*/
std::string& IMidiInfo::get_device_name() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_device_name();
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Returns the MIDI program name assigned to this sound.
    It will be used in the ProgramName meta-events when exporting the score as
    a Standard MIDI File (not yet implemented).
*/
std::string& IMidiInfo::get_program_name() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_name();
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Returns the MIDI bank assigned to this sound. MIDI 1.0 bank numbers range
    from 1 to 16,384.
*/
int IMidiInfo::get_bank() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_bank() + 1;
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Returns the MIDI channel assigned to this sound. MIDI 1.0 channel numbers range
    from 1 to 16.
*/
int IMidiInfo::get_channel() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_channel() + 1;
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Returns the MIDI program number assigned to this sound. MIDI 1.0 program numbers
    range from 1 to 128.
*/
int IMidiInfo::get_program() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_program() + 1;
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Returns the unpitched note number assigned to this sound. It is only meaningfull
    for unpitched instruments and, for them, it specifies a MIDI 1.0 note number
    ranging from 0 to 127. It is usually used with MIDI banks for percussion.
*/
int IMidiInfo::get_unpitched() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_unpitched();
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Returns the MIDI volume assigned to this sound.
    The volume value is a percentage of the maximum, ranging from 0.0 to 1.0, with
    decimal values allowed.
    This corresponds to a scaling value for the MIDI 1.0
    channel volume controller.
*/
float IMidiInfo::get_volume() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_volume();
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Returns the MIDI pan value assigned to this sound.
    Pan and elevation allow placing of sound in a 3-D space relative to the listener.
    Pan refers to the horizontal position around the listener, expressed
    in degrees, ranging from -180 to 180. Some values:
          0 is straight ahead, in front of the listener, centered.
        -90 is hard left, 90 is hard right, and
       -180 or 180 are directly behind the listener, centered.
*/
int IMidiInfo::get_pan() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_pan();
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Returns the MIDI elevation value assigned to this sound.
    Pan and elevation allow placing of sound in a 3-D space relative to the listener.
    Elevation refers to the vertical position around the listener, expressed
    in degrees, ranging from -180 to 180. Some values:
         0 is level with the listener head,
        90 is directly above, and -90 is directly below.
*/
int IMidiInfo::get_elevation() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_elevation();
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Sets the MIDI port to use for this sound.
    It is a number from 1 to 16 that can be used with the unofficial MIDI port
    (or cable) meta event.
*/
void IMidiInfo::set_port(int value)
{
    ensure_validity();
    pimpl()->set_midi_port(value - 1);
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Sets the MIDI device name to use for this sound.
    It will be used in the DeviceName meta-event when exporting the score as a
    Standard MIDI File (not yet implemented).
*/
void IMidiInfo::set_device_name(const std::string& value)
{
    ensure_validity();
    pimpl()->set_midi_device_name(value);
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Sets the MIDI program name to use for this sound.
    It will be used in the ProgramName meta-events when exporting the score as
    a Standard MIDI File (not yet implemented).
*/
void IMidiInfo::set_program_name(const std::string& value)
{
    ensure_validity();
    pimpl()->set_midi_name(value);
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Sets the MIDI bank to use for this sound. MIDI 1.0 bank numbers range
    from 1 to 16,384.
*/
void IMidiInfo::set_bank(int value)
{
    ensure_validity();
    pimpl()->set_midi_bank(value - 1);
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Sets the MIDI channel to use for this sound. MIDI 1.0 channel numbers range
    from 1 to 16.
*/
void IMidiInfo::set_channel(int value)
{
    ensure_validity();
    pimpl()->set_midi_channel(value - 1);
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Sets the MIDI program number to use for this sound. MIDI 1.0 program numbers
    range from 1 to 128.
*/
void IMidiInfo::set_program(int value)
{
    ensure_validity();
    pimpl()->set_midi_program(value - 1);
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Sets the unpitched note number to use for this sound. It is only meaningfull
    for unpitched instruments and, for them, it specifies a MIDI 1.0 note number
    ranging from 0 to 127. It is usually used with MIDI banks for percussion.
*/
void IMidiInfo::set_unpitched(int value)
{
    ensure_validity();
    pimpl()->set_midi_unpitched(value);
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Sets the MIDI volume to use for this sound.
    The volume value is a percentage of the maximum, ranging from 0.0 to 1.0, with
    decimal values allowed.
    This corresponds to a scaling value for the MIDI 1.0
    channel volume controller.
*/
void IMidiInfo::set_volume(float value)
{
    ensure_validity();
    pimpl()->set_midi_volume(value);
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Sets the MIDI pan value to use for this sound.
    Pan and elevation allow placing of sound in a 3-D space relative to the listener.
    Pan refers to the horizontal position around the listener, expressed
    in degrees, ranging from -180 to 180. Some values:
          0 is straight ahead, in front of the listener, centered.
        -90 is hard left, 90 is hard right, and
       -180 or 180 are directly behind the listener, centered.
*/
void IMidiInfo::set_pan(int value)
{
    ensure_validity();
    pimpl()->set_midi_pan(value);
}

//---------------------------------------------------------------------------------------
/** @memberof IMidiInfo
    Sets the MIDI pan value to use for this sound.
    Pan and elevation allow placing of sound in a 3-D space relative to the listener.
    Elevation refers to the vertical position around the listener, expressed
    in degrees, ranging from -180 to 180. Some values:
         0 is level with the listener head,
        90 is directly above, and -90 is directly below.
*/
void IMidiInfo::set_elevation(int value)
{
    ensure_validity();
    pimpl()->set_midi_elevation(value);
}



//=======================================================================================
/** @class IScore
    @extends IElement
    %IScore represents a full music score, that is, an object comprising all of the
    music for all of the players and their instruments, typically laid out in a specific
    order. In lomse, an score is, basically, a collection instruments (IInstrument
    objects) and some information common to all them, such as score titles.

    In a full score, related instruments are usually grouped by sharing barlines and
    having some visual clues, such as a brace or bracket, and a group name. In lomse, a
    group of instruments is represented by an IInstrGroup object, and the IScore object
    is also responsible for managing the collection of all defined instrument groups.
*/
LOMSE_IMPLEMENT_IM_API_CLASS(IScore, ImoScore, IElement)

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoScore* IScore::get_internal_object() const
{
    return const_cast<ImoScore*>(pimpl());
}

//exclude from API documentation all private members
///@cond INTERNALS

struct IScore::Private
{
    //-----------------------------------------------------------------------------------
    static void delete_instrument(ImoScore* pScore, ImoInstrument* pInstr)
    {
        //remove the instrument from the score
        ImoInstruments* pColInstr = pScore->get_instruments();
        pColInstr->remove_child_imo(pInstr);    //AWARE: this does not delete ImoInstrument object

        //trim groups if necessary, and delete any group containing only one instrument
        int numInstrs = pScore->get_num_instruments();
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        if (pGroups)
        {
            ImoObj::children_iterator itG;
            for (itG= pGroups->begin(); itG != pGroups->end(); ++itG)
            {
                ImoInstrGroup* pGroup = static_cast<ImoInstrGroup*>(*itG);
                int iFirst = pGroup->get_index_to_first_instrument();
                if (iFirst + pGroup->get_num_instruments() > numInstrs)
                    pGroup->set_range(iFirst, numInstrs-1);

                if (pGroup->get_num_instruments() < 2)
                {
                    pGroups->remove_child_imo(pGroup);
                    delete pGroup;
                }
            }
        }

        //finally, delete the instrument object
        delete pInstr;
    }

    //-----------------------------------------------------------------------------------
    static void move_up_instrument(ImoScore* pScore, ImoInstrument* pInstr)
    {
        ImoInstrument* pInstrPrev = static_cast<ImoInstrument*>(pInstr->get_prev_sibling());
        if (pInstrPrev == nullptr)
            return;

        //remove the instrument from the score and insert it before the previous instr.
        ImoInstruments* pColInstr = pScore->get_instruments();
        pColInstr->remove_child_imo(pInstr);    //AWARE: this does not delete ImoInstrument object
        pColInstr->insert(pInstrPrev, pInstr);
    }

    //-----------------------------------------------------------------------------------
    static void move_down_instrument(ImoScore* pScore, ImoInstrument* pInstr)
    {
        ImoInstrument* pInstrNext = static_cast<ImoInstrument*>(pInstr->get_next_sibling());
        if (pInstrNext == nullptr)
            return;

        //remove the instrument from the score and insert it after the next one
        ImoInstruments* pColInstr = pScore->get_instruments();
        pColInstr->remove_child_imo(pInstr);    //AWARE: this does not delete ImoInstrument object
        ImoInstrument* pAt = static_cast<ImoInstrument*>(pInstrNext->get_next_sibling());
        if (pAt)
            pColInstr->insert(pAt, pInstr);
        else
            pColInstr->append_child(pInstr);
    }

};

///@endcond

/// @name Instruments management. Access to information
//@{

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Returns the requested instrument.
    @param iInstr    Is the index to the requested instrument (0 ... num_instruments - 1).
*/
std::unique_ptr<IInstrument> IScore::get_instrument_at(int iInstr) const
{
    ensure_validity();
    ImoInstrument* pInstr = const_cast<ImoScore*>(pimpl())->get_instrument(iInstr);
    return unique_ptr<IInstrument>(new IInstrument(pInstr, m_pDoc, m_imVersion));
}

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Returns the number of instruments that this score contains.
*/
int IScore::get_num_instruments() const
{
    ensure_validity();
    return const_cast<ImoScore*>(pimpl())->get_num_instruments();
}


//@}    //Instruments management. Access to information


/// @name Instruments management. Add/delete instruments
//@{

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Appends a new empty instrument to the score, instantiated with default values:
    - One empty staff, staff size 7.2 mm (rastral size between 2 (7.4mm) and 3 (7.0mm)).
    - No name, no abbreviation
    - Not included in any existing instruments group.
    - Has default MIDI info (see IMidiInfo for default values)

    Returns the created instrument.
*/
std::unique_ptr<IInstrument> IScore::append_new_instrument()
{
    ensure_validity();
    ImoInstrument* pInstr = pimpl()->add_instrument();

    PartIdAssigner assigner;
    assigner.assign_parts_id(pimpl());

    return unique_ptr<IInstrument>(new IInstrument(pInstr, m_pDoc, m_imVersion));
}

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Removes the specified instrument form the score and deletes it.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a delete instrument operation. A remove operation
    will not affect existing groups unless the group contained the last instrument in
    the score, as that instrument position will no longer exist after deleting an
    instrument. In that case, the group will have one less instrument or will be deleted
    if only containing one instrument.

    @param instrId    The ID of the instrument to delete.
*/
void IScore::delete_instrument(ImoId instrId)
{
    ensure_validity();
    ImoInstrument* pInstr = static_cast<ImoInstrument*>(m_pDoc->get_pointer_to_imo(instrId));
    IScore::Private::delete_instrument(pimpl(), pInstr);
}

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Removes the specified instrument form the score and deletes it.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a delete instrument operation. A remove operation
    will not affect existing groups unless the group contained the last instrument in
    the score, as that instrument position will no longer exist after deleting an
    instrument. In that case, the group will have one less instrument or will be deleted
    if only containing one instrument.

    @param instr    A reference to the instrument to delete.
*/
void IScore::delete_instrument(IInstrument& instr)
{
    ensure_validity();
    ImoInstrument* pInstr = instr.pimpl();
    IScore::Private::delete_instrument(pimpl(), pInstr);
}


//@}    //Instruments management. Add/delete instruments


/// @name Instruments management. Reordering instruments
//@{

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Changes the order of the instruments in the score by moving the referenced
    instrument before the one immediately above it.

    @param instrId    The ID of the instrument to move up.

    Trying to move up the first instrument is a valid operation but nothing
    will be changed.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a move operation.

    @sa: move_down_instrument()
*/
void IScore::move_up_instrument(ImoId instrId)
{
    ensure_validity();
    ImoInstrument* pInstr = static_cast<ImoInstrument*>(m_pDoc->get_pointer_to_imo(instrId));
    IScore::Private::move_up_instrument(pimpl(), pInstr);
}

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Changes the order of the instruments in the score by moving the referenced
    instrument before the one immediately above it.

    @param instr    A reference to the instrument to move up.

    Trying to move up the first instrument is a valid operation but nothing
    will be changed.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a move operation.

    @sa: move_down_instrument()
*/
void IScore::move_up_instrument(IInstrument& instr)
{
    ensure_validity();
    ImoInstrument* pInstr = instr.pimpl();
    IScore::Private::move_up_instrument(pimpl(), pInstr);
}

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Changes the order of the instruments in the score by moving the referenced
    instrument after the one immediately below it.

    @param instrId    The ID of the instrument to move down.

    Trying to move down the last instrument is a valid operation but nothing
    will be changed.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a move operation.

    @sa: move_up_instrument()
*/
void IScore::move_down_instrument(ImoId instrId)
{
    ensure_validity();
    ImoInstrument* pInstr = static_cast<ImoInstrument*>(m_pDoc->get_pointer_to_imo(instrId));
    IScore::Private::move_down_instrument(pimpl(), pInstr);
}

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Changes the order of the instruments in the score by moving the referenced
    instrument after the one immediately below it.

    @param instr    A reference to the instrument to move down.

    Trying to move down the last instrument is a valid operation but nothing
    will be changed.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a move operation.

    @sa: move_up_instrument()
*/
void IScore::move_down_instrument(IInstrument& instr)
{
    ensure_validity();
    ImoInstrument* pInstr = instr.pimpl();
    IScore::Private::move_down_instrument(pimpl(), pInstr);
}


//@}    //Instruments management. Reordering instruments


/// @name Groups management. Access to information
//@{

//---------------------------------------------------------------------------------------
/** @memberof IScore

*/
int IScore::get_num_instruments_groups() const
{
    ensure_validity();
    ImoInstrGroups* pGroups = const_cast<ImoScore*>(pimpl())->get_instrument_groups();
    if (!pGroups)
        return 0;

    return pGroups->get_num_children();
}

//---------------------------------------------------------------------------------------
/** @memberof IScore

;   //0..n-1
*/
std::unique_ptr<IInstrGroup> IScore::get_instruments_group_at(int iGroup) const
{
    ensure_validity();
    ImoInstrGroups* pGroups = const_cast<ImoScore*>(pimpl())->get_instrument_groups();
    if (!pGroups)
        return unique_ptr<IInstrGroup>();

    ImoObj::children_iterator itG;
    int i = 0;
    for (itG= pGroups->begin(); itG != pGroups->end() && i < iGroup; ++itG, ++i);
    if (i == iGroup && itG != pGroups->end())
    {
        ImoInstrGroup* pGroup = static_cast<ImoInstrGroup*>(*itG);
        return unique_ptr<IInstrGroup>(new IInstrGroup(pGroup, m_pDoc, m_imVersion));
    }
    else
        return unique_ptr<IInstrGroup>();
}


//@}    //Groups management. Access to information


/// @name Groups management. Create/remove groups
//@{


//---------------------------------------------------------------------------------------
/** @memberof IScore
    Creates a group whose first instrument will be instrument at position
    <i>iFirstInstr</i> and the last in the group will be instrument at position
    <i>iLastInstr</i>.

    @param iFirstInstr    Position (0..n-1) of first instrument to include in the group.

    @param iLastInstr     Position (0..n-1) of last instrument to include in the group.
        Notice that <i>iFirstInstr</i> must be lower than <i>iLastInstr</i>. Otherwise
        this method will do nothing and will return an invalid IInstrGroup.

    The created group will not have any symbol (k_group_symbol_none),
    the instruments will have their barlines joined (k_joined_barlines), and the
    group will have neither name nor abbreviation. After the group is created,
    you can change all these default settings.
*/
std::unique_ptr<IInstrGroup> IScore::create_instruments_group(int iFirstInstr, int iLastInstr)
{
    ensure_validity();
    int maxInstr = pimpl()->get_num_instruments();
    if ((iFirstInstr >= 0 && iFirstInstr < maxInstr)
        && (iLastInstr > iFirstInstr && iLastInstr < maxInstr) )
    {
        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(
                                        ImFactory::inject(k_imo_instr_group, m_pDoc));

        pGrp->set_owner_score(pimpl());
        pGrp->set_range(iFirstInstr, iLastInstr);
        pimpl()->add_instruments_group(pGrp);
        return unique_ptr<IInstrGroup>(new IInstrGroup(pGrp, m_pDoc, m_imVersion));
    }
    else
    {
        return unique_ptr<IInstrGroup>();
    }
}

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Deletes the instruments group at position <i>iGroup</i>.
    Returns @FALSE if error (invalid value for param iGroup).

    @param iGroup    Position (0..num.groups-1) of group to delete. Please, notice
        that groups are ordered as they are created: first created one has index 0.
*/
bool IScore::delete_instruments_group_at(int iGroup)
{
    if (iGroup < 0)
        return false;    //no success: invalid index, too low

    ensure_validity();
    ImoInstrGroups* pGroups = pimpl()->get_instrument_groups();
    if (!pGroups)
        return false;    //no success: no groups

    //find and remove group
    ImoObj::children_iterator itG;
    int i = 0;
    for (itG= pGroups->begin(); itG != pGroups->end() && i < iGroup; ++itG, ++i);
    if (i == iGroup && itG != pGroups->end())
    {
        pGroups->remove_child_imo(*itG);
        delete *itG;
        return true;    //success
    }
    return false;    //no success: invalid index, too high
}

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Deletes the instruments group <i>group</i>.
    Returns @FALSE if any error.

    @param group    The group to delete.
*/
bool IScore::delete_instruments_group(const IInstrGroup& group)
{
    ensure_validity();
    ImoInstrGroups* pGroups = pimpl()->get_instrument_groups();
    if (!pGroups)
        return false;    //no success: no groups

    ImoInstrGroup* pGrp = group.get_internal_object();
    pGroups->remove_child_imo(pGrp);
    delete pGrp;
    return true;    //success
}

    void delete_all_instruments_groups();

//---------------------------------------------------------------------------------------
/** @memberof IScore
    Deletes all instruments groups defined in the score, if any.
*/
void IScore::delete_all_instruments_groups()
{
    ensure_validity();
    ImoInstrGroups* pGroups = pimpl()->get_instrument_groups();
    if (!pGroups)
        return;

    pimpl()->remove_child_imo(pGroups);
    delete pGroups;
}

//@}    //Groups management. Create/remove groups


//---------------------------------------------------------------------------------------
/** @memberof IScore
    When you finish modifying the content of an score it is necessary to inform
    lomse for updating all internal structures associated to the score.
    For this it is mandatory to invoke this method. Alternatively, you can invoke
    IDocument::end_of_changes(), that will invoke this method on all scores.
*/
void IScore::end_of_changes()
{
    ensure_validity();
    pimpl()->end_of_changes();
}



//=======================================================================================
/** @class ISoundInfo
    @extends INode
    %ISoundInfo class contains and manages the information for one sound, such as its
    MIDI values. It always contains a IMidiInfo object.
    An IInstrument always have at least one sound but can have more. For each sound there
    is a ISoundInfo object and its associated IMidiInfo object.

    MusicXML files and other can contain additional information about the sound for an
    instrument, such as performance data (a solo instrument or an ensemble?), the
    virtual instrument used for the sound, or the play technique to use for all notes
    played in the associated instrument. This information is stored in the ISoundInfo
    object when the score is imported from MusicXML files. But this information is not
    yet used in lomse sound API.
*/
LOMSE_IMPLEMENT_IM_API_CLASS(ISoundInfo, ImoSoundInfo, INode)

//---------------------------------------------------------------------------------------
/** @memberof ISoundInfo
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoSoundInfo* ISoundInfo::get_internal_object() const
{
    return const_cast<ImoSoundInfo*>(pimpl());
}

//---------------------------------------------------------------------------------------
/** @memberof ISoundInfo
    Provides access to the MIDI information for this instrument. MIDI info always exists.
    By default, when no MIDI information is provided in the source file or when
    programatically building a score, MIDI information is initialized as follows:

        device name = ""
        program name = ""
        bank = 1
        port = 0        port 0 is invalid. Means "not initialized"
        channel = 0     channel 0 is invalid. Means "not initialized"
        program = 1
        unpitched = 0
        volume = 1.0    maximum volume
        pan = 0         in front, centered
        elevation = 0   at listener head level

    When MIDI information has been specified when building the score, notice that
    'port', 'channel' or both could have not been specified and, thus, any of them can
    still in "not initialized". In these cases, port and channel are automatically
    assigned right values when the score is finished, that is, when invoking
    IScore::end_of_changes() method. The algorithm, in method
    MidiAssigner::assign_port_and_channel(), ensures that
    each instrument is assigned a unique combination (port, channel) for all instruments
    with port or channel containig the "not initialized" value.
*/
std::unique_ptr<IMidiInfo> ISoundInfo::get_midi_info() const
{
    ensure_validity();
    return unique_ptr<IMidiInfo>(
                new IMidiInfo(const_cast<ImoSoundInfo*>(pimpl())->get_midi_info(),
                              m_pDoc, m_imVersion) );
}




////=======================================================================================
// /* * @class IXxxxx
//    @extends INode
//    %IXxxxx is the API object for interacting with the internal model...
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(IXxxxx, ImoXxxxx, IElement)
//
////---------------------------------------------------------------------------------------
// /* * @memberof IXxxxx
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoXxxxx* IXxxxx::get_internal_object() const
//{
//    return pimpl();
//}
//
////exclude from API documentation all private members
/////@cond INTERNALS
//
////struct IXxxxx::Private
////{
////    static void delete_instrument(ImoScore* pScore, ImoInstrument* pInstr)
////    {
////    }
////};
//
/////@endcond
//
////---------------------------------------------------------------------------------------
// /* * @memberof IXxxxx
//*/
//int IXxxxx::get_num_instruments() const
//{
//    ensure_validity();
//    return pimpl()->get_num_instruments();
//}


}  //namespace lomse


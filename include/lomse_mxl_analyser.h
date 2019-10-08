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

#ifndef __LOMSE_MXL_ANALYSER_H__
#define __LOMSE_MXL_ANALYSER_H__

#include <list>
#include "lomse_xml_parser.h"
#include "lomse_analyser.h"
#include "lomse_ldp_elements.h"
#include "lomse_relation_builder.h"
#include "lomse_internal_model.h"       //required to define MxlBeamsBuilder, MxlSlursBuilder
#include "lomse_im_note.h"              //required for enum EAccidentals

using namespace std;

namespace lomse
{

//forward declarations
class LibraryScope;
class MxlElementAnalyser;
class LdpFactory;
class MxlAnalyser;
class ImoObj;
class ImoNote;
class ImoRest;


//---------------------------------------------------------------------------------------
// helper class to save start of tie info, match them and build the tie
class MxlTiesBuilder : public RelationBuilder<ImoTieDto, MxlAnalyser>
{
public:
    MxlTiesBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoTieDto, MxlAnalyser>(reporter, pAnalyser, "tie", "Tie") {}
    virtual ~MxlTiesBuilder() {}

    void add_relation_to_staffobjs(ImoTieDto* pEndDto);

protected:
    bool notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote);
    void tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto);
    void error_notes_can_not_be_tied(ImoTieDto* pEndInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save beam info items, match them and build the beams
class MxlBeamsBuilder : public RelationBuilder<ImoBeamDto, MxlAnalyser>
{
public:
    MxlBeamsBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoBeamDto, MxlAnalyser>(reporter, pAnalyser, "beam", "Beam") {}
    virtual ~MxlBeamsBuilder() {}

    void add_relation_to_staffobjs(ImoBeamDto* pEndInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save slur info items, match them and build the slurs
class MxlSlursBuilder : public RelationBuilder<ImoSlurDto, MxlAnalyser>
{
public:
    MxlSlursBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoSlurDto, MxlAnalyser>(reporter, pAnalyser, "slur", "Slur") {}
    virtual ~MxlSlursBuilder() {}

    void add_relation_to_staffobjs(ImoSlurDto* pEndInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save tuplet info items, match them and build the tuplets
class MxlTupletsBuilder : public RelationBuilder<ImoTupletDto, MxlAnalyser>
{
public:
    MxlTupletsBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoTupletDto, MxlAnalyser>(reporter, pAnalyser, "tuplet", "Tuplet") {}
    virtual ~MxlTupletsBuilder() {}

    void add_relation_to_staffobjs(ImoTupletDto* pEndInfo);
    inline bool is_tuplet_open() { return m_pendingItems.size() > 0; }
    void add_to_open_tuplets(ImoNoteRest* pNR);
    void get_factors_from_nested_tuplets(int* pTop, int* pBottom);

};


//---------------------------------------------------------------------------------------
// helper class to save volta bracket dto items, match them and build the volta brackets
class MxlVoltasBuilder : public RelationBuilder<ImoVoltaBracketDto, MxlAnalyser>
{
protected:
    ImoVoltaBracket* m_pFirstVB;        //ptr to 1st volta of current repetition set

public:
    MxlVoltasBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoVoltaBracketDto, MxlAnalyser>(
                reporter, pAnalyser, "volta bracket", "Volta bracket")
        , m_pFirstVB(nullptr)
    {
    }
    virtual ~MxlVoltasBuilder() {}

    void add_relation_to_staffobjs(ImoVoltaBracketDto* pEndInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save wedge dto items, match them and build the wedges
class MxlWedgesBuilder : public RelationBuilder<ImoWedgeDto, MxlAnalyser>
{
public:
    MxlWedgesBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoWedgeDto, MxlAnalyser>(
                reporter, pAnalyser, "wedge", "Wedge")
    {
    }
    virtual ~MxlWedgesBuilder() {}

    void add_relation_to_staffobjs(ImoWedgeDto* pEndInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save octave-shift dto items, match them and build the octave-shift lines
class MxlOctaveShiftBuilder : public RelationBuilder<ImoOctaveShiftDto, MxlAnalyser>
{
public:
    MxlOctaveShiftBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoOctaveShiftDto, MxlAnalyser>(
                reporter, pAnalyser, "octave-shift", "Octave-shift")
    {
    }
    virtual ~MxlOctaveShiftBuilder() {}

    void add_relation_to_staffobjs(ImoOctaveShiftDto* pEndInfo);
    void add_to_open_octave_shifts(ImoNote* pNote);
};


//---------------------------------------------------------------------------------------
// helper class to save part-list info
class PartList
{
protected:
    vector<ImoInstrument*> m_instruments;
    vector<bool> m_partAdded;
    map<string, int> m_locators;
    int m_numInstrs;
    bool m_fInstrumentsAdded;

public:
    PartList();
    ~PartList();

    int get_num_items() { return static_cast<int>(m_locators.size()); }
    void add_score_part(const string& id, ImoInstrument* pInstrument);
    ImoInstrument* get_instrument(const string& id);
    bool mark_part_as_added(const string& id);
    void add_all_instruments(ImoScore* pScore);
    void check_if_missing_parts(ostream& reporter);

    //for unit tests
    void do_not_delete_instruments_in_destructor() { m_fInstrumentsAdded = true; }

protected:
    int find_index_for(const string& id);

};

//---------------------------------------------------------------------------------------
// helper class to manage open <part-group> tags
class PartGroups
{
protected:
    map<int, ImoInstrGroup*> m_groups;

//    int m_number;
//    int m_symbol;
//    bool m_fBarline;

public:
    PartGroups();
    ~PartGroups();

//    void set_name(const string& name);
//    void set_name_display(const string& name);
//    void set_abbreviation(const string& abbrev);
//    void set_abbreviation_display(const string& abbrev);
//    void set_number(int num);
//    void set_symbol(int symbol);
//    void set_barline(bool value);

    void add_instrument_to_groups(ImoInstrument* pInstr);
    void start_group(int number, ImoInstrGroup* pGrp);
    void terminate_group(int number);
    bool group_exists(int number);
    ImoInstrGroup* get_group(int number);
    void check_if_all_groups_are_closed(ostream& reporter);

protected:
    void set_barline_layout_in_instruments(ImoInstrGroup* pGrp);

};



//---------------------------------------------------------------------------------------
//MxlAnalyser: responsible for parsing a tree of MusicXML nodes
//             and building an internal model for it.
class MxlAnalyser : public Analyser
{
protected:
    //helpers and collaborators
    ostream&            m_reporter;
    LibraryScope&       m_libraryScope;
    Document*           m_pDoc;
    XmlParser*          m_pParser;
    LdpFactory*         m_pLdpFactory;
    MxlTiesBuilder*     m_pTiesBuilder;
    MxlBeamsBuilder*    m_pBeamsBuilder;
    MxlTupletsBuilder*  m_pTupletsBuilder;
    MxlSlursBuilder*    m_pSlursBuilder;
    MxlVoltasBuilder*   m_pVoltasBuilder;
    MxlWedgesBuilder*   m_pWedgesBuilder;
    MxlOctaveShiftBuilder*  m_pOctaveShiftBuilder;
    map<string, int>    m_lyricIndex;
    vector<ImoLyric*>   m_lyrics;
    map<string, int>    m_soundIdToIdx;     //conversion sound-instrument id to index
	vector<ImoMidiInfo*> m_latestMidiInfo;  //latest MidiInfo for each soundIdx


    int             m_musicxmlVersion;
    ImoObj*         m_pNodeImo;
    map<int, ImoId> m_tieIds;
    int             m_tieNum;
    map<int, ImoId> m_slurIds;
    int             m_slurNum;
    int             m_voltaNum;
    map<int, ImoId> m_wedgeIds;
    int             m_wedgeNum;
    map<int, ImoId> m_octaveShiftIds;
    int             m_octaveShiftNum;


    //analysis input
    XmlNode* m_pTree;
    string m_fileLocator;

    // information maintained in MxlAnalyser
    ImoScore*       m_pCurScore;        //the score under construction
    ImoInstrument*  m_pCurInstrument;   //the instrument being analysed
    ImoNote*        m_pLastNote;        //last note added to the score
    ImoBarline*     m_pLastBarline;     //last barline added to current instrument
    ImoDocument*    m_pImoDoc;          //the document containing the score
    PartList        m_partList;         //the list of instruments
    PartGroups      m_partGroups;       //the list of intrument groups
    TimeUnits       m_time;             //time-position counter
    TimeUnits       m_maxTime;          //max time-position reached
    float           m_divisions;        //fractions of quarter note to use as units for 'duration' values
    string          m_curPartId;        //Part Id being analysed
    string          m_curMeasureNum;    //Num of measure being analysed
    int             m_measuresCounter;  //counter for measures in current instrument

    vector<ImoNote*> m_notes;           //last note for each staff

    //inherited values
//    int m_curStaff;
    int m_curVoice;
//    int m_nShowTupletBracket;
//    int m_nShowTupletNumber;

    //conversion from xml element name to int
    std::map<std::string, int>	m_NameToEnum;

public:
    MxlAnalyser(ostream& reporter, LibraryScope& libraryScope, Document* pDoc,
                XmlParser* parser);
    virtual ~MxlAnalyser();

    //access to results
    ImoObj* analyse_tree(XmlNode* tree, const string& locator);
    ImoObj* analyse_tree_and_get_object(XmlNode* tree);

    //analysis
    ImoObj* analyse_node(XmlNode* pNode, ImoObj* pAnchor=nullptr);
    bool analyse_node_bool(XmlNode* pNode, ImoObj* pAnchor=nullptr);
    void prepare_for_new_instrument_content();

    //part-list
    bool part_list_is_valid() { return m_partList.get_num_items() > 0; }
    void add_score_part(const string& id, ImoInstrument* pInstrument) {
        m_partList.add_score_part(id, pInstrument);
        m_partGroups.add_instrument_to_groups(pInstrument);
    }
    void add_all_instruments(ImoScore* pScore) { m_partList.add_all_instruments(pScore); }
    bool mark_part_as_added(const string& id) {
        return m_partList.mark_part_as_added(id);
    }
    void check_if_missing_parts() { m_partList.check_if_missing_parts(m_reporter); }

    //part-group
    ImoInstrGroup* start_part_group(int number);
    void terminate_part_group(int number);
    ImoInstrGroup* get_part_group(int number);
    void check_if_all_groups_are_closed();

    //global info: setters, getters and checkers
    int set_musicxml_version(const string& version);
    inline int get_musicxml_version() { return m_musicxmlVersion; }
    ImoInstrument* get_instrument(const string& id) { return m_partList.get_instrument(id); }
    float current_divisions() { return m_divisions; }
    void set_current_divisions(float value) { m_divisions = value; }
    TimeUnits duration_to_timepos(int duration);

    //timepos
    TimeUnits get_current_time() { return m_time; }
    void set_current_time(TimeUnits value)
    {
        m_time = value;
        m_maxTime = max<TimeUnits>(m_time, m_maxTime);
    }
    void shift_time(TimeUnits amount)
    {
        m_time += amount;
        m_maxTime = max<TimeUnits>(m_time, m_maxTime);
    }
    TimeUnits get_max_time() { return m_maxTime; }

//    //inherited and saved values setters & getters
//    inline void set_current_staff(int nStaff) { m_curStaff = nStaff; }
//    inline int get_current_staff() { return m_curStaff; }

    inline void set_current_voice(int nVoice) { m_curVoice = nVoice; }
    inline int get_current_voice() { return m_curVoice; }
//
//    inline void set_current_show_tuplet_bracket(int value) { m_nShowTupletBracket = value; }
//    inline int get_current_show_tuplet_bracket() { return m_nShowTupletBracket; }
//
//    inline void set_current_show_tuplet_number(int value) { m_nShowTupletNumber = value; }
//    inline int get_current_show_tuplet_number() { return m_nShowTupletNumber; }

    void save_last_note(ImoNote* pNote);
    inline ImoNote* get_last_note() { return m_pLastNote; }
    ImoNote* get_last_note_for(int iStaff);

    //last barline added to current instrument
    inline void save_last_barline(ImoBarline* pBarline) { m_pLastBarline = pBarline; }
    inline ImoBarline* get_last_barline() { return m_pLastBarline; }

    //access to score being analysed
    inline void score_analysis_begin(ImoScore* pScore) { m_pCurScore = pScore; }
    inline ImoScore* get_score_being_analysed() { return m_pCurScore; }

    //access to instrument being analysed
    void save_current_instrument(ImoInstrument* pInstr);
    inline ImoInstrument* get_current_instrument() { return m_pCurInstrument; }

    //access to document being analysed
    inline Document* get_document_being_analysed() { return m_pDoc; }
    inline const string& get_document_locator() { return m_fileLocator; }

    //access to root ImoDocument
    inline void save_root_imo_document(ImoDocument* pDoc) { m_pImoDoc = pDoc; }
    inline ImoDocument* get_root_imo_document() { return m_pImoDoc; }

    //sound-instruments and midi info management
    int get_index_for_sound(const string& id);
    int create_index_for_sound(const string& id);
    ImoMidiInfo* get_latest_midi_info_for(const string& id);
    void set_latest_midi_info_for(const string& id, ImoMidiInfo* pMidi);

    //for creating measures info
    inline int increment_measures_counter() { return ++m_measuresCounter; }
    inline void save_current_measure_num(const string& num) { m_curMeasureNum = num; }
    inline int get_measures_counter() { return m_measuresCounter; }

    //interface for building relations
    void add_relation_info(ImoObj* pDto);
    void clear_pending_relations();

    //interface for building beams
    inline bool fix_beams() { return m_libraryScope.get_musicxml_options()->fix_beams(); }

    //interface for building lyric lines
    void add_lyrics_data(ImoNote* pNote, ImoLyric* pData);

    //interface for building ties
    int new_tie_id(int numTie, FPitch fp);
    int get_tie_id(int numTie, FPitch fp);
    int get_tie_id_and_close(int numTie, FPitch fp);

    //interface for building slurs
    int new_slur_id(int numSlur);
    int get_slur_id(int numSlur);
    int get_slur_id_and_close(int numSlur);

    //interface for building volta brackets
    int new_volta_id();
    int get_volta_id();

    //interface for building wedges
    int new_wedge_id(int numWedge);
    bool wedge_id_exists(int numWedge);
    int get_wedge_id(int numWedge);
    int get_wedge_id_and_close(int numWedge);

    //interface for building octave-shift lines
    int new_octave_shift_id(int num);
    bool octave_shift_id_exists(int num);
    int get_octave_shift_id(int num);
    int get_octave_shift_id_and_close(int num);

    //interface for MxlTupletsBuilder
    inline bool is_tuplet_open() { return m_pTupletsBuilder->is_tuplet_open(); }
    inline void add_to_open_tuplets(ImoNoteRest* pNR) {
        m_pTupletsBuilder->add_to_open_tuplets(pNR);
    }
    inline void get_factors_from_nested_tuplets(int* pTop, int* pBottom)
    {
        m_pTupletsBuilder->get_factors_from_nested_tuplets(pTop, pBottom);
    }

    //interface for MxlOctaveShiftBuilder
    inline void add_to_open_octave_shifts(ImoNote* pNote) {
        m_pOctaveShiftBuilder->add_to_open_octave_shifts(pNote);
    }

    //information for reporting errors
    string get_element_info();
    inline void save_current_part_id(const string& id) { m_curPartId = id; }
    int get_line_number(XmlNode* node);


    int name_to_enum(const string& name) const;
    bool to_integer(const string& text, int* pResult);


protected:
    MxlElementAnalyser* new_analyser(const string& name, ImoObj* pAnchor=nullptr);
    void delete_relation_builders();
    void add_marging_space_for_lyrics(ImoNote* pNote, ImoLyric* pLyric);
};

//defined in WordsMxlAnalyser to simplify unit testing of the regex
extern int mxl_type_of_repetion_mark(const string& value);
//defined in EndingMxlAnalyser to simplify unit testing of the regex
extern bool mxl_is_valid_ending_number(const string& num);
//defined in EndingMxlAnalyser to simplify unit testing of the regex
extern void mxl_extract_numbers_from_ending(const string& num, vector<int>* repetitions);


}   //namespace lomse

#endif      //__LOMSE_MXL_ANALYSER_H__

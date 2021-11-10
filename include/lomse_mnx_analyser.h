//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#ifndef __LOMSE_MNX_ANALYSER_H__
#define __LOMSE_MNX_ANALYSER_H__

#include <list>
#include "lomse_xml_parser.h"
#include "lomse_analyser.h"
//#include "lomse_ldp_elements.h"
#include "lomse_relation_builder.h"
#include "lomse_internal_model.h"       //required to define MnxSlursBuilder
#include "lomse_im_note.h"              //required for enum EAccidentals

namespace lomse
{

//forward declarations
class LibraryScope;
class MnxElementAnalyser;
class LdpFactory;
class MnxAnalyser;
class ImoObj;
class ImoNote;
class ImoRest;

typedef std::vector<XmlNode> MeasuresVector;    //global measures for a part

////---------------------------------------------------------------------------------------
//// helper class to save start of tie info, match them and build the tie
//class MnxTiesBuilder : public RelationBuilder<ImoTieDto, MnxAnalyser>
//{
//public:
//    MnxTiesBuilder(ostream& reporter, MnxAnalyser* pAnalyser)
//        : RelationBuilder<ImoTieDto, MnxAnalyser>(reporter, pAnalyser, "tie", "Tie") {}
//    virtual ~MnxTiesBuilder() {}
//
//    void add_relation_to_staffobjs(ImoTieDto* pEndDto) override;
//
//protected:
//    bool notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote);
//    void tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto);
//    void error_notes_can_not_be_tied(ImoTieDto* pEndInfo);
//};


//---------------------------------------------------------------------------------------
// helper class to save slur info items, match them and build the slurs
class MnxSlursBuilder : public RelationBuilder<ImoSlurDto, MnxAnalyser>
{
public:
    MnxSlursBuilder(ostream& reporter, MnxAnalyser* pAnalyser)
        : RelationBuilder<ImoSlurDto, MnxAnalyser>(reporter, pAnalyser, "slur", "Slur") {}
    virtual ~MnxSlursBuilder() {}

    void add_relation_to_staffobjs(ImoSlurDto* pEndInfo) override;
};


//---------------------------------------------------------------------------------------
// helper class to save tuplet info items, match them and build the tuplets
class MnxTupletsBuilder : public RelationBuilder<ImoTupletDto, MnxAnalyser>
{
public:
    MnxTupletsBuilder(ostream& reporter, MnxAnalyser* pAnalyser)
        : RelationBuilder<ImoTupletDto, MnxAnalyser>(reporter, pAnalyser, "tuplet", "Tuplet") {}
    virtual ~MnxTupletsBuilder() {}

    void add_relation_to_staffobjs(ImoTupletDto* pEndInfo) override;
    inline bool is_tuplet_open() { return m_pendingItems.size() > 0; }
    void add_to_open_tuplets(ImoNoteRest* pNR);
    void get_factors_from_nested_tuplets(int* pTop, int* pBottom);

};


//---------------------------------------------------------------------------------------
// helper class to save volta bracket dto items, match them and build the volta brackets
class MnxVoltasBuilder : public RelationBuilder<ImoVoltaBracketDto, MnxAnalyser>
{
public:
    MnxVoltasBuilder(ostream& reporter, MnxAnalyser* pAnalyser)
        : RelationBuilder<ImoVoltaBracketDto, MnxAnalyser>(
                reporter, pAnalyser, "volta bracket", "Volta bracket")
    {}
    virtual ~MnxVoltasBuilder() {}

    void add_relation_to_staffobjs(ImoVoltaBracketDto* pEndInfo) override;
};


//---------------------------------------------------------------------------------------
// helper class to save part-list info
class MnxPartList
{
protected:
    std::vector<ImoInstrument*> m_instruments;
    std::vector<bool> m_partAdded;
    std::map<std::string, int> m_locators;
    int m_numInstrs;
    bool m_fInstrumentsAdded;

public:
    MnxPartList();
    ~MnxPartList();

    int get_num_items() { return static_cast<int>(m_locators.size()); }
    int add_score_part(const std::string& id, ImoInstrument* pInstrument);
    ImoInstrument* get_instrument(const std::string& id);
    bool mark_part_as_added(const std::string& id);
    void add_all_instruments(ImoScore* pScore);
    void check_if_missing_parts(ostream& reporter);

    //for unit tests
    void do_not_delete_instruments_in_destructor() { m_fInstrumentsAdded = true; }

protected:
    int find_index_for(const std::string& id);

};

//---------------------------------------------------------------------------------------
// helper class to manage open <part-group> tags
class MnxPartGroups
{
protected:
    std::map<int, ImoInstrGroup*> m_groups;

//    int m_number;
//    int m_symbol;
//    bool m_fBarline;

public:
    MnxPartGroups();
    ~MnxPartGroups();

//    void set_name(const std::string& name);
//    void set_name_display(const std::string& name);
//    void set_abbreviation(const std::string& abbrev);
//    void set_abbreviation_display(const std::string& abbrev);
//    void set_number(int num);
//    void set_symbol(int symbol);
//    void set_barline(bool value);

    void add_instrument_to_groups(int iInstr);
    void start_group(int number, ImoInstrGroup* pGrp);
    void terminate_group(int number);
    bool group_exists(int number);
    ImoInstrGroup* get_group(int number);
    void check_if_all_groups_are_closed(ostream& reporter);

};



//---------------------------------------------------------------------------------------
//MnxAnalyser: responsible for parsing a tree of MusicXML nodes
//             and building an internal model for it.
class MnxAnalyser : public Analyser
{
protected:
    //helpers and collaborators
    ostream&            m_reporter;
    LibraryScope&       m_libraryScope;
    Document*           m_pDoc;
    XmlParser*          m_pParser;
    MnxTupletsBuilder*  m_pTupletsBuilder;
    MnxSlursBuilder*    m_pSlursBuilder;
    MnxVoltasBuilder*   m_pVoltasBuilder;
    std::map<std::string, int>    m_lyricIndex;
    std::vector<ImoLyric*>   m_lyrics;
    std::vector<XmlNode>     m_beams;    //pending to process: <beams> elements
    std::vector< std::pair<XmlNode, ImoNote*> > m_ties;     //pending to process: <tied> elements

    int         m_musicxmlVersion;
    ImoObj*     m_pNodeImo;
    std::map<int, ImoId> m_tieIds;
    int         m_tieNum;
    std::map<int, ImoId> m_slurIds;
    int         m_slurNum;
    int         m_voltaNum;

    //analysis input
    XmlNode* m_pTree;
    std::string m_fileLocator;

    //analysis output
    void*           m_pResult;

    // information maintained in MnxAnalyser
    ImoScore*       m_pCurScore;        //the score under construction
    ImoInstrument*  m_pCurInstrument;   //the instrument being analysed
    ImoNote*        m_pLastNote;        //last note added to the score
    ImoBarline*     m_pLastBarline;     //last barline added to current instrument
    ImoDocument*    m_pImoDoc;          //the document containing the score
    MnxPartList     m_partList;         //the list of instruments
    MnxPartGroups   m_partGroups;       //the list of intrument groups
    TimeUnits       m_time;             //time-position counter
    TimeUnits       m_maxTime;          //max time-position reached
    float           m_divisions;        //fractions of quarter note to use as units for 'duration' values
    std::string     m_curPartId;        //Part Id being analysed
    std::string     m_curMeasureNum;    //Num of measure being analysed
    int             m_measuresCounter;  //counter for measures in current instrument
    std::string     m_directionsParent; //parent type for <directions> element

    //for dealing with <global> elements
    std::map<std::string, MeasuresVector*> m_globals;   //map: part name -> ptr to its global element
    std::vector<ImoStaffObj*> m_rightGlobals;           //global directions to be placed at right

    //for dealing with <repeat> elements
    int m_startRepeat = 0;
    int m_endRepeat = 0;

    //current values
    int m_curStaff;
    int m_curVoice;
    int m_beamLevel;
    int m_noteClass = k_imo_note_regular;    //current class: k_imo_note_regular, k_imo_note_grace, or  k_imo_note_cue

    std::map<std::string, int> m_nameToVoice;     //sequence name to voice conversion
//    int m_nShowTupletBracket;
//    int m_nShowTupletNumber;

    //conversion from xml element name to int
    std::map<std::string, int>	m_NameToEnum;


public:
    MnxAnalyser(ostream& reporter, LibraryScope& libraryScope, Document* pDoc,
                XmlParser* parser);
    virtual ~MnxAnalyser();

    //access to results
    ImoObj* analyse_tree(XmlNode* tree, const std::string& locator);
    ImoObj* analyse_tree_and_get_object(XmlNode* tree);

    //analysis
    bool analyse_node(XmlNode* pNode, ImoObj* pAnchor=nullptr);
    ImoMusicData* analyse_global_measure(XmlNode* pNode);
    void prepare_for_new_instrument_content();
    void* get_result() { return m_pResult; }

    //part-list
    bool part_list_is_valid() { return m_partList.get_num_items() > 0; }
    void add_score_part(const std::string& id, ImoInstrument* pInstrument) {
        int iInstr = m_partList.add_score_part(id, pInstrument);
        m_partGroups.add_instrument_to_groups(iInstr);
    }
    void add_all_instruments(ImoScore* pScore) { m_partList.add_all_instruments(pScore); }
    bool mark_part_as_added(const std::string& id) {
        return m_partList.mark_part_as_added(id);
    }
    void check_if_missing_parts() { m_partList.check_if_missing_parts(m_reporter); }

    //part-group
    ImoInstrGroup* start_part_group(int number);
    void terminate_part_group(int number);
    ImoInstrGroup* get_part_group(int number);
    void check_if_all_groups_are_closed();

    //global info: setters, getters and checkers
    int set_musicxml_version(const std::string& version);
    inline int get_musicxml_version() { return m_musicxmlVersion; }
    ImoInstrument* get_instrument(const std::string& id) { return m_partList.get_instrument(id); }
    void set_current_divisions(float value) { m_divisions = value; }
    TimeUnits duration_to_timepos(int duration);

    //access to document being analysed
    inline Document* get_document_being_analysed() { return m_pDoc; }
    inline const std::string& get_document_locator() { return m_fileLocator; }

    //access to score being analysed
    inline void score_analysis_begin(ImoScore* pScore) { m_pCurScore = pScore; }
    inline ImoScore* get_score_being_analysed() { return m_pCurScore; }

    //access to instrument being analysed
    inline void instrument_analysis_begin(ImoInstrument* pInstr) { m_pCurInstrument = pInstr; }
    ImoInstrument* get_instrument_being_analysed() { return m_pCurInstrument; }

    //access to root ImoDocument
    inline void save_root_imo_document(ImoDocument* pDoc) { m_pImoDoc = pDoc; }
    inline ImoDocument* get_root_imo_document() { return m_pImoDoc; }

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

    //inherited and saved values setters & getters
    inline void set_current_staff(int iStaff) { m_curStaff = iStaff; }  //iStaff=0..n-1
    inline int get_current_staff() { return m_curStaff; }  //return 0..n-1

    inline void set_current_voice(int nVoice) { m_curVoice = nVoice; }  //nVoice=1..n
    inline int get_current_voice() { return m_curVoice; }   //return 1..n
    int get_voice_for_name(const std::string& name) const;		//return 1..n

//
//    inline void set_current_show_tuplet_bracket(int value) { m_nShowTupletBracket = value; }
//    inline int get_current_show_tuplet_bracket() { return m_nShowTupletBracket; }
//
//    inline void set_current_show_tuplet_number(int value) { m_nShowTupletNumber = value; }
//    inline int get_current_show_tuplet_number() { return m_nShowTupletNumber; }

    inline void save_last_note(ImoNote* pNote) { m_pLastNote = pNote; }
    inline ImoNote* get_last_note() { return m_pLastNote; }
    inline void set_note_class(int value) { m_noteClass = value; }
    inline int get_note_class() { return m_noteClass; }

    //for creating measures info
    inline int increment_measures_counter() { return ++m_measuresCounter; }
    inline void save_current_measure_num(const std::string& num) { m_curMeasureNum = num; }
    inline int get_measures_counter() { return m_measuresCounter; }

    //last barline added to current instrument
    inline void save_last_barline(ImoBarline* pBarline) { m_pLastBarline = pBarline; }
    inline ImoBarline* get_last_barline() { return m_pLastBarline; }

    //for dealing with <global> elements
    MeasuresVector* new_global(const std::string& partList);
    void add_left_data_from_global_measure(ImoMusicData* pMD);
    void add_right_data_from_global_measure(ImoMusicData* pMD);

    //for dealing with <directions> elements
    inline const std::string& get_parent_for_directions() { return m_directionsParent; }
    inline void set_parent_for_directions(const std::string& parent) { m_directionsParent = parent; }

    //for dealing with <repeat> elements
    void save_repeat_info(const std::string& type, int times);
    void clear_repeat_info();
    void transfer_data_from_temporal_analyser(MnxAnalyser& a);
    int get_start_repeat_info();
    int get_end_repeat_info();


    //interface for building relations
    void add_relation_info(ImoObj* pDto);
    void clear_pending_relations();

    //interface for building beams
    void process_beams();
    void save_beams_element(XmlNode node);
    inline void reset_beam_level() { m_beamLevel = 0; }
    inline void increment_beam_level() { ++m_beamLevel; }
    inline void decrement_beam_level() { --m_beamLevel; }
    inline int get_beam_level() { return m_beamLevel; }

    //interface for building lyric lines
    void add_lyrics_data(ImoNote* pNote, ImoLyric* pData);

    //interface for building ties
    void process_ties();
    void save_tied_element(XmlNode node, ImoNote* pNote);
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

    //interface for MnxTupletsBuilder
    inline bool is_tuplet_open() { return m_pTupletsBuilder->is_tuplet_open(); }
    inline void add_to_open_tuplets(ImoNoteRest* pNR) {
        m_pTupletsBuilder->add_to_open_tuplets(pNR);
    }
    inline void get_factors_from_nested_tuplets(int* pTop, int* pBottom)
    {
        m_pTupletsBuilder->get_factors_from_nested_tuplets(pTop, pBottom);
    }

    //information for reporting errors
    std::string get_element_info();
    inline void save_current_part_id(const std::string& id) { m_curPartId = id; }
    int get_line_number(XmlNode* node);


    int name_to_enum(const std::string& name) const;
    bool to_integer(const std::string& text, int* pResult);

    //public utilities
    static bool pitch_to_components(const std::string& pitch, int *step, int* octave,
                                    float* alter);
    static std::vector<std::string> tokenize_spaces(const std::string& input);

    // Analysers for notational syntaxes
    static bool get_note_value(const string& value, int* noteType, int* dots);
    static bool get_note_value_quantity(const string& value, int* noteType, int* dots,
                                        int* multiplier);
    static bool note_value_quantity_to_duration(const string& value, TimeUnits* duration);
    //bool get_time_signature_value(const string& value, );
    //bool get_chromatic_pitch_value(const string& value, );
    //bool get_measure_location_value(const string& value, );
    //bool get_smufl_glyph_name(const string& value, );


protected:
    friend class MnxElementAnalyser;
    MnxElementAnalyser* new_analyser(const std::string& name, ImoObj* pAnchor=nullptr);
    void set_result(void* pValue) { m_pResult = pValue; }

    void delete_relation_builders();
    void delete_globals();
    void add_marging_space_for_lyrics(ImoNote* pNote, ImoLyric* pLyric);

    //auxiliary. for pitch analysis
    static int to_step(const char& letter);
    static int to_octave(const char& letter);
    static float to_alteration(const std::string& accidentals);

    //to deal with global directions
    void split_global_content(ImoMusicData* pGlobal, ImoMusicData* pMD);
    bool goes_at_right(ImoStaffObj* pSO);

};

////defined out of WordsMnxAnalyser for unit tests
//extern int mnx_type_of_repetion_mark(const std::string& value);


}   //namespace lomse

#endif      //__LOMSE_MNX_ANALYSER_H__

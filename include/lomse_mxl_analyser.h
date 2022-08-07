//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

    void add_relation_to_staffobjs(ImoTieDto* pEndDto) override;

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

    void add_relation_to_staffobjs(ImoBeamDto* pEndInfo) override;
};


//---------------------------------------------------------------------------------------
// helper class to save slur info items, match them and build the slurs
class MxlSlursBuilder : public RelationBuilder<ImoSlurDto, MxlAnalyser>
{
public:
    MxlSlursBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoSlurDto, MxlAnalyser>(reporter, pAnalyser, "slur", "Slur") {}
    virtual ~MxlSlursBuilder() {}

    void add_relation_to_staffobjs(ImoSlurDto* pEndInfo) override;
};


//---------------------------------------------------------------------------------------
// helper class to save tuplet info items, match them and build the tuplets
class MxlTupletsBuilder : public RelationBuilder<ImoTupletDto, MxlAnalyser>
{
public:
    MxlTupletsBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoTupletDto, MxlAnalyser>(reporter, pAnalyser, "tuplet", "Tuplet") {}
    virtual ~MxlTupletsBuilder() {}

    void add_relation_to_staffobjs(ImoTupletDto* pEndInfo) override;
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

    void add_relation_to_staffobjs(ImoVoltaBracketDto* pEndInfo) override;
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

    void add_relation_to_staffobjs(ImoWedgeDto* pEndInfo) override;
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

    void add_relation_to_staffobjs(ImoOctaveShiftDto* pEndInfo) override;
    void add_to_open_octave_shifts(ImoNoteRest* pNR);
};


//---------------------------------------------------------------------------------------
// helper class to save pedal dto items, match them and build pedals
class MxlPedalBuilder : public RelationBuilder<ImoPedalLineDto, MxlAnalyser>
{
public:
    MxlPedalBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoPedalLineDto, MxlAnalyser>(
                reporter, pAnalyser, "pedal", "Pedal")
    {
    }
    virtual ~MxlPedalBuilder() {}

    void add_relation_to_staffobjs(ImoPedalLineDto* pEndInfo) override;
};


//---------------------------------------------------------------------------------------
// helper class to save part-list info
class PartList
{
protected:
    std::vector<ImoInstrument*> m_instruments;
    std::vector<bool> m_partAdded;
    std::map<std::string, int> m_locators;
    int m_numInstrs;
    bool m_fInstrumentsAdded;

public:
    PartList();
    ~PartList();

    int get_num_items() { return static_cast<int>(m_locators.size()); }
    int add_score_part(const std::string& id, ImoInstrument* pInstrument);
    ImoInstrument* get_instrument(const std::string& id);
    bool mark_part_as_added(const std::string& id);
    void add_all_instruments(ImoScore* pScore);
    void check_if_missing_parts(ostream& reporter);

    //debug, for unit tests
    void do_not_delete_instruments_in_destructor() { m_fInstrumentsAdded = true; }

protected:
    int find_index_for(const std::string& id);

};

//---------------------------------------------------------------------------------------
// helper class to manage open <part-group> tags
class PartGroups
{
protected:
    std::map<int, ImoInstrGroup*> m_groups;

//    int m_number;
//    int m_symbol;
//    bool m_fBarline;

public:
    PartGroups();
    ~PartGroups();

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
/** MxlTimeKeeper: Helper class to manage time and keep track of current time for
    each voice. It encloses the knowledge to manage note/rests voices, duration,
    divisions, backup, forward, and anything related to time position.
*/
class MxlTimeKeeper
{
protected:
    ostream&        m_reporter;
    MxlAnalyser*    m_pAnalyser = nullptr;
    long            m_divisions = 1L;   //fractions of quarter note, to use as units for 'duration' values

    //global times ("global" means "referred to start of score")
    TimeUnits       m_time = 0.0;       //current time (global, in TimeUnits)
    TimeUnits       m_maxTime = 0.0;    //max time-position reached (global)
    TimeUnits       m_startTime = 0.0;  //global time at start of current measure, for conversions local time <--> global time

    //voices
    std::map<int, int> m_voiceStaff;    //map(voice, staff) staff for each voice

    //local times ("local" means "referred to start of current measure). All times in <divisions>
    std::map<int, long> m_voiceTime;    //current time reached by each voice. Reset at each measure
    long m_curTime = 0L;                //current time (local, in divisions)

    //other
    bool m_fResetVoiceTime = true;      //for debug: when false, vector is never reset so that values can be checked


public:
    MxlTimeKeeper(ostream& reporter, MxlAnalyser* pAnalyser);

    long current_divisions() { return m_divisions; }
    void set_current_divisions(long value) { m_divisions = value; }

    //conversion to TimeUnits
    TimeUnits duration_to_time_units(long duration);

    //current timepos and voices position
    TimeUnits get_current_time() { return m_time; }
    void forward_timepos(long amount, int voice, int staff);
    void backup_timepos(long amount);
    long get_timepos_for_voice(int voice);
    void increment_time(int voice, int staff, long amount);
    int determine_voice_and_timepos(int voice, int staff);
    void move_time_as_required_by_voice(int voice, int staff);

    //staves management
    int get_staff_for_voice(int voice);

    //reset counters
    void reset_for_new_measure();
    void full_reset();

    //debug, for unit tests
    void dbg_do_not_reset_voice_times() { m_fResetVoiceTime = false; }
    std::map<int, long>& dbg_get_voice_times() { return m_voiceTime; }

protected:
    int assign_voice();

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
    MxlOctaveShiftBuilder*          m_pOctaveShiftBuilder;
    MxlPedalBuilder*                m_pPedalBuilder;
    std::vector<ImoDynamicsMark*>   m_pendingDynamicsMarks;
    std::map<std::string, int>      m_lyricIndex;
    std::vector<ImoLyric*>          m_lyrics;
    std::map<std::string, int>      m_soundIdToIdx;     //conversion sound-instrument id to index
	std::vector<ImoMidiInfo*>       m_latestMidiInfo;  //latest MidiInfo for each soundIdx

    int                  m_musicxmlVersion;
    ImoObj*              m_pNodeImo;
    std::map<int, ImoId> m_tieIds;
    int                  m_tieNum;
    std::map<int, ImoId> m_slurIds;
    int                  m_slurNum;
    int                  m_voltaNum;
    std::map<int, ImoId> m_wedgeIds;
    int                  m_wedgeNum;
    std::map<int, ImoId> m_octaveShiftIds;
    int                  m_octaveShiftNum;
    std::map<int, ImoId> m_pedalIds;
    int                  m_pedalNum;


    //analysis input
    XmlNode* m_pTree;
    std::string m_fileLocator;

    // information maintained in MxlAnalyser
    ImoScore*       m_pCurScore;        //the score under construction
    ImoInstrument*  m_pCurInstrument;   //the instrument being analysed
    ImoNote*        m_pLastNote;        //last note added to the score
    ImoArpeggioDto* m_pArpeggioDto;     //data for the last arpeggio encountered
    ImoBarline*     m_pLastBarline;     //last barline added to current instrument
    ImoDocument*    m_pImoDoc;          //the document containing the score
    PartList        m_partList;         //the list of instruments
    PartGroups      m_partGroups;       //the list of intrument groups
    MxlTimeKeeper   m_timeKeeper;       //time position & voices manager
    std::string     m_curPartId;        //Part Id being analysed
    std::string     m_curMeasureNum;    //Num of measure being analysed
    int             m_measuresCounter;  //counter for measures in current instrument

    std::vector<ImoNote*> m_notes;          //last note for each staff

    //default fonts
    ImoFontStyleDto* m_pMusicFont = nullptr;
    ImoFontStyleDto* m_pWordFont = nullptr;
    std::map<int, ImoStyle*> m_lyricStyle;
    std::map<int, std::string> m_lyricLang;

    //<backup> and <forward> managemet
    std::list<ImoStaffObj*> m_pendingStaffObjs;     //non-timed, after a <backup> or <forward>
    bool m_fWaitingForVoice = false;                //no voice yet defined
    ImoMusicData* m_currentMD = nullptr;            //current ImoMusicData

    //temporary storage for <staff-distance>
    std::map<int, LUnits> m_defaultStaffDistance;   //global <staff-distance> defined in <defaults>
    std::map<int, LUnits> m_staffDistance;          //staff-distance for each staff, for current <part>
    bool m_fDefaultStaffDistanceForAllStaves = false;   //global <staff-distance> defined in <defaults> is
                                                        //for all staves

    //inherited values
//    int m_curStaff;
    int m_curVoice;
//    int m_nShowTupletBracket;
//    int m_nShowTupletNumber;

    //conversion from xml element name to int
    std::map<std::string, int> m_NameToEnum;

public:
    MxlAnalyser(ostream& reporter, LibraryScope& libraryScope, Document* pDoc,
                XmlParser* parser);
    virtual ~MxlAnalyser();

    //access to results
    ImoObj* analyse_tree(XmlNode* tree, const std::string& locator);
    ImoObj* analyse_tree_and_get_object(XmlNode* tree);

    //analysis
    ImoObj* analyse_node(XmlNode* pNode, ImoObj* pAnchor=nullptr);
    bool analyse_node_bool(XmlNode* pNode, ImoObj* pAnchor=nullptr);
    void prepare_for_new_instrument_content();

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

    //timepos management
    void increment_time(int voice, int staff, long amount) { m_timeKeeper.increment_time(voice, staff, amount); }
    void save_current_music_data(ImoMusicData* pMD) { m_currentMD = pMD; }
    void forward_timepos(long amount, int voice, int staff);
    void backup_timepos(long amount);
    void add_to_model(ImoObj* pImo, int type, ImoObj* pAnchor);
    void add_note_to_model(ImoNoteRest* pNR, bool fInChord, long duration, ImoObj* pAnchor);

    //current timepos and voices position
    TimeUnits get_current_time() { return m_timeKeeper.get_current_time(); }
    int determine_voice_and_timepos(int voice, int staff) { return m_timeKeeper.determine_voice_and_timepos(voice, staff); }
    long current_divisions() { return m_timeKeeper.current_divisions(); }
    void set_current_divisions(long value) { m_timeKeeper.set_current_divisions(value); }
    TimeUnits duration_to_time_units(int duration) { return m_timeKeeper.duration_to_time_units(long(duration)); }
    TimeUnits duration_to_time_units(long duration) { return m_timeKeeper.duration_to_time_units(duration); }
    void set_type_duration(ImoNoteRest* pNR, long duration);
    void insert_go_fwd(int voice, long shift);

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

    //
    void save_arpeggio_data(ImoArpeggioDto* pArpeggioDto);
    ImoArpeggioDto* get_arpeggio_data() { return m_pArpeggioDto; }
    void reset_arpeggio_data();

    //default fonts
        //music
    inline ImoFontStyleDto* get_music_font() { return m_pMusicFont; }
    inline void set_music_font(ImoFontStyleDto* pFont) {
        delete m_pMusicFont;
        m_pMusicFont = pFont;
    }
        //words
    inline ImoFontStyleDto* get_word_font() { return m_pWordFont; }
    inline void set_word_font(ImoFontStyleDto* pFont) {
        delete m_pWordFont;
        m_pWordFont = pFont;
    }
        //lyric lines
    ImoStyle* get_lyric_style(int number);
    void set_lyric_style(int number, ImoStyle* pStyle);
    std::string get_lyric_language(int number);
    void set_lyric_language(int number, const std::string& lang);

    //last barline added to current instrument
    inline void save_last_barline(ImoBarline* pBarline) { m_pLastBarline = pBarline; }
    inline ImoBarline* get_last_barline() { return m_pLastBarline; }

    //access to score being analysed
    inline void score_analysis_begin(ImoScore* pScore) { m_pCurScore = pScore; }
    inline ImoScore* get_score_being_analysed() { return m_pCurScore; }

    //access to instrument being analysed
    void save_current_instrument(ImoInstrument* pInstr);
    inline ImoInstrument* get_current_instrument() { return m_pCurInstrument; }

    //access to defualt staves spacing, for current instrument being analysed
    void save_default_staff_distance(int iStaff, LUnits distance);
    LUnits get_default_staff_distance(int iStaff);
    bool default_staff_distance_is_imported(int iStaff);
    void set_default_staff_distance_is_for_all_staves() { m_fDefaultStaffDistanceForAllStaves = true; }
    void save_staff_distance(int iStaff, LUnits distance);
    LUnits get_staff_distance(int iStaff);
    bool staff_distance_is_imported(int iStaff);
    void clear_staff_distances();


    //access to document being analysed
    inline Document* get_document_being_analysed() { return m_pDoc; }
    inline const std::string& get_document_locator() { return m_fileLocator; }

    //access to root ImoDocument
    inline void save_root_imo_document(ImoDocument* pDoc) { m_pImoDoc = pDoc; }
    inline ImoDocument* get_root_imo_document() { return m_pImoDoc; }

    //sound-instruments and midi info management
    int get_index_for_sound(const std::string& id);
    int create_index_for_sound(const std::string& id);
    ImoMidiInfo* get_latest_midi_info_for(const std::string& id);
    void set_latest_midi_info_for(const std::string& id, ImoMidiInfo* pMidi);

    //for creating measures info
    inline int increment_measures_counter() { return ++m_measuresCounter; }
    inline void save_current_measure_num(const std::string& num) {
        m_curMeasureNum = num;
        m_timeKeeper.reset_for_new_measure();
    }
    inline int get_measures_counter() { return m_measuresCounter; }

    //interface for building relations
    void add_relation_info(ImoObj* pDto);
    void clear_pending_relations();

    //interface for building beams
    inline bool fix_beams() { return m_libraryScope.get_musicxml_options()->fix_beams(); }

    //interface for building dynamics marks
    void add_pending_dynamics_mark(ImoDynamicsMark* pObj) { m_pendingDynamicsMarks.push_back(pObj); }
    void attach_pending_dynamics_marks(ImoNoteRest* pNR);

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

    //interface for building pedals
    int new_pedal_id(int num);
    bool pedal_id_exists(int num);
    int get_pedal_id(int num);
    int get_pedal_id_and_close(int num);

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
    inline void add_to_open_octave_shifts(ImoNoteRest* pNR) {
        m_pOctaveShiftBuilder->add_to_open_octave_shifts(pNR);
    }

    //information for reporting errors
    std::string get_element_info();
    inline void save_current_part_id(const std::string& id) { m_curPartId = id; }
    int get_line_number(XmlNode* node);


    int name_to_enum(const std::string& name) const;
    bool to_integer(const std::string& text, int* pResult);

    //debug, for unit tests
    void dbg_do_not_reset_voice_times() { m_timeKeeper.dbg_do_not_reset_voice_times(); }

protected:
    MxlElementAnalyser* new_analyser(const std::string& name, ImoObj* pAnchor=nullptr);
    void delete_relation_builders();
    void add_marging_space_for_lyrics(ImoNote* pNote, ImoLyric* pLyric);
    void add_pending_staffobjs(int voice);
};

//defined in WordsMxlAnalyser to simplify unit testing of the regex
extern int mxl_type_of_repetion_mark(const std::string& value);
//defined in EndingMxlAnalyser to simplify unit testing of the regex
extern bool mxl_is_valid_ending_number(const std::string& num);
//defined in EndingMxlAnalyser to simplify unit testing of the regex
extern void mxl_extract_numbers_from_ending(const std::string& num, std::vector<int>* repetitions);


}   //namespace lomse

#endif      //__LOMSE_MXL_ANALYSER_H__

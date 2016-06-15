//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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
class InternalModel;
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

    void add_relation_to_notes_rests(ImoTieDto* pEndDto);

protected:
    bool notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote);
    void tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto);
    void error_notes_can_not_be_tied(ImoTieDto* pEndInfo);
    void error_duplicated_tie(ImoTieDto* pExistingInfo, ImoTieDto* pNewInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save beam info items, match them and build the beams
class MxlBeamsBuilder : public RelationBuilder<ImoBeamDto, MxlAnalyser>
{
public:
    MxlBeamsBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoBeamDto, MxlAnalyser>(reporter, pAnalyser, "beam", "Beam") {}
    virtual ~MxlBeamsBuilder() {}

    void add_relation_to_notes_rests(ImoBeamDto* pEndInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save slur info items, match them and build the slurs
class MxlSlursBuilder : public RelationBuilder<ImoSlurDto, MxlAnalyser>
{
public:
    MxlSlursBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
        : RelationBuilder<ImoSlurDto, MxlAnalyser>(reporter, pAnalyser, "slur", "Slur") {}
    virtual ~MxlSlursBuilder() {}

    void add_relation_to_notes_rests(ImoSlurDto* pEndInfo);
};



////---------------------------------------------------------------------------------------
//// helper class to save beam info items, match them and build the beams
//// For old g+/g- syntax
//class OldMxlBeamsBuilder
//{
//protected:
//    ostream& m_reporter;
//    MxlAnalyser* m_pAnalyser;
//    std::list<ImoBeamDto*> m_pendingOldBeams;
//
//public:
//    OldMxlBeamsBuilder(ostream& reporter, MxlAnalyser* pAnalyser);
//    ~OldMxlBeamsBuilder();
//
//    void add_old_beam(ImoBeamDto* pInfo);
//    bool is_old_beam_open();
//    void close_old_beam(ImoBeamDto* pInfo);
//    void clear_pending_old_beams();
//
//protected:
//    void do_create_old_beam();
//
//    //errors
//    void error_no_end_old_beam(ImoBeamDto* pInfo);
//
//};
//
//
////---------------------------------------------------------------------------------------
//// helper class to save tuplet info items, match them and build the tuplets
//class MxlTupletsBuilder : public RelationBuilder<ImoTupletDto, MxlAnalyser>
//{
//public:
//    MxlTupletsBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
//        : RelationBuilder<ImoTupletDto, MxlAnalyser>(reporter, pAnalyser, "tuplet", "Tuplet") {}
//    virtual ~MxlTupletsBuilder() {}
//
//    void add_relation_to_notes_rests(ImoTupletDto* pEndInfo);
//    inline bool is_tuplet_open() { return m_pendingItems.size() > 0; }
//};

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
    ostream&        m_reporter;
    LibraryScope&   m_libraryScope;
    Document*       m_pDoc;
    XmlParser*      m_pParser;
    LdpFactory*     m_pLdpFactory;
    MxlTiesBuilder*    m_pTiesBuilder;
    MxlBeamsBuilder*   m_pBeamsBuilder;
//    MxlTupletsBuilder* m_pTupletsBuilder;
    MxlSlursBuilder*   m_pSlursBuilder;
    map<string, ImoLyrics*> m_lyrics;

    int             m_musicxmlVersion;
    ImoObj*         m_pNodeImo;
    map<int, ImoId> m_tieIds;
    int             m_tieNum;
    map<int, ImoId> m_slurIds;
    int             m_slurNum;

    //analysis input
    XmlNode* m_pTree;
    string m_fileLocator;

    // information maintained in MxlAnalyser
    ImoScore*       m_pCurScore;        //the score under construction
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
    InternalModel* analyse_tree(XmlNode* tree, const string& locator);
    ImoObj* analyse_tree_and_get_object(XmlNode* tree);

    //analysis
    ImoObj* analyse_node(XmlNode* pNode, ImoObj* pAnchor=NULL);
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

    inline void save_last_note(ImoNote* pNote) { m_pLastNote = pNote; }
    inline ImoNote* get_last_note() { return m_pLastNote; }

    //last barline added to current instrument
    inline void save_last_barline(ImoBarline* pBarline) { m_pLastBarline = pBarline; }
    inline ImoBarline* get_last_barline() { return m_pLastBarline; }

    //interface for building relations
    void add_relation_info(ImoObj* pDto);
    void clear_pending_relations();

    //interface for building lyric lines
    void add_lyrics_data(ImoNote* pNote, ImoLyricsData* pData);

    //interface for building ties
    int new_tie_id(int numTie, FPitch fp);
    int get_tie_id(int numTie, FPitch fp);
    int get_tie_id_and_close(int numTie, FPitch fp);

    //interface for building slurs
    int new_slur_id(int numSlur);
    int get_slur_id(int numSlur);
    int get_slur_id_and_close(int numSlur);

//    //interface for MxlTupletsBuilder
//    inline bool is_tuplet_open() { return m_pTupletsBuilder->is_tuplet_open(); }

//    //interface for ChordBuilder
//    void add_chord(ImoChord* pChord);

    //information for reporting errors
    string get_element_info();
    inline void save_current_part_id(const string& id) { m_curPartId = id; }
    inline void save_current_measure_num(const string& num) { m_curMeasureNum = num; }
    int get_line_number(XmlNode* node);

    //access to score being analysed
    inline void score_analysis_begin(ImoScore* pScore) { m_pCurScore = pScore; }
    inline ImoScore* get_score_being_analysed() { return m_pCurScore; }

    //access to document being analysed
    inline Document* get_document_being_analysed() { return m_pDoc; }
    inline const string& get_document_locator() { return m_fileLocator; }

    //access to root ImoDocument
    inline void save_root_imo_document(ImoDocument* pDoc) { m_pImoDoc = pDoc; }
    inline ImoDocument* get_root_imo_document() { return m_pImoDoc; }

//    //static methods for general use
    static int xml_data_to_clef_type(const string& sign, int line, int octaveChange);
//    static bool ldp_pitch_to_components(const string& pitch, int *step, int* octave,
//                                        EAccidentals* accidentals);


    int name_to_enum(const string& name) const;
    bool to_integer(const string& text, int* pResult);


protected:
    MxlElementAnalyser* new_analyser(const string& name, ImoObj* pAnchor=NULL);
    void delete_relation_builders();

//    //auxiliary. for ldp notes analysis
//    static int to_step(const char& letter);
//    static int to_octave(const char& letter);
//    static EAccidentals to_accidentals(const std::string& accidentals);
};


////---------------------------------------------------------------------------------------
////Helper, to determine beam types automatically
//class MxlAutoBeamer
//{
//protected:
//    ImoBeam* m_pBeam;
//
//public:
//    MxlAutoBeamer(ImoBeam* pBeam) : m_pBeam(pBeam) {}
//    ~MxlAutoBeamer() {}
//
//    void do_autobeam();
//
//protected:
//
//
//    int get_beaming_level(ImoNote* pNote);
//    void extract_notes();
//    void determine_maximum_beam_level_for_current_triad();
//    void process_notes();
//    void compute_beam_types_for_current_note();
//    void get_triad(int iNote);
//    void compute_beam_type_for_current_note_at_level(int level);
//
//    //notes in the beam, after removing rests
//    std::vector<ImoNote*> m_notes;
//
//    //notes will be processed in triads. The triad is the current
//    //note being processed and the previous and next ones
//    enum ENotePos { k_first_note=0, k_middle_note, k_last_note, };
//    ENotePos m_curNotePos;
//    ImoNote* m_pPrevNote;
//    ImoNote* m_pCurNote;
//    ImoNote* m_pNextNote;
//
//    //maximum beam level for each triad note
//    int m_nLevelPrev;
//    int m_nLevelCur;
//    int m_nLevelNext;
//
//};


}   //namespace lomse

#endif      //__LOMSE_MXL_ANALYSER_H__

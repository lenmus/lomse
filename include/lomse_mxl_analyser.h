//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2014-2014 Cecilio Salmeron. All rights reserved.
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


////---------------------------------------------------------------------------------------
//// helper class to save start of tie info, match them and build the tie
//class OldMxlTiesBuilder
//{
//protected:
//    ostream& m_reporter;
//    MxlAnalyser* m_pAnalyser;
//    ImoNote* m_pStartNoteTieOld;     //tie: old syntax
//    XmlNode* m_pOldTieParam;
//
//public:
//    OldMxlTiesBuilder(ostream& reporter, MxlAnalyser* pAnalyser);
//    ~OldMxlTiesBuilder() {}
//
//    void start_old_tie(ImoNote* pNote, XmlNode* pOldTie);
//    void create_tie_if_old_syntax_tie_pending(ImoNote* pNote);
//
//protected:
//    bool notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote);
//    void tie_notes(ImoNote* pStartNote, ImoNote* pEndNote);
//    void error_notes_can_not_be_tied(ImoTieDto* pEndInfo);
//    void error_invalid_tie_old_syntax(int line);
//};
//
//
////---------------------------------------------------------------------------------------
//// helper class to save start of tie info, match them and build the tie
//class MxlTiesBuilder : public RelationBuilder<ImoTieDto, MxlAnalyser>
//{
//public:
//    MxlTiesBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
//        : RelationBuilder<ImoTieDto, MxlAnalyser>(reporter, pAnalyser, "tie", "Tie") {}
//    virtual ~MxlTiesBuilder() {}
//
//    void add_relation_to_notes_rests(ImoTieDto* pEndDto);
//
//protected:
//    bool notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote);
//    void tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto);
//    void error_notes_can_not_be_tied(ImoTieDto* pEndInfo);
//    void error_duplicated_tie(ImoTieDto* pExistingInfo, ImoTieDto* pNewInfo);
//};
//
//
////---------------------------------------------------------------------------------------
//// helper class to save beam info items, match them and build the beams
//class MxlBeamsBuilder : public RelationBuilder<ImoBeamDto, MxlAnalyser>
//{
//public:
//    MxlBeamsBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
//        : RelationBuilder<ImoBeamDto, MxlAnalyser>(reporter, pAnalyser, "beam", "Beam") {}
//    virtual ~MxlBeamsBuilder() {}
//
//    void add_relation_to_notes_rests(ImoBeamDto* pEndInfo);
//};
//
//
////---------------------------------------------------------------------------------------
//// helper class to save slur info items, match them and build the slurs
//class MxlSlursBuilder : public RelationBuilder<ImoSlurDto, MxlAnalyser>
//{
//public:
//    MxlSlursBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
//        : RelationBuilder<ImoSlurDto, MxlAnalyser>(reporter, pAnalyser, "slur", "Slur") {}
//    virtual ~MxlSlursBuilder() {}
//
//    void add_relation_to_notes_rests(ImoSlurDto* pEndInfo);
//};
//
//
//
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
//MxlAnalyser: responsible for phase II of LDP language compiler (syntax validation
//and semantic analysis). The result of the analysis is a tree of ImoObj objects.
class MxlAnalyser : public Analyser
{
protected:
    //helpers and collaborators
    ostream&        m_reporter;
    LibraryScope&   m_libraryScope;
    Document*       m_pDoc;
    XmlParser*      m_pParser;
    LdpFactory*     m_pLdpFactory;
//    MxlTiesBuilder*    m_pTiesBuilder;
//    OldMxlTiesBuilder* m_pOldTiesBuilder;
//    MxlBeamsBuilder*   m_pBeamsBuilder;
//    OldMxlBeamsBuilder* m_pOldBeamsBuilder;
//    MxlTupletsBuilder* m_pTupletsBuilder;
//    MxlSlursBuilder*   m_pSlursBuilder;
    int             m_musicxmlVersion;
    ImoObj*         m_pNodeImo;

    //analysis input
    XmlNode* m_pTree;
    string m_fileLocator;

    // information maintained in MxlAnalyser
    ImoScore*       m_pCurScore;        //the score under construction
    ImoNote*        m_pLastNote;        //last note added to the score
//    ImoScore*       m_pLastScore;
    ImoDocument*    m_pImoDoc;          //the document containing the score
    PartList        m_partList;         //the list of instruments and their grouping
    TimeUnits   m_time;             //time-position counter
    float       m_divisions;        //fractions of quarter note to use as units for 'duration' values

//    //inherited values
//    int m_curStaff;
//    int m_curVoice;
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
    //void analyse_node(LdpTree::iterator itNode);
    ImoObj* analyse_node(XmlNode* pNode, ImoObj* pAnchor=NULL);
    void prepare_for_new_instrument_content();
        //m_pAnalyser->clear_pending_relations();

    //global info: setters, getters and checkers
    int set_musicxml_version(const string& version);
    inline int get_musicxml_version() { return m_musicxmlVersion; }
    void add_all_instruments(ImoScore* pScore) { m_partList.add_all_instruments(pScore); }
    bool part_list_is_valid() { return m_partList.get_num_items() > 0; }
    void add_score_part(const string& id, ImoInstrument* pInstrument)
    {
            m_partList.add_score_part(id, pInstrument);
    }
    ImoInstrument* get_instrument(const string& id) { return m_partList.get_instrument(id); }
    bool mark_part_as_added(const string& id) {
        return m_partList.mark_part_as_added(id);
    }
    void check_if_missing_parts() { m_partList.check_if_missing_parts(m_reporter); }
    void shift_time(float amount) { m_time += amount; }
    TimeUnits get_current_time() { return m_time; }
    void set_current_time(float value) { m_time = value; }
    float current_divisions() { return m_divisions; }
    void set_current_divisions(float value) { m_divisions = value; }
    TimeUnits duration_to_timepos(int duration);

//    //inherited and saved values setters & getters
//    inline void set_current_staff(int nStaff) { m_curStaff = nStaff; }
//    inline int get_current_staff() { return m_curStaff; }
//
//    inline void set_current_voice(int nVoice) { m_curVoice = nVoice; }
//    inline int get_current_voice() { return m_curVoice; }
//
//    inline void set_current_show_tuplet_bracket(int value) { m_nShowTupletBracket = value; }
//    inline int get_current_show_tuplet_bracket() { return m_nShowTupletBracket; }
//
//    inline void set_current_show_tuplet_number(int value) { m_nShowTupletNumber = value; }
//    inline int get_current_show_tuplet_number() { return m_nShowTupletNumber; }

    inline void save_last_note(ImoNote* pNote) { m_pLastNote = pNote; }
    inline ImoNote* get_last_note() { return m_pLastNote; }

//    //interface for building relations
//    void add_relation_info(ImoObj* pDto);
//    void clear_pending_relations();
//
//    //interface for building ties: old 'l' syntax
//    inline void start_old_tie(ImoNote* pNote, XmlNode* pOldTie) {
//        m_pOldTiesBuilder->start_old_tie(pNote, pOldTie);
//    }
//    inline void create_tie_if_old_syntax_tie_pending(ImoNote* pNote) {
//        m_pOldTiesBuilder->create_tie_if_old_syntax_tie_pending(pNote);
//    }
//
//    //interface for building beams: old 'g+/g-' syntax
//    inline void add_old_beam(ImoBeamDto* pInfo) {
//        m_pOldBeamsBuilder->add_old_beam(pInfo);
//    }
//    inline bool is_old_beam_open() { return m_pOldBeamsBuilder->is_old_beam_open(); }
//    inline void close_old_beam(ImoBeamDto* pInfo) {
//        m_pOldBeamsBuilder->close_old_beam(pInfo);
//    }
//
//    //interface for MxlTupletsBuilder
//    inline bool is_tuplet_open() { return m_pTupletsBuilder->is_tuplet_open(); }

//    //interface for ChordBuilder
//    void add_chord(ImoChord* pChord);


    //access to score being analysed
    inline void score_analysis_begin(ImoScore* pScore) { m_pCurScore = pScore; }
//    inline void score_analysis_end() {
//        m_pLastScore = m_pCurScore;
//        m_pCurScore = NULL;
//    }
    inline ImoScore* get_score_being_analysed() { return m_pCurScore; }
//    inline ImoScore* get_last_analysed_score() { return m_pLastScore; }

    //access to document being analysed
    inline Document* get_document_being_analysed() { return m_pDoc; }
    inline const string& get_document_locator() { return m_fileLocator; }

    //access to root ImoDocument
    inline void save_root_imo_document(ImoDocument* pDoc) { m_pImoDoc = pDoc; }
    inline ImoDocument* get_root_imo_document() { return m_pImoDoc; }

//    //static methods for general use
    static int xml_data_to_clef_type(const string& sign, int line, int octaveChange);
//    static int ldp_name_to_clef_type(const string& value);
//    static bool ldp_pitch_to_components(const string& pitch, int *step, int* octave,
//                                        EAccidentals* accidentals);


//    //-----------------------------------------------------------------------------------
//    long get_node_id(XmlNode* node)
//    {
//        XmlAttribute* attr = node->first_attribute("id");
//        if (attr == NULL)
//            return -1L;
//        else
//            return std::atol( attr->value() );
//    }

    //-----------------------------------------------------------------------------------
    string get_value(XmlNode* node)
    {
        XmlNode* child = node->first_node();
        if (!child)
            return "";
        else
            return string( child->value() );
    }

    //-----------------------------------------------------------------------------------
    string get_name(XmlNode* node)
    {
        return m_pParser->get_node_name_as_string(node);
    }

    //-----------------------------------------------------------------------------------
    int get_line_number(XmlNode* node)
    {
        //TODO_X
        return 0;
    }

//    //-----------------------------------------------------------------------------------
//    ELdpElement get_type(XmlNode* node)
//    {
//        if (node->type() == rapidxml::node_data)
//            return k_string;
//
//        string name = get_name(node);
//        LdpElement* elm = m_pLdpFactory->create(name, 0);
//        ELdpElement type = elm->get_type();
//        delete elm;
//        return type;
//    }

//    //-----------------------------------------------------------------------------------
//    ImoObj* get_imo(XmlNode* node)
//    {
//        //TODO_X
//        return NULL;
//    }

//    //-----------------------------------------------------------------------------------
//    void set_imo(XmlNode* node, ImoObj* pImo)
//    {
//        m_pNodeImo = pImo;
//    }


    int name_to_enum(const string& name) const;
    bool to_integer(const string& text, int* pResult);


protected:
    MxlElementAnalyser* new_analyser(const string& name, ImoObj* pAnchor=NULL);
//    void delete_relation_builders();

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

//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//  
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE__ANALYSER_H__
#define __LOMSE__ANALYSER_H__

#include <list>
#include "lomse_ldp_elements.h"

using namespace std;

namespace lomse
{

//forward declarations
class ElementAnalyser;
class ImoObj;
class ImoStaffObj;
class ImoDocObj;
class ImoNoteRest;
class LdpFactory;
class ImoTieDto;
class ImoTie;
class ImoNote;
class ImoBezierInfo;
class LdpElement;
class Analyser;
class ImoBeamInfo;
class ImoBeam;
class ImoTupletDto;
class ImoTuplet;
class BasicModel;


// helper class to save start of tie info, match it against stop info and
// build the tie
//--------------------------------------------------------------------------------
class TiesBuilder
{
protected:
    ostream& m_reporter;
    Analyser* m_pAnalyser;
    BasicModel* m_pBasicModel;
    std::list<ImoTieDto*> m_pendingTies;
    ImoNote* m_pStartNoteTieOld;     //tie: old syntax
    LdpElement* m_pOldTieParam;

public:
    TiesBuilder(ostream& reporter, BasicModel* pBasicModel, Analyser* pAnalyser);
    ~TiesBuilder();

    void start_tie(ImoTieDto* pInfo);
    void end_tie(ImoTieDto* pInfo);
    void clear_pending_ties();
    void start_old_tie(ImoNote* pNote, LdpElement* pOldTie);
    void create_tie_if_old_syntax_tie_pending(ImoNote* pNote);

protected:
    ImoTieDto* find_matching_tie_info(ImoTieDto* pEndInfo);
    bool notes_can_be_tied(ImoTieDto* pStartInfo, ImoTieDto* pEndInfo);
    bool notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote);
    void tie_notes(ImoTieDto* pStartInfo, ImoTieDto* pEndInfo);
    void tie_notes(ImoNote* pStartNote, ImoNote* pEndNote);
    void remove_from_pending(ImoTieDto* pTieInfo);

    //errors
    void error_duplicated_tie(ImoTieDto* pExistingInfo, ImoTieDto* pNewInfo);
    void error_notes_can_not_be_tied(ImoTieDto* pEndInfo);
    void error_no_start_tie(ImoTieDto* pEndInfo);
    void error_no_end_tie(ImoTieDto* pStartInfo);
    void error_invalid_tie_old_syntax(int line);

};


// helper class to save beam info items, match them and build the beams
//--------------------------------------------------------------------------------
class BeamsBuilder
{
protected:
    ostream& m_reporter;
    Analyser* m_pAnalyser;
    BasicModel* m_pBasicModel;
    std::list<ImoBeamInfo*> m_pendingBeams;

public:
    BeamsBuilder(ostream& reporter, BasicModel* pBasicModel, Analyser* pAnalyser);
    ~BeamsBuilder();

    void add_beam_info(ImoBeamInfo* pBeamInfo);
    void clear_pending_beams();

protected:
    bool find_matching_info_items(int beamNum);
    void create_beam(ImoBeamInfo* pInfo);
    void save_beam_info(ImoBeamInfo* pNewInfo);
    void delete_consumed_info_items(ImoBeamInfo* pEndInfo);
    void do_beam_notes_rests(ImoBeamInfo* pInfo);
    void delete_beam_element(ImoBeamInfo* pInfo);

    //errors
    void error_no_matching_items(ImoBeamInfo* pInfo);
    void error_no_end_beam(ImoBeamInfo* pInfo);

    //temporary: items that match
    std::list<ImoBeamInfo*> m_matches;

};


// helper class to save beam info items, match them and build the beams
// For old g+/g- syntax
//--------------------------------------------------------------------------------
class OldBeamsBuilder
{
protected:
    ostream& m_reporter;
    Analyser* m_pAnalyser;
    BasicModel* m_pBasicModel;
    std::list<ImoBeamInfo*> m_pendingOldBeams;

public:
    OldBeamsBuilder(ostream& reporter, BasicModel* pBasicModel, Analyser* pAnalyser);
    ~OldBeamsBuilder();

    void add_old_beam(ImoBeamInfo* pInfo);
    bool is_old_beam_open();
    void close_old_beam(ImoBeamInfo* pInfo);
    void clear_pending_old_beams();

protected:
    void do_create_old_beam();

    //errors
    void error_no_end_old_beam(ImoBeamInfo* pInfo);

};


// helper class to save tuplet info items, match them and build the tuplets
//--------------------------------------------------------------------------------
class TupletsBuilder
{
protected:
    ostream& m_reporter;
    Analyser* m_pAnalyser;
    BasicModel* m_pBasicModel;
    std::list<ImoTupletDto*> m_pendingTuplets;

public:
    TupletsBuilder(ostream& reporter, BasicModel* pBasicModel, Analyser* pAnalyser);
    ~TupletsBuilder();

    void add_tuplet_info(ImoTupletDto* pTupletInfo);
    void clear_pending_tuplets();
    inline bool is_tuplet_open() { return m_pendingTuplets.size() > 0; }

protected:
    void create_tuplet(ImoTupletDto* pInfo);
    void save_tuplet_info(ImoTupletDto* pNewInfo);

    //errors
    void error_no_end_tuplet(ImoTupletDto* pInfo);

};


//Analyser: responsible for phase II of LDP language compiler (syntax validation
//and semantic analysis). The result of the analysis is a 'decorated' parse tree,
//that is, the parse tree with an ImoObj added to certain nodes.
//--------------------------------------------------------------------------------
class Analyser
{
protected:
    //helpers and collaborators
    ostream&    m_reporter;
    LdpFactory* m_pLdpFactory;
    TiesBuilder* m_pTiesBuilder;
    BeamsBuilder* m_pBeamsBuilder;
    OldBeamsBuilder* m_pOldBeamsBuilder;
    TupletsBuilder* m_pTupletsBuilder;
    BasicModel* m_pBasicModel;

    //analysis input
    LdpTree* m_pTree;

    //inherited values
    int m_curStaff;
    int m_curVoice;
    bool m_fShowTupletBracket;
    bool m_fShowTupletNumber;

public:
    Analyser(ostream& reporter, LdpFactory* pFactory);
    ~Analyser();

    //access to results
    BasicModel* analyse_tree(LdpTree* tree);

    //analysis
    void analyse_node(LdpTree::iterator itNode);
    ImoObj* analyse_node(LdpElement* pNode, ImoObj* pAnchor=NULL);

    //auxiliary
    void erase_node(LdpElement* pNode);
    void replace_node(LdpElement* pOldNode, LdpElement* pNewNode);
    void remove_tie_element(ImoTieDto* pInfo);
    void remove_old_tie_element(LdpElement* pOldTieParam);

    //inherited values setters & getters
    inline void set_current_staff(int nStaff) { m_curStaff = nStaff; }
    inline int get_current_staff() { return m_curStaff; }

    inline void set_current_voice(int nVoice) { m_curVoice = nVoice; }
    inline int get_current_voice() { return m_curVoice; }

    inline void set_current_show_tuplet_bracket(bool value) { m_fShowTupletBracket = value; }
    inline bool get_current_show_tuplet_bracket() { return m_fShowTupletBracket; }

    inline void set_current_show_tuplet_number(bool value) { m_fShowTupletNumber = value; }
    inline bool get_current_show_tuplet_number() { return m_fShowTupletNumber; }


    //interface for TiesBuilder
    inline void start_tie(ImoTieDto* pInfo) {
        m_pTiesBuilder->start_tie(pInfo);
    }
    inline void end_tie(ImoTieDto* pInfo) {
        m_pTiesBuilder->end_tie(pInfo);
    }
    inline void clear_pending_ties() { m_pTiesBuilder->clear_pending_ties(); }

    //ties: old syntax
    inline void start_old_tie(ImoNote* pNote, LdpElement* pOldTie) {
        m_pTiesBuilder->start_old_tie(pNote, pOldTie);
    }
    inline void create_tie_if_old_syntax_tie_pending(ImoNote* pNote) {
        m_pTiesBuilder->create_tie_if_old_syntax_tie_pending(pNote);
    }

    //interface for BeamsBuilder
    inline void add_beam_info(ImoBeamInfo* pInfo) {
        m_pBeamsBuilder->add_beam_info(pInfo);
    }
    inline void clear_pending_beams() {
        m_pBeamsBuilder->clear_pending_beams();
        m_pOldBeamsBuilder->clear_pending_old_beams();
    }
    inline void add_old_beam(ImoBeamInfo* pInfo) {
        m_pOldBeamsBuilder->add_old_beam(pInfo);
    }
    inline bool is_old_beam_open() { return m_pOldBeamsBuilder->is_old_beam_open(); }
    inline void close_old_beam(ImoBeamInfo* pInfo) {
        m_pOldBeamsBuilder->close_old_beam(pInfo);
    }

    //interface for TupletsBuilder
    inline void add_tuplet_info(ImoTupletDto* pInfo) {
        m_pTupletsBuilder->add_tuplet_info(pInfo);
    }
    inline void clear_pending_tuplets() {
        m_pTupletsBuilder->clear_pending_tuplets();
    }
    inline bool is_tuplet_open() { return m_pTupletsBuilder->is_tuplet_open(); }

protected:
    ElementAnalyser* new_analyser(ELdpElement type, ImoObj* pAnchor=NULL);

};


//Auxiliary, to determine beam types automatically
//----------------------------------------------------------------------------------
class AutoBeamer
{
protected:
    ImoBeam* m_pBeam;

public:
    AutoBeamer(ImoBeam* pBeam) : m_pBeam(pBeam) {}
    ~AutoBeamer() {}

    void do_autobeam();

protected:


    int get_beaming_level(ImoNote* pNote);
    void extract_notes();
    void determine_maximum_beam_level_for_current_triad();
    void process_notes();
    void compute_beam_types_for_current_note();
    void get_triad(int iNote);
    void compute_beam_type_for_current_note_at_level(int level);

    //notes in the beam, after removing rests
    std::vector<ImoNote*> m_notes;

    //notes will be processed in triads. The triad is the current
    //note being processed and the previous and next ones
    enum ENotePos { k_first_note=0, k_middle_note, k_last_note, };
    ENotePos m_curNotePos;
    ImoNote* m_pPrevNote;
    ImoNote* m_pCurNote;
    ImoNote* m_pNextNote;

    //maximum beam level for each triad note
    int m_nLevelPrev;
    int m_nLevelCur;
    int m_nLevelNext;

};


}   //namespace lomse

#endif      //__LOMSE__ANALYSER_H__

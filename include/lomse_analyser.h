//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_ANALYSER_H__
#define __LOMSE_ANALYSER_H__

#include <list>
#include "lomse_ldp_elements.h"
#include "lomse_relation_builder.h"
#include "lomse_internal_model.h"       //required to define BeamsBuilder, SlursBuilder

using namespace std;

namespace lomse
{

//forward declarations
class LibraryScope;
class ElementAnalyser;
class LdpFactory;
class LdpElement;
class Analyser;
class InternalModel;


//---------------------------------------------------------------------------------------
// helper class to save start of tie info, match them and build the tie
class OldTiesBuilder
{
protected:
    ostream& m_reporter;
    Analyser* m_pAnalyser;
    ImoNote* m_pStartNoteTieOld;     //tie: old syntax
    LdpElement* m_pOldTieParam;

public:
    OldTiesBuilder(ostream& reporter, Analyser* pAnalyser);
    ~OldTiesBuilder() {}

    void start_old_tie(ImoNote* pNote, LdpElement* pOldTie);
    void create_tie_if_old_syntax_tie_pending(ImoNote* pNote);

protected:
    bool notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote);
    void tie_notes(ImoNote* pStartNote, ImoNote* pEndNote);
    void error_notes_can_not_be_tied(ImoTieDto* pEndInfo);
    void error_invalid_tie_old_syntax(int line);
};


//---------------------------------------------------------------------------------------
// helper class to save start of tie info, match them and build the tie
class TiesBuilder : public RelationBuilder<ImoTieDto>
{
public:
    TiesBuilder(ostream& reporter, Analyser* pAnalyser)
        : RelationBuilder<ImoTieDto>(reporter, pAnalyser, "tie", "Tie") {}
    virtual ~TiesBuilder() {}

    void add_relation_to_notes_rests(ImoTieDto* pEndDto);

protected:
    bool notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote);
    void tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto);
    void error_notes_can_not_be_tied(ImoTieDto* pEndInfo);
    void error_duplicated_tie(ImoTieDto* pExistingInfo, ImoTieDto* pNewInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save beam info items, match them and build the beams
class BeamsBuilder : public RelationBuilder<ImoBeamDto>
{
public:
    BeamsBuilder(ostream& reporter, Analyser* pAnalyser)
        : RelationBuilder<ImoBeamDto>(reporter, pAnalyser, "beam", "Beam") {}
    virtual ~BeamsBuilder() {}

    void add_relation_to_notes_rests(ImoBeamDto* pEndInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save slur info items, match them and build the slurs
class SlursBuilder : public RelationBuilder<ImoSlurDto>
{
public:
    SlursBuilder(ostream& reporter, Analyser* pAnalyser)
        : RelationBuilder<ImoSlurDto>(reporter, pAnalyser, "slur", "Slur") {}
    virtual ~SlursBuilder() {}

    void add_relation_to_notes_rests(ImoSlurDto* pEndInfo);
};



//---------------------------------------------------------------------------------------
// helper class to save beam info items, match them and build the beams
// For old g+/g- syntax
class OldBeamsBuilder
{
protected:
    ostream& m_reporter;
    Analyser* m_pAnalyser;
    std::list<ImoBeamDto*> m_pendingOldBeams;

public:
    OldBeamsBuilder(ostream& reporter, Analyser* pAnalyser);
    ~OldBeamsBuilder();

    void add_old_beam(ImoBeamDto* pInfo);
    bool is_old_beam_open();
    void close_old_beam(ImoBeamDto* pInfo);
    void clear_pending_old_beams();

protected:
    void do_create_old_beam();

    //errors
    void error_no_end_old_beam(ImoBeamDto* pInfo);

};


//---------------------------------------------------------------------------------------
// helper class to save tuplet info items, match them and build the tuplets
class TupletsBuilder : public RelationBuilder<ImoTupletDto>
{
public:
    TupletsBuilder(ostream& reporter, Analyser* pAnalyser)
        : RelationBuilder<ImoTupletDto>(reporter, pAnalyser, "tuplet", "Tuplet") {}
    virtual ~TupletsBuilder() {}

    void add_relation_to_notes_rests(ImoTupletDto* pEndInfo);
    inline bool is_tuplet_open() { return m_pendingItems.size() > 0; }
};


//---------------------------------------------------------------------------------------
//Analyser: responsible for phase II of LDP language compiler (syntax validation
//and semantic analysis). The result of the analysis is a 'decorated' parse tree,
//that is, the parse tree with an ImoObj added to certain nodes.
class Analyser
{
protected:
    //helpers and collaborators
    ostream&        m_reporter;
    LibraryScope&   m_libraryScope;
    LdpFactory*     m_pLdpFactory;
    TiesBuilder*    m_pTiesBuilder;
    OldTiesBuilder* m_pOldTiesBuilder;
    BeamsBuilder*   m_pBeamsBuilder;
    OldBeamsBuilder* m_pOldBeamsBuilder;
    TupletsBuilder* m_pTupletsBuilder;
    SlursBuilder*   m_pSlursBuilder;
    ImoScore*       m_pScore;
    ImoDocument*    m_pDoc;

    //analysis input
    LdpTree* m_pTree;

    //inherited values
    int m_curStaff;
    int m_curVoice;
    int m_nShowTupletBracket;
    int m_nShowTupletNumber;

    //saved values
    ImoNote* m_pLastNote;


public:
    //Analyser(ostream& reporter, LdpFactory* pFactory);
    Analyser(ostream& reporter, LibraryScope& libraryScope);
    ~Analyser();

    //access to results
    InternalModel* analyse_tree(LdpTree* tree);
    ImoObj* analyse_tree_and_get_object(LdpTree* tree);

    //analysis
    void analyse_node(LdpTree::iterator itNode);
    ImoObj* analyse_node(LdpElement* pNode, ImoObj* pAnchor=NULL);

    //inherited and saved values setters & getters
    inline void set_current_staff(int nStaff) { m_curStaff = nStaff; }
    inline int get_current_staff() { return m_curStaff; }

    inline void set_current_voice(int nVoice) { m_curVoice = nVoice; }
    inline int get_current_voice() { return m_curVoice; }

    inline void set_current_show_tuplet_bracket(int value) { m_nShowTupletBracket = value; }
    inline int get_current_show_tuplet_bracket() { return m_nShowTupletBracket; }

    inline void set_current_show_tuplet_number(int value) { m_nShowTupletNumber = value; }
    inline int get_current_show_tuplet_number() { return m_nShowTupletNumber; }

    inline void save_last_note(ImoNote* pNote) { m_pLastNote = pNote; }
    inline ImoNote* get_last_note() { return m_pLastNote; }

    //interface for building relations
    void add_relation_info(ImoObj* pDto);
    void clear_pending_relations();

    //interface for building ties: old 'l' syntax
    inline void start_old_tie(ImoNote* pNote, LdpElement* pOldTie) {
        m_pOldTiesBuilder->start_old_tie(pNote, pOldTie);
    }
    inline void create_tie_if_old_syntax_tie_pending(ImoNote* pNote) {
        m_pOldTiesBuilder->create_tie_if_old_syntax_tie_pending(pNote);
    }

    //interface for building beams: old 'g+/g-' syntax
    inline void add_old_beam(ImoBeamDto* pInfo) {
        m_pOldBeamsBuilder->add_old_beam(pInfo);
    }
    inline bool is_old_beam_open() { return m_pOldBeamsBuilder->is_old_beam_open(); }
    inline void close_old_beam(ImoBeamDto* pInfo) {
        m_pOldBeamsBuilder->close_old_beam(pInfo);
    }

    //interface for TupletsBuilder
    inline bool is_tuplet_open() { return m_pTupletsBuilder->is_tuplet_open(); }

//    //interface for ChordBuilder
//    void add_chord(ImoChord* pChord);

    //access to score being analysed
    inline void set_score_being_analysed(ImoScore* pScore) { m_pScore = pScore; }
    inline ImoScore* get_score_being_analysed() { return m_pScore; }

    //access to document being analysed
    inline void set_document_being_analysed(ImoDocument* pDoc) { m_pDoc = pDoc; }
    inline ImoDocument* get_document_being_analysed() { return m_pDoc; }

protected:
    ElementAnalyser* new_analyser(ELdpElement type, ImoObj* pAnchor=NULL);
    void delete_relation_builders();

};


//---------------------------------------------------------------------------------------
//Helper, to determine beam types automatically
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



//---------------------------------------------------------------------------------------
// ImoObjFactory: helper to create ImoObj objects
class ImoObjFactory
{
protected:
    LibraryScope& m_libraryScope;
    ostream& m_reporter;

public:
    ImoObjFactory(LibraryScope& libraryScope, ostream& reporter=cout);
    virtual ~ImoObjFactory() {}

    ImoObj* create_object(const std::string& ldpSource);

    ImoTextItem* create_text_item(const std::string& ldpSource,
                                  ImoTextStyleInfo* pStyle);


};



}   //namespace lomse

#endif      //__LOMSE_ANALYSER_H__

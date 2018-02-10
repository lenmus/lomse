//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#ifndef __LOMSE_LDP_ANALYSER_H__
#define __LOMSE_LDP_ANALYSER_H__

#include <list>
#include "lomse_ldp_elements.h"
#include "lomse_relation_builder.h"
#include "lomse_internal_model.h"       //required to define BeamsBuilder, SlursBuilder
#include "lomse_im_note.h"              //required for enum EAccidentals
#include "lomse_analyser.h"              //required for enum EAccidentals

using namespace std;

namespace lomse
{

//forward declarations
class LibraryScope;
class ElementAnalyser;
class LdpFactory;
class LdpElement;
class LdpAnalyser;
class ImoObj;
class ImoNote;
class ImoRest;


//---------------------------------------------------------------------------------------
// helper class to save start of tie info, match them and build the tie
class OldTiesBuilder
{
protected:
    ostream& m_reporter;
    LdpAnalyser* m_pAnalyser;
    ImoNote* m_pStartNoteTieOld;     //tie: old syntax
    LdpElement* m_pOldTieParam;

public:
    OldTiesBuilder(ostream& reporter, LdpAnalyser* pAnalyser);
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
class TiesBuilder : public RelationBuilder<ImoTieDto, LdpAnalyser>
{
public:
    TiesBuilder(ostream& reporter, LdpAnalyser* pAnalyser)
        : RelationBuilder<ImoTieDto, LdpAnalyser>(reporter, pAnalyser, "tie", "Tie") {}
    virtual ~TiesBuilder() {}

    void add_relation_to_staffobjs(ImoTieDto* pEndDto);

protected:
    bool notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote);
    void tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto);
    void error_notes_can_not_be_tied(ImoTieDto* pEndInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save beam info items, match them and build the beams
class BeamsBuilder : public RelationBuilder<ImoBeamDto, LdpAnalyser>
{
public:
    BeamsBuilder(ostream& reporter, LdpAnalyser* pAnalyser)
        : RelationBuilder<ImoBeamDto, LdpAnalyser>(reporter, pAnalyser, "beam", "Beam") {}
    virtual ~BeamsBuilder() {}

    void add_relation_to_staffobjs(ImoBeamDto* pEndInfo);
};


//---------------------------------------------------------------------------------------
// helper class to save slur info items, match them and build the slurs
class SlursBuilder : public RelationBuilder<ImoSlurDto, LdpAnalyser>
{
public:
    SlursBuilder(ostream& reporter, LdpAnalyser* pAnalyser)
        : RelationBuilder<ImoSlurDto, LdpAnalyser>(reporter, pAnalyser, "slur", "Slur") {}
    virtual ~SlursBuilder() {}

    void add_relation_to_staffobjs(ImoSlurDto* pEndInfo);
};



//---------------------------------------------------------------------------------------
// helper class to save beam info items, match them and build the beams
// For old g+/g- syntax
class OldBeamsBuilder
{
protected:
    ostream& m_reporter;
    LdpAnalyser* m_pAnalyser;
    std::list<ImoBeamDto*> m_pendingOldBeams;

public:
    OldBeamsBuilder(ostream& reporter, LdpAnalyser* pAnalyser);
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
class TupletsBuilder : public RelationBuilder<ImoTupletDto, LdpAnalyser>
{
private:
    ImoTuplet* m_pTuplet;

public:
    TupletsBuilder(ostream& reporter, LdpAnalyser* pAnalyser)
        : RelationBuilder<ImoTupletDto, LdpAnalyser>(reporter, pAnalyser, "tuplet", "Tuplet")
        , m_pTuplet(nullptr)
    {
    }
    virtual ~TupletsBuilder() {}

    void add_relation_to_staffobjs(ImoTupletDto* pEndInfo);
    inline bool is_tuplet_open() { return m_pendingItems.size() > 0; }

    inline ImoTuplet* get_last_created_tuplet() { return m_pTuplet; }

    void add_to_open_tuplets(ImoNoteRest* pNR);

};


//---------------------------------------------------------------------------------------
//LdpAnalyser: responsible for phase II of LDP language compiler (syntax validation
//and semantic analysis). The result of the analysis is a tree of ImoObj objects.
class LdpAnalyser : public Analyser
{
protected:
    //helpers and collaborators
    ostream&        m_reporter;
    LibraryScope&   m_libraryScope;
    Document*       m_pDoc;
    LdpFactory*     m_pLdpFactory;
    TiesBuilder*    m_pTiesBuilder;
    OldTiesBuilder* m_pOldTiesBuilder;
    BeamsBuilder*   m_pBeamsBuilder;
    OldBeamsBuilder* m_pOldBeamsBuilder;
    TupletsBuilder* m_pTupletsBuilder;
    SlursBuilder*   m_pSlursBuilder;
    map<string, int> m_lyricIndex;
    vector<ImoLyric*>  m_lyrics;
    vector<int>     m_lyricsPlacement;
    ImoScore*       m_pCurScore;
    ImoScore*       m_pLastScore;
    ImoDocument*    m_pImoDoc;
    int             m_scoreVersion;     //100*major + minor

    //analysis input
    LdpTree* m_pTree;
    string m_fileLocator;

    //inherited values
    int m_curStaff;
    int m_curVoice;
    int m_nShowTupletBracket;
    int m_nShowTupletNumber;

    //saved values
    ImoNote* m_pLastNote;

    //other
    bool    m_fInstrIdRequired;     //Id required in instruments

    //FIX: for lyrics space
    friend class InstrumentAnalyser;
    ImoInstrument*  m_pCurInstr;    //current instrument being analysed
    LUnits  m_extraMarginSpace;     //extra margin for next instrument

public:
    LdpAnalyser(ostream& reporter, LibraryScope& libraryScope, Document* pDoc);
    ~LdpAnalyser();

    //access to results
    ImoObj* analyse_tree(LdpTree* tree, const string& locator);
    ImoObj* analyse_tree_and_get_object(LdpTree* tree);

    //analysis
    void analyse_node(LdpTree::iterator itNode);
    ImoObj* analyse_node(LdpElement* pNode, ImoObj* pAnchor=nullptr);

    //helper, for score edition
    ImoBeam* create_beam(const list<ImoNoteRest*>& notes);
    ImoTie* create_tie(ImoNote* pStart, ImoNote* pEnd);

    //inherited and saved values. setters & getters
    void reset_defaults_for_instrument();

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
    inline ImoTuplet* get_last_created_tuplet() {
         return m_pTupletsBuilder->get_last_created_tuplet();
    }
    inline void add_to_open_tuplets(ImoNoteRest* pNR) {
        m_pTupletsBuilder->add_to_open_tuplets(pNR);
    }

    //interface for building lyric lines
    void add_lyric(ImoNote* pNote, ImoLyric* pL);
    void set_lyrics_placement(int line, int placement);
    int get_lyrics_placement(int line);

//    //interface for ChordBuilder
//    void add_chord(ImoChord* pChord);

    //access to score being analysed
    int set_score_version(const string& version);
    inline int get_score_version() { return m_scoreVersion; }
    inline void score_analysis_begin(ImoScore* pScore) { m_pCurScore = pScore; }
    inline void score_analysis_end() {
        m_pLastScore = m_pCurScore;
        m_pCurScore = nullptr;
    }
    inline ImoScore* get_score_being_analysed() { return m_pCurScore; }
    inline ImoScore* get_last_analysed_score() { return m_pLastScore; }

    //access to document being analysed
    inline Document* get_document_being_analysed() { return m_pDoc; }
    inline const string& get_document_locator() { return m_fileLocator; }

    //access to root ImoDocument
    inline void save_root_imo_document(ImoDocument* pDoc) { m_pImoDoc = pDoc; }
    inline ImoDocument* get_root_imo_document() { return m_pImoDoc; }

    //methods related to analysing instruments
    void require_instr_id() { m_fInstrIdRequired = true; }
    bool is_instr_id_required() { return m_fInstrIdRequired; }

    //static methods for general use
    static int ldp_name_to_key_type(const string& value);
    static int ldp_name_to_clef_type(const string& value);
    static bool ldp_pitch_to_components(const string& pitch, int *step, int* octave,
                                        EAccidentals* accidentals);

protected:
    ElementAnalyser* new_analyser(ELdpElement type, ImoObj* pAnchor=nullptr);
    void delete_relation_builders();
    void add_marging_space_for_lyrics(ImoNote* pNote, ImoLyric* pLyric);

    //auxiliary
    bool to_integer(const string& text, int* pResult);

    //auxiliary. for ldp notes analysis
    static int to_step(const char& letter);
    static int to_octave(const char& letter);
    static EAccidentals to_accidentals(const std::string& accidentals);
};


}   //namespace lomse

#endif      //__LOMSE_LDP_ANALYSER_H__

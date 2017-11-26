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

#ifndef __LOMSE_LDP_PARSER_H__
#define __LOMSE_LDP_PARSER_H__

#include <vector>
#include <set>

#include "lomse_ldp_factory.h"
#include "lomse_tokenizer.h"
#include "lomse_ldp_elements.h"
#include "lomse_reader.h"
#include "lomse_parser.h"

using namespace std;

namespace lomse
{

//forward declarations
class LdpFactory;

// The LDP parser
class LdpParser : public Parser
{
protected:
    LdpTree* m_tree;

public:
    LdpParser(ostream& reporter, LdpFactory* pFactory);
    ~LdpParser();

//    //setings and options
//    inline void SetIgnoreList(std::set<long>* pSet) { m_pIgnoreSet = pSet; }
//
    void parse_file(const std::string& filename, bool fErrorMsg = true);
    void parse_text(const std::string& sourceText);
    void parse_input(LdpReader& reader);

    //access to parser result
    LdpTree* get_ldp_tree();
    inline void release_last_tree_ownership() {
        m_tree->set_root(nullptr);
        delete m_tree;
        m_tree = nullptr;
    }

//    inline int get_num_errors() { return m_numErrors; }

protected:
    enum EParsingState
    {
        A0_WaitingForStartOfElement = 0,
        A1_WaitingForName,
        A2_WaitingForParameter,
        A3_ProcessingParameter,
        A4_Exit,
        A5_ExitError
    };

    void do_syntax_analysis(LdpReader& reader);

    void clear_all();
    void PushNode(EParsingState nPopState);
    bool PopNode();
    void Do_WaitingForStartOfElement();
    void Do_WaitingForName();
    void Do_ProcessingParameter();
    bool must_replace_tag(const std::string& nodename);
    void replace_current_tag();
    void terminate_current_parameter();

    void report_error(EParsingState nState, LdpToken* pTk);
    void report_error(const std::string& msg);

    LdpFactory*     m_pLdpFactory;
    LdpTokenizer*   m_pTokenizer;
    LdpToken*       m_pTk;              // current token
    EParsingState   m_state;            // current automata state
    std::stack<pair<EParsingState, LdpElement*> >  m_stack;    // To save current automata state and node
    LdpElement*     m_curNode;             //node in process

    // parsing control, options and error variables
//    bool            m_fDebugMode;
//    std::set<long>*         m_pIgnoreSet;   //set with elements to ignore
};


} //namespace lomse

#endif    //__LOMSE_LDP_PARSER_H__

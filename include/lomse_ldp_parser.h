//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_LDP_PARSER_H__
#define __LOMSE_LDP_PARSER_H__

#include <vector>
#include <set>
#include <stack>

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
    void parse_file(const std::string& filename, bool fErrorMsg = true) override;
    void parse_text(const std::string& sourceText) override;
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

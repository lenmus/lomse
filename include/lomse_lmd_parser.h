//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_LMD_PARSER_H__
#define __LOMSE_LMD_PARSER_H__

#include "lomse_parser.h"

//#include <vector>
//#include <set>
#include <string>
using namespace std;

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "lomse_internal_model.h"
//#include "lomse_ldp_elements.h"
//#include "lomse_reader.h"


namespace lomse
{

//forward declarations
//class LdpFactory;
//class XmlNode;

typedef rapidxml::xml_node<>         XmlNode;
typedef rapidxml::xml_attribute<>    XmlAttribute;

//---------------------------------------------------------------------------------------
class LmdParser : public Parser
{
private:
    rapidxml::xml_document<> m_doc;
    rapidxml::xml_node<>* m_root;
    string m_encoding;         //i.e. "utf-8"
    string m_error;
    rapidxml::file<>* m_file;

public:
    LmdParser(ostream& reporter=cout);
    ~LmdParser();

    void parse_file(const std::string& filename, bool fErrorMsg = true);
    void parse_text(const std::string& sourceText);
//    XmlNode* parse_input(LdpReader& reader);
    inline const string& get_error() { return m_error; }

    inline const string& get_encoding() { return m_encoding; }

    inline XmlNode* get_tree_root() { return m_root; }


    //node utility methods
    //ELdpElement get_node_type(XmlNode* node);
    string get_node_value(XmlNode* node);
    string get_node_name_as_string(XmlNode* node);
    inline const char* get_node_name(XmlNode* node) { return node->name(); }
//    long get_id() { return 0L; }
//    XmlNode* get_next_sibling() { return NULL; }
//    XmlNode* get_first_child() { return NULL; }
//    int get_line_number() { return 0; }
//    XmlNode* get_parameter(int i) { return NULL; }
//    float get_value_as_float() { return 0.0f; }
//
//
//    ImoObj* get_imo() { return NULL; }
//    void set_imo(ImoObj* pImo) {}
//    ELdpElement get_type();
//    bool is_type(ELdpElement type) { return false; }

protected:
    void parse_char_string(char* string);


};

////
////---------------------------------------------------------------------------------------
//// To wrap, isolate and extend rapidxml::xml_node<>
//class XmlNode
//{
//private:
//    rapidxml::xml_node<>* m_node;
//    LmdParser* m_parser;
//
//public:
//    XmlNode() : m_node(NULL), m_parser(NULL) {}
//    XmlNode(LmdParser* parser, rapidxml::xml_node<>* node) : m_node(node), m_parser(parser) {}
//
//    const string& get_value() { return "TODO"; }
//    //const char* get_name() const { return m_node->name(); }
//    string get_name() { return "TODO"; }
//    long get_id() { return 0L; }
//    XmlNode* get_next_sibling() { return NULL; }
//    XmlNode* get_first_child() { return NULL; }
//    int get_line_number() { return 0; }
//    XmlNode* get_parameter(int i) { return NULL; }
//    float get_value_as_float() { return 0.0f; }
//
//
//    ImoObj* get_imo() { return NULL; }
//    void set_imo(ImoObj* pImo) {}
//    ELdpElement get_type();
//    bool is_type(ELdpElement type) { return false; }
//
//};


//// The LDP parser
//class LmdParser
//{
//public:
//    LmdParser(ostream& reporter, LdpFactory* pFactory);
//    ~LmdParser();
//
////    //setings and options
////    inline void SetIgnoreList(std::set<long>* pSet) { m_pIgnoreSet = pSet; }
////
//    XmlNode* parse_file(const std::string& filename, bool fErrorMsg = true);
//    XmlNode* parse_text(const std::string& sourceText);
//    XmlNode* parse_input(LdpReader& reader);
//
//    inline int get_num_errors() { return m_numErrors; }
//    inline long get_max_id() { return m_nMaxId; }
//
//protected:
//    enum EParsingState
//    {
//        A0_WaitingForStartOfElement = 0,
//        A1_WaitingForName,
//        A2_WaitingForParameter,
//        A3_ProcessingParameter,
//        A4_Exit,
//        A5_ExitError
//    };
//
//    XmlNode* do_syntax_analysis(LdpReader& reader);
//
//    void clear_all();
//    void PushNode(EParsingState nPopState);
//    bool PopNode();
//    void Do_WaitingForStartOfElement();
//    void Do_WaitingForName();
//    void Do_WaitingForParameter();
//    void Do_ProcessingParameter();
//    bool must_replace_tag(const std::string& nodename);
//    void replace_current_tag();
//    void terminate_current_parameter();
//
//    void report_error(EParsingState nState, LdpToken* pTk);
//    void report_error(const std::string& msg);
//
////    long GetNodeId(SpLdpElement pNode);
////
//
//    ostream&        m_reporter;
//    LdpFactory*     m_pLdpFactory;
//    LdpTokenizer*   m_pTokenizer;
//    LdpToken*       m_pTk;              // current token
//    EParsingState   m_state;            // current automata state
//    long            m_id;
//    std::stack<pair<EParsingState, LdpElement*> >  m_stack;    // To save current automata state and node
//    LdpElement*     m_curNode;             //node in process
//
//    // parsing control, options and error variables
////    bool            m_fDebugMode;
//    int            m_numErrors;     // number of errors found during parsing
//    long           m_nMaxId;        //maximun ID found
////    std::set<long>*         m_pIgnoreSet;   //set with elements to ignore
//};


} //namespace lomse

#endif    //__LOMSE_LMD_PARSER_H__

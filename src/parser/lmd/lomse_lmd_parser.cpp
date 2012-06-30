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

#include "lomse_lmd_parser.h"

#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>
using namespace std;

//#include "rapidxml_print.hpp"
using namespace rapidxml;

namespace lomse
{


//=======================================================================================
// LmdParser implementation
//=======================================================================================
LmdParser::LmdParser(ostream& reporter)
    : Parser(reporter)
    , m_root(NULL)
    , m_file(NULL)
{
}

//---------------------------------------------------------------------------------------
LmdParser::~LmdParser()
{
    delete m_file;
}

//---------------------------------------------------------------------------------------
void LmdParser::parse_text(const std::string& sourceText)
{
    parse_char_string( const_cast<char*>(sourceText.c_str()) );
}

//---------------------------------------------------------------------------------------
void LmdParser::parse_file(const std::string& filename, bool fErrorMsg)
{
    delete m_file;
    m_file = new rapidxml::file<>( filename.c_str() );
    parse_char_string( m_file->data() );
}

//---------------------------------------------------------------------------------------
void LmdParser::parse_char_string(char* str)
{
    try
    {
        m_doc.parse<  parse_declaration_node  //XML declaration node
                    //| parse_no_data_nodes
                    //| parse_comment_nodes
                    //| parse_doctype_node
                    | parse_no_element_values
                    | parse_normalize_whitespace
                    >( m_doc.allocate_string(str) );

        // since we have parsed the XML declaration, it is the first node (if exists).
        // Otherwise the first will be the root node
        m_root = m_doc.first_node();
        if (m_root->type() == rapidxml::node_declaration)
        {
            m_encoding = m_root->first_attribute("encoding")->value();
            while (m_root && m_root->type() != rapidxml::node_element)
                m_root = m_root->next_sibling();
        }
        else
            m_encoding = "unknown";
    }

    catch( rapidxml::parse_error& e)
    {
        m_error = e.what();
        m_root = NULL;
    }
}

//---------------------------------------------------------------------------------------
string LmdParser::get_node_value(XmlNode* node)
{
    return string( node->value() );
}

//---------------------------------------------------------------------------------------
string LmdParser::get_node_name_as_string(XmlNode* node)
{
    return string( node->name() );
}



////=======================================================================================
//// XmlNode implementation
////=======================================================================================
//ELdpElement XmlNode::get_type()
//{
//    return m_parser->get_node_type(this);
//}

//const char* XmlNode::get_name() const
//{
//    return m_node->value();
//}

//
//LmdParser::LmdParser(ostream& reporter, LdpFactory* pFactory)
//    : m_reporter(reporter)
//    , m_pLdpFactory(pFactory)
//    //, m_fDebugMode(g_pLogger->IsAllowedTraceMask("LmdParser"))
//    //, m_pIgnoreSet((std::set<long>*)NULL)
//    , m_pTokenizer(NULL)
//    , m_pTk(NULL)
//    , m_curNode(NULL)
//{
//}
//
////---------------------------------------------------------------------------------------
//LmdParser::~LmdParser()
//{
//    clear_all();
//}
//
////---------------------------------------------------------------------------------------
//void LmdParser::clear_all()
//{
//    delete m_pTokenizer;
//    m_pTokenizer = NULL;
//
//    while (!m_stack.empty())
//    {
//        std::pair<EParsingState, LdpElement*> data = m_stack.top();
//        delete data.second;
//        m_stack.pop();
//    }
//    m_curNode = NULL;
//    m_numErrors = 0;
//}
//
////---------------------------------------------------------------------------------------
//SpLdpTree LmdParser::parse_text(const std::string& sourceText)
//{
//    LdpTextReader reader(sourceText);
//    return parse_input(reader);
//    //return do_syntax_analysis(reader);
//}
//
////---------------------------------------------------------------------------------------
//SpLdpTree LmdParser::parse_file(const std::string& filename, bool fErrorMsg)
//{
//    LdpFileReader reader(filename);
//    return parse_input(reader);
//    //return do_syntax_analysis(reader);
//}
//
////---------------------------------------------------------------------------------------
//SpLdpTree LmdParser::parse_input(LdpReader& reader)
//{
//    return do_syntax_analysis(reader);
//}
//
////---------------------------------------------------------------------------------------
//SpLdpTree LmdParser::do_syntax_analysis(LdpReader& reader)
//{
//    //This function analyzes source code. The result of the analysis is a tree
//    //of nodes, each one representing an element. The root node is the parsed
//    //elemnent, usually the whole score. Nevertheless, the parser can be used
//    //to parse any sub-element, such as a note, or a measure.
//    //
//    //This method performs the lexical and syntactical analysis and,
//    //as result, builds a tree of syntactically correct nodes: the source code
//    //has the structure of an element with nested elements (data between parenthesis).
//    //
//    //The analyzer is implemented with a main loop to deal with current
//    //automata state and as many functions as automata states, to perform the
//    //tasks asociated to each state.
//
//    clear_all();
//
//    m_pTokenizer = LOMSE_NEW LdpTokenizer(reader, m_reporter);
//    m_pTokenizer->skip_utf_bom();
//    m_id = 0L;
//    m_state = A0_WaitingForStartOfElement;
//    PushNode(A0_WaitingForStartOfElement);      //start the tree with the root node
//    bool fExitLoop = false;
//    while(!fExitLoop)
//    {
//        m_pTk = m_pTokenizer->read_token();        //m_pTk->read_token();
//
//        switch (m_state) {
//            case A0_WaitingForStartOfElement:
//                Do_WaitingForStartOfElement();
//                break;
//            case A1_WaitingForName:
//                Do_WaitingForName();
//                break;
//            case A2_WaitingForParameter:
//                Do_ProcessingParameter();
//                break;
//            case A3_ProcessingParameter:
//                Do_ProcessingParameter();
//                break;
//            case A4_Exit:
//            case A5_ExitError:
//                fExitLoop = true;
//                break;
//            default:
//                report_error(m_state, m_pTk);
//                fExitLoop = true;
//        }
//        if (m_pTk->get_type() == tkEndOfFile)
//            fExitLoop = true;
//    }
//    m_nMaxId = --m_id;
//
//    // exit if error
//    if (m_state == A5_ExitError)
//        return SpLdpTree( LOMSE_NEW LdpTree() );
//
//    // at this point m_curNode is all the tree
//    if (!m_curNode)
//        throw std::runtime_error(
//            "[LmdParser::do_syntax_analysis] LDP file format error.");
//
//    return SpLdpTree( LOMSE_NEW LdpTree(m_curNode) );
//}
//
////---------------------------------------------------------------------------------------
//void LmdParser::Do_WaitingForStartOfElement()
//{
//    switch (m_pTk->get_type())
//    {
//        case tkStartOfElement:
//            m_state = A1_WaitingForName;
//            break;
//        case tkEndOfFile:
//            m_state = A4_Exit;
//            break;
//        default:
//            report_error(m_state, m_pTk);
//            m_state = A0_WaitingForStartOfElement;
//    }
//}
//
////---------------------------------------------------------------------------------------
//void LmdParser::Do_WaitingForName()
//{
//    switch (m_pTk->get_type())
//    {
//        case tkLabel:
//        {
//            //check if the name has an ID and extract it
//            const std::string& tagname = m_pTk->get_value();
//            std::string nodename = tagname;
//            size_t i = tagname.find('#');
//            if (i != string::npos)
//            {
//                long id;
//                nodename = tagname.substr(0, i);
//                std::istringstream sid( tagname.substr(i+1) );
//                if (!(sid >> id))
//                    m_reporter << "Line " << m_pTk->get_line_number()
//                               << ". Bad id in name '" + tagname + "'." << endl;
//                else
//                {
//                    if (id < m_id)
//                        m_reporter << "Line " << m_pTk->get_line_number()
//                                << ". In '" + tagname + "'. Value for id already exists. Ignored." << endl;
//                    else
//                        m_id = id;
//                }
//            }
//
//            //create the node
//            m_curNode = m_pLdpFactory->create(nodename, m_pTk->get_line_number());
//            if (m_curNode->get_type() == k_undefined)
//                m_reporter << "Line " << m_pTk->get_line_number()
//                           << ". Unknown tag '" + nodename + "'." << endl;
//            m_curNode->set_id(m_id++);
//            m_state = A2_WaitingForParameter;
//            break;
//        }
//
//        default:
//            report_error(m_state, m_pTk);
//            if (m_pTk->get_type() == tkEndOfFile)
//                m_state = A4_Exit;
//            else
//                m_state = A1_WaitingForName;
//    }
//
//}
//
////---------------------------------------------------------------------------------------
//void LmdParser::Do_WaitingForParameter()
//{
//    //switch (m_pTk->get_type())
//    //{
//    //    case tkStartOfElement:
//    //        PushNode(A3_ProcessingParameter);    // add current node (name of element or parameter) to the tree
//    //        m_pTokenizer->repeat_token();
//    //        m_state = A0_WaitingForStartOfElement;
//    //        break;
//    //    case tkLabel:
//    //        //m_curNode->append_child( m_pLdpFactory->new_label(m_pTk->get_value(),
//    //        //                                                  m_pTk->get_line_number()) );
//    //        //m_state = A3_ProcessingParameter;
//    //        //break;
//    //        if ( must_replace_tag(m_pTk->get_value()) )
//    //            replace_current_tag();
//    //        else
//    //        {
//    //            m_curNode->append_child( m_pLdpFactory->new_label(m_pTk->get_value(),
//    //                                                              m_pTk->get_line_number()) );
//    //            m_state = A3_ProcessingParameter;
//    //        }
//    //        break;
//    //    case tkIntegerNumber:
//    //    case tkRealNumber:
//    //        m_curNode->append_child( m_pLdpFactory->new_number(m_pTk->get_value(),
//    //                                                           m_pTk->get_line_number()) );
//    //        m_state = A3_ProcessingParameter;
//    //        break;
//    //    case tkString:
//    //        m_curNode->append_child( m_pLdpFactory->new_string(m_pTk->get_value(),
//    //                                                           m_pTk->get_line_number()) );
//    //        m_state = A3_ProcessingParameter;
//    //        break;
//    //    default:
//    //        report_error(m_state, m_pTk);
//    //        if (m_pTk->get_type() == tkEndOfFile)
//    //            m_state = A4_Exit;
//    //        else
//    //            m_state = A2_WaitingForParameter;
//    //}
//}
//
////---------------------------------------------------------------------------------------
//void LmdParser::Do_ProcessingParameter()
//{
//    switch (m_pTk->get_type())
//    {
//        case tkLabel:
//            //m_curNode->append_child( m_pLdpFactory->new_label(m_pTk->get_value(),
//            //                                                  m_pTk->get_line_number()) );
//            //m_state = A3_ProcessingParameter;
//            //break;
//            if ( must_replace_tag(m_pTk->get_value()) )
//                replace_current_tag();
//            else
//            {
//                m_curNode->append_child( m_pLdpFactory->new_label(m_pTk->get_value(),
//                                                                  m_pTk->get_line_number()) );
//                m_state = A3_ProcessingParameter;
//            }
//            break;
//        case tkIntegerNumber:
//        case tkRealNumber:
//            m_curNode->append_child( m_pLdpFactory->new_number(m_pTk->get_value(),
//                                                               m_pTk->get_line_number()) );
//            m_state = A3_ProcessingParameter;
//            break;
//        case tkString:
//            m_curNode->append_child( m_pLdpFactory->new_string(m_pTk->get_value(),
//                                                               m_pTk->get_line_number()) );
//            m_state = A3_ProcessingParameter;
//            break;
//        case tkStartOfElement:
//            PushNode(A3_ProcessingParameter);    // add current node (name of element or parameter) to the tree
//            m_pTokenizer->repeat_token();
//            m_state = A0_WaitingForStartOfElement;
//            break;
//        case tkEndOfElement:
//            terminate_current_parameter();
//            break;
//        default:
//            report_error(m_state, m_pTk);
//            if (m_pTk->get_type() == tkEndOfFile)
//                m_state = A4_Exit;
//            else
//                m_state = A3_ProcessingParameter;
//    }
//}
//
////---------------------------------------------------------------------------------------
//bool LmdParser::must_replace_tag(const std::string& nodename)
//{
//    return nodename == "noVisible";
//}
//
////---------------------------------------------------------------------------------------
//void LmdParser::replace_current_tag()
//{
//    //TODO: refactor to deal with many replacements
//
//    PushNode(A3_ProcessingParameter);    // add current node (name of element or parameter) to the tree
//
//    //get new element name
//    //const std::string& oldname = m_pTk->get_value();
//    std::string newname;
//    //if (oldname == "noVisible")
//        newname = "visible";
//    //else if (oldname == "l")
//    //    newname = "tied";
//
//    //create the replacement node
//    m_curNode = m_pLdpFactory->create(newname, m_pTk->get_line_number());
//    m_curNode->set_id(m_id++);
//
//    //add parameter
//    m_curNode->append_child( m_pLdpFactory->new_label("no",
//                                                      m_pTk->get_line_number()) );
//
//    //close node
//    terminate_current_parameter();
//}
//
////---------------------------------------------------------------------------------------
//void LmdParser::terminate_current_parameter()
//{
//    m_state = A3_ProcessingParameter;
//    LdpElement* pParm = m_curNode;        //save ptr to node just created
//    if (PopNode()) {                      //restore previous node (the owner of this parameter)
//        //error
//        m_state = A5_ExitError;
//    }
//    else
//    {
//        if (m_curNode)
//            m_curNode->append_child(pParm);
//        else
//            m_curNode = pParm;
//
//        ////Filter out this element if its ID is in the ignore list
//        //long nId = GetNodeId(pParm);
//        //if (!(m_pIgnoreSet
//        //      && nId != lmNEW_ID
//        //      && m_pIgnoreSet->find(nId) != m_pIgnoreSet->end() ))
//        //    m_curNode->append_child(pParm);
//        //else
//        //    delete pParm;   //discard this node
//    }
//}
//
////---------------------------------------------------------------------------------------
//void LmdParser::PushNode(EParsingState state)
//{
//    std::pair<EParsingState, LdpElement*> data(state, m_curNode);
//    m_stack.push(data);
//}
//
////---------------------------------------------------------------------------------------
//bool LmdParser::PopNode()
//{
//    //returns true if error
//
//    if (m_stack.size() == 0)
//    {
//        //more closing parenthesis than parenthesis opened
//        report_error("Syntax error: more closing parenthesis than parenthesis opened. Analysis stopped.");
//        return true;    //error
//    }
//    else
//    {
//        std::pair<EParsingState, LdpElement*> data = m_stack.top();
//        m_state = data.first;
//        m_curNode = data.second;
//        m_stack.pop();
//        return false;   //no error
//    }
//}
//
////---------------------------------------------------------------------------------------
//void LmdParser::report_error(EParsingState nState, LdpToken* pTk)
//{
//    m_numErrors++;
//    m_reporter << "** LDP ERROR **: Syntax error. State " << nState
//               << ", TkType " << pTk->get_type()
//               << ", tkValue <" << pTk->get_value() << ">" << endl;
//}
//
////---------------------------------------------------------------------------------------
//void LmdParser::report_error(const std::string& msg)
//{
//    m_numErrors++;
//    m_reporter << msg << endl;
//}
//
////========================================================================================
////========================================================================================
////========================================================================================
//#if 0
//long LmdParser::GetNodeId(LdpElement* pNode)
//{
//    long nId = pNode->get_id();
//    return nId;
//}
//
//bool LmdParser::ParenthesisMatch(const std::string& sSource)
//{
//    int i = sSource.length();
//    int nPar = 0;
//    for(i=0; i < (int)sSource.length(); i++) {
//        if (sSource.GetChar(i) == _T('('))
//            nPar++;
//        else if (sSource.GetChar(i) == _T(')'))
//            nPar--;
//    }
//    return (nPar == 0);
//}
//
//
//#endif

} //namespace lomse


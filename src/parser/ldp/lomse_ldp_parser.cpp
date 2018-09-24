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

#include "lomse_ldp_parser.h"

#include <algorithm>
#include <iostream>
#include "lomse_ldp_factory.h"
#include "lomse_logger.h"

using namespace std;

namespace lomse
{


//=======================================================================================
// LdpParser implementation
//=======================================================================================

LdpParser::LdpParser(ostream& reporter, LdpFactory* pFactory)
    : Parser(reporter)
    , m_tree(nullptr)
    , m_pLdpFactory(pFactory)
    //, m_fDebugMode(g_pLogger->IsAllowedTraceMask("LdpParser"))
    //, m_pIgnoreSet((std::set<long>*)nullptr)
    , m_pTokenizer(nullptr)
    , m_pTk(nullptr)
    , m_state(A0_WaitingForStartOfElement)
    , m_curNode(nullptr)
{
}

//---------------------------------------------------------------------------------------
LdpParser::~LdpParser()
{
    clear_all();
}

//---------------------------------------------------------------------------------------
void LdpParser::clear_all()
{
    delete m_pTokenizer;
    m_pTokenizer = nullptr;

    while (!m_stack.empty())
    {
        std::pair<EParsingState, LdpElement*> data = m_stack.top();
        delete data.second;
        m_stack.pop();
    }
    m_curNode = nullptr;
    m_numErrors = 0;
    delete m_tree;
    m_tree = nullptr;
}

//---------------------------------------------------------------------------------------
void LdpParser::parse_text(const std::string& sourceText)
{
    LdpTextReader reader(sourceText);
    parse_input(reader);
}

//---------------------------------------------------------------------------------------
void LdpParser::parse_file(const std::string& filename, bool UNUSED(fErrorMsg))
{
    LdpFileReader reader(filename);
    parse_input(reader);
}

//---------------------------------------------------------------------------------------
void LdpParser::parse_input(LdpReader& reader)
{
    do_syntax_analysis(reader);
}

//---------------------------------------------------------------------------------------
LdpTree* LdpParser::get_ldp_tree()
{
    return m_tree;  //SpLdpTree( LOMSE_NEW LdpTree(m_curNode) );
}

//---------------------------------------------------------------------------------------
void LdpParser::do_syntax_analysis(LdpReader& reader)
{
    //This function analyzes source code. The result of the analysis is a tree
    //of nodes, each one representing an element. The root node is the parsed
    //elemnent, usually the whole score. Nevertheless, the parser can be used
    //to parse any sub-element, such as a note, or a measure.
    //
    //This method performs the lexical and syntactical analysis and,
    //as result, builds a tree of syntactically correct nodes: the source code
    //has the structure of an element with nested elements (data between parenthesis).
    //
    //The analyzer is implemented with a main loop to deal with current
    //automata state and as many functions as automata states, to perform the
    //tasks asociated to each state.

    clear_all();

    m_pTokenizer = LOMSE_NEW LdpTokenizer(reader, m_reporter);
    m_pTokenizer->skip_utf_bom();
    m_state = A0_WaitingForStartOfElement;
    PushNode(A0_WaitingForStartOfElement);      //start the tree with the root node
    bool fExitLoop = false;
    while(!fExitLoop)
    {
        m_pTk = m_pTokenizer->read_token();        //m_pTk->read_token();

        switch (m_state) {
            case A0_WaitingForStartOfElement:
                Do_WaitingForStartOfElement();
                break;
            case A1_WaitingForName:
                Do_WaitingForName();
                break;
            case A2_WaitingForParameter:
                Do_ProcessingParameter();
                break;
            case A3_ProcessingParameter:
                Do_ProcessingParameter();
                break;
            case A4_Exit:
            case A5_ExitError:
                fExitLoop = true;
                break;
            default:
                report_error(m_state, m_pTk);
                fExitLoop = true;
        }
        if (m_pTk->get_type() == tkEndOfFile)
            fExitLoop = true;
    }

    // exit if error
    if (m_state == A5_ExitError)
    {
        m_tree = nullptr;
        return;
    }

    // at this point m_curNode is all the tree
    if (!m_curNode)
    {
        LOMSE_LOG_ERROR("[LdpParser::do_syntax_analysis] LDP file format error.");
        report_error("[LdpParser::do_syntax_analysis] LDP file format error.");
        //throw runtime_error("[LdpParser::do_syntax_analysis] LDP file format error.");
        m_tree = nullptr;
        return;
    }

    m_tree = LOMSE_NEW LdpTree(m_curNode);
}

//---------------------------------------------------------------------------------------
void LdpParser::Do_WaitingForStartOfElement()
{
    switch (m_pTk->get_type())
    {
        case tkStartOfElement:
            m_state = A1_WaitingForName;
            break;
        case tkEndOfFile:
            m_state = A4_Exit;
            break;
        default:
            report_error(m_state, m_pTk);
            m_state = A0_WaitingForStartOfElement;
    }
}

//---------------------------------------------------------------------------------------
void LdpParser::Do_WaitingForName()
{
    switch (m_pTk->get_type())
    {
        case tkLabel:
        {
            //check if the name has an ID and extract it
            const std::string& tagname = m_pTk->get_value();
            std::string nodename = tagname;
            size_t i = tagname.find('#');
            ImoId id = k_no_imoid;
            if (i != string::npos)
            {
                nodename = tagname.substr(0, i);
                std::istringstream sid( tagname.substr(i+1) );
                if (!(sid >> id))
                {
                    m_reporter << "Line " << m_pTk->get_line_number()
                               << ". Bad id in name '" + tagname + "'." << endl;
                    id = k_no_imoid;
                }
            }

            //create the node
            m_curNode = m_pLdpFactory->create(nodename, m_pTk->get_line_number());
            if (m_curNode->get_type() == k_undefined)
                m_reporter << "Line " << m_pTk->get_line_number()
                           << ". Unknown tag '" + nodename + "'." << endl;
            m_curNode->set_id(id);
            m_state = A2_WaitingForParameter;
            break;
        }

        default:
            report_error(m_state, m_pTk);
            if (m_pTk->get_type() == tkEndOfFile)
                m_state = A4_Exit;
            else
                m_state = A1_WaitingForName;
    }

}

//---------------------------------------------------------------------------------------
void LdpParser::Do_ProcessingParameter()
{
    switch (m_pTk->get_type())
    {
        case tkLabel:
            //m_curNode->append_child( m_pLdpFactory->new_label(m_pTk->get_value(),
            //                                                  m_pTk->get_line_number()) );
            //m_state = A3_ProcessingParameter;
            //break;
            if ( must_replace_tag(m_pTk->get_value()) )
                replace_current_tag();
            else
            {
                m_curNode->append_child( m_pLdpFactory->new_label(m_pTk->get_value(),
                                                                  m_pTk->get_line_number()) );
                m_state = A3_ProcessingParameter;
            }
            break;
        case tkIntegerNumber:
        case tkRealNumber:
            m_curNode->append_child( m_pLdpFactory->new_number(m_pTk->get_value(),
                                                               m_pTk->get_line_number()) );
            m_state = A3_ProcessingParameter;
            break;
        case tkString:
            m_curNode->append_child( m_pLdpFactory->new_string(m_pTk->get_value(),
                                                               m_pTk->get_line_number()) );
            m_state = A3_ProcessingParameter;
            break;
        case tkStartOfElement:
            PushNode(A3_ProcessingParameter);    // add current node (name of element or parameter) to the tree
            m_pTokenizer->repeat_token();
            m_state = A0_WaitingForStartOfElement;
            break;
        case tkEndOfElement:
            terminate_current_parameter();
            break;
        default:
            report_error(m_state, m_pTk);
            terminate_current_parameter();
            if (m_pTk->get_type() == tkEndOfFile)
                m_state = A4_Exit;
            else
                m_state = A3_ProcessingParameter;
    }
}

//---------------------------------------------------------------------------------------
bool LdpParser::must_replace_tag(const std::string& nodename)
{
    return nodename == "noVisible";
}

//---------------------------------------------------------------------------------------
void LdpParser::replace_current_tag()
{
    //TODO: refactor to deal with many replacements

    PushNode(A3_ProcessingParameter);    // add current node (name of element or parameter) to the tree

    //get new element name
    //const std::string& oldname = m_pTk->get_value();
    std::string newname;
    //if (oldname == "noVisible")
        newname = "visible";
    //else if (oldname == "l")
    //    newname = "tied";

    //create the replacement node
    m_curNode = m_pLdpFactory->create(newname, m_pTk->get_line_number());

    //add parameter
    m_curNode->append_child( m_pLdpFactory->new_label("no",
                                                      m_pTk->get_line_number()) );

    //close node
    terminate_current_parameter();
}

//---------------------------------------------------------------------------------------
void LdpParser::terminate_current_parameter()
{
    m_state = A3_ProcessingParameter;
    LdpElement* pParm = m_curNode;        //save ptr to node just created
    if (PopNode()) {                      //restore previous node (the owner of this parameter)
        //error
        m_state = A5_ExitError;
    }
    else
    {
        if (m_curNode)
            m_curNode->append_child(pParm);
        else
            m_curNode = pParm;

        ////Filter out this element if its ID is in the ignore list
        //long nId = GetNodeId(pParm);
        //if (!(m_pIgnoreSet
        //      && nId != lmNEW_ID
        //      && m_pIgnoreSet->find(nId) != m_pIgnoreSet->end() ))
        //    m_curNode->append_child(pParm);
        //else
        //    delete pParm;   //discard this node
    }
}

//---------------------------------------------------------------------------------------
void LdpParser::PushNode(EParsingState state)
{
    std::pair<EParsingState, LdpElement*> data(state, m_curNode);
    m_stack.push(data);
}

//---------------------------------------------------------------------------------------
bool LdpParser::PopNode()
{
    //returns true if error

    if (m_stack.size() == 0)
    {
        //more closing parenthesis than parenthesis opened
        report_error("Syntax error: more closing parenthesis than parenthesis opened. Analysis stopped.");
        return true;    //error
    }
    else
    {
        std::pair<EParsingState, LdpElement*> data = m_stack.top();
        m_state = data.first;
        m_curNode = data.second;
        m_stack.pop();
        return false;   //no error
    }
}

//---------------------------------------------------------------------------------------
void LdpParser::report_error(EParsingState nState, LdpToken* pTk)
{
    m_numErrors++;
    m_reporter << "** LDP ERROR **: Syntax error. State " << nState
               << ", TkType " << pTk->get_type()
               << ", tkValue <" << pTk->get_value() << ">" << endl;
}

//---------------------------------------------------------------------------------------
void LdpParser::report_error(const std::string& msg)
{
    m_numErrors++;
    m_reporter << msg << endl;
}


} //namespace lomse


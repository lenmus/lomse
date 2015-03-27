//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2014 Cecilio Salmeron. All rights reserved.
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

#include "lomse_xml_parser.h"

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
// XmlParser implementation
//=======================================================================================
XmlParser::XmlParser(ostream& reporter)
    : Parser(reporter)
    , m_root(NULL)
    , m_file(NULL)
{
}

//---------------------------------------------------------------------------------------
XmlParser::~XmlParser()
{
    delete m_file;
}

//---------------------------------------------------------------------------------------
void XmlParser::parse_text(const std::string& sourceText)
{
    parse_char_string( const_cast<char*>(sourceText.c_str()) );
}

//---------------------------------------------------------------------------------------
void XmlParser::parse_cstring(char* sourceText)
{
    parse_char_string(sourceText);
}

//---------------------------------------------------------------------------------------
void XmlParser::parse_file(const std::string& filename, bool fErrorMsg)
{
    delete m_file;
    m_file = new rapidxml::file<>( filename.c_str() );
    parse_char_string( m_file->data() );
}

//---------------------------------------------------------------------------------------
void XmlParser::parse_char_string(char* str)
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
        m_encoding = "unknown";
        if (m_root->type() == rapidxml::node_declaration)
        {
            if (m_root->first_attribute("encoding") != NULL)
                m_encoding = m_root->first_attribute("encoding")->value();
            while (m_root && m_root->type() != rapidxml::node_element)
                m_root = m_root->next_sibling();
        }
    }

    catch( rapidxml::parse_error& e)
    {
        m_error = e.what();
        m_root = NULL;
    }
}

//---------------------------------------------------------------------------------------
string XmlParser::get_node_value(XmlNode* node)
{
    return string( node->value() );
}

//---------------------------------------------------------------------------------------
string XmlParser::get_node_name_as_string(XmlNode* node)
{
    return string( node->name() );
}


} //namespace lomse


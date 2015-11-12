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


namespace lomse
{

//=======================================================================================
// XmlNode implementation
//=======================================================================================
int XmlNode::type()
{
    switch( m_node.type() )
    {
		case pugi::node_null:           return XmlNode::k_node_null;
		case pugi::node_document:       return XmlNode::k_node_document;
		case pugi::node_element:        return XmlNode::k_node_element;
		case pugi::node_pcdata:         return XmlNode::k_node_pcdata;
		case pugi::node_cdata:          return XmlNode::k_node_cdata;
		case pugi::node_comment:        return XmlNode::k_node_comment;
		case pugi::node_pi:             return XmlNode::k_node_pi;
		case pugi::node_declaration:    return XmlNode::k_node_declaration;
		case pugi::node_doctype:        return XmlNode::k_node_doctype;
		default:                        return XmlNode::k_node_unknown;
    }
}

//---------------------------------------------------------------------------------------
string XmlNode::value()
{
    //Depending on node type,
    //name or value may be absent. node_document nodes do not have a name or value,
    //node_element and node_declaration nodes always have a name but never have a value,
    //node_pcdata, node_cdata, node_comment and node_doctype nodes never have a
    //name but always have a value (it may be empty though), node_pi nodes always
    //have a name and a value (again, value may be empty).

    if (m_node.type() == pugi::node_pcdata)
        return string( m_node.value() );

    if (m_node.type() == pugi::node_element)
    {
        pugi::xml_node child = m_node.first_child();
        return string( child.value() );
    }

    return "";
}

//=======================================================================================
// XmlParser implementation
//=======================================================================================
XmlParser::XmlParser(ostream& reporter)
    : Parser(reporter)
    , m_root()
    , m_errorOffset(0)
{
}

//---------------------------------------------------------------------------------------
XmlParser::~XmlParser()
{
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
    pugi::xml_parse_result result = m_doc.load_file(filename.c_str());

    if (!result)
    {
        m_errorMsg = string(result.description());
        m_errorOffset = result.offset;
    }
    find_root();
}

//---------------------------------------------------------------------------------------
void XmlParser::parse_char_string(char* str)
{
    pugi::xml_parse_result result = m_doc.load_string(str, pugi::parse_default | pugi::parse_declaration);

    if (!result)
    {
        m_errorMsg = string(result.description());
        m_errorOffset = result.offset;
    }
    find_root();
}

//---------------------------------------------------------------------------------------
void XmlParser::find_root()
{
    pugi::xml_node root = m_doc.first_child();
    m_encoding = "unknown";
    if (root.type() == pugi::node_declaration)
    {
        if (root.attribute("encoding") != NULL)
            m_encoding = root.attribute("encoding").value();
    }
    while (root && root.type() != pugi::node_element)
        root = root.next_sibling();

    m_root = XmlNode(root);
}


} //namespace lomse


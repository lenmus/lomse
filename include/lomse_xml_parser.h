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

#ifndef __LOMSE_XML_PARSER_H__
#define __LOMSE_XML_PARSER_H__

#include "lomse_parser.h"

#include <string>
using namespace std;

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "lomse_internal_model.h"

namespace lomse
{

//forward declarations
//class LdpFactory;
//class XmlNode;

typedef rapidxml::xml_node<>         XmlNode;
typedef rapidxml::xml_attribute<>    XmlAttribute;

//---------------------------------------------------------------------------------------
class XmlParser : public Parser
{
private:
    rapidxml::xml_document<> m_doc;
    rapidxml::xml_node<>* m_root;
    string m_encoding;         //i.e. "utf-8"
    string m_error;
    rapidxml::file<>* m_file;

public:
    XmlParser(ostream& reporter=cout);
    ~XmlParser();

    void parse_file(const std::string& filename, bool fErrorMsg = true);
    void parse_text(const std::string& sourceText);
    void parse_cstring(char* sourceText);

    inline const string& get_error() { return m_error; }
    inline const string& get_encoding() { return m_encoding; }
    inline XmlNode* get_tree_root() { return m_root; }

    //node utility methods
    string get_node_value(XmlNode* node);
    string get_node_name_as_string(XmlNode* node);
    inline const char* get_node_name(XmlNode* node) { return node->name(); }

protected:
    void parse_char_string(char* string);


};


} //namespace lomse

#endif    //__LOMSE_XML_PARSER_H__

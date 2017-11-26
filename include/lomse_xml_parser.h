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

#ifndef __LOMSE_XML_PARSER_H__
#define __LOMSE_XML_PARSER_H__

#include "lomse_parser.h"
#include "lomse_internal_model.h"

#include <string>
using namespace std;

#include "pugixml/pugiconfig.hpp"
#include "pugixml/pugixml.hpp"
using namespace pugi;


namespace lomse
{

//forward declarations and definitions
typedef pugi::xml_document          XmlDocument;
typedef pugi::xml_attribute         XmlAttribute;


//---------------------------------------------------------------------------------------
class XmlNode
{
protected:
    pugi::xml_node  m_node;

    friend class XmlParser;
    XmlNode(pugi::xml_node node) : m_node(node) {}

public:
    XmlNode() {}
    XmlNode(const XmlNode& node) : m_node(node.m_node) {}
    XmlNode(const XmlNode* node) : m_node(node->m_node) {}

    string name() { return string(m_node.name()); }
    string value();
    XmlAttribute attribute(const string& name) {
        return m_node.attribute(name.c_str());
    }
    int type();

    enum {
		k_node_null = 0,	// Empty (null) node handle
		k_node_document,	// A document tree's absolute root
		k_node_element,		// Element tag, i.e. '<node/>'
		k_node_pcdata,		// Plain character data, i.e. 'text'
		k_node_cdata,		// Character data, i.e. '<![CDATA[text]]>'
		k_node_comment,		// Comment tag, i.e. '<!-- text -->'
		k_node_pi,			// Processing instruction, i.e. '<?name?>'
		k_node_declaration,	// Document declaration, i.e. '<?xml version="1.0"?>'
		k_node_doctype,		// Document type declaration, i.e. '<!DOCTYPE doc>'
        k_node_unknown
    };

    //XmlNode helper methods
    inline bool is_null() { return ! bool(m_node); }
	///get child with the specified name
    inline XmlNode child(const string& name) {
        return XmlNode( m_node.child(name.c_str()));
    }
    inline XmlNode first_child() { return XmlNode( m_node.first_child() ); }
    inline XmlNode next_sibling() { return XmlNode( m_node.next_sibling() ); }
    inline bool has_attribute(const string& name)
    {
        return m_node.attribute(name.c_str()) != nullptr;
    }
	///get value of attribute with the specified name
    inline string attribute_value(const string& name)
    {
        XmlAttribute attr = m_node.attribute(name.c_str());
        return string( attr.value() );
    }

    ptrdiff_t offset();

protected:
    string normalize_value(string str);

};

//---------------------------------------------------------------------------------------
class XmlParser : public Parser
{
private:
    XmlDocument m_doc;
    XmlNode m_root;
    string m_encoding;         //i.e. "utf-8"
    string m_errorMsg;
    int m_errorOffset;
    vector<ptrdiff_t> m_offsetData;     // offset -> line mapping
    bool m_fOffsetDataReady;
    string m_filename;

public:
    XmlParser(ostream& reporter=cout);
    ~XmlParser();

    void parse_file(const std::string& filename, bool fErrorMsg = true);
    void parse_text(const std::string& sourceText);
    void parse_cstring(char* sourceText);

    inline const string& get_error() { return m_errorMsg; }
    inline const string& get_encoding() { return m_encoding; }
    inline XmlNode* get_tree_root() { return &m_root; }
    int get_line_number(XmlNode* node);

protected:
    void parse_char_string(char* string);
    void find_root();
    bool build_offset_data(const char* file);
    std::pair<int, int> get_location(ptrdiff_t offset);

};


} //namespace lomse

#endif    //__LOMSE_XML_PARSER_H__

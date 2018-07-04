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

#include "lomse_xml_parser.h"

#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>
using namespace std;


//---------------------------------------------------------------------------------------
//AWARE: Microsoft deprecated fopen() but the "security enhanced" new function
//fopen_s() is only defined by Microsoft which "coincidentally" makes your code
//non-portable.
//
//The addressed security issue is, basically, for files opened for writing.
//Microsoft improves security by opening the file with exclusive access.
//
//This is not needed in this source code, as the file is opened for read, but there
//is a need to suppress the annoying Microsoft warnings, which is not easy.
//Thanks Microsoft!
//
//See:
//  https://stackoverflow.com/questions/906599/why-cant-i-use-fopen
//  https://stackoverflow.com/questions/858252/alternatives-to-ms-strncpy-s
//
#if (defined(_MSC_VER) && (_MSC_VER >= 1400) )
    #include <cstdio>

    inline extern FILE* my_fopen_s(const char *fname, char *mode)
    {
        FILE *fptr;
        fopen_s(&fptr, fname, mode);
        return fptr;
    }
    #define fopen(fname, mode)               my_fopen_s((fname), (mode))
#else
    #define fopen_s(fp, fmt, mode)          *(fp)=fopen((fmt), (mode))
#endif
//---------------------------------------------------------------------------------------




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
        return string(m_node.value());

    if (m_node.type() == pugi::node_element)
    {
        pugi::xml_node child = m_node.first_child();
        return string(child.value());
    }

    return "";
}

//---------------------------------------------------------------------------------------
ptrdiff_t XmlNode::offset()
{
    return m_node.offset_debug();
}


//=======================================================================================
// XmlParser implementation
//=======================================================================================
XmlParser::XmlParser(ostream& reporter)
    : Parser(reporter)
    , m_root()
    , m_errorOffset(0)
    , m_fOffsetDataReady(false)
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
void XmlParser::parse_file(const std::string& filename, bool UNUSED(fErrorMsg))
{
    m_fOffsetDataReady = false;
    m_filename = filename;
    pugi::xml_parse_result result = m_doc.load_file(filename.c_str(),
                                                    (pugi::parse_default |
                                                     //pugi::parse_trim_pcdata |
                                                     //pugi::parse_wnorm_attribute |
                                                     pugi::parse_declaration)
                                                   );

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
    m_fOffsetDataReady = false;
    m_filename.clear();
    pugi::xml_parse_result result = m_doc.load_string(str, (pugi::parse_default |
                                                            //pugi::parse_trim_pcdata |
                                                            //pugi::parse_wnorm_attribute |
                                                            pugi::parse_declaration)
                                                     );

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
        if (root.attribute("encoding") != nullptr)
            m_encoding = root.attribute("encoding").value();
    }
    while (root && root.type() != pugi::node_element)
        root = root.next_sibling();

    m_root = XmlNode(root);
}

//---------------------------------------------------------------------------------------
bool XmlParser::build_offset_data(const char* filename)
{
    //AWARE:
    // * Windows and DOS use a pair of CR (\r) and LF (\n) chars to end lines
    // * UNIX (including Linux and FreeBSD) uses only an LF char
    // * OS X also uses a single LF character
    // * The old classic Mac operating system used a single CR char
    //This code does not handle old Mac-style line breaks but it is not expected
    //to run in these old machines.
    //Also, this code does not handle tabs, that are counted as 1 char

    m_offsetData.clear();

    FILE* f = fopen(filename, "rb");
    if (!f)
        return false;

    ptrdiff_t offset = 0;

    char buffer[1024];
    size_t size;

    while ((size = fread(buffer, 1, sizeof(buffer), f)) > 0)
    {
        for (size_t i = 0; i < size; ++i)
            if (buffer[i] == '\n')
                m_offsetData.push_back(offset + i);

        offset += size;
    }

    fclose(f);

    return true;
}

//---------------------------------------------------------------------------------------
std::pair<int, int> XmlParser::get_location(ptrdiff_t offset)
{
    vector<ptrdiff_t>::const_iterator it =
        std::lower_bound(m_offsetData.begin(), m_offsetData.end(), offset);
    size_t index = it - m_offsetData.begin();

    return std::make_pair(1 + index, index == 0 ? offset + 1
                                                : offset - m_offsetData[index - 1]);
}

//---------------------------------------------------------------------------------------
int XmlParser::get_line_number(XmlNode* node)
{
    ptrdiff_t offset = node->offset();
    if (!m_fOffsetDataReady && !m_filename.empty())
        m_fOffsetDataReady = build_offset_data(m_filename.c_str());

    if ( m_fOffsetDataReady)
    {
        std::pair<int, int> pos = get_location(offset);
        return pos.first;
    }
    else
        return 0;
}


} //namespace lomse


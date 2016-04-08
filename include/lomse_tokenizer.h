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

#ifndef __LOMSE_LDP_TOKEN_H__
#define __LOMSE_LDP_TOKEN_H__

#include <sstream>

using namespace std;

namespace lomse
{

    class LdpReader;

enum ETokenType {
    tkStartOfElement = 0,
    tkEndOfElement,
    tkIntegerNumber,
    tkRealNumber,
    tkLabel,
    tkString,
    tkEndOfFile,
    //tokens for internal use
    tkSpaces,        //token separator
    tkComment        //to be filtered out in tokenizer routines
};


    /*!
    \brief The lexical analyzer decompose the input into tokens. Class LdpToken represents a token
    */
    //----------------------------------------------------------------------------------------------
    class LdpToken
    {
    private:
        ETokenType m_type;
        std::string m_value;
        int m_numLine;

    public:
        LdpToken(ETokenType type, std::string value, int numLine)
            : m_type(type), m_value(value), m_numLine(numLine) {}
        LdpToken(ETokenType type, char value, int numLine)
            : m_type(type), m_value(""), m_numLine(numLine) { m_value += value; }

        ~LdpToken() {}

        inline ETokenType get_type() { return m_type; }
        inline const std::string& get_value() { return m_value; }
        inline int get_line_number() { return m_numLine; }
    };

    /*!
    \brief implements the lexical analyzer
    */
    //----------------------------------------------------------------------------------------------
    class LdpTokenizer
    {
    public:
        LdpTokenizer(LdpReader& reader, ostream& reporter);
        ~LdpTokenizer();

        inline void repeat_token() { m_repeatToken = true; }
        LdpToken* read_token();
        int get_line_number();
        void skip_utf_bom();

    private:
        LdpToken* parse_new_token();
        char get_next_char();
        static bool is_number(char ch);
        static bool is_letter(char ch);

        LdpReader&  m_reader;
        ostream&    m_reporter;
        bool        m_repeatToken;
        LdpToken*   m_pToken;

        //to deal with compact notation [  name:value  -->  (name value)  ]
        bool        m_expectingEndOfElement;
        bool        m_expectingValuePart;
        bool        m_expectingNamePart;
        LdpToken*   m_pTokenNamePart;
    };


} //namespace lomse

#endif      //__LOMSE_LDP_TOKEN_H__

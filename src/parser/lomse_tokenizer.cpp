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

#include "lomse_tokenizer.h"

#include "lomse_reader.h"
#include "lomse_logger.h"

#include <sstream>
#include <stdexcept>
using namespace std;

namespace lomse
{


//constants for comparations
const char chCR = '\x0d';
const char chLF = '\x0a';
const char chApostrophe = '\'';
const char chAsterisk = '*';
const char chBar = '|';
const char chCloseBracket = ']';
const char chCloseParenthesis = ')';
const char chColon = ':';
const char chComma = ',';
//const char chDollar = '$';
const char chDot = '.';
const char chEqualSign = '=';
//const char chGreaterSign = '>';
//const char chLowerSign = '<';
const char chMinusSign = '-';
const char chOpenBracket = '[';
const char chOpenParenthesis = '(';
const char chPlusSign = '+';
const char chQuotes = '"';
const char chSharp = '#';
const char chSlash = '/';
const char chSpace = ' ';
const char chTab = '\t';
const char chUnderscore = '_';


const char nEOF = EOF;         //End Of File


//---------------------------------------------------------------------------------------
// Implementation of class LdpTokenizer
//---------------------------------------------------------------------------------------

//These methods perform an analysis at character level to form tokens.
//If during the analysis the char sequence "//" is found the remaining chars until
//end of line are ignoerd, including both "//" chars. An then analyisis continues as
//if all those ignored chars never existed.

LdpTokenizer::LdpTokenizer(LdpReader& reader, ostream& reporter)
    : m_reader(reader)
    , m_reporter(reporter)
    , m_repeatToken(false)
    , m_pToken(nullptr)
    //to deal with compact notation [ name:value --> (name value) ]
    , m_expectingEndOfElement(false)
    , m_expectingValuePart(false)
    , m_expectingNamePart(false)
{
}

//---------------------------------------------------------------------------------------
LdpTokenizer::~LdpTokenizer()
{
    delete m_pToken;
}

//---------------------------------------------------------------------------------------
void LdpTokenizer::skip_utf_bom()
{
    char curChar = get_next_char();
    if (curChar == '\xef')
    {
        curChar = get_next_char();  // 0xbb
        curChar = get_next_char();  // 0xbf
    }
    else
        m_reader.repeat_last_char();
}

//---------------------------------------------------------------------------------------
LdpToken* LdpTokenizer::read_token()
{

    if (m_repeatToken)
    {
        m_repeatToken = false;
        return m_pToken;
    }

    int numLine = 0;
    if (m_pToken)
    {
        numLine = m_pToken->get_line_number();
        delete m_pToken;
    }

    // To deal with compact notation [ name:value --> (name value) ]
    if (m_expectingEndOfElement)
    {
        // when flag 'm_expectingEndOfElement' is set it implies that the 'value' part was
        // the last returned token. Therefore, the next token to return is an implicit ')'
        m_expectingEndOfElement = false;
        m_pToken = LOMSE_NEW LdpToken(tkEndOfElement, chCloseParenthesis, numLine);
        return m_pToken;
    }
    if (m_expectingNamePart)
    {
        // when flag 'm_expectingNamePart' is set this implies that last returned token
        // was an implicit '(' and that the real token (the 'name' part of an element
        // written in compact notation) is pending and must be returned now
        m_expectingNamePart = false;
        m_expectingValuePart = true;
        m_pToken = m_pTokenNamePart;
        return m_pToken;
    }
    if (m_expectingValuePart)
    {
        //next token is the 'value' part. Set flag to indicate that after the value
        //part an implicit 'end of element' must be issued
        m_expectingValuePart = false;
        m_expectingEndOfElement = true;
    }

    // loop until a token is found
    while(true)
    {
        if (m_reader.end_of_data())
        {
            m_pToken = LOMSE_NEW LdpToken(tkEndOfFile, "", m_reader.get_line_number());
            return m_pToken;
        }

        m_pToken = parse_new_token();

        //filter out tokens of type 'spaces' and 'comment' to optimize.
        if (m_pToken->get_type() == tkSpaces || m_pToken->get_type() == tkComment)
            delete m_pToken;
        else
            return m_pToken;
    }

}

//---------------------------------------------------------------------------------------
LdpToken* LdpTokenizer::parse_new_token()
{
    //Finite automata for parsing LDP tokens

    enum EAutomataState {
        k_Start,
        k_CMT01,
        k_CMT02,
        k_CMT03,
        k_CMT04,
        k_ETQ01,
        k_ETQ02,
        k_ETQ03,
        k_NUM01,
        k_NUM02,
        k_SPC01,
        k_STR00,
        k_STR01,
        k_STR02,
        k_STR03,
        k_STR04,
        k_S01,
        k_Error
    };

    EAutomataState state = k_Start;
    stringstream tokendata;
    char curChar = 0;
    int numLine = 0;

    while (true)
    {
        switch(state)
        {
            case k_Start:
                curChar = get_next_char();
                numLine = m_reader.get_line_number();
                if (is_letter(curChar)
                    || curChar == chOpenBracket
                    || curChar == chBar
                    || curChar == chColon
                    || curChar == chAsterisk
                    || curChar == chSharp )
                {
                    state = k_ETQ01;
                }
                else if (curChar == chApostrophe) {
                    state = k_ETQ02;
                }
                else if (curChar == chUnderscore) {
                    state = k_ETQ03;
                }
                else if (is_number(curChar)) {
                    state = k_NUM01;
                }
                else {
                    switch (curChar)
                    {
                        case chOpenParenthesis:
                            return LOMSE_NEW LdpToken(tkStartOfElement, chOpenParenthesis, numLine);
                        case chCloseParenthesis:
                            return LOMSE_NEW LdpToken(tkEndOfElement, chCloseParenthesis, numLine);
                        case chSpace:
                            state = k_SPC01;
                            break;
                        case chSlash:
                            state = k_CMT01;
                            break;
                        case chPlusSign:
                        case chMinusSign:
                        case chEqualSign:
                            state = k_S01;
                            break;
                        case chQuotes:
                            state = k_STR00;
                            break;
                        case chApostrophe:
                            state = k_STR02;
                            break;
                        case nEOF:
                            return LOMSE_NEW LdpToken(tkEndOfFile, "", numLine);
                        case chLF:
                            return LOMSE_NEW LdpToken(tkSpaces, chSpace, numLine);
                        case chComma:
                            state = k_Error;
                            break;
                        default:
                            state = k_Error;
                    }
                }
                break;

            case k_ETQ01:
                tokendata << curChar;
                curChar = get_next_char();
                if (is_letter(curChar) || is_number(curChar) ||
                    curChar == chUnderscore || curChar == chDot ||
                    curChar == chPlusSign || curChar == chMinusSign ||
                    curChar == chSharp || curChar == chSlash ||
                    curChar == chEqualSign || curChar == chApostrophe ||
                    curChar == chCloseBracket || curChar == chBar )
                {
                    state = k_ETQ01;
                }
                else if (curChar == chColon) {
                    // compact notation [ name:value --> (name value) ]
                    // 'name' part is parsed and we've found the ':' sign
                    m_expectingNamePart = true;
                    m_pTokenNamePart = LOMSE_NEW LdpToken(tkLabel, tokendata.str(), numLine);
                    return LOMSE_NEW LdpToken(tkStartOfElement, chOpenParenthesis, numLine);
                }
                else {
                    m_reader.repeat_last_char();
                    return LOMSE_NEW LdpToken(tkLabel, tokendata.str(), numLine);
                }
                break;

            case k_ETQ02:
                curChar = get_next_char();
                if (curChar == chApostrophe) {
                    state = k_STR04;
                } else {
                    state = k_ETQ01;
                }
                break;

            case k_ETQ03:
                curChar = get_next_char();
                if (curChar == chApostrophe)
                    state = k_ETQ02;
                else if (curChar == chQuotes)
                    state = k_STR00;
                else
                    state = k_Error;
                break;

            case k_STR00:
                curChar = get_next_char();
                if (curChar == chQuotes) {
                    return LOMSE_NEW LdpToken(tkString, tokendata.str(), numLine);
                } else {
                    if (curChar == nEOF) {
                        state = k_Error;
                    } else {
                        state = k_STR01;
                    }
                }
                break;

            case k_STR01:
                tokendata << curChar;
                curChar = get_next_char();
                if (curChar == chQuotes) {
                    return LOMSE_NEW LdpToken(tkString, tokendata.str(), numLine);
                } else {
                    if (curChar == nEOF) {
                        state = k_Error;
                    } else {
                        state = k_STR01;
                    }
                }
                break;

            case k_STR04:
                curChar = get_next_char();
                if (curChar == nEOF) {
                    state = k_Error;
                } else {
                    state = k_STR02;
                }
                break;

            case k_STR02:
                tokendata << curChar;
                curChar = get_next_char();
                if (curChar == chApostrophe) {
                    state = k_STR03;
                } else {
                    state = k_STR02;
                }
                break;

            case k_STR03:
                curChar = get_next_char();
                if (curChar == chApostrophe) {
                    return LOMSE_NEW LdpToken(tkString, tokendata.str(), numLine);
                } else {
                    state = k_STR02;
                }
                break;

            case k_CMT01:
                tokendata << curChar;
                curChar = get_next_char();
                if (curChar == chSlash)
                    state = k_CMT02;
                else if (curChar == chAsterisk)
                    state = k_CMT03;
                else
                    state = k_Error;
                break;

            case k_CMT02:
                tokendata << curChar;
                curChar = get_next_char();
                if (curChar == chLF || curChar == nEOF) {
                    return LOMSE_NEW LdpToken(tkComment, tokendata.str(), numLine);
                }
                //else continue in this state
                break;

            case k_CMT03:
                tokendata << curChar;
                curChar = get_next_char();
                if (curChar == chAsterisk || curChar == nEOF) {
                    state = k_CMT04;
                }
                //else continue in this state
                break;

            case k_CMT04:
                tokendata << curChar;
                curChar = get_next_char();
                if (curChar == chSlash || curChar == nEOF) {
                    tokendata << curChar;
                    return LOMSE_NEW LdpToken(tkComment, tokendata.str(), numLine);
                }
                else
                    state = k_CMT03;
                break;

            case k_NUM01:
                tokendata << curChar;
                curChar = get_next_char();
                if (is_number(curChar)) {
                    state = k_NUM01;
                } else if (curChar == chDot) {
                    state = k_NUM02;
                } else if (is_letter(curChar) || curChar == chUnderscore) {
                    state = k_ETQ01;
                } else {
                    m_reader.repeat_last_char();
                    return LOMSE_NEW LdpToken(tkIntegerNumber, tokendata.str(), numLine);
                }
                break;

            case k_NUM02:
                tokendata << curChar;
                curChar = get_next_char();
                if (is_number(curChar)) {
                    state = k_NUM02;
                } else {
                    m_reader.repeat_last_char();
                    return LOMSE_NEW LdpToken(tkRealNumber, tokendata.str(), numLine);
                }
                break;

            case k_SPC01:
                curChar = get_next_char();
                if (curChar == chSpace || curChar == chTab) {
                    state = k_SPC01;
                } else {
                    m_reader.repeat_last_char();
                    return LOMSE_NEW LdpToken(tkSpaces, chSpace, numLine);
                }
                break;

            case k_S01:
                tokendata << curChar;
                curChar = get_next_char();
                if (curChar == chSpace || curChar == chTab) {
                    return LOMSE_NEW LdpToken(tkLabel, tokendata.str(), numLine);
                }
                else if (curChar == chCloseParenthesis)
                {
                    m_reader.repeat_last_char();
                    return LOMSE_NEW LdpToken(tkLabel, tokendata.str(), numLine);
                }
                else if (is_number(curChar)) {
                    state = k_NUM01;
                }
                else {
                    state = k_ETQ01;
                }
                break;

            case k_Error:
                if (curChar == nEOF)
                {
                    return LOMSE_NEW LdpToken(tkEndOfFile, "", numLine);
                }
                else
                {
                    stringstream s;
                    s << "[LdpTokenizer::parse_new_token]: Bad character '"
                      << curChar << "' found" << endl;
                    m_reporter << s.str();
                    LOMSE_LOG_ERROR(s.str());
                    throw runtime_error(s.str());
                }
                state = k_Start;
                break;

            default:
            {
                LOMSE_LOG_ERROR("[LdpTokenizer::parse_new_token] Invalid state");
                throw runtime_error("[LdpTokenizer::parse_new_token] Invalid state");
            }

        }
    }
}

//---------------------------------------------------------------------------------------
char LdpTokenizer::get_next_char()
{
    char ch = m_reader.get_next_char();
    if (ch == chTab || ch == chCR)
        return ' ';
    else
        return ch;
}

//---------------------------------------------------------------------------------------
bool LdpTokenizer::is_letter(char ch)
{
    static const std::string letters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    return (letters.find(ch) != string::npos);
}

//---------------------------------------------------------------------------------------
bool LdpTokenizer::is_number(char ch)
{
    static const std::string numbers("0123456789");
    return (numbers.find(ch) != string::npos);
}

//---------------------------------------------------------------------------------------
int LdpTokenizer::get_line_number()
{
    return m_reader.get_line_number();
}


}  //namespace lomse


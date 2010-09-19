//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//  
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#include "lomse_tokenizer.h"

#include "lomse_reader.h"

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
const char chDollar = '$';
const char chDot = '.';
const char chEqualSign = '=';
const char chGreaterSign = '>';
const char chLowerSign = '<';
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


//-------------------------------------------------------------------------------------------
// Implementation of class LdpTokenizer
//-------------------------------------------------------------------------------------------

//These methods perform an analysis at character level to form tokens.
//If during the analyisis the char sequence "//" is found the remaining chars until
//end of line are ignoerd, including both "//" chars. An then analyisis continues as
//if all those ignored chars never existed.

LdpTokenizer::LdpTokenizer(LdpReader& reader, ostream& reporter)
    : m_reader(reader)
    , m_reporter(reporter)
    , m_repeatToken(false)
    , m_pToken(NULL)
    //to deal with compact notation [ name:value --> (name value) ]
    , m_expectingEndOfElement(false)
    , m_expectingValuePart(false)
    , m_expectingNamePart(false)
{
}

LdpTokenizer::~LdpTokenizer()
{
    if (m_pToken)
        delete m_pToken;
}

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
        m_pToken = new LdpToken(tkEndOfElement, chCloseParenthesis, numLine);
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
            m_pToken = new LdpToken(tkEndOfFile, "", m_reader.get_line_number());
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

LdpToken* LdpTokenizer::parse_new_token()
{
    //Finite automata for parsing LDP tokens

    enum EAutomataState {
        k_Start,
        k_CMT01,
        k_CMT02,
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
        k_S02,
        k_S03,
        k_Error
    };

    EAutomataState state = k_Start;
    stringstream tokendata;
    char curChar;
    int numLine;

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
                            return new LdpToken(tkStartOfElement, chOpenParenthesis, numLine);
                        case chCloseParenthesis:
                            return new LdpToken(tkEndOfElement, chCloseParenthesis, numLine);
                        case chSpace:
                            state = k_SPC01;
                            break;
                        case chSlash:
                            state = k_CMT01;
                            break;
                        case chPlusSign:
                        case chMinusSign:
                            state = k_S01;
                            break;
                        case chEqualSign:
                            state = k_S02;
                            break;
                        case chQuotes:
                            state = k_STR00;
                            break;
                        case chApostrophe:
                            state = k_STR02;
                            break;
                        case nEOF:
                            return new LdpToken(tkEndOfFile, "", numLine);
                        case chLF:
                            return new LdpToken(tkSpaces, chSpace, numLine);
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
                    m_pTokenNamePart = new LdpToken(tkLabel, tokendata.str(), numLine);
                    return new LdpToken(tkStartOfElement, chOpenParenthesis, numLine);
                }
                else {
                    m_reader.repeat_last_char();
                    return new LdpToken(tkLabel, tokendata.str(), numLine);
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
                    return new LdpToken(tkString, tokendata.str(), numLine);
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
                    return new LdpToken(tkString, tokendata.str(), numLine);
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
                    return new LdpToken(tkString, tokendata.str(), numLine);
                } else {
                    state = k_STR02;
                }
                break;

            case k_CMT01:
                tokendata << curChar;
                curChar = get_next_char();
                if (curChar == chSlash)
                    state = k_CMT02;
                else
                    state = k_Error;
                break;

            case k_CMT02:
                tokendata << curChar;
                curChar = get_next_char();
                if (curChar == chLF) {
                    return new LdpToken(tkComment, tokendata.str(), numLine);
                }
                //else continue in this state
                break;

            case k_NUM01:
                tokendata << curChar;
                curChar = get_next_char();
                if (is_number(curChar)) {
                    state = k_NUM01;
                } else if (curChar == chDot) {
                    state = k_NUM02;
                } else if (is_letter(curChar)) {
                    state = k_ETQ01;
                } else {
                    m_reader.repeat_last_char();
                    return new LdpToken(tkIntegerNumber, tokendata.str(), numLine);
                }
                break;

            case k_NUM02:
                tokendata << curChar;
                curChar = get_next_char();
                if (is_number(curChar)) {
                    state = k_NUM02;
                } else {
                    m_reader.repeat_last_char();
                    return new LdpToken(tkRealNumber, tokendata.str(), numLine);
                }
                break;

            case k_SPC01:
                curChar = get_next_char();
                if (curChar == chSpace || curChar == chTab) {
                    state = k_SPC01;
                } else {
                    m_reader.repeat_last_char();
                    return new LdpToken(tkSpaces, chSpace, numLine);
                }
                break;

            case k_S01:
                tokendata << curChar;
                curChar = get_next_char();
                if (is_letter(curChar)) {
                    state = k_ETQ01;
                } else if (is_number(curChar)) {
                    state = k_NUM01;
                } else if (curChar == chPlusSign || curChar == chMinusSign) {
                    state = k_S03;
                } else if (curChar == chSpace || curChar == chTab) {
                    return new LdpToken(tkLabel, tokendata.str(), numLine);
                }
                else if (curChar == chCloseParenthesis)
                {
                    m_reader.repeat_last_char();
                    return new LdpToken(tkLabel, tokendata.str(), numLine);
                }
                else {
                    state = k_Error;
                }
                break;

            case k_S02:
                tokendata << curChar;
                curChar = get_next_char();
                if (is_letter(curChar)) {
                    state = k_ETQ01;
                } else if (is_number(curChar)) {
                    state = k_NUM01;
                } else if (curChar == chPlusSign || curChar == chMinusSign) {
                    state = k_S03;
                } else {
                    state = k_Error;
                }
                break;

            case k_S03:
                tokendata << curChar;
                curChar = get_next_char();
                if (is_letter(curChar)) {
                    state = k_ETQ01;
                } else if (is_number(curChar)) {
                    state = k_NUM01;
                } else {
                    state = k_Error;
                }
                break;

            case k_Error:
                if (curChar == nEOF) {
                    return new LdpToken(tkEndOfFile, "", numLine);
                } else {
                    m_reporter << "[LdpTokenizer::parse_new_token]: Bad character '"
                               << curChar << "' found" << endl;
                throw "Invalid char";
                }
                state = k_Start;
                break;

            default:
                throw "Invalid state";

        }
    }
}

char LdpTokenizer::get_next_char()
{
    char ch = m_reader.get_next_char();
    if (ch == chTab || ch == chCR)
        return ' ';
    else
        return ch;
}

bool LdpTokenizer::is_letter(char ch)
{
    static const std::string letters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    return (letters.find(ch) != string::npos);
}

bool LdpTokenizer::is_number(char ch)
{
    static const std::string numbers("0123456789");
    return (numbers.find(ch) != string::npos);
}

int LdpTokenizer::get_line_number()
{
    return m_reader.get_line_number();
}


}  //namespace lomse


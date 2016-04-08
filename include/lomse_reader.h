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

#ifndef __LOMSE_LDP_READER_H__
#define __LOMSE_LDP_READER_H__

#include "lomse_build_options.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

namespace lomse
{

//forward declarations
class InputStream;

//---------------------------------------------------------------------------------------
// LdpReader: Base class for any provider of LDP source code to be parsed
class LdpReader
{
public:
    LdpReader() {}
    virtual ~LdpReader() {}

    // Returns the next char from the source
    virtual char get_next_char()=0;
    // Instruct reader to repeat last returned char at next invocation of get_next_char()
    virtual void repeat_last_char()=0;
    // The reader is ready for get_next_char(), unget() operations
    virtual bool is_ready()=0;
    // End of data reached. No more data available
    virtual bool end_of_data()=0;
    // Returns the current line number (the one for char returned in last get_next_char() )
    virtual int get_line_number()=0;
    // Returns the file locator associated to this reader
    virtual string get_locator() = 0;

};


//---------------------------------------------------------------------------------------
// LdpFileReader: An LDP reader using a file as origin of source code
class LdpFileReader : public LdpReader
{
private:
    InputStream* m_file;
    const std::string m_locator;
    int m_numLine;
    bool m_repeating_last_char;

public:
    LdpFileReader(const std::string& locator);
    virtual ~LdpFileReader();

    virtual char get_next_char();
    virtual void repeat_last_char();
    virtual bool is_ready();
    virtual bool end_of_data();
    virtual int get_line_number() { return m_numLine; }
    virtual string get_locator() { return m_locator; }

};


//---------------------------------------------------------------------------------------
// LdpTextReader: an LDP reader using a string as origin of source code
class LdpTextReader : public LdpReader
{
public:
    LdpTextReader(const std::string& sourceText);
    virtual ~LdpTextReader() {}

    virtual char get_next_char();
    virtual void repeat_last_char();
    virtual bool is_ready();
    virtual bool end_of_data();
    virtual int get_line_number() { return 0; }
    virtual string get_locator() { return "string:"; }

private:
    stringstream   m_stream;

};


}   //namespace lomse

#endif      //__LOMSE_LDP_READER_H__

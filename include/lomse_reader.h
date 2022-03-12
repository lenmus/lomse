//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
    ~LdpFileReader() override;

    char get_next_char() override;
    void repeat_last_char() override;
    bool is_ready() override;
    bool end_of_data() override;
    int get_line_number() override { return m_numLine; }
    string get_locator() override { return m_locator; }

};


//---------------------------------------------------------------------------------------
// LdpTextReader: an LDP reader using a string as origin of source code
class LdpTextReader : public LdpReader
{
public:
    LdpTextReader(const std::string& sourceText);
    ~LdpTextReader() override {}

    char get_next_char() override;
    void repeat_last_char() override;
    bool is_ready() override;
    bool end_of_data() override;
    int get_line_number() override { return 0; }
    string get_locator() override { return "string:"; }

private:
    stringstream   m_stream;

};


}   //namespace lomse

#endif      //__LOMSE_LDP_READER_H__

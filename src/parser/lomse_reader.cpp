//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_reader.h"

#include "lomse_file_system.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;


namespace lomse
{

//=======================================================================================
// LdpFileReader implementation
//=======================================================================================
LdpFileReader::LdpFileReader(const std::string& filelocator)
    : LdpReader()
    , m_file( FileSystem::open_input_stream(filelocator) )
    , m_locator(filelocator)
    , m_numLine(1)
    , m_repeating_last_char(false)
{
}
//---------------------------------------------------------------------------------------
LdpFileReader::~LdpFileReader()
{
    delete m_file;
}

//---------------------------------------------------------------------------------------
char LdpFileReader::get_next_char()
{
    char ch = m_file->get_char();
    if (!m_repeating_last_char && ch == 0x0a)
        m_numLine++;
    m_repeating_last_char = false;
    return ch;
}

//---------------------------------------------------------------------------------------
void LdpFileReader::repeat_last_char()
{
    m_repeating_last_char = true;
    m_file->unget();
}

//---------------------------------------------------------------------------------------
bool LdpFileReader::is_ready()
{
    return m_file->is_open();
}

//---------------------------------------------------------------------------------------
bool LdpFileReader::end_of_data()
{
    return m_file->eof();
}




//=======================================================================================
// LdpTextReader implementation
//=======================================================================================

LdpTextReader::LdpTextReader(const std::string& sourceText)
    : LdpReader()
    , m_stream(sourceText)
{
}

//---------------------------------------------------------------------------------------
char LdpTextReader::get_next_char()
{
    return m_stream.get();
}

//---------------------------------------------------------------------------------------
void LdpTextReader::repeat_last_char()
{
    m_stream.unget();
}

//---------------------------------------------------------------------------------------
bool LdpTextReader::is_ready()
{
    return true;
}

//---------------------------------------------------------------------------------------
bool LdpTextReader::end_of_data()
{
    return m_stream.peek() == EOF;
}


}  //namespace lomse

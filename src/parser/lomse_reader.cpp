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

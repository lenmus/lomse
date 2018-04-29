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

#include "lomse_file_system.h"
#include "lomse_logger.h"

#if (LOMSE_ENABLE_COMPRESSION == 1)
	#include "lomse_zip_stream.h"
#endif

#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace lomse
{

//=======================================================================================
// DocLocator implementation
//=======================================================================================
DocLocator::DocLocator(const string& locator)
    : m_fullLocator(locator)
    , m_protocol(k_unknown)
    , m_innerProtocol(k_none)
    , m_fullpath("")
    , m_path("")
    , m_innerFullPath("")
    , m_innerFile("")
    , m_fValid(false)
{
    split_locator(locator);
    if (is_valid())
    {
        extract_file();
        extract_path();
    }
}

//---------------------------------------------------------------------------------------
void DocLocator::split_locator(const string& locator)
{
    //protocol
    m_protocol = k_file;
    int pathStart = 0;
    int colon = int( locator.find(':') );
    if (colon >= 0)
    {
        if (locator.substr(0, colon) == "file")
            pathStart = colon+1;
        else if (locator.substr(0, colon) == "string")
        {
            m_protocol = k_string;
            m_fValid = true;
            return;
        }
    }

    //inner protocol & path
    int sharp = int( locator.find('#') );
    if (sharp >= 0)
    {
        m_fullpath = locator.substr(pathStart, sharp - pathStart);
        int iMax = int( locator.length() );
        int i = sharp+1;
        for (; i < iMax && locator[i] != ':'; ++i);
        if (i == iMax)
        {
            m_fValid = false;
            return;
        }
        string proto =  locator.substr(sharp+1, i - sharp - 1);
        m_innerProtocol = (proto == "zip" ? k_zip : k_unknown);
        if (i < iMax - 2)
            m_innerFullPath = locator.substr(i+1);
    }
    else
    {
        //path
        if (pathStart > 0)
            m_fullpath = locator.substr(pathStart);
        else
            m_fullpath = locator;
    }

    m_fValid = true;
}

//---------------------------------------------------------------------------------------
void DocLocator::extract_file()
{
    if (m_innerFullPath.empty())
        return;

    int iMax = int( m_innerFullPath.length() );
    int i = iMax-1;
    for (; i >=0 && m_innerFullPath[i] != '/'; --i);
    if (i >= 0)
        m_innerFile = m_innerFullPath.substr(i+1);
}

//---------------------------------------------------------------------------------------
void DocLocator::extract_path()
{
    int iMax = int( m_fullpath.length() );
    int i = iMax-1;
    for (; i >=0 && !(m_fullpath[i] == '/' || m_fullpath[i] == '\\'); --i);
    if (i >= 0)
        m_path = m_fullpath.substr(0, i+1);
}

//---------------------------------------------------------------------------------------
string DocLocator::get_protocol_string()
{
    switch (m_protocol)
    {
        case k_file:        return "";
        case k_string:      return "string:";
        default:
            return "";
    }
}

//---------------------------------------------------------------------------------------
string DocLocator::get_locator_as_string()
{
    string loc = get_protocol_string() + get_full_path();
    if (m_innerProtocol == k_zip)
        return loc + "#zip:" + get_inner_fullpath();
    else
        return loc;
}

//---------------------------------------------------------------------------------------
string DocLocator::get_locator_for_image(const string& imagename)
{
    if (m_innerProtocol != k_zip)
    {
        //images in the same folder than file referred by this locator
        return get_protocol_string() + get_path() + imagename;

//        //remove lms file from path
//        int iMax = int( m_fullpath.length() );
//        int i = iMax-1;
//        for (; i >=0 && !(m_fullpath[i] == '/' || m_fullpath[i] == '\\'); --i);
//        if (i >= 0)
//            return get_protocol_string() + m_fullpath.substr(0, i+1) + imagename;
    }
    else
    {
        //zip file. It is assumed that image is in the inner root folder.
        //Otherwise, for specific format using zip compression, you will
        //need to override this method
        return get_protocol_string() + get_full_path() + "#zip:" + imagename;
    }
}



//=======================================================================================
// LmbDocLocator implementation
//=======================================================================================
string LmbDocLocator::get_locator_for_image(const string& imagename)
{
    return get_protocol_string() + get_full_path() + "#zip:images/" + imagename;
}


//=======================================================================================
// FileSystem implementation
//=======================================================================================
InputStream* FileSystem::open_input_stream(const string& filelocator)
{
    //factory method to create InputStream objects

    DocLocator loc(filelocator);
    switch( loc.get_protocol() )
    {
        case DocLocator::k_file:
        {
            switch( loc.get_inner_protocol() )
            {
                case DocLocator::k_none:
                    return LOMSE_NEW LocalInputStream(filelocator);
#if (LOMSE_ENABLE_COMPRESSION == 1)
                case DocLocator::k_zip:
                    return LOMSE_NEW ZipInputStream(filelocator);
#endif
                default:
                {
                    LOMSE_LOG_ERROR("Invalid file locator protocol");
                    throw runtime_error("[FileSystem::open_input_stream] Invalid file locator protocol");
                }
            }
        }

        default:
        {
            LOMSE_LOG_ERROR("Invalid file locator protocol");
            throw runtime_error("[FileSystem::open_input_stream] Invalid file locator protocol");
        }
    }
    return nullptr;    //compiler happy
}


//=======================================================================================
// LocalInputStream implementation
//=======================================================================================
LocalInputStream::LocalInputStream(const std::string& filelocator)
    : InputStream()
    , m_file(filelocator.c_str(), ios::in | ios::binary)
{
    if(!m_file.is_open())
    {
        stringstream s;
        s << "[LocalInputStream::LocalInputStream] File not found: \""
          << filelocator << "\"";
        LOMSE_LOG_ERROR(s.str());
        throw runtime_error(s.str());
    }
}

//---------------------------------------------------------------------------------------
char LocalInputStream::get_char()
{
    return m_file.get();
}

//---------------------------------------------------------------------------------------
void LocalInputStream::unget()
{
    m_file.unget();
}

//---------------------------------------------------------------------------------------
bool LocalInputStream::is_open()
{
    return m_file.is_open();
}

//---------------------------------------------------------------------------------------
bool LocalInputStream::eof()
{
    return m_file.eof();
}

//---------------------------------------------------------------------------------------
long LocalInputStream::read (unsigned char* pDestBuffer, long nBytesToRead)
{
    //reads the specified number of bytes from the stream into the dest. buffer.
    //Invoker should allocate a buffer large enough, as this method does not
    //do any checks
    //Returns the actual number of bytes that were read. It might be lower than the
    //requested number of bites if the end of stream is reached.

    m_file.read((char*)pDestBuffer, nBytesToRead);
    return m_file.gcount();
}


}  //namespace lomse

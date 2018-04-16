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

#include "lomse_zip_stream.h"

#include "lomse_logger.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <algorithm> // min()
using namespace std;

namespace lomse
{

//=======================================================================================
// ZipInputStream implementation
//=======================================================================================
ZipInputStream::ZipInputStream(const std::string& filelocator)
    : InputStream()
    , m_fIsLastBuffer(true)
    , m_remainingBytes(0)
{
    if (!open_zip_archive(filelocator))
    {
        stringstream s;
        s << "[ZipInputStream::ZipInputStream] File not found: \""
          << filelocator << "\"";
        LOMSE_LOG_ERROR(s.str());
        throw runtime_error(s.str());
    }

    if (get_num_entries() != 0)
        open_specified_entry_or_first(filelocator);
    else
        m_curEntry.fEOF = true;
}

//---------------------------------------------------------------------------------------
ZipInputStream::~ZipInputStream()
{
    close_zip_archive();
}

//---------------------------------------------------------------------------------------
bool ZipInputStream::open_zip_archive(const std::string& filelocator)
{
    DocLocator loc(filelocator);
    string path = loc.get_full_path();
    if (path.empty())
        return false;

    m_uzFile = unzOpen(path.c_str());
    return (m_uzFile != nullptr);
}

//---------------------------------------------------------------------------------------
void ZipInputStream::close_zip_archive()
{
    if (m_uzFile)
    {
        close_current_entry();
        unzClose(m_uzFile);
        m_uzFile = nullptr;
    }
    m_curEntry.fEOF = true;
}

//---------------------------------------------------------------------------------------
int ZipInputStream::get_num_entries()
{
    unz_global_info info;

    if (unzGetGlobalInfo(m_uzFile, &info) == UNZ_OK)
        return (int)info.number_entry;

    return 0;
}

//---------------------------------------------------------------------------------------
void ZipInputStream::open_specified_entry_or_first(const std::string& filelocator)
{
    DocLocator loc(filelocator);
    string innerPath = loc.get_inner_fullpath();
    if (innerPath.empty())
    {
        if (!move_to_first_entry())
        {
            stringstream s;
            s << "[ZipInputStream::open_specified_entry_or_first] Failure positioning at first entry in zip archive: \"" << filelocator << "\"";
            LOMSE_LOG_ERROR(s.str());
            throw runtime_error(s.str());
        }
    }
    else
    {
        if (!move_to_entry(innerPath))
        {
            stringstream s;
            s << "[ZipInputStream::open_specified_entry_or_first] Failure positioning at entry \"" << innerPath
              << "\" in zip archive: \"" << filelocator << "\"";
            LOMSE_LOG_ERROR(s.str());
            throw runtime_error(s.str());
        }
    }

    open_current_entry();
}

//---------------------------------------------------------------------------------------
bool ZipInputStream::move_to_first_entry()
{
    close_current_entry();

    bool fSuccess = (unzGoToFirstFile(m_uzFile) == UNZ_OK);
    return set_up_current_entry(fSuccess);
}

//---------------------------------------------------------------------------------------
bool ZipInputStream::move_to_next_entry()
{
    close_current_entry();

    bool fSuccess = (unzGoToNextFile(m_uzFile) == UNZ_OK);
    return set_up_current_entry(fSuccess);
}

//---------------------------------------------------------------------------------------
bool ZipInputStream::move_to_entry(const std::string& innerPath)
{
    close_current_entry();

    bool fSuccess = (unzLocateFile(m_uzFile, innerPath.c_str(), 2) == UNZ_OK);
    return set_up_current_entry(fSuccess);
}

//---------------------------------------------------------------------------------------
bool ZipInputStream::set_up_current_entry(bool fSuccess)
{
    if (fSuccess)
        get_current_entry_info(m_curEntry);
    else
        m_curEntry.fIsValid = false;

    m_curEntry.fIsOpen = false;
    m_curEntry.fEOF = true;
    return fSuccess;
}

//---------------------------------------------------------------------------------------
bool ZipInputStream::get_current_entry_info(ZipEntryInfo& info)
{
    unz_file_info uzfi;

    char comment[256];
    char filename[256];

    if (UNZ_OK != unzGetCurrentFileInfo(m_uzFile, &uzfi, filename,
                                        255, nullptr, 0, comment, 255))
        return false;

    // copy across
    info.dwVersion = uzfi.version;
    info.dwVersionNeeded = uzfi.version_needed;
    info.dwFlags = uzfi.flag;
    info.dwCompressionMethod = uzfi.compression_method;
    info.dwDosDate = uzfi.dosDate;
    info.dwCRC = uzfi.crc;
    info.dwCompressedSize = uzfi.compressed_size;
    info.dwUncompressedSize = uzfi.uncompressed_size;
    info.dwInternalAttrib = uzfi.internal_fa;
    info.dwExternalAttrib = uzfi.external_fa;

    info.filename = string(filename);
    info.comment = string(comment);

    // is it a folder?
    info.bFolder = ((info.dwExternalAttrib & k_zip_subdir) == k_zip_subdir);

    //other data
    info.fIsValid = true;
	info.fIsOpen = false;
    info.fEOF = true;

    return true;
}

//---------------------------------------------------------------------------------------
bool ZipInputStream::read_buffer()
{
    //returns false if no buffer read or no more data

    if (m_fIsLastBuffer)
        return false;

    m_remainingBytes = unzReadCurrentFile(m_uzFile, m_buffer, k_buffersize);
    m_pNextChar = m_buffer;
    m_fIsLastBuffer = (m_remainingBytes < k_buffersize);

    m_curEntry.fEOF = (m_remainingBytes <= 0);
    if (m_remainingBytes < 0)
        m_remainingBytes = 0;
    return (m_remainingBytes > 0);
}

//---------------------------------------------------------------------------------------
bool ZipInputStream::open_current_entry()
{
    //Open for reading data the current file entry in the zip archive.
    //If there is no error and the file is opened, returns true.

    if (m_curEntry.fIsValid && m_curEntry.fIsOpen)
        return true;

    m_remainingBytes = 0;

    if (!m_curEntry.fIsValid)
        return false;

    m_curEntry.fIsOpen = (unzOpenCurrentFile(m_uzFile) == UNZ_OK);

    m_curEntry.fEOF = !m_curEntry.fIsOpen;
    if (m_curEntry.fIsOpen)
    {
        m_fIsLastBuffer = false;
        read_buffer();
    }

    return m_curEntry.fIsOpen;
}

//---------------------------------------------------------------------------------------
void ZipInputStream::close_current_entry()
{
    if (m_curEntry.fIsValid && m_curEntry.fIsOpen)
        unzCloseCurrentFile(m_uzFile);

    m_curEntry.fIsOpen = false;
    m_curEntry.fEOF = true;
}

//---------------------------------------------------------------------------------------
void ZipInputStream::unget()
{
    if (!m_curEntry.fIsOpen)
    {
        LOMSE_LOG_ERROR("[ZipInputStream::unget] Invoking unget() in closed ZipInputStream entry");
        throw logic_error("[ZipInputStream::unget] Invoking unget() in closed ZipInputStream entry");
    }

    if (m_pNextChar > m_buffer)
    {
        m_pNextChar--;
        m_remainingBytes++;
        m_curEntry.fEOF = false;
    }
}

//---------------------------------------------------------------------------------------
bool ZipInputStream::is_open()
{
    return m_curEntry.fIsOpen;
}

//---------------------------------------------------------------------------------------
bool ZipInputStream::eof()
{
    return m_curEntry.fEOF;
}

//---------------------------------------------------------------------------------------
char ZipInputStream::get_char()
{
    if (!m_curEntry.fIsOpen)
    {
        LOMSE_LOG_ERROR("[ZipInputStream::get_char] Invoking get_char() in closed ZipInputStream entry");
        throw logic_error("[ZipInputStream::get_char] Invoking get_char() in closed ZipInputStream entry");
    }

    if (m_curEntry.fEOF)
    {
        LOMSE_LOG_ERROR("[ZipInputStream::get_char] Invoking get_char() after eof in ZipInputStream entry");
        throw logic_error("[ZipInputStream::get_char] Invoking get_char() after eof in ZipInputStream entry");
    }

    if (m_remainingBytes <= 0)
    {
        LOMSE_LOG_ERROR("[ZipInputStream::get_char] Invoking get_char() with empty buffer in ZipInputStream entry");
        throw logic_error("[ZipInputStream::get_char] Invoking get_char() with empty buffer in ZipInputStream entry");
    }

    if (m_remainingBytes == 1)
    {
        char ret = *(m_pNextChar++);
        m_remainingBytes = 0;

        if (m_fIsLastBuffer)
            m_curEntry.fEOF = true;
        else
        {
            read_buffer();
            if (m_fIsLastBuffer && m_remainingBytes == 0)
                m_curEntry.fEOF = true;
        }
        return ret;
    }
    else
    {
        m_remainingBytes--;
        return *(m_pNextChar++);
    }
}

//---------------------------------------------------------------------------------------
long ZipInputStream::read(unsigned char* pDestBuffer, long nBytesToRead)
{
    if (!m_curEntry.fIsOpen)
    {
        LOMSE_LOG_ERROR("[ZipInputStream::read] Invoking read() in closed ZipInputStream entry");
        throw logic_error("[ZipInputStream::read] Invoking read() in closed ZipInputStream entry");
    }

    if (m_curEntry.fEOF)
    {
        LOMSE_LOG_ERROR("[ZipInputStream::read] Invoking read() after eof in ZipInputStream entry");
        throw logic_error("[ZipInputStream::read] Invoking read() after eof in ZipInputStream entry");
    }

    if (m_remainingBytes <= 0)
    {
        LOMSE_LOG_ERROR("[ZipInputStream::read] Invoking read() with empty buffer in ZipInputStream entry");
        throw std::logic_error("[ZipInputStream::read] Invoking read() with empty buffer in ZipInputStream entry");
    }

    long bytesRead = min(m_remainingBytes, nBytesToRead);
    memcpy(pDestBuffer, m_pNextChar, bytesRead);
    m_remainingBytes -= bytesRead;
    m_pNextChar += bytesRead;
    pDestBuffer += bytesRead;

    if (m_remainingBytes == 0)
    {
        if (m_fIsLastBuffer)
            m_curEntry.fEOF = true;
        else
        {
            read_buffer();
            if (m_fIsLastBuffer && m_remainingBytes == 0)
                m_curEntry.fEOF = true;
        }
    }

    if (bytesRead < nBytesToRead && !m_curEntry.fEOF)
        bytesRead += read(pDestBuffer, nBytesToRead - bytesRead);

    return bytesRead;
}

//---------------------------------------------------------------------------------------
long ZipInputStream::get_size()
{
    if (!m_curEntry.fIsOpen)
    {
        LOMSE_LOG_ERROR("[ZipInputStream::get_size] Invoking get_size() in closed ZipInputStream entry");
        throw logic_error("[ZipInputStream::get_size] Invoking get_size() in closed ZipInputStream entry");
    }

    return long(m_curEntry.dwUncompressedSize);
}

//---------------------------------------------------------------------------------------
unsigned char* ZipInputStream::get_as_string()
{
    unsigned char* buffer;
    long size = get_size();
    if ((buffer = LOMSE_NEW unsigned char[size+1]) == nullptr)
    {
        throw std::runtime_error("[ZipInputStream::get_as_string] error allocating memory for zip file");
    }
    else
    {
        long i = read(buffer, size);
        buffer[i] = 0;
    }
    return buffer;
}


}  //namespace lomse

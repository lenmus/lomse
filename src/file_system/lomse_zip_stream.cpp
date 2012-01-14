//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#include "lomse_zip_stream.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
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
        s << "File not found: \"" << filelocator << "\"";
        throw std::invalid_argument(s.str());
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
    string path = loc.get_path();
    if (path.empty())
        return false;

    m_uzFile = unzOpen(path.c_str());
    return (m_uzFile != NULL);
}

//---------------------------------------------------------------------------------------
void ZipInputStream::close_zip_archive()
{
    if (m_uzFile)
    {
        close_current_entry();
        unzClose(m_uzFile);
        m_uzFile = NULL;
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
    string innerPath = loc.get_inner_path();
    if (innerPath.empty())
    {
        if (!move_to_first_entry())
        {
            stringstream s;
            s << "Failure positioning at first entry in zip archive: \"" << filelocator << "\"";
            throw std::runtime_error(s.str());
        }
    }
    else
    {
        if (!move_to_entry(innerPath))
        {
            stringstream s;
            s << "Failure positioning at entry \"" << innerPath
              << "\" in zip archive: \"" << filelocator << "\"";
            throw std::runtime_error(s.str());
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
                                        255, NULL, 0, comment, 255))
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
        throw std::logic_error("Invoking unget() in closed ZipInputStream entry");

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
        throw std::logic_error("Invoking get_char() in closed ZipInputStream entry");

    if (m_curEntry.fEOF)
        throw std::logic_error("Invoking get_char() after eof in ZipInputStream entry");

    if (m_remainingBytes <= 0)
        throw std::logic_error("Invoking get_char() with empty buffer in ZipInputStream entry");

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
int ZipInputStream::read(unsigned char* pDestBuffer, int nBytesToRead)
{
    if (!m_curEntry.fIsOpen)
        throw std::logic_error("Invoking read() in closed ZipInputStream entry");

    if (m_curEntry.fEOF)
        throw std::logic_error("Invoking read() after eof in ZipInputStream entry");

    if (m_remainingBytes <= 0)
        throw std::logic_error("Invoking read() with empty buffer in ZipInputStream entry");

    int bytesRead = min(m_remainingBytes, nBytesToRead);
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


}  //namespace lomse

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#ifndef __LOMSE_ZIP_STREAM_H__
#define __LOMSE_ZIP_STREAM_H__

#include "lomse_build_options.h"
#include "lomse_file_system.h"

#include <fstream>
#include <sstream>
using namespace std;

#include "unzip.h"      //minizip package

namespace lomse
{


//---------------------------------------------------------------------------------------
// Zip uses Dos/Win file attributes
enum ZipAttributes
{
    k_zip_read_only = 0x01,
    k_zip_hidden    = 0x02,
    k_zip_system    = 0x04,
    k_zip_subdir    = 0x10,
    k_zip_modified  = 0x20,    //Archive

    k_zip_mask      = 0x37
};

//---------------------------------------------------------------------------------------
// ZipEntryInfo: contains information about a file in the zip archive
struct ZipEntryInfo
{
	string filename;
	string comment;

	unsigned long dwVersion;
	unsigned long dwVersionNeeded;
	unsigned long dwFlags;
	unsigned long dwCompressionMethod;
	unsigned long dwDosDate;
	unsigned long dwCRC;
	unsigned long dwCompressedSize;
	unsigned long dwUncompressedSize;
	unsigned long dwInternalAttrib;
	unsigned long dwExternalAttrib;
	bool          bFolder;

    //my flags, for controlling read
	bool  fIsValid;     //entry contains valid data
	bool  fIsOpen;      //entry file is open: ready to read
	bool  fEOF;         //End of file reached

	ZipEntryInfo()
        : dwVersion(0L)
        , dwVersionNeeded(0L)
        , dwFlags(0L)
        , dwCompressionMethod(0L)
        , dwDosDate(0L)
        , dwCRC(0L)
        , dwCompressedSize(0L)
        , dwUncompressedSize(0L)
        , dwInternalAttrib(0L)
        , dwExternalAttrib(0L)
        , bFolder(false)
        //
        , fIsValid(false)
        , fIsOpen(false)
        , fEOF(true)
    {
    }
};

//---------------------------------------------------------------------------------------
// ZipInputStream: A stream for reading an entry in a zip file in the local file system
class ZipInputStream : public InputStream
{
protected:
    enum { k_buffersize = 4096, };

    void* m_uzFile;
    bool m_fIsLastBuffer;
    long m_remainingBytes;
    char m_buffer[k_buffersize];
    char* m_pNextChar;
    ZipEntryInfo m_curEntry;


public:
	ZipInputStream(const std::string& filelocator);
	virtual ~ZipInputStream();

    //mandatory overrides inherited from InputStream
    char get_char();
    void unget();
    bool is_open();
    bool eof();
    long read(unsigned char* pDestBuffer, long nBytesToRead);

    //operations
    unsigned char* get_as_string();

	//positioning
	bool move_to_first_entry();
	bool move_to_next_entry();
    bool move_to_entry(const std::string& innerPath);

    //opening files
    bool open_current_entry();

    //info
    int get_num_entries();
    bool get_current_entry_info(ZipEntryInfo& info);

    //current entry info
    long get_size();


protected:
    bool open_zip_archive(const std::string& filelocator);
    void open_specified_entry_or_first(const std::string& filelocator);
    bool read_buffer();
    void close_current_entry();
    void close_zip_archive();
    bool set_up_current_entry(bool fSuccess);

};





}   //namespace lomse

#endif      //__LOMSE_ZIP_STREAM_H__

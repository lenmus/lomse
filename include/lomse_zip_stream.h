//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_ZIP_STREAM_H__
#define __LOMSE_ZIP_STREAM_H__

#include "lomse_build_options.h"
#include "lomse_file_system.h"

#include <fstream>
#include <sstream>
#include <vector>
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
    char get_char() override;
    void unget() override;
    bool is_open() override;
    bool eof() override;
    long read(unsigned char* pDestBuffer, long nBytesToRead) override;

    //operations
    unsigned char* get_as_string();
    std::vector<unsigned char> get_as_vector();

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

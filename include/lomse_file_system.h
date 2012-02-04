//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_FILE_SYSTEM_H__
#define __LOMSE_FILE_SYSTEM_H__

#include "lomse_build_options.h"

#include <fstream>
#include <sstream>
using namespace std;


namespace lomse
{

//forward declarations
class DocLocator;
class FileSystem;
class InputStream;
class LocalInputStream;


//-------------------------------------------------------------------------------------
// DocLocator: understands document locators
class DocLocator
{
protected:
    string m_fullLocator;
    int m_protocol;
    int m_innerProtocol;
    string m_path;
    string m_innerPath;
    string m_innerFile;
    bool m_fValid;

public:
    DocLocator(const string& filelocator);
    virtual ~DocLocator() {}

    enum { k_unknown, k_file, k_zip, k_string, k_none, };    //protocol, inner-protocol

    //info
    inline bool is_valid() { return m_fValid; }
    inline int get_protocol() { return m_protocol; }
    inline const string& get_path() { return m_path; }
    inline int get_inner_protocol() { return m_innerProtocol; }
    inline const string& get_inner_path() { return m_innerPath; }
    inline const string& get_inner_file() { return m_innerFile; }
    string get_locator_string();

    //modify locator
    inline void set_file(const string& filename) { m_innerFile = filename; }


protected:
    void split_locator(const string& filelocator);
    void extract_file();
    string get_protocol_string();

};

//-------------------------------------------------------------------------------------
// LmbDocLocator: a DocLocator that knows the LMB file structure
class LmbDocLocator : public DocLocator
{
protected:

public:
    LmbDocLocator(const string& filelocator) : DocLocator(filelocator) {}
    virtual ~LmbDocLocator() {}

    inline bool is_valid_lmb() {
        return is_valid() && get_inner_protocol() == DocLocator::k_zip;
    }

    //operations
    string get_locator_for_image(const string& imagename);

};

//-------------------------------------------------------------------------------------
// FileSystem: responsible for locating and opening files
class FileSystem
{
public:
    FileSystem() {}
    ~FileSystem() {}

    static InputStream* open_input_stream(const string& filelocator);

};

//-------------------------------------------------------------------------------------
// InputStream: base class for streams that read data from some kind of destination
class InputStream
{
public:
	virtual ~InputStream() {}

    virtual char get_char() = 0;
    virtual void unget() = 0;
    virtual bool is_open() = 0;
    virtual bool eof() = 0;
    virtual int read(unsigned char* pDestBuffer, int nBytesToRead) = 0;

protected:
	InputStream() {}
};


//-------------------------------------------------------------------------------------
// LocalInputStream: A stream for reading a file in the local file system
class LocalInputStream : public InputStream
{
private:
    std::ifstream m_file;

public:
	LocalInputStream(const std::string& filelocator);
	virtual ~LocalInputStream() {}

    char get_char();
    void unget();
    bool is_open();
    bool eof();
    int read(unsigned char* pDestBuffer, int nBytesToRead);
};


}   //namespace lomse


#endif      //__LOMSE_FILE_SYSTEM_H__

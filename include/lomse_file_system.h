//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

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

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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_zip_stream.h"

#include <cstring>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// to have access to protected members
class MyZipInputStream : public ZipInputStream
{
public:
	MyZipInputStream(const std::string& filelocator) : ZipInputStream(filelocator) {}
	virtual ~MyZipInputStream() {}

	bool my_read_buffer() { return read_buffer(); }
    bool my_open_current_entry() { return open_current_entry(); }
    int my_bytes_pending() { return m_remainingBytes; }
    bool my_is_last_buffer() { return m_fIsLastBuffer; }

};

//---------------------------------------------------------------------------------------
class ZipInputStreamTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    ZipInputStreamTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~ZipInputStreamTestFixture()    //TearDown fixture
    {
    }

    bool check_content(const string& data)
    {
        char exp[] = "(lenmusdoc (vers 0.0) \r\n"
                     "    (content \r\n"
                     "        (para (image (file \"test-image-1.png\")) )\r\n"
                     "    )\r\n"
                     ")\r\n\r\n";
        string expected(exp);
        return data == expected;
//        cout << data.length() << endl;
//        for (int i=0; i < 102; ++i)
//            cout << "data=" << hex << (int)(data[i]) << ",  "
//                 << "expected=" << hex << (int)(expected[i]) << endl;
    }
};


SUITE(ZipInputStreamTest)
{

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_can_open_1)
    {
        string path = m_scores_path + "90101-read-png-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        ZipInputStream* zs  = dynamic_cast<ZipInputStream*>(file);
        CHECK( zs != NULL );
        CHECK( zs->eof() == false );
        CHECK( zs->get_num_entries() == 1 );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_can_open_2)
    {
        string path = m_scores_path + "90101-does-not-exits.zip#zip:";
        bool fThrows = false;
        try
        {
            FileSystem::open_input_stream(path);
        }
        catch(...)
        {
            fThrows = true;
        }
        CHECK( fThrows == true );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_can_open_3)
    {
        string path = m_scores_path + "90102-source-and-image.zip#zip:test-image-1.png";
        InputStream* file = FileSystem::open_input_stream(path);
        ZipInputStream* zs  = dynamic_cast<ZipInputStream*>(file);
        ZipEntryInfo info;
        zs->get_current_entry_info(info);
        CHECK( info.filename == "test-image-1.png" );
        CHECK( zs->eof() == false );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_get_file_count_2)
    {
        string path = m_scores_path + "90102-source-and-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        ZipInputStream* zs  = dynamic_cast<ZipInputStream*>(file);
        CHECK( zs != NULL );
        CHECK( zs->eof() == false );
        CHECK( zs->get_num_entries() == 2 );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_get_file_info_1)
    {
        string path = m_scores_path + "90101-read-png-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        ZipInputStream* zs  = dynamic_cast<ZipInputStream*>(file);
        ZipEntryInfo info;
        zs->get_current_entry_info(info);

//        cout << "filename='" << info.filename << "', " << endl;
//        cout << "version='" << info.dwVersion << "', " << endl;
//        cout << "version needed='" << info.dwVersionNeeded << "', "  << endl;
//        cout << "flags='" << info.dwFlags << "', "  << endl;
//        cout << "compresion method='" << info.dwCompressionMethod << "', "  << endl;
//        cout << "DOS date='" << info.dwDosDate << "', "  << endl;
//        cout << "crc='" << info.dwCRC << "', "  << endl;
//        cout << "compressed size='" << info.dwCompressedSize << "', "  << endl;
//        cout << "uncompressed size='" << info.dwUncompressedSize << "', "  << endl;
//        cout << "internal attrib='" << info.dwInternalAttrib << "', "  << endl;
//        cout << "external attrib='" << info.dwExternalAttrib << "', "  << endl;
//        cout << "is folder='" << info.bFolder << endl;

        CHECK( info.filename == "90101-read-png-image.lms" );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_move_to_next_entry_1)
    {
        string path = m_scores_path + "90102-source-and-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        ZipInputStream* zs  = dynamic_cast<ZipInputStream*>(file);
        ZipEntryInfo info;
        zs->get_current_entry_info(info);
        CHECK( info.filename == "90101-read-png-image.lms" );
        zs->move_to_next_entry();
        zs->get_current_entry_info(info);
        CHECK( info.filename == "test-image-1.png" );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_move_to_entry_1)
    {
        string path = m_scores_path + "90102-source-and-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        ZipInputStream* zs  = dynamic_cast<ZipInputStream*>(file);
        zs->move_to_entry("test-image-1.png");
        ZipEntryInfo info;
        zs->get_current_entry_info(info);
        CHECK( info.filename == "test-image-1.png" );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_open_current_entry_1)
    {
        string path = m_scores_path + "90101-read-png-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);

        CHECK( zs->my_open_current_entry() == true );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_read_buffer_1)
    {
        string path = m_scores_path + "90101-read-png-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);

        CHECK( zs->my_bytes_pending() == 102 );
        CHECK( zs->my_is_last_buffer() == true );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_read_1)
    {
        string path = m_scores_path + "90101-read-png-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);

        unsigned char data[2000];
        CHECK( zs->read(data, 102) == 102 );
        data[102] = 0;
        string content((char*)data);
        CHECK ( check_content(content) == true );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_read_3)
    {
        string path = m_scores_path + "90101-read-png-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);

        unsigned char data[2000];
        CHECK( zs->read(data, 20) == 20 );
        CHECK( zs->read(data+20, 82) == 82 );
        CHECK( zs->my_bytes_pending() == 0 );
        data[102] = 0;
        string content((char*)data);
        CHECK ( check_content(content) == true );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_read_4)
    {
        string path = m_scores_path + "90102-source-and-image.zip#zip:test-image-1.png";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);

        unsigned char expected[5000];
        int i=0;
        for (i=0; i < 5000 && !zs->eof(); ++i)
            expected[i] = zs->get_char();
        CHECK( i == 5000 );

        zs->move_to_entry("test-image-1.png");
        CHECK( zs->open_current_entry() == true );
        CHECK( zs->my_bytes_pending() == 4096 );
        unsigned char actual[5000];
        CHECK( zs->read(actual, 4000) == 4000 );
        CHECK( zs->read(actual+4000, 1000) == 1000 );
        CHECK( zs->my_bytes_pending() == 3192 );
        CHECK ( memcmp((void*)actual, (void*)expected, 5000) == 0 );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_get_char_1)
    {
        string path = m_scores_path + "90101-read-png-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);

        CHECK( zs->eof() == false );
        CHECK( zs->get_char() == '(' );
        CHECK( zs->eof() == false );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_get_char_2)
    {
        string path = m_scores_path + "90101-read-png-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);

        unsigned char data[2000];
        int i=0;
        for (i=0; i < 200 && !zs->eof(); ++i)
            data[i] = zs->get_char();
        data[i] = 0;
        string content((char*)data);

        CHECK( i == 102 );
        CHECK( zs->eof() == true );
        CHECK ( check_content(content) == true );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_get_char_3)
    {
        string path = m_scores_path + "90102-source-and-image.zip#zip:test-image-1.png";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);

        int i=0;
        for (i=0; !zs->eof(); ++i)
            zs->get_char();

        CHECK( i == 44490 );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_get_char_4)
    {
        string path = m_scores_path + "90103-src-img-txt4096.zip#zip:test-text-4096.txt";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);
        CHECK( zs->get_num_entries() == 3 );

        int i=0;
        for (i=0; !zs->eof(); ++i)
            zs->get_char();

        CHECK( i == 4096 );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_unget_middle)
    {
        string path = m_scores_path + "90103-src-img-txt4096.zip#zip:test-text-4096.txt";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);

        int i=0;
        for (int i=0; i < 8; ++i)
            zs->get_char();

        CHECK( zs->my_bytes_pending() == 4088 );
        CHECK( zs->get_char() == '9' );
        CHECK( zs->my_bytes_pending() == 4087 );
        zs->unget();
        CHECK( zs->my_bytes_pending() == 4088 );
        CHECK( zs->get_char() == '9' );
        CHECK( zs->get_char() == '0' );
    }

    TEST_FIXTURE(ZipInputStreamTestFixture, ZipInputStream_unget_last_1)
    {
        string path = m_scores_path + "90101-read-png-image.zip#zip:";
        InputStream* file = FileSystem::open_input_stream(path);
        MyZipInputStream* zs  = static_cast<MyZipInputStream*>(file);

        int i=0;
        for (int i=0; i < 97; ++i)
            zs->get_char();

        CHECK( zs->get_char() == ')' );
        CHECK( zs->get_char() == '\r' );
        CHECK( zs->get_char() == '\n' );
        CHECK( zs->get_char() == '\r' );
        CHECK( zs->my_bytes_pending() == 1 );
        CHECK( zs->get_char() == '\n' );
        CHECK( zs->eof() == true );
        CHECK( zs->my_bytes_pending() == 0 );
        zs->unget();
        CHECK( zs->my_bytes_pending() == 1 );
        CHECK( zs->eof() == false );
        CHECK( zs->get_char() == '\n' );
        CHECK( zs->eof() == true );
    }

}

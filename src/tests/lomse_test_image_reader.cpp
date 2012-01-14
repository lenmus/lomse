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
#include "lomse_image_reader.h"
#include "lomse_document.h"
//#include "lomse_compiler.h"
//#include "lomse_user_command.h"
//#include "lomse_view.h"
//#include "lomse_document_cursor.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class ImageReaderTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    ImageReaderTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~ImageReaderTestFixture()    //TearDown fixture
    {
    }
};


SUITE(ImageReaderTest)
{

//    TEST_FIXTURE(ImageReaderTestFixture, ImageReader_1)
//    {
//        ImageReader reader("");
//        CHECK( reader.is_valid() == true );
//    }

//    TEST_FIXTURE(ImageReaderTestFixture, ImageReader_1)
//    {
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90100-read-jpg-image.lms");
//    }

    TEST_FIXTURE(ImageReaderTestFixture, ImageReader_can_decode_png_1)
    {
        string path = m_scores_path + "test-image-1.png";
        InputStream* file = FileSystem::open_input_stream(path);
        PngImageDecoder dec;
        CHECK( dec.can_decode(file) == true );
    }

    TEST_FIXTURE(ImageReaderTestFixture, ImageReader_can_decode_png_2)
    {
        string path = m_scores_path + "test-image-rgb.jpg";
        InputStream* file = FileSystem::open_input_stream(path);
        PngImageDecoder dec;
        CHECK( dec.can_decode(file) == false );
    }

}

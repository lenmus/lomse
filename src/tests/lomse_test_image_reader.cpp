//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_image_reader.h"
#include "private/lomse_document_p.h"
//#include "lomse_ldp_compiler.h"
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
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
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
//        doc.from_file(m_scores_path + "08041-read-jpg-image.lms");
//    }

#if (LOMSE_ENABLE_PNG == 1)

    TEST_FIXTURE(ImageReaderTestFixture, ImageReader_can_decode_png_1)
    {
        string path = m_scores_path + "test-image-1.png";
        InputStream* file = FileSystem::open_input_stream(path);
        PngImageDecoder dec;
        CHECK( dec.can_decode(file) == true );
        delete file;
    }

    TEST_FIXTURE(ImageReaderTestFixture, ImageReader_can_decode_png_2)
    {
        string path = m_scores_path + "test-image-2.jpg";
        InputStream* file = FileSystem::open_input_stream(path);
        PngImageDecoder dec;
        CHECK( dec.can_decode(file) == false );
        delete file;
    }

#endif // LOMSE_ENABLE_PNG

}

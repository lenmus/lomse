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
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_score_meter.h"
#include "private/lomse_document_p.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class ScoreMeterTestFixture
{
public:
    LibraryScope m_libraryScope;

    ScoreMeterTestFixture()   // setUp()
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ScoreMeterTestFixture()  // tearDown()
    {
    }
};


SUITE(ScoreMeterTest)
{

    TEST_FIXTURE(ScoreMeterTestFixture, ScoreMeter_NumStaves)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (staves 2)(musicData))"
            "(instrument (staves 3)(musicData))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ScoreMeter meter(pScore);

        CHECK( meter.num_instruments() == 2 );
        CHECK( meter.num_staves() == 5 );
    }

};

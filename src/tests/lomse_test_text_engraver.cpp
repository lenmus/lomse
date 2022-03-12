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
#include "lomse_calligrapher.h"
#include "lomse_text_engraver.h"
#include "private/lomse_document_p.h"
#include "lomse_score_meter.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class TextEngraverTestFixture
{
public:
    LibraryScope m_libraryScope;

    TextEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~TextEngraverTestFixture()    //TearDown fixture
    {
    }

};

SUITE(TextEngraverTest)
{

    TEST_FIXTURE(TextEngraverTestFixture, TextEngraver_MeasureWidth)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pStyle = doc.get_default_style();
        string text("This is a test");
        ScoreMeter meter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        TextEngraver engraver(m_libraryScope, &meter, text, "", pStyle);

        LUnits width = engraver.measure_width();
        CHECK( width > 0.0f );
    }

    TEST_FIXTURE(TextEngraverTestFixture, TextEngraver_MeasureWidthDefaultStyle)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (name ''Violin'')(musicData (n c4 q))))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        TypeTextInfo& text = pInstr->get_name();
        ImoStyle* pStyle = pScore->get_default_style();
        ScoreMeter meter(pScore, 1, 1, LOMSE_STAFF_LINE_SPACING);
        TextEngraver engraver(m_libraryScope, &meter, text.text, "", pStyle);

        LUnits width = engraver.measure_width();
        CHECK( width > 0.0f );
    }

}



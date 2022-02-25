//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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



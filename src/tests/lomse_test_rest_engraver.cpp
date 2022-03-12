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
#include "lomse_im_note.h"
#include "lomse_rest_engraver.h"
#include "private/lomse_document_p.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_engravers_map.h"
#include "lomse_im_factory.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class RestEngraverTestFixture
{
public:
    LibraryScope m_libraryScope;

    RestEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~RestEngraverTestFixture()    //TearDown fixture
    {
    }

};

SUITE(RestEngraverTest)
{

    TEST_FIXTURE(RestEngraverTestFixture, RestEngraver_OnlyGlyph)
    {
        Document doc(m_libraryScope);
        ImoRest* pRest = static_cast<ImoRest*>(ImFactory::inject(k_imo_rest, &doc));
        pRest->set_note_type(k_whole);
        pRest->set_dots(0);

        ScoreMeter meter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        EngraversMap storage;
        RestEngraver engraver(m_libraryScope, &meter, &storage, 0, 0, 0, 0);
        GmoShapeRest* pShape =
            dynamic_cast<GmoShapeRest*>(engraver.create_shape(pRest,
                                        UPoint(10.0f, 15.0f)));
        CHECK( pShape != nullptr );
        CHECK( pShape && pShape->is_shape_rest() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 1 );
        CHECK( (*it)->is_shape_rest_glyph() );

        delete pShape;
        delete pRest;
    }

    TEST_FIXTURE(RestEngraverTestFixture, RestEngraver_GlyphAndDot)
    {
        Document doc(m_libraryScope);
        ImoRest* pRest = static_cast<ImoRest*>(ImFactory::inject(k_imo_rest, &doc));
        pRest->set_note_type(k_whole);
        pRest->set_dots(1);

        ScoreMeter meter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        EngraversMap storage;
        RestEngraver engraver(m_libraryScope, &meter, &storage, 0, 0, 0, 0);
        GmoShapeRest* pShape =
            dynamic_cast<GmoShapeRest*>(engraver.create_shape(pRest,
                                        UPoint(10.0f, 15.0f)));
        CHECK( pShape != nullptr );
        CHECK( pShape && pShape->is_shape_rest() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 2 );
        CHECK( (*it)->is_shape_rest_glyph() );
        ++it;
        CHECK( (*it)->is_shape_dot() );

        delete pShape;
        delete pRest;
    }

}



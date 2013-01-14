//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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
#include "lomse_im_note.h"
#include "lomse_rest_engraver.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"
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

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        RestEngraver engraver(m_libraryScope, &meter, &storage, 0, 0);
        GmoShapeRest* pShape =
            dynamic_cast<GmoShapeRest*>(engraver.create_shape(pRest,
                                        UPoint(10.0f, 15.0f)));
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_rest() == true );
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

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        RestEngraver engraver(m_libraryScope, &meter, &storage, 0, 0);
        GmoShapeRest* pShape =
            dynamic_cast<GmoShapeRest*>(engraver.create_shape(pRest,
                                        UPoint(10.0f, 15.0f)));
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_rest() == true );
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



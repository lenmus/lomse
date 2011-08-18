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
        RestEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeRest* pShape =
            dynamic_cast<GmoShapeRest*>(engraver.create_shape(pRest, 0, 0,
                                        UPoint(10.0f, 15.0f), pRest->get_note_type(),
                                        pRest->get_dots(), pRest));
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
        RestEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeRest* pShape =
            dynamic_cast<GmoShapeRest*>(engraver.create_shape(pRest, 0, 0,
                                        UPoint(10.0f, 15.0f), pRest->get_note_type(),
                                        pRest->get_dots(), pRest));
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



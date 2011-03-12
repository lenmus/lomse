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

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_basic_objects.h"
#include "lomse_im_note.h"
#include "lomse_rest_engraver.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"

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
        DtoRest dtoRest;
        dtoRest.set_note_type(ImoRest::k_whole);
        dtoRest.set_dots(0);
        ImoRest rest(dtoRest);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        RestEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeRest* pShape =
            dynamic_cast<GmoShapeRest*>(engraver.create_shape(&rest, 0, 0, 
                                        UPoint(10.0f, 15.0f), rest.get_note_type(),
                                        rest.get_dots(), &rest));
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_rest() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 1 );
        CHECK( (*it)->is_shape_rest_glyph() );

        delete pShape;
    }

    TEST_FIXTURE(RestEngraverTestFixture, RestEngraver_GlyphAndDot)
    {
        DtoRest dtoRest;
        dtoRest.set_note_type(ImoRest::k_whole);
        dtoRest.set_dots(1);
        ImoRest rest(dtoRest);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        RestEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeRest* pShape =
            dynamic_cast<GmoShapeRest*>(engraver.create_shape(&rest, 0, 0, 
                                        UPoint(10.0f, 15.0f), rest.get_note_type(),
                                        rest.get_dots(), &rest));
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_rest() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 2 );
        CHECK( (*it)->is_shape_rest_glyph() );
        ++it;
        CHECK( (*it)->is_shape_dot() );

        delete pShape;
    }

    //TEST_FIXTURE(RestEngraverTestFixture, RestEngraver_HeadStemFlagTwoDots)
    //{
    //    DtoRest dtoRest;
    //    dtoRest.set_step(0);
    //    dtoRest.set_octave(4);
    //    dtoRest.set_accidentals(0);
    //    dtoRest.set_note_type(ImoRest::k_eighth);
    //    dtoRest.set_dots(2);
    //    ImoRest rest(dtoRest);

    //    ScoreMeter meter(1, 1, 180.0f);
    //    ShapesStorage storage;
    //    RestEngraver engraver(m_libraryScope, &meter, &storage);
    //    GmoShapeRest* pShape =
    //        dynamic_cast<GmoShapeRest*>(engraver.create_shape(&rest, 0, 0, ImoClef::k_F4, UPoint(10.0f, 15.0f)) );
    //    CHECK( pShape != NULL );
    //    CHECK( pShape->is_shape_rest() == true );
    //    std::list<GmoShape*>& components = pShape->get_components();
    //    std::list<GmoShape*>::iterator it = components.begin();
    //    CHECK( components.size() == 5 );
    //    CHECK( (*it)->is_shape_notehead() );
    //    ++it;
    //    CHECK( (*it)->is_shape_dot() );
    //    ++it;
    //    CHECK( (*it)->is_shape_dot() );
    //    ++it;
    //    CHECK( (*it)->is_shape_stem() );
    //    ++it;
    //    CHECK( (*it)->is_shape_flag() );

    //    delete pShape;
    //}

}



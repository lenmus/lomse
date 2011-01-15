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
#include <list>
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_shape_note.h"
#include "lomse_glyphs.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
class GmoShapeTestFixture
{
public:

    GmoShapeTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~GmoShapeTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

//---------------------------------------------------------------------------------------
SUITE(GmoShapeTest)
{

    // ShapeGlyph -----------------------------------------------------------------------

    TEST_FIXTURE(GmoShapeTestFixture, Notehead_ShiftOrigin)
    {
        UPoint pos(200.0f, 500.0f);
        GmoShapeNotehead shape(0, k_glyph_notehead_quarter, pos, Color(0,0,0),
                               *m_pLibraryScope);
        //cout << "origin(" << shape.get_origin().x << ", " << shape.get_origin().y << ")" << endl;
        //cout << "width=" << shape.get_width() << ", height=" << shape.get_height() << endl;
        UPoint oldOrigin = shape.get_origin();
        CHECK( shape.get_width() > 0.0f );
        CHECK( shape.get_height() > 0.0f );

        USize shift(1800.0f, 2500.0f);
        shape.shift_origin(shift);
        //cout << "shifted origin(" << shape.get_origin().x << ", " << shape.get_origin().y << ")" << endl;

        UPoint newOrigin(oldOrigin.x + shift.width, oldOrigin.y + shift.height);
        CHECK( shape.get_origin() == newOrigin );
    }

//    TEST_FIXTURE(GmoShapeTestFixture, Shape_SetOrigin)
//    {
//        GmoBoxDocPage page;
//        GmoBoxDocPageContent* pDPC = new GmoBoxDocPageContent();
//        page.add_child_box(pDPC);
//        GmoStubScore stub(NULL);
//        GmoBoxScorePage* pScorePage = new GmoBoxScorePage(&stub);
//        pDPC->add_child_box(pScorePage);
//        GmoBoxSystem* pBox = new GmoBoxSystem();
//        pScorePage->add_child_box(pBox);
//        ImoStaffInfo staff;
//        GmoShapeStaff* pShape = new GmoShapeStaff(0, &staff, 0, 0.0f, Color(0,0,0));
//        pBox->add_shape(pShape, 0);
//        pShape->set_origin(2000.0f, 3000.0f);
//        CHECK( pShape->get_left() == 2000.0f );
//        CHECK( pShape->get_top() == 3000.0f );
//    }

}



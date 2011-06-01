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
#include "lomse_basic_objects.h"
#include "lomse_shape_note.h"
#include "lomse_shape_staff.h"
#include "lomse_glyphs.h"
#include "lomse_im_note.h"
#include "lomse_note_engraver.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
class GmoShapeTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    GmoShapeTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~GmoShapeTestFixture()    //TearDown fixture
    {
    }
};

//---------------------------------------------------------------------------------------
SUITE(GmoShapeTest)
{

    // ShapeGlyph -----------------------------------------------------------------------

    TEST_FIXTURE(GmoShapeTestFixture, Notehead_ShiftOrigin)
    {
        UPoint pos(200.0f, 500.0f);
        GmoShapeNotehead shape(NULL, 0, k_glyph_notehead_quarter, pos, Color(0,0,0),
                               m_libraryScope, 21.0);
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

    TEST_FIXTURE(GmoShapeTestFixture, Composite_IsLocked)
    {
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(k_whole);
        ImoNote note(dtoNote);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, 0, 0, ImoClef::k_F4, UPoint(10.0f, 15.0f)) );

        CHECK( pShape != NULL );
        CHECK( pShape->is_locked() == true );

        delete pShape;
    }

    TEST_FIXTURE(GmoShapeTestFixture, Composite_LockRecomputesBounds)
    {
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(k_whole);
        ImoNote note(dtoNote);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, 0, 0, ImoClef::k_F4, UPoint(10.0f, 15.0f)) );

        pShape->unlock();
        CHECK( pShape->is_locked() == false );

        USize size = pShape->get_size();
        UPoint org = pShape->get_origin();
        GmoShapeNotehead* pNH = pShape->get_notehead_shape();
        USize shift(200.0f, 300.0f);
        pNH->shift_origin(shift);
        pShape->lock();

        CHECK( pShape->is_locked() == true );
        CHECK( pShape->get_size() == size );
        CHECK( pShape->get_origin().x == org.x + shift.width );
        CHECK( pShape->get_origin().y == org.y + shift.height );

        delete pShape;
    }

    TEST_FIXTURE(GmoShapeTestFixture, Composite_LockRecomputesBounds2)
    {
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(k_flat);
        dtoNote.set_note_type(k_whole);
        ImoNote note(dtoNote);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, 0, 0, ImoClef::k_F4, UPoint(10.0f, 15.0f)) );

        pShape->unlock();
        CHECK( pShape->is_locked() == false );

        USize size = pShape->get_size();
        UPoint org = pShape->get_origin();
        GmoShapeNotehead* pNH = pShape->get_notehead_shape();
        GmoShapeAccidentals* pAcc = pShape->get_accidentals_shape();
        USize shift(200.0f, 300.0f);
        pNH->shift_origin(shift);
        pShape->lock();

        CHECK( pShape->is_locked() == true );
        CHECK( pShape->get_size().width == size.width + shift.width );
        CHECK( pShape->get_origin().x == min(pAcc->get_origin().x, pNH->get_origin().x) );
        CHECK( pShape->get_origin().y == min(pAcc->get_origin().y, pNH->get_origin().y) );

        delete pShape;
    }

    TEST_FIXTURE(GmoShapeTestFixture, Shape_SetOrigin)
    {
        ImoStaffInfo staff;
        GmoShapeStaff shape(NULL, 0, &staff, 0, 0.0f, Color(0,0,0));
        shape.set_origin(2000.0f, 3000.0f);

        CHECK( shape.get_left() == 2000.0f );
        CHECK( shape.get_top() == 3000.0f );
    }


    // related shapes -------------------------------------------------------------------

    TEST_FIXTURE(GmoShapeTestFixture, GetRelatedShapes_Empty)
    {
        ImoStaffInfo staffInfo;
        GmoShapeStaff staff(NULL, 0, &staffInfo, 0, 0.0f, Color(0,0,0));

        CHECK( staff.get_related_shapes() == NULL );
    }

    TEST_FIXTURE(GmoShapeTestFixture, AddRelatedShape)
    {
        ImoStaffInfo staffInfo;
        GmoShapeStaff staff(NULL, 0, &staffInfo, 0, 0.0f, Color(0,0,0));
        GmoShapeNote note(NULL, 150.0f, 200.0f, Color(0,0,0), m_libraryScope);

        staff.add_related_shape(&note);

        std::list<GmoShape*>* m_pRelated = staff.get_related_shapes();
        CHECK( m_pRelated != NULL );
        CHECK( m_pRelated->front() == &note );
    }

    TEST_FIXTURE(GmoShapeTestFixture, GetRelatedShapeOfType)
    {
        ImoStaffInfo staffInfo;
        GmoShapeStaff staff(NULL, 0, &staffInfo, 0, 0.0f, Color(0,0,0));
        GmoShapeNote note(NULL, 150.0f, 200.0f, Color(0,0,0), m_libraryScope);
        GmoShapeRest rest(NULL, 0, 150.0f, 200.0f, Color(0,0,0), m_libraryScope);

        staff.add_related_shape(&note);
        staff.add_related_shape(&rest);

        CHECK( staff.find_related_shape(GmoObj::k_shape_note) == &note );
        CHECK( staff.find_related_shape(GmoObj::k_shape_rest) == &rest );
        CHECK( staff.find_related_shape(GmoObj::k_shape_beam) == NULL );
    }


}



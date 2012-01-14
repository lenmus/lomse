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
#include <list>
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_shape_note.h"
#include "lomse_shape_staff.h"
#include "lomse_glyphs.h"
#include "lomse_im_note.h"
#include "lomse_note_engraver.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"

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
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote->set_step(0);
        pNote->set_octave(4);
        pNote->set_notated_accidentals(k_no_accidentals);
        pNote->set_note_type(k_whole);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, 0, 0, k_clef_F4, UPoint(10.0f, 15.0f)) );

        CHECK( pShape != NULL );
        CHECK( pShape->is_locked() == true );

        delete pNote;
        delete pShape;
    }

    TEST_FIXTURE(GmoShapeTestFixture, Composite_LockRecomputesBounds)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote->set_step(0);
        pNote->set_octave(4);
        pNote->set_notated_accidentals(k_no_accidentals);
        pNote->set_note_type(k_whole);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, 0, 0, k_clef_F4, UPoint(10.0f, 15.0f)) );

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

        delete pNote;
        delete pShape;
    }

    TEST_FIXTURE(GmoShapeTestFixture, Composite_LockRecomputesBounds2)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote->set_step(0);
        pNote->set_octave(4);
        pNote->set_notated_accidentals(k_flat);
        pNote->set_note_type(k_whole);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, 0, 0, k_clef_F4, UPoint(10.0f, 15.0f)) );

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

        delete pNote;
        delete pShape;
    }

    TEST_FIXTURE(GmoShapeTestFixture, Shape_SetOrigin)
    {
        Document doc(m_libraryScope);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff shape(NULL, 0, pInfo, 0, 0.0f, Color(0,0,0));
        shape.set_origin(2000.0f, 3000.0f);

        CHECK( shape.get_left() == 2000.0f );
        CHECK( shape.get_top() == 3000.0f );
        delete pInfo;
    }


    // related shapes -------------------------------------------------------------------

    TEST_FIXTURE(GmoShapeTestFixture, GetRelatedShapes_Empty)
    {
        Document doc(m_libraryScope);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff staff(NULL, 0, pInfo, 0, 0.0f, Color(0,0,0));

        CHECK( staff.get_related_shapes() == NULL );
        delete pInfo;
    }

    TEST_FIXTURE(GmoShapeTestFixture, AddRelatedShape)
    {
        Document doc(m_libraryScope);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff staff(NULL, 0, pInfo, 0, 0.0f, Color(0,0,0));
        GmoShapeNote note(NULL, 150.0f, 200.0f, Color(0,0,0), m_libraryScope);

        staff.add_related_shape(&note);

        std::list<GmoShape*>* m_pRelated = staff.get_related_shapes();
        CHECK( m_pRelated != NULL );
        CHECK( m_pRelated->front() == &note );
        delete pInfo;
    }

    TEST_FIXTURE(GmoShapeTestFixture, GetRelatedShapeOfType)
    {
        Document doc(m_libraryScope);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff staff(NULL, 0, pInfo, 0, 0.0f, Color(0,0,0));
        GmoShapeNote note(NULL, 150.0f, 200.0f, Color(0,0,0), m_libraryScope);
        GmoShapeRest rest(NULL, 0, 150.0f, 200.0f, Color(0,0,0), m_libraryScope);

        staff.add_related_shape(&note);
        staff.add_related_shape(&rest);

        CHECK( staff.find_related_shape(GmoObj::k_shape_note) == &note );
        CHECK( staff.find_related_shape(GmoObj::k_shape_rest) == &rest );
        CHECK( staff.find_related_shape(GmoObj::k_shape_beam) == NULL );
        delete pInfo;
    }

    // flags ----------------------------------------------------------------------------

    TEST_FIXTURE(GmoShapeTestFixture, set_selected)
    {
        Document doc(m_libraryScope);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff staff(NULL, 0, pInfo, 0, 0.0f, Color(0,0,0));
        CHECK( staff.is_selected() == false );

        staff.set_selected(true);
        CHECK( staff.is_selected() == true );
        staff.set_selected(false);
        CHECK( staff.is_selected() == false );

        delete pInfo;
    }

    TEST_FIXTURE(GmoShapeTestFixture, set_dirty)
    {
        Document doc(m_libraryScope);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff staff(NULL, 0, pInfo, 0, 0.0f, Color(0,0,0));
        CHECK( staff.is_dirty() == true );

        staff.set_dirty(false);
        CHECK( staff.is_dirty() == false );
        staff.set_dirty(true);
        CHECK( staff.is_dirty() == true );

        delete pInfo;
    }

    TEST_FIXTURE(GmoShapeTestFixture, set_children_dirty)
    {
        Document doc(m_libraryScope);
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, &doc));
        GmoShapeStaff staff(NULL, 0, pInfo, 0, 0.0f, Color(0,0,0));
        CHECK( staff.are_children_dirty() == false );

        staff.set_children_dirty(true);
        CHECK( staff.are_children_dirty() == true );
        staff.set_children_dirty(false);
        CHECK( staff.are_children_dirty() == false );

        delete pInfo;
    }

}



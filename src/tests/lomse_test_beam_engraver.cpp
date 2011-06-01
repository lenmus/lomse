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
#include "lomse_beam_engraver.h"
#include "lomse_note_engraver.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_shape_beam.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"
#include "lomse_analyser.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// Access to protected members
class MyBeamEngraver : public BeamEngraver
{
public:
    MyBeamEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
        : BeamEngraver(libraryScope, pScoreMeter)
    {
    }

    inline void my_decide_on_stems_direction() { decide_on_stems_direction(); }
    inline bool my_stems_forced() { return m_fStemForced; }
    inline bool my_stems_mixed() { return m_fStemMixed; }
    inline bool my_stems_down() { return m_fStemsDown; }
    inline int my_get_num_stems_down() { return m_numStemsDown; }
    inline int my_get_num_notes() { return m_numNotes; }
    inline int my_get_average_pos_on_staff() { return m_averagePosOnStaff; }

    ShapeBoxInfo* my_get_shape_box_info() { return get_shape_box_info(0); }
};


//---------------------------------------------------------------------------------------
class BeamEngraverTestFixture
{
public:
    LibraryScope    m_libraryScope;
    ScoreMeter*     m_pMeter;
    ShapesStorage*  m_pStorage;
    NoteEngraver*   m_pNoteEngrv;
    MyBeamEngraver* m_pBeamEngrv;
    GmoShapeBeam*   m_pBeamShape;
    std::vector<GmoShapeNote*> m_shapes;

    BeamEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pMeter(NULL)
        , m_pStorage(NULL)
        , m_pNoteEngrv(NULL)
        , m_pBeamEngrv(NULL)
        , m_pBeamShape(NULL)
    {
    }

    ~BeamEngraverTestFixture()    //TearDown fixture
    {
    }

    ImoBeam* create_beam(int numNotes, ImoNote* notes[], string beams[], int beamNum)
    {
        Analyser a(cout, m_libraryScope);
        BeamsBuilder builder(cout, &a);
        for (int i=0; i < numNotes; ++i)
        {
            ImoBeamDto* pDto = new ImoBeamDto();
            pDto->set_beam_number(beamNum);
            pDto->set_note_rest(notes[i]);
            pDto->set_beam_type(beams[i]);
            builder.add_item_info(pDto);
        }

        return notes[0]->get_beam();
    }

    void prepare_to_engrave_beam(int numNotes, ImoNote* notes[], ImoBeam* pBeam)
    {
        int iInstr = 0;
        int iStaff = 0;
        int iSystem = 0;
        int iCol = 0;
        UPoint pos(0.0f, 0.0f);

        m_shapes.reserve(numNotes);
        m_pMeter = new ScoreMeter(1, 1, 180.0f);
        m_pStorage = new ShapesStorage();

        //engrave notes
        m_pNoteEngrv = new NoteEngraver(m_libraryScope, m_pMeter, m_pStorage);
        for (int i=0; i < numNotes; ++i)
        {
            GmoShapeNote* pShape = dynamic_cast<GmoShapeNote*>(
                m_pNoteEngrv->create_shape(notes[i], 0, 0, ImoClef::k_G2,
                                           UPoint(10.0f, 15.0f)) );
            m_shapes.push_back(pShape);
        }

        //send note data to beam engraver
        for (int i=0; i < numNotes; ++i)
        {
            if (i == 0)
            {
                //first note
                m_pBeamEngrv = new MyBeamEngraver(m_libraryScope, m_pMeter);
                m_pBeamEngrv->set_start_staffobj(pBeam, notes[i], m_shapes[i],
                                                 iInstr, iStaff, iSystem, iCol, pos);
                m_pStorage->save_engraver(m_pBeamEngrv, pBeam);
            }
            else if (i == numNotes-1)
            {
                //last note
                m_pBeamEngrv->set_end_staffobj(pBeam, notes[i], m_shapes[i],
                                               iInstr, iStaff, iSystem, iCol);
            }
            else
            {
                //intermediate note
                m_pBeamEngrv->set_middle_staffobj(pBeam, notes[i], m_shapes[i],
                                                  iInstr, iStaff, iSystem, iCol);
            }
        }
    }

    void delete_test_data()
    {
        delete m_pMeter;
        delete m_pStorage;
        delete m_pNoteEngrv;
        delete m_pBeamEngrv;
        delete m_pBeamShape;

        std::vector<GmoShapeNote*>::iterator it;
        for (it = m_shapes.begin(); it != m_shapes.end(); ++it)
            delete *it;
        m_shapes.clear();

        m_pMeter = NULL;
        m_pStorage = NULL;
        m_pNoteEngrv = NULL;
        m_pBeamEngrv = NULL;
        m_pBeamShape = NULL;
    }



};

SUITE(BeamEngraverTest)
{

    TEST_FIXTURE(BeamEngraverTestFixture, CreateBeam)
    {
        ImoNote note1(k_step_C, 4, k_eighth);
        ImoNote note2(k_step_F, 4, k_eighth);
        ImoNote* notes[] = { &note1, &note2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);

        CHECK( pBeam != NULL );
        //CHECK( pBeam->get_beam_number() == 12 );

        CHECK( notes[0]->is_beamed() == true );
        CHECK( notes[0]->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( notes[0]->get_beam_type(1) == ImoBeam::k_none );

        CHECK( notes[1]->is_beamed() == true );
        CHECK( notes[1]->get_beam_type(0) == ImoBeam::k_end );
        CHECK( notes[1]->get_beam_type(1) == ImoBeam::k_none );
    }

    TEST_FIXTURE(BeamEngraverTestFixture, FeedEngraver)
    {
        ImoNote note1(k_step_C, 4, k_eighth);
        ImoNote note2(k_step_F, 4, k_eighth);
        ImoNote* notes[] = { &note1, &note2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        MyBeamEngraver* pEngrv = dynamic_cast<MyBeamEngraver*>(m_pStorage->get_engraver(pBeam));

        CHECK( pEngrv != NULL );
        CHECK( pEngrv == m_pBeamEngrv );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, CreateBeamShape)
    {
        ImoNote note1(k_step_C, 4, k_eighth);
        ImoNote note2(k_step_F, 4, k_eighth);
        ImoNote* notes[] = { &note1, &note2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        m_pBeamEngrv->create_shapes();
        m_pBeamShape = dynamic_cast<GmoShapeBeam*>( m_pBeamEngrv->get_shape() );
        CHECK( m_pBeamShape != NULL );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, BeamShapeAddedToNoteShape)
    {
        ImoNote note1(k_step_C, 4, k_eighth);
        ImoNote note2(k_step_F, 4, k_eighth);
        ImoNote* notes[] = { &note1, &note2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        m_pBeamEngrv->create_shapes();
        m_pBeamShape = dynamic_cast<GmoShapeBeam*>( m_pBeamEngrv->get_shape() );

        //method prepare_to_engrave_beam() store notes shapes in m_shapes
        std::vector<GmoShapeNote*>::iterator it;
        for (it = m_shapes.begin(); it != m_shapes.end(); ++it)
            CHECK( (*it)->find_related_shape(GmoObj::k_shape_beam) == m_pBeamShape );

        delete_test_data();
    }


    // decide_on_stems_direction --------------------------------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, DecideStemsDirection)
    {
        ImoNote note1(k_step_C, 4, k_eighth);
        ImoNote note2(k_step_F, 4, k_eighth);
        ImoNote* notes[] = { &note1, &note2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        m_pBeamEngrv->my_decide_on_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == false );
        CHECK( m_pBeamEngrv->my_stems_mixed() == false );
        CHECK( m_pBeamEngrv->my_stems_down() == false );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, StemsForced)
    {
        ImoNote note1(k_step_C, 4, k_eighth, k_no_accidentals, 0, 0, 0, k_stem_up);
        ImoNote note2(k_step_F, 4, k_eighth, k_no_accidentals, 0, 0, 0, k_stem_up);
        ImoNote* notes[] = { &note1, &note2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        m_pBeamEngrv->my_decide_on_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_stems_mixed() == false );
        CHECK( m_pBeamEngrv->my_stems_down() == false );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, StemsMixedButNotAllowed)
    {
        ImoNote note1(k_step_C, 5, k_eighth);
        ImoNote note2(k_step_F, 4, k_eighth);
        ImoNote* notes[] = { &note1, &note2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        m_pBeamEngrv->my_decide_on_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == false );
        CHECK( m_pBeamEngrv->my_stems_mixed() == false );   //for now forced to false
        CHECK( m_pBeamEngrv->my_stems_down() == false );
//        cout << "num stems down = " << m_pBeamEngrv->my_get_num_stems_down() << endl;
//        cout << "num notes = " << m_pBeamEngrv->my_get_num_notes() << endl;
//        cout << "average pos = " << m_pBeamEngrv->my_get_average_pos_on_staff() << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, StemsMixed)
    {
        ImoNote note1(k_step_C, 4, k_eighth, k_no_accidentals, 0, 0, 0, k_stem_down);
        ImoNote note2(k_step_F, 4, k_eighth, k_no_accidentals, 0, 0, 0, k_stem_up);
        ImoNote* notes[] = { &note1, &note2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        m_pBeamEngrv->my_decide_on_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_stems_mixed() == false );   //always false when stems forced
        CHECK( m_pBeamEngrv->my_stems_down() == false );    //stem forced by last forced stem

        delete_test_data();
    }


//     decide_beam_position -------------------------------------------------------------
//
//    TEST_FIXTURE(BeamEngraverTestFixture, DecideStemsDirection)
//    {
//        ImoNote note1(k_step_C, 4, k_eighth);
//        ImoNote note2(k_step_F, 4, k_eighth);
//        ImoNote* notes[] = { &note1, &note2 };
//        string beams[] = { "+", "-" };
//
//        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
//        prepare_to_engrave_beam(2, notes, pBeam);
//
//        m_pBeamEngrv->my_decide_on_stems_direction();
//        m_pBeamEngrv->decide_beam_position();
//
//        CHECK( m_pBeamEngrv->my_stems_forced() == false );
//        CHECK( m_pBeamEngrv->my_stems_mixed() == false );
//        CHECK( m_pBeamEngrv->my_stems_down() == false );
//
//        delete_test_data();
//    }

}



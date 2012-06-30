//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
#include "lomse_beam_engraver.h"
#include "lomse_note_engraver.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_shape_beam.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"
#include "lomse_ldp_analyser.h"
#include "lomse_im_factory.h"

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
        Document doc(m_libraryScope);
        LdpAnalyser a(cout, m_libraryScope, &doc);
        BeamsBuilder builder(cout, &a);
        for (int i=0; i < numNotes; ++i)
        {
            ImoBeamDto* pDto = static_cast<ImoBeamDto*>(
                                        ImFactory::inject(k_imo_beam_dto, &doc));
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

        m_shapes.reserve(numNotes);
        m_pMeter = LOMSE_NEW ScoreMeter(1, 1, 180.0f);
        m_pStorage = LOMSE_NEW ShapesStorage();

        //engrave notes
        m_pNoteEngrv = LOMSE_NEW NoteEngraver(m_libraryScope, m_pMeter, m_pStorage, 0, 0);
        for (int i=0; i < numNotes; ++i)
        {
            GmoShapeNote* pShape = dynamic_cast<GmoShapeNote*>(
                m_pNoteEngrv->create_shape(notes[i], k_clef_G2,
                                           UPoint(10.0f, 15.0f)) );
            m_shapes.push_back(pShape);
        }

        //send note data to beam engraver
        for (int i=0; i < numNotes; ++i)
        {
            if (i == 0)
            {
                //first note
                m_pBeamEngrv = LOMSE_NEW MyBeamEngraver(m_libraryScope, m_pMeter);
                m_pBeamEngrv->set_start_staffobj(pBeam, notes[i], m_shapes[i],
                                                 iInstr, iStaff, iSystem, iCol);
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
        Document doc(m_libraryScope);
        ImoNote* pNote1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth);
        ImoNote* pNote2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth);
        ImoNote* notes[] = { pNote1, pNote2 };
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

        delete pNote1;
        delete pNote2;
    }

    TEST_FIXTURE(BeamEngraverTestFixture, FeedEngraver)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth);
        ImoNote* pNote2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth);
        ImoNote* notes[] = { pNote1, pNote2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        MyBeamEngraver* pEngrv = dynamic_cast<MyBeamEngraver*>(m_pStorage->get_engraver(pBeam));

        CHECK( pEngrv != NULL );
        CHECK( pEngrv == m_pBeamEngrv );

        delete_test_data();
        delete pNote1;
        delete pNote2;
    }

    TEST_FIXTURE(BeamEngraverTestFixture, CreateBeamShape)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth);
        ImoNote* pNote2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth);
        ImoNote* notes[] = { pNote1, pNote2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        m_pBeamEngrv->create_shapes();
        m_pBeamShape = dynamic_cast<GmoShapeBeam*>( m_pBeamEngrv->get_shape() );
        CHECK( m_pBeamShape != NULL );

        delete_test_data();
        delete pNote1;
        delete pNote2;
    }

    TEST_FIXTURE(BeamEngraverTestFixture, BeamShapeAddedToNoteShape)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth);
        ImoNote* pNote2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth);
        ImoNote* notes[] = { pNote1, pNote2 };
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
        delete pNote1;
        delete pNote2;
    }


    // decide_on_stems_direction --------------------------------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, DecideStemsDirection)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth);
        ImoNote* pNote2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth);
        ImoNote* notes[] = { pNote1, pNote2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        m_pBeamEngrv->my_decide_on_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == false );
        CHECK( m_pBeamEngrv->my_stems_mixed() == false );
        CHECK( m_pBeamEngrv->my_stems_down() == false );

        delete_test_data();
        delete pNote1;
        delete pNote2;
    }

    TEST_FIXTURE(BeamEngraverTestFixture, StemsForced)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth, k_no_accidentals, 0, 0, 0, k_stem_up);
        ImoNote* pNote2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth, k_no_accidentals, 0, 0, 0, k_stem_up);
        ImoNote* notes[] = { pNote1, pNote2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        m_pBeamEngrv->my_decide_on_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_stems_mixed() == false );
        CHECK( m_pBeamEngrv->my_stems_down() == false );

        delete_test_data();
        delete pNote1;
        delete pNote2;
    }

    TEST_FIXTURE(BeamEngraverTestFixture, StemsMixedButNotAllowed)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote1 = ImFactory::inject_note(&doc, k_step_C, 5, k_eighth);
        ImoNote* pNote2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth);
        ImoNote* notes[] = { pNote1, pNote2 };
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
        delete pNote1;
        delete pNote2;
    }

    TEST_FIXTURE(BeamEngraverTestFixture, StemsMixed)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth, k_no_accidentals, 0, 0, 0, k_stem_down);
        ImoNote* pNote2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth, k_no_accidentals, 0, 0, 0, k_stem_up);
        ImoNote* notes[] = { pNote1, pNote2 };
        string beams[] = { "+", "-" };

        ImoBeam* pBeam = create_beam(2, notes, beams, 12);
        prepare_to_engrave_beam(2, notes, pBeam);

        m_pBeamEngrv->my_decide_on_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_stems_mixed() == false );   //always false when stems forced
        CHECK( m_pBeamEngrv->my_stems_down() == false );    //stem forced by last forced stem

        delete_test_data();
        delete pNote1;
        delete pNote2;
    }


//     decide_beam_position -------------------------------------------------------------
//
//    TEST_FIXTURE(BeamEngraverTestFixture, DecideStemsDirection)
//    {
//        ImoNote* pNote1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth);
//        ImoNote* pNote2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth);
//        ImoNote* notes[] = { pNote1, pNote2 };
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



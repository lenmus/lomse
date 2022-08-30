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
#include "lomse_note_engraver.h"
#include "lomse_chord_engraver.h"
#include "lomse_tuplet_engraver.h"
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
class NoteEngraverTestFixture
{
public:
    LibraryScope m_libraryScope;

    NoteEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pMeter(nullptr)
        , m_pEngrv(nullptr)
        , m_pTuplet(nullptr)
        , m_pNote1(nullptr)
        , m_pNote2(nullptr)
        , m_pNote3(nullptr)
        , m_pShape1(nullptr)
        , m_pShape2(nullptr)
        , m_pShape3(nullptr)
        , m_pStorage(nullptr)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~NoteEngraverTestFixture()    //TearDown fixture
    {
    }

    void create_tuplet()
    {
        delete_tuplet();
        Document doc(m_libraryScope);
        m_pTuplet = static_cast<ImoTuplet*>( ImFactory::inject(k_imo_tuplet, &doc) );

        m_pNote1 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note_regular, &doc));
        m_pNote1->set_notated_pitch(k_step_C, k_octave_4, k_no_accidentals);
        m_pNote1->set_note_type(k_eighth);
        m_pNote1->set_dots(0);

        ImoTupletDto dto1;
        dto1.set_tuplet_type(ImoTupletDto::k_start);

        m_pNote1->include_in_relation(m_pTuplet, nullptr);

        m_pNote2 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note_regular, &doc));
        m_pNote2->set_notated_pitch(k_step_E, k_octave_4, k_no_accidentals);

        ImoTupletDto dto2;
        dto2.set_tuplet_type(ImoTupletDto::k_continue);

        m_pNote2->include_in_relation(m_pTuplet, nullptr);

        m_pNote3 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note_regular, &doc));
        m_pNote3->set_notated_pitch(k_step_G, k_octave_4, k_no_accidentals);

        ImoTupletDto dto3;
        dto3.set_tuplet_type(ImoTupletDto::k_stop);
        m_pNote3->include_in_relation(m_pTuplet, nullptr);

        m_pMeter = LOMSE_NEW ScoreMeter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        m_pStorage = LOMSE_NEW EngraversMap();
        m_pEngrv = LOMSE_NEW NoteEngraver(m_libraryScope, m_pMeter, m_pStorage, 0, 0);
        m_pShape1 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote1, k_clef_G2, 0, UPoint(10.0f, 15.0f)) );
        m_pShape2 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote2, k_clef_G2, 0, UPoint(10.0f, 15.0f)) );
        m_pShape3 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote3, k_clef_G2, 0, UPoint(10.0f, 15.0f)) );
    }

    void delete_tuplet()
    {
        delete m_pMeter;
        delete m_pEngrv;
        delete m_pNote1;
        delete m_pNote2;
        delete m_pNote3;
        delete m_pShape1;
        delete m_pShape2;
        delete m_pShape3;
        delete m_pStorage;
    }

    ScoreMeter* m_pMeter;
    NoteEngraver* m_pEngrv;
    ImoTuplet* m_pTuplet;
    ImoNote* m_pNote1;
    ImoNote* m_pNote2;
    ImoNote* m_pNote3;
    GmoShapeNote* m_pShape1;
    GmoShapeNote* m_pShape2;
    GmoShapeNote* m_pShape3;
    EngraversMap* m_pStorage;

};

SUITE(NoteEngraverTest)
{

    TEST_FIXTURE(NoteEngraverTestFixture, note_engraver_01)
    {
        //@01 - shape in block, no stem no flag
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note_regular, &doc));
        pNote->set_notated_pitch(k_step_C, k_octave_4, k_no_accidentals);
        pNote->set_note_type(k_whole);

        ScoreMeter meter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        EngraversMap storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage, 0, 0);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, k_clef_F4, 0, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != nullptr );
        CHECK( pShape->is_shape_note() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 1 );
        CHECK( (*it)->is_shape_notehead() );
        CHECK( pNote->get_computed_stem() == k_computed_stem_undecided );

        delete pNote;
        delete pShape;
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_HeadAndStem)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note_regular, &doc));
        pNote->set_notated_pitch(k_step_C, k_octave_4, k_no_accidentals);
        pNote->set_note_type(k_quarter);

        ScoreMeter meter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        EngraversMap storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage, 0, 0);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, k_clef_F4, 0, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != nullptr );
        CHECK( pShape->is_shape_note() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 2 );
        CHECK( (*it)->is_shape_notehead() );
        ++it;
        CHECK( (*it)->is_shape_stem() );

        delete pNote;
        delete pShape;
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_HeadStemAndFlag)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note_regular, &doc));
        pNote->set_notated_pitch(k_step_C, k_octave_4, k_no_accidentals);
        pNote->set_note_type(k_eighth);

        ScoreMeter meter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        EngraversMap storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage, 0, 0);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, k_clef_F4, 0, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != nullptr );
        CHECK( pShape->is_shape_note() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 3 );
        CHECK( (*it)->is_shape_notehead() );
        ++it;
        CHECK( (*it)->is_shape_stem() );
        ++it;
        CHECK( (*it)->is_shape_flag() );

        delete pNote;
        delete pShape;
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ShapeInBlockWithDot)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note_regular, &doc));
        pNote->set_notated_pitch(k_step_C, k_octave_4, k_no_accidentals);
        pNote->set_note_type(k_whole);
        pNote->set_dots(1);

        ScoreMeter meter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        EngraversMap storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage, 0, 0);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, k_clef_F4, 0, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != nullptr );
        CHECK( pShape->is_shape_note() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 2 );
        CHECK( (*it)->is_shape_notehead() );
        ++it;
        CHECK( (*it)->is_shape_dot() );

        delete pNote;
        delete pShape;
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_HeadStemFlagTwoDots)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note_regular, &doc));
        pNote->set_notated_pitch(k_step_C, k_octave_4, k_no_accidentals);
        pNote->set_note_type(k_eighth);
        pNote->set_dots(2);

        ScoreMeter meter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        EngraversMap storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage, 0, 0);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, k_clef_F4, 0, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != nullptr );
        CHECK( pShape->is_shape_note() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 5 );
        CHECK( (*it)->is_shape_notehead() );
        ++it;
        CHECK( (*it)->is_shape_dot() );
        ++it;
        CHECK( (*it)->is_shape_dot() );
        ++it;
        CHECK( (*it)->is_shape_stem() );
        ++it;
        CHECK( (*it)->is_shape_flag() );

        delete pNote;
        delete pShape;
    }


    // tuplet ----------------------------------------------------------------------------

//    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_TupletEngraverCreated)
//    {
//        create_tuplet();
//        TupletEngraver* pEngrv = dynamic_cast<TupletEngraver*>(m_pStorage->get_engraver(m_pTuplet));
//
//        CHECK( pEngrv != nullptr );
//        CHECK( pEngrv->get_start_noterest() == m_pNote1 );
//        CHECK( pEngrv->get_end_noterest() == m_pNote3 );
//
//        delete_tuplet();
//    }


}



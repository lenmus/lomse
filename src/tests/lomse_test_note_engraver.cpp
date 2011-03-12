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
#include "lomse_note_engraver.h"
#include "lomse_chord_engraver.h"
#include "lomse_tuplet_engraver.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"

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
    {
    }

    ~NoteEngraverTestFixture()    //TearDown fixture
    {
    }

    void create_chord(int step1, int step2, int step3=-1)
    {
        create_chord(step1, 4, step2, 4, step3, 4);
    }

    void create_chord(int step1, int octave1, int step2, int octave2,
                     int step3=-1, int octave3=-1)
    {
        m_pChord = new ImoChord();
        DtoNote dtoNote;
        dtoNote.set_step( step1 );
        dtoNote.set_octave( octave1 );
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(ImoNote::k_eighth);
        dtoNote.set_dots(0);
        m_pNote1 = new ImoNote(dtoNote);
        m_pNote1->set_in_chord(m_pChord);
        m_pChord->push_back(m_pNote1);

        dtoNote.set_step( step2 );
        dtoNote.set_octave( octave2 );
        m_pNote2 = new ImoNote(dtoNote);
        m_pNote2->set_in_chord(m_pChord);
        m_pChord->push_back(m_pNote2);

        if (step3 >= 0)
        {
            dtoNote.set_step( step3 );
            dtoNote.set_octave( octave3 );
            m_pNote3 = new ImoNote(dtoNote);
            m_pNote3->set_in_chord(m_pChord);
            m_pChord->push_back(m_pNote3);
        }
        else
            m_pNote3 = NULL;

        m_pMeter = new ScoreMeter(1, 1, 180.0f);
        m_pStorage = new ShapesStorage();
        m_pEngrv = new NoteEngraver(m_libraryScope, m_pMeter, m_pStorage);
        m_pShape1 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote1, 0, 0, ImoClef::k_G2, UPoint(10.0f, 15.0f)) );
        m_pShape2 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote2, 0, 0, ImoClef::k_G2, UPoint(10.0f, 15.0f)) );
        if (step3 >= 0)
            m_pShape3 =
                dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote3, 0, 0, ImoClef::k_G2, UPoint(10.0f, 15.0f)) );
        else
            m_pShape3 = NULL;
    }

    void delete_chord()
    {
        delete m_pMeter;
        delete m_pEngrv;
        delete m_pChord;
        delete m_pNote1;
        delete m_pNote2;
        delete m_pNote3;
        delete m_pShape1;
        delete m_pShape2;
        delete m_pShape3;
        delete m_pStorage;
    }

    void create_tuplet()
    {
        m_pTuplet = new ImoTuplet();
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(ImoNote::k_eighth);
        dtoNote.set_dots(0);
        m_pNote1 = new ImoNote(dtoNote);
        m_pNote1->set_tuplet(m_pTuplet);
        m_pTuplet->push_back(m_pNote1);

        dtoNote.set_step(2);
        m_pNote2 = new ImoNote(dtoNote);
        m_pNote2->set_tuplet(m_pTuplet);
        m_pTuplet->push_back(m_pNote2);

        dtoNote.set_step(4);
        m_pNote3 = new ImoNote(dtoNote);
        m_pNote3->set_tuplet(m_pTuplet);
        m_pTuplet->push_back(m_pNote3);

        m_pMeter = new ScoreMeter(1, 1, 180.0f);
        m_pStorage = new ShapesStorage();
        m_pEngrv = new NoteEngraver(m_libraryScope, m_pMeter, m_pStorage);
        m_pShape1 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote1, 0, 0, ImoClef::k_G2, UPoint(10.0f, 15.0f)) );
        m_pShape2 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote2, 0, 0, ImoClef::k_G2, UPoint(10.0f, 15.0f)) );
        m_pShape3 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote3, 0, 0, ImoClef::k_G2, UPoint(10.0f, 15.0f)) );
    }

    void delete_tuplet()
    {
        delete m_pMeter;
        delete m_pEngrv;
        delete m_pTuplet;
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
    ShapesStorage* m_pStorage;
    ImoTuplet* m_pTuplet;
    ImoChord* m_pChord;
    ImoNote* m_pNote1;
    ImoNote* m_pNote2;
    ImoNote* m_pNote3;
    GmoShapeNote* m_pShape1;
    GmoShapeNote* m_pShape2;
    GmoShapeNote* m_pShape3;

};

SUITE(NoteEngraverTest)
{

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ShapeInBlock)
    {
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(ImoNote::k_whole);
        ImoNote note(dtoNote);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, 0, 0, ImoClef::k_F4, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_note() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 1 );
        CHECK( (*it)->is_shape_notehead() );

        delete pShape;
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_HeadAndStem)
    {
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(ImoNote::k_quarter);
        ImoNote note(dtoNote);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, 0, 0, ImoClef::k_F4, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_note() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 2 );
        CHECK( (*it)->is_shape_notehead() );
        ++it;
        CHECK( (*it)->is_shape_stem() );

        delete pShape;
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_HeadStemAndFlag)
    {
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(ImoNote::k_eighth);
        ImoNote note(dtoNote);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, 0, 0, ImoClef::k_F4, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_note() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 3 );
        CHECK( (*it)->is_shape_notehead() );
        ++it;
        CHECK( (*it)->is_shape_stem() );
        ++it;
        CHECK( (*it)->is_shape_flag() );

        delete pShape;
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ShapeInBlockWithDot)
    {
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(ImoNote::k_whole);
        dtoNote.set_dots(1);
        ImoNote note(dtoNote);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, 0, 0, ImoClef::k_F4, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_note() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 2 );
        CHECK( (*it)->is_shape_notehead() );
        ++it;
        CHECK( (*it)->is_shape_dot() );

        delete pShape;
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_HeadStemFlagTwoDots)
    {
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(ImoNote::k_eighth);
        dtoNote.set_dots(2);
        ImoNote note(dtoNote);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, 0, 0, ImoClef::k_F4, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != NULL );
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

        delete pShape;
    }

    // chord ----------------------------------------------------------------------------

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordEngraverCreated)
    {
        create_chord(0, 2, 4);
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv != NULL );
        CHECK( pEngrv->get_notes().size() == 3 );
        CHECK( pEngrv->get_base_note() == m_pNote1 );

        delete_chord();
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordNotesSortedByPitch)
    {
        create_chord(4, 0, 2);
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv->get_base_note() == m_pNote1 );
        std::list<ChordEngraver::ChordNoteData*>& notes = pEngrv->get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->pNote == m_pNote2 );
        ++it;
        CHECK( (*it)->pNote == m_pNote3 );
        ++it;
        CHECK( (*it)->pNote == m_pNote1 );

        delete_chord();
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordStemDirectionRuleA1)
    {
        create_chord(5,4,  1,5);     //(a4,d5)
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv->is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordStemDirectionRuleA2)
    {
        create_chord(2,4,  0,5);     //(e4,c5)
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv->is_stem_up() == true );

        delete_chord();
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordStemDirectionRuleA3)
    {
        create_chord(4,4,  1,5);     //(g4.d5)
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv->is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordStemDirectionRuleB3Up)
    {
        create_chord(2,4,  4,4,  3,5);     //(e4, g4, f5)
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv->is_stem_up() == true );

        delete_chord();
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordStemDirectionRuleB3Down)
    {
        create_chord(2,4,  1,5,  3,5);     //(e4, d5, f5)
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv->is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordNoteheadReversedStemDown)
    {
        create_chord(5,4,  6,4,  1,5);     //(a4, b4, d5)
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv->is_stem_up() == false );
        std::list<ChordEngraver::ChordNoteData*>& notes = pEngrv->get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordNoteheadReversedStemDown2)
    {
        create_chord(5,4,  6,4,  0,5);     //(a4, b4, c5)
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv->is_stem_up() == false );
        std::list<ChordEngraver::ChordNoteData*>& notes = pEngrv->get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordNoteheadReversedStemUp)
    {
        create_chord(3,4,  4,4,  6,4);     //(f4, g4, b4)
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv->is_stem_up() == true );
        std::list<ChordEngraver::ChordNoteData*>& notes = pEngrv->get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ChordNoteheadReversedStemUp2)
    {
        create_chord(3,4,  4,4,  5,4);     //(f4, g4, a4)
        ChordEngraver* pEngrv = dynamic_cast<ChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv->is_stem_up() == true );
        std::list<ChordEngraver::ChordNoteData*>& notes = pEngrv->get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }

    // tuplet ----------------------------------------------------------------------------

//    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_TupletEngraverCreated)
//    {
//        create_tuplet();
//        TupletEngraver* pEngrv = dynamic_cast<TupletEngraver*>(m_pStorage->get_engraver(m_pTuplet));
//
//        CHECK( pEngrv != NULL );
//        CHECK( pEngrv->get_start_noterest() == m_pNote1 );
//        CHECK( pEngrv->get_end_noterest() == m_pNote3 );
//
//        delete_tuplet();
//    }


}



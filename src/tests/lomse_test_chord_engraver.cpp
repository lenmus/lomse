//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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
#include "lomse_note_engraver.h"
#include "lomse_chord_engraver.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"
#include "lomse_im_factory.h"

#include <cmath>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// Helper. Access to protected members
class MyChordEngraver : public ChordEngraver
{
public:
    MyChordEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter, int numNotes)
        : ChordEngraver(libraryScope, pScoreMeter, numNotes)
    {
    }

    inline ImoNote* my_get_min_note() { return get_min_note(); }
    inline ImoNote* my_get_max_note() { return get_max_note(); }
	inline ImoNote* my_get_base_note() { return get_base_note(); }
    inline bool my_is_stem_up() { return is_stem_up(); }
    inline std::list<ChordNoteData*>& my_get_notes() { return m_notes; }
    inline void my_decide_on_stem_direction() { decide_on_stem_direction(); }
    inline void my_layout_noteheads() { layout_noteheads(); }
    inline void my_align_noteheads() { align_noteheads(); }
    inline void my_arrange_notheads_to_avoid_collisions() { arrange_notheads_to_avoid_collisions(); }
    inline LUnits my_get_stem_width() { return m_stemWidth; }
    inline void my_add_stem_and_flag() { add_stem_and_flag(); }
    inline void my_set_anchor_offset() { set_anchor_offset(); }

};


//---------------------------------------------------------------------------------------
class ChordEngraverTestFixture
{
public:
    LibraryScope m_libraryScope;

    ChordEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pMeter(NULL)
        , m_pNoteEngrv(NULL)
        , m_pChord(NULL)
        , m_pNote1(NULL)
        , m_pNote2(NULL)
        , m_pNote3(NULL)
        , m_pShape1(NULL)
        , m_pShape2(NULL)
        , m_pShape3(NULL)
        , m_pStorage(NULL)
        , m_pChordEngrv(NULL)
        , m_pDoc(NULL)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ChordEngraverTestFixture()    //TearDown fixture
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

    void create_chord(int step1, int octave1, int step2, int octave2,
                     int step3=-1, int octave3=-1, int noteType=k_eighth)
    {
        delete_chord();
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pChord = static_cast<ImoChord*>( ImFactory::inject(k_imo_chord, m_pDoc) );

        m_pNote1 = ImFactory::inject_note(m_pDoc, step1, octave1, noteType, k_no_accidentals);
        m_pNote1->include_in_relation(m_pDoc, m_pChord);

        m_pNote2 = ImFactory::inject_note(m_pDoc, step2, octave2, noteType, k_no_accidentals);
        m_pNote2->include_in_relation(m_pDoc, m_pChord);

        if (step3 >= 0)
        {
            m_pNote3 = ImFactory::inject_note(m_pDoc, step3, octave3, noteType, k_no_accidentals);
            m_pNote3->include_in_relation(m_pDoc, m_pChord);
        }
        else
            m_pNote3 = NULL;

        m_pMeter = LOMSE_NEW ScoreMeter(1, 1, 180.0f);
        m_pStorage = LOMSE_NEW ShapesStorage();
        m_pNoteEngrv = LOMSE_NEW NoteEngraver(m_libraryScope, m_pMeter, m_pStorage, 0, 0);
        m_pShape1 =
            dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote1, k_clef_G2, UPoint(10.0f, 15.0f)) );
        m_pShape2 =
            dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote2, k_clef_G2, UPoint(10.0f, 15.0f)) );
        if (step3 >= 0)
            m_pShape3 =
                dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote3, k_clef_G2, UPoint(10.0f, 15.0f)) );
        else
            m_pShape3 = NULL;
    }

    void delete_chord()
    {
        delete m_pMeter;
        delete m_pNoteEngrv;
        delete m_pNote1;
        delete m_pNote2;
        delete m_pNote3;
        delete m_pShape1;
        delete m_pShape2;
        delete m_pShape3;
        delete m_pStorage;
        delete m_pChordEngrv;
        delete m_pDoc;
    }

    void engrave_chord()
    {
        int iInstr = 0;
        int iStaff = 0;
        int iSystem = 0;
        int iCol = 0;

        int numNotes = (m_pShape3 ? 3 : 2);

        //first note
        m_pChordEngrv = LOMSE_NEW MyChordEngraver(m_libraryScope, m_pMeter, numNotes);
        m_pChordEngrv->set_start_staffobj(m_pChord, m_pNote1, m_pShape1, iInstr, iStaff,
                                          iSystem, iCol, 0.0f, 0.0f, 0.0f);
        m_pStorage->save_engraver(m_pChordEngrv, m_pChord);

        if (numNotes == 3)
        {
            //second note
            m_pChordEngrv->set_middle_staffobj(m_pChord, m_pNote2, m_pShape2, iInstr,
                                        iStaff, iSystem, iCol,
                                        0.0f, 0.0f, 0.0f);
            //last note
            m_pChordEngrv->set_end_staffobj(m_pChord, m_pNote3, m_pShape3, iInstr,
                                     iStaff, iSystem, iCol,
                                     0.0f, 0.0f, 0.0f);
        }
        else
            m_pChordEngrv->set_end_staffobj(m_pChord, m_pNote2, m_pShape2, iInstr,
                                     iStaff, iSystem, iCol,
                                     0.0f, 0.0f, 0.0f);
    }


    ScoreMeter* m_pMeter;
    NoteEngraver* m_pNoteEngrv;
    ImoChord* m_pChord;
    ImoNote* m_pNote1;
    ImoNote* m_pNote2;
    ImoNote* m_pNote3;
    GmoShapeNote* m_pShape1;
    GmoShapeNote* m_pShape2;
    GmoShapeNote* m_pShape3;
    ShapesStorage* m_pStorage;
    MyChordEngraver* m_pChordEngrv;
    Document* m_pDoc;

};

SUITE(ChordEngraverTest)
{

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_CreateShapes)
    {

        create_chord(0,4,  2,4,  4,4);
        engrave_chord();

        m_pChordEngrv->create_shapes();
        MyChordEngraver* pEngrv = dynamic_cast<MyChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv != NULL );
        CHECK( pEngrv->my_get_notes().size() == 3 );
        CHECK( pEngrv->my_get_base_note() == m_pNote1 );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_NotesSortedByPitch)
    {
        create_chord(4,4,  0,4,  2,4);
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_get_base_note() == m_pNote1 );
        std::list<ChordEngraver::ChordNoteData*>& notes = m_pChordEngrv->my_get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->pNote == m_pNote2 );
        ++it;
        CHECK( (*it)->pNote == m_pNote3 );
        ++it;
        CHECK( (*it)->pNote == m_pNote1 );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_StemDirectionRuleA1)
    {
        create_chord(5,4,  1,5);     //(a4,d5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_StemDirectionRuleA2)
    {
        create_chord(2,4,  0,5);     //(e4,c5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == true );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_StemDirectionRuleA3)
    {
        create_chord(4,4,  1,5);     //(g4.d5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_StemDirectionRuleB3Up)
    {
        create_chord(2,4,  4,4,  3,5);     //(e4, g4, f5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == true );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_StemDirectionRuleB3Down)
    {
        create_chord(2,4,  1,5,  3,5);     //(e4, d5, f5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_NoteheadReversedStemDown)
    {
        create_chord(5,4,  6,4,  1,5);     //(a4, b4, d5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );
        std::list<ChordEngraver::ChordNoteData*>& notes = m_pChordEngrv->my_get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_NoteheadReversedStemDown2)
    {
        create_chord(5,4,  6,4,  0,5);     //(a4, b4, c5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );
        std::list<ChordEngraver::ChordNoteData*>& notes = m_pChordEngrv->my_get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_NoteheadReversedStemUp)
    {
        create_chord(3,4,  4,4,  6,4);     //(f4, g4, b4)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == true );
        std::list<ChordEngraver::ChordNoteData*>& notes = m_pChordEngrv->my_get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_NoteheadReversedStemUp2)
    {
        create_chord(3,4,  4,4,  5,4);     //(f4, g4, a4)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == true );
        std::list<ChordEngraver::ChordNoteData*>& notes = m_pChordEngrv->my_get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }


    // Engrave chord. Case 1:  stem up, no accidentals, no reversed note ----------------

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_AligneNoteheads_C1)
    {
        create_chord(2,4,  4,4);     //(e4,g4)
        engrave_chord();
        //cout << "note1. left=" << m_pShape1->get_notehead_left()
        //     << ", width=" << m_pShape1->get_notehead_width() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left()
        //     << ", width=" << m_pShape2->get_notehead_width() << endl;
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 219.0f) );

        //cout << "note1. left=" << m_pShape1->get_notehead_left() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left() << endl;

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_ArrangeNoteheads_C1)
    {
        create_chord(2,4,  4,4);     //(e4,g4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pChordEngrv->my_get_stem_width(), 21.6f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_AddStemAndFlag_C1)
    {
        // adding stem doesn't alter positions

        create_chord(2,4,  4,4);     //(e4,g4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        m_pChordEngrv->my_add_stem_and_flag();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_SetOffset_C1)
    {
        // set offset doesn' alter positions

        create_chord(2,4,  4,4);     //(e4,g4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();
        m_pChordEngrv->my_add_stem_and_flag();

        m_pChordEngrv->my_set_anchor_offset();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 219.0f) );

        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape2->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_EngraveChord_C1)
    {
        create_chord(2,4,  4,4);     //(e4,g4)
        engrave_chord();

        m_pChordEngrv->create_shapes();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 219.0f) );

        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape2->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 219.0f) );

        //cout << m_pShape1->get_stem_left() << endl;
        CHECK ( is_equal(m_pShape1->get_stem_left(), 207.4f) );
        CHECK ( is_equal(m_pShape1->get_stem_width(), 21.6f) );

        delete_chord();
    }


    // Engrave chord. Case 2:  stem up, no accidentals, reversed note -------------------

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_AligneNoteheads_C2)
    {
        create_chord(2,4,  3,4);     //(e4,f4)
        engrave_chord();
        //cout << "note1. left=" << m_pShape1->get_notehead_left()
        //     << ", width=" << m_pShape1->get_notehead_width() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left()
        //     << ", width=" << m_pShape2->get_notehead_width() << endl;
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 219.0f) );

        //cout << "note1. left=" << m_pShape1->get_notehead_left() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left() << endl;

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_ArrangeNoteheads_C2)
    {
        // reversed notehead is shifted: left = 10 + 232 - 21.6

        create_chord(2,4,  3,4);     //(e4,f4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 207.4f) );
        CHECK ( is_equal(m_pChordEngrv->my_get_stem_width(), 21.6f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_AddStemAndFlag_C2)
    {
        // adding stem doesn't alter positions

        create_chord(2,4,  3,4);     //(e4,f4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        m_pChordEngrv->my_add_stem_and_flag();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 207.4f) );
        CHECK ( is_equal(m_pChordEngrv->my_get_stem_width(), 21.6f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_SetOffset_C2)
    {
        // set offset doesn' alter positions

        create_chord(2,4,  3,4);     //(e4,f4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();
        m_pChordEngrv->my_add_stem_and_flag();

        m_pChordEngrv->my_set_anchor_offset();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 219.0f) );

        //cout << "anchor offset: " << m_pShape2->get_anchor_offset() << endl;
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 207.4f) );
        CHECK ( is_equal(m_pShape2->get_anchor_offset(), 197.4f) );
        CHECK ( is_equal(m_pShape2->get_left(), 207.4f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_EngraveChord_C2)
    {
        create_chord(2,4,  3,4);     //(e4,f4)
        engrave_chord();

        m_pChordEngrv->create_shapes();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 219.0f) );

        CHECK ( is_equal(m_pShape2->get_notehead_left(), 207.4f) );
        CHECK ( is_equal(m_pShape2->get_anchor_offset(), 197.4f) );
        CHECK ( is_equal(m_pShape2->get_left(), 207.4f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 219.0f) );

        CHECK ( is_equal(m_pShape1->get_stem_left(), 207.4f) );
        CHECK ( is_equal(m_pShape1->get_stem_width(), 21.6f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_EngraveChord_Test00030)
    {
        create_chord(0,4,  2,4,  4,4,  k_whole);     //(c4,e4,g4)
        engrave_chord();

        m_pChordEngrv->create_shapes();

        //cout << "notehead width: " << m_pShape1->get_notehead_width() << endl;
        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 313.0f) );

        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape2->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 313.0f) );

        CHECK ( is_equal(m_pShape3->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape3->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape3->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape3->get_notehead_width(), 313.0f) );

        CHECK ( is_equal(m_pShape1->get_stem_left(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_stem_width(), 0.0f) );

        //cout << "note1. left=" << m_pShape1->get_notehead_left()
        //     << ", width=" << m_pShape1->get_notehead_width() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left()
        //     << ", width=" << m_pShape2->get_notehead_width() << endl;
        //cout << "note3. left=" << m_pShape3->get_notehead_left()
        //     << ", width=" << m_pShape3->get_notehead_width() << endl;
        //cout << "stem. left=" << m_pShape1->get_stem_left()
        //     << ", width=" << m_pShape1->get_stem_width() << endl;

        delete_chord();
    }

}



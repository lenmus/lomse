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
#include "private/lomse_document_p.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_engravers_map.h"
#include "lomse_im_factory.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_interactor.h"
#include "lomse_graphic_view.h"
#include "lomse_doorway.h"
#include "lomse_presenter.h"

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
        : ChordEngraver(libraryScope, pScoreMeter, numNotes, 21.0f, k_size_full)
    {
    }

    inline ImoNote* my_get_min_note() { return get_min_note(); }
    inline ImoNote* my_get_max_note() { return get_max_note(); }
	inline ImoNote* my_get_base_note() { return get_base_note(); }
    inline bool my_is_stem_up() { return is_stem_up(); }
    inline std::list<ChordNoteData*>& my_get_notes() { return m_notes; }
    inline void my_decide_stem_direction() { decide_stem_direction(); }
    inline void my_layout_noteheads() { layout_noteheads(); }
    inline void my_align_noteheads() { align_noteheads(); }
    inline void my_arrange_notheads_to_avoid_collisions() { arrange_notheads_to_avoid_collisions(); }
    inline LUnits my_get_stem_width() { return m_stemWidth; }
    inline void my_add_stem_and_flag() { add_stem_and_flag(); }
    inline void my_set_anchor_offset() { set_anchor_offset(); }
    inline void my_find_reference_notes() { find_reference_notes(); }

};


//---------------------------------------------------------------------------------------
class ChordEngraverTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;
    ScoreMeter* m_pMeter;
    NoteEngraver* m_pNoteEngrv;
    ImoChord* m_pChord;
    ImoNote* m_pNote1;
    ImoNote* m_pNote2;
    ImoNote* m_pNote3;
    GmoShapeNote* m_pShape1;
    GmoShapeNote* m_pShape2;
    GmoShapeNote* m_pShape3;
    EngraversMap* m_pStorage;
    MyChordEngraver* m_pChordEngrv;
    Document* m_pDoc;


    ChordEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pMeter(nullptr)
        , m_pNoteEngrv(nullptr)
        , m_pChord(nullptr)
        , m_pNote1(nullptr)
        , m_pNote2(nullptr)
        , m_pNote3(nullptr)
        , m_pShape1(nullptr)
        , m_pShape2(nullptr)
        , m_pShape3(nullptr)
        , m_pStorage(nullptr)
        , m_pChordEngrv(nullptr)
        , m_pDoc(nullptr)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ChordEngraverTestFixture()    //TearDown fixture
    {
    }

    //-----------------------------------------------------------------------------------
    void create_chord(int step1, int octave1, int step2, int octave2,
                     int step3=-1, int octave3=-1, int noteType=k_eighth)
    {
        delete_chord();
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pChord = static_cast<ImoChord*>( ImFactory::inject(k_imo_chord, m_pDoc) );

        m_pNote1 = ImFactory::inject_note(m_pDoc, step1, octave1, noteType, k_no_accidentals);
        m_pNote1->include_in_relation(m_pChord);

        m_pNote2 = ImFactory::inject_note(m_pDoc, step2, octave2, noteType, k_no_accidentals);
        m_pNote2->include_in_relation(m_pChord);

        if (step3 >= 0)
        {
            m_pNote3 = ImFactory::inject_note(m_pDoc, step3, octave3, noteType, k_no_accidentals);
            m_pNote3->include_in_relation(m_pChord);
        }
        else
            m_pNote3 = nullptr;

        m_pMeter = LOMSE_NEW ScoreMeter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        m_pStorage = LOMSE_NEW EngraversMap();
        m_pNoteEngrv = LOMSE_NEW NoteEngraver(m_libraryScope, m_pMeter, m_pStorage, 0, 0);
        m_pShape1 =
            dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote1, k_clef_G2, 0, UPoint(10.0f, 15.0f)) );
        m_pShape2 =
            dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote2, k_clef_G2, 0, UPoint(10.0f, 15.0f)) );
        if (step3 >= 0)
            m_pShape3 =
                dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote3, k_clef_G2, 0, UPoint(10.0f, 15.0f)) );
        else
            m_pShape3 = nullptr;
    }

    //-----------------------------------------------------------------------------------
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

    //-----------------------------------------------------------------------------------
    void load_notes_in_chord_engraver(ImoChord* pChord)
    {
        m_pChord = pChord;

        //engrave the notes (two or three)
        list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes = pChord->get_related_objects();
        list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it = notes.begin();
        m_pNote1 = pChord->get_start_note();
        ++it;
        m_pNote2 = static_cast<ImoNote*>((*it).first);
        m_pNote3 = (pChord->get_num_objects() == 3 ? pChord->get_end_note() : nullptr);

        m_pMeter = LOMSE_NEW ScoreMeter(nullptr, 1, 1, LOMSE_STAFF_LINE_SPACING);
        m_pStorage = LOMSE_NEW EngraversMap();
        m_pNoteEngrv = LOMSE_NEW NoteEngraver(m_libraryScope, m_pMeter, m_pStorage, 0, 0);
        m_pShape1 =
            dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote1, k_clef_G2, 0, UPoint(10.0f, 15.0f)) );
        m_pShape2 =
            dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote2, k_clef_G2, 0, UPoint(10.0f, 15.0f)) );
        if (m_pNote3)
            m_pShape3 =
                dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote3, k_clef_G2, 0, UPoint(10.0f, 15.0f)) );

        //engrave the chord
        load_notes_in_chord_engraver();

        //clear the notes, as they will be deleted with the score
        m_pNote1 = nullptr;
        m_pNote2 = nullptr;
        m_pNote3 = nullptr;
    }

    //-----------------------------------------------------------------------------------
    void load_notes_in_chord_engraver()
    {
        int iInstr = 0;
        int iStaff = 0;
        int iCol = 0;

        int numNotes = (m_pShape3 ? 3 : 2);

        //first note
        m_pChordEngrv = LOMSE_NEW MyChordEngraver(m_libraryScope, m_pMeter, numNotes);
        AuxObjContext aoc1(m_pNote1, m_pShape1, iInstr, iStaff, iCol, 0, nullptr, -1);
        m_pChordEngrv->set_start_staffobj(m_pChord, aoc1);
        m_pStorage->save_engraver(m_pChordEngrv, m_pChord);

        if (numNotes == 3)
        {
            //second note
            AuxObjContext aoc2(m_pNote2, m_pShape2, iInstr, iStaff, iCol, 0, nullptr, -1);
            m_pChordEngrv->set_middle_staffobj(m_pChord, aoc2);
            //last note
            AuxObjContext aoc3(m_pNote3, m_pShape3, iInstr, iStaff, iCol, 0, nullptr, -1);
            m_pChordEngrv->set_end_staffobj(m_pChord, aoc3);
        }
        else
        {
            AuxObjContext aoc2(m_pNote2, m_pShape2, iInstr, iStaff, iCol, 0, nullptr, -1);
            m_pChordEngrv->set_end_staffobj(m_pChord, aoc2);
        }
    }

    //-----------------------------------------------------------------------------------
    ImoChord* get_imochord_for_chord(int iChord, Document* pDoc)
    {
        ImoScore* pScore = static_cast<ImoScore*>(pDoc->get_content_item(0));
        StaffObjsCursor cursor(pScore);
        while(!cursor.is_end() && !cursor.get_staffobj()->is_note())
        {
            cursor.move_next();
        }
        if (cursor.is_end())
            return nullptr;

        int numChords = -1;
        while(!cursor.is_end())
        {
            if (cursor.get_staffobj()->is_note())
            {
                ImoNote* pNote = static_cast<ImoNote*>( cursor.get_staffobj() );
                if (pNote->is_in_chord() && pNote->is_start_of_chord())
                {
                    ++numChords;
                    if (numChords == iChord)
                        return pNote->get_chord();
                }
            }
            cursor.move_next();
        }
        return nullptr;
    }

    //-----------------------------------------------------------------------------------
    ImoNote* get_single_note_in_beam(int iNote, Document* pDoc)
    {
        ImoScore* pScore = static_cast<ImoScore*>(pDoc->get_content_item(0));
        StaffObjsCursor cursor(pScore);
        while(!cursor.is_end() && !cursor.get_staffobj()->is_note())
        {
            cursor.move_next();
        }
        if (cursor.is_end())
            return nullptr;

        int numNotes = -1;
        while(!cursor.is_end())
        {
            if (cursor.get_staffobj()->is_note())
            {
                ImoNote* pNote = static_cast<ImoNote*>( cursor.get_staffobj() );
                if (!pNote->is_in_chord())
                {
                    ++numNotes;
                    if (numNotes == iNote)
                        return pNote;
                }
            }
            cursor.move_next();
        }
        return nullptr;
    }

    //-----------------------------------------------------------------------------------
    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }



};

SUITE(ChordEngraverTest)
{

    //@00x - initial tests --------------------------------------------------------------

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_001)
    {
        //@001- engraver created. Stores the notes

        create_chord(0,4,  2,4,  4,4);
        load_notes_in_chord_engraver();

        m_pChordEngrv->create_shapes();
        MyChordEngraver* pEngrv = dynamic_cast<MyChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv != nullptr );
        CHECK( pEngrv->my_get_notes().size() == 3 );
        CHECK( pEngrv->my_get_base_note() == m_pNote1 );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_002)
    {
        //@002- notes are sorted by pitch

        create_chord(4,4,  0,4,  2,4);
        load_notes_in_chord_engraver();
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


    //@1xx -  Stem direction engraving rules --------------------------------------------

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_101)
    {
        //@101 - stem direction. rule a1

        create_chord(5,4,  1,5);     //(a4,d5)
        load_notes_in_chord_engraver();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_102)
    {
        //@102 - stem direction. rule a2

        create_chord(2,4,  0,5);     //(e4,c5)
        load_notes_in_chord_engraver();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == true );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_103)
    {
        //@ 103 - stem direction. rule a3

        create_chord(4,4,  1,5);     //(g4.d5)
        load_notes_in_chord_engraver();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_104)
    {
        //@104 - stem direction. rule b4 up

        create_chord(2,4,  4,4,  3,5);     //(e4, g4, f5)
        load_notes_in_chord_engraver();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == true );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_105)
    {
        //@105 - stem direction. rule b4 down

        create_chord(2,4,  1,5,  3,5);     //(e4, d5, f5)
        load_notes_in_chord_engraver();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );

        delete_chord();
    }


    //@2xx - noteheads engraving --------------------------------------------------------

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_201)
    {
        //@201 - notehead reversed. stem down

        create_chord(5,4,  6,4,  1,5);     //(a4, b4, d5)
        load_notes_in_chord_engraver();
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

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_202)
    {
        //@202 - notehead reversed to the other side. stem down

        create_chord(5,4,  6,4,  0,5);     //(a4, b4, c5)
        load_notes_in_chord_engraver();
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

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_203)
    {
        //@203 - notehead reversed. stem up

        create_chord(3,4,  4,4,  6,4);     //(f4, g4, b4)
        load_notes_in_chord_engraver();
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

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_204)
    {
        //@204 - notehead reversed to the other side. stem up

        create_chord(3,4,  4,4,  5,4);     //(f4, g4, a4)
        load_notes_in_chord_engraver();
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


    //@3xx - all shapes created and positioned ------------------------------------------

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_301)
    {
        //@301 - No reversed notes. Nothead shapes created and positioned

        create_chord(2,4,  4,4);     //(e4,g4)
        load_notes_in_chord_engraver();
        //cout << "note1. left=" << m_pShape1->get_notehead_left()
        //     << ", width=" << m_pShape1->get_notehead_width() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left()
        //     << ", width=" << m_pShape2->get_notehead_width() << endl;
        m_pChordEngrv->my_find_reference_notes();
        m_pChordEngrv->my_decide_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 219.0f) );

        //cout << "note1. left=" << m_pShape1->get_notehead_left() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left() << endl;

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_302)
    {
        //@302 - No reversed notes. all shapes created and positioned

        create_chord(2,4,  4,4);     //(e4,g4)
        load_notes_in_chord_engraver();
        m_pChordEngrv->my_find_reference_notes();
        m_pChordEngrv->my_decide_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pChordEngrv->my_get_stem_width(), 21.6f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_303)
    {
        //@303 - No reversed notes. adding stem doesn't alter positions

        create_chord(2,4,  4,4);     //(e4,g4)
        load_notes_in_chord_engraver();
        m_pChordEngrv->my_find_reference_notes();
        m_pChordEngrv->my_decide_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        m_pChordEngrv->my_add_stem_and_flag();

        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_304)
    {
        //304 - No reversed notes. set offset doesn' alter positions

        create_chord(2,4,  4,4);     //(e4,g4)
        load_notes_in_chord_engraver();
        m_pChordEngrv->my_find_reference_notes();
        m_pChordEngrv->my_decide_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();
        m_pChordEngrv->my_add_stem_and_flag();

        m_pChordEngrv->my_set_anchor_offset();

        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 219.0f) );

        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_305)
    {
        //@305 - No reversed notes. totally engrave the chord

        create_chord(2,4,  4,4);     //(e4,g4)
        load_notes_in_chord_engraver();

        m_pChordEngrv->create_shapes();

        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 219.0f) );

        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 219.0f) );

        //cout << m_pShape1->get_stem_left() << endl;
        CHECK ( is_equal_pos(m_pShape1->get_stem_left(), 207.4f) );
        CHECK ( is_equal_pos(m_pShape1->get_stem_width(), 21.6f) );

        delete_chord();
    }


    // Engrave chord. Case 2:  stem up, no accidentals, reversed note -------------------

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_311)
    {
        //@311 - Reversed notes. noteheads correctly aligned

        create_chord(2,4,  3,4);     //(e4,f4)
        load_notes_in_chord_engraver();
        //cout << "note1. left=" << m_pShape1->get_notehead_left()
        //     << ", width=" << m_pShape1->get_notehead_width() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left()
        //     << ", width=" << m_pShape2->get_notehead_width() << endl;
        m_pChordEngrv->my_find_reference_notes();
        m_pChordEngrv->my_decide_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 219.0f) );

        //cout << "note1. left=" << m_pShape1->get_notehead_left() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left() << endl;

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_312)
    {
        //312 - Reversed notes. reversed notehead is shifted: left = 10 + 232 - 21.6

        create_chord(2,4,  3,4);     //(e4,f4)
        load_notes_in_chord_engraver();
        m_pChordEngrv->my_find_reference_notes();
        m_pChordEngrv->my_decide_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 207.4f) );
        CHECK ( is_equal_pos(m_pChordEngrv->my_get_stem_width(), 21.6f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_313)
    {
        //313 - Reversed notes. adding stem doesn't alter positions

        create_chord(2,4,  3,4);     //(e4,f4)
        load_notes_in_chord_engraver();
        m_pChordEngrv->my_find_reference_notes();
        m_pChordEngrv->my_decide_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        m_pChordEngrv->my_add_stem_and_flag();

        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 207.4f) );
        CHECK ( is_equal_pos(m_pChordEngrv->my_get_stem_width(), 21.6f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 219.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_314)
    {
        //314 - Reversed notes. set offset doesn' alter positions

        create_chord(2,4,  3,4);     //(e4,f4)
        load_notes_in_chord_engraver();
        m_pChordEngrv->my_find_reference_notes();
        m_pChordEngrv->my_decide_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();
        m_pChordEngrv->my_add_stem_and_flag();

        m_pChordEngrv->my_set_anchor_offset();

        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 219.0f) );

        //cout << "anchor offset: " << m_pShape2->get_anchor_offset() << endl;
        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 207.4f) );
        CHECK ( is_equal_pos(m_pShape2->get_anchor_offset(), 197.4f) );
        CHECK ( is_equal_pos(m_pShape2->get_left(), 207.4f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 219.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_315)
    {
        //@315 - Reversed notes. totally engrave the chord

        create_chord(2,4,  3,4);     //(e4,f4)
        load_notes_in_chord_engraver();

        m_pChordEngrv->create_shapes();

        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 219.0f) );

        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 207.4f) );
        CHECK ( is_equal_pos(m_pShape2->get_anchor_offset(), 197.4f) );
        CHECK ( is_equal_pos(m_pShape2->get_left(), 207.4f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 219.0f) );

        CHECK ( is_equal_pos(m_pShape1->get_stem_left(), 207.4f) );
        CHECK ( is_equal_pos(m_pShape1->get_stem_width(), 21.6f) );

        delete_chord();
    }

    //@4xx - beamed chords --------------------------------------------------------------


    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_401)
    {
        //@401 - stems direction updated in ImoChord. Up

        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.1)(instrument (musicData (clef G)"
            "(chord (n f4 q)(n a4 q))"
            ")))", Document::k_format_ldp);
        ImoChord* pChord = get_imochord_for_chord(0, &doc);
        CHECK( pChord != nullptr );

        load_notes_in_chord_engraver(pChord);
        m_pChordEngrv->create_shapes();

        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == true );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_402)
    {
        //@402 - stems direction updated in ImoChord. Down

        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.1)(instrument (musicData (clef G)"
            "(chord (n a4 q)(n e5 q))"
            ")))", Document::k_format_ldp);
        ImoChord* pChord = get_imochord_for_chord(0, &doc);
        CHECK( pChord != nullptr );

        load_notes_in_chord_engraver(pChord);
        m_pChordEngrv->create_shapes();

        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );

        delete_chord();
    }

//    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_403)
//    {
//        //@403 - stems direction forced by beam when beam position is forced
//
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "unit-tests/chords/01-beam-forced-down.xml",
//                      Document::k_format_mxl);
//
//        ImoChord* pChord = get_imochord_for_chord(0, &doc);
//        CHECK( pChord != nullptr );
//
//        load_notes_in_chord_engraver(pChord);
//        m_pChordEngrv->create_shapes();
//
//        CHECK( pChord != nullptr );
//        CHECK( pChord->is_stem_direction_decided() == true );
//        CHECK( pChord->is_stem_up() == false );
//
//        delete_chord();
//    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_410)
    {
        //@410 - collect_chord_notes()

        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.1)(instrument (musicData (clef G)"
            "(chord (n d4 e (beam 1 +))(n g4 e))"
            "(chord (n f4 e (beam 1 -))(n a4 e)(n c5 e))"
            ")))", Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoChord* pChord1 = get_imochord_for_chord(1, &doc);
        CHECK( pChord1 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs(1, k_clef_G2);
        BeamedChordHelper helper(pBeam, &clefs);
        vector<ImoNote*> chordNotes = helper.collect_chord_notes(pChord1);

        CHECK( chordNotes.size() == 3 );
        ImoNote* pNote = chordNotes[0];
        CHECK( pNote->get_step() == k_step_F );
        CHECK( pNote->get_octave() == 4 );
        pNote = chordNotes[1];
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->get_octave() == 4 );
        pNote = chordNotes[2];
        CHECK( pNote->get_step() == k_step_C );
        CHECK( pNote->get_octave() == 5 );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_415)
    {
        //@415 - determine_mean_pos_on_staff()

        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.1)(instrument (musicData (clef G)"
            "(chord (n a4 q)(n e5 q))"
            ")))", Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );

        vector<int> clefs(1, k_clef_G2);
        BeamedChordHelper helper(nullptr, &clefs);
        vector<ImoNote*> notes = helper.collect_chord_notes(pChord0);
        int meanPos = helper.determine_mean_pos_on_staff(notes, &clefs);

        CHECK( meanPos == 7 );      // a4,e5 = (5+9)/2 = 7
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_420)
    {
        //@420 - find_applicable_clefs()

        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.1)(instrument (musicData (clef G)"
            "(chord (n d4 e (beam 1 +))(n g4 e))"
            "(clef F4)"
            "(chord (n f3 e (beam 1 -))(n a3 e)(n c4 e))"
            ")))", Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoChord* pChord1 = get_imochord_for_chord(1, &doc);
        CHECK( pChord1 != nullptr );
        ImoNote* pBase1 = pChord1->get_start_note();
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs(1, k_clef_G2);
        BeamedChordHelper helper(pBeam, &clefs);
        helper.find_applicable_clefs(pBase1, pBase0, &clefs);

        CHECK( clefs[0] == k_clef_F4 );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_425)
    {
        //@425 - compute_stem_direction_for_chord()

        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.1)(instrument (musicData (clef G)"
            "(chord (n d4 e (beam 1 +))(n g4 e))"
            "(clef F4)"
            "(chord (n f3 e (beam 1 -))(n a3 e)(n c4 e))"
            ")))", Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoChord* pChord1 = get_imochord_for_chord(1, &doc);
        CHECK( pChord1 != nullptr );
        ImoNote* pBase1 = pChord1->get_start_note();
        CHECK( pChord1->is_stem_direction_decided() == false );
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs(1, k_clef_G2);
        BeamedChordHelper helper(pBeam, &clefs);
        helper.compute_stem_direction_for_chord(pBase1, pBase0, &clefs);

        CHECK( clefs[0] == k_clef_F4 );
        CHECK( pChord1->is_stem_direction_decided() == true );
        CHECK( pChord1->is_stem_up() == false );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_430)
    {
        //@430 - compute_stem_direction_for_note(). Not forced

        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.1)(instrument (musicData (clef G)"
            "(chord (n d4 e (beam 1 +))(n g4 e))"
            "(clef F4)"
            "(n f3 e (beam 1 =))"
            "(chord (n f3 e (beam 1 -))(n a3 e)(n c4 e))"
            ")))", Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoNote* pNote = get_single_note_in_beam(0, &doc);
        CHECK( pNote != nullptr );
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs(1, k_clef_G2);
        BeamedChordHelper helper(pBeam, &clefs);
        bool fDown = helper.compute_stem_direction_for_note(pNote, pBase0, &clefs);

        CHECK( clefs[0] == k_clef_F4 );
        CHECK( fDown == true );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_431)
    {
        //@431 - compute_stem_direction_for_note(). Forced

        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.1)(instrument (musicData (clef G)"
            "(chord (n d4 e (beam 1 +))(n g4 e))"
            "(clef F4)"
            "(n f3 e (stem up)(beam 1 =))"
            "(chord (n f3 e (beam 1 -))(n a3 e)(n c4 e))"
            ")))", Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoNote* pNote = get_single_note_in_beam(0, &doc);
        CHECK( pNote != nullptr );
        ImoBeam* pBeam = pNote->get_beam();

        vector<int> clefs(1, k_clef_G2);
        BeamedChordHelper helper(pBeam, &clefs);
        bool fDown = helper.compute_stem_direction_for_note(pNote, pBase0, &clefs);

        CHECK( clefs[0] == k_clef_F4 );
        CHECK( fDown == false );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_435)
    {
        //@435 - compute_stems_direction() works when only chords

        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.1)(instrument (musicData (clef G)"
            "(chord (n d4 e (beam 1 +))(n g4 e))"
            "(clef F4)"
            "(chord (n f3 e (beam 1 -))(n a3 e)(n c4 e))"
            ")))", Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs = {k_clef_G2, k_clef_F4};
        BeamedChordHelper helper(pBeam, &clefs);
        helper.compute_stems_directions();

        CHECK( pChord0->is_stem_direction_decided() == true );
        CHECK( pChord0->is_stem_up() == false );
        ImoChord* pChord1 = get_imochord_for_chord(1, &doc);
        CHECK( pChord1 != nullptr );
        CHECK( pChord1->is_stem_direction_decided() == true );
        CHECK( pChord1->is_stem_up() == false );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_436)
    {
        //@436 - compute_stems_direction() when chords and notes.
        //       It works if the note forces stems down

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/chords/" +
            "436-beamed-chord-with-note-that-forces-down.lms",
            Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs = {k_clef_G2, k_clef_F4};
        BeamedChordHelper helper(pBeam, &clefs);
        helper.compute_stems_directions();

        CHECK( pChord0->is_stem_direction_decided() == true );
        CHECK( pChord0->is_stem_up() == false );
        ImoChord* pChord1 = get_imochord_for_chord(1, &doc);
        CHECK( pChord1 != nullptr );
        CHECK( pChord1->is_stem_direction_decided() == true );
        CHECK( pChord1->is_stem_up() == false );

        vector<int>* pStemsDir = pBeam->get_stems_direction();
        CHECK( pStemsDir->size() == 3 );
        CHECK( pStemsDir->at(0) == k_computed_stem_down );
        CHECK( pStemsDir->at(1) == k_computed_stem_down );
        CHECK( pStemsDir->at(2) == k_computed_stem_down );
//        cout << test_name() << ", stems=" << pStemsDir->at(0)
//             << ", " << pStemsDir->at(1) << ", " << pStemsDir->at(2) << endl;
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_437)
    {
        //@437 - compute_stems_direction() works when note at start.
        //       It works if the note forces stems down

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/chords/" +
            "437-beamed-chord-with-note-at-start-forces-down.lms",
            Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs = {k_clef_G2, k_clef_F4};
        BeamedChordHelper helper(pBeam, &clefs);
        helper.compute_stems_directions();

        CHECK( pChord0->is_stem_direction_decided() == true );
        CHECK( pChord0->is_stem_up() == false );
        ImoChord* pChord1 = get_imochord_for_chord(1, &doc);
        CHECK( pChord1 != nullptr );
        CHECK( pChord1->is_stem_direction_decided() == true );
        CHECK( pChord1->is_stem_up() == false );

        vector<int>* pStemsDir = pBeam->get_stems_direction();
        CHECK( pStemsDir->size() == 3 );
        CHECK( pStemsDir->at(0) == k_computed_stem_down );
        CHECK( pStemsDir->at(1) == k_computed_stem_down );
        CHECK( pStemsDir->at(2) == k_computed_stem_down );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_440)
    {
        //@440 - beamed chord invokes compute_stems_directions()
        //       chord: d4,g4 - f4,c5

        LomseDoorway doorway;
        doorway.init_library(k_pix_format_rgba32, 96);
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        Presenter* pPresenter = doorway.open_document(k_view_vertical_book,
            m_scores_path + "unit-tests/chords/440-beamed-chord-both-up.lms");
        Document* pDoc = pPresenter->get_document_raw_ptr();
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);
        pIntor->get_graphic_model();        //force to engrave the score

        ImoChord* pChord0 = get_imochord_for_chord(0, pDoc);
        CHECK( pChord0 != nullptr );
        ImoChord* pChord1 = get_imochord_for_chord(1, pDoc);
        CHECK( pChord1 != nullptr );

        CHECK( pChord0->is_stem_direction_decided() == true );
        CHECK( pChord0->is_stem_up() == true );
        CHECK( pChord1->is_stem_direction_decided() == true );
        CHECK( pChord1->is_stem_up() == true );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_460)
    {
        //@460 - single staff, furthest note: stems down

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/chords/" +
            "460-beamed-chord-single-staff-furthest-note.lms",
            Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs = {k_clef_G2, k_clef_F4};
        BeamedChordHelper helper(pBeam, &clefs);
        helper.compute_stems_directions();

        CHECK( pChord0->is_stem_direction_decided() == true );
        CHECK( pChord0->is_stem_up() == false );
        ImoChord* pChord = get_imochord_for_chord(1, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
        pChord = get_imochord_for_chord(2, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
        pChord = get_imochord_for_chord(2, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_461)
    {
        //@461 - two staves, chords on staff 1. furthest note: stems down

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/chords/" +
            "461-beamed-chord-two-staves-chors-on-staff1.lms",
            Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs = {k_clef_G2, k_clef_F4};
        BeamedChordHelper helper(pBeam, &clefs);
        helper.compute_stems_directions();

        CHECK( pChord0->is_stem_direction_decided() == true );
        CHECK( pChord0->is_stem_up() == false );
        ImoChord* pChord = get_imochord_for_chord(1, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
        pChord = get_imochord_for_chord(2, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
        pChord = get_imochord_for_chord(2, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_462)
    {
        //@462 - two staves, chords on staff 2. furthest note: stems down

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/chords/" +
            "462-beamed-chord-two-staves-chors-on-staff2.lms",
            Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs = {k_clef_G2, k_clef_F4};
        BeamedChordHelper helper(pBeam, &clefs);
        helper.compute_stems_directions();

        CHECK( pChord0->is_stem_direction_decided() == true );
        CHECK( pChord0->is_stem_up() == false );
        ImoChord* pChord = get_imochord_for_chord(1, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
        pChord = get_imochord_for_chord(2, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
        pChord = get_imochord_for_chord(2, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_463)
    {
        //@463 - on both staves, furthest note: stems down

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/chords/" +
            "463-beamed-chord-on-both-staves.lms",
            Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs = {k_clef_G2, k_clef_F4};
        BeamedChordHelper helper(pBeam, &clefs);
        helper.compute_stems_directions();

        CHECK( pChord0->is_stem_direction_decided() == true );
        CHECK( pChord0->is_stem_up() == false );
        ImoChord* pChord = get_imochord_for_chord(1, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
        pChord = get_imochord_for_chord(2, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
        pChord = get_imochord_for_chord(2, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
    }

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_464)
    {
        //@464 - cross-staff, furthest note: stems down

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/chords/" +
            "464-beamed-chord-cross-staff.lms",
            Document::k_format_ldp);

        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
        CHECK( pChord0 != nullptr );
        ImoNote* pBase0 = pChord0->get_start_note();
        ImoBeam* pBeam = pBase0->get_beam();
        CHECK( pBeam != nullptr );

        vector<int> clefs = {k_clef_G2, k_clef_F4};
        BeamedChordHelper helper(pBeam, &clefs);
        helper.compute_stems_directions();

        CHECK( pChord0->is_stem_direction_decided() == true );
        CHECK( pChord0->is_stem_up() == false );
        ImoChord* pChord = get_imochord_for_chord(1, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
        pChord = get_imochord_for_chord(2, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );
        pChord = get_imochord_for_chord(2, &doc);
        CHECK( pChord != nullptr );
        CHECK( pChord->is_stem_direction_decided() == true );
        CHECK( pChord->is_stem_up() == false );

        vector<int>* pStemsDir = pBeam->get_stems_direction();
        CHECK( pStemsDir->size() == 4 );
        CHECK( pStemsDir->at(0) == k_computed_stem_down );
        CHECK( pStemsDir->at(1) == k_computed_stem_down );
        CHECK( pStemsDir->at(2) == k_computed_stem_down );
        CHECK( pStemsDir->at(2) == k_computed_stem_down );
//        cout << test_name() << ", stems=" << pStemsDir->at(0)
//             << ", " << pStemsDir->at(1) << ", " << pStemsDir->at(2)
//             << ", " << pStemsDir->at(3) << endl;
    }

//    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_465)
//    {
//        //@465 - also contains single notes.
//
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "unit-tests/chords/" +
//            "464-beamed-chord-cross-staff.lms",
//            Document::k_format_ldp);
//
//        ImoChord* pChord0 = get_imochord_for_chord(0, &doc);
//        CHECK( pChord0 != nullptr );
//        ImoNote* pBase0 = pChord0->get_start_note();
//        ImoBeam* pBeam = pBase0->get_beam();
//        CHECK( pBeam != nullptr );
//
//        vector<int> clefs = {k_clef_G2, k_clef_F4};
//        BeamedChordHelper helper(clefs.size());
//        helper.compute_stems_directions(pBeam, &clefs);
//
//        CHECK( pChord0->is_stem_direction_decided() == true );
//        CHECK( pChord0->is_stem_up() == false );
//        ImoChord* pChord = get_imochord_for_chord(1, &doc);
//        CHECK( pChord != nullptr );
//        CHECK( pChord->is_stem_direction_decided() == true );
//        CHECK( pChord->is_stem_up() == false );
//        pChord = get_imochord_for_chord(2, &doc);
//        CHECK( pChord != nullptr );
//        CHECK( pChord->is_stem_direction_decided() == true );
//        CHECK( pChord->is_stem_up() == false );
//        pChord = get_imochord_for_chord(2, &doc);
//        CHECK( pChord != nullptr );
//        CHECK( pChord->is_stem_direction_decided() == true );
//        CHECK( pChord->is_stem_up() == false );
//    }


    //@9xx - other tests ----------------------------------------------------------------

    TEST_FIXTURE(ChordEngraverTestFixture, chord_engraver_901)
    {
        //@901 - bug in regression score

        create_chord(0,4,  2,4,  4,4,  k_whole);     //(c4,e4,g4)
        load_notes_in_chord_engraver();

        m_pChordEngrv->create_shapes();

        //cout << "notehead width: " << m_pShape1->get_notehead_width() << endl;
        CHECK ( is_equal_pos(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_notehead_width(), 313.0f) );

        CHECK ( is_equal_pos(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape2->get_notehead_width(), 313.0f) );

        CHECK ( is_equal_pos(m_pShape3->get_notehead_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape3->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal_pos(m_pShape3->get_left(), 10.0f) );
        CHECK ( is_equal_pos(m_pShape3->get_notehead_width(), 313.0f) );

        CHECK ( is_equal_pos(m_pShape1->get_stem_left(), 0.0f) );
        CHECK ( is_equal_pos(m_pShape1->get_stem_width(), 0.0f) );

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



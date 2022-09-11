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
#include "lomse_beam_engraver.h"
#include "lomse_note_engraver.h"
#include "private/lomse_document_p.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_shape_beam.h"
#include "lomse_score_meter.h"
#include "lomse_engravers_map.h"
#include "lomse_ldp_analyser.h"
#include "lomse_im_factory.h"
#include "lomse_document_layouter.h"
#include "lomse_staffobjs_table.h"
#include "lomse_graphical_model.h"
#include "lomse_instrument_engraver.h"

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

    inline void my_decide_stems_direction() { decide_stems_direction(); }
    inline bool my_stems_forced() { return m_fStemForced; }
    inline bool my_stems_up() { return m_fStemsUp; }
    inline int my_get_beam_position() { return m_beamPos; }
    inline bool my_is_cross_staff() { return m_fCrossStaff; }
    inline bool my_is_double_stemmed() { return m_fDoubleStemmed; }
    inline int my_get_max_staff() { return m_maxStaff; }
    inline int my_get_min_staff() { return m_minStaff; }
    inline int my_get_num_notes() { return m_numNotes; }
    inline bool my_has_repeated_pattern_of_pitches() {
        return has_repeated_pattern_of_pitches(m_note);
    }
    inline bool my_check_all_notes_outside_first_ledger_line() {
        return check_all_notes_outside_first_ledger_line();
    }
    inline void my_determine_number_of_beam_levels() {
        determine_number_of_beam_levels();
    }

    float my_determine_slant_direction_for_cross_double_stemmed() {
        vector<GmoShapeNote*> upNoteShapes;
        vector<GmoShapeNote*> downNoteShapes;
        list< pair<ImoNoteRest*, GmoShape*> >::iterator it;
        for(it = m_noteRests.begin(); it != m_noteRests.end(); ++it)
        {
            ImoNoteRest* pNR = (*it).first;
            if (pNR->is_note())
            {
                GmoShapeNote* pShapeNote = static_cast<GmoShapeNote*>((*it).second);
                if (pShapeNote->is_up())
                    upNoteShapes.push_back(pShapeNote);
                else
                    downNoteShapes.push_back(pShapeNote);
            }
        }
        return determine_slant_direction_for_cross_double_stemmed(upNoteShapes,
                                                                        downNoteShapes);
    }

    void my_determine_segments(ImoBeam* pBeam, std::vector<SegmentData>* pSegs) {
        determine_segments(pBeam, pSegs);
    }
    void my_classify_segments(std::vector<SegmentData>* pSegs) {
        classify_segments(pSegs);
    }
    void my_position_segments(std::vector<SegmentData>* pSegs) {
        position_segments(pSegs);
    }


};


//---------------------------------------------------------------------------------------
class BeamEngraverTestFixture
{
public:
    LibraryScope    m_libraryScope;
    ImoScore*       m_pScore;
    ScoreMeter*     m_pMeter;
    EngraversMap*   m_pStorage;
    NoteEngraver*   m_pNoteEngrv;
    MyBeamEngraver* m_pBeamEngrv;
    GmoShapeBeam*   m_pBeamShape;
    Document*       m_pDoc;
    DocLayouter*    m_pDocLayouter;
    GraphicModel*   m_pGModel;
    std::vector<GmoShapeNote*> m_shapes;

    BeamEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pScore(nullptr)
        , m_pMeter(nullptr)
        , m_pStorage(nullptr)
        , m_pNoteEngrv(nullptr)
        , m_pBeamEngrv(nullptr)
        , m_pBeamShape(nullptr)
        , m_pDoc(nullptr)
        , m_pDocLayouter(nullptr)
        , m_pGModel(nullptr)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~BeamEngraverTestFixture()    //TearDown fixture
    {
    }

    //-----------------------------------------------------------------------------------
    ImoBeam* create_beam(Document& doc, const string& src)
    {
        string ldp = "(score (vers 2.0)(instrument (musicData (clef G)";
        ldp += src;
        ldp += ")))";

        doc.from_string(ldp);
        m_pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoNote* pNote1 = static_cast<ImoNote*>( pMD->get_child(1) );
        return dynamic_cast<ImoBeam*>( pNote1->get_relation(0) );
    }

    //-----------------------------------------------------------------------------------
    ImoBeam* create_beam_two_staves(Document& doc, const string& src)
    {
        string ldp = "(score (vers 2.0)(instrument (staves 2)"
                     "(musicData (clef G p1)(clef F4 p2)";
        ldp += src;
        ldp += ")))";

        doc.from_string(ldp);
        m_pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        //child 0 = clef G, child 1= clef F4
        ImoNote* pNote1 = static_cast<ImoNote*>( pMD->get_child(2) );
        return dynamic_cast<ImoBeam*>( pNote1->get_relation(0) );
    }

    //-----------------------------------------------------------------------------------
    void prepare_to_engrave_beam(ImoBeam* pBeam)
    {
        int iInstr = 0;
        int iStaff = 0;
        int iCol = 0;

        list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes = pBeam->get_related_objects();
        int numNotes = int( notes.size() );

        m_shapes.reserve(numNotes);
        m_pMeter = LOMSE_NEW ScoreMeter(m_pScore, 1, 1, LOMSE_STAFF_LINE_SPACING);
        m_pStorage = LOMSE_NEW EngraversMap();

        //engrave notes
        list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;

        m_pNoteEngrv = LOMSE_NEW NoteEngraver(m_libraryScope, m_pMeter, m_pStorage, 0, 0);
        for (it = notes.begin(); it != notes.end(); ++it)
        {
            ImoNote* pNote = static_cast<ImoNote*>( (*it).first );
            GmoShapeNote* pShape = dynamic_cast<GmoShapeNote*>(
                m_pNoteEngrv->create_shape(pNote, k_clef_G2, 0,
                                           UPoint(10.0f, 15.0f)) );
            m_shapes.push_back(pShape);
        }

        //send note data to beam engraver
        int i=0;
        for (it = notes.begin(); it != notes.end(); ++it, ++i)
        {
            ImoNote* pNote = static_cast<ImoNote*>( (*it).first );
            AuxObjContext aoc(pNote, m_shapes[i], iInstr, iStaff, iCol, 0, nullptr, -1);
            if (i == 0)
            {
                //first note
                m_pBeamEngrv = LOMSE_NEW MyBeamEngraver(m_libraryScope, m_pMeter);
                m_pBeamEngrv->set_start_staffobj(pBeam, aoc);
                m_pStorage->save_engraver(m_pBeamEngrv, pBeam);
            }
            else if (i == numNotes-1)
            {
                //last note
                m_pBeamEngrv->set_end_staffobj(pBeam, aoc);
            }
            else
            {
                //intermediate note
                m_pBeamEngrv->set_middle_staffobj(pBeam, aoc);
            }
        }
    }

    //-----------------------------------------------------------------------------------
    ColStaffObjs* do_layout(const string& options, const string& ldpNotes)
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string(
            "(score (vers 2.0)"
            + options
            + "(instrument (musicData (clef G)"
            + ldpNotes +
            ")))"
        );

        ImoDocument* pImoDoc = m_pDoc->get_im_root();
        ImoPageInfo* pInfo = pImoDoc->get_page_info();
        pInfo->set_page_width(60000.0f);    //ensure enough for ony one system

        m_pDocLayouter = LOMSE_NEW DocLayouter(m_pDoc, m_libraryScope);
        m_pDocLayouter->layout_document();
        m_pGModel = m_pDocLayouter->get_graphic_model();

        ImoDocument* pRoot = m_pDoc->get_im_root();
        m_pScore = static_cast<ImoScore*>(pRoot->get_first_content_item());
        m_pMeter = LOMSE_NEW ScoreMeter(m_pScore, 1, 1, LOMSE_STAFF_LINE_SPACING);

        return m_pScore->get_staffobjs_table();
    }

    void delete_test_data()
    {
        delete m_pMeter;
        delete m_pStorage;
        delete m_pNoteEngrv;
        delete m_pBeamEngrv;
        delete m_pBeamShape;
        delete m_pGModel;

        m_pMeter = nullptr;
        m_pStorage = nullptr;
        m_pNoteEngrv = nullptr;
        m_pBeamEngrv = nullptr;
        m_pBeamShape = nullptr;
        m_pGModel = nullptr;

        std::vector<GmoShapeNote*>::iterator it;
        for (it = m_shapes.begin(); it != m_shapes.end(); ++it)
            delete *it;
        m_shapes.clear();

        delete m_pDocLayouter;
        delete m_pDoc;
        m_pDocLayouter = nullptr;
        m_pDoc = nullptr;
    }

//    //-----------------------------------------------------------------------------------
//    ImoBeam* get_beam(Document* pDoc)
//    {
//        ImoScore* pScore = static_cast<ImoScore*>(pDoc->get_content_item(0));
//        StaffObjsCursor cursor(pScore);
//        while(!cursor.is_end() && !cursor.get_staffobj()->is_note())
//        {
//            cursor.move_next();
//        }
//        if (cursor.is_end())
//            return nullptr;
//
//        while(!cursor.is_end())
//        {
//            if (cursor.get_staffobj()->is_note())
//            {
//                ImoNote* pNote = static_cast<ImoNote*>( cursor.get_staffobj() );
//                if (pNote->is_beamed())
//                    return pNote->get_beam();
//            }
//            cursor.move_next();
//        }
//        return nullptr;
//    }

    //-----------------------------------------------------------------------------------
    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    //-----------------------------------------------------------------------------------
    void check_stem_is_equal(int iLine, int iNote, ColStaffObjsIterator* pIt, float expected)
    {
        //pIt points to first note
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>((**pIt)->imo_object());
        GmoShapeNote* pShapeNote =
                    static_cast<GmoShapeNote*>(m_pGModel->get_shape_for_noterest(pNR));
        //LUnits halfStaffLine = m_pMeter->line_thickness_for_instr_staff(0,0) / 2.0f;
        LUnits result = pShapeNote->get_stem_height();
        LUnits target = expected * m_pMeter->tenths_to_logical(10.0f);// + halfStaffLine;
        bool fBadStem = is_equal_float(target, result);
        CHECK( fBadStem );
        if (!fBadStem)
        {
            cout << "    " << test_name() << " (line " << iLine << "): note "
                << iNote << ", target=" << target << ", result=" << result << endl;
        }
    }

    //-----------------------------------------------------------------------------------
    void check_slant_and_stems(int iLine, float expectedSlant, ColStaffObjsIterator* pIt,
                               int numStemsInChord = 2)
    {
            do_check_slant_and_stems(iLine, expectedSlant, pIt, 1, numStemsInChord);
    }

    //-----------------------------------------------------------------------------------
    void check_chord_slant_and_stems(int iLine, float expectedSlant, ColStaffObjsIterator* pIt,
                                     int numNotesPerStem, int numStemsInChord=2)
    {
            do_check_slant_and_stems(iLine, expectedSlant, pIt, numNotesPerStem, numStemsInChord);
    }

    //-----------------------------------------------------------------------------------
    void do_check_slant_and_stems(int iLine, float expectedSlant, ColStaffObjsIterator* pIt,
                                  int numNotesPerStem, int numStemsInChord)
    {
        //pIt points to first note

        LUnits oneSpace = m_pMeter->tenths_to_logical(10.0f);
        LUnits minStem = 2.5f * oneSpace;

        ImoNoteRest* pNR = static_cast<ImoNoteRest*>((**pIt)->imo_object());
        GmoShapeNote* pShapeNote1 =
                    static_cast<GmoShapeNote*>(m_pGModel->get_shape_for_noterest(pNR));
        LUnits top1 = pShapeNote1->get_stem_y_flag();
        if (numNotesPerStem > 1)
        {
            for (int i=0; i < numNotesPerStem-1; ++i, ++(*pIt));
            pNR = static_cast<ImoNoteRest*>((**pIt)->imo_object());
            GmoShapeNote* pShapeNote =
                        static_cast<GmoShapeNote*>(m_pGModel->get_shape_for_noterest(pNR));
            LUnits incrStem = abs(pShapeNote1->get_pos_on_staff() - pShapeNote->get_pos_on_staff());
            minStem = (2.5f + LUnits(incrStem * 0.5f)) * oneSpace;
        }
        LUnits stem = pShapeNote1->get_stem_height();
        bool fBadStem = (stem >= minStem);
        CHECK( fBadStem );
        if (!fBadStem)
        {
            cout << "    " << test_name() << " (line " << iLine << "): stem="
                 << stem/oneSpace << ", should be=" << minStem/oneSpace << endl;
        }

        //advance to last base note
        if (numStemsInChord > 2)
        {
            int numNotes = numNotesPerStem * (numStemsInChord - 2);
            for (int i=0; i < numNotes; ++i, ++(*pIt));
        }

        ++(*pIt);
        pNR = static_cast<ImoNoteRest*>((**pIt)->imo_object());
        GmoShapeNote* pShapeNote2 = static_cast<GmoShapeNote*>(m_pGModel->get_shape_for_noterest(pNR));
        LUnits top2 = pShapeNote2->get_stem_y_flag();
        LUnits Ay = top2 - top1;
        float angle = Ay / oneSpace;
        bool fBadSlant = is_equal_float(angle, expectedSlant);
        CHECK( fBadSlant );
        if (!fBadSlant)
        {
            cout << "    " << test_name() << " (line " << iLine << "): expected slant="
                 << expectedSlant << ", real slant= " << angle
                 << ", top2=" << top2 << ", top1=" << top1
                 << ", oneSpace=" << oneSpace << endl;
        }
        if (numNotesPerStem > 1)
        {
            for (int i=0; i < numNotesPerStem-1; ++i, ++(*pIt));
            pNR = static_cast<ImoNoteRest*>((**pIt)->imo_object());
            GmoShapeNote* pShapeNote =
                        static_cast<GmoShapeNote*>(m_pGModel->get_shape_for_noterest(pNR));
            LUnits incrStem = abs(pShapeNote2->get_pos_on_staff() - pShapeNote->get_pos_on_staff());
            minStem = (2.5f + LUnits(incrStem * 0.5f)) * oneSpace;
        }
        stem = pShapeNote2->get_stem_height();
        fBadStem = (stem >= minStem);
        CHECK( fBadStem );
        if (!fBadStem)
        {
            cout << "    " << test_name() << " (line " << iLine << "): stem="
                 << stem/oneSpace << ", should be=" << minStem/oneSpace << endl;
        }
    }

    //-----------------------------------------------------------------------------------
    void check_repeated_pattern_of_pitches(int iLine, ImoBeam* pBeam, bool fExpected)
    {
        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_decide_stems_direction();
        bool fResult = m_pBeamEngrv->my_has_repeated_pattern_of_pitches();
        bool fSuccess = (fResult == fExpected);
        CHECK( fSuccess );
        if (!fSuccess)
        {
            cout << "    " << test_name() << " (line " << iLine << ") " << endl;
            cout << "m_shapes.size()=" << m_shapes.size() << ", fResult=" << fResult
                 << ", fExpected=" << fExpected << endl;
        }
        delete_test_data();
    }

    //-----------------------------------------------------------------------------------
    void check_all_notes_outside_first_ledger_line(int iLine, ImoBeam* pBeam, bool fExpected)
    {
        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_decide_stems_direction();
        bool fResult = m_pBeamEngrv->my_check_all_notes_outside_first_ledger_line();
        bool fSuccess = (fResult == fExpected);
        CHECK( fSuccess );
        if (!fSuccess)
        {
            cout << "    " << test_name() << " (line " << iLine << ") " << endl;
            cout << "m_shapes.size()=" << m_shapes.size() << ", fResult=" << fResult
                 << ", fExpected=" << fExpected << endl;
        }
        delete_test_data();
    }

};

SUITE(BeamEngraverTest)
{

    // preliminary test -----------------------------------------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_p01)
    {
        //@p01. Check that create_beam() creates the ImoBeam object.
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        CHECK( pBeam != nullptr );
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_p02)
    {
        //@p02. Check that prepare_to_engrave_beam() creates BeamEngraver object.
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        CHECK( pBeam != nullptr );
        prepare_to_engrave_beam(pBeam);

        MyBeamEngraver* pEngrv = dynamic_cast<MyBeamEngraver*>(m_pStorage->get_engraver(pBeam));

        CHECK( pEngrv != nullptr );
        CHECK( pEngrv == m_pBeamEngrv );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_p03)
    {
        //@p03. Check that beam engraver creates the beam shape
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        prepare_to_engrave_beam(pBeam);

        RelObjEngravingContext ctx;
        m_pBeamShape = dynamic_cast<GmoShapeBeam*>( m_pBeamEngrv->create_last_shape(ctx) );
        CHECK( m_pBeamShape != nullptr );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_p04)
    {
        //@p04. Check that prepare_to_engrave_beam() store the notes shapes in m_shapes
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        prepare_to_engrave_beam(pBeam);

        RelObjEngravingContext ctx;
        m_pBeamShape = dynamic_cast<GmoShapeBeam*>( m_pBeamEngrv->create_last_shape(ctx) );

        std::vector<GmoShapeNote*>::iterator it;
        for (it = m_shapes.begin(); it != m_shapes.end(); ++it)
            CHECK( (*it)->find_related_shape(GmoObj::k_shape_beam) == m_pBeamShape );

        delete_test_data();
    }


    // decide_stems_direction --------------------------------------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_p11)
    {
        //@p11. decide_stems_direction(). Stems default, beam above
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == false );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == false );
        CHECK( m_pBeamEngrv->my_stems_up() == true );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );
        CHECK( m_pBeamEngrv->my_get_max_staff() == 0 );
        CHECK( m_pBeamEngrv->my_get_min_staff() == 0 );

        delete_test_data();
    }


    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_p12)
    {
        //@p12. decide_stems_direction(). Stems forced, beam above
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+ (stem up))(n f4 e g- (stem up))");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == false );
        CHECK( m_pBeamEngrv->my_stems_up() == true );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );
        CHECK( m_pBeamEngrv->my_get_max_staff() == 0 );
        CHECK( m_pBeamEngrv->my_get_min_staff() == 0 );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_p13)
    {
        //@p13. decide_stems_direction(). Stems forced
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c5 e g+)(n f4 e g-)");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == false );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == false );
        CHECK( m_pBeamEngrv->my_stems_up() == true );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );
        CHECK( m_pBeamEngrv->my_get_max_staff() == 0 );
        CHECK( m_pBeamEngrv->my_get_min_staff() == 0 );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_p14)
    {
        //@p14. decide_stems_direction(). Stems mixed
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+ (stem down))(n f4 e g- (stem up))");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == true );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );
        CHECK( m_pBeamEngrv->my_get_max_staff() == 0 );
        CHECK( m_pBeamEngrv->my_get_min_staff() == 0 );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_p15)
    {
        //@p15. decide_stems_direction(). Cross-staff
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n a3 e p2 (beam 1 +))"
            "(n e4 e p1 (beam 1 =))"
            "(n a3 e p2 (beam 1 =))"
            "(n e4 e p1 (beam 1 -))"
        );
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == false );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == false );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == true );
        CHECK( m_pBeamEngrv->my_get_max_staff() == 1 );
        CHECK( m_pBeamEngrv->my_get_min_staff() == 0 );

        delete_test_data();
    }


    // engraving rules ------------------------------------------------------------------


    // 02x. Tests for beam position: above / below --------------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_020)
    {
        //@020. D1. The furthest note from the middle line determines beam position.
        //      Case 1: furthest note above forces beam below.
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n f5 e g+)(n f4 e)(n f5 e g-)");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == false );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == false );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );
        CHECK( m_pBeamEngrv->my_get_beam_position() == k_beam_below );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_021)
    {
        //@021. D1. The furthest note from the middle line determines beam position.
        //      Case 2: furthest note below forces beam above.
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n d5 e g+)(n f4 e)(n d5 e g-)");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == false );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == false );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );
        CHECK( m_pBeamEngrv->my_get_beam_position() == k_beam_above );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_022)
    {
        //@022. D2. If there are two extreme notes in opposite directions and at the
        //          same distance from middle line, then group will be stemmed down.
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n g4 e g+)(n a4 e)(n b4 e)(n d5 e g-)");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == false );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == false );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );
        CHECK( m_pBeamEngrv->my_get_beam_position() == k_beam_below );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_023)
    {
        //@023. D3. When there is only one stem direction forced the beam placement is
        //          forced by it.
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc,
            "(n d5 e g+)(n f4 e (stem down))(n d5 e g-)");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == false );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );
        CHECK( m_pBeamEngrv->my_get_beam_position() == k_beam_below );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_024)
    {
        //@024. D4. More than one stem forced: if all forced in the same direction this
        //          direction forces beam placement. Case 1: all forced up -> beam above
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc,
            "(n f5 e g+ (stem up))(n f4 e)(n f5 e g- (stem up))");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == false );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );
        CHECK( m_pBeamEngrv->my_get_beam_position() == k_beam_above );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_025)
    {
        //@025. D4. More than one stem forced: if all forced in the same direction this
        //          direction forces beam placement. Case 2: all forced down -> beam below
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc,
            "(n g4 e g+ (stem down))(n a4 e (stem down))(n b4 e)(n f4 e g-)");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == false );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );
        CHECK( m_pBeamEngrv->my_get_beam_position() == k_beam_below );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_026)
    {
        //@026. D4. More than one stem forced: when stems forced in different
        //          directions, the beam is double-stemmed.
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc,
            "(n g4 e g+ (stem down))(n a4 e)(n b4 e)(n d5 e g- (stem up))");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == true );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == false );

        delete_test_data();
    }





    // 05x. Tests for stem length -------------------------------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_051)
    {
        //@051. A1. Beams on the staff. If note is on a line, the stem is 3.25 spaces
        ColStaffObjs* pTable = do_layout("",
            "(n c4 e g+)(n c4 e g-)"
            "(n a5 e g+)(n a5 e g-)"
            "(n d5 e g+)(n d5 e g-)"
            "(n e4 e g+)(n e4 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //1st note
        check_stem_is_equal(__LINE__, 1, &it, 3.25f);
        ++it;   //2nd note
        check_stem_is_equal(__LINE__, 2, &it, 3.25f);
        ++it;   //3rd note
        check_stem_is_equal(__LINE__, 3, &it, 3.25f);
        ++it;   //4th note
        check_stem_is_equal(__LINE__, 4, &it, 3.25f);
        ++it;   //5th note
        check_stem_is_equal(__LINE__, 5, &it, 3.25f);
        ++it;   //6th note
        check_stem_is_equal(__LINE__, 6, &it, 3.25f);
        ++it;   //7th note
        check_stem_is_equal(__LINE__, 7, &it, 3.25f);
        ++it;   //8th note
        check_stem_is_equal(__LINE__, 8, &it, 3.25f);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_052)
    {
        //@052. A1. Beams on the staff. For notes on space stem must be 3.0 or 3.5
        //      depending on the space
        ColStaffObjs* pTable = do_layout("",
            "(n d4 e g+)(n d4 e g-)"
            "(n a4 e g+)(n a4 e g-)"
            "(n e5 e g+)(n e5 e g-)"
            "(n c5 e g+)(n c5 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //1st note
        check_stem_is_equal(__LINE__, 1, &it, 3.5f);
        ++it;   //2nd note
        check_stem_is_equal(__LINE__, 2, &it, 3.5f);
        ++it;   //3rd note
        check_stem_is_equal(__LINE__, 3, &it, 3.0f);
        ++it;   //4th note
        check_stem_is_equal(__LINE__, 4, &it, 3.0f);
        ++it;   //5th note
        check_stem_is_equal(__LINE__, 5, &it, 3.5f);
        ++it;   //6th note
        check_stem_is_equal(__LINE__, 6, &it, 3.5f);
        ++it;   //7th note
        check_stem_is_equal(__LINE__, 7, &it, 3.0f);
        ++it;   //8th note
        check_stem_is_equal(__LINE__, 8, &it, 3.0f);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_053)
    {
        //@053. A4. Minimum space between notehead and beam is 1.5 spaces. This implies
        //          a minimum stem lenght of 2.5 for one beam and 3 spaces for two beams
        ColStaffObjs* pTable = do_layout("",
            "(n e5 e g+ (stem up))(n e5 e g- (stem up))"
            "(n c4 e g+ (stem down))(n c4 e g- (stem down))"
            "(n e5 s g+ (stem up))(n e5 s g- (stem up))"
            "(n c4 s g+ (stem down))(n c4 s g- (stem down))"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //1st note
        check_stem_is_equal(__LINE__, 1, &it, 2.5f);
        ++it;   //2nd note
        check_stem_is_equal(__LINE__, 2, &it, 2.5f);
        ++it;   //3rd note
        check_stem_is_equal(__LINE__, 3, &it, 2.5f);
        ++it;   //4th note
        check_stem_is_equal(__LINE__, 4, &it, 2.5f);
        ++it;   //5th note
        check_stem_is_equal(__LINE__, 5, &it, 3.0f);
        ++it;   //6th note
        check_stem_is_equal(__LINE__, 6, &it, 3.0f);
        ++it;   //7th note
        check_stem_is_equal(__LINE__, 7, &it, 3.0f);
        ++it;   //8th note
        check_stem_is_equal(__LINE__, 8, &it, 3.0f);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_054)
    {
        //@054. A4. For each additional beam over two, the stem must be extended one space
        ColStaffObjs* pTable = do_layout("",
            "(n c4 f g+)(n c4 f g-)(n b5 f g+)(n b5 f g-)");
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //1st note
        check_stem_is_equal(__LINE__, 1, &it, 7.0f);
        ++it;   //2nd note
        check_stem_is_equal(__LINE__, 2, &it, 7.0f);
        ++it;   //3rd note
        check_stem_is_equal(__LINE__, 3, &it, 6.5f);
        ++it;   //4th note
        check_stem_is_equal(__LINE__, 4, &it, 6.5f);

        delete_test_data();
    }


    // 1xx. Tests for horizontal beams --------------------------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_100)
    {
        //@100. check_all_notes_outside_first_ledger_line(). All notes inside.
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n e4 e g+)(n c5 e g-)");

        check_all_notes_outside_first_ledger_line(__LINE__, pBeam, false);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_101)
    {
        //@101. check_all_notes_outside_first_ledger_line(). All notes outside, above.
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n b5 e g+)(n d6 e)(n e6 e)(n g6 e g-)");

        check_all_notes_outside_first_ledger_line(__LINE__, pBeam, true);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_102)
    {
        //@102. check_all_notes_outside...(). One note on first ledger, above
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n a5 e g+)(n d6 e)(n e6 e)(n g6 e g-)");

        check_all_notes_outside_first_ledger_line(__LINE__, pBeam, false);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_103)
    {
        //@103. check_all_notes_outside...(). One note on first ledger, below
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n b3 e)(n g3 e)(n a3 e g-)");

        check_all_notes_outside_first_ledger_line(__LINE__, pBeam, false);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_104)
    {
        //@104. check_all_notes_outside_first_ledger_line(). All notes outside, below
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n b3 e g+)(n f3 e)(n g3 e)(n a3 e g-)");

        check_all_notes_outside_first_ledger_line(__LINE__, pBeam, true);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_110)
    {
        //@110. H1b. Repeated pattern of pitches. False for two notes
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c5 e g+)(n f4 e g-)");

        check_repeated_pattern_of_pitches(__LINE__, pBeam,false);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_111)
    {
        //@111. H1c. Repeated pattern of pitches. False for even number of notes
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c5 e g+)(n a4 e)(n f4 e g-)");

        check_repeated_pattern_of_pitches(__LINE__, pBeam,false);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_112)
    {
        //@112. H1c. Repeated pattern of pitches. Four notes with pattern
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc,
            "(n c5 s g+)(n a4 s)(n c5 s)(n a4 s g-)"
        );

        check_repeated_pattern_of_pitches(__LINE__, pBeam,true);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_113)
    {
        //@113. H1c. Repeated pattern of pitches. Four notes no pattern
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc,
            "(n c5 s g+)(n g4 s)(n c5 s)(n a4 s g-)"
        );

        check_repeated_pattern_of_pitches(__LINE__, pBeam,false);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_114)
    {
        //@114. H1c. Repeated pattern of pitches. Six notes, pattern of three
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc,
            "(n c5 s g+)(n b4 s)(n a4 s)(n c5 s)(n b4 s)(n a4 s g-)"
        );

        check_repeated_pattern_of_pitches(__LINE__, pBeam,true);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_115)
    {
        //@115. H1c. Repeated pattern of pitches. 24 notes, pattern of 6
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc,
            "(n c5 s g+)(n b4 s)(n a4 s)(n g4 s)(n f4 s)(n e4 s)"
            "(n c5 s)(n b4 s)(n a4 s)(n g4 s)(n f4 s)(n e4 s)"
            "(n c5 s)(n b4 s)(n a4 s)(n g4 s)(n f4 s)(n e4 s)"
            "(n c5 s)(n b4 s)(n a4 s)(n g4 s)(n f4 s)(n e4 s g-)"
        );

        check_repeated_pattern_of_pitches(__LINE__, pBeam,true);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_116)
    {
        //@116. H1c. Repeated pattern of pitches. 24 notes, pattern of 12
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc,
            "(n c5 s g+)(n b4 s)(n a4 s)(n g4 s)(n f4 s)(n e4 s)"
            "(n d4 s)(n c4 s)(n b3 s)(n a3 s)(n g3 s)(n f3 s)"
            "(n c5 s)(n b4 s)(n a4 s)(n g4 s)(n f4 s)(n e4 s)"
            "(n d4 s)(n c4 s)(n b3 s)(n a3 s)(n g3 s)(n f3 s g-)"
        );

        check_repeated_pattern_of_pitches(__LINE__, pBeam,true);
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_130)
    {
        //@130. H1a. The beam is horizontal when the group begins and ends with the same note
        ColStaffObjs* pTable = do_layout(
            "(opt Render.SpacingMethod 1)(opt Render.SpacingValue 40)", //40 -> 58.5
            //
            "(n e4 e g+)(n g4 e)(n c5 e)(n e4 e g-)"
            "(n a5 e g+)(n f5 e)(n c5 e)(n a5 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //group 1
        check_slant_and_stems(__LINE__, 0.0f, &it, 4);
        ++it;   //group 2
        check_slant_and_stems(__LINE__, 0.0f, &it, 4);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_131)
    {
        //@131. H1b. The beam is horizontal when there is a repeated pattern of pitches
        ColStaffObjs* pTable = do_layout(
            "(opt Render.SpacingMethod 1)(opt Render.SpacingValue 40)", //40 -> 58.5
            //
            "(n d4 s (beam 1 ++))(n a4 s (beam 1 ==))(n c5 s (beam 1 =-))"
            "(n d4 s (beam 1 =+))(n a4 s (beam 1 ==))(n c5 s (beam 1 --))"
            "(n b4 e g+)(n f5 e)(n b4 e)(n f5 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //group 1
        check_slant_and_stems(__LINE__, 0.0f, &it, 6);
        ++it;   //group 2
        check_slant_and_stems(__LINE__, 0.0f, &it, 4);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_132)
    {
        //@132. H1c. The beam is horizontal when an inner note is closer to the beam than
        //           either of the outer notes
        ColStaffObjs* pTable = do_layout("",
            "(n c5 s g+ (stem up))(n a4 s (stem up))"
            "(n g5 s (stem up))(n f5 s g- (stem up))"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //1st note
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>((*it)->imo_object());
        GmoShapeNote* pShapeNote =
                    static_cast<GmoShapeNote*>(m_pGModel->get_shape_for_noterest(pNR));
        LUnits top1 = pShapeNote->get_stem_y_flag();
        LUnits result = pShapeNote->get_stem_height();
        LUnits target = 5.0f * m_pMeter->tenths_to_logical(10.0f);
        CHECK( is_equal_float(target, result) );
//        cout << test_name() << ", note 1. target=" << target << ", result=" << result << endl;

        ++it;   //2nd
        ++it;   //3rd
        ++it;   //4th
        pNR = static_cast<ImoNoteRest*>((*it)->imo_object());
        pShapeNote = static_cast<GmoShapeNote*>(m_pGModel->get_shape_for_noterest(pNR));
        LUnits top4 = pShapeNote->get_stem_y_flag();
        CHECK( is_equal_float(top1, top4) );
//        cout << test_name() << ", top1=" << top1 << ", top4=" << top4 << endl;

        delete_test_data();
    }


    // 2xx. Tests for beam slant --------------------------------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_200)
    {
        //@200. R2. Beam slant. Case 1: distance 3 spaces gives slant 1/4
        ColStaffObjs* pTable = do_layout(
            "(opt Render.SpacingMethod 1)(opt Render.SpacingValue 30)",
            //
            "(n d5 e g+)(n b3 e g-)"
            "(n d6 e g+)(n d5 e g-)"
            "(n d6 e g+ (stem up))(n d5 e g- (stem up))"
            "(n d5 e g+ (stem down))(n b3 e g- (stem down))"
            "(n b3 e g+)(n d5 e g-)"
            "(n d5 e g+)(n d6 e g-)"
            "(n d5 e g+ (stem up))(n d6 e g- (stem up))"
            "(n b3 e g+ (stem down))(n d5 e g- (stem down))"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //1st note. group 1
        check_slant_and_stems(__LINE__, 0.25f, &it);
        ++it;   //3rd note. group 2
        check_slant_and_stems(__LINE__, 0.25f, &it);
        ++it;   //5th note. group 3
        check_slant_and_stems(__LINE__, 0.25f, &it);
        ++it;   //7th note. group 4
        check_slant_and_stems(__LINE__, 0.25f, &it);
        ++it;   //9th note. group 5
        check_slant_and_stems(__LINE__, -0.25f, &it);
        ++it;   //11th note. group 6
        check_slant_and_stems(__LINE__, -0.25f, &it);
        ++it;   //13th note. group 7
        check_slant_and_stems(__LINE__, -0.25f, &it);
        ++it;   //15th note. group 8
        check_slant_and_stems(__LINE__, -0.25f, &it);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_201)
    {
        //@201. R2. Beam slant. Case 2: distance 5 spaces gives slant 1/2 space
        ColStaffObjs* pTable = do_layout(
            "(opt Render.SpacingMethod 1)(opt Render.SpacingValue 40)", //40 -> 58.5
            //
            "(n e4 e g+)(n e3 e g-)"
            "(n e3 e g+)(n e4 e g-)"
            "(n d5 e g+)(n d6 e g-)"
            "(n d6 e g+)(n d5 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //group 1
        check_slant_and_stems(__LINE__, 0.5f, &it);
        ++it;   //group 2
        check_slant_and_stems(__LINE__, -0.5f, &it);
        ++it;   //group 3
        check_slant_and_stems(__LINE__, -0.5f, &it);
        ++it;   //group 4
        check_slant_and_stems(__LINE__, 0.5f, &it);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_202)
    {
        //@202. R2. Beam slant. Case 3: distance 6.5 spaces gives slant 1 space
        ColStaffObjs* pTable = do_layout(
            "(opt Render.SpacingMethod 1)(opt Render.SpacingValue 50)" //50 --> 72.5
            ,
            "(n e4 e g+)(n e3 e g-)"
            "(n e3 e g+)(n e4 e g-)"
            "(n d5 e g+)(n d6 e g-)"
            "(n d6 e g+)(n d5 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //group 1
        check_slant_and_stems(__LINE__, 1.0f, &it);
        ++it;   //group 2
        check_slant_and_stems(__LINE__, -1.0f, &it);
        ++it;   //group 3
        check_slant_and_stems(__LINE__, -1.0f, &it);
        ++it;   //group 4
        check_slant_and_stems(__LINE__, 1.0f, &it);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_203)
    {
        //@203. R2. Beam slant. Case 4: distance 10 spaces gives slant 1.25 spaces
        ColStaffObjs* pTable = do_layout(
            "(opt Render.SpacingMethod 1)(opt Render.SpacingValue 70)" //70 --> 100.5
            ,
            "(n e4 e g+)(n e3 e g-)"
            "(n e3 e g+)(n e4 e g-)"
            "(n d5 e g+)(n d6 e g-)"
            "(n d6 e g+)(n d5 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //group 1
        check_slant_and_stems(__LINE__, 1.25f, &it);
        ++it;   //group 2
        check_slant_and_stems(__LINE__, -1.25f, &it);
        ++it;   //group 3
        check_slant_and_stems(__LINE__, -1.25f, &it);
        ++it;   //group 4
        check_slant_and_stems(__LINE__, 1.25f, &it);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_204)
    {
        //@204. R2. Beam slant. Case 5: distance 15 spaces gives slant 1.5 spaces
        ColStaffObjs* pTable = do_layout(
            "(opt Render.SpacingMethod 1)(opt Render.SpacingValue 100)" // 100 -> 142.5
            ,
            "(n e4 e g+)(n e3 e g-)"
            "(n e3 e g+)(n e4 e g-)"
            "(n d5 e g+)(n d6 e g-)"
            "(n d6 e g+)(n d5 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //group 1
        check_slant_and_stems(__LINE__, 1.5f, &it);
        ++it;   //group 2
        check_slant_and_stems(__LINE__, -1.5f, &it);
        ++it;   //group 3
        check_slant_and_stems(__LINE__, -1.5f, &it);
        ++it;   //group 4
        check_slant_and_stems(__LINE__, 1.5f, &it);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_205)
    {
        //@205. R2. Beam slant. Case 6: distance 21 spaces gives slant 2 spaces
        ColStaffObjs* pTable = do_layout(
            "(opt Render.SpacingMethod 1)"
            "(opt Render.SpacingValue 150)"  //150 -> 212.5
            ,
            "(n e4 e g+)(n e3 e g-)"
            "(n e3 e g+)(n e4 e g-)"
            "(n d5 e g+)(n d6 e g-)"
            "(n d6 e g+)(n d5 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //group 1
        check_slant_and_stems(__LINE__, 2.0f, &it);
        ++it;   //group 2
        check_slant_and_stems(__LINE__, -2.0f, &it);
        ++it;   //group 3
        check_slant_and_stems(__LINE__, -2.0f, &it);
        ++it;   //group 4
        check_slant_and_stems(__LINE__, 2.0f, &it);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_206)
    {
        //@206. R3. Beam slant must be limited by notes interval
        ColStaffObjs* pTable = do_layout(
            "(opt Render.SpacingMethod 1)"
            "(opt Render.SpacingValue 210)"
            ,
            "(n g4 e g+)(n f4 e g-)"  //2nd, 0.5
            "(n g4 e g+)(n e4 e g-)"  //3rd, 1.0
            "(n g4 e g+)(n d4 e g-)"  //4th, 1.5
            "(n g4 e g+)(n c4 e g-)"  //5th, 2.0
            "(n g4 e g+)(n b3 e g-)"  //greater, 2.0
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //group 1
        check_slant_and_stems(__LINE__, 0.5f, &it);
        ++it;   //group 2
        check_slant_and_stems(__LINE__, 1.0f, &it);
        ++it;   //group 3
        check_slant_and_stems(__LINE__, 1.5f, &it);
        ++it;   //group 4
        check_slant_and_stems(__LINE__, 2.0f, &it);
        ++it;   //group 5
        check_slant_and_stems(__LINE__, 2.0f, &it);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_207)
    {
        //@207. R4. Beam slant: all notes on ledger lines
        ColStaffObjs* pTable = do_layout(
        "(opt Render.SpacingMethod 1)"
        "(opt Render.SpacingValue 200)"
        ,
        "(n f3 e g+)(n g3 e g-)"
        "(n b5 e g+)(n d6 e)(n e6 e)(n g6 e g-)"
        "(n c7 e g+)(n c6 e g-)"
        "(n d6 e g+)(n d6 e)(n b5 e)(n b5 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //group 1
        check_slant_and_stems(__LINE__, -0.25f, &it);
        ++it;   //group 2
        check_slant_and_stems(__LINE__, -0.50f, &it, 4);
        ++it;   //group 3
        check_slant_and_stems(__LINE__, 0.50f, &it);
        ++it;   //group 4
        check_slant_and_stems(__LINE__, 0.50f, &it, 4);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_208)
    {
        //@208. Smooth transition between rule A2 and rule A3
        ColStaffObjs* pTable = do_layout(
        "(opt Render.SpacingMethod 1)"
        "(opt Render.SpacingValue 30)"
        ,
        "(n b4 e g+ (stem down))(n b4 e g- (stem down))"
        "(n a4 e g+ (stem down))(n a4 e g- (stem down))"
        "(n b4 e g+ (stem up))(n b4 e g- (stem up))"
        "(n c5 e g+ (stem up))(n c5 e g- (stem up))"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //1st note
        check_stem_is_equal(__LINE__, 1, &it, 3.25f);
        ++it;   //2nd note
        check_stem_is_equal(__LINE__, 2, &it, 3.25f);
        ++it;   //3rd note
        check_stem_is_equal(__LINE__, 3, &it, 2.75f);
        ++it;   //4th note
        check_stem_is_equal(__LINE__, 4, &it, 2.75f);
        ++it;   //5th note
        check_stem_is_equal(__LINE__, 5, &it, 3.25f);
        ++it;   //6th note
        check_stem_is_equal(__LINE__, 6, &it, 3.25f);
        ++it;   //7th note
        check_stem_is_equal(__LINE__, 7, &it, 2.75f);
        ++it;   //8th note
        check_stem_is_equal(__LINE__, 8, &it, 2.75f);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_209)
    {
        //@209. Stems for inner notes correctly computed
        ColStaffObjs* pTable = do_layout(
        "(opt Render.SpacingMethod 1)"
        "(opt Render.SpacingValue 30)"
        ,
        "(n g4 e g+)(n f4 e)(n d4 e)(n b3 e g-)"
        "(n c5 e g+)(n e5 e)(n g5 e)(n b5 e g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //1st note
        check_stem_is_equal(__LINE__, 1, &it, 3.25f);
        ++it;   //2nd note
        check_stem_is_equal(__LINE__, 2, &it, 3.33333333f);
        ++it;   //3rd note
        check_stem_is_equal(__LINE__, 3, &it, 3.91666666f);
        ++it;   //4th note
        check_stem_is_equal(__LINE__, 4, &it, 4.5f);
        ++it;   //5th note
        check_stem_is_equal(__LINE__, 5, &it, 2.75f);
        ++it;   //6th note
        check_stem_is_equal(__LINE__, 6, &it, 3.33333333f);
        ++it;   //7th note
        check_stem_is_equal(__LINE__, 7, &it, 3.91666666f);
        ++it;   //8th note
        check_stem_is_equal(__LINE__, 8, &it, 4.5f);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_210)
    {
        //@210. A4. Make stems larger when more that two beams
        ColStaffObjs* pTable = do_layout(
        "(opt Render.SpacingMethod 1)"
        "(opt Render.SpacingValue 30)"
        ,
        "(n g4 t g+)(n f4 t)(n d4 t)(n b3 t g-)"
        "(n c5 t g+)(n e5 t)(n g5 t)(n b5 t g-)"
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //1st note
        check_stem_is_equal(__LINE__, 1, &it, 4.0f);
        ++it;   //2nd note
        check_stem_is_equal(__LINE__, 2, &it, 4.16666666f);
        ++it;   //3rd note
        check_stem_is_equal(__LINE__, 3, &it, 4.83333333f);
        ++it;   //4th note
        check_stem_is_equal(__LINE__, 4, &it, 5.5f);
        ++it;   //5th note
        check_stem_is_equal(__LINE__, 5, &it, 3.5f);
        ++it;   //6th note
        check_stem_is_equal(__LINE__, 6, &it, 4.16666666f);
        ++it;   //7th note
        check_stem_is_equal(__LINE__, 7, &it, 4.83333333f);
        ++it;   //8th note
        check_stem_is_equal(__LINE__, 8, &it, 5.5f);

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_211)
    {
        //@211. R5. When three or more beams, the beam must slant a whole stave-space
        ColStaffObjs* pTable = do_layout(
        "(opt Render.SpacingMethod 1)"
        "(opt Render.SpacingValue 30)"
        ,
        "(n d6 t g+)(n g5 t g-)"              //200, 0.25
        "(n e3 t g+)(spacer 40)(n e4 t g-)"   //201, 0.50
        "(n e4 t g+)(spacer 70)(n e3 t g-)"   //203, 1.25
        "(n d5 t g+)(spacer 150)(n d6 t g-)"  //204, 1.50
        "(n g4 t g+)(spacer 210)(n b3 t g-)"  //206, 2.0
        );
        ColStaffObjsIterator it = pTable->begin();  //clef

        ++it;   //group 1
        check_slant_and_stems(__LINE__, 1.0f, &it);
        ++it;   //group 2
        check_slant_and_stems(__LINE__, -1.0f, &it, 3);
        ++it;   //group 3
        check_slant_and_stems(__LINE__, 1.0f, &it, 3);
        ++it;   //group 4
        check_slant_and_stems(__LINE__, -1.0f, &it, 3);
        ++it;   //group 5
        check_slant_and_stems(__LINE__, 1.0f, &it, 3);

        delete_test_data();
    }


    // 3xx. Tests for cross-staff double-stemmed beams ----------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_300)
    {
        //@300. cross-staff double-stemmed. Two notes. beam up

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n a3 e p2 (stem up)(beam 1 +))"
            "(n e4 e p1 (stem down)(beam 1 -))"
        );
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == true );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == true );
        CHECK( m_pBeamEngrv->my_get_max_staff() == 1 );
        CHECK( m_pBeamEngrv->my_get_min_staff() == 0 );

        m_pBeamEngrv->my_determine_number_of_beam_levels();
        float slant = m_pBeamEngrv->my_determine_slant_direction_for_cross_double_stemmed();
        CHECK( slant == 1.0f );
//        cout << test_name() << ", slant=" << slant << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_301)
    {
        //@301. cross-staff double-stemmed. Two notes. beam down

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n e4 e p1 (stem down)(beam 1 +))"
            "(n a3 e p2 (stem up)(beam 1 -))"
        );
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_is_double_stemmed() == true );
        CHECK( m_pBeamEngrv->my_is_cross_staff() == true );
        CHECK( m_pBeamEngrv->my_get_max_staff() == 1 );
        CHECK( m_pBeamEngrv->my_get_min_staff() == 0 );

        m_pBeamEngrv->my_determine_number_of_beam_levels();
        float slant = m_pBeamEngrv->my_determine_slant_direction_for_cross_double_stemmed();
        CHECK( slant == -1.0f );
//        cout << test_name() << ", slant=" << slant << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_302)
    {
        //@302. cross-staff double-stemmed. Three notes, outher notes in different
        //      staff. Beam down

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n e4 e p1 (stem down)(beam 1 +))"
            "(n g4 e p1 (stem down)(beam 1 =))"
            "(n a3 e p2 (stem up)(beam 1 -))"
        );
        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_decide_stems_direction();
        m_pBeamEngrv->my_determine_number_of_beam_levels();

        float slant = m_pBeamEngrv->my_determine_slant_direction_for_cross_double_stemmed();
        CHECK( slant == -1.0f );
//        cout << test_name() << ", slant=" << slant << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_303)
    {
        //@303. cross-staff double-stemmed. Three notes, outher notes in the
        //      same staff. Beam up

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n f3 e p2 (stem up)(beam 1 +))"
            "(n e4 e p1 (stem down)(beam 1 =))"
            "(n a3 e p2 (stem up)(beam 1 -))"
        );
        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_decide_stems_direction();
        m_pBeamEngrv->my_determine_number_of_beam_levels();

        float slant = m_pBeamEngrv->my_determine_slant_direction_for_cross_double_stemmed();
        CHECK( slant == 1.0f );
//        cout << test_name() << ", slant=" << slant << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_304)
    {
        //@304. cross-staff double-stemmed. Three notes, outher notes in the
        //      same staff. Beam horizontal

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n a4 e p1 (stem down)(beam 1 +))"
            "(n e3 e p2 (stem up)(beam 1 =))"
            "(n a4 e p1 (stem down)(beam 1 -))"
        );
        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_decide_stems_direction();
        m_pBeamEngrv->my_determine_number_of_beam_levels();

        float slant = m_pBeamEngrv->my_determine_slant_direction_for_cross_double_stemmed();
        CHECK( slant == 0.0f );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_310)
    {
        //@310. cross-staff double-stemmed. Four or more notes and only one note in one
        //      side. Beam horizontal

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n a4 e p1 (stem down)(beam 1 +))"
            "(n c5 e p1 (stem down)(beam 1 =))"
            "(n e3 e p2 (stem up)(beam 1 =))"
            "(n a4 e p1 (stem down)(beam 1 -))"
        );
        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_decide_stems_direction();
        m_pBeamEngrv->my_determine_number_of_beam_levels();

        float slant = m_pBeamEngrv->my_determine_slant_direction_for_cross_double_stemmed();
        CHECK( slant == 0.0f );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_311)
    {
        //@311. cross-staff double-stemmed. Four or more notes and only one note in one
        //      side. Opposite to 310. Beam horizontal

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n e3 e p2 (stem up)(beam 1 +))"
            "(n c3 e p2 (stem up)(beam 1 =))"
            "(n g4 e p1 (stem down)(beam 1 =))"
            "(n e3 e p2 (stem up)(beam 1 -))"
        );
        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_decide_stems_direction();
        m_pBeamEngrv->my_determine_number_of_beam_levels();

        float slant = m_pBeamEngrv->my_determine_slant_direction_for_cross_double_stemmed();
        CHECK( slant == 0.0f );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_312)
    {
        //@312. cross-staff double-stemmed. Four or more notes and only one note in one
        //      side. Intermediate note near beam forces beam horizontal

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n c3 e p2 (stem up)(beam 1 +))"
            "(n e3 e p2 (stem up)(beam 1 =))"
            "(n g4 e p1 (stem down)(beam 1 =))"
            "(n d3 e p2 (stem up)(beam 1 -))"
        );
        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_decide_stems_direction();
        m_pBeamEngrv->my_determine_number_of_beam_levels();

        float slant = m_pBeamEngrv->my_determine_slant_direction_for_cross_double_stemmed();
        CHECK( slant == 0.0f );
//        cout << test_name() << ", slant=" << slant << endl;

        delete_test_data();
    }


    // preliminary test -----------------------------------------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_501)
    {
        //@501. simple beam, two notes one segment

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        CHECK( pBeam != nullptr );

        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_determine_number_of_beam_levels();
        m_pBeamEngrv->my_decide_stems_direction();

        vector<SegmentData> segs;
        m_pBeamEngrv->my_determine_segments(pBeam, &segs);

        CHECK( segs.size() == 1 );
        CHECK( segs[0].pStartNote == m_shapes[0] );
        CHECK( segs[0].pEndNote == m_shapes[1] );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_segments_engraver_502)
    {
        //@502. simple beam, forward hook

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 s g+)(n f4 e. g-)");
        CHECK( pBeam != nullptr );

        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_determine_number_of_beam_levels();
        m_pBeamEngrv->my_decide_stems_direction();

        vector<SegmentData> segs;
        m_pBeamEngrv->my_determine_segments(pBeam, &segs);

        CHECK( segs.size() == 2 );
        CHECK( segs[0].pStartNote == m_shapes[0] );
        CHECK( segs[0].pEndNote == m_shapes[1] );
        CHECK( segs[1].pStartNote == nullptr );
        CHECK( segs[1].pEndNote == m_shapes[1] );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_510)
    {
        //@510. R1a. one subgroup, one segment [E.Gould, p.316 (a)-1]

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n c3 e p2 (stem up)(beam 1 +))"
            "(n g4 s p1 (stem down)(beam 1 =+))"
            "(n g4 s p1 (stem down)(beam 1 --))"
        );

        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_determine_number_of_beam_levels();
        m_pBeamEngrv->my_decide_stems_direction();

        vector<SegmentData> segs;
        m_pBeamEngrv->my_determine_segments(pBeam, &segs);

        CHECK( segs.size() == 2 );
        CHECK( segs[0].pStartNote == m_shapes[0] );
        CHECK( segs[0].pEndNote == m_shapes[2] );
        CHECK( segs[1].pStartNote == m_shapes[1] );
        CHECK( segs[1].pEndNote == m_shapes[2] );

        m_pBeamEngrv->my_classify_segments(&segs);

        CHECK( segs[0].position != k_beam_undecided );
        CHECK( segs[1].position == k_beam_above );
//        cout << test_name() << ", pos1=" << segs[0].position
//             << ", pos2=" << segs[1].position << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_511)
    {
        //@511. R1a. one subgroup, two segments [E.Gould, p.316 (a)-2]

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n c3 s p2 (stem up)(beam 1 ++))"
            "(n c3 s p2 (stem up)(beam 1 ==))"
            "(n c3 s p2 (stem up)(beam 1 =-))"
            "(n g4 e p1 (stem down)(beam 1 -))"
        );

        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_determine_number_of_beam_levels();
        m_pBeamEngrv->my_decide_stems_direction();

        vector<SegmentData> segs;
        m_pBeamEngrv->my_determine_segments(pBeam, &segs);

        CHECK( segs.size() == 2 );
        CHECK( segs[0].pStartNote == m_shapes[0] );
        CHECK( segs[0].pEndNote == m_shapes[3] );
        CHECK( segs[1].pStartNote == m_shapes[0] );
        CHECK( segs[1].pEndNote == m_shapes[2] );

        m_pBeamEngrv->my_classify_segments(&segs);

        CHECK( segs[0].position != k_beam_undecided );
        CHECK( segs[1].position == k_beam_below );
//        cout << test_name() << ", pos1=" << segs[0].position
//             << ", pos2=" << segs[1].position << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_512)
    {
        //@512. R1a. three subgroups, six segments [E.Gould, p.316 (a)-3]

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n g4 t p1 (stem down)(beam 1 +++))"
            "(n g4 t p1 (stem down)(beam 1 ===))"
            "(n g4 t p1 (stem down)(beam 1 ===))"
            "(n g4 t p1 (stem down)(beam 1 =--))"
            "(n c3 s. p2 (stem up)(beam 1 =+))"
            "(n c3 t p2 (stem up)(beam 1 =-b))"
            "(n g4 s p1 (stem down)(beam 1 =+))"
            "(n g4 s p1 (stem down)(beam 1 --))"
        );

        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_determine_number_of_beam_levels();
        m_pBeamEngrv->my_decide_stems_direction();

        vector<SegmentData> segs;
        m_pBeamEngrv->my_determine_segments(pBeam, &segs);

        CHECK( segs.size() == 6 );
        CHECK( segs[0].pStartNote == m_shapes[0] );
        CHECK( segs[0].pEndNote == m_shapes[7] );
        CHECK( segs[1].pStartNote == m_shapes[0] );
        CHECK( segs[1].pEndNote == m_shapes[3] );
        CHECK( segs[2].pStartNote == m_shapes[4] );
        CHECK( segs[2].pEndNote == m_shapes[5] );
        CHECK( segs[3].pStartNote == m_shapes[6] );
        CHECK( segs[3].pEndNote == m_shapes[7] );
        CHECK( segs[4].pStartNote == m_shapes[0] );
        CHECK( segs[4].pEndNote == m_shapes[3] );
        CHECK( segs[5].pStartNote == m_shapes[5] );
        CHECK( segs[5].pEndNote == m_shapes[5] );

        m_pBeamEngrv->my_classify_segments(&segs);

        CHECK( segs[0].position != k_beam_undecided );
        CHECK( segs[1].position == k_beam_above );
        CHECK( segs[2].position == k_beam_below );
        CHECK( segs[3].position == k_beam_above );
        CHECK( segs[4].position == k_beam_above );
        CHECK( segs[5].position == k_beam_below );
//        cout << test_name() << ", pos1=" << segs[0].position
//             << ", pos2=" << segs[1].position << ", pos3=" << segs[2].position
//             << ", pos4=" << segs[3].position << ", pos5=" << segs[4].position
//             << ", pos6=" << segs[5].position << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_513)
    {
        //@513. R1b. two subgroups, three segments [E.Gould, p.316 (b)]

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n g4 s p1 (stem down)(beam 1 ++))"
            "(n c3 s p2 (stem up)(beam 1 ==))"
            "(n c3 s p2 (stem up)(beam 1 ==))"
            "(n g4 s p1 (stem down)(beam 1 =-))"
            "(n c3 s p2 (stem up)(beam 1 =+))"
            "(n g4 s p1 (stem down)(beam 1 ==))"
            "(n c3 s p2 (stem up)(beam 1 --))"
        );

        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_determine_number_of_beam_levels();
        m_pBeamEngrv->my_decide_stems_direction();

        vector<SegmentData> segs;
        m_pBeamEngrv->my_determine_segments(pBeam, &segs);

        CHECK( segs.size() == 3 );
        CHECK( segs[0].pStartNote == m_shapes[0] );
        CHECK( segs[0].pEndNote == m_shapes[6] );
        CHECK( segs[1].pStartNote == m_shapes[0] );
        CHECK( segs[1].pEndNote == m_shapes[3] );
        CHECK( segs[2].pStartNote == m_shapes[4] );
        CHECK( segs[2].pEndNote == m_shapes[6] );

        m_pBeamEngrv->my_classify_segments(&segs);

        CHECK( segs[0].position != k_beam_undecided );
        CHECK( segs[1].position == k_beam_above );
        CHECK( segs[2].position == k_beam_below );
//        cout << test_name() << ", pos1=" << segs[0].position
//             << ", pos2=" << segs[1].position << ", pos3=" << segs[2].position << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_514)
    {
        //@514. R1c. beam fractional [E.Gould, p.316 (c)]

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n g4 e p1 (stem down)(beam 1 +))"
            "(n c3 s p2 (stem up)(beam 1 =b))"
            "(n c3 e p2 (stem up)(beam 1 =))"
            "(n g4 s p1 (stem down)(beam 1 -b))"
        );

        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_determine_number_of_beam_levels();
        m_pBeamEngrv->my_decide_stems_direction();

        vector<SegmentData> segs;
        m_pBeamEngrv->my_determine_segments(pBeam, &segs);

        CHECK( segs.size() == 3 );
        CHECK( segs[0].pStartNote == m_shapes[0] );
        CHECK( segs[0].pEndNote == m_shapes[3] );
        CHECK( segs[1].pStartNote == m_shapes[1] );
        CHECK( segs[1].pEndNote == m_shapes[1] );
        CHECK( segs[2].pStartNote == m_shapes[3] );
        CHECK( segs[2].pEndNote == m_shapes[3] );

        m_pBeamEngrv->my_classify_segments(&segs);

        CHECK( segs[0].position != k_beam_undecided );
        CHECK( segs[1].position == k_beam_below );
        CHECK( segs[2].position == k_beam_above );
//        cout << test_name() << ", pos1=" << segs[0].position
//             << ", pos2=" << segs[1].position << ", pos3=" << segs[2].position << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_515)
    {
        //@515. R2.1. on the opposite side of first note [E.Gould, p.316 fig.3]

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n g4 s p1 (stem down)(beam 1 ++))"
            "(n g4 s p1 (stem down)(beam 1 ==))"
            "(n c3 s p2 (stem up)(beam 1 =-))"
            "(n g4 e p1 (stem down)(beam 1 -))"
        );

        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_determine_number_of_beam_levels();
        m_pBeamEngrv->my_decide_stems_direction();

        vector<SegmentData> segs;
        m_pBeamEngrv->my_determine_segments(pBeam, &segs);

        CHECK( segs.size() == 2 );
        CHECK( segs[0].pStartNote == m_shapes[0] );
        CHECK( segs[0].pEndNote == m_shapes[3] );
        CHECK( segs[1].pStartNote == m_shapes[0] );
        CHECK( segs[1].pEndNote == m_shapes[2] );

        m_pBeamEngrv->my_classify_segments(&segs);

        CHECK( segs[0].position != k_beam_undecided );
        CHECK( segs[1].position == k_beam_below );
//        cout << test_name() << ", pos1=" << segs[0].position
//             << ", pos2=" << segs[1].position << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_516)
    {
        //@516. R2.2. on the same side of first note [E.Gould, p.316 fig.4]

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n g4 e p1 (stem down)(beam 1 +))"
            "(n g4 s p1 (stem down)(beam 1 =+))"
            "(n c3 s p2 (stem up)(beam 1 --))"
        );

        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_determine_number_of_beam_levels();
        m_pBeamEngrv->my_decide_stems_direction();

        vector<SegmentData> segs;
        m_pBeamEngrv->my_determine_segments(pBeam, &segs);

        CHECK( segs.size() == 2 );
        CHECK( segs[0].pStartNote == m_shapes[0] );
        CHECK( segs[0].pEndNote == m_shapes[2] );
        CHECK( segs[1].pStartNote == m_shapes[1] );
        CHECK( segs[1].pEndNote == m_shapes[2] );

        m_pBeamEngrv->my_classify_segments(&segs);

        CHECK( segs[0].position != k_beam_undecided );
        CHECK( segs[1].position == k_beam_above );
//        cout << test_name() << ", pos1=" << segs[0].position
//             << ", pos2=" << segs[1].position << endl;

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_517)
    {
        //@517. R2.3. when several mixed, all in the same side [E.Gould, p.317 fig.1]

        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam_two_staves(doc,
            "(n g4 e p1 (stem down)(beam 1 +))"
            "(n c3 s p2 (stem up)(beam 1 =+))"
            "(n g4 s p1 (stem down)(beam 1 =-))"
            "(n g4 s p1 (stem down)(beam 1 =+))"
            "(n c3 s p2 (stem up)(beam 1 --))"
        );

        prepare_to_engrave_beam(pBeam);
        m_pBeamEngrv->my_determine_number_of_beam_levels();
        m_pBeamEngrv->my_decide_stems_direction();

        vector<SegmentData> segs;
        m_pBeamEngrv->my_determine_segments(pBeam, &segs);

        CHECK( segs.size() == 3 );
        CHECK( segs[0].pStartNote == m_shapes[0] );
        CHECK( segs[0].pEndNote == m_shapes[4] );
        CHECK( segs[1].pStartNote == m_shapes[1] );
        CHECK( segs[1].pEndNote == m_shapes[2] );
        CHECK( segs[2].pStartNote == m_shapes[3] );
        CHECK( segs[2].pEndNote == m_shapes[4] );

        m_pBeamEngrv->my_classify_segments(&segs);

        CHECK( segs[0].position != k_beam_undecided );
        CHECK( segs[1].position == k_beam_below );
        CHECK( segs[1].position == k_beam_below );
//        cout << test_name() << ", pos1=" << segs[0].position
//             << ", pos2=" << segs[1].position << ", pos3=" << segs[2].position << endl;

        delete_test_data();
    }


//    TEST_FIXTURE(BeamEngraverTestFixture, beam_engraver_900)
//    {
//        //@900. Beam slant: error in chords
//        ColStaffObjs* pTable = do_layout(
//        "(opt Render.SpacingMethod 1)"
//        "(opt Render.SpacingValue 50)"
//        ,
//        "(chord (n c5 e (beam 1 +))(n f5 e)(n a5 e))"
//        "(chord (n d5 e (beam 1 -))(n g5 e)(n b5 e))"
//        "(chord (n c4 e (beam 2 +))(n f4 e)(n a4 e))"
//        "(chord (n d4 e (beam 2 -))(n g4 e)(n b4 e))"
//        "(chord (n c5 s (beam 3 ++))(n f5 s)(n a5 s))"
//        "(chord (n g4 s (beam 3 --))(n e5 s)(n g5 s))"
//        "(chord (n c4 s (beam 4 ++))(n f4 s)(n a4 s))"
//        "(chord (n g3 s (beam 4 --))(n e4 s)(n g4 s))"
//        );
//        ColStaffObjsIterator it = pTable->begin();  //clef
//
//        ++it;   //group 1 (6 notes)
//        check_chord_slant_and_stems(__LINE__, -1.0f, &it, 3);
//        ++it;   //group 2 (6 notes)
//        check_chord_slant_and_stems(__LINE__, -1.0f, &it, 3);
//        ++it;   //group 3 (6 notes)
//        check_chord_slant_and_stems(__LINE__, 1.0f, &it, 3);
//        ++it;   //group 4 (6 notes)
//        check_chord_slant_and_stems(__LINE__, 1.0f, &it, 3);
//
//        delete_test_data();
//    }

};


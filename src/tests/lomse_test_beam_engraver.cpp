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
#include "lomse_beam_engraver.h"
#include "lomse_note_engraver.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_shape_beam.h"
#include "lomse_score_meter.h"
#include "lomse_engravers_map.h"
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
    inline bool my_stems_mixed() { return m_fStemsMixed; }
    inline bool my_stems_down() { return m_fStemsDown; }
    inline int my_get_num_stems_down() { return m_numStemsDown; }
    inline int my_get_num_notes() { return m_numNotes; }
    inline int my_get_average_pos_on_staff() { return m_averagePosOnStaff; }
};


//---------------------------------------------------------------------------------------
class BeamEngraverTestFixture
{
public:
    LibraryScope    m_libraryScope;
    ImoScore*       m_pScore;
    ScoreMeter*     m_pMeter;
    EngraversMap*  m_pStorage;
    NoteEngraver*   m_pNoteEngrv;
    MyBeamEngraver* m_pBeamEngrv;
    GmoShapeBeam*   m_pBeamShape;
    std::vector<GmoShapeNote*> m_shapes;

    BeamEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pScore(nullptr)
        , m_pMeter(nullptr)
        , m_pStorage(nullptr)
        , m_pNoteEngrv(nullptr)
        , m_pBeamEngrv(nullptr)
        , m_pBeamShape(nullptr)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~BeamEngraverTestFixture()    //TearDown fixture
    {
    }

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

    void prepare_to_engrave_beam(ImoBeam* pBeam)
    {
        int iInstr = 0;
        int iStaff = 0;
        int iSystem = 0;
        int iCol = 0;

        list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes = pBeam->get_related_objects();
        int numNotes = int( notes.size() );

        m_shapes.reserve(numNotes);
        m_pMeter = LOMSE_NEW ScoreMeter(m_pScore, 1, 1, 180.0f);
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
            if (i == 0)
            {
                //first note
                m_pBeamEngrv = LOMSE_NEW MyBeamEngraver(m_libraryScope, m_pMeter);
                m_pBeamEngrv->set_start_staffobj(pBeam, pNote, m_shapes[i],
                                                 iInstr, iStaff, iSystem, iCol,
                                                 0.0f, 0.0f, 0.0f, -1, nullptr);
                m_pStorage->save_engraver(m_pBeamEngrv, pBeam);
            }
            else if (i == numNotes-1)
            {
                //last note
                m_pBeamEngrv->set_end_staffobj(pBeam, pNote, m_shapes[i],
                                               iInstr, iStaff, iSystem, iCol,
                                               0.0f, 0.0f, 0.0f, -1, nullptr);
            }
            else
            {
                //intermediate note
                m_pBeamEngrv->set_middle_staffobj(pBeam, pNote, m_shapes[i],
                                                  iInstr, iStaff, iSystem, iCol,
                                                  0.0f, 0.0f, 0.0f, -1, nullptr);
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

        m_pMeter = nullptr;
        m_pStorage = nullptr;
        m_pNoteEngrv = nullptr;
        m_pBeamEngrv = nullptr;
        m_pBeamShape = nullptr;
    }



};

SUITE(BeamEngraverTest)
{

    TEST_FIXTURE(BeamEngraverTestFixture, CreateBeam)
    {
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        CHECK( pBeam != nullptr );
    }

    TEST_FIXTURE(BeamEngraverTestFixture, FeedEngraver)
    {
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        CHECK( pBeam != nullptr );
        prepare_to_engrave_beam(pBeam);

        MyBeamEngraver* pEngrv = dynamic_cast<MyBeamEngraver*>(m_pStorage->get_engraver(pBeam));

        CHECK( pEngrv != nullptr );
        CHECK( pEngrv == m_pBeamEngrv );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, CreateBeamShape)
    {
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        prepare_to_engrave_beam(pBeam);

        m_pBeamShape = dynamic_cast<GmoShapeBeam*>( m_pBeamEngrv->create_last_shape() );
        CHECK( m_pBeamShape != nullptr );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, BeamShapeAddedToNoteShape)
    {
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        prepare_to_engrave_beam(pBeam);

        m_pBeamShape = dynamic_cast<GmoShapeBeam*>( m_pBeamEngrv->create_last_shape() );

        //method prepare_to_engrave_beam() store notes shapes in m_shapes
        std::vector<GmoShapeNote*>::iterator it;
        for (it = m_shapes.begin(); it != m_shapes.end(); ++it)
            CHECK( (*it)->find_related_shape(GmoObj::k_shape_beam) == m_pBeamShape );

        delete_test_data();
    }


    // decide_on_stems_direction --------------------------------------------------------

    TEST_FIXTURE(BeamEngraverTestFixture, DecideStemsDirection)
    {
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+)(n f4 e g-)");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_on_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == false );
        CHECK( m_pBeamEngrv->my_stems_mixed() == false );
        CHECK( m_pBeamEngrv->my_stems_down() == false );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, StemsForced)
    {
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+ (stem up))(n f4 e g- (stem up))");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_on_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_stems_mixed() == false );
        CHECK( m_pBeamEngrv->my_stems_down() == false );

        delete_test_data();
    }

    TEST_FIXTURE(BeamEngraverTestFixture, StemsMixedButNotAllowed)
    {
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c5 e g+)(n f4 e g-)");
        prepare_to_engrave_beam(pBeam);

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
        Document doc(m_libraryScope);
        ImoBeam* pBeam = create_beam(doc, "(n c4 e g+ (stem down))(n f4 e g- (stem up))");
        prepare_to_engrave_beam(pBeam);

        m_pBeamEngrv->my_decide_on_stems_direction();

        CHECK( m_pBeamEngrv->my_stems_forced() == true );
        CHECK( m_pBeamEngrv->my_stems_mixed() == false );   //always false when stems forced
        CHECK( m_pBeamEngrv->my_stems_down() == false );    //stem forced by last forced stem

        delete_test_data();
    }

}



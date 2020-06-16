//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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
#include "lomse_tuplet_engraver.h"
#include "lomse_note_engraver.h"
#include "lomse_rest_engraver.h"
#include "private/lomse_document_p.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_shape_tuplet.h"
#include "lomse_score_meter.h"
#include "lomse_engravers_map.h"
#include "lomse_ldp_analyser.h"
#include "lomse_im_factory.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class TupletEngraverTestFixture
{
public:
    LibraryScope    m_libraryScope;
    ImoScore*       m_pScore;
    ScoreMeter*     m_pMeter;
    EngraversMap*  m_pStorage;
    NoteEngraver*   m_pNoteEngrv;
    RestEngraver*   m_pRestEngrv;
    TupletEngraver* m_pTupletEngrv;
    GmoShapeTuplet*   m_pTupletShape;
    std::vector<GmoShape*> m_shapes;

    TupletEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pScore(nullptr)
        , m_pMeter(nullptr)
        , m_pStorage(nullptr)
        , m_pNoteEngrv(nullptr)
        , m_pRestEngrv(nullptr)
        , m_pTupletEngrv(nullptr)
        , m_pTupletShape(nullptr)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~TupletEngraverTestFixture()    //TearDown fixture
    {
    }

    ImoTuplet* create_tuplet(Document* doc, const string& src)
    {
        string ldp = "(score (vers 2.0)(instrument (musicData (clef G)";
        ldp += src;
        ldp += ")))";

        doc->from_string(ldp);
        m_pScore = static_cast<ImoScore*>( doc->get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoNote* pNote1 = static_cast<ImoNote*>( pMD->get_child(1) );
        return dynamic_cast<ImoTuplet*>( pNote1->get_relation(0) );
    }

    void prepare_to_engrave_tuplet(ImoTuplet* pTuplet)
    {
        int iInstr = 0;
        int iStaff = 0;
        int iSystem = 0;
        int iCol = 0;

        list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes = pTuplet->get_related_objects();
        int numNotes = int( notes.size() );

        m_shapes.reserve(numNotes);
        m_pMeter = LOMSE_NEW ScoreMeter(m_pScore, 1, 1, LOMSE_STAFF_LINE_SPACING);
        m_pStorage = LOMSE_NEW EngraversMap();

        //engrave notes/rests
        list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;

        m_pNoteEngrv = LOMSE_NEW NoteEngraver(m_libraryScope, m_pMeter, m_pStorage, 0, 0);
        m_pRestEngrv = LOMSE_NEW RestEngraver(m_libraryScope, m_pMeter, m_pStorage, 0, 0);
        for (it = notes.begin(); it != notes.end(); ++it)
        {
            ImoNoteRest* pNR = static_cast<ImoNoteRest*>( (*it).first );
            GmoShape* pShape;
            if (pNR->is_note())
            {
                ImoNote* pNote = static_cast<ImoNote*>(pNR);
                pShape = m_pNoteEngrv->create_shape(pNote, k_clef_G2, 0,
                                                    UPoint(10.0f, 15.0f) );
            }
            else
            {
                ImoRest* pRest = static_cast<ImoRest*>(pNR);
                pShape = m_pRestEngrv->create_shape(pRest, UPoint(10.0f, 15.0f));
            }
            m_shapes.push_back(pShape);
        }

        //send note data to tuplet engraver
        int i=0;
        for (it = notes.begin(); it != notes.end(); ++it, ++i)
        {
            ImoNoteRest* pNR = static_cast<ImoNoteRest*>( (*it).first );
            if (i == 0)
            {
                //first note
                m_pTupletEngrv = LOMSE_NEW TupletEngraver(m_libraryScope, m_pMeter);
                m_pTupletEngrv->set_start_staffobj(pTuplet, pNR, m_shapes[i],
                                                 iInstr, iStaff, iSystem, iCol,
                                                 0.0f, 0.0f, 0.0f, -1, nullptr);
                m_pStorage->save_engraver(m_pTupletEngrv, pTuplet);
            }
            else if (i == numNotes-1)
            {
                //last note
                m_pTupletEngrv->set_end_staffobj(pTuplet, pNR, m_shapes[i],
                                               iInstr, iStaff, iSystem, iCol,
                                               0.0f, 0.0f, 0.0f, -1, nullptr);
            }
            else
            {
                //intermediate note
                m_pTupletEngrv->set_middle_staffobj(pTuplet, pNR, m_shapes[i],
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
        delete m_pRestEngrv;
        delete m_pTupletEngrv;
        delete m_pTupletShape;

        std::vector<GmoShape*>::iterator it;
        for (it = m_shapes.begin(); it != m_shapes.end(); ++it)
            delete *it;
        m_shapes.clear();

        m_pMeter = nullptr;
        m_pStorage = nullptr;
        m_pNoteEngrv = nullptr;
        m_pRestEngrv = nullptr;
        m_pTupletEngrv = nullptr;
        m_pTupletShape = nullptr;
    }



};

SUITE(TupletEngraverTest)
{

    TEST_FIXTURE(TupletEngraverTestFixture, CreateTuplet)
    {
        Document doc(m_libraryScope);
        ImoTuplet* pTuplet = create_tuplet(&doc, "(n c4 e (t + 2 3))(n f4 e (t -))");
        CHECK( pTuplet != nullptr );
        CHECK( pTuplet && pTuplet->get_actual_number() == 2 );
        CHECK( pTuplet && pTuplet->get_normal_number() == 3 );
    }

    TEST_FIXTURE(TupletEngraverTestFixture, FeedEngraver)
    {
        Document doc(m_libraryScope);
        ImoTuplet* pTuplet = create_tuplet(&doc, "(n c4 e (t + 2 3))(n f4 e (t -))");
        prepare_to_engrave_tuplet(pTuplet);

        TupletEngraver* pEngrv = dynamic_cast<TupletEngraver*>(m_pStorage->get_engraver(pTuplet));

        CHECK( pEngrv != nullptr );
        CHECK( pEngrv && pEngrv == m_pTupletEngrv );

        delete_test_data();
    }

    TEST_FIXTURE(TupletEngraverTestFixture, CreateTupletShape)
    {
        Document doc(m_libraryScope);
        ImoTuplet* pTuplet = create_tuplet(&doc, "(n c4 e (t + 2 3))(n f4 e (t -))");
        prepare_to_engrave_tuplet(pTuplet);

        m_pTupletShape = dynamic_cast<GmoShapeTuplet*>( m_pTupletEngrv->create_last_shape() );
        CHECK( m_pTupletShape != nullptr );

        delete_test_data();
    }

    // decide_on_stems_direction --------------------------------------------------------


}



//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_shape_tuplet.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"
#include "lomse_ldp_analyser.h"
#include "lomse_im_factory.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// Access to protected members
class MyTupletEngraver : public TupletEngraver
{
public:
    MyTupletEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
        : TupletEngraver(libraryScope, pScoreMeter)
    {
    }

    //inline void my_decide_on_stems_direction() { decide_on_stems_direction(); }
    //inline bool my_stems_forced() { return m_fStemForced; }
    //inline bool my_stems_mixed() { return m_fStemMixed; }
    //inline bool my_stems_down() { return m_fStemsDown; }
    //inline int my_get_num_stems_down() { return m_numStemsDown; }
    //inline int my_get_num_notes() { return m_numNotes; }
    //inline int my_get_average_pos_on_staff() { return m_averagePosOnStaff; }

    //ShapeBoxInfo* my_get_shape_box_info() { return get_shape_box_info(0); }
};


//---------------------------------------------------------------------------------------
class TupletEngraverTestFixture
{
public:
    LibraryScope    m_libraryScope;
    ScoreMeter*     m_pMeter;
    ShapesStorage*  m_pStorage;
    NoteEngraver*   m_pNoteEngrv;
    RestEngraver*   m_pRestEngrv;
    MyTupletEngraver* m_pTupletEngrv;
    GmoShapeTuplet*   m_pTupletShape;
    std::vector<GmoShape*> m_shapes;

    TupletEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pMeter(NULL)
        , m_pStorage(NULL)
        , m_pNoteEngrv(NULL)
        , m_pRestEngrv(NULL)
        , m_pTupletEngrv(NULL)
        , m_pTupletShape(NULL)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~TupletEngraverTestFixture()    //TearDown fixture
    {
    }

    ImoTuplet* create_tuplet(int numNotes, ImoNoteRest* notes[], string tuplets[],
                             int tupletNum)
    {
        Document doc(m_libraryScope);
        LdpAnalyser a(cout, m_libraryScope, &doc);
        TupletsBuilder builder(cout, &a);
        for (int i=0; i < numNotes; ++i)
        {
            ImoTupletDto* pDto = LOMSE_NEW ImoTupletDto();
            if (i == 0)
            {
                pDto->set_tuplet_type(ImoTupletDto::k_start);
                pDto->set_note_rest(notes[i]);
                pDto->set_actual_number(2);
                pDto->set_normal_number(3);
                builder.add_item_info(pDto);
            }
            else if (i == numNotes-1)
            {
                pDto->set_tuplet_type(ImoTupletDto::k_stop);
                pDto->set_note_rest(notes[i]);
                builder.add_item_info(pDto);
            }
        }

        return notes[0]->get_tuplet();
    }

    void prepare_to_engrave_tuplet(int numNotes, ImoNoteRest* notes[], ImoTuplet* pTuplet)
    {
        int iInstr = 0;
        int iStaff = 0;
        int iSystem = 0;
        int iCol = 0;

        m_shapes.reserve(numNotes);
        m_pMeter = LOMSE_NEW ScoreMeter(1, 1, 180.0f);
        m_pStorage = LOMSE_NEW ShapesStorage();

        //engrave notes/rests
        m_pNoteEngrv = LOMSE_NEW NoteEngraver(m_libraryScope, m_pMeter, m_pStorage, 0, 0);
        m_pRestEngrv = LOMSE_NEW RestEngraver(m_libraryScope, m_pMeter, m_pStorage, 0, 0);
        for (int i=0; i < numNotes; ++i)
        {
            GmoShape* pShape;
            if (notes[i]->is_note())
            {
                ImoNote* pNote = dynamic_cast<ImoNote*>(notes[i]);
                pShape = m_pNoteEngrv->create_shape(pNote, k_clef_G2,
                                                    UPoint(10.0f, 15.0f) );
            }
            else
            {
                ImoRest* pRest = dynamic_cast<ImoRest*>(notes[i]);
                pShape = m_pRestEngrv->create_shape(pRest, UPoint(10.0f, 15.0f));
            }
            m_shapes.push_back(pShape);
        }

        //send note data to tuplet engraver
        for (int i=0; i < numNotes; ++i)
        {
            if (i == 0)
            {
                //first note
                m_pTupletEngrv = LOMSE_NEW MyTupletEngraver(m_libraryScope, m_pMeter);
                m_pTupletEngrv->set_start_staffobj(pTuplet, notes[i], m_shapes[i],
                                                 iInstr, iStaff, iSystem, iCol);
                m_pStorage->save_engraver(m_pTupletEngrv, pTuplet);
            }
            else if (i == numNotes-1)
            {
                //last note
                m_pTupletEngrv->set_end_staffobj(pTuplet, notes[i], m_shapes[i],
                                               iInstr, iStaff, iSystem, iCol);
            }
            else
            {
                //intermediate note
                m_pTupletEngrv->set_middle_staffobj(pTuplet, notes[i], m_shapes[i],
                                                  iInstr, iStaff, iSystem, iCol);
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

        m_pMeter = NULL;
        m_pStorage = NULL;
        m_pNoteEngrv = NULL;
        m_pRestEngrv = NULL;
        m_pTupletEngrv = NULL;
        m_pTupletShape = NULL;
    }



};

SUITE(TupletEngraverTest)
{

    TEST_FIXTURE(TupletEngraverTestFixture, CreateTuplet)
    {
        Document doc(m_libraryScope);
        ImoNote* note1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth);
        ImoNote* note2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth);
        ImoNoteRest* notes[] = { note1, note2 };
        string tuplets[] = { "+", "-" };

        ImoTuplet* pTuplet = create_tuplet(2, notes, tuplets, 12);

        CHECK( pTuplet != NULL );
        CHECK( pTuplet->get_actual_number() == 2 );
        CHECK( pTuplet->get_normal_number() == 3 );
        CHECK( notes[0]->is_in_tuplet() == true );
        CHECK( notes[1]->is_in_tuplet() == true );

        delete note1;
        delete note2;
    }

    TEST_FIXTURE(TupletEngraverTestFixture, FeedEngraver)
    {
        Document doc(m_libraryScope);
        ImoNote* note1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth);
        ImoNote* note2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth);
        ImoNoteRest* notes[] = { note1, note2 };
        string tuplets[] = { "+", "-" };

        ImoTuplet* pTuplet = create_tuplet(2, notes, tuplets, 12);
        prepare_to_engrave_tuplet(2, notes, pTuplet);

        MyTupletEngraver* pEngrv = dynamic_cast<MyTupletEngraver*>(m_pStorage->get_engraver(pTuplet));

        CHECK( pEngrv != NULL );
        CHECK( pEngrv == m_pTupletEngrv );

        delete note1;
        delete note2;
        delete_test_data();
    }

    TEST_FIXTURE(TupletEngraverTestFixture, CreateTupletShape)
    {
        Document doc(m_libraryScope);
        ImoNote* note1 = ImFactory::inject_note(&doc, k_step_C, 4, k_eighth);
        ImoNote* note2 = ImFactory::inject_note(&doc, k_step_F, 4, k_eighth);
        ImoNoteRest* notes[] = { note1, note2 };
        string tuplets[] = { "+", "-" };

        ImoTuplet* pTuplet = create_tuplet(2, notes, tuplets, 12);
        prepare_to_engrave_tuplet(2, notes, pTuplet);

        m_pTupletEngrv->create_shapes();
        m_pTupletShape = dynamic_cast<GmoShapeTuplet*>( m_pTupletEngrv->get_shape() );
        CHECK( m_pTupletShape != NULL );

        delete note1;
        delete note2;
        delete_test_data();
    }

    //TEST_FIXTURE(TupletEngraverTestFixture, TupletShapeAddedToNoteShape)
    //{
    //    ImoNote note1(k_step_C, 4, k_eighth);
    //    ImoNote note2(k_step_F, 4, k_eighth);
    //    ImoNoteRest* notes[] = { note1, note2 };
    //    string tuplets[] = { "+", "-" };

    //    ImoTuplet* pTuplet = create_tuplet(2, notes, tuplets, 12);
    //    prepare_to_engrave_tuplet(2, notes, pTuplet);

    //    m_pTupletEngrv->create_shapes();
    //    m_pTupletShape = dynamic_cast<GmoShapeTuplet*>( m_pTupletEngrv->get_shape() );

    //    //method prepare_to_engrave_tuplet() store notes shapes in m_shapes
    //    std::vector<GmoShape*>::iterator it;
    //    for (it = m_shapes.begin(); it != m_shapes.end(); ++it)
    //        CHECK( (*it)->find_related_shape(GmoObj::k_shape_tuplet) == m_pTupletShape );

    //    delete_test_data();
    //}


    // decide_on_stems_direction --------------------------------------------------------


}



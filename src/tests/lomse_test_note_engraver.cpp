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
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_note_engraver.h"
#include "lomse_chord_engraver.h"
#include "lomse_tuplet_engraver.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"
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
        , m_pMeter(NULL)
        , m_pEngrv(NULL)
        , m_pTuplet(NULL)
        , m_pTupletData1(NULL)
        , m_pTupletData2(NULL)
        , m_pTupletData3(NULL)
        , m_pNote1(NULL)
        , m_pNote2(NULL)
        , m_pNote3(NULL)
        , m_pShape1(NULL)
        , m_pShape2(NULL)
        , m_pShape3(NULL)
        , m_pStorage(NULL)
    {
    }

    ~NoteEngraverTestFixture()    //TearDown fixture
    {
    }

    void create_tuplet()
    {
        delete_tuplet();
        Document doc(m_libraryScope);
        m_pTuplet = static_cast<ImoTuplet*>( ImFactory::inject(k_imo_tuplet, &doc) );

        m_pNote1 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        m_pNote1->set_step(0);
        m_pNote1->set_octave(4);
        m_pNote1->set_notated_accidentals(k_no_accidentals);
        m_pNote1->set_note_type(k_eighth);
        m_pNote1->set_dots(0);

        ImoTupletDto dto1;
        dto1.set_tuplet_type(ImoTupletDto::k_start);
        m_pTupletData1 = ImFactory::inject_tuplet_data(&doc, &dto1);

        m_pNote1->include_in_relation(&doc, m_pTuplet, m_pTupletData1);
        m_pNote1->set_step(2);

        m_pNote2 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        m_pNote2->set_step(4);

        ImoTupletDto dto2;
        dto2.set_tuplet_type(ImoTupletDto::k_continue);
        m_pTupletData2 = ImFactory::inject_tuplet_data(&doc, &dto2);

        m_pNote2->include_in_relation(&doc, m_pTuplet, m_pTupletData2);

        m_pNote3 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));

        ImoTupletDto dto3;
        dto3.set_tuplet_type(ImoTupletDto::k_stop);
        m_pTupletData3 = ImFactory::inject_tuplet_data(&doc, &dto3);
        m_pNote3->include_in_relation(&doc, m_pTuplet, m_pTupletData3);

        m_pMeter = LOMSE_NEW ScoreMeter(1, 1, 180.0f);
        m_pStorage = LOMSE_NEW ShapesStorage();
        m_pEngrv = LOMSE_NEW NoteEngraver(m_libraryScope, m_pMeter, m_pStorage);
        m_pShape1 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote1, 0, 0, k_clef_G2, UPoint(10.0f, 15.0f)) );
        m_pShape2 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote2, 0, 0, k_clef_G2, UPoint(10.0f, 15.0f)) );
        m_pShape3 =
            dynamic_cast<GmoShapeNote*>(m_pEngrv->create_shape(m_pNote3, 0, 0, k_clef_G2, UPoint(10.0f, 15.0f)) );
    }

    void delete_tuplet()
    {
        delete m_pMeter;
        delete m_pEngrv;
//        delete m_pTuplet;
//        delete m_pTupletData1;
//        delete m_pTupletData2;
//        delete m_pTupletData3;
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
    ImoTupletData* m_pTupletData1;
    ImoTupletData* m_pTupletData2;
    ImoTupletData* m_pTupletData3;
    ImoNote* m_pNote1;
    ImoNote* m_pNote2;
    ImoNote* m_pNote3;
    GmoShapeNote* m_pShape1;
    GmoShapeNote* m_pShape2;
    GmoShapeNote* m_pShape3;
    ShapesStorage* m_pStorage;

};

SUITE(NoteEngraverTest)
{

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ShapeInBlock)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote->set_step(0);
        pNote->set_octave(4);
        pNote->set_notated_accidentals(k_no_accidentals);
        pNote->set_note_type(k_whole);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, 0, 0, k_clef_F4, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_note() == true );
        std::list<GmoShape*>& components = pShape->get_components();
        std::list<GmoShape*>::iterator it = components.begin();
        CHECK( components.size() == 1 );
        CHECK( (*it)->is_shape_notehead() );

        delete pNote;
        delete pShape;
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_HeadAndStem)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote->set_step(0);
        pNote->set_octave(4);
        pNote->set_notated_accidentals(k_no_accidentals);
        pNote->set_note_type(k_quarter);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, 0, 0, k_clef_F4, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != NULL );
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
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote->set_step(0);
        pNote->set_octave(4);
        pNote->set_notated_accidentals(k_no_accidentals);
        pNote->set_note_type(k_eighth);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, 0, 0, k_clef_F4, UPoint(10.0f, 15.0f)) );
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

        delete pNote;
        delete pShape;
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_ShapeInBlockWithDot)
    {
        Document doc(m_libraryScope);
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote->set_step(0);
        pNote->set_octave(4);
        pNote->set_notated_accidentals(k_no_accidentals);
        pNote->set_note_type(k_whole);
        pNote->set_dots(1);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, 0, 0, k_clef_F4, UPoint(10.0f, 15.0f)) );
        CHECK( pShape != NULL );
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
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote->set_step(0);
        pNote->set_octave(4);
        pNote->set_notated_accidentals(k_no_accidentals);
        pNote->set_note_type(k_eighth);
        pNote->set_dots(2);

        ScoreMeter meter(1, 1, 180.0f);
        ShapesStorage storage;
        NoteEngraver engraver(m_libraryScope, &meter, &storage);
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(pNote, 0, 0, k_clef_F4, UPoint(10.0f, 15.0f)) );
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

        delete pNote;
        delete pShape;
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



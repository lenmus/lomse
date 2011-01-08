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
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"

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

        NoteEngraver engraver(m_libraryScope);
        LUnits lineSpacing = 180.0f;
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, ImoClef::k_F4, UPoint(10.0f, 15.0f), lineSpacing) );
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_note() == true );
        std::vector<GmoShape*>& components = pShape->get_components();
        CHECK( components.size() == 1 );
        CHECK( components[0]->is_shape_notehead() );
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_HeadAndStem)
    {
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(ImoNote::k_quarter);
        ImoNote note(dtoNote);

        NoteEngraver engraver(m_libraryScope);
        LUnits lineSpacing = 180.0f;
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, ImoClef::k_F4, UPoint(10.0f, 15.0f), lineSpacing) );
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_note() == true );
        std::vector<GmoShape*>& components = pShape->get_components();
        CHECK( components.size() == 2 );
        CHECK( components[0]->is_shape_notehead() );
        CHECK( components[1]->is_shape_stem() );
    }

    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_HeadStemAndFlag)
    {
        DtoNote dtoNote;
        dtoNote.set_step(0);
        dtoNote.set_octave(4);
        dtoNote.set_accidentals(0);
        dtoNote.set_note_type(ImoNote::k_eighth);
        ImoNote note(dtoNote);

        NoteEngraver engraver(m_libraryScope);
        LUnits lineSpacing = 180.0f;
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, ImoClef::k_F4, UPoint(10.0f, 15.0f), lineSpacing) );
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_note() == true );
        std::vector<GmoShape*>& components = pShape->get_components();
        CHECK( components.size() == 3 );
        CHECK( components[0]->is_shape_notehead() );
        CHECK( components[1]->is_shape_stem() );
        CHECK( components[2]->is_shape_flag() );
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

        NoteEngraver engraver(m_libraryScope);
        LUnits lineSpacing = 180.0f;
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, ImoClef::k_F4, UPoint(10.0f, 15.0f), lineSpacing) );
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_note() == true );
        std::vector<GmoShape*>& components = pShape->get_components();
        CHECK( components.size() == 2 );
        CHECK( components[0]->is_shape_notehead() );
        CHECK( components[1]->is_shape_dot() );
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

        NoteEngraver engraver(m_libraryScope);
        LUnits lineSpacing = 180.0f;
        GmoShapeNote* pShape =
            dynamic_cast<GmoShapeNote*>(engraver.create_shape(&note, ImoClef::k_F4, UPoint(10.0f, 15.0f), lineSpacing) );
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_note() == true );
        std::vector<GmoShape*>& components = pShape->get_components();
        CHECK( components.size() == 5 );
        CHECK( components[0]->is_shape_notehead() );
        CHECK( components[1]->is_shape_dot() );
        CHECK( components[2]->is_shape_dot() );
        CHECK( components[3]->is_shape_stem() );
        CHECK( components[4]->is_shape_flag() );
    }

//    TEST_FIXTURE(NoteEngraverTestFixture, NoteEngraver_MeasureWidthDefaultStyle)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (name ''Violin'')(musicData (n c4 q))))))");
//        ImoScore* pScore = doc.get_score();
//        ImoInstrument* pInstr = pScore->get_instrument(0);
//        ImoScoreText& text = pInstr->get_name();
//        NoteEngraver engraver(m_libraryScope, text, pScore);
//
//        LUnits width = engraver.measure_width();
//        CHECK( width > 0.0f );
//    }

}



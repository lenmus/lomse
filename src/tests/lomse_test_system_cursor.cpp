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
#include "lomse_ldp_factory.h"
#include "lomse_document.h"
#include "lomse_staffobjs_table.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_pitch.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class SystemCursorTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    SystemCursorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~SystemCursorTestFixture()    //TearDown fixture
    {
    }
};

SUITE(SystemCursorTest)
{

    TEST_FIXTURE(SystemCursorTestFixture, SystemCursor_InitiallyNoProlog)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q v2)(n d4 e.)(n d4 s v3)(n e4 h) ))) "
            "))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);

        CHECK( cursor.get_num_instruments() == 1 );
        CHECK( cursor.get_clef_for_instr_staff(0, 0) == NULL );
        CHECK( cursor.get_key_type_for_instr_staff(0, 0) == k_key_undefined );
    }

    TEST_FIXTURE(SystemCursorTestFixture, SystemCursor_InitiallyNoPrologTwoInstr)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
                        "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
                        "(key D)(time 2 4)(n f4 w p1)(goBack w)(n c3 e g+ p2)"
                        "(n c3 e g-)(n d3 q)(barline)))"
                        "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
                        "(key D)(time 2 4)(n f4 q. p1)(clef F4 p1)(n a3 e)"
                        "(goBack h)(n c3 q p2)(n c3 e)(clef G p2)(clef F4 p2)"
                        "(n c3 e)(barline)))  )))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);
        CHECK( cursor.get_num_instruments() == 2 );
        CHECK( cursor.get_clef_for_instr_staff(0, 0) == NULL );
        CHECK( cursor.get_clef_for_instr_staff(0, 1) == NULL );
        CHECK( cursor.get_clef_for_instr_staff(1, 0) == NULL );
        CHECK( cursor.get_clef_for_instr_staff(1, 1) == NULL );
        CHECK( cursor.get_key_type_for_instr_staff(0, 0) == k_key_undefined );
        CHECK( cursor.get_key_type_for_instr_staff(0, 1) == k_key_undefined );
        CHECK( cursor.get_key_type_for_instr_staff(1, 0) == k_key_undefined );
        CHECK( cursor.get_key_type_for_instr_staff(1, 1) == k_key_undefined );
    }

    TEST_FIXTURE(SystemCursorTestFixture, SystemCursor_PrologUpdateClef)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key a)(n c4 q v2)(n d4 e.)(n d4 s v3)(n e4 h) ))) "
            "))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);
        cursor.move_next();     //points to key

        CHECK( cursor.get_applicable_clef_type() == k_clef_G2 );
        CHECK( cursor.get_key_type_for_instr_staff(0, 0) == k_key_undefined );
    }

    TEST_FIXTURE(SystemCursorTestFixture, SystemCursor_PrologUpdateKey)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key a)(n c4 q v2)(n d4 e.)(n d4 s v3)(n e4 h) ))) "
            "))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);
        cursor.move_next();     //points to key
        cursor.move_next();     //points to first note

        CHECK( cursor.get_applicable_clef_type() == k_clef_G2 );
        CHECK( cursor.get_key_type_for_instr_staff(0, 0) == k_key_a );
    }

    TEST_FIXTURE(SystemCursorTestFixture, SystemCursor_PrologUpdateClefKey)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
                        "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
                        "(key D)(time 2 4)(n f4 w p1)(goBack w)(n c3 e g+ p2)"
                        "(n c3 e g-)(n d3 q)(barline)))"
                        "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
                        "(key D)(time 2 4)(n f4 q. p1)(clef F4 p1)(n a3 e)"
                        "(goBack h)(n c3 q p2)(n c3 e)(clef G p2)(clef F4 p2)"
                        "(n c3 e)(barline)))  )))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);

        CHECK( cursor.get_num_instruments() == 2 );

        //start                   points to instr0 staff0   (clef G p1)
        cursor.move_next();     //points to instr0 staff0   (clef F4 p2)

        CHECK( cursor.get_clef_type_for_instr_staff(0, 0) == k_clef_G2 );
        CHECK( cursor.get_clef_type_for_instr_staff(0, 1) == k_clef_undefined );
        CHECK( cursor.get_clef_type_for_instr_staff(1, 0) == k_clef_undefined );
        CHECK( cursor.get_clef_type_for_instr_staff(1, 1) == k_clef_undefined );

        cursor.move_next();     //points to instr0 staff0   (key D)
        cursor.move_next();     //points to instr0 staff1   [(key D)]

        CHECK( cursor.get_key_type_for_instr_staff(0, 0) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(0, 1) == k_key_undefined );
        CHECK( cursor.get_key_type_for_instr_staff(1, 0) == k_key_undefined );
        CHECK( cursor.get_key_type_for_instr_staff(1, 1) == k_key_undefined );

        cursor.move_next();     //points to instr0 staff1   (time 2 4)
        cursor.move_next();     //points to instr0 staff1   [(time 2 4)]
        cursor.move_next();     //points to instr0 staff0   (n f4 w p1)

        CHECK( cursor.get_clef_type_for_instr_staff(0, 0) == k_clef_G2 );
        CHECK( cursor.get_clef_type_for_instr_staff(0, 1) == k_clef_F4 );
        CHECK( cursor.get_clef_type_for_instr_staff(1, 0) == k_clef_undefined );
        CHECK( cursor.get_clef_type_for_instr_staff(1, 1) == k_clef_undefined );
        CHECK( cursor.get_key_type_for_instr_staff(0, 0) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(0, 1) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(1, 0) == k_key_undefined );
        CHECK( cursor.get_key_type_for_instr_staff(1, 1) == k_key_undefined );

        cursor.move_next();     //points to instr0 staff1   (n c3 e g+ p2)
        cursor.move_next();     //points to instr1 staff0   (clef G p1)
        cursor.move_next();     //points to instr1 staff1   (clef F4 p2)
        cursor.move_next();     //points to instr1 staff0   (key D)
        cursor.move_next();     //points to instr1 staff1   [(key D)]
        cursor.move_next();     //points to instr1 staff0   (time 2 4)

        CHECK( cursor.get_clef_type_for_instr_staff(0, 0) == k_clef_G2 );
        CHECK( cursor.get_clef_type_for_instr_staff(0, 1) == k_clef_F4 );
        CHECK( cursor.get_clef_type_for_instr_staff(1, 0) == k_clef_G2 );
        CHECK( cursor.get_clef_type_for_instr_staff(1, 1) == k_clef_F4 );
        CHECK( cursor.get_key_type_for_instr_staff(0, 0) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(0, 1) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(1, 0) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(1, 1) == k_key_D );

        cursor.move_next();     //points to instr1 staff1   [(time 2 4)]
        cursor.move_next();     //points to instr1 staff0   (n f4 q. p1)
        cursor.move_next();     //points to instr1 staff1   (n c3 q p2)
        CHECK( cursor.get_key_type_for_instr_staff(1, 1) == k_key_D );
        cursor.move_next();     //points to instr0 staff1   (n c3 e g-)
        cursor.move_next();     //points to instr0 staff1   (n d3 q)
        cursor.move_next();     //points to instr1 staff1   (n c3 e)
        cursor.move_next();     //points to instr1 staff0   (clef F4 p1)
        cursor.move_next();     //points to instr1 staff0   (n a3 e)

        CHECK( cursor.get_clef_type_for_instr_staff(0, 0) == k_clef_G2 );
        CHECK( cursor.get_clef_type_for_instr_staff(0, 1) == k_clef_F4 );
        CHECK( cursor.get_clef_type_for_instr_staff(1, 0) == k_clef_F4 );
        CHECK( cursor.get_clef_type_for_instr_staff(1, 1) == k_clef_F4 );
        CHECK( cursor.get_key_type_for_instr_staff(0, 0) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(0, 1) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(1, 0) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(1, 1) == k_key_D );

        cursor.move_next();     //points to instr1 staff1   (clef G p2)
        cursor.move_next();     //points to instr1 staff1   (clef F p2)
        cursor.move_next();     //points to instr1 staff1   (n c3 e)

        CHECK( cursor.get_clef_type_for_instr_staff(0, 0) == k_clef_G2 );
        CHECK( cursor.get_clef_type_for_instr_staff(0, 1) == k_clef_F4 );
        CHECK( cursor.get_clef_type_for_instr_staff(1, 0) == k_clef_F4 );
        CHECK( cursor.get_clef_type_for_instr_staff(1, 1) == k_clef_F4 );
        CHECK( cursor.get_key_type_for_instr_staff(0, 0) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(0, 1) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(1, 0) == k_key_D );
        CHECK( cursor.get_key_type_for_instr_staff(1, 1) == k_key_D );
    }

    TEST_FIXTURE(SystemCursorTestFixture, SystemCursor_GetApplicableClef)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
                        "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
                        "(key D)(time 2 4)(n f4 w p1)(goBack w)(n c3 e g+ p2)"
                        "(n c3 e g-)(n d3 q)(barline)))"
                        "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
                        "(key D)(time 2 4)(n f4 q. p1)(clef F4 p1)(n a3 e)"
                        "(goBack h)(n c3 q p2)(n c3 e)(clef G p2)(clef F4 p2)"
                        "(n c3 e)(barline)))  )))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        cout << pScore->get_staffobjs_table()->dump();
        StaffObjsCursor cursor(pScore);

        //start                   points to instr0 staff0   (clef G p1)
        cursor.move_next();     //points to instr0 staff1   (clef F4 p2)
        cursor.move_next();     //points to instr0 staff0   (key D)
        cursor.move_next();     //points to instr0 staff1   [(key D)]
        cursor.move_next();     //points to instr0 staff0   (time 2 4)

        CHECK( cursor.get_applicable_clef_type() == k_clef_G2 );

        cursor.move_next();     //points to instr0 staff1   [(time 2 4)]
        cursor.move_next();     //points to instr0 staff0   (n f4 w p1)
        cursor.move_next();     //points to instr0 staff1   (n c3 e g+ p2)

        CHECK( cursor.get_applicable_clef_type() == k_clef_F4 );

        cursor.move_next();     //points to instr1 staff0   (clef G p1)
        cursor.move_next();     //points to instr1 staff1   (clef F4 p2)
        cursor.move_next();     //points to instr1 staff0   (key D)
        cursor.move_next();     //points to instr1 staff1   [(key D)]
        cursor.move_next();     //points to instr1 staff0   (time 2 4)
        cursor.move_next();     //points to instr0 staff1   [(time 2 4)]
        cursor.move_next();     //points to instr1 staff0   (n f4 q. p1)

        CHECK( cursor.get_applicable_clef_type() == k_clef_G2 );

        cursor.move_next();     //points to instr1 staff1   (n c3 q p2)
        cursor.move_next();     //points to instr0 staff1   (n c3 e g-)
        cursor.move_next();     //points to instr0 staff1   (n d3 q)
        cursor.move_next();     //points to instr1 staff1   (n c3 e)
        cursor.move_next();     //points to instr1 staff0   (clef F4 p1)
        cursor.move_next();     //points to instr1 staff0   (n a3 e)
        cursor.move_next();     //points to instr1 staff1   (clef G p2)

        CHECK( cursor.get_applicable_clef_type() == k_clef_F4 );

        cursor.move_next();     //points to instr1 staff1   (clef F p2)

        CHECK( cursor.get_applicable_clef_type() == k_clef_G2 );

        cursor.move_next();     //points to instr1 staff1   (n c3 e)

        CHECK( cursor.get_applicable_clef_type() == k_clef_F4 );

        cursor.move_next();     //points to instr0 staff0   (barline)
        cursor.move_next();     //points to instr1 staff0   (barline)

        CHECK( cursor.get_applicable_clef_type() == k_clef_F4 );

    }

    TEST_FIXTURE(SystemCursorTestFixture, SystemCursor_EmptyScore)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData ) )) "
            "))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);

        CHECK( cursor.get_num_instruments() == 1 );
        CHECK( cursor.get_applicable_clef_type() == k_clef_undefined );
        CHECK( cursor.get_applicable_key_type() == k_key_undefined );
    }

    TEST_FIXTURE(SystemCursorTestFixture, SystemCursor_MoveNext)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key a)(n c4 q v2)(n d4 e.)(n d4 s v3)(n e4 h) ))) "
            "))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);
                                //points to (clef G)
        cursor.move_next();     //points to (key a)
        cursor.move_next();     //points to (n c4 q v2)
        cursor.move_next();     //points to (n d4 e.)

        ImoNote* pNote = dynamic_cast<ImoNote*>( cursor.imo_object() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_step() == k_step_D );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_dots() == 1 );
    }

    TEST_FIXTURE(SystemCursorTestFixture, SystemCursor_SaveRestorePos)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key a)(n c4 q v2)(n d4 e.)(n d4 s v3)(n e4 h) ))) "
            "))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);
                                //points to (clef G)
        cursor.move_next();     //points to (key a)
        cursor.save_position();
        cursor.move_next();     //points to (n c4 q v2)
        cursor.move_next();     //points to (n d4 e.)

        cursor.go_back_to_saved_position();  //points to (key a)
        ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( cursor.imo_object() );
        CHECK( pKey != NULL );

        cursor.move_next();     //points to (n c4 q v2)
        cursor.move_next();     //points to (n d4 e.)
        ImoNote* pNote = dynamic_cast<ImoNote*>( cursor.imo_object() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_step() == k_step_D );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_dots() == 1 );
    }


    // time signature -------------------------------------------------------------------

    TEST_FIXTURE(SystemCursorTestFixture, InitiallyNoTimeSignature)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q v2)(n d4 e.)(n d4 s v3)(n e4 h) ))) "
            "))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);

        CHECK( cursor.get_time_signature_for_instrument(0) == NULL );
    }

    TEST_FIXTURE(SystemCursorTestFixture, InitiallyTimeSignatureTwoInstr)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
                        "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
                        "(key D)(time 2 4)(n f4 w p1)(goBack w)(n c3 e g+ p2)"
                        "(n c3 e g-)(n d3 q)(barline)))"
                        "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
                        "(key D)(time 2 4)(n f4 q. p1)(clef F4 p1)(n a3 e)"
                        "(goBack h)(n c3 q p2)(n c3 e)(clef G p2)(clef F4 p2)"
                        "(n c3 e)(barline)))  )))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);
        CHECK( cursor.get_num_instruments() == 2 );
        CHECK( cursor.get_time_signature_for_instrument(0) == NULL );
        CHECK( cursor.get_time_signature_for_instrument(1) == NULL );
    }

    TEST_FIXTURE(SystemCursorTestFixture, PrologUpdateTimeSignature)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key a)(time 2 4)(n c4 q v2)(n d4 e.)"
            "(n d4 s v3)(n e4 h) ))) ))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);
        cursor.move_next();     //points to (clef G)
        cursor.move_next();     //points to (key a)
        cursor.move_next();     //points to (time 2 4)
        cursor.move_next();     //points to (n c4 q v2)
        cursor.move_next();     //points to (n d4 e.)

        ImoTimeSignature* pTS = cursor.get_time_signature_for_instrument(0);
        CHECK( pTS != NULL );
        CHECK( pTS->get_top_number() == 2 );
        CHECK( pTS->get_bottom_number() == 4 );
    }


}



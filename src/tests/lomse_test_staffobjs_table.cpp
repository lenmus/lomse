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
#include "lomse_staffobjs_table.h"
#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_document.h"
#include "lomse_score_iterator.h"
#include "lomse_model_builder.h"
#include "lomse_time.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// StaffVoiceLineTable tests
//=======================================================================================
class StaffVoiceLineTableTestFixture
{
public:

    StaffVoiceLineTableTestFixture()     //SetUp fixture
    {
    }

    ~StaffVoiceLineTableTestFixture()    //TearDown fixture
    {
    }
};

SUITE(StaffVoiceLineTableTest)
{

    TEST_FIXTURE(StaffVoiceLineTableTestFixture, StaffVoiceLineTable_Voice0FirstStaff)
    {
        StaffVoiceLineTable table;
        CHECK( table.get_line_assigned_to(0, 0) == 0 );
    }

    TEST_FIXTURE(StaffVoiceLineTableTestFixture, StaffVoiceLineTable_Voice0Staff1)
    {
        StaffVoiceLineTable table;
        //cout << table.get_line_assigned_to(0, 1) << endl;
        CHECK( table.get_line_assigned_to(0, 1) == 1 );
    }

    TEST_FIXTURE(StaffVoiceLineTableTestFixture, StaffVoiceLineTable_FirstVoiceAssignedToDefault)
    {
        StaffVoiceLineTable table;
        table.get_line_assigned_to(0, 1);
        CHECK( table.get_line_assigned_to(3, 1) == 1 );
    }

}


//---------------------------------------------------------------------------------------
// helper macros
// CHECK_ENTRY: checks/displays ids
// CHECK_ENTRY0: does not check/displays ids
#define CHECK_ENTRY(it, _instr, _staff, _measure, _time, _line, _object) \
            CHECK( (*it)->num_instrument() == _instr );         \
            CHECK( (*it)->staff() == _staff );                  \
            CHECK( (*it)->measure() == _measure );              \
            CHECK( is_equal_time((*it)->time(), _time) );       \
            CHECK( (*it)->line() == _line );                    \
            CHECK( (*it)->to_string_with_ids() == _object );

#define CHECK_ENTRY0(it, _instr, _staff, _measure, _time, _line, _object) \
            CHECK( (*it)->num_instrument() == _instr );         \
            CHECK( (*it)->staff() == _staff );                  \
            CHECK( (*it)->measure() == _measure );              \
            CHECK( is_equal_time((*it)->time(), _time) );       \
            CHECK( (*it)->line() == _line );                    \
            CHECK( (*it)->to_string() == _object );             \
            ++it;


//=======================================================================================
// ColStaffObjsBuilder test
//=======================================================================================
class ColStaffObjsBuilderTestFixture
{
public:
    LibraryScope m_libraryScope;
    Document* m_pDoc;
    LdpTree* m_pTree;
    LdpFactory* m_pLdpFactory;

    ColStaffObjsBuilderTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pDoc(nullptr)
        , m_pTree(nullptr)
    {
        m_pLdpFactory = m_libraryScope.ldp_factory();
    }

    ~ColStaffObjsBuilderTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    ImoScore* create_score(const string &ldp)
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text(ldp);
        LdpTree* pTree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, m_pDoc);
        ImoObj* pImo = a.analyse_tree_and_get_object(pTree);
        delete pTree->get_root();
        return dynamic_cast<ImoScore*>(pImo);
    }

};

SUITE(ColStaffObjsBuilderTest)
{

    // ColStaffObjsBuilderEngine2x ------------------------------------------------------

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_1)
    {
        //empty score creates empty table
        ImoScore* pScore = create_score(
            "(score (vers 2.0)(instrument (musicData "
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 0 );
        CHECK( pTable->is_anacrusis_start() == false );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_2)
    {
        //each voice uses a different time counter
        ImoScore* pScore = create_score(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 q v1)(n c5 e v2)(n e4 e v1)(n e5 e v2)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 5 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
        //cout << pTable->dump();
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c5 e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(n e5 e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n e4 e v1 p1)" );
        CHECK( pTable->min_note_duration() == 32.0 );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_3)
    {
        //barline is placed at maximum time reached in the measure
        ImoScore* pScore = create_score(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 q v1)(n c5 e v2)(n e4 e v1)(n e5 w v2)(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
//        cout << pTable->dump();
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c5 e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(n e5 w v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n e4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0, 288,     0, "(barline simple)" );
        CHECK( pTable->min_note_duration() == 32.0 );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_4)
    {
        //after barline voices are correctly positioned
        ImoScore* pScore = create_score(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 q v1)(n c5 e v2)(n e4 e v1)(n e5 w v2)(barline)"
            "(n c4 q v1)(n c5 e v2)(n e4 e v1)(n e5 w v2)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 10 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
//        cout << pTable->dump();
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c5 e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(n e5 w v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n e4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0, 288,     0, "(barline simple)" );
        CHECK_ENTRY0(it, 0,    0,      1, 288,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      1, 288,     1, "(n c5 e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      1, 320,     1, "(n e5 w v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      1, 352,     0, "(n e4 e v1 p1)" );
        CHECK( pTable->min_note_duration() == 32.0 );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_5)
    {
        //key and time signatures generate secondary entries
        ImoScore* pScore = create_score(
            "(score (vers 2.0) (instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
        //cout << pTable->dump();
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    1,      0,   0,     1, "(clef F4 p2)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(key C)" );
        CHECK_ENTRY0(it, 0,    1,      0,   0,     1, "(key C)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time 2 4)" );
        CHECK_ENTRY0(it, 0,    1,      0,   0,     1, "(time 2 4)" );
        CHECK( pTable->min_note_duration() == LOMSE_NO_NOTE_DURATION );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_6)
    {
        //chord notes have time and staff properly assigned
        ImoScore* pScore = create_score(
            "(score (vers 2.0) (instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key C)(time 2 4)(chord (n c3 w p2)(n g3 w p2)"
            "(n e4 w p1)(n c5 w p1))(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 11 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
        //cout << pTable->dump();
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    1,      0,   0,     1, "(clef F4 p2)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(key C)" );
        CHECK_ENTRY0(it, 0,    1,      0,   0,     1, "(key C)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time 2 4)" );
        CHECK_ENTRY0(it, 0,    1,      0,   0,     1, "(time 2 4)" );
        CHECK_ENTRY0(it, 0,    1,      0,   0,     1, "(chord (n c3 w v1 p2)" );
        CHECK_ENTRY0(it, 0,    1,      0,   0,     1, "(n g3 w v1 p2)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 w v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c5 w v1 p1))" );
        CHECK_ENTRY0(it, 0,    0,      0, 256,     0, "(barline simple)" );
        CHECK( pTable->min_note_duration() == 256.0 );
    }

#if LOMSE_COMPATIBILITY_LDP_1_5
    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_7)
    {
        //anchor staffobjs
        ImoScore* pScore = create_score(
            "(score (vers 1.5) (instrument (musicData "
            "(clef G)(key C)(n f4 q)(text \"Hello world\")(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 5 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
        //cout << pTable->dump();
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(key C)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n f4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(dir 0 p1 (text \"Hello world\"))" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(barline simple)" );
        CHECK( pTable->min_note_duration() == 64.0 );
    }
#endif

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_8)
    {
        //anacruxis
        ImoScore* pScore = create_score(
            "(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 3 4)(n c4 q)(barline)(n d4 e.)(n d4 s)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacrusis_start() == true );
        CHECK( is_equal_time( pTable->anacrusis_missing_time(), 128.0f) == true );

//        cout << test_name() << endl;
        //cout << pTable->dump();
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time 3 4)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(barline simple)" );
        CHECK_ENTRY0(it, 0,    0,      1,  64,     0, "(n d4 e. v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      1, 112,     0, "(n d4 s v1 p1)" );
        CHECK( pTable->min_note_duration() == 16.0 );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_9)
    {
        //goFwd replaced by special invisible rest
        ImoScore* pScore = create_score(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 e v1)(goFwd e v1)(n e4 e v1)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 4 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
        //cout << pTable->dump();
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(goFwd e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n e4 e v1 p1)" );
        CHECK( pTable->min_note_duration() == 32.0 );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_10)
    {
        //notes in other voices intermixed in beamed group
        ImoScore* pScore = create_score(
            "(score (vers 2.0)(instrument#100 (musicData "
            "(clef F4)(n e3 e g+)(n g3 e)(n c4 e g-)"
            "(n c2 w v3)(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
//        cout << pTable->dump();
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef F4 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e3 e v1 p1 (beam 106 +))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c2 w v3 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n g3 e v1 p1 (beam 106 =))" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n c4 e v1 p1 (beam 106 -))" );
        CHECK_ENTRY0(it, 0,    0,      0, 256,     0, "(barline simple)" );
        CHECK( pTable->min_note_duration() == 32.0 );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_11)
    {
        //intermediate non-time get assigned right time
        ImoScore* pScore = create_score(
            "(score (vers 2.0)(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(n c4 q p1 v1)(n e4 e v1)"
            "(n c3 e p2 v2)(clef G p2)(n c4 e v2)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 7 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
//        cout << pTable->dump();
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    1,      0,   0,     1, "(clef F4 p2)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    1,      0,   0,     1, "(n c3 e v2 p2)" );
        CHECK_ENTRY0(it, 0,    1,      0,  32,     1, "(clef G p2)" );
        CHECK_ENTRY0(it, 0,    1,      0,  32,     1, "(n c4 e v2 p2)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n e4 e v1 p1)" );
        CHECK( pTable->min_note_duration() == 32.0 );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_12)
    {
        //barlines properly ordered and with right time
        ImoScore* pScore = create_score(
            "(score (vers 2.0)"
            "(instrument#100 (staves 2)"
            "(musicData (clef G p1)(clef F4 p2)"
            "(key D)(time 2 4)(n f4 h p1 v1)"
            "(n c3 e g+ p2 v2)"
            "(n c3 e g-)(n d3 q)"
            "(barline)))"
            "(instrument (staves 2)"
            "(musicData (clef G p1)(clef F4 p2)"
            "(key D)(time 2 4)(n f4 q. p1 v1)(clef F4 p1)(n a3 e)"
            "(n c3 q p2 v2)(n c3 e)(clef G p2)(clef F4 p2)"
            "(n c3 e)(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 4 );
        CHECK( pTable->num_entries() == 26 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
//        cout << pTable->dump();

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,     0,	    0,	0,	    0,	"(clef G p1)" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	0,	    1,	"(clef F4 p2)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(key D)" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	0,	    1,	"(key D)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(time 2 4)" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	0,	    1,	"(time 2 4)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    2,	"(clef G p1)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	0,	    3,	"(clef F4 p2)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    2,	"(key D)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	0,	    3,	"(key D)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    2,	"(time 2 4)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	0,	    3,	"(time 2 4)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(n f4 h v1 p1)" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	0,	    1,	"(n c3 e v2 p2 (beam 109 +))" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    2,	"(n f4 q. v1 p1)" );  // line 4 !
        CHECK_ENTRY0(it, 1,	    1,	    0,	0,	    3,	"(n c3 q v2 p2)" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	32,	    1,	"(n c3 e v2 p2 (beam 109 -))" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	64,	    1,	"(n d3 q v2 p2)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	64,	    3,	"(n c3 e v2 p2)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	96,	    2,	"(clef F4 p1)" );  // line 4 !
        CHECK_ENTRY0(it, 1,	    1,	    0,	96,	    3,	"(clef G p2)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	96,	    3,	"(clef F4 p2)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	96,	    2,	"(n a3 e v1 p1)" );  // line 4 !
        CHECK_ENTRY0(it, 1,	    1,	    0,	96,	    3,	"(n c3 e v2 p2)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	128,	0,	"(barline simple)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	128,	2,	"(barline simple)" );
        CHECK( pTable->min_note_duration() == 32.0 );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_13)
    {
        //@13. Direction in prolog is re-ordered after prolog

        ImoScore* pScore = create_score(    //unit test score 02033
            "(score (vers 2.1)"
            "(instrument P1"
            "(musicData (clef G) (dir (metronome e 40))"
            "(key a)(time 3 8)(n e5 s v1 (beam 45 ++))"
            "(n +d5 s v1 (beam 45 --))"
            "(barline simple)"
            "))"
            "(instrument P2 (musicData"
            "(clef F4)(key a)(time 3 8)"
            "(r e v1)(barline simple)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 12 );
        CHECK( pTable->is_anacrusis_start() == true );

//        cout << test_name() << endl;
//        cout << pTable->dump();
//
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,     0,	    0,	0,	    0,	"(clef G p1)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(key a)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(time 3 8)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    1,	"(clef F4 p1)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    1,	"(key a)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    1,	"(time 3 8)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(dir 0 p1 (metronome e 40))" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(n e5 s v1 p1 (beam 31 ++))" );
        CHECK( pTable->min_note_duration() == 16.0 );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_14)
    {
        //@14. Direction before prolog is re-ordered

        ImoScore* pScore = create_score(    //unit test score 02034
            "(score (vers 2.1)"
            "(instrument P1"
            "(musicData (dir (metronome e 40))(clef G)"
            "(key a)(time 3 8)(n e5 s v1 (beam 45 ++))"
            "(n +d5 s v1 (beam 45 --))"
            "(barline simple)"
            "))"
            "(instrument P2 (musicData"
            "(clef F4)(key a)(time 3 8)"
            "(r e v1)(barline simple)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 12 );
        CHECK( pTable->is_anacrusis_start() == true );

//        cout << test_name() << endl;
//        cout << pTable->dump();
//
        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,     0,	    0,	0,	    0,	"(clef G p1)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(key a)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(time 3 8)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    1,	"(clef F4 p1)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    1,	"(key a)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    1,	"(time 3 8)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(dir 0 p1 (metronome e 40))" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(n e5 s v1 p1 (beam 31 ++))" );
        CHECK( pTable->min_note_duration() == 16.0 );
    }

    // ColStaffObjsBuilderEngine1x ------------------------------------------------------


    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsAddEntries)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData (n c4 q) (barline simple))))))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);

//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_entries() == 2 );
        CHECK( pColStaffObjs->num_lines() == 1 );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ScoreIteratorPointsFirst)
    {
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (barline simple))))))" );
        LdpTree* tree = parser.get_ldp_tree();
        Document doc(m_libraryScope);
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();

//        cout << test_name() << endl;
       //cout << (*it)->dump();
        //cout << (*it)->to_string() << endl;
        //CHECK( (*it)->to_string() == "(n c4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsChangeMeasure)
    {
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData (n c4 q) (barline simple)"
            "(n d4 e) (barline simple) (n e4 w))))))" );
        LdpTree* tree = parser.get_ldp_tree();
        Document doc(m_libraryScope);
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 5 );
        CHECK( pColStaffObjs->num_lines() == 1 );

        //CHECK( (*it)->to_string() == "(n c4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(barline simple)" );
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->measure() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 e)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(barline simple)" );
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->measure() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n e4 w)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 2 );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsTimeInSequence)
    {
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
                "(score (vers 1.6) (instrument (musicData "
                "(n c4 q)(n d4 e.)(n d4 s)(n e4 h)))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        Document doc(m_libraryScope);
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 4 );
        CHECK( pColStaffObjs->num_lines() == 1 );

        //CHECK( (*it)->to_string() == "(n c4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 64.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 s)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 112.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n e4 h)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 128.0f) );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsTimeGoBack)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData"
            "(n c4 q)(n d4 e.)(n d4 s)(goBack start)(n e4 h)(n g4 q)))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);

        CHECK( pColStaffObjs->num_entries() == 5 );
        CHECK( pColStaffObjs->num_lines() == 1 );
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        ColStaffObjsIterator it = pColStaffObjs->begin();
        //CHECK( (*it)->to_string() == "(n c4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n e4 h)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 64.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 s)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 112.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n g4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 128.0f) );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, GoBack_StartTime)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData"
            "(n c4 q)(n d4 e.)(n e4 s)(barline)"
            "(n f4 q)(n g4 e.)(n a4 s)(goBack start)(n b4 q)(n c5 q)))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);

        CHECK( pColStaffObjs->num_entries() == 9 );
        CHECK( pColStaffObjs->num_lines() == 1 );
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        ColStaffObjsIterator it = pColStaffObjs->begin();
        //CHECK( (*it)->to_string() == "(n c4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 64.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n e4 s)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 112.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(barline )" );
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 128.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n f4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 128.0f) );
        ++it;   //(n b4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 128.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n g4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 192.0f) );
        ++it;   //(n g4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 192.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n a4 s)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 240.0f) );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsTimeGoFwd)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(n c4 q)(n d4 e.)(n d4 s)(goBack start)(n e4 q)(goFwd end)(barline)))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);

        CHECK( pColStaffObjs->num_entries() == 5 );
        CHECK( pColStaffObjs->num_lines() == 1 );
//        cout << test_name() << endl;
        //cout << pColStaffObjs->dump();

        ColStaffObjsIterator it = pColStaffObjs->begin();
        //CHECK( (*it)->to_string() == "(n c4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n e4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( is_equal_time((*it)->time(), 64.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 s)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( is_equal_time((*it)->time(), 112.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(barline )" );
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( is_equal_time((*it)->time(), 128.0f) );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsStaffAssigned)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(n c4 q p2)(n d4 e.)(n d4 s p3)(n e4 h)))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();
//        cout << test_name() << endl;
        //cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 4 );

        //CHECK( (*it)->to_string() == "(n c4 q p2)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 s p3)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->staff() == 2 );
        ++it;
        //CHECK( (*it)->to_string() == "(n e4 h)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->staff() == 2 );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, Anacrusis)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(clef G)(time 3 4)(n c4 q)(barline)(n d4 e.)(n d4 s)) )) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();
//        cout << test_name() << endl;
        //cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 6 );
        CHECK( pColStaffObjs->is_anacrusis_start() == true );
        CHECK( is_equal_time( pColStaffObjs->anacrusis_missing_time(), 128.0f) == true );

                   // (clef G)
        ++it;       //(time 2 4)
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n c4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(barline)
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->line() == 0 );
        CHECK( is_equal_time((*it)->time(), 64.0f) );
        ++it;       //(n d4 e.)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n d4 s)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

#if LOMSE_COMPATIBILITY_LDP_1_5
    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsAddAnchor)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content (score (vers 1.5)"
                        "(instrument (musicData (clef G)(key C)"
                        "(n f4 q)(text \"Hello world\")(barline)))  )))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();
//        cout << test_name() << endl;
        //cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 5 );
        ++it;   //(key C)
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        ++it;   //(n f4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        ++it;
        //CHECK( (*it)->to_string() == "(text \"Hello world\")" );
        CHECK( (*it)->imo_object()->is_spacer() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 64.0f) );
        CHECK( (*it)->line() == 0 );
        CHECK( (*it)->staff() == 0 );
        ImoDirection* pAnchor = dynamic_cast<ImoDirection*>( (*it)->imo_object() );
        CHECK( pAnchor != nullptr );
        ++it;
        //CHECK( (*it)->to_string() == "(barline )" );
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 64.0f) );
        CHECK( (*it)->line() == 0 );
        CHECK( (*it)->staff() == 0 );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }
#endif

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ChordAcrossTwoStaves)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key C)(time 2 4)(chord (n c3 w p2)(n g3 w p2)"
            "(n e4 w p1)(n c5 w p1))(barline)) )) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 11 );
        CHECK( pColStaffObjs->is_anacrusis_start() == false );
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();
        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(clef F4 p2)
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(key C)
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(key C)
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(time 2 4)
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(time 2 4)
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(n c3 w p2)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(n g3 w p2)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(n e4 w p1)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n c5 w p1)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(barline)
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->line() == 0 );
        CHECK( is_equal_time((*it)->time(), 256.0f) );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

//Additional test for ColStaffObjsIterator -------------------------------------

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, CSOIteratorAtEnd)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (staves 2)"
            "(musicData (clef G p1)(clef F4 p2)(key D)(n c4 q v2 p1)(n d4 e.)"
            "(n d4 s v3 p2)(n e4 h)))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();
//        cout << test_name() << endl;
        //cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 8 );
        //CHECK( (*it)->to_string() == "(clef G p1)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(clef F4 p2)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->line() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(key D)" );
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(key D)" );
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->line() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n c4 q v2 p1)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 s v3 p2)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n e4 h)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );
        ++it;
        CHECK( it == pColStaffObjs->end() );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjs_Chord)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(clef G)(chord (n c4 q)(n e4 q)(n g4 q))"
            "))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();

//        cout << test_name() << endl;
        //cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 4 );

       // (clef G)
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;

       // (n c4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;

       // (n e4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;

       // (n g4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjs_NoMusicData)
    {
        Document doc(m_libraryScope);
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLdpFactory);
        stringstream expected;
        expected << "Line 0. instrument: missing mandatory element 'musicData'." << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) (content (score "
            "(vers 1.6) (instrument )) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pColStaffObjs != nullptr );
        CHECK( pColStaffObjs->num_entries() == 0 );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsTimeInSequenceWhenDecimals)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (staves 2)(musicData "
            "(n a3 w p1)(goBack start)"
            "(n f2 w p2)(barline)"
            "(n a3 q p1)"
            "(n a3 e g+ t3)(n c4 e)(n e4 e g- t-)"
            "(n a3 h)(goBack start)"
            "(n a2 h p2)"
            "(n f2 h p2)(barline)"
            ")) )))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();

//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 11 );
        CHECK( pColStaffObjs->num_lines() == 2 );

        //it        (n a3 w p1)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n f2 w p2)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 1 );
        ++it;       //(barline)
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 256.0f) );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n a3 q p1)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 256.0f) );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n a2 h p2)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 256.0f) );
        CHECK( (*it)->line() == 1 );
        ++it;       //(n a3 e g+ t3)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 320.0f) );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n c4 e)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 341.33f) );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n e4 e g- t-)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 362.66f) );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n a3 h)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 384.0f) );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n f2 h p2)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 384.0f) );
        CHECK( (*it)->line() == 1 );
        ++it;       //(barline)
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 512.0f) );
        CHECK( (*it)->line() == 0 );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsLineAssigned)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(n c4 q v1)(n d4 e.)(n d4 s v3)(n e4 h)))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();
//        cout << test_name() << endl;
        //cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 4 );
        CHECK( pColStaffObjs->num_lines() == 2 );

        //CHECK( (*it)->to_string() == "(n c4 q v1)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 s v3)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n e4 h)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsAssigLineToClef)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(clef G)(n c4 q v2)(n d4 e.)(n d4 s v3)(n e4 h)))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();
//        cout << test_name() << endl;
        //cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 5 );
        CHECK( pColStaffObjs->num_lines() == 2 );

        //CHECK( (*it)->to_string() == "(clef G)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n c4 q v2)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 s v3)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n e4 h)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsAssigLineToKey)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key D)(n c4 q v2 p1)(n d4 e.)"
            "(n d4 s v3 p2)(n e4 h)))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();
//        cout << test_name() << endl;
        //cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 8 );
        CHECK( pColStaffObjs->num_lines() == 2 );

        //CHECK( (*it)->to_string() == "(clef G p1)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(clef F4 p2)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->line() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(key D)" );
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(key D)" );
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->line() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n c4 q v2 p1)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d4 s v3 p2)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n e4 h)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, ColStaffObjsAssigLineToTime)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key D)(time 2 4)(n c4 q v2 p1)"
            "(n d4 e.)(n d4 s v3 p2)(n e4 h)))) ))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjsIterator it = pColStaffObjs->begin();
//        cout << test_name() << endl;
        //cout << pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 10 );
                   // (clef G p1)
        ++it;       //(clef F4 p2)
        ++it;       //(key D)
        ++it;       //(key D)
        ++it;       //(time 2 4)
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(time 2 4)
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(n c4 q v2 p1)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );

        delete tree->get_root();
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, builder_12_v16)
    {
        //barlines properly ordered and with right time
        ImoScore* pScore = create_score(
            "(score (vers 1.6)"
            "(instrument#100 (staves 2)"
            "(musicData (clef G p1)(clef F4 p2)"
            "(key D)(time 2 4)(n f4 h p1 v1)(goBack h)(n c3 e g+ p2 v2)"
            "(n c3 e g-)(n d3 q)(barline)))"
            "(instrument (staves 2)"
            "(musicData (clef G p1)(clef F4 p2)"
            "(key D)(time 2 4)(n f4 q. p1 v1)(clef F4 p1)(n a3 e)"
            "(goBack h)(n c3 q p2 v2)(n c3 e)(clef G p2)(clef F4 p2)"
            "(n c3 e)(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(pScore);

        CHECK( pTable->num_lines() == 4 );
        CHECK( pTable->num_entries() == 26 );
        CHECK( pTable->is_anacrusis_start() == false );

//        cout << test_name() << endl;
//        cout << pTable->dump();

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,     0,	    0,	0,	    0,	"(clef G p1)" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	0,	    1,	"(clef F4 p2)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(key D)" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	0,	    1,	"(key D)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(time 2 4)" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	0,	    1,	"(time 2 4)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    2,	"(clef G p1)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	0,	    3,	"(clef F4 p2)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    2,	"(key D)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	0,	    3,	"(key D)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    2,	"(time 2 4)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	0,	    3,	"(time 2 4)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(n f4 h v1 p1)" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	0,	    1,	"(n c3 e v2 p2 (beam 110 +))" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	0,	    2,	"(n f4 q. v1 p1)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	0,	    3,	"(n c3 q v2 p2)" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	32,	    1,	"(n c3 e v2 p2 (beam 110 -))" );
        CHECK_ENTRY0(it, 0,	    1,	    0,	64,	    1,	"(n d3 q v2 p2)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	64,	    3,	"(n c3 e v2 p2)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	96,	    2,	"(clef F4 p1)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	96,	    3,	"(clef G p2)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	96,	    3,	"(clef F4 p2)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	96,	    2,	"(n a3 e v1 p1)" );
        CHECK_ENTRY0(it, 1,	    1,	    0,	96,	    3,	"(n c3 e v2 p2)" );
        CHECK_ENTRY0(it, 0,	    0,	    0,	128,	0,	"(barline simple)" );
        CHECK_ENTRY0(it, 1,	    0,	    0,	128,	2,	"(barline simple)" );
        CHECK( pTable->min_note_duration() == 32.0 );
    }

}



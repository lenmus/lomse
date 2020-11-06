//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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
#include "private/lomse_document_p.h"
#include "lomse_score_iterator.h"
#include "lomse_model_builder.h"
#include "lomse_time.h"
#include "lomse_xml_parser.h"
#include "lomse_mxl_analyser.h"

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
    ImoScore* m_pScore;
    LdpFactory* m_pLdpFactory;
    std::string m_scores_path;

    ColStaffObjsBuilderTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pDoc(nullptr)
        , m_pScore(nullptr)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
        m_pLdpFactory = m_libraryScope.ldp_factory();
    }

    ~ColStaffObjsBuilderTestFixture()    //TearDown fixture
    {
        delete m_pScore;
        delete m_pDoc;
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    void create_score(const string &ldp, ostream& reporter=cout)
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        LdpParser parser(reporter, m_libraryScope.ldp_factory());
        parser.parse_text(ldp);
        LdpTree* pTree = parser.get_ldp_tree();
        LdpAnalyser a(reporter, m_libraryScope, m_pDoc);
        ImoObj* pImo = a.analyse_tree_and_get_object(pTree);
        delete pTree->get_root();
        m_pScore = dynamic_cast<ImoScore*>(pImo);
    }

    void check_entry(int iLine, ColStaffObjsEntry* pEntry, int type, int line,
                        TimeUnits timepos, TimeUnits duration)
    {
        bool fTypeOK = (pEntry->imo_object()->get_obj_type() == type);
        bool fLineOK = (pEntry->line() == line);
        bool fTimePosOK = is_equal_time(pEntry->time(), timepos);
        bool fDurationOK = is_equal_time(pEntry->duration(), duration);
        CHECK( fTypeOK );
        CHECK( fLineOK );
        CHECK( fTimePosOK );
        CHECK( fDurationOK );
        if (!(fTypeOK && fLineOK && fTimePosOK && fDurationOK))
        {
            cout << test_name() << " (line " << iLine << ") " << endl;
            cout << "    imo=" << pEntry->imo_object()->get_name()
                 << ", line=" << pEntry->line() << ", timepos=" << pEntry->time()
                 << ", duration=" << pEntry->duration() << endl;
        }
    }

    void check_note(int iLine, ColStaffObjsEntry* pEntry, int type, int line,
                    TimeUnits playtime, TimeUnits playdur)
    {
        bool fTypeOK = (pEntry->imo_object()->get_obj_type() == type);
        bool fLineOK = (pEntry->line() == line);
        ImoNote* pNote = static_cast<ImoNote*>(pEntry->imo_object());
        bool fPlayTimeOK = is_equal_time(pNote->get_playback_time(), playtime);
        bool fDurationOK = is_equal_time(pNote->get_playback_duration(), playdur);
        CHECK( fTypeOK );
        CHECK( fLineOK );
        CHECK( fPlayTimeOK );
        CHECK( fDurationOK );
        if (!(fTypeOK && fLineOK && fPlayTimeOK && fDurationOK))
        {
            cout << test_name() << " (line " << iLine << ") " << endl;
            cout << "    imo=" << pEntry->imo_object()->get_name() << " expected: "
                 << playtime << "  " << playdur << "  " << line << "  "
                 << pNote->to_string_with_ids() << endl;
        }
    }

};


SUITE(ColStaffObjsBuilderTest)
{

    // Compare function for sorting entries in ColStaffObjs -----------------------------

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_01)
    {
        //@01. R1-2 & R999. Single line: By timepos in arrival order
        create_score(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 q)(n e4 q)(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

//        cout << test_name() << endl;
//        cout << pTable->dump();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 4 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_quarter)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n e4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_02)
    {
        //@02. R2. Two lines: By timepos. Non-timed before timed
        create_score(
            "(score (vers 2.0)"
            "(instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "))"
            "(instrument (musicData "
            "(clef G)(time 2 4)(n c4 h)(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

//        cout << test_name() << endl;
//        cout << pTable->dump();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 9 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_quarter)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time 2 4)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(clef G p1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(time 2 4)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(n c4 h v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n e4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(barline simple)" );
        CHECK_ENTRY0(it, 1,    0,      0, 128,     1, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_03)
    {
        //@03. Multi-metric: Barlines must precede all existing objects at same timepos
        create_score(
            "(score (vers 2.0)"
            "(instrument (musicData "
            "(clef G)(time 4 4)(n c4 q)(n e4 q)(n g4 q)(n c5 q)(barline)"
            "))"
            "(instrument (musicData "
            "(clef G)(time 2 4)(n c4 h)(barline)(n e4 h)(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

//        cout << test_name() << endl;
//        cout << pTable->dump();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 13 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_quarter)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time 4 4)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(clef G p1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(time 2 4)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(n c4 h v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n e4 q v1 p1)" );
        CHECK_ENTRY0(it, 1,    0,      0, 128,     1, "(barline simple)" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(n g4 q v1 p1)" );
        CHECK_ENTRY0(it, 1,    0,      1, 128,     1, "(n e4 h v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0, 192,     0, "(n c5 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0, 256,     0, "(barline simple)" );
        CHECK_ENTRY0(it, 1,    0,      1, 256,     1, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_04)
    {
        //@04. R3. <direction>, <sound> and <transpose> can not go between clefs/key/time
        create_score(
            "(score (vers 2.0)"
            "(instrument (musicData "
            "(clef G)(dir (metronome e 40))(n e5 q)"
            "))"
            "(instrument (musicData "
            "(clef F4)(r q)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

//        cout << test_name() << endl;
//        cout << pTable->dump();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 5 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_quarter)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(clef F4 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(dir 0 p1 (metronome e 40))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e5 q v1 p1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(r q v1 p1)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_05)
    {
        //@05. R4. <direction> and <sound> can not go between clefs/key/time
        stringstream errormsg;
        create_score(
            "(score (vers 2.0)"
            "(instrument (musicData"
            "(clef G)(r q)(barline)"
            "(dir (dyn 'p'))(n a4 q)(barline) ))"
            "(instrument (musicData"
            "(clef G)(r q)(barline)"
            "(dir (dyn 'p'))(n e4 q)(barline)"
            ")))"
        , errormsg);
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

//        cout << test_name() << endl;
//        cout << pTable->dump();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 12 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_quarter)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0, 	0,	  0,	0,    0,	"(clef G p1)" );
        CHECK_ENTRY0(it, 1,	    0,	  0,	0,    1,	"(clef G p1)" );
        CHECK_ENTRY0(it, 0,	    0,	  0,	0,	  0,	"(r q v1 p1)" );
        CHECK_ENTRY0(it, 1,	    0,	  0,	0,	  1,	"(r q v1 p1)" );
        CHECK_ENTRY0(it, 0,	    0,	  0,	64,	  0,	"(barline simple)" );
        CHECK_ENTRY0(it, 1,	    0,	  0,	64,	  1,	"(barline simple)" );
        CHECK_ENTRY0(it, 0,	    0,	  1,	64,	  0,	"(dir unknown)" );
        CHECK_ENTRY0(it, 1,	    0,	  1,	64,	  1,	"(dir unknown)" );
        CHECK_ENTRY0(it, 0,	    0,	  1,	64,	  0,	"(n a4 q v1 p1)" );
        CHECK_ENTRY0(it, 1,	    0,	  1,	64,	  1,	"(n e4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,	    0,	  1,	128,  0,	"(barline simple)" );
        CHECK_ENTRY0(it, 1,	    0,	  1,	128,  1,	"(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_06)
    {
        //@06. R5. Non-timed must go before barlines with equal measure number
        create_score(
            "(score (vers 2.0)"
            "(instrument (musicData "
            "(clef G)(time 2 4)"
            "(n d5 h)(barline) ))"
            "(instrument (musicData "
            "(clef F4)(time 2 4) "
            "(r h)(clef G)(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

//        cout << test_name() << endl;
//        cout << pTable->dump();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 9 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_half)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time 2 4)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(clef F4 p1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(time 2 4)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n d5 h v1 p1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(r h v1 p1)" );
        CHECK_ENTRY0(it, 1,    0,      0, 128,     1, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(barline simple)" );
        CHECK_ENTRY0(it, 1,    0,      0, 128,     1, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_07)
    {
        //@07. R6. Graces in the same timepos must go before note/rest in that timepos

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/222-graces-two-voices.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();

//        cout << test_name() << endl;
//        cout << pTable->dump();

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_half)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(grace e5 e v1 p1 (stem up))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(grace b4 e v2 p1 (stem down))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e5 h v1 p1 (stem up))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c5 h v2 p1 (stem down))" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_08)
    {
        //@08. R7. Graces in the same timepos go ordered by align timepos

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/224-beamed-graces-two-voices.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();

//        cout << test_name() << endl;
//        cout << pTable->dump();

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 8 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_half)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(grace e5 s v1 p1 (stem up)(beam 41 ++))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(grace a4 s v2 p1 (stem down)(beam 51 ++))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(grace d5 s v1 p1 (stem up)(beam 41 --))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(grace g4 s v2 p1 (stem down)(beam 51 --))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c5 h v1 p1 (stem up))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n f4 h v2 p1 (stem down))" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_09)
    {
        //@09. R8. After graces must go before the barline in the same timepos

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/218-after-graces.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();

//        cout << test_name() << endl;
//        cout << pTable->dump();

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_quarter)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n +f4 q v1 p1 (stem up))" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(grace +e4 s v1 p1 (stem up)(beam 44 ++))" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(grace +f4 s v1 p1 (stem up)(beam 44 ==))" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(grace +e4 s v1 p1 (stem up)(beam 44 --))" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_10)
    {
        //@10. R8. Graces must go before barlines in the same timepos (two parts)

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/227-grace-notes-two-parts-alignment.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();

//        cout << test_name() << endl;
//        cout << pTable->dump();

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 13 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_eighth)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n a4 e v1 p1 (stem up))" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(n d4 e v1 p1 (stem up))" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(barline simple)" );
        CHECK_ENTRY0(it, 1,    0,      0,  32,     1, "(barline simple)" );
        CHECK_ENTRY0(it, 0,    0,      1,  32,     0, "(grace b4 s v1 p1 (stem up)(beam 58 ++))" );
        CHECK_ENTRY0(it, 0,    0,      1,  32,     0, "(grace +c5 s v1 p1 (stem up)(beam 58 ==))" );
        CHECK_ENTRY0(it, 0,    0,      1,  32,     0, "(grace +d5 s v1 p1 (stem up)(beam 58 --))" );
        CHECK_ENTRY0(it, 0,    0,      1,  32,     0, "(n e5 q v1 p1 (stem down))" );
        CHECK_ENTRY0(it, 1,    0,      1,  32,     1, "(n c4 q v1 p1 (stem up))" );
        CHECK_ENTRY0(it, 0,    0,      1,  96,     0, "(barline simple)" );
        CHECK_ENTRY0(it, 1,    0,      1,  96,     1, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, lower_entry_11)
    {
        //@11. R3. <transpose> can not go between clefs/key/time

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/transpose/001-transpose.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();

//        cout << test_name() << endl;
//        cout << pTable->dump();

        CHECK( pTable->num_lines() == 3 );
        CHECK( pTable->num_entries() == 17 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_quarter)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(key D)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time common)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(clef G p1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(key A)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(time common)" );
        CHECK_ENTRY0(it, 2,    0,      0,   0,     2, "(clef G p1)" );
        CHECK_ENTRY0(it, 2,    0,      0,   0,     2, "(key C)" );
        CHECK_ENTRY0(it, 2,    0,      0,   0,     2, "(time common)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(transpose -1 -2 -1)" );
        CHECK_ENTRY0(it, 1,    0,      0,   0,     1, "(transpose -1 -9 -5)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n d4 q v1 p1)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_200)
    {
        //@200. One grace. From previous 10%

        Document doc(m_libraryScope);
        doc.from_string(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace slash=\"yes\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>4</duration><voice>1</voice><type>half</type></note>"
            "</measure></part></score-partwise>"
            , Document::k_format_mxl
        );
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 5 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(n g4 q v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 51.2);    //previous dur 80%
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 51.2, 12.8);     //grace dur 20%, the time stolen
        ++it;       //(n c5 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 64.0, 128.0);   //ppal. unmodified
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_201)
    {
        //@201. One grace. From next 30%

        Document doc(m_libraryScope);
        doc.from_string(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace steal-time-following=\"30\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>4</duration><voice>1</voice><type>half</type></note>"
            "</measure></part></score-partwise>"
            , Document::k_format_mxl
        );
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 5 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(n g4 q v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 64.0);    //prev. dur 100%
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 64.0, 38.4);     //grace dur 30% of next
        ++it;       //(n c5 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 102.4, 89.6);   //ppal. dur 70%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_202)
    {
        //@202. Two graces. From previous 20%

        Document doc(m_libraryScope);
        doc.from_string(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace steal-time-previous=\"20\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><grace slash=\"no\"/><pitch><step>B</step><octave>4</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>4</duration><voice>1</voice><type>half</type></note>"
            "</measure></part></score-partwise>"
            , Document::k_format_mxl
        );
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 6 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(n g4 q v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 51.2);    //prev. dur 80%
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 51.2, 6.4);     //graces dur 20% of prev
        ++it;       //(grace b4 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 57.6, 6.4);
        ++it;       //(n c5 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 64.0, 128.0);   //ppal. dur 100%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_203)
    {
        //@203. Two graces. From next 40%

        Document doc(m_libraryScope);
        doc.from_string(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace steal-time-following=\"40\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><grace slash=\"no\"/><pitch><step>B</step><octave>4</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>4</duration><voice>1</voice><type>half</type></note>"
            "</measure></part></score-partwise>"
            , Document::k_format_mxl
        );
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 6 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(n g4 q v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 64.0);    //prev. dur 100%
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 64.0, 25.6);     //graces dur 40% of next
        ++it;       //(grace b4 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 89.6, 25.6);
        ++it;       //(n c5 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 115.2, 76.8);   //ppal. dur 60%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_204)
    {
        //@204. graces steal from prev chord. All chord notes play duration shortened

        Document doc(m_libraryScope);
        doc.from_string(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>4</duration><voice>1</voice><type>half</type></note>"
            "<note><chord/><pitch><step>B</step><octave>4</octave></pitch>"
                "<duration>4</duration><voice>1</voice><type>half</type></note>"
            "<note><grace steal-time-previous=\"10\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><grace slash=\"no\"/><pitch><step>B</step><octave>4</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>4</duration><voice>1</voice><type>half</type></note>"
            "</measure></part></score-partwise>"
            , Document::k_format_mxl
        );
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 7 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(chord (n g4 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 115.2);    //prev. dur 90%
        ++it;       //(n b4 h v1 p1))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 115.2);    //prev. dur 90%
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 115.2, 6.4);     //graces total dur 10%
        ++it;       //(grace b4 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 121.6, 6.4);
        ++it;       //(n c5 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 128.0, 128.0);   //ppal. dur 100%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_205)
    {
        //@205. graces steal from next chord. All chord notes start play time and
	    //      duration are modified

        Document doc(m_libraryScope);
        doc.from_string(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace steal-time-following=\"40\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><grace slash=\"no\"/><pitch><step>B</step><octave>4</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>4</duration><voice>1</voice><type>half</type></note>"
            "<note><chord/><pitch><step>E</step><octave>5</octave></pitch>"
                "<duration>4</duration><voice>1</voice><type>half</type></note>"
            "</measure></part></score-partwise>"
            , Document::k_format_mxl
        );
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 7 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(n g4 q v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 64.0);    //prev. dur 100%
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 64.0, 25.6);     //graces dur 40% of next
        ++it;       //(grace b4 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 89.6, 25.6);
        ++it;       //(chord (n c5 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 115.2, 76.8);   //ppal. dur 60%
        ++it;       //(n e5 h v1 p1))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 115.2, 76.8);   //ppal. dur 60%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_206)
    {
        //@206. graces steal from prev chord. Additional notes in second voice in
	    //      chord timepos are not affected

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/206-chord-second-voice-and graces.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 8 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(chord (n g4 h v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 115.2);    //prev. dur 90%
        ++it;       //(n b4 h v1 p1 (stem up)))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 115.2);    //prev. dur 90%
        ++it;       //(n d4 h v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 0.0, 128.0);    //other voice. dur 100%
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 115.2, 6.4);     //graces total dur 10%
        ++it;       //(grace b4 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 121.6, 6.4);
        ++it;       //(n c5 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 128.0, 128.0);   //ppal. dur 100%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_207)
    {
        //@205. graces steal from next chord. All chord notes start play time and
	    //      duration are modified

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/207-graces-and-chord-second-voice.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 8 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(n g4 q v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 64.0);    //prev. dur 100%
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 64.0, 25.6);     //graces dur 40% of next
        ++it;       //(grace b4 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 89.6, 25.6);
        ++it;       //(chord (n c5 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 115.2, 76.8);   //ppal. dur 60%
        ++it;       //(n e5 h v1 p1))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 115.2, 76.8);   //ppal. dur 60%
        ++it;       //(n e5 h v1 p1))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 64.0, 128.0);   //second voice. dur 100%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_208)
    {
        //@208. graces at end. From previous 25%

        Document doc(m_libraryScope);
        doc.from_string(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace steal-time-previous=\"25\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><grace slash=\"no\"/><pitch><step>B</step><octave>4</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "</measure></part></score-partwise>"
            , Document::k_format_mxl
        );
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 5 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(n g4 q v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 48.0);    //prev. dur 75%
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 48.0, 8.0);     //graces dur 25% of prev
        ++it;       //(grace b4 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 56.0, 8.0);
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_209)
    {
        //@209. graces at end. From next 10% but no next. Use quarter note

        Document doc(m_libraryScope);
        doc.from_string(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>4</duration><voice>1</voice><type>half</type></note>"
            "<note><grace steal-time-following=\"10\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><grace slash=\"no\"/><pitch><step>B</step><octave>4</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "</measure></part></score-partwise>"
            , Document::k_format_mxl
        );
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 5 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(n g4 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 128.0);    //prev. dur 100%
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 128.0, 3.2);     //graces total dur 10% of quarter
        ++it;       //(grace b4 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 131.2, 3.2);
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_210)
    {
        //@210. steal from previous but does not exist. Use quarter note

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/210-grace-note.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 4 );
        CHECK( is_equal_time(pColStaffObjs->anacruxis_missing_time(), 51.2) );
        CHECK( pColStaffObjs->is_anacruxis_start() == true );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 0.0, 12.8);     //grace dur 20% of quarter
        ++it;       //(n c5 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 12.8, 128.0);   //ppal. dur 100%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_211)
    {
        //@211. playback time for graces in chord properly computed

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/211-graces-chord.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 5 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(chord (grace#37 d5 e v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_grace, 0, 0.0, 12.8);  //grace dur 20% of quarter
        ++it;       //(grace#40 f5 e v1 p1 (stem up)))
        check_note(__LINE__, *it, k_imo_note_grace, 0, 0.0, 12.8);  //grace dur 20% of quarter
        ++it;       //(n#43 c5 q v1 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 12.8, 51.2);   //ppal. dur 80%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_212)
    {
        //@212. playback time for graces in two voices

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/212-beamed-graces-two-voices.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 8 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(grace#37 e5 s v1 p1 (stem up)(beam#41 41 ++))
        check_note(__LINE__, *it, k_imo_note_grace, 0, 0.0, 12.8);  //graces total dur 20% of half
        ++it;       //(grace#48 a4 s v2 p1 (stem down)(beam#52 52 ++))
        check_note(__LINE__, *it, k_imo_note_grace, 1, 0.0, 12.8);
        ++it;       //(grace#40 d5 s v1 p1 (stem up)(beam#41 41 --))
        check_note(__LINE__, *it, k_imo_note_grace, 0, 12.8, 12.8);  //graces total dur 20% of half
        ++it;       //(grace#51 g4 s v2 p1 (stem down)(beam#52 52 --))
        check_note(__LINE__, *it, k_imo_note_grace, 1, 12.8, 12.8);
        ++it;       //(n#45 c5 h v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 25.6, 102.4);   //ppal. dur 80%
        ++it;       //(n#56 f4 h v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 25.6, 102.4);   //ppal. dur 80%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_213)
    {
        //@213. locate previous note when graces in two voices

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/213-grace-previous-note.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 11 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(n#37 c5 q v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 51.2);  //prev. dur 80%
        ++it;       //(n#39 f4 q v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 0.0, 51.2);  //prev. dur 80%
        ++it;       //(barline#40 simple)
        ++it;       //(grace#41 g5 e v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_grace, 0, 51.2, 12.8);  //grace dur 20%
        ++it;       //(grace#48 f4 e v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_grace, 1, 51.2, 12.8);  //grace dur 20%
        ++it;       //(n#44 e5 h v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 64.0, 128.0);  //normal. dur 100%
        ++it;       //(n#51 a4 h v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 64.0, 128.0);  //normal. dur 100%
        ++it;       //(n#46 e5 q v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 192.0, 64.0);  //normal. dur 100%
        ++it;       //(n#53 a4 q v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 192.0, 64.0);  //normal. dur 100%
   }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_214)
    {
        //@214. graces in second voice that steal prev, but no prev in second voice:
        //      playback time shifted back when not at start of score

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/214-grace-chord-two-voices-previos-note.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 12 );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                   // (clef G p1)
        ++it;       //(n#37 c5 q v1 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 51.2);  //prev. dur 80%
        ++it;       //(barline#38 simple)
        ++it;       //(chord (grace#39 d5 e v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_grace, 0, 51.2, 12.8);  //grace dur 20%
        ++it;       //(grace#42 f5 e v1 p1 (stem up)))
        check_note(__LINE__, *it, k_imo_note_grace, 0, 51.2, 12.8);  //grace dur 20%
        ++it;       //(chord (grace#48 b4 e v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_grace, 1, 51.2, 12.8);  //grace dur 20%
        ++it;       //(grace#51 g4 e v2 p1 (stem down)))
        check_note(__LINE__, *it, k_imo_note_grace, 1, 51.2, 12.8);  //grace dur 20%
        ++it;       //(n#45 e5 h v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 64.0, 128.0);  //normal. dur 100%
        ++it;       //(n#54 a4 h v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 64.0, 128.0);  //normal. dur 100%
        ++it;       //(n#46 e5 q v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 192.0, 64.0);  //normal. dur 100%
        ++it;       //(n#55 a4 q v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 192.0, 64.0);  //normal. dur 100%
   }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_215)
    {
        //@215. graces at start that steal from previous create anacruxis start. No TS

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/215-graces-anacruxis-start.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 11 );
        CHECK( is_equal_time(pColStaffObjs->anacruxis_missing_time(), 32.0) );
        CHECK( is_equal_time(pColStaffObjs->anacruxis_extra_time(), 32.0) );
        CHECK( pColStaffObjs->is_anacruxis_start() == true );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                    //(clef#36 G p1)
        ++it;       //(clef#37 F4 p2)
        ++it;       //(grace#40 a2 t v3 p2 (stem up)(slur#52 1 start)(beam#46 46 +++))
        check_note(__LINE__, *it, k_imo_note_grace, 1, 0.0, 10.67);  //graces total dur 50% of quarter
        ++it;       //(grace#43 +c3 t v3 p2 (stem up)(beam#46 46 ===))
        check_note(__LINE__, *it, k_imo_note_grace, 1, 10.67, 10.67);
        ++it;       //(grace#45 e3 t v3 p2 (stem up)(beam#46 46 ---))
        check_note(__LINE__, *it, k_imo_note_grace, 1, 21.33, 10.67);
        ++it;       //(n#38 a5 h v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 32.0, 128.0);  //dur 100%
        ++it;       //(n#51 a3 e v3 p2 (stem down)(slur#52 1 stop)(beam#59 59 +))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 32.0, 32.0);  //dur 100%
        ++it;       //(n#56 a3 e v3 p2 (stem down)(beam#59 59 =))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 64.0, 32.0);  //dur 100%
        ++it;       //(n#57 a3 e v3 p2 (stem down)(beam#59 59 =))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 96.0, 32.0);  //dur 100%
        ++it;       //(n#58 a3 e v3 p2 (stem down)(beam#59 59 -))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 128.0, 32.0);  //dur 100%
   }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_216)
    {
        //@216. graces anacruxis time when time signature

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/216-grace-anacruxis-time-signature.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 5 );
        CHECK( is_equal_time(pColStaffObjs->anacruxis_missing_time(), 115.2) );
        CHECK( is_equal_time(pColStaffObjs->anacruxis_extra_time(), 12.8) );
        CHECK( pColStaffObjs->is_anacruxis_start() == true );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                    //(clef G p1)
        ++it;       //(time 2 4)
        ++it;       //(grace d5 e v1 p1)
        check_note(__LINE__, *it, k_imo_note_grace, 0, 0.0, 12.8);     //grace dur 20% of quarter
        ++it;       //(n c5 h v1 p1)
        check_note(__LINE__, *it, k_imo_note_regular, 0, 12.8, 128.0);   //ppal. dur 100%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_217)
    {
        //@217. graces anacruxis greater than notated anacruxis. Anacruxis extra time

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/217-graces-anacruxis-greater-than-notated.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 8 );
        CHECK( is_equal_time(pColStaffObjs->anacruxis_missing_time(), 102.4) );
        CHECK( is_equal_time(pColStaffObjs->anacruxis_extra_time(), 9.6) );
        CHECK( pColStaffObjs->is_anacruxis_start() == true );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                    //(clef G p1)
        ++it;       //(time 2 4)
        ++it;       //(n#38 d5 s v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 9.6, 16.0);   //anacrux. dur 100%
        ++it;       //(barline#39 simple)
        ++it;       //(grace#42 f4 e v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_grace, 1, 0, 25.6);     //grace dur 40% of quarter
        ++it;       //(n#40 c5 h v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 25.6, 128.0);   //ppal. dur 100%
        ++it;       //(n#45 g4 h v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 25.6, 128.0);   //ppal. dur 100%
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_218)
    {
        //@218. graces anacruxis lower than notated anacruxis. No anacruxis extra time

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/219-graces-anacruxis-lower-than-notated.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();

        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 8 );
        CHECK( is_equal_time(pColStaffObjs->anacruxis_missing_time(), 112.0) );
        CHECK( is_equal_time(pColStaffObjs->anacruxis_extra_time(), 0.0) );
        CHECK( pColStaffObjs->is_anacruxis_start() == true );

        ColStaffObjsIterator it = pColStaffObjs->begin();
                    //(clef G p1)
        ++it;       //(time 2 4)
        ++it;       //(n#38 d5 s v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 0.0, 16.0);   //anacrux. dur 100%
        ++it;       //(barline#39 simple)
        ++it;       //(grace#42 f4 e v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_grace, 1, 3.2, 12.8);     //grace dur 20% of quarter
        ++it;       //(n#40 c5 h v1 p1 (stem up))
        check_note(__LINE__, *it, k_imo_note_regular, 0, 16.0, 128.0);   //ppal. dur 100%
        ++it;       //(n#45 g4 h v2 p1 (stem down))
        check_note(__LINE__, *it, k_imo_note_regular, 1, 16.0, 128.0);   //ppal. dur 100%
    }


    // ColStaffObjsBuilderEngine2x ------------------------------------------------------

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_01)
    {
        //@01. empty score creates empty table
        create_score(
            "(score (vers 2.0)(instrument (musicData "
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 0 );
        CHECK( pTable->is_anacruxis_start() == false );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_02)
    {
        //@02. each voice uses a different time counter
        create_score(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 q v1)(n c5 e v2)(n e4 e v1)(n e5 e v2)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 5 );
        CHECK( pTable->is_anacruxis_start() == false );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_03)
    {
        //@03. barline is placed at maximum time reached in the measure
        create_score(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 q v1)(n c5 e v2)(n e4 e v1)(n e5 w v2)(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacruxis_start() == false );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_04)
    {
        //@04. after barline voices are correctly positioned
        create_score(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 q v1)(n c5 e v2)(n e4 e v1)(n e5 w v2)(barline)"
            "(n c4 q v1)(n c5 e v2)(n e4 e v1)(n e5 w v2)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 10 );
        CHECK( pTable->is_anacruxis_start() == false );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_05)
    {
        //@05. key and time signatures generate secondary entries
        create_score(
            "(score (vers 2.0) (instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacruxis_start() == false );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_06)
    {
        //@06. chord notes have time and staff properly assigned
        create_score(
            "(score (vers 2.0) (instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key C)(time 2 4)(chord (n c3 w p2)(n g3 w p2)"
            "(n e4 w p1)(n c5 w p1))(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 11 );
        CHECK( pTable->is_anacruxis_start() == false );

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
    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_07)
    {
        //@07. anchor staffobjs
        create_score(
            "(score (vers 1.5) (instrument (musicData "
            "(clef G)(key C)(n f4 q)(text \"Hello world\")(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 5 );
        CHECK( pTable->is_anacruxis_start() == false );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_08)
    {
        //@08. anacruxis detected and its duration computed
        create_score(
            "(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 3 4)(n c4 q)(barline)(n d4 e.)(n d4 s)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacruxis_start() == true );
        CHECK( is_equal_time( pTable->anacruxis_missing_time(), 128.0f) == true );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_09)
    {
        //@09. goFwd replaced by special invisible rest
        create_score(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 e v1)(goFwd e v1)(n e4 e v1)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 4 );
        CHECK( pTable->is_anacruxis_start() == false );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_10)
    {
        //@10. notes in other voices intermixed in beamed group
        create_score(
            "(score (vers 2.0)(instrument#100 (musicData "
            "(clef F4)(n e3 e g+)(n g3 e)(n c4 e g-)"
            "(n c2 w v3)(barline)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacruxis_start() == false );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_11)
    {
        //@11. intermediate non-time get assigned right time
        create_score(
            "(score (vers 2.0)(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(n c4 q p1 v1)(n e4 e v1)"
            "(n c3 e p2 v2)(clef G p2)(n c4 e v2)"
            ")))"
        );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 7 );
        CHECK( pTable->is_anacruxis_start() == false );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_12)
    {
        //@12. barlines properly ordered and with right time
        create_score(
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
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 4 );
        CHECK( pTable->num_entries() == 26 );
        CHECK( pTable->is_anacruxis_start() == false );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_13)
    {
        //@13. Direction in prolog is re-ordered after prolog

        create_score(    //unit test score 02033
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
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 12 );
        CHECK( pTable->is_anacruxis_start() == true );

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
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(n e5 s v1 p1 (beam 32 ++))" );
        CHECK( pTable->min_note_duration() == 16.0 );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine2x_14)
    {
        //@14. Direction before prolog is re-ordered

        create_score(    //unit test score 02034
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
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 12 );
        CHECK( pTable->is_anacruxis_start() == true );

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
        CHECK_ENTRY0(it, 0,	    0,	    0,	0,	    0,	"(n e5 s v1 p1 (beam 32 ++))" );
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
        CHECK( pColStaffObjs->is_anacruxis_start() == true );
        CHECK( is_equal_time( pColStaffObjs->anacruxis_missing_time(), 128.0f) == true );

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
        CHECK( pColStaffObjs->is_anacruxis_start() == false );
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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine1x_12)
    {
        //@12. barlines properly ordered and with right time
        create_score(
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
        ColStaffObjs* pTable = builder.build(m_pScore);

        CHECK( pTable->num_lines() == 4 );
        CHECK( pTable->num_entries() == 26 );
        CHECK( pTable->is_anacruxis_start() == false );

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

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine1x_13)
    {
        //@13. align timepos assigned to grace notes

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/223-beamed-graces.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();

//        cout << test_name() << endl;
//        cout << pTable->dump();

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 5 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_quarter)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );

        ImoGraceNote* pNote = static_cast<ImoGraceNote*>((*it)->imo_object());
        CHECK( is_equal_time(pNote->get_align_timepos(), 0.0 ) );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(grace d5 s v1 p1 (stem up)(beam 41 ++))" );

        pNote = static_cast<ImoGraceNote*>((*it)->imo_object());
        CHECK( is_equal_time(pNote->get_align_timepos(), 1.0 ) );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(grace c5 s v1 p1 (stem up)(beam 41 --))" );

        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n b4 q v1 p1 (stem down))" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine1x_14)
    {
        //@14. align timepos for grace notes in two voices

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/222-graces-two-voices.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();

//        cout << test_name() << endl;
//        cout << pTable->dump();

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_half)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );

        ImoGraceNote* pNote = static_cast<ImoGraceNote*>((*it)->imo_object());
        CHECK( is_equal_time(pNote->get_align_timepos(), 0.0 ) );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(grace e5 e v1 p1 (stem up))" );

        pNote = static_cast<ImoGraceNote*>((*it)->imo_object());
        CHECK( is_equal_time(pNote->get_align_timepos(), 0.0 ) );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(grace b4 e v2 p1 (stem down))" );

        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e5 h v1 p1 (stem up))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c5 h v2 p1 (stem down))" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine1x_15)
    {
        //@15. align timepos for beamed grace notes in two voices

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/224-beamed-graces-two-voices.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();

//        cout << test_name() << endl;
//        cout << pTable->dump();

        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 8 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_half)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );

        ImoGraceNote* pNote = static_cast<ImoGraceNote*>((*it)->imo_object());
        CHECK( is_equal_time(pNote->get_align_timepos(), 0.0 ) );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(grace e5 s v1 p1 (stem up)(beam 41 ++))" );

        pNote = static_cast<ImoGraceNote*>((*it)->imo_object());
        CHECK( is_equal_time(pNote->get_align_timepos(), 0.0 ) );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(grace a4 s v2 p1 (stem down)(beam 51 ++))" );

        pNote = static_cast<ImoGraceNote*>((*it)->imo_object());
        CHECK( is_equal_time(pNote->get_align_timepos(), 1.0 ) );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(grace d5 s v1 p1 (stem up)(beam 41 --))" );

        pNote = static_cast<ImoGraceNote*>((*it)->imo_object());
        CHECK( is_equal_time(pNote->get_align_timepos(), 1.0 ) );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(grace g4 s v2 p1 (stem down)(beam 51 --))" );

        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c5 h v1 p1 (stem up))" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n f4 h v2 p1 (stem down))" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(barline simple)" );
    }

    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, engine1x_16)
    {
        //@16. align timepos for grace notes in chord

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/225-graces-chord.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();

//        cout << test_name() << endl;
//        cout << pTable->dump();

        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 5 );
        CHECK( is_equal_time(pTable->min_note_duration(), TimeUnits(k_duration_quarter)));

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );

        ImoGraceNote* pNote = static_cast<ImoGraceNote*>((*it)->imo_object());
        CHECK( is_equal_time(pNote->get_align_timepos(), 0.0 ) );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(chord (grace d5 e v1 p1 (stem up))" );

        pNote = static_cast<ImoGraceNote*>((*it)->imo_object());
        CHECK( is_equal_time(pNote->get_align_timepos(), 0.0 ) );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(grace f5 e v1 p1 (stem up)))" );

        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c5 q v1 p1 (stem down))" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(barline simple)" );
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

//    TEST_FIXTURE(ColStaffObjsBuilderTestFixture, playback_time_100)
//    {
//        //@100. auxiliary, for checking the ColStaffObjs
//        stringstream errormsg;
//        stringstream expected;
//        Document doc(m_libraryScope);
//        //002-beamed-graces.xml
//        doc.from_string(
//            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
//            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
//            "<clef><sign>G</sign><line>2</line></clef></attributes>"
//            "<note><grace/><pitch><step>D</step><octave>5</octave></pitch>"
//                "<voice>1</voice><type>16th</type><stem>up</stem><beam number=\"1\">begin</beam></note>"
//            "<note><grace/><pitch><step>C</step><octave>5</octave></pitch>"
//                "<voice>1</voice><type>16th</type><stem>up</stem><beam number=\"1\">end</beam></note>"
//            "<note><pitch><step>B</step><octave>4</octave></pitch>"
//                "<duration>2</duration><voice>1</voice><type>quarter</type><stem>down</stem></note>"
//            "</measure></part></score-partwise>"
//            , Document::k_format_mxl
//        );
//        CHECK( errormsg.str() == expected.str() );
//        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
//        CHECK( pScore != nullptr );
//        ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
//        cout << test_name() << endl;
//        cout << pColStaffObjs->dump();
////
////        CHECK( pColStaffObjs->num_lines() == 1 );
////        CHECK( pColStaffObjs->num_entries() == 4 );
////
////        ColStaffObjsIterator it = pColStaffObjs->begin();
////                   // (clef G p1)
////        ++it;       //(grace d5 e v1 p1)
////        check_entry(__LINE__, *it, k_imo_note_grace, 0, 0, 12.8);     //grace dur 20% of quarter
////        ++it;       //(n c5 h v1 p1)
////        check_entry(__LINE__, *it, k_imo_note_regular, 0, 12.8, 128.0);   //ppal. dur 100%
////        ++it;       //(barline simple)
////        check_entry(__LINE__, *it, k_imo_barline, 0, 140.8, 0.0);
//    }

}



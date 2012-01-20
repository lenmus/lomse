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
#include "lomse_staffobjs_table.h"
#include "lomse_parser.h"
#include "lomse_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_document.h"
#include "lomse_score_iterator.h"
#include "lomse_model_builder.h"
#include "lomse_time.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
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
class ColStaffObjsTestFixture
{
public:

    ColStaffObjsTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
        m_pLdpFactory = m_libraryScope.ldp_factory();
    }

    ~ColStaffObjsTestFixture()    //TearDown fixture
    {
    }

    LibraryScope m_libraryScope;
    LdpFactory* m_pLdpFactory;
    std::string m_scores_path;
};

SUITE(ColStaffObjsTest)
{

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsAddEntries)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData (n c4 q) (barline simple))))))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort

        CHECK( pColStaffObjs->num_entries() == 2 );
        CHECK( pColStaffObjs->num_lines() == 1 );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ScoreIteratorPointsFirst)
    {
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (barline simple))))))" );
        Document doc(m_libraryScope);
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();

        //(*it)->dump();
        //cout << (*it)->to_string() << endl;
        //CHECK( (*it)->to_string() == "(n c4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsChangeSegment)
    {
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData (n c4 q) (barline simple)"
            "(n d4 e) (barline simple) (n e4 w))))))" );
        Document doc(m_libraryScope);
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //(*it)->dump();
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
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsTimeInSequence)
    {
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
                "(score (vers 1.6) (instrument (musicData "
                "(n c4 q)(n d4 e.)(n d4 s)(n e4 h)))) ))" );
        Document doc(m_libraryScope);
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //pColStaffObjs->dump();
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
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsTimeGoBack)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData"
            "(n c4 q)(n d4 e.)(n d4 s)(goBack start)(n e4 h)(n g4 q)))) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort

        CHECK( pColStaffObjs->num_entries() == 5 );
        CHECK( pColStaffObjs->num_lines() == 1 );
        //pColStaffObjs->dump();

        ColStaffObjs::iterator it = pColStaffObjs->begin();
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
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n g4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 128.0f) );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, GoBack_StartTime)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData"
            "(n c4 q)(n d4 e.)(n e4 s)(barline)"
            "(n f4 q)(n g4 e.)(n a4 s)(goBack start)(n b4 q)(n c5 q)))) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort

        CHECK( pColStaffObjs->num_entries() == 9 );
        CHECK( pColStaffObjs->num_lines() == 1 );
//        pColStaffObjs->dump();

        ColStaffObjs::iterator it = pColStaffObjs->begin();
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
        ++it;
        //CHECK( (*it)->to_string() == "(n g4 e.)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 192.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(n a4 s)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 240.0f) );
        ++it;   //(n b4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 128.0f) );
        ++it;   //(n g4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 1 );
        CHECK( is_equal_time((*it)->time(), 192.0f) );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsTimeGoFwd)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(n c4 q)(n d4 e.)(n d4 s)(goBack start)(n e4 q)(goFwd end)(barline)))) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort

        CHECK( pColStaffObjs->num_entries() == 5 );
        CHECK( pColStaffObjs->num_lines() == 1 );
        //pColStaffObjs->dump();

        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //CHECK( (*it)->to_string() == "(n c4 q)" );
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
        //CHECK( (*it)->to_string() == "(n e4 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;
        //CHECK( (*it)->to_string() == "(barline )" );
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( is_equal_time((*it)->time(), 128.0f) );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsStaffAssigned)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(n c4 q p2)(n d4 e.)(n d4 s p3)(n e4 h)))) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //pColStaffObjs->dump();
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
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsLineAssigned)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(n c4 q v1)(n d4 e.)(n d4 s v3)(n e4 h)))) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //pColStaffObjs->dump();
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
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsAssigLineToClef)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(clef G)(n c4 q v2)(n d4 e.)(n d4 s v3)(n e4 h)))) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //pColStaffObjs->dump();
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
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsAssigLineToKey)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key D)(n c4 q v2 p1)(n d4 e.)"
            "(n d4 s v3 p2)(n e4 h)))) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //pColStaffObjs->dump();
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
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsAssigLineToTime)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key D)(time 2 4)(n c4 q v2 p1)"
            "(n d4 e.)(n d4 s v3 p2)(n e4 h)))) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 10 );
                    //(clef G p1)
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
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, Anacrusis)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(clef G)(time 3 4)(n c4 q)(barline)(n d4 e.)(n d4 s)) )) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_lines() == 1 );
        CHECK( pColStaffObjs->num_entries() == 6 );
        CHECK( pColStaffObjs->is_anacrusis_start() == true );
        CHECK( is_equal_time( pColStaffObjs->anacrusis_missing_time(), 128.0f) == true );

                    //(clef G)
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
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsFullExample)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
                        "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
                        "(key D)(time 2 4)(n f4 w p1)(goBack w)(n c3 e g+ p2)"
                        "(n c3 e g-)(n d3 q)(barline)))"
                        "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
                        "(key D)(time 2 4)(n f4 q. p1)(clef F4 p1)(n a3 e)"
                        "(goBack h)(n c3 q p2)(n c3 e)(clef G p2)(clef F4 p2)"
                        "(n c3 e)(barline)))  )))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_lines() == 4 );
        CHECK( pColStaffObjs->num_entries() == 26 );
        CHECK( pColStaffObjs->is_anacrusis_start() == false );
        CHECK( is_equal_time( pColStaffObjs->anacrusis_missing_time(), 0.0f) == true );
        //CHECK( (*it)->to_string() == "(clef G p1)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 0 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(key D)" );
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 0 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(time 2 4)" );
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 0 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n f4 w p1)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 0 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(clef F4 p2)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 1 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(key D)" );
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 1 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(time 2 4)" );
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 1 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n c3 e g+ p2)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 1 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(clef G p1)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 2 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(key D)" );
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 2 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(time 2 4)" );
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 2 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n f4 q. p1)" );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 2 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(clef F4 p2)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 3 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(key D)" );
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 3 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(time 2 4)" );
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 3 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n c3 q p2)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        CHECK( (*it)->line() == 3 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n c3 e g-)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 32.0f) );
        CHECK( (*it)->line() == 1 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n d3 q)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 64.0f) );
        CHECK( (*it)->line() == 1 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n c3 e)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 64.0f) );
        CHECK( (*it)->line() == 3 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(clef F4 p1)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 96.0f) );
        CHECK( (*it)->line() == 2 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(n a3 e)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 96.0f) );
        CHECK( (*it)->line() == 2 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(clef G p2)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 96.0f) );
        CHECK( (*it)->line() == 3 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(clef F4 p2)" );
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 96.0f) );
        CHECK( (*it)->line() == 3 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(n c3 e)" );
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 96.0f) );
        CHECK( (*it)->line() == 3 );
        CHECK( (*it)->staff() == 1 );
        ++it;
        //CHECK( (*it)->to_string() == "(barline )" );
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 128.0f) );
        CHECK( (*it)->line() == 0 );
        CHECK( (*it)->staff() == 0 );
        ++it;
        //CHECK( (*it)->to_string() == "(barline )" );
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->num_instrument() == 1 );
        CHECK( is_equal_time((*it)->time(), 128.0f) );
        CHECK( (*it)->line() == 2 );
        CHECK( (*it)->staff() == 0 );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsAddAnchor)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
                        "(instrument (musicData (clef G)(key C)"
                        "(n f4 q)(text \"Hello world\")(barline)))  )))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore);
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //pColStaffObjs->dump();
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
        ImoSpacer* pAnchor = dynamic_cast<ImoSpacer*>( (*it)->imo_object() );
        CHECK( pAnchor != NULL );
        ++it;
        //CHECK( (*it)->to_string() == "(barline )" );
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->num_instrument() == 0 );
        CHECK( is_equal_time((*it)->time(), 64.0f) );
        CHECK( (*it)->line() == 0 );
        CHECK( (*it)->staff() == 0 );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ChordAcrossTwoStaves)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key C)(time 2 4)(chord (n c3 w p2)(n g3 w p2)"
            "(n e4 w p1)(n c5 w p1))(barline)) )) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, true /*sort table*/);
        //pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_lines() == 2 );
        CHECK( pColStaffObjs->num_entries() == 11 );
        CHECK( pColStaffObjs->is_anacrusis_start() == false );
        //pColStaffObjs->dump();
        ColStaffObjs::iterator it = pColStaffObjs->begin();
                    //(clef G p1)
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(key C)
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(time 2 4)
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n e4 w p1)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(n c5 w p1)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 0 );
        ++it;       //(clef F4 p2)
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(key C)
        CHECK( (*it)->imo_object()->is_key_signature() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(time 2 4)
        CHECK( (*it)->imo_object()->is_time_signature() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(n c3 w p2)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(n g3 w p2)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->line() == 1 );
        ++it;       //(barline)
        CHECK( (*it)->imo_object()->is_barline() == true );
        CHECK( (*it)->line() == 0 );
        CHECK( is_equal_time((*it)->time(), 256.0f) );

        delete tree->get_root();
        delete pIModel;
    }

//Additional test for ColStaffObjs::iterator -------------------------------------

    TEST_FIXTURE(ColStaffObjsTestFixture, CSOIteratorAtEnd)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (staves 2)"
            "(musicData (clef G p1)(clef F4 p2)(key D)(n c4 q v2 p1)(n d4 e.)"
            "(n d4 s v3 p2)(n e4 h)))) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();
        //pColStaffObjs->dump();
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
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjs_Chord)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (musicData "
            "(clef G)(chord (n c4 q)(n e4 q)(n g4 q))"
            "))) ))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort
        ColStaffObjs::iterator it = pColStaffObjs->begin();

        //pColStaffObjs->dump();
        CHECK( pColStaffObjs->num_entries() == 4 );

        //(clef G)
        CHECK( (*it)->imo_object()->is_clef() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;

        //(n c4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;

        //(n e4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );
        ++it;

        //(n g4 q)
        CHECK( (*it)->imo_object()->is_note() == true );
        CHECK( (*it)->measure() == 0 );
        CHECK( is_equal_time((*it)->time(), 0.0f) );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjs_NoMusicData)
    {
        Document doc(m_libraryScope);
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLdpFactory);
        stringstream expected;
        expected << "Line 0. instrument: missing mandatory element 'musicData'." << endl;
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content (score "
            "(vers 1.6) (instrument )) ))" );
        Analyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, false);    //false: only creation, no sort

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pColStaffObjs != NULL );
        CHECK( pColStaffObjs->num_entries() == 0 );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(ColStaffObjsTestFixture, ColStaffObjsTimeInSequenceWhenDecimals)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_pLdpFactory);
        SpLdpTree tree = parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) (instrument (staves 2)(musicData "
            "(n a3 w p1)(goBack start)"
            "(n f2 w p2)(barline)"
            "(n a3 q p1)"
            "(n a3 e g+ t3)(n c4 e)(n e4 e g- t-)"
            "(n a3 h)(goBack start)"
            "(n a2 h p2)"
            "(n f2 h p2)(barline)"
            ")) )))" );
        Analyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ColStaffObjsBuilder builder;
        ColStaffObjs* pColStaffObjs = builder.build(pScore, true);    //true: do sort table
        ColStaffObjs::iterator it = pColStaffObjs->begin();

        //pColStaffObjs->dump();
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
        delete pIModel;
    }

}



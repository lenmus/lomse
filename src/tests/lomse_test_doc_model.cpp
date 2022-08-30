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
#include "private/lomse_document_p.h"
#include "lomse_compiler.h"
#include "lomse_time.h"
#include "lomse_internal_model.h"
#include "lomse_im_factory.h"
#include "lomse_id_assigner.h"
#include "lomse_staffobjs_table.h"
#include "lomse_mxl_exporter.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

#include <ctime>
#include <ratio>
#include <chrono>
using namespace std::chrono;




//=======================================================================================
// MyDocument:  Helper class to use Document protected members
//=======================================================================================
class MyDocument : public Document
{
public:
    MyDocument(LibraryScope& libraryScope) : Document(libraryScope) {}
   ~MyDocument() override {}

    void my_set_imo_doc(ImoDocument* pImo) { set_imo_doc(pImo); }
};

//=======================================================================================
// DumpVisitor:  Helper class for traversing the document and printing a dump
//=======================================================================================
class DumpVisitor : public Visitor<ImoObj>
{
protected:
    ostream& m_reporter;

    int m_indent = 0;
    int m_nodesIn = 0;
    int m_nodesOut = 0;
    int m_maxDepth = 0;

public:
    DumpVisitor(ostream& reporter) : Visitor<ImoObj>(), m_reporter(reporter) {}

    int num_in_nodes() { return m_nodesIn; }
    int num_out_nodes() { return m_nodesOut; }
    int max_depth() { return m_maxDepth; }

    void start_visit(ImoObj* pImo) override
    {
        m_reporter << indent() << "(" << pImo->get_name() << ", id=" << pImo->get_id();
        if (!pImo->has_visitable_children())
            m_reporter << ")";
        m_reporter << endl;

        m_indent++;
        m_nodesIn++;
        if (m_maxDepth < m_indent)
            m_maxDepth = m_indent;
    }

	void end_visit(ImoObj* pImo) override
    {
        m_indent--;
        m_nodesOut++;
        if (!pImo->has_visitable_children())
            return;
        m_reporter << indent() << ")" << endl;
    }


protected:

    string indent()
    {
        string spaces = "";
        for (int i=0; i < 3*m_indent; ++i)
            spaces += " ";
        return spaces;
    }

};

//=======================================================================================
// CheckTreeVisitor:  Helper class for traversing the document checking its nodes
//=======================================================================================
class CheckTreeVisitor : public Visitor<ImoObj>
{
protected:
    stringstream ss;
    DocModel* m_pDocModel = nullptr;
    bool m_fTestPassed = true;

public:
    CheckTreeVisitor(DocModel* pModel)
        : Visitor<ImoObj>()
        , m_pDocModel(pModel)
    {
    }

    bool test_passed() const { return m_fTestPassed; }
    std::string result() const { return ss.str(); }

    void start_visit(ImoObj* pImo) override
    {
        ImoId id = pImo->get_id();
        if (pImo->get_doc_model() != m_pDocModel)
        {
            m_fTestPassed = false;
            ss << "    " << pImo->get_name() << " #" << id << " different model "
                << pImo->get_doc_model() << endl;
        }
    }
};


//=======================================================================================
// DocModel cloning tests
//=======================================================================================

//---------------------------------------------------------------------------------------
class DocModelTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    DocModelTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
    }

    ~DocModelTestFixture()    //TearDown fixture
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    inline void failure_header()
    {
        cout << endl << "*** Failure in " << test_name() << ":" << endl;
    }

    bool check_result(const string& ss, const string& expected)
    {
        if (ss != expected)
        {
            failure_header();
            cout << "  result=[" << ss << "]" << endl;
            cout << endl << "expected=[" << expected << "]" << endl;
            return false;
        }
        return true;
    }

    bool check_to_string(ImoObj* pImo, ImoObj* pCopy)
    {
        if (pImo->to_string(true) != pCopy->to_string(true))
        {
            failure_header();
            cout << "    original: " << pImo->to_string(true) << endl;
            cout << "        copy: " << pCopy->to_string(true) << endl;
            return false;
        }
        else
            return true;
    }

    bool check_tree(ImoObj* pImo, ImoObj* pCopy)
    {
        stringstream dataOld;
        DumpVisitor v1(dataOld);
        pImo->accept_visitor(v1);

        stringstream dataCopy;
        DumpVisitor v2(dataCopy);
        pCopy->accept_visitor(v2);

        if (dataOld.str() != dataCopy.str())
        {
            failure_header();
            cout << "    original: " << dataOld.str() << endl;
            cout << "        copy: " << dataCopy.str() << endl;
            return false;
        }
        else
            return true;
    }

    void dump_tree(ImoObj* pImo)
    {
        stringstream data;
        DumpVisitor v(data);
        pImo->accept_visitor(v);
        cout << endl << test_name() << ":" << endl << data.str() << endl << endl;
    }

    bool check_equal_id_assigners(IdAssigner* pAssigner, IdAssigner* pAssignerCopy)
    {
        stringstream msg;
        bool fOK = pAssigner->check_ids(pAssignerCopy, msg, "copy");
        if (!fOK)
        {
            pAssignerCopy->check_ids(pAssigner, msg, "original");
            failure_header();
            cout << msg.str();
        }
        return fOK;
    }

    bool check_tree_owner(ImoObj* pImo, DocModel* pModel)
    {
        CheckTreeVisitor v(pModel);
        pImo->accept_visitor(v);
        if (!v.test_passed())
        {
            failure_header();
            cout << v.result() << endl;
            dump_tree(pImo);
            return false;
        }
        else
            return true;
    }

};

SUITE(DocModelTest)
{

    TEST_FIXTURE(DocModelTestFixture, clone_01)
    {
        //@01. clone node ImoDocument with only default children
        MyDocument doc(m_libraryScope);
        ImoDocument* pImo = static_cast<ImoDocument*>(ImFactory::inject(k_imo_document, &doc, 27L));
        doc.my_set_imo_doc(pImo);

        doc.create_backup_copy();
        DocModel* pModelCopy = doc.debug_get_backup_copy();
        ImoDocument* pImoCopy = pModelCopy->get_im_root();

        CHECK( pImoCopy->get_doc_model() == pModelCopy );
        CHECK( check_tree(pImo, pImoCopy) );
        CHECK( check_to_string(pImo, pImoCopy) );
        IdAssigner* pAssigner = doc.get_doc_model()->get_id_assigner();
        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
        CHECK( check_tree_owner(pImoCopy, pModelCopy) );
    }

    TEST_FIXTURE(DocModelTestFixture, clone_02)
    {
        //@02. clone simple ImoDocument

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q)"
                ")))" );
        ImoObj* pImo = doc.get_im_root();

        doc.create_backup_copy();
        DocModel* pModelCopy = doc.debug_get_backup_copy();
        ImoDocument* pImoCopy = pModelCopy->get_im_root();

        CHECK( pImoCopy->get_doc_model() == pModelCopy );
        CHECK( check_tree(pImo, pImoCopy) );
        CHECK( check_to_string(pImo, pImoCopy) );
        IdAssigner* pAssigner = doc.get_doc_model()->get_id_assigner();
        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
        CHECK( check_tree_owner(pImoCopy, pModelCopy) );
    }

    TEST_FIXTURE(DocModelTestFixture, clone_03)
    {
        //@03. clone ImoDocument with one ImoRelObj

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 e g+)(n e4 e g-)"
                ")))" );
        ImoObj* pImo = doc.get_im_root();

        doc.create_backup_copy();
        DocModel* pModelCopy = doc.debug_get_backup_copy();
        ImoDocument* pImoCopy = pModelCopy->get_im_root();

        CHECK( pImoCopy->get_doc_model() == pModelCopy );
        CHECK( check_tree(pImo, pImoCopy) );
        CHECK( check_to_string(pImo, pImoCopy) );
        IdAssigner* pAssigner = doc.get_doc_model()->get_id_assigner();
        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
        CHECK( check_tree_owner(pImoCopy, pModelCopy) );
    }

    TEST_FIXTURE(DocModelTestFixture, clone_04)
    {
        //@04. clone ImoDocument with several ImoRelObj

        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "unit-tests/xml-export/00050-grace-notes-alignment.xml", Document::k_format_mxl);
        doc.from_file(m_scores_path + "00050-grace-notes-alignment.xml",
                      Document::k_format_mxl);
        ImoObj* pImo = doc.get_im_root();

        doc.create_backup_copy();
        DocModel* pModelCopy = doc.debug_get_backup_copy();
        ImoDocument* pImoCopy = pModelCopy->get_im_root();

        CHECK( pImoCopy->get_doc_model() == pModelCopy );
        CHECK( check_tree(pImo, pImoCopy) );
        CHECK( check_to_string(pImo, pImoCopy) );
        IdAssigner* pAssigner = doc.get_doc_model()->get_id_assigner();
        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
        CHECK( check_tree_owner(pImoCopy, pModelCopy) );
    }

    TEST_FIXTURE(DocModelTestFixture, clone_05)
    {
        //@05. clone ImoDocument with ImoAuxRelObj

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/docmodel/02091-lyrics-melisma-hyphenation.lms",
                      Document::k_format_ldp);
        ImoObj* pImo = doc.get_im_root();

        doc.create_backup_copy();
        DocModel* pModelCopy = doc.debug_get_backup_copy();
        ImoDocument* pImoCopy = pModelCopy->get_im_root();

        CHECK( pImoCopy->get_doc_model() == pModelCopy );
        CHECK( check_tree(pImo, pImoCopy) );
        CHECK( check_to_string(pImo, pImoCopy) );
        IdAssigner* pAssigner = doc.get_doc_model()->get_id_assigner();
        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
        CHECK( check_tree_owner(pImoCopy, pModelCopy) );
    }

    TEST_FIXTURE(DocModelTestFixture, clone_06)
    {
        //@06. clone ImoDocument with ImoControl

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/docmodel/08031-score-player.lms",
                      Document::k_format_ldp);
        ImoObj* pImo = doc.get_im_root();

        doc.create_backup_copy();
        DocModel* pModelCopy = doc.debug_get_backup_copy();
        ImoDocument* pImoCopy = pModelCopy->get_im_root();

        CHECK( pImoCopy->get_doc_model() == pModelCopy );
        CHECK( check_tree(pImo, pImoCopy) );
        CHECK( check_to_string(pImo, pImoCopy) );
        IdAssigner* pAssigner = doc.get_doc_model()->get_id_assigner();
        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
        CHECK( check_tree_owner(pImoCopy, pModelCopy) );
//        dump_tree(pImoCopy);
//        cout << pAssignerCopy->dump() << endl;
    }

    TEST_FIXTURE(DocModelTestFixture, clone_07)
    {
        //@07. clone ImoDocument with ImoStaffInfo

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/docmodel/14a-StaffDetails-LineChanges.xml",
                      Document::k_format_mxl);
        ImoObj* pImo = doc.get_im_root();

        doc.create_backup_copy();
        DocModel* pModelCopy = doc.debug_get_backup_copy();
        ImoDocument* pImoCopy = pModelCopy->get_im_root();

        CHECK( pImoCopy->get_doc_model() == pModelCopy );
        CHECK( check_tree(pImo, pImoCopy) );
        CHECK( check_to_string(pImo, pImoCopy) );
        IdAssigner* pAssigner = doc.get_doc_model()->get_id_assigner();
        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
//        cout << pAssigner->dump() << endl;
//        dump_tree(pImo);
//        dump_tree(pImoCopy);
        CHECK( check_tree_owner(pImoCopy, pModelCopy) );
    }

    TEST_FIXTURE(DocModelTestFixture, clone_08)
    {
        //@08. AttrList and AttrObj are cloned

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/012-directive-moved-to-note.xml",
                      Document::k_format_mxl);
        ImoObj* pImo = doc.get_im_root();

        doc.create_backup_copy();
        DocModel* pModelCopy = doc.debug_get_backup_copy();
        ImoDocument* pImoCopy = pModelCopy->get_im_root();

        CHECK( pImoCopy->get_doc_model() == pModelCopy );
        CHECK( check_tree(pImo, pImoCopy) );
        CHECK( check_to_string(pImo, pImoCopy) );
        IdAssigner* pAssigner = doc.get_doc_model()->get_id_assigner();
        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
        CHECK( check_tree_owner(pImoCopy, pModelCopy) );

        ImoScore* pScore = static_cast<ImoScore*>( pImoCopy->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef (clef#39 G p1)
        ++it;   //direction (dir#40 0 p1 (TODO: #43 No LdpGenerator for sound-change))
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<direction placement=\"below\">"
            "<direction-type><dynamics><p/></dynamics></direction-type>"
            "<sound dynamics=\"54\"/>"
            "</direction>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(DocModelTestFixture, clone_09)
    {
        //@09. Key signature is correctly generated in ColStaffObjs

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/conversion/20-wedge.xml",
                      Document::k_format_mxl);

        DocModel* pModel = doc.get_doc_model();
        ImoObj* pImo = pModel->get_im_root();
        doc.create_backup_copy();
        DocModel* pModelCopy = doc.debug_get_backup_copy();
        ImoDocument* pImoCopy = pModelCopy->get_im_root();

        CHECK( pImoCopy->get_doc_model() == pModelCopy );
        CHECK( check_tree(pImo, pImoCopy) );
        CHECK( check_to_string(pImo, pImoCopy) );
        IdAssigner* pAssigner = pModel->get_id_assigner();
        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
//        cout << pAssigner->dump() << endl;
//        dump_tree(pImo);
//        dump_tree(pImoCopy);
        CHECK( check_tree_owner(pImoCopy, pModelCopy) );

//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << test_name() << endl << pTable->dump();
    }

    TEST_FIXTURE(DocModelTestFixture, clone_099)
    {
        //@099. check for bugs when using a cloned copy

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/conversion/20-wedge.xml",
                      Document::k_format_mxl);

        doc.create_backup_copy();
        doc.restore_from_backup_copy();

        DocModel* pModel = doc.get_doc_model();
        ImoObj* pImo = pModel->get_im_root();
        doc.create_backup_copy();

        DocModel* pModelCopy = doc.debug_get_backup_copy();
        ImoDocument* pImoCopy = pModelCopy->get_im_root();

        CHECK( pImoCopy->get_doc_model() == pModelCopy );
        CHECK( check_tree(pImo, pImoCopy) );
        CHECK( check_to_string(pImo, pImoCopy) );
        IdAssigner* pAssigner = pModel->get_id_assigner();
        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
//        cout << pAssigner->dump() << endl;
//        dump_tree(pImo);
//        dump_tree(pImoCopy);
        CHECK( check_tree_owner(pImoCopy, pModelCopy) );
    }

//    TEST_FIXTURE(DocModelTestFixture, clone_999)
//    {
//        //@999. benchmarks and measurements
//        cout << test_name() << ":" << endl;
//        cout << "    sizeof(ImoBarline) = " << sizeof(ImoBarline)
//            << " (" << sizeof(ImoBarline) / 8 << " words)"<< endl;
//        cout << "    sizeof(ImoNote) = " << sizeof(ImoNote)
//            << " (" << sizeof(ImoNote) / 8 << " words)"<< endl;
//        cout << "    sizeof(ImoObj) = " << sizeof(ImoObj)
//            << " (" << sizeof(ImoObj) / 8 << " words)"<< endl;
//        cout << "    sizeof(std::list<ImoObj*>) = " << sizeof(std::list<ImoObj*>)
//            << " (" << sizeof(std::list<ImoObj*>) / 8 << " words)"<< endl;
//        cout << "    sizeof(TreeNode<ImoObj*>) = " << sizeof(TreeNode<ImoObj*>)
//            << " (" << sizeof(TreeNode<ImoObj*>) / 8 << " words)"<< endl;
//
//        stringstream errormsg;
//        Document doc(m_libraryScope, errormsg);
//        high_resolution_clock::time_point t1 = high_resolution_clock::now();
//        doc.from_file("/datos/cecilio/lm/projects/lomse/vregress/scores/recordare/ActorPreludeSample.musicxml",
//                      Document::k_format_mxl);
//        high_resolution_clock::time_point t2 = high_resolution_clock::now();
//        duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
//        IdAssigner* pAssigner = doc.get_doc_model()->get_id_assigner();
//        cout << endl << "    Some measurements using ActorPreludeSample test score:" << endl;
//        cout << "        Tree has " << pAssigner->size() << " nodes." << endl;
//        cout << "        Load from file requires " << timeSpan.count() << " seconds." << endl;
//
//        ImoObj* pImo = doc.get_im_root();
//
//        t1 = high_resolution_clock::now();
//        doc.create_backup_copy();
//        t2 = high_resolution_clock::now();
//        timeSpan = duration_cast<duration<double>>(t2 - t1);
//        cout << "        Clone requires " << timeSpan.count() << " seconds." << endl << endl;;
//
//        DocModel* pModelCopy = doc.debug_get_backup_copy();
//        ImoDocument* pImoCopy = pModelCopy->get_im_root();
//
//        CHECK( pImoCopy->get_doc_model() == pModelCopy );
//        CHECK( check_tree(pImo, pImoCopy) );
//        CHECK( check_to_string(pImo, pImoCopy) );
//        IdAssigner* pAssignerCopy = pModelCopy->get_id_assigner();
//        CHECK( check_equal_id_assigners(pAssigner, pAssignerCopy) );
//        CHECK( check_tree_owner(pImoCopy, pModelCopy) );
//    }

}


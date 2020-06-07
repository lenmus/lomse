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
#include "lomse_internal_model.h"
#include "lomse_time.h"
#include "private/lomse_document_p.h"
#include "lomse_document.h"
#include "lomse_presenter.h"
#include "lomse_graphic_view.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class InternalModelApiTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    InternalModelApiTestFixture()
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~InternalModelApiTestFixture()
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

};

//---------------------------------------------------------------------------------------
SUITE(InternalModelApiTest)
{

    //@ IDocument -----------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0100)
    {
        //@0100. Document returns valid IDocument
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        ImoDocument* pImoDoc = theDoc.get_im_root();

        unique_ptr<IDocument> doc = theDoc.get_document_api();

        CHECK( doc->get_lmd_version() == pImoDoc->get_version() );
        CHECK( doc->get_lmd_version() == "0.0" );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0101)
    {
        //@0101. Presenter returns valid IDocument
        PresenterBuilder builder(m_libraryScope);
        Presenter* pPresenter = builder.new_document(k_view_simple,
            "(lenmusdoc (vers 0.0)(content "
            "(text \"hello world\")"
            "(score#505 (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
            "))", cout, Document::k_format_ldp
        );
        unique_ptr<IDocument> doc = pPresenter->get_document();

        unique_ptr<IScore> score = doc->get_first_score();
        CHECK( score->get_object_id() == 505 );

        delete pPresenter;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0201)
    {
        //@0201. Access to first score-> Not found
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content ))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();

        unique_ptr<IScore> score = doc->get_first_score();

        CHECK( score == nullptr );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0202)
    {
        //@0202. Access to first score-> Success

        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();

        unique_ptr<IScore> score = doc->get_first_score();

        CHECK( score != nullptr );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0203)
    {
        //@0203. Access to first score-> More content. Success
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
            "(text \"hello world\")"
            "(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
            "))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();

        unique_ptr<IScore> score = doc->get_first_score();

        CHECK( score != nullptr );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0204)
    {
        //@0204. get_num_children(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
                "(text \"hello world\")"
                "(score (vers 2.0)"
                    "(instrument (musicData (clef G)(n g4 q)))"
                ")"
            "))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();

        CHECK( doc->get_num_children() == 2 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0205)
    {
        //@0205. get_num_children(). Empty document
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();

        CHECK( doc->get_num_children() == 0 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0206)
    {
        //@0206. get_first_child(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
                "(score (vers 2.0)"
                    "(instrument (musicData (clef G)(n g4 q)) )"
                ")"
                "(text \"hello world\")"
                "(score (vers 2.0)"
                    "(instrument (musicData (clef G)(n g4 q)) )"
                ")"
            "))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();

        //child found
        CHECK( doc->get_num_children() == 3 );
        unique_ptr<IElement> content = doc->get_first_child();
        CHECK( content != nullptr );

        //and can be downcasted
        IScore* score = dynamic_cast<IScore*>(content.get());
        CHECK( score != nullptr );
        ImoScore* pScore = score->get_internal_object();
        CHECK( pScore->is_score() == true );
    }


    //@ IScore --------------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0100)
    {
        //@0100. get_object_id()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score#301 (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();

        CHECK( score->get_object_id() == 301 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0201)
    {
        //@0201. get_num_instruments()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();

        CHECK( score->get_num_instruments() == 1 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0202)
    {
        //@0202. get_instrument_at()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument#402 (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();

        unique_ptr<IInstrument> instr = score->get_instrument_at(0);

        CHECK( instr->get_object_id() == 402 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0203)
    {
        //@0203. delete_instrument(). Not in group
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument#402 (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrument> instr = score->get_instrument_at(0);

        score->delete_instrument(instr->get_object_id());

        CHECK( score->get_num_instruments() == 0 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0204)
    {
        //@0204. delete instrument does not remove the group when enough instruments
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)(group P1 P3))"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C2)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrument> instr = score->get_instrument_at(0);

        score->delete_instrument(instr->get_object_id());

        CHECK( score->get_num_instruments() == 3 );
        CHECK( score->get_num_instruments_groups() == 1 );

        //check group integrity
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);
        CHECK( group->get_num_instruments() == 3 );
        unique_ptr<IInstrument> instr1 = group->get_first_instrument();
        unique_ptr<IInstrument> instr2 = group->get_last_instrument();
        CHECK( instr1->get_object_id() == 400 );
        CHECK( instr2->get_object_id() == 600 );

//        cout << test_name() << ", num.instrs=" << group->get_num_instruments() << endl;
//        ImoInstrGroup* pGrp = group->get_internal_object();
//        int numInstrs = pGrp->get_num_instruments();
//        for (int i=0; i < numInstrs; ++i)
//        {
//            ImoInstrument* pInstr = pGrp->get_instrument(i);
//            cout << "        instr: index=" << i << ", id=" << pInstr->get_id() << endl;
//        }
//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0205)
    {
        //@0205. delete instrument reduces group size when less instruments
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P1 P3))"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef C2)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrument> instr = score->get_instrument_at(0);

        score->delete_instrument(instr->get_object_id());

        CHECK( score->get_num_instruments() == 2 );
        CHECK( score->get_num_instruments_groups() == 1 );

        //check group integrity
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);
        CHECK( group->get_num_instruments() == 2 );
        unique_ptr<IInstrument> instr1 = group->get_first_instrument();
        unique_ptr<IInstrument> instr2 = group->get_last_instrument();
        CHECK( instr1->get_object_id() == 400 );
        CHECK( instr2->get_object_id() == 500 );

//        cout << test_name() << ", num.instrs=" << group->get_num_instruments() << endl;
//        ImoInstrGroup* pGrp = group->get_internal_object();
//        int numInstrs = pGrp->get_num_instruments();
//        for (int i=0; i < numInstrs; ++i)
//        {
//            ImoInstrument* pInstr = pGrp->get_instrument(i);
//            cout << "        instr: index=" << i << ", id=" << pInstr->get_id() << endl;
//        }
//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0206)
    {
        //@0206. delete instrument removes group when less than two instruments
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2)(group P1 P2))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrument> instr = score->get_instrument_at(0);

        score->delete_instrument(instr->get_object_id());

        CHECK( score->get_num_instruments() == 1 );
        CHECK( score->get_num_instruments_groups() == 0 );

//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0210)
    {
        //@0210. move up instrument. When first instrument nothing changes
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P1 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrument> instr = score->get_instrument_at(0);

        score->move_up_instrument(*instr);

        CHECK( score->get_num_instruments() == 3 );
        CHECK( score->get_num_instruments_groups() == 1 );
        CHECK( score->get_instrument_at(0)->get_object_id() == 300L );
        CHECK( score->get_instrument_at(1)->get_object_id() == 400L );
        CHECK( score->get_instrument_at(2)->get_object_id() == 500L );

        //check group
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);
        CHECK( group->get_num_instruments() == 3 );
        CHECK( group->get_first_instrument()->get_object_id() == 300L );
        CHECK( group->get_last_instrument()->get_object_id() == 500L );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0211)
    {
        //@0211. move up instrument. OK.
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)(group P2 P4))"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrument> instr = score->get_instrument_at(1);

        score->move_up_instrument(*instr);

        CHECK( score->get_num_instruments() == 4 );
        CHECK( score->get_num_instruments_groups() == 1 );
        CHECK( score->get_instrument_at(0)->get_object_id() == 400L );
        CHECK( score->get_instrument_at(1)->get_object_id() == 300L );
        CHECK( score->get_instrument_at(2)->get_object_id() == 500L );
        CHECK( score->get_instrument_at(3)->get_object_id() == 600L );

        //check group
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);
        CHECK( group->get_num_instruments() == 3 );
        CHECK( group->get_first_instrument()->get_object_id() == 300L );
        CHECK( group->get_last_instrument()->get_object_id() == 600L );

//        cout << test_name() << ", num.instrs=" << group->get_num_instruments() << endl;
//        ImoInstrGroup* pGrp = group->get_internal_object();
//        int numInstrs = pGrp->get_num_instruments();
//        for (int i=0; i < numInstrs; ++i)
//        {
//            ImoInstrument* pInstr = pGrp->get_instrument(i);
//            cout << "        instr: index=" << i << ", id=" << pInstr->get_id() << endl;
//        }
//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0220)
    {
        //@0220. move down instrument. When last instrument nothing changes
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P1 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrument> instr = score->get_instrument_at(2);

        score->move_down_instrument(*instr);

        CHECK( score->get_num_instruments() == 3 );
        CHECK( score->get_num_instruments_groups() == 1 );
        CHECK( score->get_instrument_at(0)->get_object_id() == 300L );
        CHECK( score->get_instrument_at(1)->get_object_id() == 400L );
        CHECK( score->get_instrument_at(2)->get_object_id() == 500L );

        //check group
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);
        CHECK( group->get_num_instruments() == 3 );
        CHECK( group->get_first_instrument()->get_object_id() == 300L );
        CHECK( group->get_last_instrument()->get_object_id() == 500L );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0221)
    {
        //@0221. move down instrument. OK.
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)(group P2 P4))"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrument> instr = score->get_instrument_at(2);

        score->move_down_instrument(*instr);

        CHECK( score->get_num_instruments() == 4 );
        CHECK( score->get_num_instruments_groups() == 1 );
        CHECK( score->get_instrument_at(0)->get_object_id() == 300L );
        CHECK( score->get_instrument_at(1)->get_object_id() == 400L );
        CHECK( score->get_instrument_at(2)->get_object_id() == 600L );
        CHECK( score->get_instrument_at(3)->get_object_id() == 500L );

        //check group
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);
        CHECK( group->get_num_instruments() == 3 );
        CHECK( group->get_first_instrument()->get_object_id() == 400L );
        CHECK( group->get_last_instrument()->get_object_id() == 500L );

//        cout << test_name() << ", num.instrs=" << group->get_num_instruments() << endl;
//        ImoInstrGroup* pGrp = group->get_internal_object();
//        int numInstrs = pGrp->get_num_instruments();
//        for (int i=0; i < numInstrs; ++i)
//        {
//            ImoInstrument* pInstr = pGrp->get_instrument(i);
//            cout << "        instr: index=" << i << ", id=" << pInstr->get_id() << endl;
//        }
//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0230)
    {
        //@0230. append_new_instrument()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P2 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();

        unique_ptr<IInstrument> instr = score->append_new_instrument();

        CHECK( score->get_num_instruments() == 4 );
        CHECK( score->get_num_instruments_groups() == 1 );
        CHECK( score->get_instrument_at(0)->get_object_id() == 300L );
        CHECK( score->get_instrument_at(1)->get_object_id() == 400L );
        CHECK( score->get_instrument_at(2)->get_object_id() == 500L );
        CHECK( score->get_instrument_at(3)->get_object_id() == instr->get_object_id() );
        ImoInstrument* pInstr = instr->get_internal_object();
        CHECK( pInstr->get_instr_id().empty() == false );

        //check group
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);
        CHECK( group->get_num_instruments() == 2 );
        CHECK( group->get_first_instrument()->get_object_id() == 400L );
        CHECK( group->get_last_instrument()->get_object_id() == 500L );

//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << ". Source= " << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0301)
    {
        //@0301. create_instruments_group()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();

        unique_ptr<IInstrGroup> group = score->create_instruments_group(1, 2);

        CHECK( score->get_num_instruments() == 3 );

        //check group
        CHECK( score->get_num_instruments_groups() == 1 );
        unique_ptr<IInstrGroup> group2 = score->get_instruments_group_at(0);
        CHECK( group->get_object_id() == group2->get_object_id() );
        CHECK( group->get_num_instruments() == 2 );
        CHECK( group->get_first_instrument()->get_object_id() == 400L );
        CHECK( group->get_last_instrument()->get_object_id() == 500L );

//        cout << test_name() << ", num.instrs=" << group->get_num_instruments() << endl;
//        ImoInstrGroup* pGrp = group->get_internal_object();
//        int numInstrs = pGrp->get_num_instruments();
//        for (int i=0; i < numInstrs; ++i)
//        {
//            ImoInstrument* pInstr = pGrp->get_instrument(i);
//            cout << "        instr: index=" << i << ", id=" << pInstr->get_id() << endl;
//        }
//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0302)
    {
        //@0302. create_instruments_group(). Invalid arguments
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();

        //out of range
        unique_ptr<IInstrGroup> group = score->create_instruments_group(2, 3);
        CHECK( group == nullptr );
        CHECK( score->get_num_instruments_groups() == 0 );

        //out of order
        unique_ptr<IInstrGroup> group2 = score->create_instruments_group(1, 0);
        CHECK( group2 == nullptr );
        CHECK( score->get_num_instruments_groups() == 0 );

        //only one instrument
        unique_ptr<IInstrGroup> group3 = score->create_instruments_group(1, 1);
        CHECK( group3 == nullptr );
        CHECK( score->get_num_instruments_groups() == 0 );

//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0310)
    {
        //@0310. delete_instruments_group_at(). Non-existing group
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P2 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();

        CHECK( score->delete_instruments_group_at(2) == false );

        CHECK( score->get_num_instruments_groups() == 1 );

//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0311)
    {
        //@0311. delete_instruments_group_at(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P2 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();

        score->delete_instruments_group_at(0);

        CHECK( score->get_num_instruments_groups() == 0 );

//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0312)
    {
        //@0312. delete_instruments_group()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P2 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);

        score->delete_instruments_group(*group);

        CHECK( score->get_num_instruments_groups() == 0 );

//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0313)
    {
        //@0313. delete_all_instruments_groups()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P1 P2)(group P2 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();

        score->delete_all_instruments_groups();

        CHECK( score->get_num_instruments_groups() == 0 );

//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }


    //@ IInstrument ---------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrument_0100)
    {
        //@0100. get and set name and abbreviation
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument (name \"Violin\")(abbrev \"V.\")(musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrument> instr = score->get_instrument_at(0);
        CHECK( instr->get_name_string() == "Violin" );
        CHECK( instr->get_abbreviation_string() == "V." );

        instr->set_name_string("Flute");
        instr->set_abbreviation_string("F.");

        CHECK( instr->get_name_string() == "Flute" );
        CHECK( instr->get_abbreviation_string() == "F." );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrument_0101)
    {
        //@0101. get and set MIDI information
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument (name \"Violin\")(abbrev \"V.\")(musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrument> instr = score->get_instrument_at(0);

        CHECK( instr->get_num_sounds() == 1 );
        unique_ptr<ISoundInfo> sound = instr->get_sound_info_at(0);
        unique_ptr<IMidiInfo> midi = sound->get_midi_info();
        CHECK( midi->get_channel() == 1 );
        CHECK( midi->get_program() == 1 );
        midi->set_channel(7);
        midi->set_program(2);

        unique_ptr<IScore> score2 = doc->get_first_score();
        unique_ptr<IInstrument> instr2 = score2->get_instrument_at(0);
        unique_ptr<ISoundInfo> sound2 = instr2->get_sound_info_at(0);
        unique_ptr<IMidiInfo> midi2 = sound2->get_midi_info();
        CHECK( midi2->get_channel() == 7 );
        CHECK( midi2->get_program() == 2 );
    }


    //@ IInstrGroup ---------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0100)
    {
        //@0100. get and set name and abbreviation
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P1 P2 (name \"Group1\")(symbol bracket)(joinBarlines no))"
                "(group P3 P4 (abbrev \"G2\")(symbol brace)(joinBarlines yes))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrGroup> group1 = score->get_instruments_group_at(0);
        unique_ptr<IInstrGroup> group2 = score->get_instruments_group_at(1);
        CHECK( group1->get_name_string() == "Group1" );
        CHECK( group1->get_abbreviation_string() == "" );
        CHECK( group2->get_name_string() == "" );
        CHECK( group2->get_abbreviation_string() == "G2" );

        group1->set_abbreviation_string("G1");
        group2->set_name_string("Group2");

        CHECK( group1->get_name_string() == "Group1" );
        CHECK( group1->get_abbreviation_string() == "G1" );
        CHECK( group2->get_name_string() == "Group2" );
        CHECK( group2->get_abbreviation_string() == "G2" );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0110)
    {
        //@0110. get and set symbol
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P1 P2 (name \"Group1\")(symbol bracket)(joinBarlines no))"
                "(group P3 P4 (abbrev \"G2\")(symbol brace)(joinBarlines yes))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrGroup> group1 = score->get_instruments_group_at(0);
        unique_ptr<IInstrGroup> group2 = score->get_instruments_group_at(1);

        CHECK( group1->get_symbol() == EGroupSymbol::k_group_symbol_bracket );
        CHECK( group2->get_symbol() == EGroupSymbol::k_group_symbol_brace );

        group1->set_symbol(EGroupSymbol::k_group_symbol_none);
        group2->set_symbol(EGroupSymbol::k_group_symbol_line);

        CHECK( group1->get_symbol() == k_group_symbol_none );
        CHECK( group2->get_symbol() == k_group_symbol_line );

//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0120)
    {
        //@0120. get and set barlines_mode
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P1 P2 (name \"Group1\")(symbol bracket)(joinBarlines no))"
                "(group P3 P4 (abbrev \"G2\")(symbol brace)(joinBarlines yes))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrGroup> group1 = score->get_instruments_group_at(0);
        unique_ptr<IInstrGroup> group2 = score->get_instruments_group_at(1);

        CHECK( group1->get_barlines_mode() == EJoinBarlines::k_non_joined_barlines );
        CHECK( group2->get_barlines_mode() == EJoinBarlines::k_joined_barlines );

        group1->set_barlines_mode(EJoinBarlines::k_mensurstrich_barlines);
        group2->set_barlines_mode(EJoinBarlines::k_non_joined_barlines);

        CHECK( group1->get_barlines_mode() == k_mensurstrich_barlines );
        CHECK( group2->get_barlines_mode() == k_non_joined_barlines );

//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0130)
    {
        //@0130. set_range()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P1 P2 (name \"Group1\")(symbol bracket)(joinBarlines no))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);

        //out of range
        CHECK( group->set_range(0, 4) == false );

        //out of order
        CHECK( group->set_range(3, 2) == false );

        //only one instrument
        CHECK( group->set_range(1, 1) == false );

        //valid range
        CHECK( group->set_range(1, 3) == true );

        //check group
        unique_ptr<IInstrGroup> group1 = score->get_instruments_group_at(0);
        CHECK( group1->get_num_instruments() == 3 );
        CHECK( group1->get_first_instrument()->get_object_id() == 400L );
        CHECK( group1->get_last_instrument()->get_object_id() == 600L );

//        Document* pDoc = doc->get_internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0140)
    {
        //@0140. get_instrument_at(). Invalid parameter
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P2 P4 (name \"Group1\")(symbol bracket)(joinBarlines no))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);

        unique_ptr<IInstrument> instr = group->get_instrument_at(3);
        CHECK( instr == nullptr );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0141)
    {
        //@0141. get_instrument_at(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P2 P4 (name \"Group1\")(symbol bracket)(joinBarlines no))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);

        unique_ptr<IInstrument> instr = group->get_instrument_at(2);
        CHECK( instr != nullptr );
        CHECK( instr->get_name_string() == "P4" );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0142)
    {
        //@0142. get index to_first and last instruments
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P2 P4 (name \"Group1\")(symbol bracket)(joinBarlines no))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        unique_ptr<IDocument> doc = theDoc.get_document_api();
        unique_ptr<IScore> score = doc->get_first_score();
        unique_ptr<IInstrGroup> group = score->get_instruments_group_at(0);

        CHECK( group->get_index_to_first_instrument() == 1 );
        CHECK( group->get_index_to_last_instrument() == 3 );
    }

}



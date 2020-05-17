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
//#include "lomse_view.h"
#include "lomse_graphic_view.h"
//#include "lomse_document_cursor.h"
//#include "lomse_interactor.h"


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

        IDocument doc = theDoc.get_document_api();

        CHECK( doc.get_version() == pImoDoc->get_version() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0101)
    {
        //@0101. Access to first score. Not found
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content ))"
        );
        IDocument doc = theDoc.get_document_api();

        IScore score = doc.get_first_score();

        CHECK( score.is_valid() == false );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0102)
    {
        //@0102. Access to first score. Success

        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        IDocument doc = theDoc.get_document_api();

        IScore score = doc.get_first_score();

        CHECK( score.is_valid() == true );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0103)
    {
        //@0103. Access to first score. More content. Success
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
        IDocument doc = theDoc.get_document_api();

        IScore score = doc.get_first_score();

        CHECK( score.is_valid() == true );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0104)
    {
        //@0104. Presenter returns valid IDocument
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
        IDocument doc = pPresenter->get_document();

        IScore score = doc.get_first_score();
        CHECK( score.get_score_id() == 505 );

        delete pPresenter;
    }


    //@ IScore --------------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0100)
    {
        //@0100. get_score_id()

        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score#301 (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        IDocument doc = theDoc.get_document_api();

        IScore score = doc.get_first_score();

        CHECK( score.get_score_id() == 301 );
    }

}



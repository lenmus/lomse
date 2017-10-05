//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2017. All rights reserved.
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
#include "lomse_jumps_table.h"
#include "lomse_document.h"

#include <exception>
using namespace UnitTest;
using namespace std;
using namespace lomse;


////---------------------------------------------------------------------------------------
////Derived class to access protected members
//class MyJumpsTable : public JumpsTable
//{
//protected:
//
//public:
//    MyJumpsTable(ImoScore* pScore)
//        : JumpsTable(pScore)
//    {
//    }
//    virtual ~MyJumpsTable() {}
//
//    //access to protected member methods
//    bool my_are_there_staves_needing_clef()
//    {
//        find_staves_needing_clef();
//
//        vector<bool>::iterator it;
//        for (it=m_fNeedsClef.begin(); it != m_fNeedsClef.end(); ++it)
//        {
//            if (*it==true)
//                return true;
//        }
//        return false;
//    }
//    FPitch my_max_pitch(int idx)
//    {
//        return m_maxPitch[idx];
//    }
//    FPitch my_min_pitch(int idx)
//    {
//        return m_minPitch[idx];
//    }
//    int my_num_notes(int idx)
//    {
//        return m_numNotes[idx];
//    }
//
//};

//=======================================================================================
// JumpsTable tests
//=======================================================================================
class JumpsTableTestFixture
{
public:
    LibraryScope m_libraryScope;
    string m_scores_path;

    JumpsTableTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~JumpsTableTestFixture()    //TearDown fixture
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

};

//---------------------------------------------------------------------------------------
SUITE(JumpsTableTest)
{

    TEST_FIXTURE(JumpsTableTestFixture, jumps_table_01)
    {
        //@001. empty score creates empty table
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );

		JumpsTable* pTable = LOMSE_NEW JumpsTable(pScore);
		pTable->create_table();

        CHECK( pTable->num_entries() == 0 );
    }


};


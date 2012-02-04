//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
#include "lomse_parser.h"
#include "lomse_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LinkerTestFixture
{
public:

    LinkerTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = LOMSE_NEW LibraryScope(cout);
    }

    ~LinkerTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
};

SUITE(LinkerTest)
{

//    TEST_FIXTURE(LinkerTestFixture, AnalyserOneOrMorePresentOne)
//    {
//        stringstream errormsg;
//        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        SpLdpTree tree = parser.parse_text("(score (vers 1.6)(instrument (musicData)))");
//        Analyser a(errormsg, m_pLibraryScope->ldp_factory());
//        a.analyse_tree(tree);
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImScore* pScore = dynamic_cast<ImScore*>( tree->get_root()->get_imobj() );
//        CHECK( pScore != NULL );
//        //CHECK( pScore->get_num_instruments() == 1 );
//        delete tree->get_root();
//    }

}


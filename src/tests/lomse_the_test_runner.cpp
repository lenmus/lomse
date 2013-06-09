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

#include <iostream>
#include <UnitTest++.h>
#include <TestReporterStdout.h>

#include "lomse_build_options.h"
#include "lomse_injectors.h"

#include <string.h>

using namespace std;
using namespace lomse;
using namespace UnitTest;

int main(int argc, char** argv)
{
    //Based on code published here:
    //  http://stackoverflow.com/questions/3546054/how-do-i-run-a-single-test-with-unittest
    //
    //invoke without arguments to run all tests:
    //  testlib
    //
    //invoke with arguments to run a single test:
    //  testlib MyTestName
    //
    //or single suite
    //  testlib suite MySuite

    cout << "Lomse version " << LibraryScope::get_version_string()
         << ". Library tests runner." << endl << endl;

    cout << "Path for tests scores: '" << TESTLIB_SCORES_PATH << "'" << endl << endl;

    int nErrors = 0;

    if( argc > 1 )
    {
        //if first arg is "suite", we search for suite names instead of test names
        const bool fSuite = strcmp( "suite", argv[1] ) == 0;

        if (fSuite)
            cout << "Running tests in suite " << argv[2] << endl << endl;
        else
            cout << "Running some tests " << endl << endl;

        //walk list of all tests, add those with a name that
        //matches one of the arguments  to a new TestList
        const TestList& allTests( Test::GetTestList() );
        TestList selectedTests;
        Test* p = allTests.GetHead();
        while( p )
        {
            for( int i = 1 ; i < argc ; ++i )
            {
                if( strcmp( fSuite ? p->m_details.suiteName
                                   : p->m_details.testName, argv[ i ] ) == 0 )
                    selectedTests.Add( p );
            }
            Test* q = p;
            p = p->next;
            q->next = NULL;
        }

        //run selected test(s) only
        UnitTest::TestReporterStdout reporter;
        UnitTest::TestRunner runner( reporter );
        nErrors = runner.RunTestsIf( selectedTests, 0, True(), 0 );
    }
    else
    {
        cout << "Running all tests" << endl << endl;
        nErrors = UnitTest::RunAllTests();
    }

    #if defined WIN32 || defined _WIN32
        //system("pause");
        cin.get();
        _CrtDumpMemoryLeaks();

    #endif

    return nErrors;
}

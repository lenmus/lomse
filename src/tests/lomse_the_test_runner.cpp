//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

const char* basefilename(const char* fullfilename)
{
    const char* onlyFilenamePosix = strrchr(fullfilename, '/');
    const char* onlyFilenameWin = strrchr(fullfilename, '\\');
    const char* basename = std::max(onlyFilenamePosix, onlyFilenameWin) + 1;
    return basename;
}

class MyTestReporterStdout : public UnitTest::TestReporterStdout
{
public:
    MyTestReporterStdout(bool verbose) : m_verbose(verbose) {}

    void ReportTestStart(TestDetails const& test) override
    {
        if (m_verbose)
        {
            printf("%s / %s / %s\n", basefilename(test.filename), test.suiteName, test.testName);
        }
    }

    void ReportFailure(TestDetails const& test, char const* failure) override
    {
        if (m_verbose)
        {
            printf("    Failure at line %d: %s\n", test.lineNumber, failure);
        }
        else
        {
            printf("%s(%d): Failure in %s / %s: %s\n",
                basefilename(test.filename), test.lineNumber, test.suiteName, test.testName, failure);
        }
    }

    void ReportSummary(int totalTestCount, int failedTestCount,
        int failureCount, float secondsElapsed) override
    {
        if (failureCount > 0)
            printf("\nFailure: %d out of %d tests failed (%d failures).\n", failedTestCount, totalTestCount, failureCount);
        else
            printf("\nSuccess: %d tests passed.\n", totalTestCount);

        printf("Test time: %.2f seconds.\n", secondsElapsed);
    }

private:
    bool m_verbose;
    string m_lastFailed;
};

bool TestMatches(TestDetails const& test, const string& name)
{
    return name == test.suiteName || name == test.testName ||
        name == basefilename(test.filename);
}

int main(int argc, char** argv)
{
    //invoke without arguments to run all tests:
    //  testlib
    //
    //invoke with arguments to run a single test:
    //  testlib MyTestName
    //
    //or single suite:
    //  testlib MySuite
    //
    //or single filename:
    //  testlib filename.cpp
    //
    //exclude Test/Suite/filename:
    //  testlib -MyTestNameOrSuiteNameOrFilename
    //
    //verbose output:
    //  testlib -v
    //  testlib --verbose
    //  testlib -v MyTestName
    //  testlib -v MySuite
    //  testlib -v filename.cpp

    cout << "Lomse version " << LibraryScope::get_version_long_string()
         << ". Library tests runner." << endl << endl;

    cout << "Lomse build date: " << LibraryScope::get_build_date() << endl;
    cout << "Path for tests scores: '" << TESTLIB_SCORES_PATH << "'" << endl << endl;

    bool verbose = argc > 1 && (!strcmp(argv[1], "--verbose") || !strcmp(argv[1], "-v"));
    int nextArg = verbose ? 2 : 1;

    MyTestReporterStdout reporter(verbose);
    UnitTest::TestRunner runner(reporter);

    int nErrors = 0;

    if (argc > nextArg)
    {
        cout << "Running some tests " << endl << endl;

        //run only matching tests
        nErrors = runner.RunTestsIf(Test::GetTestList(), nullptr,
            [nextArg, argc, argv](Test* p)
            {
                bool hasPositiveArgs = false;
                bool positiveMatch = false;
                bool negativeMatch = false;
                for (int i = nextArg ; i < argc ; ++i)
                {
                    const char* arg = argv[i];
                    bool negative = arg[0] == '-';
                    negativeMatch |= negative && TestMatches(p->m_details, arg + 1);
                    positiveMatch |= !negative && TestMatches(p->m_details, arg);
                    hasPositiveArgs |= !negative;
                }
                return (!hasPositiveArgs || positiveMatch) && !negativeMatch;
            }
            , 0);
    }
    else
    {
        cout << "Running all tests" << endl << endl;
        nErrors = runner.RunTestsIf(Test::GetTestList(), nullptr, True(), 0);
    }

    #if defined WIN32 || defined _WIN32
        _CrtDumpMemoryLeaks();
    #endif

    return nErrors;
}

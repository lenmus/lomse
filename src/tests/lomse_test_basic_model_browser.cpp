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

#ifdef _LM_DEBUG_

#include <UnitTest++.h>
#include <sstream>

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_basic_model.h"
#include "lomse_basic_model_browser.h"
#include "lomse_compiler.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


class TestVisitor : public ImObjVisitor
{
public:
    TestVisitor() : ImObjVisitor() {}
	~TestVisitor() {}

	void start_visit(ImObj* pImo) {}
	void end_visit(ImObj* pImo) {}

	std::string& get_result() { return m_result; }

protected:
    std::string m_result;

};


class BasicModelBrowserTestFixture
{
public:

    BasicModelBrowserTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_scores_path = "../../../test-scores/";        //linux CodeBlobks
        //m_scores_path = "../../../../test-scores/";        //windows MS Visual studio .NET
    }

    ~BasicModelBrowserTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

SUITE(BasicModelBrowserTest)
{
    TEST_FIXTURE(BasicModelBrowserTestFixture, LdpCompilerCreateEmpty)
    {
        DocumentScope documentScope(cout);
        LdpCompiler compiler(*m_pLibraryScope, documentScope);
        BasicModel* pBasicModel = compiler.compile_file(m_scores_path + "00011-empty-fill-page.lms");

        TestVisitor v;
        BasicModelBrowser browser(v);
        browser.browse(pBasicModel->get_root());
        cout << v.get_result() << endl;
        CHECK( v.get_result() == "" );
        LdpTree* tree = compiler.get_final_tree();
        CHECK( tree != NULL );
        LdpElement* root = tree->get_root();
        //cout << root->to_string() << endl;
        CHECK( root->to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData )))))" );
        delete root;
        delete tree;
        delete pBasicModel;
    }

};

#endif  // _LM_DEBUG_


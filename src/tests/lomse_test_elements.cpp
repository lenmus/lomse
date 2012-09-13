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
#include "lomse_internal_model.h"
#include "lomse_ldp_elements.h"
#include "lomse_ldp_factory.h"

#include <exception>

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LdpElementsTestFixture
{
public:

    LdpElementsTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = LOMSE_NEW LibraryScope(cout);
        m_pLdpFactory = m_pLibraryScope->ldp_factory();
        m_pLibraryScope->set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~LdpElementsTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    LdpFactory* m_pLdpFactory;
};

SUITE(LdpElementsTest)
{
    TEST_FIXTURE(LdpElementsTestFixture, CanCreateElementFromName)
    {
        LdpElement* clef = m_pLdpFactory->create("clef");
        CHECK( clef->get_type() == k_clef );
        CHECK( clef->get_name() == "clef" );
        delete clef;
    }

    TEST_FIXTURE(LdpElementsTestFixture, CanCreateElementFromType)
    {
        LdpElement* clef = m_pLdpFactory->create(k_clef);
        CHECK( clef->get_type() == k_clef );
        CHECK( clef->get_name() == "clef" );
        delete clef;
    }

    TEST_FIXTURE(LdpElementsTestFixture, InvalidElementName)
    {
        LdpElement* clef = m_pLdpFactory->create("invalid");
        CHECK( clef->get_type() == k_undefined );
        CHECK( clef->get_name() == "undefined" );
        delete clef;
    }

    TEST_FIXTURE(LdpElementsTestFixture, CanAddSimpleSubElements)
    {
        LdpElement* note = m_pLdpFactory->create("n");
        note->append_child( m_pLdpFactory->new_value(k_pitch, "c4") );
        //cout << note->to_string() << endl;
        CHECK( note->to_string() == "(n c4)" );
        delete note;
    }

    TEST_FIXTURE(LdpElementsTestFixture, CanAddCompositeSubElements)
    {
        LdpElement* note = m_pLdpFactory->create("n");
        note->append_child( m_pLdpFactory->new_value(k_pitch, "c4") );
        note->append_child( m_pLdpFactory->new_value(k_duration, "q") );
        note->append_child( m_pLdpFactory->new_element(k_stem, m_pLdpFactory->new_label("up")) );
        //cout << note->to_string() << endl;
        CHECK( note->to_string() == "(n c4 q (stem up))" );
        delete note;
    }

    TEST_FIXTURE(LdpElementsTestFixture, CanAddCompositeWithManySubElements)
    {
        LdpElement* note = m_pLdpFactory->create("n");
        note->append_child( m_pLdpFactory->new_value(k_pitch, "c4") );
        note->append_child( m_pLdpFactory->new_value(k_duration, "q") );
        note->append_child( m_pLdpFactory->new_element(k_stem, m_pLdpFactory->new_label("up")) );
        LdpElement* text = m_pLdpFactory->create("text");
        text->append_child( m_pLdpFactory->new_string("This is a text") );
        text->append_child( m_pLdpFactory->new_element(k_dx, m_pLdpFactory->new_number("12")) );
        text->append_child( m_pLdpFactory->new_element(k_dy, m_pLdpFactory->new_number("20.5")) );
        note->append_child(text);
        //cout << note->to_string() << endl;
        CHECK( note->to_string() == "(n c4 q (stem up) (text \"This is a text\" (dx 12) (dy 20.5)))" );
        delete note;
    }

    TEST_FIXTURE(LdpElementsTestFixture, FactoryReturnsName)
    {
        const std::string& name = m_pLdpFactory->get_name(k_score);
        CHECK( name == "score" );
    }

    TEST_FIXTURE(LdpElementsTestFixture, FactoryGetNameThrowsException)
    {
        bool fOk = false;
        try
        {
            m_pLdpFactory->get_name(static_cast<ELdpElement>(99999));
        }
        catch(std::exception& e)
        {
            //cout << e.what() << endl;
            e.what();
            fOk = true;
        }
        CHECK( fOk );
    }

    TEST_FIXTURE(LdpElementsTestFixture, LdpElementsGetFloat)
    {
        LdpElement* pNum = m_pLdpFactory->new_value(k_number, "128.3");
        //cout << note->to_string() << endl;
        CHECK( pNum->to_string() == "128.3" );
        CHECK( pNum->get_value_as_float() == 128.3f );
        delete pNum;
    }

    TEST_FIXTURE(LdpElementsTestFixture, LdpElementsGetFloatFail)
    {
        bool fOk = false;
        LdpElement* pNum = NULL;
        try
        {
            pNum = m_pLdpFactory->new_value(k_number, "abc");
            pNum->get_value_as_float();
        }
        catch(std::exception& e)
        {
            //cout << e.what() << endl;
            e.what();
            fOk = true;
        }
        CHECK( pNum != NULL );
        CHECK( fOk );
        delete pNum;
    }

}

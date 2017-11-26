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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include <list>
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_shape_note.h"
#include "lomse_glyphs.h"
#include "lomse_observable.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
// for testing Linkable class
class MyShape : public Linkable<int>
{
public:
    Linkable<int>* m_refLinkedTo;
    Linkable<int>* m_refHandle;
    int m_typeLinkedTo;
    int m_typeHandle;


    MyShape() : Linkable<int>(), m_refLinkedTo(nullptr), m_refHandle(nullptr)
              , m_typeLinkedTo(-1), m_typeHandle(-1)
    {}

    void handle_link_event(Linkable<int>* ref, int type, int UNUSED(data)) {
        m_refHandle = ref;
        m_typeHandle = type;
    }

    void on_linked_to(Linkable<int>* ref, int type) {
        m_refLinkedTo = ref;
        m_typeLinkedTo = type;
    }

    void do_notify_observers() { notify_linked_observers(25); }

    //for tests
    int get_num_links_from() { return int(m_linkedFrom.size()); }
    int get_num_links_to() { return int(m_linkedTo.size()); }

};

//---------------------------------------------------------------------------------------
class LinkableTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    LinkableTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~LinkableTestFixture()    //TearDown fixture
    {
    }

};

//---------------------------------------------------------------------------------------
SUITE(LinkableTest)
{

    TEST_FIXTURE(LinkableTestFixture, MasterSlaveDoubleLinked)
    {
        MyShape master;
        MyShape slave;
        master.accept_link_from(&slave, 7);

        CHECK( master.get_num_links_from() == 1 );
        CHECK( master.get_num_links_to() == 0 );
        CHECK( slave.get_num_links_from() == 0 );
        CHECK( slave.get_num_links_to() == 1 );
    }

    TEST_FIXTURE(LinkableTestFixture, RefToMasterRemoved)
    {
        MyShape* pMaster = LOMSE_NEW MyShape();
        MyShape* pSlave = LOMSE_NEW MyShape();
        pMaster->accept_link_from(pSlave, 7);
        CHECK( pSlave->get_num_links_to() == 1 );

        delete pMaster;
        CHECK( pSlave->get_num_links_to() == 0 );

        delete pSlave;
    }

    TEST_FIXTURE(LinkableTestFixture, RefToSlaveRemoved)
    {
        MyShape* pMaster = LOMSE_NEW MyShape();
        MyShape* pSlave = LOMSE_NEW MyShape();
        pMaster->accept_link_from(pSlave, 7);
        CHECK( pMaster->get_num_links_from() == 1 );

        delete pSlave;
        CHECK( pMaster->get_num_links_from() == 0 );

        delete pMaster;
    }

    TEST_FIXTURE(LinkableTestFixture, ChainDeleteMaster)
    {
        MyShape* pNote = LOMSE_NEW MyShape();
        MyShape* pFermata = LOMSE_NEW MyShape();
        MyShape* pText = LOMSE_NEW MyShape();
        pNote->accept_link_from(pFermata, 7);
        pFermata->accept_link_from(pText, 6);
        CHECK( pNote->get_num_links_from() == 1 );
        CHECK( pFermata->get_num_links_from() == 1 );
        CHECK( pText->get_num_links_from() == 0 );
        CHECK( pNote->get_num_links_to() == 0 );
        CHECK( pFermata->get_num_links_to() == 1 );
        CHECK( pText->get_num_links_to() == 1 );

        delete pNote;
        CHECK( pFermata->get_num_links_from() == 1 );
        CHECK( pText->get_num_links_from() == 0 );
        CHECK( pFermata->get_num_links_to() == 0 );
        CHECK( pText->get_num_links_to() == 1 );

        delete pFermata;
        delete pText;
    }

    TEST_FIXTURE(LinkableTestFixture, ChainDeleteMasterSlave)
    {
        MyShape* pNote = LOMSE_NEW MyShape();
        MyShape* pFermata = LOMSE_NEW MyShape();
        MyShape* pText = LOMSE_NEW MyShape();
        pNote->accept_link_from(pFermata, 7);
        pFermata->accept_link_from(pText, 6);

        delete pFermata;
        CHECK( pNote->get_num_links_from() == 0 );
        CHECK( pText->get_num_links_from() == 0 );
        CHECK( pNote->get_num_links_to() == 0 );
        CHECK( pText->get_num_links_to() == 0 );

        delete pNote;
        delete pText;
    }

    TEST_FIXTURE(LinkableTestFixture, ChainDeleteSlave)
    {
        MyShape* pNote = LOMSE_NEW MyShape();
        MyShape* pFermata = LOMSE_NEW MyShape();
        MyShape* pText = LOMSE_NEW MyShape();
        pNote->accept_link_from(pFermata, 7);
        pFermata->accept_link_from(pText, 6);

        delete pText;
        CHECK( pNote->get_num_links_from() == 1 );
        CHECK( pFermata->get_num_links_from() == 0 );
        CHECK( pNote->get_num_links_to() == 0 );
        CHECK( pFermata->get_num_links_to() == 1 );

        delete pNote;
        delete pFermata;
    }

    TEST_FIXTURE(LinkableTestFixture, Slave_OnLinkedTo)
    {
        MyShape master;
        MyShape slave;

        master.accept_link_from(&slave, 7);

        CHECK( slave.m_refLinkedTo == &master );
        CHECK( slave.m_typeLinkedTo == 7 );
    }

    TEST_FIXTURE(LinkableTestFixture, SlaveIsNotified)
    {
        MyShape master;
        MyShape slave;
        master.accept_link_from(&slave, 7);

        master.do_notify_observers();

        CHECK( slave.m_refHandle == &master );
        CHECK( slave.m_typeHandle == 7 );
    }

    TEST_FIXTURE(LinkableTestFixture, SlaveIsRemoved)
    {
        MyShape master;
        MyShape slave;
        master.accept_link_from(&slave, 7);
        CHECK( master.get_num_links_from() == 1 );

        master.remove_link_from(&slave);

        CHECK( master.get_num_links_from() == 0 );
    }

}



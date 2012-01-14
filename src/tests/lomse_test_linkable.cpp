//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
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


    MyShape() : Linkable<int>(), m_refLinkedTo(NULL), m_refHandle(NULL)
              , m_typeLinkedTo(-1), m_typeHandle(-1)
    {}

    void handle_link_event(Linkable<int>* ref, int type, int data) {
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
        m_scores_path = LOMSE_TEST_SCORES_PATH;
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



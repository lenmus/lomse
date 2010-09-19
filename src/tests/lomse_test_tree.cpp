//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifdef _LM_DEBUG_

#include <iostream>
#include <UnitTest++.h>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_tree.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


class Element : virtual public NodeInTree<Element>
{
public:
    Element(const std::string& v) : m_value(v) {}

    std::string m_value;
};


class TreeTestFixture
{
public:

    TreeTestFixture()     //SetUp fixture
    {
    }

    ~TreeTestFixture()    //TearDown fixture
    {
    }

    Tree<Element> m_tree;
    Element* a;
    Element* b;
    Element* c;
    Element* d;
    Element* e;
    Element* f;
    Element* g;
    Element* h;
    Element* i;
    Element* j;
    Element* k;
    Element* l;
    Element* m;
    Element* n;
    Element* o;
    Element* p;
    Element* q;
    Element* r;
    Element* s;
    Element* t;

//  A ---+--- B ------- C
//       |
//       +--- D ---+--- E ---+--- F
//       |         |         +--- G
//       |         +--- H
//       |
//       +--- I ---+--- J ---+--- K
//                 |         +--- L ---+--- M
//                 |                   +--- N
//                 +--- O
//                 +--- P ---+--- Q
//                           +--- R ------- S
//                           +--- T
    void CreateTree()
    {
        a = new Element("A");
        b = new Element("B");
        c = new Element("C");
        d = new Element("D");
        e = new Element("E");
        f = new Element("F");
        g = new Element("G");
        h = new Element("H");
        i = new Element("I");
        j = new Element("J");
        k = new Element("K");
        l = new Element("L");
        m = new Element("M");
        n = new Element("N");
        o = new Element("O");
        p = new Element("P");
        q = new Element("Q");
        r = new Element("R");
        s = new Element("S");
        t = new Element("T");
        m_tree.set_root(a);
        a->append_child(b);
        b->append_child(c);
        a->append_child(d);
        d->append_child(e);
        e->append_child(f);
        e->append_child(g);
        d->append_child(h);
        a->append_child(i);
        i->append_child(j);
        j->append_child(k);
        j->append_child(l);
        l->append_child(m);
        l->append_child(n);
        i->append_child(o);
        i->append_child(p);
        p->append_child(q);
        p->append_child(r);
        r->append_child(s);
        p->append_child(t);
    }

    void DeleteTestData()
    {
        delete a;
        delete b;
        delete c;
        delete d;
        delete e;
        delete f;
        delete g;
        delete h;
        delete i;
        delete j;
        delete k;
        delete l;
        delete m;
        delete n;
        delete o;
        delete p;
        delete q;
        delete r;
        delete s;
        delete t;
    }

};

SUITE(TreeTest)
{
    TEST_FIXTURE(TreeTestFixture, TreeCreateTree)
    {
        Tree<Element> tree;
        Element root("animal");
        tree.set_root(&root);
        CHECK( root.is_root() );
        CHECK( root.m_value == "animal" );
    }

    TEST_FIXTURE(TreeTestFixture, TreeCreateTreeFromNode)
    {
        Element root("animal");
        Tree<Element> tree(&root);
        CHECK( root.is_root() );
        CHECK( root.m_value == "animal" );
    }

    TEST_FIXTURE(TreeTestFixture, TreeAppendChild)
    {
        Element root("animal");
        Tree<Element> tree(&root);
        Element two("mammal");
        root.append_child(&two);
        CHECK( root.get_num_children() == 1 );
    }

    TEST_FIXTURE(TreeTestFixture, TreeBeginChildren)
    {
        Element root("animal");
        Tree<Element> tree(&root);
        Element two("mammal");
        root.append_child(&two);
        NodeInTree<Element>::children_iterator it = root.begin();
        CHECK( (*it)->m_value == "mammal" );
    }

    TEST_FIXTURE(TreeTestFixture, TreeGetNumChildren)
    {
        Element root("animal");
        Element two1("mammal");
        root.append_child(&two1);
        Element two2("bird");
        root.append_child(&two2);
        Element two3("fish");
        root.append_child(&two3);
        CHECK( root.get_num_children() == 3 );
        CHECK( two3.get_num_children() == 0 );
    }

    TEST_FIXTURE(TreeTestFixture, TreeStartChildrenIteratorInGivenNode)
    {
        Element root("animal");
        Element two1("mammal");
        root.append_child(&two1);
        Element two2("bird");
        root.append_child(&two2);
        Element two3("fish");
        NodeInTree<Element>::children_iterator it(&two2);
        CHECK( (*it)->m_value == "bird" );
    }

    TEST_FIXTURE(TreeTestFixture, TreeGetChild)
    {
        Element root("animal");
        Element two1("mammal");
        root.append_child(&two1);
        Element two2("bird");
        root.append_child(&two2);
        Element two3("fish");
        CHECK( root.get_child(1)->m_value == "bird" );
    }

    TEST_FIXTURE(TreeTestFixture, TreeGetChildOutOfRange)
    {
        Element root("animal");
        Element two1("mammal");
        root.append_child(&two1);
        Element two2("bird");
        root.append_child(&two2);
        Element two3("fish");
        bool fOk = false;
        try {
            root.get_child(4);
        }
        catch(exception& e)
        {
            e.what();       //compiler happy
            //cout << e.what() << endl;
            fOk = true;
        }

        CHECK( fOk );
    }

    TEST_FIXTURE(TreeTestFixture, TreeDepthFirstTraversal)
    {
        CreateTree();

        stringstream path;
        Tree<Element>::depth_first_iterator it;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCDEFGHIJKLMNOPQRST" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeIsBuiltAgain)
    {
        //checks that the test tree can be re-built again witout problems
        CreateTree();

        stringstream path;
        Tree<Element>::depth_first_iterator it;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCDEFGHIJKLMNOPQRST" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeEraseNodeAtMiddle)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        ++it;   //C
        ++it;   //D
        Tree<Element>::depth_first_iterator itNext = m_tree.erase(it);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCIJKLMNOPQRST" );
        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode->m_value == "I" );
        Element* parent = curNode->get_parent();
        CHECK( parent->m_value == "A" );
        CHECK( parent->get_first_child()->m_value == "B" );
        CHECK( parent->get_last_child()->m_value == "I" );
        Element* prevSibling = curNode->get_prev_sibling();
        Element* nextSibling = curNode->get_next_sibling();
        CHECK( prevSibling->m_value == "B" );
        CHECK( nextSibling == NULL );
        CHECK( prevSibling->get_next_sibling()->m_value == "I" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeEraseNodeAtStart)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        Tree<Element>::depth_first_iterator itNext = m_tree.erase(it);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ADEFGHIJKLMNOPQRST" );
        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode->m_value == "D" );
        Element* parent = curNode->get_parent();
        CHECK( parent->m_value == "A" );
        CHECK( parent->get_first_child()->m_value == "D" );
        CHECK( parent->get_last_child()->m_value == "I" );
        Element* prevSibling = curNode->get_prev_sibling();
        Element* nextSibling = curNode->get_next_sibling();
        CHECK( prevSibling == NULL );
        CHECK( nextSibling->m_value == "I" );
        //CHECK( prevSibling->get_next_sibling()->m_value == "I" );
        CHECK( nextSibling->get_prev_sibling()->m_value == "D" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeEraseNodeAtEnd)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        ++it;   //C
        ++it;   //D
        ++it;   //E
        ++it;   //F
        ++it;   //G
        ++it;   //H
        ++it;   //I
        Element* parent = (*it)->get_parent();
        Tree<Element>::depth_first_iterator itNext = m_tree.erase(it);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCDEFGH" );
        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode == NULL );
        CHECK( parent->m_value == "A" );
        CHECK( parent->get_first_child()->m_value == "B" );
        Element* lastChild = parent->get_last_child();
        CHECK( lastChild->m_value == "D" );
        CHECK( lastChild->get_next_sibling() == NULL );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeEraseTerminalNode)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        ++it;   //C
        ++it;   //D
        ++it;   //E
        ++it;   //F
        ++it;   //G
        Tree<Element>::depth_first_iterator itNext = m_tree.erase(it);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCDEFHIJKLMNOPQRST" );
        //cout << (*itNext).m_value << endl;
        CHECK( (*itNext)->m_value == "H" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeEraseWholeTree)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        Tree<Element>::depth_first_iterator itNext = m_tree.erase(it);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "" );
        CHECK( itNext == m_tree.end() );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeReplaceNodeAtMiddle)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        ++it;   //C
        ++it;   //D
        Element elm("Z");
        Tree<Element>::depth_first_iterator itNext = m_tree.replace_node(it, &elm);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCZIJKLMNOPQRST" );
        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode->m_value == "Z" );
        Element* parent = curNode->get_parent();
        CHECK( parent->m_value == "A" );
        CHECK( parent->get_first_child()->m_value == "B" );
        CHECK( parent->get_last_child()->m_value == "I" );
        Element* prevSibling = curNode->get_prev_sibling();
        Element* nextSibling = curNode->get_next_sibling();
        CHECK( prevSibling->m_value == "B" );
        CHECK( nextSibling->m_value == "I" );
        CHECK( prevSibling->get_next_sibling()->m_value == "Z" );
        CHECK( nextSibling->get_prev_sibling()->m_value == "Z" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeReplaceNodeAtStart)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        Element elm("Z");
        Tree<Element>::depth_first_iterator itNext = m_tree.replace_node(it, &elm);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "AZDEFGHIJKLMNOPQRST" );
        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode->m_value == "Z" );
        Element* parent = curNode->get_parent();
        CHECK( parent->m_value == "A" );
        CHECK( parent->get_first_child()->m_value == "Z" );
        CHECK( parent->get_last_child()->m_value == "I" );
        Element* prevSibling = curNode->get_prev_sibling();
        Element* nextSibling = curNode->get_next_sibling();
        CHECK( prevSibling == NULL );
        CHECK( nextSibling->m_value == "D" );
        CHECK( nextSibling->get_prev_sibling()->m_value == "Z" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeReplaceNodeAtEnd)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        ++it;   //C
        ++it;   //D
        ++it;   //E
        ++it;   //F
        ++it;   //G
        ++it;   //H
        ++it;   //I
        Element elm("Z");
        Tree<Element>::depth_first_iterator itNext = m_tree.replace_node(it, &elm);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCDEFGHZ" );
        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode->m_value == "Z" );
        Element* parent = curNode->get_parent();
        CHECK( parent->m_value == "A" );
        CHECK( parent->get_first_child()->m_value == "B" );
        CHECK( parent->get_last_child()->m_value == "Z" );
        Element* prevSibling = curNode->get_prev_sibling();
        Element* nextSibling = curNode->get_next_sibling();
        CHECK( prevSibling->m_value == "D" );
        CHECK( nextSibling == NULL );
        CHECK( prevSibling->get_next_sibling()->m_value == "Z" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeReplaceWholeTree)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        Element elm("(ALL)");
        Tree<Element>::depth_first_iterator itNext = m_tree.replace_node(it, &elm);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "(ALL)" );
        //cout << (*itNext).m_value << endl;
        CHECK( (*itNext)->m_value == "(ALL)" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeInsertNodeAtMiddle)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        ++it;   //C
        ++it;   //D
        Element elm("(NEW)");
        Tree<Element>::depth_first_iterator itNext = m_tree.insert(it, &elm);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABC(NEW)DEFGHIJKLMNOPQRST" );
        Element* newNode = *itNext;
        //cout << newNode->m_value << endl;
        CHECK( newNode->m_value == "(NEW)" );
        Element* parent = newNode->get_parent();
        CHECK( parent->m_value == "A" );
        CHECK( parent->get_first_child()->m_value == "B" );
        CHECK( parent->get_last_child()->m_value == "I" );
        Element* prevSibling = newNode->get_prev_sibling();
        Element* nextSibling = newNode->get_next_sibling();
        CHECK( prevSibling->m_value == "B" );
        CHECK( nextSibling->m_value == "D" );
        CHECK( prevSibling->get_next_sibling()->m_value == "(NEW)" );
        CHECK( nextSibling->get_prev_sibling()->m_value == "(NEW)" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeInsertNodeAtStart)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        Element elm("(NEW)");
        Tree<Element>::depth_first_iterator itNext = m_tree.insert(it, &elm);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "A(NEW)BCDEFGHIJKLMNOPQRST" );
        Element* newNode = *itNext;
        //cout << newNode->m_value << endl;
        CHECK( newNode->m_value == "(NEW)" );
        Element* parent = newNode->get_parent();
        CHECK( parent->m_value == "A" );
        CHECK( parent->get_first_child()->m_value == "(NEW)" );
        CHECK( parent->get_last_child()->m_value == "I" );
        Element* prevSibling = newNode->get_prev_sibling();
        Element* nextSibling = newNode->get_next_sibling();
        CHECK( prevSibling == NULL );
        CHECK( nextSibling->m_value == "B" );
        CHECK( nextSibling->get_prev_sibling()->m_value == "(NEW)" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeInsertNodeAtEnd)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        ++it;   //C
        ++it;   //D
        ++it;   //E
        ++it;   //F
        ++it;   //G
        ++it;   //H
        ++it;   //I
        Element elm("(NEW)");
        Tree<Element>::depth_first_iterator itNext = m_tree.insert(it, &elm);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCDEFGH(NEW)IJKLMNOPQRST" );
        Element* newNode = *itNext;
        //cout << newNode->m_value << endl;
        CHECK( newNode->m_value == "(NEW)" );
        Element* parent = newNode->get_parent();
        CHECK( parent->m_value == "A" );
        CHECK( parent->get_first_child()->m_value == "B" );
        CHECK( parent->get_last_child()->m_value == "I" );
        Element* prevSibling = newNode->get_prev_sibling();
        Element* nextSibling = newNode->get_next_sibling();
        CHECK( prevSibling->m_value == "D" );
        CHECK( nextSibling->m_value == "I" );
        CHECK( nextSibling->get_prev_sibling()->m_value == "(NEW)" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeSetIterator)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it;
        stringstream path;
        for (it=d; it != d->get_next_sibling(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "DEFGH" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeTraverseNode)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it(d);
        stringstream path;
        for (; (*it) != d->get_next_sibling(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "DEFGH" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeAtEnd)
    {
        //advance beyond the last element it is equal end()
        CreateTree();
        Tree<Element>::depth_first_iterator it(t);
        ++it;
        CHECK( it == m_tree.end() );
        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeDecrementEnd)
    {
        //decrement end() moves to last element.
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.end();
        --it;
        CHECK( (*it)->m_value == "T" );
        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeDecrementBeginMovesToEnd)
    {
        //decrement begin() moves to end()
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        --it;
        CHECK( it == m_tree.end() );
        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeIncrementEndRemainsAtEnd)
    {
        //increment end() remains at end()
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.end();
        ++it;
        CHECK( it == m_tree.end() );
        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeNodeDefaultIsNotModified)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();    //A
        CHECK( !(*it)->is_modified() );
        ++it;   //B
        CHECK( !(*it)->is_modified() );
        ++it;   //C
        CHECK( !(*it)->is_modified() );
        ++it;   //D
        CHECK( !(*it)->is_modified() );
        ++it;   //E
        CHECK( !(*it)->is_modified() );
        ++it;   //F
        CHECK( !(*it)->is_modified() );
        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeNodeSetModified)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();    //A
        ++it;   //B
        ++it;   //C
        ++it;   //D
        ++it;   //E
        ++it;   //F
        (*it)->set_modified();
        it = m_tree.begin();    //A
        CHECK( (*it)->is_modified() );
        ++it;   //B
        CHECK( !(*it)->is_modified() );
        ++it;   //C
        CHECK( !(*it)->is_modified() );
        ++it;   //D
        CHECK( (*it)->is_modified() );
        ++it;   //E
        CHECK( (*it)->is_modified() );
        ++it;   //F
        CHECK( (*it)->is_modified() );
        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeNodeResetModified)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();    //A
        ++it;   //B
        ++it;   //C
        ++it;   //D
        ++it;   //E
        (*it)->set_modified();
        ++it;   //F
        (*it)->set_modified();
        it = m_tree.begin();    //A
        CHECK( (*it)->is_modified() );
        ++it;   //B
        CHECK( !(*it)->is_modified() );
        ++it;   //C
        CHECK( !(*it)->is_modified() );
        ++it;   //D
        CHECK( (*it)->is_modified() );
        ++it;   //E
        CHECK( (*it)->is_modified() );
        ++it;   //F
        CHECK( (*it)->is_modified() );
        (*it)->reset_modified();
        it = m_tree.begin();    //A
        CHECK( (*it)->is_modified() );
        ++it;   //B
        CHECK( !(*it)->is_modified() );
        ++it;   //C
        CHECK( !(*it)->is_modified() );
        ++it;   //D
        CHECK( (*it)->is_modified() );
        ++it;   //E
        CHECK( (*it)->is_modified() );
        ++it;   //F
        CHECK( !(*it)->is_modified() );
        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeClearModified)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();    //A
        ++it;   //B
        ++it;   //C
        ++it;   //D
        ++it;   //E
        (*it)->set_modified();
        ++it;   //F
        (*it)->set_modified();
        it = m_tree.begin();    //A
        CHECK( (*it)->is_modified() );
        ++it;   //B
        CHECK( !(*it)->is_modified() );
        ++it;   //C
        CHECK( !(*it)->is_modified() );
        ++it;   //D
        CHECK( (*it)->is_modified() );
        ++it;   //E
        CHECK( (*it)->is_modified() );
        ++it;   //F
        CHECK( (*it)->is_modified() );
        m_tree.clear_modified();
        it = m_tree.begin();    //A
        CHECK( !(*it)->is_modified() );
        ++it;   //B
        CHECK( !(*it)->is_modified() );
        ++it;   //C
        CHECK( !(*it)->is_modified() );
        ++it;   //D
        CHECK( !(*it)->is_modified() );
        ++it;   //E
        CHECK( !(*it)->is_modified() );
        ++it;   //F
        CHECK( !(*it)->is_modified() );
        DeleteTestData();
    }

}

#endif  // _LM_DEBUG_

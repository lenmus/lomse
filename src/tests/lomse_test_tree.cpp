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
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_tree.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


class Element : virtual public TreeNode<Element>
{
public:
    Element(const std::string& v) : m_value(v) {}
    virtual ~Element() {}

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
        a = LOMSE_NEW Element("A");
        b = LOMSE_NEW Element("B");
        c = LOMSE_NEW Element("C");
        d = LOMSE_NEW Element("D");
        e = LOMSE_NEW Element("E");
        f = LOMSE_NEW Element("F");
        g = LOMSE_NEW Element("G");
        h = LOMSE_NEW Element("H");
        i = LOMSE_NEW Element("I");
        j = LOMSE_NEW Element("J");
        k = LOMSE_NEW Element("K");
        l = LOMSE_NEW Element("L");
        m = LOMSE_NEW Element("M");
        n = LOMSE_NEW Element("N");
        o = LOMSE_NEW Element("O");
        p = LOMSE_NEW Element("P");
        q = LOMSE_NEW Element("Q");
        r = LOMSE_NEW Element("R");
        s = LOMSE_NEW Element("S");
        t = LOMSE_NEW Element("T");
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
        TreeNode<Element>::children_iterator it = root.begin();
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
        TreeNode<Element>::children_iterator it(&two2);
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

    TEST_FIXTURE(TreeTestFixture, TreeInsertNodeAtMiddle_no_iterator)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        ++it;   //C
        ++it;   //D
        Element elm("(NEW)");

        Tree<Element>::depth_first_iterator itNext = m_tree.insert(*it, &elm);

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

    TEST_FIXTURE(TreeTestFixture, TreeInsertNodeAtStart_no_iterator)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        Element elm("(NEW)");

        Tree<Element>::depth_first_iterator itNext = m_tree.insert(*it, &elm);

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

    TEST_FIXTURE(TreeTestFixture, TreeInsertNodeAtEnd_no_iterator)
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

        Tree<Element>::depth_first_iterator itNext = m_tree.insert(*it, &elm);

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

    TEST_FIXTURE(TreeTestFixture, RemomeChildAtMiddle)
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
        Element* curNode = *it;
        curNode->remove_child(o);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCDEFGHIJKLMNPQRST" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, RemomeChildAtStart)
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
        Element* curNode = *it;
        curNode->remove_child(j);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCDEFGHIOPQRST" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, RemoveChildAtEnd)
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
        Element* curNode = *it;
        curNode->remove_child(p);

        stringstream path;
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCDEFGHIJKLMNO" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, RemoveLastChildAnddAppendNew)
    {
        Element* a = LOMSE_NEW Element("A");
        Element* b = LOMSE_NEW Element("B");
        Element* c = LOMSE_NEW Element("C");
        Element* d = LOMSE_NEW Element("D");
        Element* z = LOMSE_NEW Element("Z");

        m_tree.set_root(a);
        a->append_child(b);
        a->append_child(c);
        a->append_child(d);

        a->remove_child(d);
        a->append_child(z);

        stringstream path;
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        for (it=m_tree.begin(); it != m_tree.end(); ++it)
            path << (*it)->m_value;
        //cout << path.str() << endl;
        CHECK( path.str() == "ABCZ" );

        delete a;
        delete b;
        delete c;
        delete d;
        delete z;
    }

}

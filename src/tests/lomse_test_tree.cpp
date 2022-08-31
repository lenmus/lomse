//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include <iostream>
#include <UnitTest++.h>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_tree.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


class Element : public TreeNode<Element>
{
public:
    Element(const std::string& v) : TreeNode<Element>(), m_value(v) {}
    virtual ~Element() {}
    Element(const Element& x) : TreeNode<Element>(), m_value(x.m_value) {}

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
    Element* a1;
    Element* b1;
    Element* c1;
    Element* d;
    Element* e1;    //renamed as 'e1' to avoid warning "declaration of ‘e’ shadows a member of ‘SuiteTreeTest ...
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

//                      A
//                      |
//    B --------------- D --------------------- I
//    |                 |                       |
//    C          E ------------ H        J ---- O ---- P
//               |                       |             |
//            F --- G                 K --- L     Q -- R -- T
//                                          |          |
//                                       M --- N       S

    void CreateTree()
    {
        a1 = LOMSE_NEW Element("A");
        b1 = LOMSE_NEW Element("B");
        c1 = LOMSE_NEW Element("C");
        d = LOMSE_NEW Element("D");
        e1 = LOMSE_NEW Element("E");
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
        m_tree.set_root(a1);
        a1->append_child(b1);
        b1->append_child(c1);
        a1->append_child(d);
        d->append_child(e1);
        e1->append_child(f);
        e1->append_child(g);
        d->append_child(h);
        a1->append_child(i);
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
        delete a1;
        delete b1;
        delete c1;
        delete d;
        delete e1;
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

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    bool check_path(Tree<Element>& tree, const string& expected)
    {
        stringstream path;
        Tree<Element>::depth_first_iterator it;
        for (it=tree.begin(); it != tree.end(); ++it)
            path << (*it)->m_value;

        if (path.str() == expected)
            return true;

        cout << endl << test_name() << ":" << endl;
        cout << "      result=" << path.str() << endl;
        cout << "    expected=" << expected << endl;
        return false;
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

        string expected = "ABCDEFGHIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeIsBuiltAgain)
    {
        //checks that the test tree can be re-built again witout problems
        CreateTree();

        string expected = "ABCDEFGHIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

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

        string expected = "ABCIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode->m_value == "I" );
        Element* parent = curNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "B" );
        CHECK( parent && parent->get_last_child()->m_value == "I" );
        Element* prevSibling = curNode->get_prev_sibling();
        Element* nextSibling = curNode->get_next_sibling();
        CHECK( prevSibling && prevSibling->m_value == "B" );
        CHECK( nextSibling == nullptr );
        CHECK( prevSibling && prevSibling->get_next_sibling()->m_value == "I" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeEraseNodeAtStart)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        ++it;   //B
        Tree<Element>::depth_first_iterator itNext = m_tree.erase(it);

        string expected = "ADEFGHIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode->m_value == "D" );
        Element* parent = curNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "D" );
        CHECK( parent && parent->get_last_child()->m_value == "I" );
        Element* prevSibling = curNode->get_prev_sibling();
        Element* nextSibling = curNode->get_next_sibling();
        CHECK( prevSibling == nullptr );
        CHECK( nextSibling->m_value == "I" );
        //CHECK( prevSibling && prevSibling->get_next_sibling()->m_value == "I" );
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

        string expected = "ABCDEFGH";
        CHECK( check_path(m_tree, expected) );

        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode == nullptr );
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "B" );
        Element* lastChild = parent->get_last_child();
        CHECK( lastChild->m_value == "D" );
        CHECK( lastChild->get_next_sibling() == nullptr );

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

        string expected = "ABCDEFHIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        //cout << (*itNext).m_value << endl;
        CHECK( (*itNext)->m_value == "H" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeEraseWholeTree)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        Tree<Element>::depth_first_iterator itNext = m_tree.erase(it);

        string expected = "";
        CHECK( check_path(m_tree, expected) );

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

        string expected = "ABCZIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode->m_value == "Z" );
        Element* parent = curNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "B" );
        CHECK( parent && parent->get_last_child()->m_value == "I" );
        Element* prevSibling = curNode->get_prev_sibling();
        Element* nextSibling = curNode->get_next_sibling();
        CHECK( prevSibling && prevSibling->m_value == "B" );
        CHECK( nextSibling->m_value == "I" );
        CHECK( prevSibling && prevSibling->get_next_sibling()->m_value == "Z" );
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

        string expected = "AZDEFGHIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode->m_value == "Z" );
        Element* parent = curNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "Z" );
        CHECK( parent && parent->get_last_child()->m_value == "I" );
        Element* prevSibling = curNode->get_prev_sibling();
        Element* nextSibling = curNode->get_next_sibling();
        CHECK( prevSibling == nullptr );
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

        string expected = "ABCDEFGHZ";
        CHECK( check_path(m_tree, expected) );

        Element* curNode = *itNext;
        //cout << curNode->m_value << endl;
        CHECK( curNode->m_value == "Z" );
        Element* parent = curNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "B" );
        CHECK( parent && parent->get_last_child()->m_value == "Z" );
        Element* prevSibling = curNode->get_prev_sibling();
        Element* nextSibling = curNode->get_next_sibling();
        CHECK( prevSibling && prevSibling->m_value == "D" );
        CHECK( nextSibling == nullptr );
        CHECK( prevSibling && prevSibling->get_next_sibling()->m_value == "Z" );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, TreeReplaceWholeTree)
    {
        CreateTree();
        Tree<Element>::depth_first_iterator it = m_tree.begin();
        Element elm("(ALL)");
        Tree<Element>::depth_first_iterator itNext = m_tree.replace_node(it, &elm);

        string expected = "(ALL)";
        CHECK( check_path(m_tree, expected) );

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

        string expected = "ABC(NEW)DEFGHIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        Element* newNode = *itNext;
        //cout << newNode->m_value << endl;
        CHECK( newNode->m_value == "(NEW)" );
        Element* parent = newNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "B" );
        CHECK( parent && parent->get_last_child()->m_value == "I" );
        Element* prevSibling = newNode->get_prev_sibling();
        Element* nextSibling = newNode->get_next_sibling();
        CHECK( prevSibling && prevSibling->m_value == "B" );
        CHECK( nextSibling->m_value == "D" );
        CHECK( prevSibling && prevSibling->get_next_sibling()->m_value == "(NEW)" );
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

        string expected = "A(NEW)BCDEFGHIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        Element* newNode = *itNext;
        //cout << newNode->m_value << endl;
        CHECK( newNode->m_value == "(NEW)" );
        Element* parent = newNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "(NEW)" );
        CHECK( parent && parent->get_last_child()->m_value == "I" );
        Element* prevSibling = newNode->get_prev_sibling();
        Element* nextSibling = newNode->get_next_sibling();
        CHECK( prevSibling == nullptr );
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

        string expected = "ABCDEFGH(NEW)IJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        Element* newNode = *itNext;
        //cout << newNode->m_value << endl;
        CHECK( newNode->m_value == "(NEW)" );
        Element* parent = newNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "B" );
        CHECK( parent && parent->get_last_child()->m_value == "I" );
        Element* prevSibling = newNode->get_prev_sibling();
        Element* nextSibling = newNode->get_next_sibling();
        CHECK( prevSibling && prevSibling->m_value == "D" );
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

        string expected = "ABC(NEW)DEFGHIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        Element* newNode = *itNext;
        //cout << newNode->m_value << endl;
        CHECK( newNode->m_value == "(NEW)" );
        Element* parent = newNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "B" );
        CHECK( parent && parent->get_last_child()->m_value == "I" );
        Element* prevSibling = newNode->get_prev_sibling();
        Element* nextSibling = newNode->get_next_sibling();
        CHECK( prevSibling && prevSibling->m_value == "B" );
        CHECK( nextSibling->m_value == "D" );
        CHECK( prevSibling && prevSibling->get_next_sibling()->m_value == "(NEW)" );
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

        string expected = "A(NEW)BCDEFGHIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        Element* newNode = *itNext;
        //cout << newNode->m_value << endl;
        CHECK( newNode->m_value == "(NEW)" );
        Element* parent = newNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "(NEW)" );
        CHECK( parent && parent->get_last_child()->m_value == "I" );
        Element* prevSibling = newNode->get_prev_sibling();
        Element* nextSibling = newNode->get_next_sibling();
        CHECK( prevSibling == nullptr );
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

        string expected = "ABCDEFGH(NEW)IJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );

        Element* newNode = *itNext;
        //cout << newNode->m_value << endl;
        CHECK( newNode->m_value == "(NEW)" );
        Element* parent = newNode->get_parent();
        CHECK( parent && parent->m_value == "A" );
        CHECK( parent && parent->get_first_child()->m_value == "B" );
        CHECK( parent && parent->get_last_child()->m_value == "I" );
        Element* prevSibling = newNode->get_prev_sibling();
        Element* nextSibling = newNode->get_next_sibling();
        CHECK( prevSibling && prevSibling->m_value == "D" );
        CHECK( nextSibling->m_value == "I" );
        CHECK( nextSibling->get_prev_sibling()->m_value == "(NEW)" );

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

        string expected = "ABCDEFGHIJKLMNPQRST";
        CHECK( check_path(m_tree, expected) );

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

        string expected = "ABCDEFGHIOPQRST";
        CHECK( check_path(m_tree, expected) );

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

        string expected = "ABCDEFGHIJKLMNO";
        CHECK( check_path(m_tree, expected) );

        DeleteTestData();
    }

    TEST_FIXTURE(TreeTestFixture, RemoveLastChildAnddAppendNew)
    {
        Element* a2 = LOMSE_NEW Element("A");
        Element* b2 = LOMSE_NEW Element("B");
        Element* c2 = LOMSE_NEW Element("C");
        Element* d2 = LOMSE_NEW Element("D");
        Element* z2 = LOMSE_NEW Element("Z");

        m_tree.set_root(a2);
        a2->append_child(b2);
        a2->append_child(c2);
        a2->append_child(d2);

        a2->remove_child(d2);
        a2->append_child(z2);

        string expected = "ABCZ";
        CHECK( check_path(m_tree, expected) );

        delete a2;
        delete b2;
        delete c2;
        delete d2;
        delete z2;
    }

    TEST_FIXTURE(TreeTestFixture, tree_clone_01)
    {
        Element a2("A");
        Element b2("B");
        Element c2("C");
        Element d2("D");
        Element z2("Z");

        m_tree.set_root(&a2);
        a2.append_child(&b2);
        a2.append_child(&c2);
        a2.append_child(&d2);

        string expected = "ABCD";
        CHECK( check_path(m_tree, expected) );

        Tree<Element> dup(m_tree);

        CHECK (m_tree.get_root() != dup.get_root() );

        expected = "ABCD";
        CHECK( check_path(m_tree, expected) );
        CHECK( check_path(dup, expected) );

        //get the cloned nodes
        CHECK (dup.get_root() && dup.get_root()->m_value == "A" );
        Tree<Element>::depth_first_iterator it = dup.begin();
        Element* aa = *it;
        CHECK ( aa && aa->m_value == "A" );
        Element* bb = *(++it);
        CHECK ( bb && bb->m_value == "B" );
        Element* cc = *(++it);
        CHECK ( cc && cc->m_value == "C" );
        Element* dd = *(++it);
        CHECK ( dd && dd->m_value == "D" );

        dup.get_root()->remove_child(dd);
        dup.get_root()->append_child(&z2);

        expected = "ABCD";
        CHECK( check_path(m_tree, expected) );

        expected = "ABCZ";
        CHECK( check_path(dup, expected) );
    }

    TEST_FIXTURE(TreeTestFixture, tree_clone_02)
    {
        CreateTree();
        Tree<Element> dup(m_tree);

        CHECK (m_tree.get_root() != dup.get_root() );

        string expected = "ABCDEFGHIJKLMNOPQRST";
        CHECK( check_path(m_tree, expected) );
        CHECK( check_path(dup, expected) );

        Element z2("Z");
        m_tree.get_root()->remove_child(i);
        b1->remove_child(c1);
        dup.get_root()->append_child(&z2);

        expected = "ABDEFGH";
        CHECK( check_path(m_tree, expected) );
        expected = "ABCDEFGHIJKLMNOPQRSTZ";
        CHECK( check_path(dup, expected) );

        DeleteTestData();
    }

}

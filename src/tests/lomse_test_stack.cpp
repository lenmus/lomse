//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_stack.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;

typedef Stack<int*>  StackInt;

class StackTestFixture
{
public:

    StackTestFixture()     //SetUp fixture
    {
    }

    ~StackTestFixture()    //TearDown fixture
    {
    }
};

SUITE(StackTest)
{
    TEST_FIXTURE(StackTestFixture, StackIsEmpty)
    {
        StackInt stack;
        CHECK( stack.size() == 0 );
    }

    TEST_FIXTURE(StackTestFixture, StackPush)
    {
        StackInt stack;
        stack.push( new int(1) );
        CHECK( stack.size() == 1 );
        stack.push( new int(2) );
        CHECK( stack.size() == 2 );
    }

    TEST_FIXTURE(StackTestFixture, StackPop)
    {
        StackInt stack;
        stack.push( new int(1) );
        stack.push( new int(2) );
        CHECK( stack.size() == 2 );
        int* p = stack.pop();
        CHECK( *p == 2 );
        delete p;
        p = stack.pop();
        CHECK( *p == 1 );
        delete p;
        CHECK( stack.size() == 0 );
    }

    TEST_FIXTURE(StackTestFixture, StackGetItem)
    {
        StackInt stack;
        stack.push( new int(1) );
        stack.push( new int(2) );
        stack.push( new int(3) );
        stack.push( new int(4) );
        CHECK( stack.size() == 4 );
        CHECK( *(stack.get_item(2)) == 3 );
    }

    TEST_FIXTURE(StackTestFixture, StackPopWhenEmpty)
    {
        StackInt stack;
        int* p = stack.pop();
        CHECK( p == nullptr );
    }

}

//-------------------------------------------------------------------------------

typedef UndoableStack<int*>  UndoStackInt;

class UndoableStackTestFixture
{
public:

    UndoableStackTestFixture()     //SetUp fixture
    {
    }

    ~UndoableStackTestFixture()    //TearDown fixture
    {
    }
};

SUITE(UndoableStackTest)
{
    TEST_FIXTURE(UndoableStackTestFixture, UndoableStackIsEmpty)
    {
        UndoStackInt stack;
        CHECK( stack.size() == 0 );
    }

    TEST_FIXTURE(UndoableStackTestFixture, UndoableStackPush)
    {
        UndoStackInt stack;
        stack.push( new int(1) );
        CHECK( stack.size() == 1 );
        stack.push( new int(2) );
        CHECK( stack.size() == 2 );
    }

    TEST_FIXTURE(UndoableStackTestFixture, UndoableStackPop)
    {
        UndoStackInt stack;
        stack.push( new int(1) );
        stack.push( new int(2) );
        CHECK( stack.size() == 2 );
        int* p = stack.pop();
        CHECK( *p == 2 );
        p = stack.pop();
        CHECK( *p == 1 );
        CHECK( stack.size() == 0 );
    }

    TEST_FIXTURE(UndoableStackTestFixture, UndoableStackGetItem)
    {
        UndoStackInt stack;
        stack.push( new int(1) );
        stack.push( new int(2) );
        stack.push( new int(3) );
        stack.push( new int(4) );
        CHECK( stack.size() == 4 );
        CHECK( *(stack.get_item(2)) == 3 );
    }

    TEST_FIXTURE(UndoableStackTestFixture, UndoableStackPopWhenEmpty)
    {
        UndoStackInt stack;
        int* p = stack.pop();
        CHECK( p == nullptr );
    }

    TEST_FIXTURE(UndoableStackTestFixture, UndoableStackUndoPop)
    {
        UndoStackInt stack;
        stack.push( new int(1) );
        stack.push( new int(2) );
        stack.push( new int(3) );
        stack.push( new int(4) );
        CHECK( stack.size() == 4 );
        CHECK( *(stack.get_item(2)) == 3 );
        CHECK( stack.history_size() == 0 );
        stack.pop();    //4
        CHECK( stack.history_size() == 1 );
        stack.pop();    //3
        CHECK( stack.history_size() == 2 );
        stack.undo_pop();   //adds 3
        CHECK( stack.history_size() == 1 );
        CHECK( stack.size() == 3 );
        CHECK( *(stack.get_item(2)) == 3 );
        stack.undo_pop();   //adds 4
        CHECK( stack.history_size() == 0 );
        CHECK( stack.size() == 4 );
        CHECK( *(stack.get_item(3)) == 4 );
    }

}

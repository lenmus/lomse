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
        CHECK( p == NULL );
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
        CHECK( p == NULL );
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

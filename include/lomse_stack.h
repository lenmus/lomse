//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#ifndef __LOMSE_STACK_H__
#define __LOMSE_STACK_H__

#include <stddef.h>
#include <list>
using namespace std;

namespace lomse
{

/*! A simple stack class, but contrary to std::stack, the pile can be inspected. */
//--------------------------------------------------------------------------------
template <typename T>
class Stack
{
protected:
    std::list<T> m_list;

public:
    Stack() {}
    virtual ~Stack() {
        typename std::list<T>::iterator it;
        for (it=m_list.begin(); it != m_list.end(); ++it)
            delete *it;
        m_list.clear();
    }

    size_t size() { return m_list.size(); }
    void push(T t) { m_list.push_back(t); }
    T pop() {
        if (m_list.size() > 0)
        {
            T t = m_list.back();
            m_list.pop_back();
            return t;
        }
        else
            return nullptr;
    }

    const T get_item(int i) {
        typename std::list<T>::iterator it;
        for (it=m_list.begin(); it != m_list.end() && i > 0; ++it)
            --i;
        if (it != m_list.end())
            return *it;
        else
            return nullptr;
    }

};


/*! A undo/redo stack class.
    It maintains the history of pop() operations, so a undo_pop() operation
    restores the previous pop(). undo_pop() only can be used after pop() one or more
    pop() operations, and as many times as consecutive pop() operations.
    The push() operation clears the pop() history so undo_pop() is no longer
    valid after a push().
*/
//------------------------------------------------------------------
template <typename T>
class UndoableStack
{
protected:
    std::list<T> m_list;
    std::list<T> m_history;

public:
    UndoableStack() {}

    virtual ~UndoableStack() {
        typename std::list<T>::iterator it;
        for (it=m_list.begin(); it != m_list.end(); ++it)
            delete *it;
        m_list.clear();

        remove_history();
    }

    size_t size() { return m_list.size(); }
    size_t history_size() { return m_history.size(); }

    void push(T t) {
        remove_history();
        m_list.push_back(t);
    }

    T pop() {
        if (m_list.size() > 0)
        {
            T t = m_list.back();
            m_list.pop_back();
            m_history.push_back(t);
            return t;
        }
        else
            return nullptr;
    }

    T undo_pop() {
        if (m_history.size() > 0)
        {
            T t = m_history.back();
            m_history.pop_back();
            m_list.push_back(t);
            return t;
        }
        else
            return nullptr;
    }

    const T get_item(int i) {
        typename std::list<T>::iterator it;
        for (it=m_list.begin(); it != m_list.end() && i > 0; ++it)
            --i;
        return (it != m_list.end() ? *it : nullptr);
    }

protected:
    void remove_history() {
        typename std::list<T>::iterator it;
        for (it = m_history.begin(); it != m_history.end(); ++it)
            delete *it;
        m_history.clear();
    }


};

}   //namespace lomse

#endif      //__LOMSE_STACK_H__

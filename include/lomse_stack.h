//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_STACK_H__
#define __LOMSE_STACK_H__

#include <stddef.h>
#include <list>
using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
/** A simple stack class, but contrary to std::stack, the pile can be inspected.
*/
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


//---------------------------------------------------------------------------------------
/** A undo/redo stack class.
    It maintains the history of pop() operations, so a undo_pop() operation
    restores the previous pop(). undo_pop() only can be used after pop() one or more
    pop() operations, and as many times as consecutive pop() operations.
    The push() operation clears the pop() history so undo_pop() is no longer
    valid after a push().
*/
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

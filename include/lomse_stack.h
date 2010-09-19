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

#ifndef __LOMSE__STACK_H__
#define __LOMSE__STACK_H__

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
            return NULL;
    }

    const T get_item(int i) {
        typename std::list<T>::iterator it;
        for (it=m_list.begin(); it != m_list.end() && i > 0; ++it)
            --i;
        return *it;
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
            return NULL;
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
            return NULL;
    }

    const T get_item(int i) {
        typename std::list<T>::iterator it;
        for (it=m_list.begin(); it != m_list.end() && i > 0; ++it)
            --i;
        return *it;
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

#endif      //__LOMSE__STACK_H__

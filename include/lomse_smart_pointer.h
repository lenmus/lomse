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
// -----------------------------
//  Credits:
//      This file is based on the "smartpointer.h" file from the MusicXML Library
//      v.2.00, distributed under LGPL 2.1 or greater. Copyright (c) 2006 Grame,
//      Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France,
//      research@grame.fr.
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SMART_POINTER_H__
#define __LOMSE_SMART_POINTER_H__

#include "lomse_build_options.h"

#include <cassert>
#include <sstream>
#include <stdexcept>
using namespace std;


namespace lomse
{

//---------------------------------------------------------------------------------------
// RefCounted: Base class for objects with intrusive reference counting
class LOMSE_EXPORT RefCounted
{
private:
	int m_counter;
public:
	inline int refs() const { return m_counter; }
	void addReference() {
        m_counter++;
        if (m_counter == 0)
            throw runtime_error("Reference counted object: invalid counter value");
    }
	void removeReference() { if (--m_counter == 0) delete this; }

protected:
	RefCounted() : m_counter(0) {}
	RefCounted(const RefCounted&): m_counter(0) {}
	virtual ~RefCounted() { assert (m_counter == 0); }
	RefCounted& operator=(const RefCounted&) { return *this; }

};

//---------------------------------------------------------------------------------------
// SmartPtr: base class for objects having smart pointers.
//    Deriving from SmartPtr also requires deriving from RefCounted
template<class T>
class SmartPtr
{
private:
	T* m_pThePointer;

public:
	//! an empty constructor - points to null
	SmartPtr() : m_pThePointer(0) {}
	//! build a smart pointer from a class pointer
	SmartPtr(T* rawptr) : m_pThePointer(rawptr) {
        if (m_pThePointer) m_pThePointer->addReference();
    }
	//! build a smart pointer from an convertible class reference
	template<class T2>
	SmartPtr(const SmartPtr<T2>& ptr) : m_pThePointer((T*)ptr) {
        if (m_pThePointer) m_pThePointer->addReference();
    }
	//! build a smart pointer from another smart pointer reference
	SmartPtr(const SmartPtr& ptr) : m_pThePointer((T*)ptr) {
        if (m_pThePointer) m_pThePointer->addReference();
    }

	//! the smart pointer destructor: simply removes one reference count
	~SmartPtr() { if (m_pThePointer) m_pThePointer->removeReference(); }

	//! cast operator to retrieve the actual class pointer
	operator T*() const { return m_pThePointer; }

    //! another way of retrieving the actual class pointer
    T* get_pointer() const { return m_pThePointer; }

	//! '*' operator to access the actual class pointer
	T& operator*() const {
		// checks for null dereference
		assert (m_pThePointer != 0);
		return *m_pThePointer;
	}

	//! operator -> overloading to access the actual class pointer
	T* operator->() const	{
		// checks for null dereference
		assert (m_pThePointer != 0);
		return m_pThePointer;
	}

	//! operator = that moves the actual class pointer
	template <class T2>
	SmartPtr& operator=(T2 p1_) { *this=(T*)p1_; return *this; }

	//! operator = that moves the actual class pointer
	SmartPtr& operator=(T* p_) {
		// check first that pointers differ
		if (m_pThePointer != p_) {
			// increments the ref count of the new pointer if not null
			if (p_ != 0) p_->addReference();
			// decrements the ref count of the old pointer if not null
			if (m_pThePointer != 0) m_pThePointer->removeReference();
			// and finally stores the new actual pointer
			m_pThePointer = p_;
		}
		return *this;
	}
	//! operator = to support inherited class reference
	SmartPtr& operator=(const SmartPtr<T>& p_) { return operator=((T *) p_); }
	//! dynamic cast support
	template<class T2> SmartPtr& cast(T2* p_) { return operator=(dynamic_cast<T*>(p_)); }
	//! dynamic cast support
	template<class T2> SmartPtr& cast(const SmartPtr<T2>& p_) { return operator=(dynamic_cast<T*>(p_)); }
	//! operator < (require by VC6 for maps)
	bool operator < (const SmartPtr<T>& p_) const { return (void*)this < (void*)p_; }
};

}   //namespace lomse

#endif      //__LOMSE_SMART_POINTER_H__

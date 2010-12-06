//---------------------------------------------------------------------------------------
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

#include <cassert>

#include "lomse_build_options.h"

namespace lomse
{

/*!
 \brief     Base class for objects that would like to have support for smart pointers.

    We use smart pointers based on reference counting. The policy used for counter
    ownership is 'intrusive reference counting'. This implies that the reference
    count is an "intruder" in the pointee. Class RefCounted provides the counter,
    members to maintain the count and automatic object delete when the reference
    count drops to zero. RefCounted is a mandatory base class for objects that
    would like to have support for smart pointers.

*/
class LOMSE_EXPORT RefCounted
{
private:
	unsigned int m_count;
public:
	//! returns the reference count of the object
	unsigned int refs() const { return m_count; }
	//! addReference increments the ref count and checks for m_count overflow
	void addReference() { m_count++; assert(m_count != 0); }
	//! removeReference delete the object when m_count is zero
	void removeReference() { if (--m_count == 0) delete this; }

protected:
	RefCounted() : m_count(0) {}
	RefCounted(const RefCounted&): m_count(0) {}
	//! destructor checks for non-zero m_count
	virtual ~RefCounted() { assert (m_count == 0); }
	RefCounted& operator=(const RefCounted&) { return *this; }

};

/*!
\brief the smart pointer implementation

	A smart pointer is in charge of maintaining the objects reference count
	by the way of pointers operators overloading. It supports class
	inheritance and conversion whenever possible.
\n	Instances of the SmartPtr class are supposed to use \e RefCounted types (or at least
	objects that implements the \e addReference and \e removeReference
	methods in a consistent way).
*/
template<class T>
class SmartPtr
{
private:
	//! the actual pointer to the class
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

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

#ifndef __LOMSE_OBSERVABLE_H__
#define __LOMSE_OBSERVABLE_H__

#include <list>

using namespace std;

namespace lomse
{

//forward declarations
class Observable;


//-------------------------------------------------------------------------------------
class Observer
{
public:
    Observer() {}
	virtual ~Observer() {}

	virtual void handle_event(Observable* ref) = 0;
};


//-------------------------------------------------------------------------------------
class Observable
{
protected:
    std::list<Observer*>    m_observers;

public:
    Observable() {}
	virtual ~Observable() {}

	void add_observer(Observer* ref) {
		m_observers.push_back(ref);
	}

	void remove_observer(Observer* ref) {
		m_observers.remove(ref);
	}

protected:
	void notify_observers() {
        std::list<Observer*>::iterator it;
		for(it = m_observers.begin(); it != m_observers.end(); ++it)
            (*it)->handle_event(this);
	}

};


}   //namespace lomse

#endif      //__LOMSE_OBSERVABLE_H__

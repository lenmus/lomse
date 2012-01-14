//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_OBSERVABLE_H__
#define __LOMSE_OBSERVABLE_H__

#include <list>

using namespace std;

namespace lomse
{

//-------------------------------------------------------------------------------------
template <typename T>
class Linkable
{
protected:
    std::list<Linkable<T>*> m_linkedTo;
    std::list< pair<Linkable<T>*, int> > m_linkedFrom;

public:
    Linkable() {}
	virtual ~Linkable()
    {
        //inform observers that this target is being deleted
        typename std::list< pair<Linkable<T>*, int> >::iterator it;
		for(it = m_linkedFrom.begin(); it != m_linkedFrom.end(); ++it)
        {
            Linkable<T>* observer = (*it).first;
            observer->link_target_deleted(this);
        }

        //inform targets that this observer is being deleted
        typename std::list<Linkable<T>*>::iterator itT;
		for(itT = m_linkedTo.begin(); itT != m_linkedTo.end(); ++itT)
        {
            (*itT)->remove_link_from(this);
        }
    }


        //methods for link destination (observed object)

	void accept_link_from(Linkable<T>* observer, int type=0)
    {
		m_linkedFrom.push_back( make_pair(observer, type) );
        observer->notify_success(this, type);
	}

	void remove_link_from(Linkable<T>* observer)
	{
        typename std::list< pair<Linkable<T>*, int> >::iterator it;
		for(it = m_linkedFrom.begin(); it != m_linkedFrom.end(); ++it)
        {
            if (observer == (*it).first)
            {
		        m_linkedFrom.remove( *it );
                break;
            }
        }
	}


        //methods for link originator (observer object)

	inline int get_num_links_to() { return int(m_linkedTo.size()); }
	Linkable<T>* get_link_target(int i)
	{
        typename std::list<Linkable<T>*>::iterator itT;
		for(itT = m_linkedTo.begin(); itT != m_linkedTo.end() && i > 0; ++itT, --i);
        return *itT;
	}

	virtual void handle_link_event(Linkable<T>* ref, int type, T data) = 0;
    virtual void on_linked_to(Linkable<T>* ref, int type) = 0;


protected:
	void notify_linked_observers(T data)
    {
        typename std::list< pair<Linkable<T>*, int> >::iterator it;
		for(it = m_linkedFrom.begin(); it != m_linkedFrom.end(); ++it)
        {
            Linkable<T>* observer = (*it).first;
            observer->handle_link_event(this, (*it).second, data);
        }
	}

    void notify_success(Linkable<T>* ref, int type)
    {
        m_linkedTo.push_back(ref);
        on_linked_to(ref, type);
    }

    void link_target_deleted(Linkable<T>* ref)
    {
        m_linkedTo.remove(ref);
    }


};


}   //namespace lomse

#endif      //__LOMSE_OBSERVABLE_H__

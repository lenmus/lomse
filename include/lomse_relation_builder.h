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
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_RELATION_BUILDER_H__
#define __LOMSE_RELATION_BUILDER_H__

#include "lomse_ldp_elements.h"

#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class Analyser;
class InternalModel;


//---------------------------------------------------------------------------------------
// helper class to save item info items, match them and build the items
template <class T>
class RelationBuilder
{
protected:
    ostream& m_reporter;
    Analyser* m_pAnalyser;
    std::string m_relationNameLowerCase;
    std::string m_relationNameUpperCase;

    typedef typename std::list<T*> List;
    typedef typename List::iterator ListIterator;
    List m_pendingItems;


public:
    RelationBuilder(ostream& reporter, Analyser* pAnalyser,
                    const std::string& relationNameLowerCase,
                    const std::string& relationNameUpperCase);
    ~RelationBuilder();

    void add_item_info(T* pInfo);
    void clear_pending_items();

protected:
    bool find_matching_info_items(int itemNum);
    void create_item(T* pInfo);
    void save_item_info(T* pNewInfo);
    void delete_consumed_info_items(T* pEndInfo);
    void delete_item_element(T* pInfo);
    T* find_matching_start_item(T* pInfo);

    virtual void add_relation_to_notes_rests(T* pInfo) = 0;

    //errors
    void error_no_matching_items(T* pInfo);
    void error_no_end_item(T* pInfo);
    void error_duplicated_number(T* pExistingInfo, T* pNewInfo);

    //temporary: items that match
    std::list<T*> m_matches;

};



//---------------------------------------------------------------------------------------
// RelationBuilder implementation
//---------------------------------------------------------------------------------------
template <class T>
RelationBuilder<T>::RelationBuilder(ostream& reporter, Analyser* pAnalyser,
                                    const std::string& relationNameLowerCase,
                                    const std::string& relationNameUpperCase)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
    , m_relationNameLowerCase(relationNameLowerCase)
    , m_relationNameUpperCase(relationNameUpperCase)
{
}

//---------------------------------------------------------------------------------------
template <class T>
RelationBuilder<T>::~RelationBuilder()
{
    clear_pending_items();
}

//---------------------------------------------------------------------------------------
template <class T>
void RelationBuilder<T>::add_item_info(T* pNewInfo)
{
    if (pNewInfo->is_start_of_relation())
    {
        T* pExistingInfo = find_matching_start_item(pNewInfo);
        if (pExistingInfo)
            error_duplicated_number(pExistingInfo, pNewInfo);
        else
            save_item_info(pNewInfo);
    }
    else if (pNewInfo->is_end_of_relation())
        create_item(pNewInfo);
    else
        save_item_info(pNewInfo);
}

//---------------------------------------------------------------------------------------
template <class T>
void RelationBuilder<T>::save_item_info(T* pNewInfo)
{
    m_pendingItems.push_back(pNewInfo);
}

//---------------------------------------------------------------------------------------
template <class T>
void RelationBuilder<T>::create_item(T* pEndInfo)
{
    int itemNum = pEndInfo->get_item_number();
    if ( find_matching_info_items(itemNum) )
    {
        add_relation_to_notes_rests(pEndInfo);
        delete_consumed_info_items(pEndInfo);
    }
    else
    {
        error_no_matching_items(pEndInfo);
        delete_item_element(pEndInfo);
    }
}

//---------------------------------------------------------------------------------------
template <class T>
bool RelationBuilder<T>::find_matching_info_items(int itemNum)
{
    m_matches.clear();
    ListIterator it;
    for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
    {
        if ((*it)->get_item_number() == itemNum)
            m_matches.push_back(*it);
    }
    return !m_matches.empty();
}

//---------------------------------------------------------------------------------------
template <class T>
T* RelationBuilder<T>::find_matching_start_item(T* pInfo)
{
    ListIterator it;
    for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
    {
         if ((*it)->get_item_number() == pInfo->get_item_number()
             && (*it)->is_start_of_relation() )
             return *it;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
template <class T>
void RelationBuilder<T>::delete_consumed_info_items(T* pEndInfo)
{
    m_matches.clear();
    int itemNum = pEndInfo->get_item_number();
    ListIterator it;
    for(it=m_pendingItems.begin(); it != m_pendingItems.end(); )
    {
        if ((*it)->get_item_number() == itemNum)
        {
            delete *it;
            it = m_pendingItems.erase(it);
        }
        else
            ++it;
    }
    delete pEndInfo;
}

//---------------------------------------------------------------------------------------
template <class T>
void RelationBuilder<T>::delete_item_element(T* pInfo)
{
    //Nothing to do: pInfo is deleted automatically when erasing node
}

//---------------------------------------------------------------------------------------
template <class T>
void RelationBuilder<T>::clear_pending_items()
{
    ListIterator it;
    for (it = m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
        error_no_end_item(*it);
    m_pendingItems.clear();
}

//---------------------------------------------------------------------------------------
template <class T>
void RelationBuilder<T>::error_no_matching_items(T* pInfo)
{
    m_reporter << "Line " << pInfo->get_line_number()
               << ". No 'start/continue' elements for "
               << m_relationNameLowerCase << " number "
               << pInfo->get_item_number()
               << ". " << m_relationNameUpperCase << " ignored." << endl;
    delete pInfo;
}

//---------------------------------------------------------------------------------------
template <class T>
void RelationBuilder<T>::error_no_end_item(T* pInfo)
{
    m_reporter << "Line " << pInfo->get_line_number()
               << ". No 'end' element for "
               << m_relationNameLowerCase << " number "
               << pInfo->get_item_number()
               << ". " << m_relationNameUpperCase << " ignored." << endl;
    delete pInfo;
}

//---------------------------------------------------------------------------------------
template <class T>
void RelationBuilder<T>::error_duplicated_number(T* pExistingInfo, T* pNewInfo)
{
    m_reporter << "Line " << pNewInfo->get_line_number()
               << ". This " << m_relationNameLowerCase
               << " has the same number than that defined in line "
               << pExistingInfo->get_line_number()
               << ". This " << m_relationNameLowerCase << " will be ignored." << endl;
    delete pNewInfo;
}



}   //namespace lomse

#endif      //__LOMSE_RELATION_BUILDER_H__

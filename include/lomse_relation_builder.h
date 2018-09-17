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

#ifndef __LOMSE_RELATION_BUILDER_H__
#define __LOMSE_RELATION_BUILDER_H__

#include "lomse_ldp_elements.h"

#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class A;


//---------------------------------------------------------------------------------------
// helper class to save item info items, match them and build the items
template <class T, class A>
class RelationBuilder
{
protected:
    ostream& m_reporter;
    A* m_pAnalyser;
    std::string m_relationNameLowerCase;
    std::string m_relationNameUpperCase;

    typedef typename std::list<T*> List;
    typedef typename List::iterator ListIterator;
    List m_pendingItems;


public:
    RelationBuilder(ostream& reporter, A* pAnalyser,
                    const std::string& relationNameLowerCase,
                    const std::string& relationNameUpperCase);
    virtual ~RelationBuilder();

    void add_item_info(T* pInfo);
    void clear_pending_items();

protected:
    bool find_matching_info_items(int itemNum);
    void create_item(T* pInfo);
    void save_item_info(T* pNewInfo);
    void delete_consumed_info_items(T* pEndInfo);
    T* find_matching_start_item(T* pInfo);
    T* find_duplicated_staffobj(T* pInfo);

    virtual void add_relation_to_staffobjs(T* pInfo) = 0;

    //errors
    void error_no_matching_items(T* pInfo);
    void error_no_end_item(T* pInfo);
    void error_duplicated_number(T* pExistingInfo, T* pNewInfo);
    void error_duplicated_staffobj(T* pExistingInfo, T* pNewInfo);

    //temporary: items that match
    std::list<T*> m_matches;

};



//---------------------------------------------------------------------------------------
// RelationBuilder implementation
//---------------------------------------------------------------------------------------
template <class T, class A>
RelationBuilder<T, A>::RelationBuilder(ostream& reporter, A* pAnalyser,
                                       const std::string& relationNameLowerCase,
                                       const std::string& relationNameUpperCase)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
    , m_relationNameLowerCase(relationNameLowerCase)
    , m_relationNameUpperCase(relationNameUpperCase)
{
}

//---------------------------------------------------------------------------------------
template <class T, class A>
RelationBuilder<T, A>::~RelationBuilder()
{
    clear_pending_items();
}

//---------------------------------------------------------------------------------------
template <class T, class A>
void RelationBuilder<T, A>::add_item_info(T* pNewInfo)
{
    T* pExistingInfo = find_duplicated_staffobj(pNewInfo);
    if (pExistingInfo)
    {
        error_duplicated_staffobj(pExistingInfo, pNewInfo);
        return;
    }

    if (pNewInfo->is_start_of_relation())
    {
        pExistingInfo = find_matching_start_item(pNewInfo);
        if (pExistingInfo)
        {
            error_duplicated_number(pExistingInfo, pNewInfo);
            return;
        }

        save_item_info(pNewInfo);
    }
    else if (pNewInfo->is_end_of_relation())
        create_item(pNewInfo);
    else
        save_item_info(pNewInfo);
}

//---------------------------------------------------------------------------------------
template <class T, class A>
void RelationBuilder<T, A>::save_item_info(T* pNewInfo)
{
    m_pendingItems.push_back(pNewInfo);
}

//---------------------------------------------------------------------------------------
template <class T, class A>
void RelationBuilder<T, A>::create_item(T* pEndInfo)
{
    int itemNum = pEndInfo->get_item_number();
    if ( find_matching_info_items(itemNum) )
    {
        add_relation_to_staffobjs(pEndInfo);
        delete_consumed_info_items(pEndInfo);
    }
    else
    {
        error_no_matching_items(pEndInfo);
    }
}

//---------------------------------------------------------------------------------------
template <class T, class A>
bool RelationBuilder<T, A>::find_matching_info_items(int itemNum)
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
template <class T, class A>
T* RelationBuilder<T, A>::find_matching_start_item(T* pInfo)
{
    ListIterator it;
    for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
    {
         if ((*it)->get_item_number() == pInfo->get_item_number()
             && (*it)->is_start_of_relation() )
             return *it;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
template <class T, class A>
T* RelationBuilder<T, A>::find_duplicated_staffobj(T* pInfo)
{
    ListIterator it;
    for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
    {
         if ((*it)->get_item_number() == pInfo->get_item_number()
             && (*it)->get_staffobj() == pInfo->get_staffobj() )
             return *it;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
template <class T, class A>
void RelationBuilder<T, A>::delete_consumed_info_items(T* pEndInfo)
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
template <class T, class A>
void RelationBuilder<T, A>::clear_pending_items()
{
    ListIterator it;
    for (it = m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
        error_no_end_item(*it);
    m_pendingItems.clear();
}

//---------------------------------------------------------------------------------------
template <class T, class A>
void RelationBuilder<T, A>::error_no_matching_items(T* pInfo)
{
    m_reporter << "Line " << pInfo->get_line_number()
               << ". No 'start/continue' elements for "
               << m_relationNameLowerCase << " number "
               << pInfo->get_item_number()
               << ". " << m_relationNameUpperCase << " ignored." << endl;
    delete pInfo;
}

//---------------------------------------------------------------------------------------
template <class T, class A>
void RelationBuilder<T, A>::error_no_end_item(T* pInfo)
{
    m_reporter << "Line " << pInfo->get_line_number()
               << ". No 'end' element for "
               << m_relationNameLowerCase << " number "
               << pInfo->get_item_number()
               << ". " << m_relationNameUpperCase << " ignored." << endl;
    delete pInfo;
}

//---------------------------------------------------------------------------------------
template <class T, class A>
void RelationBuilder<T, A>::error_duplicated_number(T* pExistingInfo, T* pNewInfo)
{
    m_reporter << "Line " << pNewInfo->get_line_number()
               << ". This " << m_relationNameLowerCase
               << " has the same number than that defined in line "
               << pExistingInfo->get_line_number()
               << ". This " << m_relationNameLowerCase << " will be ignored." << endl;
    delete pNewInfo;
}

//---------------------------------------------------------------------------------------
template <class T, class A>
void RelationBuilder<T, A>::error_duplicated_staffobj(T* pExistingInfo, T* pNewInfo)
{
    m_reporter << "Line " << pNewInfo->get_line_number()
               << ". A " << m_relationNameLowerCase
               << " with the same number is already defined for this element in line "
               << pExistingInfo->get_line_number()
               << ". This " << m_relationNameLowerCase << " will be ignored." << endl;
    delete pNewInfo;
}



}   //namespace lomse

#endif      //__LOMSE_RELATION_BUILDER_H__

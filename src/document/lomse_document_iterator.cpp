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

#include "lomse_document_iterator.h"

#include "lomse_document.h"
#include "lomse_internal_model.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// ElementIterator implementation
//---------------------------------------------------------------------------------------
ElementIterator::ElementIterator(ImoObj* pObj)
    : m_pObj(pObj)
{
    m_pCurItem = NULL;    //m_pTree->begin();
}

//---------------------------------------------------------------------------------------
ElementIterator::~ElementIterator()
{
}

////---------------------------------------------------------------------------------------
//void ElementIterator::to_begin()
//{
//    clear_stack();
//    m_pCurItem = m_pTree->begin();
//}
//
////---------------------------------------------------------------------------------------
//void ElementIterator::clear_stack()
//{
//    while (!m_stack.empty())
//        m_stack.pop();
//}
//
////---------------------------------------------------------------------------------------
//bool ElementIterator::is_pointing_to(long objType)
//{
//    return *m_pCurItem != NULL && (*m_pCurItem)->get_type() == objType;
//}
//
////---------------------------------------------------------------------------------------
//void ElementIterator::point_to(int objType)
//{
//    while (*m_pCurItem != NULL && !is_pointing_to(objType))
//        next();
//}
//
//---------------------------------------------------------------------------------------
void ElementIterator::point_to(ImoObj* pObj)
{
    m_pCurItem = pObj;
}

////---------------------------------------------------------------------------------------
//void ElementIterator::enter_element()
//{
//    if (*m_pCurItem != NULL)
//    {
//        m_stack.push(m_pCurItem);
//        ++m_pCurItem;
//    }
//}
//
////---------------------------------------------------------------------------------------
//void ElementIterator::exit_element()
//{
//    if (!m_stack.empty())
//    {
//        m_pCurItem = m_stack.top();
//        m_stack.pop();
//    }
//    else
//        m_pCurItem = NULL;
//}
//
////---------------------------------------------------------------------------------------
//void ElementIterator::exit_all_to(ImoObj* pImo)
//
//{
//     //exit elements until the received one
//
//    while (*m_pCurItem != pImo && !m_stack.empty())
//    {
//        m_pCurItem = m_stack.top();
//        if (*m_pCurItem == pImo)
//            break;
//        m_stack.pop();
//    }
//}
//
////---------------------------------------------------------------------------------------
////void ElementIterator::start_of(long objType, int num)
////{
////    //within the limits of current element finds the element #num [0..n-1]
////    //of type 'objType' and points to its first sub-element
////
////    to_begin();
////    enter_element();
////    point_to(k_content);
////    enter_element();
////}



//---------------------------------------------------------------------------------------
// DocIterator implementation
//---------------------------------------------------------------------------------------
DocIterator::DocIterator(Document* pDoc)
    : ElementIterator(pDoc->get_imodoc())
    , m_pDoc(pDoc->get_imodoc())
    //, m_pScoreElmIterator(NULL)
{
    m_numContentItems = m_pDoc->get_num_content_items();
    m_curItemIndex = -1;
    next();
}

//---------------------------------------------------------------------------------------
DocIterator::~DocIterator()
{
//    if (m_pScoreElmIterator)
//        delete m_pScoreElmIterator;
}

//---------------------------------------------------------------------------------------
void DocIterator::next()
{
    ++m_curItemIndex;
    point_to_current();
}

//---------------------------------------------------------------------------------------
void DocIterator::prev()
{
    --m_curItemIndex;
    point_to_current();
}

//---------------------------------------------------------------------------------------
void DocIterator::point_to_current()
{
    if (m_curItemIndex >=0 && m_curItemIndex < m_numContentItems)
        m_pCurItem = m_pDoc->get_content_item(m_curItemIndex);
    else
        m_pCurItem = NULL;
}

////---------------------------------------------------------------------------------------
//void DocIterator::enter_element()
//{
//    //Factory method
//    //if new elements added, create specific cursor to delegate to it.
//
//    if (m_pCurItem->is_score())
//    {
//        if (m_pScoreElmIterator)
//            delete m_pScoreElmIterator;
//        m_pScoreElmIterator = new ScoreElmIterator(this, dynamic_cast<ImScore*>(m_pCurItem) );
//    }
//}

//---------------------------------------------------------------------------------------
void DocIterator::start_of_content()
{
    //to first item in 'content' element

//    to_begin();
//    enter_element();
//    point_to(k_content);
//    enter_element();
    m_curItemIndex = -1;
    next();
}

//---------------------------------------------------------------------------------------
void DocIterator::last_of_content()
{
//    to_begin();
//    enter_element();
//    point_to(k_content);
//    m_pCurItem = (*m_pCurItem)->get_last_child();
    m_curItemIndex = m_numContentItems - 2;
    next();
}



////---------------------------------------------------------------------------------------
//// ScoreElmIterator implementation
////---------------------------------------------------------------------------------------
//ScoreElmIterator::ScoreElmIterator(ElementIterator* pParent, ImScore* pScore)
//    : m_pParent(pParent)
//    , m_pScore(pScore)
//{
//}
//
////---------------------------------------------------------------------------------------
//ScoreElmIterator::~ScoreElmIterator()
//{
//}

////---------------------------------------------------------------------------------------
//void ScoreElmIterator::start()
//{
//    m_pParent->exit_all_to(m_pScore);
//    m_pParent->enter_element();
//}
//
////---------------------------------------------------------------------------------------
//void ScoreElmIterator::start_of_instrument(int instr)
//{
//    //to first staff obj of instr (0..n-1)
//
//    find_instrument(instr);
//    m_pParent->enter_element();
//    m_pParent->point_to(k_musicData);
//    m_pParent->enter_element();
//}
//
////---------------------------------------------------------------------------------------
//void ScoreElmIterator::find_instrument(int instr)
//{
//    //instr = 0..n
//
//    start();
//    m_pParent->point_to(k_instrument);
//    for (int i=0; i != instr && !m_pParent->is_out_of_range(); i++)
//    {
//        ++(*m_pParent);
//        m_pParent->point_to(k_instrument);
//    }
//}


}  //namespace lomse

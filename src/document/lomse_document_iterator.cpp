//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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
//        m_pScoreElmIterator = LOMSE_NEW ScoreElmIterator(this, dynamic_cast<ImScore*>(m_pCurItem) );
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
    m_numContentItems = m_pDoc->get_num_content_items();
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

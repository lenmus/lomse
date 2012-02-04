//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_DOCUMENT_ITERATOR_H__
#define __LOMSE_DOCUMENT_ITERATOR_H__

#include <stack>

using namespace std;

namespace lomse
{

//forward declarations
class ImoDocument;
class ImoScore;
class ImoObj;
class Document;


////---------------------------------------------------------------------------------------
//// interfaces for traversing specific elements
////---------------------------------------------------------------------------------------
//
//class InterfaceScoreElmIterator
//{
//public:
//    //positioning
//    virtual void start_of_instrument(int instr)=0;    //to first staff obj in instr 0..n
//    virtual void find_instrument(int instr)=0;        //instr = 0..n-1
//};
//


//---------------------------------------------------------------------------------------
// ElementIterator: base class for any document iterator. It provides the basic
// capabilities for traversing elements, locating specific sub-elements, etc.
//---------------------------------------------------------------------------------------

class ElementIterator
{
protected:
    ImoObj* m_pObj;      //element being traversed
    ImoObj* m_pCurItem;  //current pointed child

public:
    ElementIterator(ImoObj* pObj);
    virtual ~ElementIterator();

    inline ImoObj* operator *() { return m_pCurItem; }

    //positioning
    inline void operator ++() { next(); }
    inline void operator --() { prev(); }
//    void point_to(int objType);
    void point_to(ImoObj* pObj);
    //void enter_element();
//    void exit_element();
//    void exit_all_to(ImoObj* pImo);     //exit elements until the received one
//    void to_begin();
//    //void start_of(long objType, int num);   //to first sub-element in element #num [0..n-1] of type 'objType'
//
//    //information
//    bool is_pointing_to(long objType);
//    inline bool is_out_of_range() { return *m_it == NULL; }
//
protected:
    virtual void next()=0;    //to next sibling element
    virtual void prev()=0;    //to prev sibling element
//    void clear_stack();

};





////---------------------------------------------------------------------------------------
//// ScoreElmIterator: adds score traversing capabilities to the received cursor.
//// The cursor must be pointing to the score when invoking the constructor
////---------------------------------------------------------------------------------------
//
//class ScoreElmIterator //: public InterfaceScoreElmIterator
//{
//protected:
//    ElementIterator* m_pParent;
//    ImoScore* m_pScore;
//
//public:
//    ScoreElmIterator(ElementIterator* pParent, ImoScore* pScore);
//    virtual ~ScoreElmIterator();
//
////    //positioning
////    void start_of_instrument(int instr);    //to first staff obj in instr 0..n
////    void find_instrument(int instr);        //instr = 0..n-1
//
//protected:
//    void start();   //to first element in score
//
//};



//---------------------------------------------------------------------------------------
// DocIterator: A cursor to traverse the document for interactive edition
// - It uses the facade pattern to hide the particularities of traversing each
//   document element type
// - For traversing each element type it delegates on a specific cursor (adaptor
//   pattern)
//---------------------------------------------------------------------------------------

class DocIterator : public ElementIterator      //, public InterfaceScoreElmIterator
{
protected:
    ImoDocument* m_pDoc;
    int m_numContentItems;
    int m_curItemIndex;

public:
    DocIterator(Document* pDoc);
    virtual ~DocIterator();

    //positioning
    void start_of_content();
    void last_of_content();

    ////overrides
    //void enter_element();

//    //implement InterfaceScoreElmIterator by delegation
//    void start_of_instrument(int instr) {   //to first staff obj in instr 0..n
//            if (m_pScoreElmIterator)
//                m_pScoreElmIterator->start_of_instrument(instr);
//        }
//
//    void find_instrument(int instr) {   //instr = 0..n-1
//            if (m_pScoreElmIterator)
//                m_pScoreElmIterator->find_instrument(instr);
//        }

protected:
    //ScoreElmIterator* m_pScoreElmIterator;

    void next();
    void prev();
    void point_to_current();

};


}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_ITERATOR_H__

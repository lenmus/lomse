//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#ifndef __LOMSE_SCORE_ITERATOR_H__
#define __LOMSE_SCORE_ITERATOR_H__

#include <vector>
#include "lomse_staffobjs_table.h"

using namespace std;

namespace lomse
{

//forward declarations
class ColStaffObjs;


//---------------------------------------------------------------------------------------
// StaffObjsIterator: An iterator to traverse the ColStaffObjs. It is is just a wrapper
// for a ColStaffObjsIterator.
// Really needed? Replace by a typedef for ColStaffObjsIterator?
class StaffObjsIterator
{
protected:
    ColStaffObjs*              m_pColStaffObjs;
    ColStaffObjsIterator     m_it;

public:
    StaffObjsIterator(ColStaffObjs* pColStaffObjs);
    StaffObjsIterator(const StaffObjsIterator& it);
    ~StaffObjsIterator();


    inline bool is_first() { return m_it == m_pColStaffObjs->begin(); }
 //   inline bool LastOfCollection() {
 //                   return (m_pSO && m_pSO == m_pColStaffObjs->GetLastSO()); }
 //   inline bool StartOfCollection() {
 //                   return FirstOfCollection() || m_pColStaffObjs->IsEmpty(); }
    inline bool is_end() { return m_it == m_pColStaffObjs->end(); }

 //   inline bool ChangeOfMeasure() { return m_fChangeOfMeasure; }
	//inline lmStaffObj* GetCurrent() { return m_pSO; }
 //   inline int GetNumMeasure() { return (m_pSO ? m_pSO->GetSegment()->GetNumMeasure()
 //                                              : m_pColStaffObjs->GetNumSegments()-1 ); }
 //   inline void ResetFlags() { m_fChangeOfMeasure = false; }
 //   inline bool IsManagingCollection(lmColStaffObjs* pCSO) {
 //                   return pCSO == m_pColStaffObjs; }


    //access to prev/next element without changing iterator position
    inline ColStaffObjsEntry* next() { return m_it.next(); }
    inline ColStaffObjsEntry* prev() { return m_it.prev(); }


 //   void AdvanceToMeasure(int nBar);
    void first();
    inline void operator ++() { ++m_it; }
    inline void operator --() { --m_it; }
 //   void MoveLast();
 //   void MoveTo(lmStaffObj* pSO);

    inline int measure() const { return (*m_it)->measure(); }

    ColStaffObjsEntry* operator *() const { return *m_it; }


protected:

};


}   //namespace lomse

#endif      //__LOMSE_SCORE_ITERATOR_H__

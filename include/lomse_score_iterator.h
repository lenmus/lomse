//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_DOCUMENT_ITERATOR_H__
#define __LOMSE_DOCUMENT_ITERATOR_H__

#include <stack>

namespace lomse
{

//forward declarations
class ImoDocument;
class ImoObj;
class Document;

//---------------------------------------------------------------------------------------
// DocIterator: An iterator to traverse the first level children of a document, that
// is, to traverse the content items of ImDocument without traversing each child content.
//---------------------------------------------------------------------------------------
class DocIterator
{
protected:
    ImoDocument* m_pDoc;
    ImoObj* m_pCurItem;  //current pointed child
    int m_numContentItems;
    int m_curItemIndex;

public:
    DocIterator(Document* pDoc);

    //positioning
    void start_of_content();
    void last_of_content();
    inline void operator ++() { next(); }
    inline void operator --() { prev(); }

    //access to content
    inline ImoObj* operator *() { return m_pCurItem; }

protected:
    void next();
    void prev();
    void point_to_current();

};


}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_ITERATOR_H__

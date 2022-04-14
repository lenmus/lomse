//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_document_iterator.h"

#include "private/lomse_document_p.h"
#include "lomse_internal_model.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// DocIterator implementation
//---------------------------------------------------------------------------------------
DocIterator::DocIterator(Document* pDoc)
    : m_pDoc(pDoc->get_im_root())
{
    m_numContentItems = m_pDoc->get_num_content_items();
    m_curItemIndex = -1;
    next();
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
        m_pCurItem = nullptr;
}

//---------------------------------------------------------------------------------------
void DocIterator::start_of_content()
{
    m_curItemIndex = -1;
    next();
}

//---------------------------------------------------------------------------------------
void DocIterator::last_of_content()
{
    m_numContentItems = m_pDoc->get_num_content_items();
    m_curItemIndex = m_numContentItems - 2;
    next();
}


}  //namespace lomse

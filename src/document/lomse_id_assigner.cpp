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

#include "lomse_id_assigner.h"

#include <algorithm>        //for max, min
#include "lomse_ldp_elements.h"
#include "lomse_basic.h"
#include "lomse_internal_model.h"

namespace lomse
{

//---------------------------------------------------------------------------------------
IdAssigner::IdAssigner()
    : m_idCounter(0L)
{
}

//---------------------------------------------------------------------------------------
void IdAssigner::reassign_ids(LdpElement* pElm)
{
    LdpTree* tree(pElm);
    reassign_ids(tree);
}

//---------------------------------------------------------------------------------------
void IdAssigner::reassign_ids(LdpTree* pTree)
{

    long minId = find_min_id(pTree);
    if (minId != -1L)
    {
        long shift = m_idCounter - minId;
        shift_ids(pTree, shift);
    }
}

//---------------------------------------------------------------------------------------
void IdAssigner::shift_ids(LdpTree* pTree, long shift)
{
    long maxId = 0L;
    LdpTree::depth_first_iterator it;
    for (it = pTree->begin(); it != pTree->end(); ++it)
    {
        if (!(*it)->is_simple())
        {
            long id = (*it)->get_id() + shift;
            maxId = std::max(maxId, id);
            (*it)->set_id(id);
        }
    }
    m_idCounter = ++maxId;
}

//---------------------------------------------------------------------------------------
long IdAssigner::find_min_id(LdpTree* pTree)
{
    long minId = -1L;
    bool fFirstId = true;
    LdpTree::depth_first_iterator it;
    for (it = pTree->begin(); it != pTree->end(); ++it)
    {
        if (!(*it)->is_simple())
        {
            if (fFirstId)
            {
                fFirstId = false;
                minId = (*it)->get_id();
            }
            else
                minId = std::min(minId, (*it)->get_id());
        }
    }
    return minId;
}

//---------------------------------------------------------------------------------------
void IdAssigner::assign_id(ImoObj* pImo)
{
    if (pImo->get_id() == -1L)
        pImo->set_id(m_idCounter++);

    //children
    TreeNode<ImoObj>::children_iterator it;
    for (it = pImo->begin(); it != pImo->end(); ++it)
        assign_id(*it);

    //TODO: refactor. IdAssigner has knowledge about ImoAttachmnets internals
    //attachments
    //if (pImo->is_attachments())
    //{
    //    ImoAttachments* pA = static_cast<ImoAttachments*>(pImo);
    //    std::list<ImoAuxObj*>& attachments = pA->get_attachments();
    //    std::list<ImoAuxObj*>::iterator it;
    //    for(it = attachments.begin(); it != attachments.end(); ++it)
    //        assign_id(*it);
    //}
    if (pImo->is_relations())
    {
        ImoRelations* pR = static_cast<ImoRelations*>(pImo);
        std::list<ImoRelObj*>& relations = pR->get_relations();
        std::list<ImoRelObj*>::iterator it;
        for(it = relations.begin(); it != relations.end(); ++it)
            assign_id(*it);
    }


}


}  //namespace lomse

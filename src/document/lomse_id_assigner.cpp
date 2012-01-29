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
    SpLdpTree tree(pElm);
    reassign_ids(tree);
}

//---------------------------------------------------------------------------------------
void IdAssigner::reassign_ids(SpLdpTree pTree)
{

    long minId = find_min_id(pTree);
    if (minId != -1L)
    {
        long shift = m_idCounter - minId;
        shift_ids(pTree, shift);
    }
}

//---------------------------------------------------------------------------------------
void IdAssigner::shift_ids(SpLdpTree pTree, long shift)
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
long IdAssigner::find_min_id(SpLdpTree pTree)
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
    if (pImo->is_attachments())
    {
        ImoAttachments* pA = static_cast<ImoAttachments*>(pImo);
        std::list<ImoAuxObj*>& attachments = pA->get_attachments();
        std::list<ImoAuxObj*>::iterator it;
        for(it = attachments.begin(); it != attachments.end(); ++it)
            assign_id(*it);
    }


}


}  //namespace lomse

//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
//-------------------------------------------------------------------------------------

#include "lomse_basic_model.h"

#include "lomse_internal_model.h"


namespace lomse
{


//-------------------------------------------------------------------------------------
// InternalModel implementation
//-------------------------------------------------------------------------------------

InternalModel::InternalModel()
    : m_pRoot(NULL)
{
}

InternalModel::~InternalModel()
{
    if (m_pRoot)
        delete m_pRoot;
    delete_beams();
    delete_tuplets();
}

void InternalModel::delete_beams()
{
    std::list<ImoBeam*>::iterator it;
    for (it = m_beams.begin(); it != m_beams.end(); ++it)
        delete *it;
    m_beams.clear();
}

void InternalModel::delete_tuplets()
{
    std::list<ImoTuplet*>::iterator it;
    for (it = m_tuplets.begin(); it != m_tuplets.end(); ++it)
        delete *it;
    m_tuplets.clear();
}


}  //namespace lomse

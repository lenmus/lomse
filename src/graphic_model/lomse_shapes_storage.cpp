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

#include "lomse_shapes_storage.h"

#include "lomse_gm_basic.h"
#include "lomse_shape_beam.h"
#include "lomse_beam_engraver.h"


namespace lomse
{

//=======================================================================================
// ShapesStorage implementation
//=======================================================================================
ShapesStorage::~ShapesStorage()
{
	delete_ready_shapes();
}

void ShapesStorage::add_ready_shapes_to_model(GmoBox* pBox)
{
	std::list< pair<GmoShape*, int> >::iterator it;
    for (it = m_readyShapes.begin(); it != m_readyShapes.end(); ++it)
        pBox->add_shape(it->first, it->second);

    m_readyShapes.clear();
}

//---------------------------------------------------------------------------------------
void ShapesStorage::delete_ready_shapes()
{
	std::list< pair<GmoShape*, int> >::iterator it;
    for (it = m_readyShapes.begin(); it != m_readyShapes.end(); ++it)
        delete it->first;

    m_readyShapes.clear();
}

//---------------------------------------------------------------------------------------
Engraver* ShapesStorage::get_engraver(ImoObj* pImo)
{
    map<ImoObj*, Engraver*>::const_iterator it = m_engravers.find(pImo);
    if (it !=  m_engravers.end())
        return it->second;
    else
    {
        throw std::runtime_error( "ImoObj doesn't exists!" );
        return NULL;
    }
}

//---------------------------------------------------------------------------------------
void ShapesStorage::shape_ready_for_gmodel(ImoObj* pImo, int layer)
{
    Engraver* pEngrv = get_engraver(pImo);
    m_readyShapes.push_back( make_pair(pEngrv->get_shape(), layer) );
    remove_engraver(pImo);
}

//---------------------------------------------------------------------------------------
void ShapesStorage::delete_engravers()
{
	std::map<ImoObj*, Engraver*>::const_iterator it;
    for (it = m_engravers.begin(); it != m_engravers.end(); ++it)
        delete it->second;
}



}  //namespace lomse

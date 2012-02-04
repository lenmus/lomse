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
        return NULL;
}

//---------------------------------------------------------------------------------------
void ShapesStorage::shape_ready_for_gmodel(ImoObj* pImo, int layer)
{
    Engraver* pEngrv = get_engraver(pImo);
    m_readyShapes.push_back( make_pair(pEngrv->get_shape(), layer) );
    remove_engraver(pImo);
    delete pEngrv;
}

//---------------------------------------------------------------------------------------
void ShapesStorage::delete_engravers()
{
	std::map<ImoObj*, Engraver*>::const_iterator it;
    for (it = m_engravers.begin(); it != m_engravers.end(); ++it)
        delete it->second;
}



}  //namespace lomse

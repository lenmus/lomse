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

#include "lomse_sizers.h"

#include "lomse_gm_basic.h"
//#include <sstream>
//
//using namespace std;

namespace lomse
{

//-------------------------------------------------------------------------------------
// SizerChild implementation
//-------------------------------------------------------------------------------------

SizerChild::SizerChild(GmoBox* pBox, int factor, int alignment)
    : m_alignment(alignment)
    , m_stretchFactor(factor)
    , m_pControlledBox(pBox)
{
} 

SizerChild::~SizerChild()
{
}

void SizerChild::set_width(LUnits width) 
{   
    m_pControlledBox->set_width(width);
}

void SizerChild::set_height(LUnits height) 
{ 
    m_pControlledBox->set_height(height);
}

LUnits SizerChild::get_width() 
{ 
    return m_pControlledBox->get_width();
}

LUnits SizerChild::get_height() 
{ 
    return m_pControlledBox->get_height();; 
}


//-------------------------------------------------------------------------------------
// Sizer implementation
//-------------------------------------------------------------------------------------

Sizer::Sizer()
{
} 

Sizer::~Sizer()
{
    std::list<SizerChild*>::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
        delete *it;
    m_children.clear();
}



//-------------------------------------------------------------------------------------
// FlowSizer implementation
//-------------------------------------------------------------------------------------

FlowSizer::FlowSizer(int orientation)
    : Sizer()
    , m_orientation(orientation)
{
}

void FlowSizer::layout(LUnits width, LUnits height)
{
    if (is_vertical())
        layout_vertical(width, height);
    else
        layout_horizontal(width, height);
} 

void FlowSizer::layout_vertical(LUnits width, LUnits height)
{
    std::list<SizerChild*>::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->set_width(width);
        (*it)->set_height(_LOMSE_CAN_GROW);   
    }
} 

void FlowSizer::layout_horizontal(LUnits width, LUnits height)
{
    std::list<SizerChild*>::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->set_width(_LOMSE_CAN_GROW);
        (*it)->set_height(height);
    }
} 


}  //namespace lomse

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

#include "lomse_gm_basic.h"

//#include <sstream>
//
//using namespace std;

namespace lomse
{

//-------------------------------------------------------------------------------------
// Graphic model implementation
//-------------------------------------------------------------------------------------

GraphicModel::GraphicModel()
{
    m_root = new GmoBoxDocument();
} 

GraphicModel::~GraphicModel()
{
    if (m_root)
        delete m_root;
}

int GraphicModel::get_num_pages()
{
    return m_root->get_num_pages();
}



//-------------------------------------------------------------------------------------
// Graphic model objects (shapes and boxes) implementation
//-------------------------------------------------------------------------------------

GmoObj::GmoObj(GmoObj* owner)
    : m_pOwnerGmo(owner)
{
} 

GmoObj::~GmoObj()
{
}


//-------------------------------------------------------------------------------------
// GmoBox
//-------------------------------------------------------------------------------------

GmoBox* GmoBox::get_child_box(int i)  //i = 0..n-1
{      
    if (i < get_num_children())
        return m_childBoxes[i]; 
    else
        return NULL;
}


//-------------------------------------------------------------------------------------
// GmoBoxDocPage
//-------------------------------------------------------------------------------------

GmoBoxDocPage::GmoBoxDocPage(GmoObj* owner)
    : GmoBox(owner)
{
}


//-------------------------------------------------------------------------------------
// GmoBoxDocument
//-------------------------------------------------------------------------------------

GmoBoxDocument::GmoBoxDocument() 
    : GmoBox(NULL) 
{
    add_new_page();
}

GmoBoxDocPage* GmoBoxDocument::add_new_page()
{
    GmoBoxDocPage* pPage = new GmoBoxDocPage(this);
    pPage->set_number(get_num_pages()+1);
    add_child_box(pPage);
    return pPage;
}

GmoBoxDocPage* GmoBoxDocument::get_page(int i) 
{ 
    return dynamic_cast<GmoBoxDocPage*>(get_child_box(i));
}


//-------------------------------------------------------------------------------------
// GmoBoxScore
//-------------------------------------------------------------------------------------





}  //namespace lomse

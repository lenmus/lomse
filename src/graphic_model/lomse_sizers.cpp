//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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
    : m_minWidth(0.0f)
    , m_minHeight(0.0f)
    , m_marginLeft(0.0f)
    , m_marginTop(0.0f)
    , m_marginRight(0.0f)
    , m_marginBottom(0.0f)
    , m_alignment(alignment)
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

void FlowSizer::layout_vertical(LUnits width, LUnits UNUSED(height))
{
    std::list<SizerChild*>::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->set_width(width);
        (*it)->set_height(_LOMSE_CAN_GROW);
    }
}

void FlowSizer::layout_horizontal(LUnits UNUSED(width), LUnits height)
{
    std::list<SizerChild*>::iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
    {
        (*it)->set_width(_LOMSE_CAN_GROW);
        (*it)->set_height(height);
    }
}


}  //namespace lomse

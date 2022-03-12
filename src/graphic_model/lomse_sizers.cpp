//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

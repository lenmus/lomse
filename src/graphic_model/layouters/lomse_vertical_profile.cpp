//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#include "lomse_vertical_profile.h"

#include "lomse_gm_basic.h"

//for generating a debug shape
#include "lomse_shapes.h"
#include "lomse_vertex_source.h"
#include "lomse_logger.h"

#include <iostream>
using namespace std;


namespace lomse
{

//=======================================================================================
// VerticalProfile implementation
//=======================================================================================
VerticalProfile::VerticalProfile(LUnits xStart, LUnits xEnd, LUnits cellWidth,
                                 int numStaves)
    : m_numStaves(numStaves)
    , m_xStart(xStart)
    , m_xEnd(xEnd)
    , m_cellWidth(cellWidth)
{
    m_numCells = int( (xEnd - xStart) / cellWidth );
    if (m_numCells > 1000)
    {
        m_numCells = 1000;
    }

    m_yMax.resize(m_numStaves, nullptr);
    m_yMin.resize(m_numStaves, nullptr);

	m_yStaffTop.resize(m_numStaves, 0.0f);
	m_yStaffBottom.resize(m_numStaves, 0.0f);
}

//---------------------------------------------------------------------------------------
VerticalProfile::~VerticalProfile()
{
    for (int idxStaff=0; idxStaff < m_numStaves; ++idxStaff)
    {
        delete m_yMax[idxStaff];
        delete m_yMin[idxStaff];
    }
}

//---------------------------------------------------------------------------------------
void VerticalProfile::initialize(int idxStaff, LUnits yTop, LUnits yBottom)
{
    LUnits yCenter = (yTop + yBottom) / 2.0f;
    m_yStaffTop[idxStaff] = yTop;
    m_yStaffBottom[idxStaff] = yBottom;

    CellsRow* pCells = LOMSE_NEW CellsRow;
    pCells->assign(m_numCells, yCenter);
    m_yMax[idxStaff] = pCells;

    pCells = LOMSE_NEW CellsRow;
    pCells->assign(m_numCells, yCenter);
    m_yMin[idxStaff] = pCells;
}

//---------------------------------------------------------------------------------------
LUnits VerticalProfile::get_max_cell(int iCell, int idxStaff)
{
    return m_yMax[idxStaff]->at(iCell);
}

//---------------------------------------------------------------------------------------
LUnits VerticalProfile::get_min_cell(int iCell, int idxStaff)
{
    return m_yMin[idxStaff]->at(iCell);
}

//---------------------------------------------------------------------------------------
void VerticalProfile::update(GmoShape* pShape, int idxStaff)
{
    GmoCompositeShape* pCS = dynamic_cast<GmoCompositeShape*>(pShape);
    if (pCS)
    {
        list<GmoShape*>& shapes = pCS->get_components();
        list<GmoShape*>::iterator it;
        for (it=shapes.begin(); it != shapes.end(); ++it)
            update_shape(*it, idxStaff);
    }
    else
        update_shape(pShape, idxStaff);
}

//---------------------------------------------------------------------------------------
void VerticalProfile::update_shape(GmoShape* pShape, int idxStaff)
{
    if (pShape->is_shape_invisible())
        return;

    int iStart = cell_index( pShape->get_left() );
    int iEnd = cell_index( pShape->get_right() );
    LUnits yTop = pShape->get_top();
    LUnits yBottom = pShape->get_bottom();
    for (int j=iStart; j <= iEnd; ++j)
    {
        m_yMin[idxStaff]->at(j) = min(m_yMin[idxStaff]->at(j), yTop);
        m_yMax[idxStaff]->at(j) = max(m_yMax[idxStaff]->at(j), yBottom);
    }
}

//---------------------------------------------------------------------------------------
int VerticalProfile::cell_index(LUnits xPos)
{
    if (xPos <= m_xStart)
        return 0;

    return min(int( floor((xPos - m_xStart) / m_cellWidth) ), m_numCells-1);
}

//---------------------------------------------------------------------------------------
LUnits VerticalProfile::get_max_for(LUnits xStart, LUnits xEnd, int idxStaff)
{
    int i = cell_index(xStart);
    int iEnd = cell_index(xEnd);
    LUnits yMax = m_yMax[idxStaff]->at(i);
    for (++i; i <= iEnd; ++i)
    {
        yMax = max(yMax, m_yMax[idxStaff]->at(i));
    }
    return yMax;
}

//---------------------------------------------------------------------------------------
LUnits VerticalProfile::get_min_for(LUnits xStart, LUnits xEnd, int idxStaff)
{
    int i = cell_index(xStart);
    int iEnd = cell_index(xEnd);
    LUnits yMin = m_yMin[idxStaff]->at(i);
    for (++i; i <= iEnd; ++i)
    {
        yMin = min(yMin, m_yMin[idxStaff]->at(i));
    }
    return yMin;
}

//---------------------------------------------------------------------------------------
void VerticalProfile::dbg_add_vertical_profile_shapes(GmoBox* pBoxSystem)
{
    for (int idxStaff=0; idxStaff < m_numStaves; ++idxStaff)
    {
        pBoxSystem->add_shape( dbg_generate_shape(true, idxStaff), 0 );
        pBoxSystem->add_shape( dbg_generate_shape(false, idxStaff), 0 );
    }
}

//---------------------------------------------------------------------------------------
GmoShape* VerticalProfile::dbg_generate_shape(bool fMax, int idxStaff)
{
    GmoShapeDebug* pShape = LOMSE_NEW GmoShapeDebug(Color(255,0,0,128));

    LUnits base = (fMax ? -20.0f : 20.0f);
    CellsRow* pCells = (fMax ? m_yMax[idxStaff] : m_yMin[idxStaff]);
    LUnits xStart = m_xStart;
    LUnits yStart = pCells->at(0) + base;
    pShape->add_vertex('M', xStart, yStart);

    LUnits xLast = xStart;
    LUnits yLast = yStart;

    vector<LUnits>::iterator it;
    for (it=pCells->begin(); it != pCells->end(); ++it)
    {
        if (*it != yLast)
        {
            pShape->add_vertex('L', xLast, yLast);
            yLast = *it;
            pShape->add_vertex('L', xLast, yLast);
        }
        xLast += m_cellWidth;
    }
    pShape->add_vertex('L', xLast, yLast);
    pShape->add_vertex('L', xLast, yStart);
    pShape->add_vertex('Z',    0.0f,    0.0f);
    pShape->close_vertex_list();

    return pShape;
}


}  //namespace lomse

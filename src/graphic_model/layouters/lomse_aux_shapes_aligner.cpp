//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#include "lomse_aux_shapes_aligner.h"

#include "lomse_gm_basic.h"
#include "lomse_vertical_profile.h"

namespace lomse
{

//=======================================================================================
// AuxShapesAligner implementation
//=======================================================================================
AuxShapesAligner::AuxShapesAligner(LUnits xAbsLeft, LUnits xAbsRight)
    : m_xAbsLeft(xAbsLeft)
    , m_xAbsRight(xAbsRight)
{
}

//---------------------------------------------------------------------------------------
void AuxShapesAligner::add_shape(GmoShape* pShape)
{
    const LUnits xLeft = pShape->get_left();
    const LUnits xRight = pShape->get_right();

    //erase intersecting shapes
    auto it = m_shapes.upper_bound(xLeft);
    while (it != m_shapes.end())
    {
        const GmoShape* pExistingShape = it->second;

        if (pExistingShape->get_right() > xRight)
        {
            if (pExistingShape->get_left() < xRight)
                m_shapes.erase(it);
            break;
        }

        m_shapes.erase(it);
        it = m_shapes.upper_bound(xLeft);
    }

    //add new shape
    m_shapes.emplace(xRight, pShape);
}

//---------------------------------------------------------------------------------------
void AuxShapesAligner::align_shape_base_lines(LUnits maxAlignDistance, VerticalProfile* pVProfile, int idxStaff, bool fDown)
{
    if (m_shapes.empty())
        return;

    auto itGroupBegin = m_shapes.begin();
    LUnits yBaseline = itGroupBegin->second->get_baseline_y();
    LUnits xPrev = itGroupBegin->second->get_right();

    for (auto it = itGroupBegin; it != m_shapes.end(); ++it)
    {
        const GmoShape* pShape = it->second;
        const LUnits yShapeBaseline = pShape->get_baseline_y();

        if (pShape->get_left() - xPrev > maxAlignDistance)
        {
            set_base_line_for_range(itGroupBegin, it, yBaseline, pVProfile, idxStaff);
            itGroupBegin = it;
            yBaseline = yShapeBaseline;
        }
        else if (fDown)
        {
            if (yShapeBaseline > yBaseline)
                yBaseline = yShapeBaseline;
        }
        else
        {
            if (yShapeBaseline < yBaseline)
                yBaseline = yShapeBaseline;
        }

        xPrev = pShape->get_right();
    }

    //align the last group
    set_base_line_for_range(itGroupBegin, m_shapes.end(), yBaseline, pVProfile, idxStaff);
}

//---------------------------------------------------------------------------------------
void AuxShapesAligner::set_base_line_for_range(ShapeIterator rangeBegin, ShapeIterator rangeEnd,
                                              LUnits yBaseline, VerticalProfile* pVProfile, int idxStaff)
{
    for (auto it = rangeBegin; it != rangeEnd; ++it)
    {
        GmoShape* pShape = it->second;
        const LUnits yShift = yBaseline - pShape->get_baseline_y();

        if (yShift)
        {
            pShape->shift_origin(USize(0, yShift));
            pVProfile->update(pShape, idxStaff);
        }
    }
}

//---------------------------------------------------------------------------------------
GmoShape* AuxShapesAligner::find_shape(LUnits x) const
{
    auto it = m_shapes.upper_bound(x);

    if (it != m_shapes.end())
    {
        GmoShape* pShape = it->second;

        if (pShape->get_left() <= x)
            return pShape;
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------
LUnits AuxShapesAligner::find_nearest_free_point_left(LUnits x) const
{
    auto it = m_shapes.upper_bound(x);

    if (it != m_shapes.end())
    {
        const GmoShape* pShape = it->second;

        if (pShape->get_left() <= x)
            return pShape->get_left();
    }

    return x;
}

//---------------------------------------------------------------------------------------
LUnits AuxShapesAligner::find_nearest_free_point_right(LUnits x) const
{
    auto it = m_shapes.upper_bound(x);

    if (it != m_shapes.end())
    {
        const GmoShape* pShape = it->second;

        if (pShape->get_left() <= x)
            return pShape->get_right();
    }

    return x;
}

//---------------------------------------------------------------------------------------
LUnits AuxShapesAligner::find_nearest_occupied_point_left(LUnits x) const
{
    auto it = m_shapes.upper_bound(x);

    if (it != m_shapes.end() && it->second->get_left() <= x)
        return x;

    if (it == m_shapes.begin())
        return m_xAbsLeft;

    --it;
    return it->first;
}

//---------------------------------------------------------------------------------------
LUnits AuxShapesAligner::find_nearest_occupied_point_right(LUnits x) const
{
    auto it = m_shapes.upper_bound(x);

    if (it != m_shapes.end() && x <= it->second->get_left())
        return it->second->get_left();

    return x;
}

//=======================================================================================
// AuxShapesAlignersSystem implementation
//=======================================================================================
AuxShapesAlignersSystem::AuxShapesAlignersSystem(size_t numStaves, LUnits xAbsLeft, LUnits xAbsRight,
                                             LUnits maxAlignDistance)
    : m_alignersAbove(numStaves, { xAbsLeft, xAbsRight })
    , m_alignersBelow(numStaves, { xAbsLeft, xAbsRight })
    , m_maxAlignDistance(maxAlignDistance)
{
}

//---------------------------------------------------------------------------------------
void AuxShapesAlignersSystem::align_shape_base_lines(VerticalProfile* pVProfile)
{
    for (size_t i = 0; i < m_alignersAbove.size(); ++i)
    {
        m_alignersAbove[i].align_shape_base_lines(m_maxAlignDistance, pVProfile, i, false);
    }

    for (size_t i = 0; i < m_alignersBelow.size(); ++i)
    {
        m_alignersBelow[i].align_shape_base_lines(m_maxAlignDistance, pVProfile, i, true);
    }
}

}  //namespace lomse

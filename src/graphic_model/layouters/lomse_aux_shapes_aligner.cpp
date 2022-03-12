//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

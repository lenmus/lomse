//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_AUX_SHAPES_ALIGNER_H__
#define __LOMSE_AUX_SHAPES_ALIGNER_H__

#include "lomse_basic.h"

#include <map>
#include <vector>

namespace lomse
{

class GmoShape;
class VerticalProfile;

class AuxShapesAligner
{
protected:
    std::map<LUnits, GmoShape*> m_shapes;
    LUnits m_xAbsLeft;
    LUnits m_xAbsRight;

    using ShapeIterator = std::map<LUnits, GmoShape*>::const_iterator;

public:
    AuxShapesAligner(LUnits xAbsLeft, LUnits xAbsRight);

    void add_shape(GmoShape* pShape);
    void align_shape_base_lines(LUnits maxAlignDistance, VerticalProfile* pVProfile, int idxStaff, bool fDown);

    GmoShape* find_shape(LUnits x) const;
    LUnits find_nearest_free_point_left(LUnits x) const;
    LUnits find_nearest_free_point_right(LUnits x) const;
    LUnits find_nearest_occupied_point_left(LUnits x) const;
    LUnits find_nearest_occupied_point_right(LUnits x) const;

protected:
    void set_base_line_for_range(ShapeIterator rangeBegin, ShapeIterator rangeEnd,
                                 LUnits yBaseline, VerticalProfile* pVProfile, int idxStaff);
};

class AuxShapesAlignersSystem
{
protected:
    std::vector<AuxShapesAligner> m_alignersAbove;
    std::vector<AuxShapesAligner> m_alignersBelow;
    LUnits m_maxAlignDistance;

public:
    AuxShapesAlignersSystem(size_t numStaves, LUnits xAbsLeft, LUnits xAbsRight,
                          LUnits maxAlignDistance);

    AuxShapesAligner& get_aligner(int staff, bool fAbove) { return fAbove ? m_alignersAbove[staff] : m_alignersBelow[staff]; }
    void align_shape_base_lines(VerticalProfile* pVProfile);
};

}   //namespace lomse

#endif      //__LOMSE_AUX_SHAPES_ALIGNER_H__

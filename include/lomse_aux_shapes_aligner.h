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

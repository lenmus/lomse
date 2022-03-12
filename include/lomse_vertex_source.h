//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_VERTEX_SOURCE_H__        //to avoid nested includes
#define __LOMSE_VERTEX_SOURCE_H__


namespace lomse
{

//---------------------------------------------------------------------------------------
struct Vertex
{
    LUnits      ux_coord;
    LUnits      uy_coord;
    unsigned    cmd;
};

//---------------------------------------------------------------------------------------
class VertexSource
{
public:
    VertexSource() {}
	virtual ~VertexSource() {}

    virtual void rewind(unsigned pathId = 0) = 0;
    virtual unsigned vertex(double* px, double* py) = 0;
};


}   //namespace lomse

#endif    // __LOMSE_VERTEX_SOURCE_H__


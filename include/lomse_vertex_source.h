//---------------------------------------------------------------------------------------
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

    virtual void rewind(int pathId = 0) = 0;
    virtual unsigned vertex(double* px, double* py) = 0;
};


}   //namespace lomse

#endif    // __LOMSE_VERTEX_SOURCE_H__


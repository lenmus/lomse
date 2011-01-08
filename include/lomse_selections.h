//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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

#ifndef __LOMSE_SELECTIONS_H__
#define __LOMSE_SELECTIONS_H__

#include "lomse_basic.h"
#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class GraphicModel;
class GmoObj;


//---------------------------------------------------------------------------------------
class SelectionSet
{
protected:
    std::list<GmoObj*> m_objects;

public:
    SelectionSet();
    ~SelectionSet();

    void add(GmoObj* pGmo, unsigned flags=0);
    bool contains(GmoObj* pGmo);
    void clear();

};


}   //namespace lomse

#endif      //__LOMSE_SELECTIONS_H__

//--------------------------------------------------------------------------------------
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
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_BASIC_MODEL_H__        //to avoid nested includes
#define __LOMSE_BASIC_MODEL_H__

#include <list>

using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class ImoBeam;
class ImoTuplet;
class ImoTie;


// InternalModel: A container for the objects composing the internal model
//----------------------------------------------------------------------------------
class InternalModel
{
protected:
    ImoObj* m_pRoot;
    std::list<ImoBeam*> m_beams;
    std::list<ImoTuplet*> m_tuplets;
    std::list<ImoTie*> m_ties;

public:
    InternalModel();
    ~InternalModel();

    //building the model
    inline void set_root(ImoObj* pRoot) { m_pRoot = pRoot; }
    inline void add_beam(ImoBeam* pBeam) { m_beams.push_back(pBeam); }
    inline void add_tuplet(ImoTuplet* pTuplet) { m_tuplets.push_back(pTuplet); }
    inline void add_tie(ImoTie* pTie) { m_ties.push_back(pTie); }

    //getters
    inline ImoObj* get_root() { return m_pRoot; }
    inline std::list<ImoBeam*>& get_beams() { return m_beams; }
    inline std::list<ImoTuplet*>& get_tuplets() { return m_tuplets; }
    inline std::list<ImoTie*>& get_ties() { return m_ties; }

protected:
    void delete_beams();
    void delete_tuplets();

};


}   //namespace lomse

#endif    // __LOMSE_BASIC_MODEL_H__


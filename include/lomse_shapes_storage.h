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

#ifndef __LOMSE_SHAPES_STORAGE_H__        //to avoid nested includes
#define __LOMSE_SHAPES_STORAGE_H__

#include "lomse_basic.h"
#include <map>
#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class BeamEngraver;
class GmoShapeBeam;
class ImoBeam;
class GmoShape;
class GmoBox;
class Engraver;
class ImoObj;


// helper class to store shapes under construction and its engravers
//---------------------------------------------------------------------------------------
class ShapesStorage
{
protected:
	std::map<ImoObj*, Engraver*> m_engravers;
//	std::map<ImoBeam*, BeamEngraver*> m_beams;
	std::list< pair<GmoShape*, int> > m_readyShapes;

public:
    ShapesStorage() {}
    ~ShapesStorage() {}

//    //beam engravers
//    inline void save_beam_engraver(BeamEngraver* pEngrv, ImoBeam* pImo) {
//        m_beams[pImo] = pEngrv;
//    }
//    BeamEngraver* get_beam_engraver(ImoBeam* pImo);
//    inline void remove_beam_engraver(ImoBeam* pImo) { m_beams.erase(pImo); }
//    void beam_shape_ready_for_gmodel(ImoBeam* pImo);

    //engravers
    inline void save_engraver(Engraver* pEngrv, ImoObj* pImo) {
        m_engravers[pImo] = pEngrv;
    }
    Engraver* get_engraver(ImoObj* pImo);
    inline void remove_engraver(ImoObj* pImo) { m_engravers.erase(pImo); }
    void shape_ready_for_gmodel(ImoObj* pImo, int layer);


    //final shapes
    void add_ready_shapes_to_model(GmoBox* pBox);
    void delete_ready_shapes();

};


}   //namespace lomse

#endif    // __LOMSE_SHAPES_STORAGE_H__


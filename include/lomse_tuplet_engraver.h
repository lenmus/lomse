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

#ifndef __LOMSE_TUPLET_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_TUPLET_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class GmoShapeTuplet;
class GmoShapeBeam;
class ScoreMeter;
class ImoTuplet;
class ImoNote;
class GmoShapeNote;
class ImoNoteRest;
class GmoShape;
class ImoTextStyleInfo;


//---------------------------------------------------------------------------------------
class TupletEngraver : public Engraver
{
protected:
    int m_iInstr;
    int m_iStaff;
    GmoShapeTuplet* m_pTupletShape;
    string m_label;
    ImoTextStyleInfo* m_pStyle;
    ImoTuplet* m_pTuplet;
    std::list< pair<ImoNoteRest*, GmoShape*> > m_noteRests;

public:
    TupletEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~TupletEngraver();

    GmoShapeTuplet* create_shape(ImoObj* pCreatorImo, int iInstr, int iStaff,
                                 const string& label, ImoTextStyleInfo* pStyle);
    GmoShapeTuplet* create_shape(ImoTuplet* pTuplet, int iInstr, int iStaff);

    void layout_tuplet(GmoShapeBeam* pBeamShape);
    inline GmoShapeTuplet* get_tuplet_shape() { return m_pTupletShape; }

    void add_note_rest(ImoNoteRest* pNoteRest, GmoShape* pShape);

    ImoNoteRest* get_start_noterest() { return m_noteRests.front().first; }
    ImoNoteRest* get_end_noterest() { return m_noteRests.back().first; }


protected:
    GmoShapeTuplet* do_create_shape(ImoObj* pCreatorImo);
    void add_text_shape(ImoObj* pCreatorImo);
    LUnits tenths_to_logical(Tenths tenths);
    bool decide_if_show_bracket();
    bool decide_if_tuplet_placement_above();
    bool check_if_single_beamed_group();

    void compute_y_coordinates(GmoShapeBeam* pBeamShape);
    GmoShapeNote* get_first_note();
    GmoShapeNote* get_last_note();

    GmoShape* get_start_noterest_shape() { return m_noteRests.front().second; }
    GmoShape* get_end_noterest_shape() { return m_noteRests.back().second; }

    LUnits m_yStart, m_yEnd;

};


}   //namespace lomse

#endif    // __LOMSE_TUPLET_ENGRAVER_H__


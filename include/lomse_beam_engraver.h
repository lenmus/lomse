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

#ifndef __LOMSE_BEAM_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_BEAM_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class GmoShapeBeam;
class ScoreMeter;
class ImoBeam;
class ImoNote;
class GmoShapeNote;
class ImoNoteRest;
class GmoShape;


//---------------------------------------------------------------------------------------
class BeamEngraver : public Engraver
{
protected:
    int m_iInstr;
    int m_iStaff;
    GmoShapeBeam* m_pBeamShape;
    ImoBeam* m_pBeam;
    std::list< pair<ImoNoteRest*, GmoShape*> > m_noteRests;

public:
    BeamEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~BeamEngraver();

    GmoShapeBeam* create_shape(ImoObj* pCreatorImo, int iInstr, int iStaff);
    void fix_stems_and_reposition_rests();
    inline GmoShapeBeam* get_beam_shape() { return m_pBeamShape; }
    void add_note_rest(ImoNoteRest* pNoteRest, GmoShape* pShape);

protected:
    void reposition_rests();
    void decide_on_stems_direction();
    void change_stems_direction();

    bool m_fStemForced;     //at least one stem forced
    bool m_fStemMixed;      //not all stems in the same direction
    bool m_fStemsDown;      //stems direction down
    int m_numStemsDown;     //number of noteheads with stem down
    int m_numNotes;         //total number of notes
    int m_averagePosOnStaff;
};


}   //namespace lomse

#endif    // __LOMSE_BEAM_ENGRAVER_H__


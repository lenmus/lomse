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

#include "lomse_beam_engraver.h"

#include "lomse_glyphs.h"
#include "lomse_shape_beam.h"
#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_im_note.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// BeamEngraver implementation
//---------------------------------------------------------------------------------------
BeamEngraver::BeamEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : Engraver(libraryScope, pScoreMeter)
    , m_pBeamShape(NULL)
{
}

//---------------------------------------------------------------------------------------
BeamEngraver::~BeamEngraver()
{
    m_noteRests.clear();
}

//---------------------------------------------------------------------------------------
GmoShapeBeam* BeamEngraver::create_shape(ImoObj* pCreatorImo, int iInstr, int iStaff)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pBeam = NULL;

    LUnits uThickness = m_pMeter->tenths_to_logical(LOMSE_BEAM_THICKNESS, iInstr, iStaff);
    LUnits uSpacing = m_pMeter->tenths_to_logical(LOMSE_BEAM_SPACING, iInstr, iStaff)
                      + uThickness;
    LUnits uHookLength = m_pMeter->tenths_to_logical(LOMSE_BEAM_HOOK_LENGTH, iInstr, iStaff);

    m_pBeamShape = new GmoShapeBeam(pCreatorImo, uThickness, uSpacing, uHookLength);

    m_pShape = m_pBeamShape;
    return m_pBeamShape;
}

//---------------------------------------------------------------------------------------
void BeamEngraver::fix_stems_and_reposition_rests()
{
	//This method is invoked by the note engraver after engraving the
	//last note of a beamed group. It shifts rests to fit gracefuly inside the
    //group and fixes stem directions.

    reposition_rests();
    decide_on_stems_direction();
    change_stems_direction();
}

//---------------------------------------------------------------------------------------
void BeamEngraver::reposition_rests()
{
    //compute the average position of all noteheads.
    int numNotes = 0;
    int posForRests = 0;
    std::list< pair<ImoNoteRest*, GmoShape*> >::iterator it;
    for(it=m_noteRests.begin(); it != m_noteRests.end(); ++it)
	{
        if ((it->first)->is_note())
        {
            GmoShapeNote* pShapeNote = dynamic_cast<GmoShapeNote*>(it->second);
            numNotes++;
            posForRests += pShapeNote->get_pos_on_staff();
        }
    }
    posForRests /= numNotes;

    //to convert to tenths it is necesary to multiply by 10/2 = 5
    Tenths meanPos = Tenths(posForRests) * 5.0f;

    // As rests are normally positioned on 3rd space (35 tenths), the shift to apply is
    Tenths tShift = 35.0f - meanPos;
    USize shift(0.0f, m_pMeter->tenths_to_logical(tShift, m_iInstr, m_iStaff));

    //shift rests
    for(it=m_noteRests.begin(); it != m_noteRests.end(); ++it)
	{
        if ((it->first)->is_rest())
        {
            GmoShapeRest* pShapeRest = dynamic_cast<GmoShapeRest*>(it->second);
            pShapeRest->shift_origin(shift);
        }
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::decide_on_stems_direction()
{
    //look for the stem direction of most notes. If one note has its stem direction
    //forced (by a tie, probably) forces the beam stems in this direction

    m_fStemForced = false;     //assume no stem forced
    m_fStemMixed = false;      //assume all stems in the same direction
    m_fStemsDown = false;      //set stems up by default
    m_numStemsDown = 0;
    m_numNotes = 0;
    m_averagePosOnStaff = 0;

    std::list< pair<ImoNoteRest*, GmoShape*> >::iterator it;
    for(it=m_noteRests.begin(); it != m_noteRests.end(); ++it)
	{
        if ((it->first)->is_note())      //ignore rests
        {
		    ImoNote* pNote = dynamic_cast<ImoNote*>(it->first);
            m_numNotes++;

            if (pNote->get_stem_direction() != ImoNote::k_stem_default)
            {
                m_fStemForced = true;
                m_fStemsDown = pNote->is_stem_down();
            }
            else
            {
                GmoShapeNote* pNoteShape = dynamic_cast<GmoShapeNote*>(it->second);
                m_averagePosOnStaff += pNoteShape->get_pos_on_staff();
                GmoShapeStem* pStemShape = pNoteShape->get_stem_shape();
                if (pStemShape && pStemShape->is_stem_down())
                    m_numStemsDown++;
            }
        }
    }

    if (!m_fStemForced)
    {
        m_fStemsDown = m_averagePosOnStaff / m_numNotes > 6;
        m_fStemMixed = false;
    }
    else
    {
        m_fStemMixed = (m_numStemsDown !=0 && m_numStemsDown != m_numNotes);
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::change_stems_direction()
{
    //TODO
    ////correct beam position (and reverse stems direction) if first note of beamed
    ////group is tied to a previous note and the stems' directions are not forced
    //if (!m_fStemForced && m_noteRests.front()->is_note())
    //{
    //    ImoNote* pFirst = (ImoNote*)m_noteRests.front();
    //    if (pFirst->IsTiedToPrev())
    //        m_fStemsDown = pFirst->GetTiedNotePrev()->is_stem_down();
    //}

    //the beam line position is going to be established by the first and last
    //notes stems. Therefore, if stems are not prefixed, it is necessary to
    //change stem directions of notes
    if (!m_fStemForced)
    {
        std::list< pair<ImoNoteRest*, GmoShape*> >::iterator it;
        for(it=m_noteRests.begin(); it != m_noteRests.end(); ++it)
	    {
            if (it->first->is_note())
            {
                GmoShapeNote* pShape = dynamic_cast<GmoShapeNote*>(it->second);
                pShape->set_stem_down(m_fStemsDown);
            }
        }

        m_pBeamShape->set_beam_above(!m_fStemsDown);
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::add_note_rest(ImoNoteRest* pNoteRest, GmoShape* pShape)
{
	m_noteRests.push_back( make_pair(pNoteRest, pShape) );
    if (!m_pBeam)
        m_pBeam = pNoteRest->get_beam();
}


}  //namespace lomse

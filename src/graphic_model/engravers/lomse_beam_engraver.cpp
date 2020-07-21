//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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

#include "lomse_beam_engraver.h"

#include "lomse_glyphs.h"
#include "lomse_shape_beam.h"
#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_im_note.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_note_engraver.h"

#include <cmath>        // fabs


namespace lomse
{

//---------------------------------------------------------------------------------------
// BeamEngraver implementation
//---------------------------------------------------------------------------------------
BeamEngraver::BeamEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pBeamShape(nullptr)
    , m_pBeam(nullptr)
    , m_uBeamThickness(0.0f)
    , m_fBeamAbove(false)
    , m_fStemForced(false)
    , m_fStemsMixed(false)
    , m_fStemsDown(false)
    , m_fCrossStaff(false)
    , m_numStemsDown(0)
    , m_numNotes(0)
    , m_averagePosOnStaff(0)
    , m_maxStaff(0)
    , m_numLevels(1)
{
}

//---------------------------------------------------------------------------------------
BeamEngraver::~BeamEngraver()
{
}

//---------------------------------------------------------------------------------------
void BeamEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                      GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                      int UNUSED(iSystem), int UNUSED(iCol),
                                      LUnits UNUSED(xStaffLeft), LUnits UNUSED(xStaffRight),
                                      LUnits UNUSED(yStaffTop),
                                      int idxStaff, VerticalProfile* pVProfile)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pBeam = dynamic_cast<ImoBeam*>(pRO);

    ImoNoteRest* pNR = dynamic_cast<ImoNoteRest*>(pSO);
    m_noteRests.push_back( make_pair(pNR, pStaffObjShape) );

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;
}

//---------------------------------------------------------------------------------------
void BeamEngraver::set_middle_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                       GmoShape* pStaffObjShape, int UNUSED(iInstr),
                                       int UNUSED(iStaff), int UNUSED(iSystem),
                                       int UNUSED(iCol), LUnits UNUSED(xStaffLeft),
                                       LUnits UNUSED(xStaffRight), LUnits UNUSED(yStaffTop),
                                       int UNUSED(idxStaff),
                                       VerticalProfile* UNUSED(pVProfile))
{
    ImoNoteRest* pNR = dynamic_cast<ImoNoteRest*>(pSO);
    m_noteRests.push_back( make_pair(pNR, pStaffObjShape) );
}

//---------------------------------------------------------------------------------------
void BeamEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                    GmoShape* pStaffObjShape, int UNUSED(iInstr),
                                    int UNUSED(iStaff), int UNUSED(iSystem),
                                    int UNUSED(iCol), LUnits UNUSED(xStaffLeft),
                                    LUnits UNUSED(xStaffRight), LUnits UNUSED(yStaffTop),
                                    int UNUSED(idxStaff),
                                    VerticalProfile* UNUSED(pVProfile))
{
    ImoNoteRest* pNR = dynamic_cast<ImoNoteRest*>(pSO);
    m_noteRests.push_back( make_pair(pNR, pStaffObjShape) );
}

//---------------------------------------------------------------------------------------
GmoShape* BeamEngraver::create_first_or_intermediate_shape(Color color)
{
    //TODO: It has been assumed that a beam cannot be split. This has to be revised
    m_color = color;
    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoShape* BeamEngraver::create_last_shape(Color color)
{
    m_color = color;
    collect_information();
    decide_on_stems_direction();
    decide_beam_position();
    change_stems_direction();
    determine_number_of_beam_levels();

    if (m_fCrossStaff || m_fStemsMixed || m_fHasChords)
        beam_angle_and_stems_for_cross_staff_and_double_steamed_beams();
    else
        compute_beam_and_stems_for_simple_beams();

    reposition_rests();
    compute_beam_segments();
    create_shape();
    add_shape_to_noterests();
    return m_pBeamShape;
}

//---------------------------------------------------------------------------------------
void BeamEngraver::create_shape()
{
    m_pBeamShape = LOMSE_NEW GmoShapeBeam(m_pBeam, m_uBeamThickness, m_color);
    m_pBeamShape->set_layout_data(m_segments, m_origin, m_size,
                                  m_outerLeftPoint, m_outerRightPoint);
    m_pBeamShape->set_add_to_vprofile(!m_fStemsMixed && !m_fCrossStaff);
    m_pBeamShape->set_cross_staff(m_fCrossStaff);
}

//---------------------------------------------------------------------------------------
void BeamEngraver::add_shape_to_noterests()
{
    std::list< pair<ImoNoteRest*, GmoShape*> >::iterator it;
    for(it=m_noteRests.begin(); it != m_noteRests.end(); ++it)
        (it->second)->add_related_shape(m_pBeamShape);

    //set voice
    it = m_noteRests.begin();
    m_pBeamShape->set_voice( (it->first)->get_voice() );
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
            GmoShapeNote* pShapeNote = static_cast<GmoShapeNote*>(it->second);
            numNotes++;
            posForRests += pShapeNote->get_pos_on_staff();
        }
    }

    if (numNotes <= 0)   //sanity check. Never must be true
    {
        LOMSE_LOG_ERROR("No notes in beam!");
        return;
    }

    //to convert to tenths it is necesary to multiply by 10/2 = 5
    posForRests /= numNotes;
    Tenths meanPos = Tenths(posForRests) * 5.0f;

    // As rests are normally positioned on 3rd space (35 tenths), the shift to apply is
    Tenths tShift = 35.0f - meanPos;
    USize shift(0.0f, m_pMeter->tenths_to_logical(tShift, m_iInstr, m_iStaff));

    //shift rests
    for(it=m_noteRests.begin(); it != m_noteRests.end(); ++it)
	{
        if ((it->first)->is_rest())
        {
            GmoShapeRest* pShapeRest = static_cast<GmoShapeRest*>(it->second);
            pShapeRest->shift_origin(shift);
        }
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::collect_information()
{
    //In next loop we collect information:
    //- filter out the rests: collect all the notes in vector m_notes
	//- determine if it is a cross-staff beam (flag m_fCrossStaff)
	//- determine if it is a beam with mixed stems, some up and some down (flag m_fStemsMixed)
	//- determine if any stem is forced or all have default direction (m_fStemForced)
    //look for the stem direction of most notes. If one note has its stem direction
    //forced (by a tie, probably) forces the beam stems in this direction

    m_fHasChords = false;       //assume no chords in the beamed group
    m_fStemForced = false;      //assume no stem forced
    m_fStemsMixed = false;      //assume no mixed stems
    m_fCrossStaff = false;      //assume no cross staff beam
    m_fDefaultSteams = true;    //assume at least one stem with default position
    m_fStemsUp = true;          //Only valid if m_fStemsMixed==false

    m_numStemsDown = 0;
    m_numNotes = 0;
    m_averagePosOnStaff = 0;    //to determine majoritary stem direction
    m_maxStaff = 0;

    m_fStemsDown = false;       //set stems up by default (old code. to be removed)
    m_uBeamThickness = m_pMeter->tenths_to_logical(LOMSE_BEAM_THICKNESS, m_iInstr, m_iStaff);

    bool fLastForcedStemUp = false;
    int prevStaff = 0;
    std::list< pair<ImoNoteRest*, GmoShape*> >::iterator it;
    for(it=m_noteRests.begin(); it != m_noteRests.end(); ++it)
	{
        if ((it->first)->is_note())      //ignore rests
        {
		    ImoNote* pNote = static_cast<ImoNote*>(it->first);
            GmoShapeNote* pNoteShape = static_cast<GmoShapeNote*>((*it).second);
            if (pNoteShape->is_shape_chord_base_note())
            {
                m_fHasChords = true;
                pNoteShape = static_cast<GmoShapeChordBaseNote*>(pNoteShape)->get_flag_note();
            }
            m_note.push_back(pNoteShape);
            m_numNotes++;

            //compute m_fCrossStaff & m_maxStaff
            if (m_numNotes == 1)
                prevStaff = pNote->get_staff();
            else
            {
                int curStaff = pNote->get_staff();
                m_fCrossStaff |= (prevStaff != curStaff);
                m_maxStaff = max(m_maxStaff, curStaff);
                prevStaff = curStaff;
            }

            //compute m_fStemsMixed, m_fStemForced, m_fStemsUp, m_fDefaultSteams & m_fStemsDown
            if (!pNote->is_stem_default())
            {
                bool fStemUp = pNote->is_stem_up();
                if (m_numNotes > 1)
                {
                    m_fStemsMixed |= (fLastForcedStemUp != fStemUp);
                }
                fLastForcedStemUp = fStemUp;

                m_fStemForced = true;
                m_fStemsUp &= fStemUp;
                m_fDefaultSteams = false;
                m_fStemsDown = pNote->is_stem_down();
                //stem forced by last forced stem
            }
            else
            {
                m_averagePosOnStaff += pNoteShape->get_pos_on_staff();
                GmoShapeStem* pStemShape = pNoteShape->get_stem_shape();
                if (pStemShape && pStemShape->is_stem_down())
                    m_numStemsDown++;
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::decide_on_stems_direction()
{
    //At this point flags are set as follows:
    //  m_fStemForced: true if at laest one stem direction is forced
    //  m_fStemsMixed: true if there are forced stems in both directions
    //  m_fCrossStaff: true the beam has notes on different staves
    //  m_fStemsDown: either false or the direction of last forced stem
    //  m_fDefaultSteams: true if at least one stem with default position
    //  m_fStemsUp: only meaningfull if m_fStemsMixed==false. True if all stems
    //                forced up or default position

    //C1. If beam placement is not forced, it is determined by the number of notes
    //    above/bellow the staff middle line
    if (!m_fCrossStaff && !m_fStemForced && m_numNotes > 0)
        m_fStemsDown = (m_averagePosOnStaff / m_numNotes) > 6;

    //Cx. if all forced in the same direction this direction forces beam placement
                //stem forced by last forced stem
}

//---------------------------------------------------------------------------------------
void BeamEngraver::decide_beam_position()
{
    if (m_fCrossStaff || m_fStemsMixed)
    {
        //Cross-staff and double-steamed beams.
        m_fBeamAbove = !m_fStemsDown;
    }
    else
    {
        //Normal single staff beams.

        if (m_fStemForced)
        {
            //all stems in the same direction, as it is not mixed stems.
            //D3. When there is only one stem direction forced the beam placement is
            //    forced by it.
            //D4. When there are more than one stem forced and all forced in the
            //    same direction this direction forces beam placement.
            m_fBeamAbove = m_fStemsUp;
        }
        else
        {
            //all stems are default direction. Therefore, rules D1 & D2 apply.

            //determine max distance above/below middle line
            int maxAboveDistance = -1;       //-1 means no note above middle line
            int maxBelowDistance = -1;       //-1 means no note below middle line
            for (int i=0; i < m_numNotes; ++i)
            {
                int pos = m_note[i]->get_pos_on_staff() - 6;    // 6 is middle line
                if (pos > 0)
                    maxAboveDistance = max (maxAboveDistance, pos);
                else
                    maxBelowDistance = max(maxBelowDistance, -pos);
            }

            //D1. (Stone, p.50) The farthest note (extreme note) from the middle line of
            //    the staff determines the direction of all stems in the beamed group.
            if ((maxBelowDistance == -1) || (maxAboveDistance > maxBelowDistance))
            {
                m_fBeamAbove = false;
            }
            else if ((maxAboveDistance == -1) || (maxBelowDistance > maxAboveDistance))
            {
                m_fBeamAbove = true;
            }
            else
            {
                //D2. (Stone, p.50) If there are two extreme notes in opposite directions and
                //    at the same distance from middle line, then group will be stemmed down.
                m_fBeamAbove = false;
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::change_stems_direction()
{
    if (m_fCrossStaff || m_fStemsMixed || m_fHasChords)
    {
        //Cross-staff and double-steamed beams.
        //Old behaviour
        if (!m_fStemForced)
        {
            std::list< pair<ImoNoteRest*, GmoShape*> >::iterator it;
            for(it=m_noteRests.begin(); it != m_noteRests.end(); ++it)
            {
                if (it->first->is_note())
                {
                    GmoShapeNote* pShape = static_cast<GmoShapeNote*>(it->second);
//                    if (pShape->is_shape_chord_base_note())
//                    {
//                        pNoteShape = static_cast<GmoShapeChordBaseNote*>(pNoteShape)->get_flag_note();
//                    }
                    pShape->set_stem_down(m_fStemsDown);
                }
            }
        }
    }
    else
    {
        //Normal single staff beams.

        //TODO: BeamEngraver::change_stems_direction
        ////  Is this rule correct? I do not thing so.
        ////correct beam position (and reverse stems direction) if first note of beamed
        ////group is tied to a previous note and the stems' directions are not forced
        //if (!m_fStemForced && m_noteRests.front()->is_note())
        //{
        //    ImoNote* pFirst = (ImoNote*)m_noteRests.front();
        //    if (pFirst->IsTiedToPrev())
        //        m_fStemsDown = pFirst->GetTiedNotePrev()->is_stem_down();
        //}

        //Change the stems direction of notes unless they are forced.
        std::list< pair<ImoNoteRest*, GmoShape*> >::iterator it;
        for(it=m_noteRests.begin(); it != m_noteRests.end(); ++it)
        {
            if ((it->first)->is_note())      //ignore rests
            {
                ImoNote* pNote = static_cast<ImoNote*>(it->first);
                GmoShapeNote* pShape = static_cast<GmoShapeNote*>(it->second);
                if (pNote->is_stem_default())
                {
                    pShape->set_stem_down(!m_fBeamAbove);    //m_fStemsDown);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::determine_number_of_beam_levels()
{
    m_numLevels = 1;
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& beamData
        = m_pBeam->get_related_objects();
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    for(it = beamData.begin(); it != beamData.end(); ++it)
    {
        ImoBeamData* pBeamData = static_cast<ImoBeamData*>( (*it).second );
        int iLevel = 0;
        for (; iLevel < 6; iLevel++)
        {
            if (pBeamData->get_beam_type(iLevel) == ImoBeam::k_none)
                break;
        }
        m_numLevels = max(m_numLevels, iLevel);
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::compute_beam_segments()
{
	//AWARE: stems length are already trimmed

    LUnits uBeamHookLength = m_pMeter->tenths_to_logical(LOMSE_BEAM_HOOK_LENGTH, m_iInstr, m_iStaff);

    LUnits uStaffSpace = m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff);
    LUnits uStaffLine = m_pMeter->line_thickness_for_instr_staff(m_iInstr, m_iStaff);
    LUnits uBeamSpacing = m_uBeamThickness;
    if (m_numLevels <= 3)
        uBeamSpacing += uStaffSpace + (uStaffLine - 3.0f * m_uBeamThickness) / 2.0f;
    else if (m_numLevels == 4)
        uBeamSpacing += uStaffSpace + (uStaffLine - 4.0f * m_uBeamThickness) / 3.0f;
    else if (m_numLevels == 5)
        uBeamSpacing += uStaffSpace + (uStaffLine - 5.0f * m_uBeamThickness) / 4.0f;
    else
        uBeamSpacing += uStaffSpace + (uStaffLine - 6.0f * m_uBeamThickness) / 5.0f;

    LUnits uxPrev=0, uyPrev=0, uxCur=0, uyCur=0;    // points for previous and current note
    LUnits uyShift = 0;     // shift, to separate a beam line from the previous one

    bool fForwardPending = false;           //finish a Forward hook in prev note

    //For re-computing bounds
    m_origin.x = 100000000.0f;       //any too big value
    m_origin.y = 100000000.0f;      //any too big value
    m_size.width = 0.0f;
    m_size.height = 0.0f;

    LUnits uHalfBeam = m_uBeamThickness / 2.0f;

    //compute beam segments
    for (int iLevel=0; iLevel < 6; iLevel++)
	{
        LUnits uxStart=0, uxEnd=0, uyStart=0, uyEnd=0; // start and end points for a segment
        bool fStartPointComputed = false;
        bool fEndPointComputed = false;


        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& beamData
            = m_pBeam->get_related_objects();
        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itLD = beamData.begin();
        std::list< pair<ImoNoteRest*, GmoShape*> >::iterator itNR;
        for(itNR = m_noteRests.begin(); itNR != m_noteRests.end(); ++itNR, ++itLD)
        {
            ImoBeamData* pBeamData = static_cast<ImoBeamData*>( (*itLD).second );
            GmoShape* pNR = (*itNR).second;
            if (pNR->is_shape_note())
            {
                GmoShapeNote* pShapeNote = static_cast<GmoShapeNote*>(pNR);
                if (pNR->is_shape_chord_base_note())
                {
                    pShapeNote = static_cast<GmoShapeChordBaseNote*>(pNR)->get_flag_note();
                }

                uxCur = pShapeNote->get_stem_left();
                uyCur = pShapeNote->get_stem_y_flag() + uyShift;

                //Let's check if we have to finish a forward hook in prev. note
                if (fForwardPending)
                {
                    uxEnd = uxPrev + uBeamHookLength;
                    uyEnd = uyPrev + uBeamHookLength*(uyCur-uyPrev)/(uxCur-uxPrev);
                    add_segment(uxStart, uyStart, uxEnd, uyEnd);
                    update_bounds(uxStart, uyStart, uxEnd, uyEnd);
                    fForwardPending = false;
                }

                // now we can deal with current note
                switch ( pBeamData->get_beam_type(iLevel) )
                {
                    case ImoBeam::k_begin:
                        //start of segment. Compute initial point
                        fStartPointComputed = true;
                        uxStart = uxCur;
                        uyStart = uyCur;
                        break;

                    case ImoBeam::k_end:
                        // end of segment. Compute end point
                        fEndPointComputed = true;
                        uxEnd = uxCur;
                        uyEnd = uyCur;
                        break;

                    case ImoBeam::k_forward:
                        // start of segment. A forward hook is pending.
                        // compute its initial point
                        fForwardPending = true;
                        uxStart = uxCur;
                        uyStart = uyCur;
                        break;

                    case ImoBeam::k_backward:
                        // end of segment. compute start and end points
                        uxEnd = uxCur;
                        uyEnd = uyCur;
                        uxStart = uxCur - uBeamHookLength;
                        uyStart = uyPrev + (uxCur-uxPrev-uBeamHookLength)*(uyCur-uyPrev)/(uxCur-uxPrev);
                        fStartPointComputed = true;      //mark 'segment ready to be drawn'
                        fEndPointComputed = true;
                        break;

                    case ImoBeam::k_continue:
                    case ImoBeam::k_none:
                        // nothing to do.
                        break;
                }

                // if we have data to draw a segment, draw it
                if (fStartPointComputed && fEndPointComputed)
			    {
				    uxEnd += pShapeNote->get_stem_width();
                    add_segment(uxStart, uyStart, uxEnd, uyEnd);
                    update_bounds(uxStart, uyStart, uxEnd, uyEnd);
                    fStartPointComputed = false;
                    fEndPointComputed = false;
                    if (iLevel == 0)
                    {
                        if (m_fBeamAbove)
                        {
                            m_outerLeftPoint = UPoint(uxStart, uyStart - uHalfBeam);
                            m_outerRightPoint = UPoint(uxEnd, uyEnd - uHalfBeam);
                        }
                        else
                        {
                            m_outerLeftPoint = UPoint(uxStart, uyStart + uHalfBeam);
                            m_outerRightPoint = UPoint(uxEnd, uyEnd + uHalfBeam);
                        }
                    }
                }

                // save position of current note
                uxPrev = uxCur;
                uyPrev = uyCur;
            }
        }

        // displace y coordinate for next beam line
        uyShift += (m_fBeamAbove ? uBeamSpacing : - uBeamSpacing);
    }

    //take beam thickness into account for bounding box
    m_origin.y -= uHalfBeam;
    m_size.height += m_uBeamThickness;

    //adjust segments to make them relative to m_origin
    make_segments_relative();
}

//---------------------------------------------------------------------------------------
void BeamEngraver::add_segment(LUnits uxStart, LUnits uyStart, LUnits uxEnd, LUnits uyEnd)
{
    LUnits uSpace = m_uBeamThickness / 2.0f;
    LUnits uyShift = m_fBeamAbove ? uSpace : -uSpace;

    m_segments.push_back(uxStart);
    m_segments.push_back(uyStart + uyShift);
    m_segments.push_back(uxEnd);
    m_segments.push_back(uyEnd + uyShift);
}

//---------------------------------------------------------------------------------------
void BeamEngraver::update_bounds(LUnits uxStart, LUnits uyStart,
                                 LUnits uxEnd, LUnits uyEnd)
{
    LUnits left = min(uxStart, uxEnd);
    LUnits top = min(uyStart, uyEnd);
    LUnits bottom = max(uyStart, uyEnd);
    LUnits right = max(uxStart, uxEnd);

    m_origin.x = min(m_origin.x, left);
    m_origin.y = min(m_origin.y, top);

    m_size.width = max(m_size.width, right - m_origin.x);
    m_size.height = max(m_size.height, bottom - m_origin.y);
}

//---------------------------------------------------------------------------------------
void BeamEngraver::make_segments_relative()
{
    list<LUnits>::iterator it = m_segments.begin();
    while (it != m_segments.end())
    {
        *it -= m_origin.x;
        ++it;
        *it -= m_origin.y;
        ++it;
        *it -= m_origin.x;
        ++it;
        *it -= m_origin.y;
        ++it;
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::compute_beam_and_stems_for_simple_beams()
{
    int pos0 = m_note[0]->get_pos_on_staff();
    int posN = m_note[m_numNotes-1]->get_pos_on_staff();

    if (beam_must_be_horizontal(pos0, posN))
    {
        create_horizontal_beam_and_set_stems(pos0, posN);
    }
    else
    {
        float slant = assign_slant_to_beam(pos0, posN);
        assing_stem_length_to_outer_notes(slant, pos0, posN);
        if (m_numNotes > 2)
            assing_stem_length_to_inner_notes();
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::beam_angle_and_stems_for_cross_staff_and_double_steamed_beams()
{
    // At this point all stems have the standard size and the stem start and end points
    // are computed (start point = nearest to notehead)

    std::vector<LUnits> yNote(m_numNotes);
    std::vector<LUnits> yFlag(m_numNotes);

    //retrieve the start and end 'y' coordinates for each stem, and we store them in
    //the auxiliary arrays yNote and yFlag, respectively.
    for(int i=0; i < m_numNotes; ++i)
    {
        yNote[i] = m_note[i]->get_stem_y_note();
        yFlag[i] = m_note[i]->get_stem_y_flag();
    }
	int n = m_numNotes - 1;	// index to last element
	bool fAdjustIntermediateFlags = m_numNotes > 2;

    m_uBeamThickness = m_pMeter->tenths_to_logical(LOMSE_BEAM_THICKNESS, m_iInstr, m_iStaff);

	//determine beam angle (measured in staff spaces).
	//negative angle means that first note is lower pitch than last note.
    LUnits Ay = yFlag[n] - yFlag[0];

    //End of rules. Following code is for adjusting the stems for the remaing notes
    if (fAdjustIntermediateFlags)
    {
        // In following loop we compute each stem and update its final position.
        // Beam line position is established by the first and last notes' stems. Now
        // let's adjust the intermediate notes' stem lengths to end up in the beam line.
        // This is just a proportional share based on line slope:
        // If (x1,y1) and (xn,yn) are, respectively, the position of first and last notes of
        // the group, the y position of any intermediate note i can be computed as:
        //     Ay = yn-y1
        //     Ax = xn-x1
        //                Ay
        //     yi = y1 + ---- (xi-x1)
        //                Ax
        //
        // The loop is also used to look for the shortest stem
        LUnits x1 = m_note[0]->get_stem_left();
        LUnits xn = m_note[n]->get_stem_left();
        LUnits Ax = xn - x1;
        LUnits uMinStem = 0.0f;
        for(int i=0; i < m_numNotes; i++)
        {
            yFlag[i] = yFlag[0] + (Ay * (m_note[i]->get_stem_left() - x1)) / Ax;

            //compute stem length. For chords we have to substract the stem segment joining
            //all chord notes. This extra length is zero for notes not in chord
            LUnits uStemLength = fabs(yNote[i] - yFlag[i]) - m_note[i]->get_stem_extra_length();

            //save the shortest stem
            if (i==0)
                uMinStem = uStemLength;
            else
                uMinStem = min(uMinStem, uStemLength);
        }

        // If the pitch of any intermediate note is out of the interval formed by
        // the first note and the last one, then its stem could be too short.
        // For example, consider a beamed group of three notes, the first and the last
        // ones D4 and the middle  one G4; the beam is horizontal, nearly the G4 line;
        // so the midle notehead (G4) would be positioned just on the beam line.
        // To avoid this problem all stems are forced to have a minimum length

        LUnits dyStem = (m_note[0]->get_stem_height() + m_note[n]->get_stem_height()) / 2.0f;
        LUnits dyMin = (2.0f * dyStem) / 3.0f;
        bool fAdjust;

        // compare the shortest with this minimun required
        LUnits uyIncr;
        if (uMinStem < dyMin)
        {
            // a stem is smaller than dyMin. Increment all stems.
            uyIncr = dyMin - uMinStem;
            fAdjust = true;
        }
        else if (uMinStem > dyStem)
        {
            // all stems are greater than the standard size. Reduce them.
            //I'm not sure if this case is possible. But it is simple to deal with it
            uyIncr = -(uMinStem - dyStem);
            fAdjust = true;
        }
        else
        {
            fAdjust = false;
        }

        if (fAdjust)
        {
            for (int i = 0; i < m_numNotes; i++)
            {
                if (yNote[i] < yFlag[i])
                    yFlag[i] += uyIncr;
                 else
                    yFlag[i] -= uyIncr;
            }
        }

        // At this point stems' lengths are computed and adjusted.
        // Transfer the computed values to the stem shape
        for(int i=0; i < m_numNotes; i++)
        {
            m_note[i]->set_stem_length( fabs(yFlag[i] - yNote[i]) );
        }
    }
}

//---------------------------------------------------------------------------------------
bool BeamEngraver::has_repeated_pattern_of_pitches()
{
    if (m_numNotes == 2)
        return false;   //only two notes
    if (m_numNotes %2 == 1)
        return false;   //odd number of notes

    //even number of notes, at least four
    size_t maxSize = m_numNotes / 2;
    bool fPattern = false;
    for (size_t groupSize=2; groupSize <= maxSize; ++groupSize)
    {
        if (m_numNotes % groupSize == 0)
        {
            fPattern = true;
            for (size_t i=0; i < maxSize; i+=groupSize)
            {
                for (size_t j=i; j < i+groupSize; ++j)
                {
                    fPattern &= m_note[j]->get_pos_on_staff() == m_note[j+groupSize]->get_pos_on_staff();
                    if (!fPattern)
                        break;
                }
                if (!fPattern)
                    break;
            }
            if (fPattern)
                break;
        }
    }
    return fPattern;
}

//---------------------------------------------------------------------------------------
bool BeamEngraver::check_all_notes_outside_first_ledger_line()
{
    bool fAllOutside = true;
    for (int i=0; i < m_numNotes && fAllOutside; ++i)
    {
        int pos = m_note[i]->get_pos_on_staff();
        fAllOutside &= (pos < 0 || pos > 12);
    }
    return fAllOutside;
}

//---------------------------------------------------------------------------------------
void BeamEngraver::increment_cross_staff_stems(LUnits yIncrement)
{
    list< pair<ImoNoteRest*, GmoShape*> >::iterator it;
    for(it = m_noteRests.begin(); it != m_noteRests.end(); ++it)
    {
        ImoNoteRest* pNR = (*it).first;
        if (pNR->is_note())
        {
            GmoShapeNote* pShapeNote = static_cast<GmoShapeNote*>((*it).second);
            if (pShapeNote->is_shape_chord_base_note())
                pShapeNote = static_cast<GmoShapeChordBaseNote*>(pShapeNote)->get_flag_note();

            pShapeNote->increment_stem_length(yIncrement);
        }
    }
}

//---------------------------------------------------------------------------------------
float BeamEngraver::assign_slant_to_beam(int pos0, int posN)
{
    //Rules for assigning the slant when the beam is not horizontal

    float slant = 0.25f;    //minimun slant: 1/4 space
    int intval = abs(pos0 - posN);
    if (check_all_notes_outside_first_ledger_line())
    {
        //R5. When all the notes fall outside first ledger line, the beam takes only
        //    a slight slope. Intervals of a second take a slope of 0.25sp; all wider
        //    intervals take a slope of 0.5sp.
        if (intval > 1)
            slant = 0.5f;
    }
    else
    {
        //R3: Limit slant depending on distance (in spaces) between outer notes.
        LUnits distance = m_note[m_numNotes-1]->get_stem_left() - m_note[0]->get_stem_left();
        LUnits staffSpace = m_pMeter->tenths_to_logical(10.0f, m_iInstr, m_iStaff);
        distance /= staffSpace;
        if (distance > 20.0f)
            slant = 2.0f;
        else if (distance > 13.75f)
            slant = 1.50f;
        else if (distance > 7.5f)
            slant = 1.25f;
        else if (distance > 6.0f)
            slant = 1.0f;
        else if (distance > 4.5f)
            slant = 0.5f;

        //R4. Limit slant for small intervals
        if (intval == 1 && slant > 0.5f)   //2nd
            slant = 0.50f;
        else if (intval == 2 && slant > 1.0f)    //3rd
            slant = 1.0f;
        else if (intval == 3 && slant > 1.5f)    //4th
            slant = 1.5f;
        else if (intval == 4 && slant > 2.0f)    //5th
            slant = 2.0f;

        //R6. Slant when three or more beams should be 1.0
        if (m_numLevels >= 3)
            slant = 1.0f;
    }


    //beam going down ==> negative slant
    if (pos0 > posN)
        slant = -slant;

    return slant;
}

//---------------------------------------------------------------------------------------
bool BeamEngraver::beam_must_be_horizontal(int pos0, int posN)
{
    //Returns true if the beam should be horizontal, according to traditional
    //engraving rules

    //R2a. The beam is horizontal when the group begins and ends with the same note
    bool fHorizontal = (pos0 == posN);

    //R3b. The beam is horizontal when there is a repeated pattern of pitches
    if (!fHorizontal && m_numNotes > 2)
        fHorizontal = has_repeated_pattern_of_pitches();


    //R2c. The beam is horizontal when an inner note is closer to the beam than
    //     either of the outer notes
    if (!fHorizontal && m_numNotes > 2)
    {
        //check if any intermediate note is placed nearer than both outer notes
        if (m_fBeamAbove)
        {
            int refPos = max(pos0, posN);
            for(int i=1; i < m_numNotes-1; i++)
                fHorizontal |= (m_note[i]->get_pos_on_staff() > refPos);
        }
        else
        {
            int refPos = min(pos0, posN);
            for(int i=1; i < m_numNotes-1; i++)
                fHorizontal |= (m_note[i]->get_pos_on_staff() < refPos);
        }
    }

    return fHorizontal;
}

//---------------------------------------------------------------------------------------
float BeamEngraver::get_staff_length_for_beam(int iNote)
{
    //Returns the lenght of the stem, in staff spaces

    int posOnStaff = m_note[iNote]->get_pos_on_staff();
    LUnits stemLength = 3.5f;   //default stem lengh: one octave

    if (m_numLevels < 3)
    {
        //A1. Beams on the staff. Notes with stem up between the previous space to
        //    first upper  ledger line (b5 in G clef) and middle line or notes with
        //    stem down between the previous space to first lower ledger line (b3 in
        //    G clef) and middle line: they have the normal length of one octave (3.5
        //    spaces), but adjusted as above described, that is, notes on a line have
        //    a stem of 3.25 spaces and notes on a space have 3.5 spaces except for
        //    notes with stem up on 2nd space or with stem down on 3rd space, that
        //    has a stem length of 3.0 spaces (K.Stone rules).
        if ((m_fBeamAbove && posOnStaff > -1 && posOnStaff <= 6) ||
            (!m_fBeamAbove && posOnStaff >= 6 && posOnStaff <= 13) )
        {
            if (abs(posOnStaff) % 2 == 1)    //note on space
            {
                if ((m_fBeamAbove && posOnStaff == 5) || (!m_fBeamAbove && posOnStaff == 7))
                    stemLength = 3.0;
                else
                    stemLength = 3.5;
            }
            else
            {
                stemLength = 3.25;
            }
        }


        //A3. Beams out the staff. Notes with stems upwards from 3rd space inclusive
        //    (c5 in G) or with stems downwards from 2nd line inclusive (g4 in G)
        //    have a length of 2.5 spaces, except a4 & c5 that have 2.75 (test 208).
        else if ((m_fBeamAbove && posOnStaff > 6) || (!m_fBeamAbove && posOnStaff < 6))
        {
            if (posOnStaff == 5 || posOnStaff == 7)
                stemLength = 2.75;
            else
                stemLength = 2.5;
        }


        //A2. Far notes with beams in middle line. Notes with stem up on or below the
        //    second lower ledger line (a3 in G) or notes with stem down on or above
        //    the second upper ledger line: the end of stem have to touch the middle
        //    staff line. But as they are beamed and the beam straddles the middle
        //    line, it is necessary to increment stem by 0.25 spaces
        else
        {
            stemLength = abs(float(0.5 * (6 - posOnStaff)));     // touch middle line
            stemLength += 0.25;
        }

        //A4. Minimum space between notehead and beam is 1.5 spaces. This implies a
        //    minimum stem lenght of 2.5 spaces for one beam and 3.0 spaces for two
        //    beams. For each additional beam, the stem must be extended one space.
        if (m_numLevels > 1 && stemLength < 3.0f)
            stemLength = 3.0f;
        if (m_numLevels > 2)
            stemLength += (m_numLevels - 2);
    }

    else    //three or more beams
    {
        //A5. (Replaces A1, A2 & A3 when three or more beams). The first beam always
        //    sits/hangs. Therefore, if note on line assign it 3.0 spaces, else 2.5 sp.
        stemLength = (abs(posOnStaff % 2) == 0 ? 3.0f : 2.5f);

        //A4. Minimum space between notehead and beam is 1.5 spaces. This implies a
        //    minimum stem lenght of 2.5 spaces for one beam and 3.0 spaces for two
        //    beams. For each additional beam, the stem must be extended one space.
        //
        //    For notes on spaces we can not use 3.0 as that will place the beam in
        //    an invalid place. Therefore we must chosee betwen 2.5 or 3.5. So for more
        //    than 2 beams I will deal only with the second part of A4 rule.
        if (m_numLevels > 2)
            stemLength += (m_numLevels - 2);
    }




    return stemLength;  //in spaces
}

//---------------------------------------------------------------------------------------
void BeamEngraver::create_horizontal_beam_and_set_stems(int pos0, int posN)
{
    //determine nearest note to the beam
    int n = m_numNotes - 1;	        // index to last note
    int iNote = 0;
    if (m_fBeamAbove)
    {
        int maxPos = max(pos0, posN);
        iNote = (pos0 > posN ? 0 : n);
        for(int i=1; i < n; i++)
        {
            int pos = m_note[i]->get_pos_on_staff();
            if (pos > maxPos)
            {
                maxPos = pos;
                iNote = i;
            }
        }
    }
    else
    {
        int minPos = min(pos0, posN);
        iNote = (pos0 < posN ? 0 : n);
        for(int i=1; i < n; i++)
        {
            int pos = m_note[i]->get_pos_on_staff();
            if (pos < minPos)
            {
                minPos = pos;
                iNote = i;
            }
        }
    }

    //assign stem length to the nearest note
    LUnits staffSpace = m_pMeter->tenths_to_logical(10.0f, m_iInstr, m_iStaff);
    m_note[iNote]->set_stem_length( get_staff_length_for_beam(iNote) * staffSpace );
    LUnits yFlag = m_note[iNote]->get_stem_y_flag();

    //assign stem length to the other stems
    for(int i=0; i < m_numNotes; i++)
    {
        if (i != iNote)
            m_note[i]->set_stem_length( fabs(yFlag - m_note[i]->get_stem_y_note()) );
    }
}

//---------------------------------------------------------------------------------------
void BeamEngraver::assing_stem_length_to_outer_notes(float slant, int pos0, int posN)
{
    //slant is the desired slant (0.25 | 0.5 | 1.0 | 1.25 | 1.50 | 2.0) in spaces.
    //slant is negative when beam going down

    //make slant always positive for comparisons
    slant = abs(slant);

    //identify the outer note having the shortest stem
    int posS = 0;   //posOnStaff for outer note with shortest stem
    int iS = 0;     //index for outer note with shortest stem
    int posL = 0;   //posOnStaff for outer note with longest stem
    int iL = 0;     //index for outer note with longest stem
    if ((pos0 < posN && m_fBeamAbove) || (pos0 > posN && !m_fBeamAbove))
    {
        posS = posN;
        iS = m_numNotes - 1;
        posL = pos0;
        iL = 0;
    }
    else
    {
        posS = pos0;
        iS = 0;
        posL = posN;
        iL = m_numNotes - 1;
    }

    float stemS = 0.0f;     //shortest stem length (in spaces)
    float stemL = 0.0f;     //longest stem length (in spaces)


    if (m_numLevels < 3)
    {
        //C1. Both ends of a slanted beam should be attached to a stave-line according the
        //    following procedure:
        //
        //  1. Assign length to shortest stem note:
        //     If note on line assign it 3.0 spaces, else assign 2.5 spaces.
        //
        //  2. Assign length to longest stem note:
        //     - for slant < 1.0 stem must be on the same line than shorter stem note. Therefore:
        //              stem2 = stem1 + abs(pos2-pos1)*0.50
        //     - for 1.0 <= slant < 2.0 there must be one line of difference. Therefore:
        //              stem2 = stem1 + abs(pos2-pos1)*0.50 - 1.0
        //     - for slant = 2 there must be two lines of difference. Therefore:
        //              stem2 = stem1 + abs(pos2-pos1)*0.50 - 2.0
        //
        //  3. Increment shortest stem: For slant 2.0 increment it by 0.25.
        //
        //  4. Increment longest stem:
        //     - for slant 0.25 increment 0.25
        //     - for slant 0.50 increment 0.50
        //     - for slant 1.00 increment 0
        //     - for slant 1.25 increment 0.25
        //     - for slant 1.50 increment 0.50
        //     - for slant 2.00 increment 0.25

        //1. Assign length to shortest stem note:
        //   If note on line assign it 3.0 spaces, else assign 2.5 spaces.
        stemS = (abs(posS % 2) == 0 ? 3.0f : 2.5f);

        //  2. Assign length to longest stem note:
        //     - for slant < 1.0 stem must be on the same line than shorter stem note. Therefore:
        //              stem2 = stem1 + abs(pos2-pos1)*0.50
        //     - for 1.0 <= slant < 2.0 there must be one line of difference. Therefore:
        //              stem2 = stem1 + abs(pos2-pos1)*0.50 - 1.0
        //     - for slant = 2 there must be two lines of difference. Therefore:
        //              stem2 = stem1 + abs(pos2-pos1)*0.50 - 2.0
        //
        stemL = stemS + abs(posL-posS) * 0.50f;
        if (slant == 2.0f)
            stemL -= 2.0f;
        else if (slant >= 1.0f)
            stemL -= 1.0f;

        //3. Increment longest stem: For slant 1.0 or 2.0 increment it by 0.25.
        if (slant == 1.0f || slant == 2.0f)
            stemL += 0.25f;

        //4. Increment shortest stem:
        //   - for slant 0.25 increment 0.25
        //   - for slant 0.50 increment 0.50
        //   - for slant 1.00 increment 0.25
        //   - for slant 1.25 increment 0.25
        //   - for slant 1.50 increment 0.50
        //   - for slant 2.00 increment 0.25
        if (slant == 0.50f || slant == 1.50f)
            stemS += 0.50f;
        else
            stemS += 0.25f;
    }

    else    //three or more beams
    {
        //C2  (Replaces C1 when three or more beams). The first beam always
        //    sits/hangs. Therefore:
        //
        //    1. Assign length to shortest stem note:
        //       If note on line assign it 3.0 spaces, else assign 2.50 spaces.
        //
        //    2. Assign length to longest stem note. as slant is 1.0 there must be one line of difference. Therefore:
        //            stem2 = stem1 + abs(pos2-pos1)*0.50 -1.0;
        stemS = (abs(posS % 2) == 0 ? 3.0f : 2.5f);
        stemL = stemS + abs(posL-posS) * 0.50f - 1.0f;

        //A4. Minimum space between notehead and beam is 1.5 spaces. This implies a
        //    minimum stem lenght of 2.5 spaces for one beam and 3.0 spaces for two
        //    beams. For each additional beam, the stem must be extended one space.
        //
        //At this point, stemS is at least 2.5 and was never decremented. Therefore we
        //only have to deal with more than two beams
        LUnits incr = (m_numLevels - 2);
        stemS += incr;
        stemL += incr;
    }

    //transfer stems to notes
    LUnits staffSpace = m_pMeter->tenths_to_logical(10.0f, m_iInstr, m_iStaff);
    m_note[iS]->set_stem_length(stemS * staffSpace);
    m_note[iL]->set_stem_length(stemL * staffSpace);
}

//---------------------------------------------------------------------------------------
void BeamEngraver::assing_stem_length_to_inner_notes()
{
    //the outer notes have the stem length correctly set

    // In following loop we compute each stem and assign it to the inner notes.
    // Beam line position is established by the outer notes' stems. Now
    // let's adjust the intermediate notes' stem lengths to end up in the beam line.
    // This is just a proportional share based on line slope:
    // If (x0,y0) and (xn,yn) are, respectively, the position of first and last notes of
    // the group, the y position of any intermediate note i can be computed as:
    //     Ay = yn-y0
    //     Ax = xn-x0
    //                Ay
    //     yi = y0 + ---- (xi-x0)
    //                Ax
    //
    int n = m_numNotes - 1;

    LUnits x0 = m_note[0]->get_stem_left();
    LUnits Ax = m_note[n]->get_stem_left() - x0;

    LUnits yFlag0 = m_note[0]->get_stem_y_flag();
    LUnits Ay = m_note[n]->get_stem_y_flag() - yFlag0;

    for(int i=1; i < n; i++)
    {
        LUnits yFlag = yFlag0 + (Ay * (m_note[i]->get_stem_left() - x0)) / Ax;
        LUnits yNote = m_note[i]->get_stem_y_note();
        LUnits stem = fabs(yNote - yFlag) + m_note[i]->get_stem_extra_length();
        m_note[i]->set_stem_length(stem);
        //extraLenght is always 0 !!!
    }
}


}  //namespace lomse

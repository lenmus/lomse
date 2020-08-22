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

#ifndef __LOMSE_BEAM_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_BEAM_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_vertical_profile.h"

#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class GmoShape;
class GmoShapeBeam;
class GmoShapeNote;
class ImoBeam;
class ImoNote;
class ImoNoteRest;
class ImoObj;
class ScoreMeter;
class VerticalProfile;


//---------------------------------------------------------------------------------------
/** SegmentData: Helper struct for storing data about one beam segment
*/
struct SegmentData
{
    int iLevel;                 //segment level: 0..5. 0=ppal, 1=next level, etc.
    LUnits xStart;
    LUnits yStart;
    LUnits xEnd;
    LUnits yEnd;
    GmoShapeNote* pStartNote;   //nullptr for forward hook, == pEndNote for backward hook
    GmoShapeNote* pEndNote;
    int position;               //from enum EComputedBeam
    bool fOpposite;             //segment with outher notes in opposite direction

    SegmentData(int level, LUnits xs, LUnits ys, LUnits xe, LUnits ye, GmoShapeNote* pSN,
                GmoShapeNote* pEN, int pos)
        : iLevel(level)
        , xStart(xs)
        , yStart(ys)
        , xEnd(xe)
        , yEnd(ye)
        , pStartNote(pSN)
        , pEndNote(pEN)
        , position(pos)
        , fOpposite(false)
    {
    }
};


//---------------------------------------------------------------------------------------
class BeamEngraver : public RelObjEngraver
{
protected:
    GmoShapeBeam* m_pBeamShape;
    ImoBeam* m_pBeam;
    std::list< pair<ImoNoteRest*, GmoShape*> > m_noteRests;

    std::list<LUnits> m_segments;
    UPoint m_origin;
    USize m_size;
    LUnits m_uBeamThickness;
    int m_beamPos;              //computed beam position. Value from enum EComputedBeam

    bool m_fDoubleStemmed;      //stems forced: not all stems in the same direction
    bool m_fCrossStaff;         //the flag notes are on both staves
    bool m_fGraceNotes;         //it is a beam with grace notes
    bool m_fChord;              //it is a beam with chords
    int m_numNotes;             //total number of notes
    int m_maxStaff;     //for cross-staff beams, the highest staff. For normal beams, just the staff
    int m_minStaff;     //for cross-staff beams, the lowest staff. For normal beams, just the staff
    int m_numLevels;    //number of beam levels for this beam;
    std::vector<GmoShapeNote*> m_note;      //shapes for notes. Rests removed.

public:
    BeamEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~BeamEngraver();

    //implementation of virtual methods from RelObjEngraver
    void set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol,
                            LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                            int idxStaff, VerticalProfile* pVProfile) override;
    void set_middle_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                             GmoShape* pStaffObjShape, int iInstr, int iStaff,
                             int iSystem, int iCol,
                             LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                             int idxStaff, VerticalProfile* pVProfile) override;
    void set_end_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol,
                          LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                          int idxStaff, VerticalProfile* pVProfile) override;

    //RelObjEngraver mandatory overrides
    void set_prolog_width(LUnits UNUSED(width)) override {}
    GmoShape* create_first_or_intermediate_shape(Color color=Color(0,0,0)) override;
    GmoShape* create_last_shape(Color color=Color(0,0,0)) override;

    //During layout, when more space is added between two staves, all shapes are shifted
    //down to account for the added extra space. If a beam is cross-staff, it is
    //necessary to increment the lenght of all stems conected to this beam
    void increment_cross_staff_stems(LUnits yIncrement);

protected:
    void add_note_rest(ImoStaffObj* pSO, GmoShape* pStaffObjShape);
    void determine_number_of_beam_levels();
    void decide_stems_direction();
        void decide_stems_direction_for_beams_with_chords();
        void decide_stems_direction_for_beams_without_chords();
            void decide_beam_position();
        void change_stems_direction();
    void reposition_rests();

    void compute_beam_segments();
        void engrave_beam_segments_for_double_stemmed();
            void determine_segments(ImoBeam* pBeam, std::vector<SegmentData>* pSegs);
            void classify_segments(std::vector<SegmentData>* pSegs);
	        void position_segments(std::vector<SegmentData>* pSegs);
        void engrave_beam_segments_for_simple_beams();
            void add_segment(LUnits uxStart, LUnits uyStart, LUnits uxEnd, LUnits uyEnd);
            void make_segments_relative();

    void create_shape();
    void add_shape_to_noterests();
    void add_stroke_for_graces();

    //simple beams (and cross-staff but not double-stemmed)
    void beam_angle_and_stems_for_simple_beams();
        void create_horizontal_beam_and_set_stems(int pos0, int posN);
            float get_staff_length_for_beam(int iNote);
        float assign_slant_to_beam_for_grace_notes(int pos0, int posN);
        void assing_stem_length_to_outer_grace_notes(float slant, int pos0, int posN);
        float assign_slant_to_beam_for_regular_notes(int pos0, int posN);
            bool check_all_notes_outside_first_ledger_line();
        void assing_stem_length_to_outer_regular_notes(float slant, int pos0, int posN);
        void assing_stem_length_to_inner_notes();

    //cross-double-stemmed beams (cross-staff double-stemmed)
    void beam_angle_and_stems_for_cross_double_stemmed_beams();
        float determine_slant_direction_for_cross_double_stemmed(
                                                std::vector<GmoShapeNote*>& upNoteShapes,
                                                std::vector<GmoShapeNote*>& downNoteShapes);
            float assign_slant_for_cross_double_stemmed(std::vector<GmoShapeNote*>& noteShapes);
                float compute_slant_for_cross_double_stemmed(int pos0, int posN);
        void compute_stems_for_cross_double_stemmed(float slant,
                                                std::vector<GmoShapeNote*>& upNoteShapes,
                                                std::vector<GmoShapeNote*>& downNoteShapes);


    static bool has_repeated_pattern_of_pitches(std::vector<GmoShapeNote*>& noteShapes);
    static bool beam_must_be_horizontal(std::vector<GmoShapeNote*>& noteShapes);


    //temporary, only while determining stem position. Methods: decide_stems_direction(),
    //decide_beam_position(), change_stems_direction()
    bool m_fStemForced;     //at least one stem forced
    bool m_fStemsUp;        //only meaningfull if m_fDoubleStemed==false. True if all stems
                            //forced up or default position

};


}   //namespace lomse

#endif    // __LOMSE_BEAM_ENGRAVER_H__


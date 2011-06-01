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

#include "lomse_noterest_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_gm_basic.h"
#include "lomse_engraving_options.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"
#include "lomse_shape_beam.h"
#include "lomse_shape_tuplet.h"
#include "lomse_box_slice_instr.h"
#include "lomse_font_storage.h"
#include "lomse_shapes.h"
#include "lomse_pitch.h"
#include "lomse_score_meter.h"
#include "lomse_beam_engraver.h"
#include "lomse_chord_engraver.h"
#include "lomse_tuplet_engraver.h"
#include "lomse_shapes_storage.h"
#include "lomse_accidentals_engraver.h"

using namespace std;

namespace lomse
{

//=======================================================================================
// NoterestEngraver implementation
//=======================================================================================
NoterestEngraver::NoterestEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           ShapesStorage* pShapesStorage)
    : Engraver(libraryScope, pScoreMeter)
    , m_pShapesStorage(pShapesStorage)
    , m_pNoteRest(NULL)
    , m_pNoteRestShape(NULL)
    , m_pStartBeamShape(NULL)
    , m_pEndBeamShape(NULL)
{
}

////---------------------------------------------------------------------------------------
//bool NoterestEngraver::is_first_note_of_beam()
//{
//    if (is_beamed())
//    {
//        ImoBeam* pBeam = get_beam();
//        return (m_pNoteRest == dynamic_cast<ImoNote*>(pBeam->get_start_object()) );
//    }
//    return false;
//}
//
////---------------------------------------------------------------------------------------
//bool NoterestEngraver::is_last_note_of_beam()
//{
//    if (is_beamed())
//    {
//        ImoBeam* pBeam = m_pNoteRest->get_beam();
//        return (m_pNoteRest == dynamic_cast<ImoNote*>(pBeam->get_end_object()) );
//    }
//    return false;
//}
//
////---------------------------------------------------------------------------------------
//void NoterestEngraver::create_beam_shape_and_link()
//{
//    ImoBeam* pBeam = get_beam();
//    BeamEngraver* pEngrv = new BeamEngraver(m_libraryScope, m_pMeter);
//    m_pBeamShape = pEngrv->create_shape(pBeam, m_iInstr, m_iStaff);
//    m_pShapesStorage->save_engraver(pEngrv, pBeam);
//
//    link_note_and_beam(pEngrv, k_link_start);
//}
//
////---------------------------------------------------------------------------------------
//void NoterestEngraver::get_beam_shape_and_link()
//{
//    ImoBeam* pBeam = get_beam();
//    BeamEngraver* pEngrv
//        = dynamic_cast<BeamEngraver*>(m_pShapesStorage->get_engraver(pBeam));
//    m_pBeamShape = pEngrv->get_beam_shape();
//
//    if (is_last_note_of_beam())
//        link_note_and_beam(pEngrv, k_link_end);
//    else
//        link_note_and_beam(pEngrv, k_link_middle);
//}
//
////---------------------------------------------------------------------------------------
//void NoterestEngraver::link_note_and_beam(BeamEngraver* pEngrv, int linkType)
//{
//    m_pNoteRestShape->accept_link_from(m_pBeamShape, linkType);
//    pEngrv->add_note_rest(m_pNoteRest, m_pNoteRestShape);
//
//    //transfer beam info to note shape
//    for (int iLevel=0; iLevel < 6; ++iLevel)
//        m_pNoteRestShape->set_beam_type(iLevel, m_pNoteRest->get_beam_type(iLevel));
//}
//
////---------------------------------------------------------------------------------------
//void NoterestEngraver::layout_beam()
//{
//    ImoBeam* pBeam = get_beam();
//    BeamEngraver* pEngrv
//        = dynamic_cast<BeamEngraver*>(m_pShapesStorage->get_engraver(pBeam));
//    pEngrv->fix_stems_and_reposition_rests();
//    m_pShapesStorage->shape_ready_for_gmodel(pBeam, GmoShape::k_layer_notes);
//}
//
////---------------------------------------------------------------------------------------
//void NoterestEngraver::add_to_beam_if_beamed()
//{
//    if ( is_beamed())
//    {
//        if (is_first_note_of_beam())
//            create_beam_shape_and_link();
//        else
//            get_beam_shape_and_link();
//    }
//    else
//        m_pBeamShape = NULL;
//}

//---------------------------------------------------------------------------------------
double NoterestEngraver::determine_font_size()
{
    //TODO
    return 21.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;
}


}  //namespace lomse

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

#include "lomse_tuplet_engraver.h"

#include "lomse_glyphs.h"
#include "lomse_shape_beam.h"
#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_im_note.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_shape_tuplet.h"
#include "lomse_shape_text.h"
#include "lomse_text_engraver.h"

#include <sstream>
using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// TupletEngraver implementation
//---------------------------------------------------------------------------------------
TupletEngraver::TupletEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : Engraver(libraryScope, pScoreMeter)
    , m_pTupletShape(NULL)
    , m_pTuplet(NULL)
{
}

//---------------------------------------------------------------------------------------
TupletEngraver::~TupletEngraver()
{
}

//---------------------------------------------------------------------------------------
GmoShapeTuplet* TupletEngraver::create_shape(ImoObj* pCreatorImo, int iInstr, int iStaff,
                                             const string& label,
                                             ImoTextStyleInfo* pStyle)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_label = label;
    m_pStyle = pStyle;

    return do_create_shape(pCreatorImo);
}

//---------------------------------------------------------------------------------------
GmoShapeTuplet* TupletEngraver::create_shape(ImoTuplet* pTuplet, int iInstr, int iStaff)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;

    stringstream label;
    label << pTuplet->get_actual_number();
    m_label = label.str();

    m_pStyle = m_pMeter->get_tuplets_style_info();

    return do_create_shape(pTuplet);
}

//---------------------------------------------------------------------------------------
GmoShapeTuplet* TupletEngraver::do_create_shape(ImoObj* pCreatorImo)
{
    m_pTupletShape = new GmoShapeTuplet(pCreatorImo, Color(0,0,0));
    m_pShape = m_pTupletShape;
    add_text_shape(pCreatorImo);

    return m_pTupletShape;
}

//---------------------------------------------------------------------------------------
void TupletEngraver::add_text_shape(ImoObj* pCreatorImo)
{
    TextEngraver engr(m_libraryScope, m_pMeter, m_label, m_pStyle);
    GmoShapeText* pShape = engr.create_shape(pCreatorImo, 0.0f, 0.0f, 0);   //TODO-LOG k_center);
    m_pTupletShape->add_label(pShape);
}

//---------------------------------------------------------------------------------------
void TupletEngraver::add_note_rest(ImoNoteRest* pNoteRest, GmoShape* pShape)
{
	m_noteRests.push_back( make_pair(pNoteRest, pShape) );
    if (!m_pTuplet)
        m_pTuplet = pNoteRest->get_tuplet();
}

//---------------------------------------------------------------------------------------
void TupletEngraver::layout_tuplet(GmoShapeBeam* pBeamShape)
{
    //get standard engraving distances
	LUnits uBorderLength = tenths_to_logical(LOMSE_TUPLET_BORDER_LENGHT);
    LUnits uBracketDistance = tenths_to_logical(LOMSE_TUPLET_BRACKET_DISTANCE);
    LUnits uLineThick = tenths_to_logical(LOMSE_TUPLET_BRACKET_THICKNESS);
    LUnits uNumberDistance = tenths_to_logical(LOMSE_TUPLET_NUMBER_DISTANCE);
    LUnits uSpaceToNumber = tenths_to_logical(LOMSE_TUPLET_SPACE_TO_NUMBER);

    //determine tuplet appearance
    bool fDrawBracket = decide_if_show_bracket();
    bool fAbove = decide_if_tuplet_placement_above();
    compute_y_coordinates(pBeamShape);

    m_pTupletShape->set_layout_data(fAbove, fDrawBracket, m_yStart, m_yEnd,
                                    uBorderLength, uBracketDistance,
                                    uLineThick, uNumberDistance, uSpaceToNumber);
}

//---------------------------------------------------------------------------------------
void TupletEngraver::compute_y_coordinates(GmoShapeBeam* pBeamShape)
{
    GmoShapeNote* pStart = get_first_note();
    GmoShapeNote* pEnd = get_last_note();

    //determine y start/end coordinates
    if (pBeamShape)
    {
        //PROBLEM: Beam is not layouted. Its exact position is not computed until 
        //method beam::on_draw() is invoked!
        //case a): tuplet encloses a whole beamed group
        UPoint pos = pBeamShape->get_outer_left_reference_point();
        m_yStart = pos.y;
        pos = pBeamShape->get_outer_right_reference_point();
        m_yEnd = pos.y;
    }
    else
    {
        //case b): tuplet encloses a whole group but not beamed
        //case c): tuplet encloses several beamed groups and notes/rests
        if (pStart->is_up())
        {
            m_yStart = pStart->get_top();
            m_yEnd = pEnd->get_top();
        }
        else
        {
            m_yStart = pStart->get_bottom();
            m_yEnd = pEnd->get_bottom();
        }
    }
}

//---------------------------------------------------------------------------------------
GmoShapeNote* TupletEngraver::get_first_note()
{
    std::list< pair<ImoNoteRest*, GmoShape*> >::iterator it;
    for (it = m_noteRests.begin(); it != m_noteRests.end(); ++it)
    {
        if ((*it).first->is_note())
            return dynamic_cast<GmoShapeNote*>((*it).second);
    }
    return NULL;    //imposible case unless all group are rests!
}

//---------------------------------------------------------------------------------------
GmoShapeNote* TupletEngraver::get_last_note()
{
    std::list< pair<ImoNoteRest*, GmoShape*> >::reverse_iterator it;
    for (it = m_noteRests.rbegin(); it != m_noteRests.rend(); ++it)
    {
        if ((*it).first->is_note())
            return dynamic_cast<GmoShapeNote*>((*it).second);
    }
    return NULL;    //imposible case unless all group are rests!
}

//---------------------------------------------------------------------------------------
LUnits TupletEngraver::tenths_to_logical(Tenths value)
{
    return m_pMeter->tenths_to_logical(value, m_iInstr, m_iStaff);
}

//---------------------------------------------------------------------------------------
bool TupletEngraver::decide_if_show_bracket()
{
    bool fIsBeamedGroup = check_if_single_beamed_group();
    return m_pTuplet->get_show_bracket() == k_yesno_yes
           || (m_pTuplet->get_show_bracket() == k_yesno_default && !fIsBeamedGroup);
}

//---------------------------------------------------------------------------------------
bool TupletEngraver::decide_if_tuplet_placement_above()
{
    GmoShapeNote* pStart = get_first_note();
    bool fNotesUp = (pStart ? pStart->is_up() : true);
    int placement = m_pTuplet->get_placement();
	return placement == k_placement_above
           || (placement == k_placement_default && fNotesUp);
}

//---------------------------------------------------------------------------------------
bool TupletEngraver::check_if_single_beamed_group()
{
    ImoNoteRest* pStart = get_start_noterest();
    if (pStart->is_beamed())
    {
        ImoNoteRest* pEnd = get_end_noterest();
        if (pEnd->is_beamed())
            return pStart->get_beam() == pEnd->get_beam();
    }
    return false;
}


}  //namespace lomse

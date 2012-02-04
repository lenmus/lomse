//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include "lomse_fermata_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_shapes.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// FermataEngraver implementation
//---------------------------------------------------------------------------------------
FermataEngraver::FermataEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : Engraver(libraryScope, pScoreMeter)
{
}

//---------------------------------------------------------------------------------------
GmoShapeFermata* FermataEngraver::create_shape(ImoFermata* pFermata, int iInstr,
                                               int iStaff, UPoint pos, int placement,
                                               GmoShape* pParentShape)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pFermata = pFermata;
    m_placement = placement;
    m_pParentShape = pParentShape;
    m_fAbove = determine_if_above();

    int iGlyph = (m_fAbove ? k_glyph_fermata_above : k_glyph_fermata_below);
    double fontSize = determine_font_size();
    UPoint position = compute_location(pos);
    m_pFermataShape = LOMSE_NEW GmoShapeFermata(pFermata, 0, iGlyph, position, Color(0,0,0),
                                          m_libraryScope, fontSize);
    center_on_parent();
    return m_pFermataShape;
}

//---------------------------------------------------------------------------------------
UPoint FermataEngraver::compute_location(UPoint pos)
{
	if (m_fAbove)
		pos.y -= tenths_to_logical(5.0f);
	else
		pos.y += tenths_to_logical(50.0f);

	return pos;
}

//---------------------------------------------------------------------------------------
void FermataEngraver::center_on_parent()
{
    if (!m_pParentShape)
        return;

    LUnits uCenterPos;
    if (m_pParentShape->is_shape_note())
    {
		//it is a note. Center fermata on notehead shape
        GmoShapeNote* pNote = dynamic_cast<GmoShapeNote*>(m_pParentShape);
		uCenterPos = pNote->get_notehead_left() + pNote->get_notehead_width() / 2.0f;
    }
    else
    {
    	//it is not a note (normally it would be a rest).
        //Center fermata on parent shape
    	uCenterPos = m_pParentShape->get_left() + m_pParentShape->get_width() / 2.0f;
    }
    LUnits xShift = uCenterPos - 
                    (m_pFermataShape->get_left() + m_pFermataShape->get_width() / 2.0f);

    if (xShift != 0.0f)
    {
        USize shift(xShift, 0.0f);
        m_pFermataShape->shift_origin(shift);
    }

    //ensure that fermata do not collides with parent shape
    URect overlap = m_pParentShape->get_bounds();
    overlap.intersection( m_pFermataShape->get_bounds() );
    LUnits yShift = overlap.get_height();
    if (yShift != 0.0f)
    {
        yShift += tenths_to_logical(5.0f);
        yShift = m_fAbove ? - yShift : yShift;

        USize shift(0.0f, yShift);
        m_pFermataShape->shift_origin(shift);
    }
}

//---------------------------------------------------------------------------------------
bool FermataEngraver::determine_if_above()
{
    if (m_placement == k_placement_above)
        return true;
    
    else if (m_placement == k_placement_below)
        return false;

    else    //k_placement_default
    {
        if (m_pParentShape && m_pParentShape->is_shape_note())
        {
            GmoShapeNote* pNote = dynamic_cast<GmoShapeNote*>(m_pParentShape);
            return !pNote->is_up();
        }
        else
            return true;
    }
}

//---------------------------------------------------------------------------------------
double FermataEngraver::determine_font_size()
{
    //TODO
    return 21.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;
}

//---------------------------------------------------------------------------------------
LUnits FermataEngraver::tenths_to_logical(Tenths tenths)
{
    return m_pMeter->tenths_to_logical(tenths, m_iInstr, m_iStaff);
}


}  //namespace lomse

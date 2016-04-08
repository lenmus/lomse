//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#include "lomse_lyrics_engraver.h"

#include "lomse_glyphs.h"
#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_im_note.h"
#include "lomse_shape_note.h"
#include "lomse_shape_line.h"
#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_text.h"
#include "lomse_instrument_engraver.h"

#include <cmath>        // fabs


namespace lomse
{

//=======================================================================================
// helper class to save information about each lyric
class LyricsInfo
{
public:
    GmoShape* pShape;
    int iSystem;
    int iCol;
    int iInstr;

    LyricsInfo(GmoShape* shape, int system, int col, int instr)
        : pShape(shape), iSystem(system), iCol(col), iInstr(instr)
    {
    }
    LyricsInfo() : pShape(NULL), iSystem(-1), iCol(-1), iInstr(-1) {}
};


//=======================================================================================
// LyricsEngraver implementation
//=======================================================================================
LyricsEngraver::LyricsEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                               InstrumentEngraver* pInstrEngrv)
    : RelAuxObjEngraver(libraryScope, pScoreMeter)
    , m_pLyricsShape(NULL)
    , m_pInstrEngrv(pInstrEngrv)
{
}

//---------------------------------------------------------------------------------------
LyricsEngraver::~LyricsEngraver()
{
    m_notes.clear();
}

//---------------------------------------------------------------------------------------
void LyricsEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                        GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                        int iSystem, int iCol, LUnits UNUSED(xRight),
                                        LUnits UNUSED(xLeft), LUnits yTop)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pLyrics = dynamic_cast<ImoLyrics*>(pRO);

    ImoNote* pNR = dynamic_cast<ImoNote*>(pSO);
    m_notes.push_back( make_pair(pNR, pStaffObjShape) );

    ShapeBoxInfo* pInfo = LOMSE_NEW ShapeBoxInfo(NULL, iSystem, iCol, iInstr);
    m_shapesInfo.push_back(pInfo);
    m_staffTops.push_back(yTop);
}

//---------------------------------------------------------------------------------------
void LyricsEngraver::set_middle_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                         GmoShape* pStaffObjShape, int iInstr,
                                         int UNUSED(iStaff), int iSystem, int iCol,
                                         LUnits UNUSED(xRight), LUnits UNUSED(xLeft),
                                         LUnits yTop)
{
    ImoNote* pNR = dynamic_cast<ImoNote*>(pSO);
    m_notes.push_back( make_pair(pNR, pStaffObjShape) );

    ShapeBoxInfo* pInfo = LOMSE_NEW ShapeBoxInfo(NULL, iSystem, iCol, iInstr);
    m_shapesInfo.push_back(pInfo);
    m_staffTops.push_back(yTop);
}

//---------------------------------------------------------------------------------------
void LyricsEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                      GmoShape* pStaffObjShape, int iInstr,
                                      int UNUSED(iStaff), int iSystem, int iCol,
                                      LUnits UNUSED(xRight), LUnits UNUSED(xLeft),
                                      LUnits yTop)
{
    ImoNote* pNR = dynamic_cast<ImoNote*>(pSO);
    m_notes.push_back( make_pair(pNR, pStaffObjShape) );

    ShapeBoxInfo* pInfo = LOMSE_NEW ShapeBoxInfo(NULL, iSystem, iCol, iInstr);
    m_shapesInfo.push_back(pInfo);
    m_staffTops.push_back(yTop);
}

//---------------------------------------------------------------------------------------
int LyricsEngraver::create_shapes(Color color)
{
    m_color = color;

    list< pair<ImoNote*, GmoShape*> >::iterator it;
    int i = 0;
    for(it=m_notes.begin(); it != m_notes.end(); ++it, ++i)
	{
        ImoNote* pNote = dynamic_cast<ImoNote*>(it->first);
        GmoShapeNote* pNoteShape = dynamic_cast<GmoShapeNote*>(it->second);

        GmoShapeNote* pNextNoteShape = NULL;
        list< pair<ImoNote*, GmoShape*> >::iterator nextIt = it;
        ++nextIt;
        if (nextIt != m_notes.end())
            pNextNoteShape = dynamic_cast<GmoShapeNote*>(nextIt->second);

        create_shape(i, pNoteShape, pNote, pNextNoteShape);
    }
    return int(m_notes.size());

};

//---------------------------------------------------------------------------------------
void LyricsEngraver::create_shape(int iNote, GmoShapeNote* pNoteShape, ImoNote* pNote,
                                  GmoShapeNote* pNextNoteShape)
{
    LUnits x = pNoteShape->get_left();

    //TODO: base position should be the maximum notes bottom y + 5.0f instead
    //      of a fixed amount (60.0f)
    //TODO: line increment should be text shape height + 0.5f instead o a fixed
    //      amount (23.0f)
    int lineNum = m_pLyrics->get_number();
    int tenths = 60.0f + 23.0f * float(lineNum);
    LUnits y = m_staffTops[iNote] + tenths_to_logical(tenths);

    //get text
    ImoLyricsData* pData = static_cast<ImoLyricsData*>(m_pLyrics->get_data_for(pNote));
    ImoLyricsTextInfo* pText = pData->get_text_item(0);
    const string& text = pText->get_syllable_text();
    const string& language = pText->get_syllable_language();
    ImoStyle* pStyle = pText->get_syllable_style();
    if (pStyle == NULL)
        pStyle = m_pMeter->get_lyrics_style_info();
    Color color = pStyle->color();


    //create container shape
    ShapeId idx = 0;
    m_pLyricsShape = LOMSE_NEW GmoShapeLyrics(m_pLyrics, idx, Color(0,0,0) /*unused*/,
                                              m_libraryScope);

    //add shape for text
    GmoShape* pSyllableShape = LOMSE_NEW GmoShapeText(m_pLyrics, idx, text, pStyle,
                                                      language, x, y, m_libraryScope);

    //center syllable on note head
    LUnits shift = (pNoteShape->get_width() - pSyllableShape->get_width()) / 2.0f;
    pSyllableShape->shift_origin(shift, 0.0f);
    m_pLyricsShape->shift_origin(USize(shift, 0.0f));

	m_pLyricsShape->add(pSyllableShape);
    x = pSyllableShape->get_right();


    //add shape for hyphenation, if needed
    if (pText->get_syllable_type() == ImoLyrics::k_begin
        || pText->get_syllable_type() == ImoLyrics::k_middle)
    {
        GmoShape* pShape = LOMSE_NEW GmoShapeText(m_pLyrics, idx, "-", pStyle,
                                                  language, x, y, m_libraryScope);

        //shift shape to center between this and next syllable
        if (pNextNoteShape)
        {
            LUnits space = pNextNoteShape->get_left() - pSyllableShape->get_right();
            LUnits shift = (space - pShape->get_width()) / 2.0f;
            pShape->shift_origin(shift, 0.0f);
        }

        m_pLyricsShape->add(pShape);
        x = pShape->get_left() + pShape->get_width();
    }

    //add shape for extend line, if needed
    if (pData->has_extend())
    {
        LUnits xStart = x + tenths_to_logical(3.0f);;
        LUnits xEnd;
        if (pNextNoteShape)
        {
            xEnd = pNextNoteShape->get_left() - tenths_to_logical(20.0f);
            if (xEnd < xStart)
                xEnd = m_pInstrEngrv->get_staves_right();     //TODO
        }
        else
            xEnd = xStart + tenths_to_logical(60.0f);     //TODO

        LUnits width = tenths_to_logical(1.0f);     //TODO engraving option
        LUnits boundsExtraWidth = tenths_to_logical(0.5f);
        GmoShape* pShape = LOMSE_NEW GmoShapeLine(m_pLyrics, idx, xStart, y, xEnd, y,
                                        width, boundsExtraWidth, k_line_solid, color,
                                        k_edge_normal, k_cap_none, k_cap_none);

        m_pLyricsShape->add(pShape);

        x += pShape->get_width();
    }

    m_pShape = m_pLyricsShape;
    ShapeBoxInfo* pInfo = m_shapesInfo[iNote];
    pInfo->pShape = m_pLyricsShape;
}

//---------------------------------------------------------------------------------------
void LyricsEngraver::decide_placement()
{
}


}  //namespace lomse

//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#ifndef __LOMSE_LYRIC_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_LYRIC_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

#include <list>
#include <vector>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class GmoShapeLyrics;
class ScoreMeter;
class ImoLyric;
class ImoNote;
class GmoShapeNote;
class GmoShape;
class InstrumentEngraver;


//---------------------------------------------------------------------------------------
class LyricEngraver : public AuxRelObjEngraver
{
protected:
    GmoShapeLyrics* m_pLyricsShape;
    InstrumentEngraver* m_pInstrEngrv;
    list< pair<ImoLyric*, GmoShape*> > m_lyrics;
    vector<ShapeBoxInfo*> m_shapesInfo;
    UPoint m_origin;
    USize m_size;
    bool m_fLyricAbove;
    int m_numShapes;
    LUnits m_uStaffTop;             //top line of current staff
    LUnits m_uStaffLeft;
    LUnits m_uStaffRight;

    LUnits m_fontAscender;
    LUnits m_fontDescender;
    LUnits m_fontHeight;
    LUnits m_fontBase;      //measured from top border

public:
    LyricEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                  InstrumentEngraver* pInstrEngrv);
    ~LyricEngraver();

    //implementation of virtual methods from AuxRelObjEngraver
    void set_start_staffobj(ImoAuxRelObj* pARO, ImoStaffObj* pSO,
                            GmoShape* pStaffObjShape, int iInstr, int iStaff,
                            int iSystem, int iCol,
                            LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                            int idxStaff, VerticalProfile* pVProfile) override;
    void set_middle_staffobj(ImoAuxRelObj* pARO, ImoStaffObj* pSO,
                             GmoShape* pStaffObjShape, int iInstr, int iStaff,
                             int iSystem, int iCol,
                             LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                             int idxStaff, VerticalProfile* pVProfile) override;
    void set_end_staffobj(ImoAuxRelObj* pARO, ImoStaffObj* pSO,
                          GmoShape* pStaffObjShape, int iInstr, int iStaff,
                          int iSystem, int iCol,
                          LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                          int idxStaff, VerticalProfile* pVProfile) override;

    int create_shapes(Color color=Color(0,0,0)) override;
    int get_num_shapes() override { return m_numShapes; }
    ShapeBoxInfo* get_shape_box_info(int i) override { return m_shapesInfo[i]; }


protected:

    void create_shape(int note, GmoShapeNote* pNoteShape, ImoLyric* pLyric,
                      GmoShapeNote* pNextNoteShape, LUnits xLeft, LUnits xRight);
    void decide_placement();
    void measure_text_height(ImoLyric* pLyric);

};


}   //namespace lomse

#endif    // __LOMSE_LYRIC_ENGRAVER_H__


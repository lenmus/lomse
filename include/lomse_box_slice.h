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
//--------------------------------------------------------------------------------------

#ifndef __LOMSE_BOX_SLICE_H__        //to avoid nested includes
#define __LOMSE_BOX_SLICE_H__

#include "lomse_basic.h"
#include "lomse_gm_basic.h"
//#include <sstream>
//
//using namespace std;

namespace lomse
{

//forward declarations
class GmoBoxSystem;
class GmoBoxSliceInstr;
class ImoInstrument;
//class lmTimeGridTable;


//---------------------------------------------------------------------------------------
// GmoBoxSlice: a system column
class GmoBoxSlice : public GmoBox
{
protected:
//    int             m_nAbsMeasure;		//number of this measure (absolute, 1..n)
//	int				m_nNumInSystem;		//number of slice for this system (0..n-1)
//
//    //start and end positions
//    LUnits    m_xStart;
//    LUnits    m_xEnd;
//
//    lmTimeGridTable*            m_pGridTable;

public:
    GmoBoxSlice(int nAbsMeasure);    //, int nNumInSystem,
			    //LUnits xStart=0, LUnits xEnd=0);
    ~GmoBoxSlice();

//    inline void UpdateSize(LUnits xStart, LUnits xEnd) {
//            m_xStart = xStart;
//            m_xEnd = xEnd;
//        }
//
//	//render
//	void DrawSelRectangle(lmPaper* pPaper);
//
//    //info
//    inline int GetNumMeasure() const { return m_nAbsMeasure; }
//
	//instrument slices
    GmoBoxSliceInstr* add_box_for_instrument(ImoInstrument* pInstr);
//	GmoBoxSliceInstr* GetSliceInstr(int i) const { return (GmoBoxSliceInstr*)m_Boxes[i]; }
//
//    //implementation of virtual methods from base class
//	int GetPageNumber() const;
//
//	//owners and related
//	GmoBoxSystem* GetOwnerSystem() { return m_pBoxSystem; }
//    GmoBoxScore* GetOwnerBoxScore();
//    GmoBoxPage* GetOwnerBoxPage();
//
//    //overrides
//    void SetBottomSpace(LUnits uyValue);
//
//    //grid table: xPositions/timepos
//    inline void SetTimeGridTable(lmTimeGridTable* pGridTable) { m_pGridTable = pGridTable; }
//    float GetGridTimeForPosition(LUnits uxPos);
//    void DrawTimeLines(lmPaper* pPaper, wxColour color, LUnits uyTop,
//                       LUnits uyBottom);
protected:
    GmoBoxSystem* get_system_box();


};


}   //namespace lomse

#endif    // __LOMSE_BOX_SLICE_H__

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

#ifndef __LOMSE_BOX_SYSTEM_H__
#define __LOMSE_BOX_SYSTEM_H__

#include "lomse_basic.h"
#include "lomse_gm_basic.h"

//using namespace std;

namespace lomse
{

//forward declarations
class GmoBoxScorePage;
//class ImoStaffInfo;
//class GmoBoxSlice;
//class lmShapeStaff;
//class lmShapeMargin;

//
// Class GmoBoxSystem represents a line of music in the printed score.
//

class GmoBoxSystem : public GmoBox
{
protected:
//    GmoBoxScorePage*  m_pBPage;           //parent page
//    int         m_nFirstMeasure;    //number of first measure
//    LUnits    m_xPos, m_yPos;     //system position: pos to render first staff
//    LUnits    m_nIndent;          //indentation for this system
    //int m_nNumPage;         //page number (1..n) on which this system is included
//	lmShapeMargin*	m_pTopSpacer;
	std::vector<GmoShapeStaff*> m_staffShapes;

public:
    GmoBoxSystem(ImoObj* pCreatorImo);
    ~GmoBoxSystem();

//	int GetSystemNumber();
//
    //slices
	inline int get_num_slices() const { return (int)m_childBoxes.size(); }
    inline GmoBoxSlice* get_slice(int i) const { return (GmoBoxSlice*)m_childBoxes[i]; }
//    void DeleteLastSlice();
//    int GetNumMeasures();
//    inline int GetFirstMeasureNumber() { return m_nFirstMeasure; }
//    inline int GetLastMeasureNumber() { return m_nFirstMeasure + GetNumMeasures() - 1; }
//    inline void SetFirstMeasure(int nAbsMeasure) { m_nFirstMeasure = nAbsMeasure; }

//    //positioning
//    void SetPosition(LUnits xPos, LUnits yPos);
//    inline LUnits GetPositionX() const { return m_xPos; }
//    inline LUnits GetPositionY() const { return m_yPos; }
//    inline void SetIndent(LUnits xDsplz) { m_nIndent = xDsplz; }
//    inline LUnits GetSystemIndent() const { return m_nIndent; }
//    inline LUnits GetSystemFinalX() const { return m_uBoundsBottom.x; }
//
//	//miscellaneous info
//	LUnits GetYTopFirstStaff();
    GmoShapeStaff* get_staff_shape(int iStaff);
//    GmoShapeStaff* get_staff_shape(lmInstrument* pInstr, int nStaff);
//    GmoShapeStaff* get_staff_shape(lmInstrument* pInstr, lmUPoint uPoint);
//
//    //pointing at
//	lmShapeStaff* FindStaffAtPosition(lmUPoint& uPoint);
//	int GetNumMeasureAt(LUnits uxPos);
//
//	//access to objects
//	GmoBoxSlice* FindBoxSliceAt(LUnits uxPos);
//
//    //implementation of virtual methods from base class
//	int GetPageNumber() const;
//
//	//overrides
//    void UpdateXRight(LUnits xPos);
//    void SetBottomSpace(LUnits uyValue);
//
//	//owners and related
//	GmoBoxSystem* GetOwnerSystem() { return this; }

    //Staff shapes
    GmoShapeStaff* add_staff_shape(GmoShapeStaff* pShape);


//private:
//    void ClearStaffShapesTable();
//

};



}   //namespace lomse

#endif      //__LOMSE_BOX_SYSTEM_H__

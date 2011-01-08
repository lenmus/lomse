//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_BOX_SYSTEM_H__
#define __LOMSE_BOX_SYSTEM_H__

#include "lomse_basic.h"
#include "lomse_gm_basic.h"

//using namespace std;

namespace lomse
{

//forward declarations
class GmoStubScore;
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
    GmoBoxSystem();
    //GmoBoxSystem(GmoBoxScorePage* pParent, int nNumPage, int iSystem,
    //             LUnits uxPos, LUnits uyPos, bool fFirstOfPage);
    ~GmoBoxSystem();

//	int GetSystemNumber();
//
    //slices
	inline int get_num_slices() const { return (int)m_childBoxes.size(); }
    GmoBoxSlice* add_slice(int nAbsMeasure); //, LUnits uxStart=0.0f, LUnits uxEnd=0.0f);
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
//    wxString Dump(int nIndent);
//	int GetPageNumber() const;
//
//	//overrides
//    void UpdateXRight(LUnits xPos);
//    void SetBottomSpace(LUnits uyValue);
//
//	//owners and related
//	GmoBoxSystem* GetOwnerSystem() { return this; }
//	GmoStubScore* GetBoxScore();
//    inline GmoBoxScorePage* GetOwnerBoxPage() { return m_pBPage; }
//    GmoStubScore* GetOwnerBoxScore();

    //Staff shapes
    GmoShapeStaff* add_staff_shape(GmoShapeStaff* pShape);

//private:
//    void ClearStaffShapesTable();
//

};



}   //namespace lomse

#endif      //__LOMSE_BOX_SYSTEM_H__

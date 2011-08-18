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

#ifndef __LOMSE_BOX_SLICE_INSTR_H__
#define __LOMSE_BOX_SLICE_INSTR_H__

#include "lomse_gm_basic.h"
//#include <sstream>
//using namespace std;

namespace lomse
{

//forward declarations
class GmoBoxSlice;
class ImoInstrument;
//class LdpCompiler;
//class IdAssigner;
//class InternalModel;
//class ImoDocument;
//class ImoScore;


//---------------------------------------------------------------------------------------
// Class GmoBoxSliceInstr represents a part (column, measure) of an instrument.
class GmoBoxSliceInstr : public GmoBox
{
private:
//    GmoBoxSlice*     m_pSlice;           //parent slice
//    ImoInstrument*   m_pInstr;           //instrument to which this slice belongs

public:
    GmoBoxSliceInstr(ImoInstrument* pInstr);
    ~GmoBoxSliceInstr();

//    inline ImoInstrument* GetInstrument() const { return m_pInstr; }
//    inline int GetNumMeasure() const { return m_pSlice->GetNumMeasure(); }
//
//    //implementation of virtual methods from base class
//	int GetPageNumber() const;
//
//	//owners and related
//	GmoBoxSystem* GetOwnerSystem() { return m_pSlice->GetOwnerSystem(); }
//    inline GmoBoxScore* GetOwnerBoxScore() { return m_pSlice->GetOwnerBoxScore(); }
//    inline GmoBoxPage* GetOwnerBoxPage() { return m_pSlice->GetOwnerBoxPage(); }
//
//    //other
//    GmoShapeStaff* GetStaffShape(int nStaff);   //1..n
//    GmoShapeStaff* GetNearestStaff(lmUPoint& uPoint);
//    void DrawTimeGrid(lmPaper* pPaper);
//    void DrawMeasureFrame(lmPaper* pPaper);
//

private:

};


}   //namespace lomse

#endif      //__LOMSE_BOX_SLICE_INSTR_H__

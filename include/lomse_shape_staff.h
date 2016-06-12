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

#ifndef __LOMSE_SHAPE_STAFF_H__        //to avoid nested includes
#define __LOMSE_SHAPE_STAFF_H__

#include "lomse_shape_base.h"

namespace lomse
{

//forward declarations
class ImoStaffInfo;


//---------------------------------------------------------------------------------------
class GmoShapeStaff : public GmoSimpleShape
{
protected:
    ImoStaffInfo* m_pStaff;
	int m_iStaff;			    //num of staff in the instrument (0..n-1)
    LUnits m_lineThickness;

public: //TO_FIX: constructor invoked from test
//    friend class InstrumentEngraver;
    GmoShapeStaff(ImoObj* pCreatorImo, ShapeId idx, ImoStaffInfo* m_pStaff, int iStaff,
                  LUnits width, Color color);

public:
    ~GmoShapeStaff();

	//implementation of pure virtual methods in base class
    void on_draw(Drawer* pDrawer, RenderOptions& opt);
//    void Shift(LUnits xIncr, LUnits yIncr);

	//ownership and related info
	inline int get_num_staff() { return m_iStaff; }

    //info
    inline LUnits get_line_thickness() { return m_lineThickness; }

//    //adding notes/rest with mouse
//    UPoint OnMouseStartMoving(lmPaper* pPaper, const UPoint& uPos);
//    UPoint OnMouseMoving(lmPaper* pPaper, const UPoint& uPos);
//	void OnMouseEndMoving(lmPaper* pPaper, UPoint uPagePos);
//

    //other
    int line_space_at(LUnits yPos);

//protected:
//    //temporary data to be used when mouse tool moving over the staff
//    int         m_nOldSteps;		//to clear leger lines while dragging
//    LUnits    m_uxOldPos;
//    int         m_nPosOnStaff;		//line/space on staff on which this note is placed

};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_STAFF_H__


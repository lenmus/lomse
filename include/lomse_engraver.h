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

#ifndef __LOMSE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_ENGRAVER_H__

#include "lomse_injectors.h"

namespace lomse
{

//forward declarations
class ScoreMeter;
class GmoShape;
class ImoAuxObj;
class ImoStaffObj;

//---------------------------------------------------------------------------------------
// base class for all engravers
class Engraver
{
protected:
    LibraryScope& m_libraryScope;
    ScoreMeter* m_pMeter;
    GmoShape* m_pShape;

public:
    Engraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
        : m_libraryScope(libraryScope)
        , m_pMeter(pScoreMeter)
        , m_pShape(NULL)
    {
    }

    virtual ~Engraver() {}

    virtual GmoShape* get_shape() { return m_pShape; }
};


//---------------------------------------------------------------------------------------
// helper struct to save a shape and its box info (instrument, system, column)
struct ShapeBoxInfo
{
    GmoShape* pShape;
    int iSystem;
    int iCol;
    int iInstr;

    ShapeBoxInfo(GmoShape* shape, int system, int col, int instr)
        : pShape(shape), iSystem(system), iCol(col), iInstr(instr)
    {
    }
    ShapeBoxInfo() : pShape(NULL), iSystem(-1), iCol(-1), iInstr(-1) {}
};


//---------------------------------------------------------------------------------------
// base class for relation aux objects engravers
class RelAuxObjEngraver : public Engraver
{
protected:

public:
    RelAuxObjEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
        : Engraver(libraryScope, pScoreMeter)
    {
    }
    virtual ~RelAuxObjEngraver() {}

    virtual void set_start_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                    GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                    int iSystem, int iCol, UPoint pos) = 0;
    virtual void set_middle_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                     GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                     int iSystem, int iCol) {}
    virtual void set_end_staffobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                  GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                  int iSystem, int iCol) = 0;
    virtual int create_shapes() = 0;
    virtual int get_num_shapes() = 0;
    virtual ShapeBoxInfo* get_shape_box_info(int i) = 0;
    virtual void set_prolog_width(LUnits width) = 0;

};


}   //namespace lomse

#endif    // __LOMSE_ENGRAVER_H__


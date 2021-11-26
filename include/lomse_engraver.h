//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#ifndef __LOMSE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_ENGRAVER_H__

#include "lomse_injectors.h"
#include "lomse_basic.h"

namespace lomse
{

//forward declarations
class AuxShapesAligner;
class AuxShapesAlignersSystem;
class GmoShape;
class ImoRelObj;
class ImoStaffObj;
class ImoAuxRelObj;
class ImoInstrument;
class ScoreMeter;
class VerticalProfile;

//=======================================================================================
// Helper struct to store context information valid for the engraver lifetime, for
// engravers that create the symbols in a single call (e.g. AuxObjs, StaffObjs).
// For RelObj engravers, most of the information will be valid for the engraver lifetime,
// with the exception of that information that changes from system to system (VProfile)
// Using this struct simplifies the list of parameters to pass to engravers constructor,
// as well as simplifies code maintenance
//
struct EngraverContext
{
    LibraryScope& libraryScope;
    ScoreMeter* pScoreMeter = nullptr;
    VerticalProfile* pVProfile = nullptr;
    AuxShapesAlignersSystem* pAuxShapesAligner = nullptr;
    int iInstr = 0;
    int iStaff = 0;
    int idxStaff = 0;

//    EngraverContext(LibraryScope& scope) : libraryScope(scope) {}
    EngraverContext(LibraryScope& scope, ScoreMeter* meter, int instr, int staff,
                    int idx, VerticalProfile* vprofile, AuxShapesAlignersSystem* aligner)
        : libraryScope(scope)
        , pScoreMeter(meter)
        , pVProfile(vprofile)
        , pAuxShapesAligner(aligner)
        , iInstr(instr)
        , iStaff(staff)
        , idxStaff(idx)
    {
    }

    EngraverContext(LibraryScope& scope, ScoreMeter* meter, int instr, int staff, int idx)
        : libraryScope(scope)
        , pScoreMeter(meter)
        , pVProfile(nullptr)
        , pAuxShapesAligner(nullptr)
        , iInstr(instr)
        , iStaff(staff)
        , idxStaff(idx)
    {
    }
};


//=======================================================================================
// Base class for all engravers
class Engraver
{
protected:
    LibraryScope& m_libraryScope;
    ScoreMeter* m_pMeter;

public:
    Engraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
        : m_libraryScope(libraryScope)
        , m_pMeter(pScoreMeter)
    {
    }

    virtual ~Engraver() {}

protected:
    virtual LUnits tenths_to_logical(Tenths value) const;
    void add_user_shift(ImoContentObj* pImo, UPoint* pos);
};


//=======================================================================================
// Base class for all engravers related to symbols on the staff
class StaffSymbolEngraver : public Engraver
{
protected:
    int m_iInstr;
    int m_iStaff;

public:
    StaffSymbolEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter, int iInstr,
                        int iStaff)
        : Engraver(libraryScope, pScoreMeter)
        , m_iInstr(iInstr)
        , m_iStaff(iStaff)
    {
    }

    virtual ~StaffSymbolEngraver() {}

protected:
    LUnits tenths_to_logical(Tenths value) const override;
    virtual double determine_font_size();
};


//=======================================================================================
// Base class for all StaffObj engravers
class StaffObjEngraver : public StaffSymbolEngraver
{
protected:

public:
    StaffObjEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter, int iInstr,
                     int iStaff)
        : StaffSymbolEngraver(libraryScope, pScoreMeter, iInstr, iStaff)
    {
    }

    virtual ~StaffObjEngraver() {}
};


//=======================================================================================
// Base class for all AuxObj engravers
class AuxObjEngraver : public StaffSymbolEngraver
{
protected:
    int m_idxStaff = 0;
    VerticalProfile* m_pVProfile = nullptr;
    AuxShapesAlignersSystem* m_pAuxShapesAligner = nullptr;

public:
    AuxObjEngraver(const EngraverContext& ctx)
        : StaffSymbolEngraver(ctx.libraryScope, ctx.pScoreMeter, ctx.iInstr, ctx.iStaff)
        , m_idxStaff(ctx.idxStaff)
        , m_pVProfile(ctx.pVProfile)
        , m_pAuxShapesAligner(ctx.pAuxShapesAligner)
    {
    }

    virtual ~AuxObjEngraver() {}

protected:
    void add_to_aux_shapes_aligner(GmoShape* pShape, bool fAboveStaff) const;
    AuxShapesAligner* get_aux_shapes_aligner(int idxStaff, bool fAbove) const;

};


//=======================================================================================
// Helper struct to save a shape and its box info (instrument, system, column)
struct ShapeBoxInfo
{
    GmoShape* pShape = nullptr;
    int iCol = -1;
    int iInstr = -1;
    int iStaff = -1;
    int idxStaff = -1;

    ShapeBoxInfo(GmoShape* shape, int col, int instr, int staff, int idx)
        : pShape(shape), iCol(col), iInstr(instr), iStaff(staff), idxStaff(idx)
    {
    }
    ShapeBoxInfo() {}
};


//=======================================================================================
// Helper struct to store the information required to engrave an AuxObj/RelObj.
// Using this struct simplifies the list of parameters to pass to methods related
// to engraving a RelObj, as well as simplifies code maintenance
//
struct AuxObjContext
{
    //info related to the AuxObj/Rel/Obj
    ImoStaffObj* pSO;
    GmoShape* pStaffObjShape;
    ImoInstrument* pInstr;
    int iInstr;
    int iStaff;
    int iCol;
    int iLine;
    int idxStaff;

    AuxObjContext(ImoStaffObj* so, GmoShape* shape, int instr, int staff,
                  int col, int line, ImoInstrument* instrPtr, int idx)
        : pSO(so)
        , pStaffObjShape(shape)
        , pInstr(instrPtr)
        , iInstr(instr)
        , iStaff(staff)
        , iCol(col)
        , iLine(line)
        , idxStaff(idx)
    {
    }
};


//=======================================================================================
// Helper struct to store context information required to create a shape.
// Using this struct simplifies the list of parameters to pass to methods related
// to engraving a RelObj, as well as simplifies code maintenance
//
struct RelObjEngravingContext
{
    VerticalProfile* pVProfile = nullptr;
    AuxShapesAlignersSystem* pAuxShapesAligner = nullptr;
    int iSystem = 0;
    LUnits xStaffLeft = 0.0f;
    LUnits xStaffRight = 0.0f;
    LUnits yStaffTop = 0.0f;
    LUnits prologWidth = 0.0f;
    Color color = Color(0,0,0);

    RelObjEngravingContext() {}
};


//=======================================================================================
// Base class for all relation objects' engravers
class RelObjEngraver : public StaffSymbolEngraver
{
protected:
    Color m_color = Color(0,0,0);
    int m_idxStaff = -1;
    VerticalProfile* m_pVProfile = nullptr;
    AuxShapesAlignersSystem* m_pAuxShapesAligner = nullptr;

    enum EShapeType {
        k_single_shape = 0,
        k_first_shape,
        k_intermediate_shape,
        k_final_shape,
    };

public:
    RelObjEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
        : StaffSymbolEngraver(libraryScope, pScoreMeter, 0, 0)
        , m_color( Color(0,0,0) )
        , m_idxStaff(-1)
        , m_pVProfile(nullptr)
    {
    }
    virtual ~RelObjEngraver() {}

    virtual void set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) = 0;
    virtual void set_middle_staffobj(ImoRelObj* UNUSED(pRO), const AuxObjContext& UNUSED(aoc)) {}
    virtual void set_end_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) = 0;

    virtual GmoShape* create_first_or_intermediate_shape(const RelObjEngravingContext& UNUSED(ctx)) { return nullptr; }
    virtual GmoShape* create_last_shape(const RelObjEngravingContext& UNUSED(ctx)) { return nullptr; }

protected:
    void add_to_aux_shapes_aligner(GmoShape* pShape, bool fAboveStaff) const;
    AuxShapesAligner* get_aux_shapes_aligner(int idxStaff, bool fAbove) const;
};

//---------------------------------------------------------------------------------------
// base class for all auxiliary relation objects' engravers
class AuxRelObjEngraver : public StaffSymbolEngraver
{
protected:
    GmoShape* m_pShape;
    Color m_color;
    int m_idxStaff;
    VerticalProfile* m_pVProfile;

public:
    AuxRelObjEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
        : StaffSymbolEngraver(libraryScope, pScoreMeter, 0, 0)
        , m_pShape(nullptr)
        , m_color( Color(0,0,0) )
        , m_idxStaff(-1)
        , m_pVProfile(nullptr)
    {
    }
    virtual ~AuxRelObjEngraver() {}

    virtual void set_start_staffobj(ImoAuxRelObj* pARO, const AuxObjContext& aoc) = 0;
    virtual void set_middle_staffobj(ImoAuxRelObj* UNUSED(pARO), const AuxObjContext& UNUSED(aoc)) {}
    virtual void set_end_staffobj(ImoAuxRelObj* pARO, const AuxObjContext& aoc) = 0;

    virtual int create_shapes(const RelObjEngravingContext& ctx) = 0;
    virtual int get_num_shapes() = 0;
    virtual ShapeBoxInfo* get_shape_box_info(int i) = 0;

    virtual GmoShape* get_shape() { return m_pShape; }
};


}   //namespace lomse

#endif    // __LOMSE_ENGRAVER_H__


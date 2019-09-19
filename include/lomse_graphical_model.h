//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#ifndef __LOMSE_GRAPHICAL_MODEL_H__
#define __LOMSE_GRAPHICAL_MODEL_H__

#include "lomse_basic.h"
#include "lomse_observable.h"
#include "lomse_events.h"

#include <vector>
#include <list>
#include <ostream>
#include <map>
using namespace std;

namespace lomse
{

//forward declarations
class GmoObj;
class GmoBox;
class GmoBoxDocument;
class GmoBoxDocPage;
class GmoBoxScorePage;
class GmoBoxSlice;
class GmoBoxSliceInstr;
class GmoBoxSystem;
class GmoShape;
class GmoShapeStaff;
class ImoContentObj;
class ImoNoteRest;
class ImoObj;
class ImoScore;
class ImoStaffObj;
class ImoStyle;
class Drawer;
struct RenderOptions;
class GmoLayer;
class SelectionSet;
class Control;
class ScoreStub;
class GmMeasuresTable;


//---------------------------------------------------------------------------------------
//valid areas for mouse interaction
const int k_point_unknown =         0x00000000; //not yet determined
const int k_point_on_staff =        0x00000001; //pointing to an staff shape
const int k_point_above_staff =     0x00000002; //pointing to top margin of lmBoxSliceInstr
const int k_point_below_staff =     0x00000004; //pointing to bottom margin of lmBoxSliceInstr
const int k_point_on_other_shape =  0x00000008; //pointing to a shape other than staff
const int k_point_on_other_box =    0x00000010; //pointing to a box, other cases
const int k_point_on_other =        0x00000020; //pointing to other place (score paper)
const int k_point_on_note_or_rest = 0x00000040; //pointing to a note or a rest
const int k_point_on_any =          0x0FFFFFFF; //pointing to any place (all score is valid)


//---------------------------------------------------------------------------------------
// AreaInfo: auxiliary container
//  information about shapes/objects located at a given point
class AreaInfo
{
public:
    LUnits              x;
    LUnits              y;
    GmoShapeStaff*      pShapeStaff;    //container shape staff
    GmoBoxSliceInstr*   pBSI;           //container BoxSliceInstr
    GmoObj*             pGmo;           //pointed object
    int                 areaType;       //classification
    TimeUnits           gridTime;       //time for that point


    AreaInfo()
        : x(-1000000000000.0f)   //any impossible big value
        , y(-1000000000000.0f)   //any impossible big value
        , pShapeStaff(nullptr)
        , pBSI(nullptr)
        , pGmo(nullptr)
        , areaType(k_point_unknown)
        , gridTime(0.0)
    {
    }

    void clear(LUnits xp, LUnits yp)
    {
        x = xp;
        y = yp;
        pShapeStaff = nullptr;
        pBSI = nullptr;
        pGmo = nullptr;
        areaType = k_point_unknown;
        gridTime = 0.0;
    }
};


//---------------------------------------------------------------------------------------
// GraphicModel: storage for the graphic objects
//
class GraphicModel
{
protected:
    GmoBoxDocument* m_root;
    long m_modelId;
    bool m_modified;
    map<ImoId, GmoBox*> m_imoToBox;
    map<ImoId, GmoShape*> m_imoToMainShape;
    map< pair<ImoId, ShapeId>, GmoShape*> m_imoToSecondaryShape;
    map<GmoRef, GmoObj*> m_ctrolToPtr;
    map<ImoId, ScoreStub*> m_scores;
    AreaInfo m_areaInfo;

public:
    GraphicModel();
    virtual ~GraphicModel();

    //accessors
    inline GmoBoxDocument* get_root() { return m_root; }
    int get_num_pages();
    GmoBoxDocPage* get_page(int i);
    inline void set_modified(bool value) { m_modified = value; }
    inline bool is_modified() { return m_modified; }
    inline long get_model_id() { return m_modelId; }
    int get_page_number_containing(GmoObj* pGmo);
    GmMeasuresTable* get_measures_table(ImoId scoreId);

    //special accessors
    GmoShapeStaff* get_shape_for_first_staff_in_first_system(ImoId scoreId);

    //drawing
    void draw_page(int iPage, UPoint& origin, Drawer* pDrawer, RenderOptions& opt);
    //void highlight_object(ImoStaffObj* pSO, bool value);

    //hit testing and related
    GmoObj* hit_test(int iPage, LUnits x, LUnits y);
    GmoShape* find_shape_at(int iPage, LUnits x, LUnits y);
    GmoBox* find_inner_box_at(int iPage, LUnits x, LUnits y);
    AreaInfo* get_info_for_point(int iPage, LUnits x, LUnits y);

    //selection
    void select_objects_in_rectangle(int iPage, SelectionSet* pSelection,
                                     const URect& selRect, unsigned flags=0);
    GmoShape* find_shape_for_object(ImoStaffObj* pSO);
    GmoShape* get_shape_for_noterest(ImoNoteRest* pNR);

    //creation
    ScoreStub* add_stub_for(ImoScore* pScore);
    void store_in_map_imo_shape(ImoObj* pImo, GmoShape* pShape);
    void add_to_map_imo_to_box(GmoBox* child);
    void add_to_map_ref_to_box(GmoBox* pBox);
    GmoShape* get_shape_for_imo(ImoId imoId, ShapeId shapeId);
    GmoShape* get_main_shape_for_imo(ImoId id);
    GmoBox* get_box_for_imo(ImoId id);
    GmoObj* get_box_for_control(GmoRef gref);
    void build_main_boxes_table();

    //active and pointed elements

    /** Returns pointer to GmoBoxSystem containing the requested timepos.
        If there is no system for the given timepos, returns @nullptr.

        This method gives preference to finding a system containing an event at the
        given @c tiempos instead of non-timed staff objects. For example, the last
        barline in one system has the same @c timepos than the first event in next
        system. Therefore, this method will return the second system.

        @param scoreId
        @param time The time position (absolute time units) for the requested system.
    */
    GmoBoxSystem* get_system_for(ImoId scoreId, TimeUnits timepos);
    GmoBoxSystem* get_system_box(int iSystem);

    GmoBoxSystem* get_system_for_staffobj(ImoId id);


    //tests
    void dump_page(int iPage, ostream& outStream);


protected:
    ScoreStub* get_stub_for(ImoId scoreId);

};

//---------------------------------------------------------------------------------------
// Algorithms for finding info in the graphical model
class GModelAlgorithms
{
protected:

public:
    GModelAlgorithms() {}
    ~GModelAlgorithms() {}

    ///mouse point is over inner box pGmo. Find box system
    static GmoBoxSystem* get_box_system_for(GmoObj* pGmo, LUnits y);

};


}   //namespace lomse

#endif      //__LOMSE_GRAPHICAL_MODEL_H__

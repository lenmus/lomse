//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_GRAPHICAL_MODEL_H__
#define __LOMSE_GRAPHICAL_MODEL_H__

#include <vector>
#include <list>
#include <ostream>
#include <map>
using namespace std;

#include "lomse_basic.h"
#include "lomse_observable.h"
#include "lomse_events.h"

///@cond INTERNALS
namespace lomse
{
///@endcond

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
/** @ingroup enumerations

	This enum describes the flags for mouse events, indicating which regions of the
	score are valid for mouse interaction.

	@#include <lomse_graphical_model.h>
*/
enum EPointType
{
    k_point_unknown =         0x00000000,   ///< not yet determined
    k_point_on_staff =        0x00000001,   ///< pointing to an staff shape
    k_point_above_staff =     0x00000002,   ///< pointing to top margin of GmoBoxSliceInstr
    k_point_below_staff =     0x00000004,   ///< pointing to bottom margin of GmoBoxSliceInstr
    k_point_on_other_shape =  0x00000008,   ///< pointing to a shape other than staff
    k_point_on_other_box =    0x00000010,   ///< pointing to a box, other cases
    k_point_on_other =        0x00000020,   ///< pointing to other place (score paper)
    k_point_on_note_or_rest = 0x00000040,   ///< pointing to a note or a rest
    k_point_on_any =          0x0FFFFFFF,   ///< any region is valid (all score is valid)
};


//---------------------------------------------------------------------------------------
/** %AreaInfo is an auxiliary container with information about the object
    located at a given point and its parent GmoBox objects.
*/
class AreaInfo
{
public:
    LUnits              x;              ///> Point x coordinate, logical units
    LUnits              y;              ///> Point y coordinate, logical units
    GmoShapeStaff*      pShapeStaff;    ///> GmoShapeStaff in which point is contained
    GmoBoxSliceInstr*   pBSI;           ///> GmoBoxSliceInstr in which point is contained
    GmoObj*             pGmo;           ///> Pointed object
    int                 areaType;       ///> Object classification, from enum #EPointType
    TimeUnits           gridTime;       ///> timepos for the point

    /** constructor */
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

    /** Initialize this AreaInfo with coordinates for a point. */
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
/** The %GraphicModel represents the visual representation of the document. It is a tree
    of objects representing the graphical elements (lines, arcs, shapes, etc.) that
    describe the final appearance of the document from a purely geometric point of view.
    The graphical model objects (GmoShape and GmoBox derived objects) behave as vertex
    sources, providing the coordinates for the polygons that define the visual
    representation of the document.

    The %GraphicModel is a representation of what in computer graphics theory is named
    the <i>real world</i>, a virtual rendition of the full document at real size
    (e.g. millimeters). It is a virtual image of the full document organized as
    expected by the chosen View type.

    The @GM subdivides the document layout in regions or <i>boxes</i>, represented by
    objects derived from GmoBox. The boxes are organized in a tree so that each box is
    also a container for other boxes. For instance, the main boxes
    for an score are depicted in following picture:

    @image html graphical-model-boxes.png "Image: Some boxes in the Graphical Model" width=700px

    - GmoBoxSystem - Represents a line of music in the printed score and encloses all
        its content.
    - GmoBoxSlice - Systems are vertically split in columns or vertical slices. Normally,
        for music with time signature, a slice corresponds to a measure. GmoBoxSlice is
        a container for all boxes and shapes in that system column.
    - GmoBoxSliceInstr - GmoBoxSlice is horizontally subdivided in regions for each
        instrument in the system. GmoBoxSliceInstr represents a column of an instrument.
        It is a container for GmoBoxSliceStaff objects that contain the shapes for
        the slice.

    The %GraphicModel is built from the @IM by the layout process. Once it is built,
    it will be re-built only if the document or the View type change. Other changes,
    such as resolution, window size, allocation of a new rendering bitmap, do not
    affect the @GM.

    Rendering the document is done using the @GM. At high level, rendering is a simple
    operation. It is just traversing the graphical model tree and invoking the
    GmoObj::on_draw() method on each object. The @c on_draw() method will issue drawing
    commands that will be processed by a Drawer object for generating the rendition.

    The %GraphicModel also provides access to global information (e.g. number of pages)
    as well as to the most relevant objects.

    The @GM can be accessed by invoking method Interactor::get_graphic_model().
*/
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

    ///@cond INTERNALS
    //excluded from public API. Only for internal use.

    GraphicModel(ImoDocument* pCreator);
    virtual ~GraphicModel();

    ///@endcond


    //information
    /// @name Access to some information
    //@{

    /** Returns the number of pages of the rendered document. This value can also be
        obtained by invoking Interactor::get_num_pages().
    */
    int get_num_pages();

    /** Returns the number of systems in the rendered score.
        @param scoreId The Id of the score to which the request is referring.
    */
    int get_num_systems(ImoId scoreId);

    /** Returns the number of the page (0..n-1) that contains the given GmoObj.
        @param pGmo Pointer to the GmoObj to which the request is referring.
    */
    int get_page_number_containing(GmoObj* pGmo);

    /** Returns the GmMeasuresTable for the given score. The GmMeasuresTable object
        manages the table with the graphical information about the measures in
        the score and provides information such as the number of measures in the
        score, or their coordinates.
        @param scoreId The Id of the score to which the request is referring.
    */
    GmMeasuresTable* get_measures_table(ImoId scoreId);

    //@}    //information


    //access to boxes
    /// @name Access to main boxes
    //@{

    /** Returns the root object of the Graphical Model. It is always a GmoBoxDocument
        object.
    */
    inline GmoBoxDocument* get_root() { return m_root; }

    /** Returns the GmoBoxDocPage for the given page number.
        @param i The number of the page (0..n-1) for which the GmoBoxDocPage is requested.
    */
    GmoBoxDocPage* get_page(int i);

    /** Returns pointer to the GmoBoxSystem that contains the specified timepos.
        If there is no system for the given timepos, returns @nullptr.

        This method gives preference to finding a system containing a note/rest at the
        given @c tiempos instead of non-timed staff objects. For example, the last
        barline in one system has the same @c timepos than the first note/rest in next
        system. Therefore, this method will return the second system not the one
        containing the barline.

        @param scoreId The Id of the score to which the request is referring.
        @param timepos The time position (absolute time units) for the requested system.
    */
    GmoBoxSystem* get_system_for(ImoId scoreId, TimeUnits timepos);

    /** Returns pointer to the GmoBoxSystem for the given system number.
        @param iSystem The number of the system (0..n-1) for which the GmoBoxSystem is
            requested.
        @param scoreId The Id of the score to which the request is referring.
    */
    GmoBoxSystem* get_system_box(int iSystem, ImoId scoreId);

    /** Returns pointer to the GmoBoxSystem that contains the specified MeasureLocator.
        If there is no system for the given location, returns @nullptr.
        @param pScore The score to which the request is referring.
        @param ml The MeasureLocator for which the GmoBoxSystem is requested.
    */
    GmoBoxSystem* get_system_for(ImoScore* pScore, const MeasureLocator& ml);

    /** Returns pointer to the GmoBoxSystem that contains the specified ImoStaffObj.
        @param id The Id of the ImoStaffObj to which the request is referring.
    */
    GmoBoxSystem* get_system_for_staffobj(ImoId id);

    /** Returns pointer to the GmoShapeStaff for the first staff in first system.
        @param scoreId The Id of the score to which the request is referring.
    */
    GmoShapeStaff* get_shape_for_first_staff_in_first_system(ImoId scoreId);

    //@}    //access to boxes


    //access to shapes and boxes related to an ImoObj
    /// @name Access to shapes and boxes related to an ImoObj
    //@{

    /** Returns pointer to the GmoShape generated by an ImoObj.
        @param imoId The Id of the ImoObj to which the request is referring.
        @param shapeId  As any ImoObj can generate several shapes it is necessary
            to provide the shape index. The first generated shape has index 0, the next
            one index 1, and so on.
    */
    GmoShape* get_shape_for_imo(ImoId imoId, ShapeId shapeId);

    /** Returns pointer to the main GmoShape generated by an ImoObj. It is equivalent
        to get_shape_for_imo(imoId, 0).
        @param id The Id of the ImoObj to which the request is referring.
    */
    GmoShape* get_main_shape_for_imo(ImoId id);

    /** Returns pointer to the main GmoShape generated by an ImoStaffObj.
        @param pSO Pointer to the ImoStaffObj to which the request is referring.
    */
    GmoShape* find_shape_for_object(ImoStaffObj* pSO);

    /** Returns pointer to the main GmoShape generated by an ImoNoteRest.
        @param pNR Pointer to the ImoNoteRest to which the request is referring.
    */
    GmoShape* get_shape_for_noterest(ImoNoteRest* pNR);

    /** Returns pointer to the innermost GmoBox in which the shape for an ImoObj is
        contained.
        @param id The Id of the ImoObj to which the request is referring.
    */
    GmoBox* get_box_for_imo(ImoId id);

    //@}    //access to shapes and boxes related to an ImoObj


    //information for clicked point
    /// @name Information for clicked point
    //@{

    /** Returns pointer to the shape (if any) located in the given cordinates of a
        document page. If no shape there, returns @nullptr.
        @param iPage The number of the page (0..n-1).
        @param x,y The relative coordinates for the point (logical units referred to
            the top-left corner of the page).
    */
    GmoShape* find_shape_at(int iPage, LUnits x, LUnits y);

    /** Returns pointer to the innermost GmoBox located in the given cordinates of a
        document page.
        @param iPage The number of the page (0..n-1).
        @param x,y The relative coordinates for the point (logical units referred to
            the top-left corner of the page).
    */
    GmoBox* find_inner_box_at(int iPage, LUnits x, LUnits y);

    /** Returns pointer to an AreaInfo object containing the classification information
        and pointers to shape and boxes related to the given cordinates of a
        document page.
        @param iPage The number of the page (0..n-1).
        @param x,y The relative coordinates for the point (logical units referred to
            the top-left corner of the page).
    */
    AreaInfo* get_info_for_point(int iPage, LUnits x, LUnits y);

    //@}    //information for clicked point


    ///@cond INTERNALS
    //excluded from public API. Only for internal use.

    inline void set_modified(bool value) { m_modified = value; }
    inline bool is_modified() { return m_modified; }
    inline long get_model_id() { return m_modelId; }

    //drawing
    void draw_page(int iPage, UPoint& origin, Drawer* pDrawer, RenderOptions& opt);
    //void highlight_object(ImoStaffObj* pSO, bool value);

    //hit testing and related
    GmoObj* hit_test(int iPage, LUnits x, LUnits y);

    //selection
    void select_objects_in_rectangle(int iPage, SelectionSet* pSelection,
                                     const URect& selRect, unsigned flags=0);

    //creation
    ScoreStub* add_stub_for(ImoScore* pScore);
    void store_in_map_imo_shape(ImoObj* pImo, GmoShape* pShape);
    void add_to_map_imo_to_box(GmoBox* child);
    void add_to_map_ref_to_box(GmoBox* pBox);
    void build_main_boxes_table();

    //access to objects/information
    GmoObj* get_box_for_control(GmoRef gref);

    //tests
    void dump_page(int iPage, ostream& outStream);


	///@endcond


protected:
    ScoreStub* get_stub_for(ImoId scoreId);

};


///@cond INTERNALS
//excluded from public API. Only for internal use.

//---------------------------------------------------------------------------------------
/** Algorithms for finding info in the graphical model. It is a collection of static
    methods that encapsulates the knowledge for traversing the @GM. The objective is
    to facilitate maintenace and to minimize the impact on Lomse and on user apps. of
    any future changes in the @GM.
*/
class GModelAlgorithms
{
protected:

public:
    GModelAlgorithms() {}

    /** mouse point is over a box pGmo. Find box system. If pGmo points to an inner
        GmoBox (e.g. GmoShapeStaff) it returns the parent GmoBoxSystem and the
        y coordinate is not used. But if the pGmo points to an outer GmoBox (e.g.
        GmoBoxScorePage) it is necessary to use the y coordinate to find the
        GmoBoxSystem at that vertical position.

        AWARE: This method is oriented to mouse clicks and will fail if passed box is
        other than the expected in a mouse click (e.g. a GmoBoxSlice). Perhaps should
        be refactored and extended or restricted for internal use.
    */
    static GmoBoxSystem* get_box_system_for(GmoObj* pGmo, LUnits y);

    ///Get info about a clicked point
    static ClickPointData find_info_for_point(LUnits x, LUnits y, GmoObj* pGmo);

//    static GmoBoxSystem* get_system_for(const MeasureLocator& ml);

};
///@endcond


}   //namespace lomse

#endif      //__LOMSE_GRAPHICAL_MODEL_H__

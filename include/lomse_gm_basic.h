//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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

#ifndef __LOMSE_GM_BASIC_H__
#define __LOMSE_GM_BASIC_H__

#include "lomse_basic.h"
#include <vector>
#include <list>

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
class GmoBoxSystem;
class GmoShape;
class GmoShapeStaff;
class GmoStub;
class GmoStubScore;
class ImoDocObj;
class ImoObj;
class ImoScore;
class Drawer;
struct RenderOptions;


//the graphic model
//---------------------------------------------------------------------------------------
class GraphicModel
{
protected:
    GmoBoxDocument* m_root;
    std::vector<GmoStub*> m_stubs;
    bool m_fCanBeDrawn;

public:
    GraphicModel();
    ~GraphicModel();

    inline GmoBoxDocument* get_root() { return m_root; }
    int get_num_pages();
    //////GmoBox* create_main_box_for(ImoDocObj* pItem);
    GmoBoxDocPage* get_page(int i);
    inline void add_stub(GmoStub* pStub) { m_stubs.push_back(pStub); }
    GmoStubScore* get_score_stub(int i);

    //drawing
    inline void set_ready(bool value) { m_fCanBeDrawn = value; }
    void draw_page(int iPage, UPoint& origin, Drawer* pDrawer, RenderOptions& opt);

protected:
    void delete_stubs();

};

//Abstract class from which all graphic objects must derive
//---------------------------------------------------------------------------------------
class GmoObj
{
protected:
    GmoObj* m_pOwnerGmo;
    int m_objtype;
    USize m_size;
    UPoint m_origin;

public:
    virtual ~GmoObj();

    enum { k_box = 0,
                k_box_document=0, k_box_doc_page, k_box_doc_page_content,
                k_box_score_page, k_box_slice, k_box_slice_instr, k_box_system,
           k_shape,
                k_shape_brace, k_shape_bracket, k_shape_clef, k_shape_staff,
                k_shape_text,
    
           k_stub,
                k_stub_score,
         };

    //classification
    inline int get_gmobj_type() { return m_objtype; }
    inline bool is_box_document() { return m_objtype == k_box_document; }
    inline bool is_box_doc_page() { return m_objtype == k_box_doc_page; }
    inline bool is_box_doc_page_content() { return m_objtype == k_box_doc_page_content; }
    inline bool is_box_score_page() { return m_objtype == k_box_score_page; }
    inline bool is_box_slice() { return m_objtype == k_box_slice; }
    inline bool is_box_slice_instr() { return m_objtype == k_box_slice_instr; }
    inline bool is_box_system() { return m_objtype == k_box_system; }
    inline bool is_shape_brace() { return m_objtype == k_shape_brace; }
    inline bool is_shape_bracket() { return m_objtype == k_shape_bracket; }
    inline bool is_shape_clef() { return m_objtype == k_shape_clef; }
    inline bool is_shape_staff() { return m_objtype == k_shape_staff; }
    inline bool is_shape_text() { return m_objtype == k_shape_text; }
    inline bool is_stub_score() { return m_objtype == k_stub_score; }

    //size
    inline LUnits get_width() { return m_size.width; }
    inline LUnits get_height() { return m_size.height; }
    inline void set_width(LUnits width) { m_size.width = width; }
    inline void set_height(LUnits height) { m_size.height = height; }

    //position
    inline LUnits get_left() { return m_origin.x; }
    inline LUnits get_top() { return m_origin.y; }
    inline LUnits get_right() { return m_origin.x + m_size.width; }
    inline LUnits get_bottom() { return m_origin.y + m_size.height; }
    void set_origin(UPoint& pos);
    void set_origin(LUnits xLeft, LUnits yTop);
    void set_left(LUnits xLeft);
    void set_top(LUnits yTop);
    virtual void shift_origin(USize& shift) = 0;

protected:
    GmoObj(GmoObj* owner, int objtype);

};

//---------------------------------------------------------------------------------------
class GmoShape : public GmoObj
{
protected:
    int m_layer;
	//lmBox*		m_pOwnerBox;	//box in which this shape is included
	//bool		m_fVisible;
 //   wxWindow*   m_pMouseCursorWindow;      //to optimize mouse cursor changes

	////list of shapes attached to this one
	//std::list<lmAttachPoint*>	m_cAttachments;

	////list of shapes to which this one is attached
	//std::list<GmoShape*>	        m_cAttachedTo;

	//wxColour	m_color;
 //   long        m_nOrder;
 //   long        m_nLayer;

	////for composite shapes
	//GmoShape*	m_pParentShape;

public:
    virtual ~GmoShape();

    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt, UPoint& origin) = 0;

    //-------------------------------------------------------------------------
    // layer identifiers. Shapes are placed in layers. The first layer to 
    // render is layer 0 (background). Then, layer 1 (staves), and so on.
    enum { k_layer_background = 0, k_layer_staff, k_layer_barlines, k_layer_notes,
           k_layer_aux_objs, k_layer_top, k_layer_user,
           k_layer_max };

    //layer
    inline int get_layer() { return m_layer; }
    inline void set_layer(int layer) { m_layer = layer; }
  //void Render(lmPaper* pPaper);
 //   virtual void Render(lmPaper* pPaper, wxColour color) { GmoObj::Render(pPaper, color); }
 //   virtual void RenderWithHandlers(lmPaper* pPaper) {}

 //   virtual bool Collision(GmoShape* pShape);

 //   //methods related to position
	//virtual void OnAttachmentPointMoved(GmoShape* pShape, lmEAttachType nTag,
	//									lmLUnits ux, lmLUnits uy, lmEParentEvent nEvent) {}

	////shapes can be attached to other shapes
	//int Attach(GmoShape* pShape, lmEAttachType nType = lm_eGMA_Simple);
	//void Detach(GmoShape* pShape, bool fInform=true);
 //   void OnAttached(GmoShape* pShape);
 //   void OnDetached(GmoShape* pShape);

 //   //Debug related methods
 //   virtual wxString Dump(int nIndent);
 //   wxString DumpSelRect();

	////visibility
	//inline bool IsVisible() const { return m_fVisible; }
	//void SetVisible(bool fVisible) { m_fVisible = fVisible; }

 //   //info
	//virtual int GetPageNumber() const;
 //   lmBoxScore* GetOwnerBoxScore();
 //   lmBoxPage* GetOwnerBoxPage();

	////owners and related
	//inline lmBox* GetOwnerBox() { return m_pOwnerBox; }
	//inline void SetOwnerBox(lmBox* pOwnerBox) { m_pOwnerBox = pOwnerBox; }
	//lmBoxSystem* GetOwnerSystem() { return m_pOwnerBox->GetOwnerSystem(); }

	////for composite shapes
	//inline bool IsChildShape() const { return (bool)(m_pParentShape != (GmoShape*)NULL ); }
	//inline GmoShape* GetParentShape() { return m_pParentShape; }
	//inline void SetParentShape(GmoShape* pShape) { m_pParentShape = pShape; }

 //   //selection
 //   bool IsInRectangle(lmURect& rect);

 //   //call backs
 //   void OnMouseIn(wxWindow* pWindow, lmUPoint& uPoint);
 //   void OnMouseOut(wxWindow* pWindow, lmUPoint& uPoint);

 //   //vertex source
 //   virtual void RewindVertices(int nPathId = 0) {}
 //   virtual unsigned GetVertex(lmLUnits* pux, lmLUnits* puy);

 //   //properties
 //   virtual void SetColour(wxColour color) { m_color = color; }
 //   inline void SetOrder(long nOrder) { m_nOrder = nOrder; }
 //   inline long GetOrder() { return m_nOrder; }
 //   inline void SetLayer(long nLayer) { m_nLayer = nLayer; }
 //   inline long GetLayer() { return m_nLayer; }
    void shift_origin(USize& shift);

protected:
    GmoShape(GmoObj* owner, int objtype);
   // GmoShape(lmEGMOType m_nType, lmScoreObj* pOwner, int nOwnerIdx, wxString sName=_T("Shape"),
			//bool fDraggable = false, bool fSelectable = false, wxColour color = *wxBLACK,
			//bool fVisible = true);
	//void InformAttachedShapes(lmLUnits ux, lmLUnits uy, lmEParentEvent nEvent);

};

//---------------------------------------------------------------------------------------
class GmoBox : public GmoObj
{
protected:
    std::vector<GmoBox*> m_childBoxes;      //contained boxes
	std::list<GmoShape*> m_shapes;		    //contained shapes, organised in layers
    GmoShape* m_pStartOfLayer[GmoShape::k_layer_max]; //ptrs to start of each layer

    // All boxes have four margins (top, bottom, left and right) around the 
    // box area (bounds rectangle). The margins defines a smaller rectangle
    // (content rectangle) contained into the bounds rectangle. You can consider 
    // the box as a content rectanglesorrounded blank space, defined by the margins.
    LUnits m_uTopMargin;
    LUnits m_uBottomMargin;
    LUnits m_uLeftMargin;
    LUnits m_uRightMargin;

public:
    virtual ~GmoBox();

    //child boxes
    inline int get_num_boxes() { return static_cast<int>( m_childBoxes.size() ); }
    inline void add_child_box(GmoBox* child) { m_childBoxes.push_back(child); }
    GmoBox* get_child_box(int i);  //i = 0..n-1

    //contained shapes
    inline int get_num_shapes() { return static_cast<int>( m_shapes.size() ); }
    virtual void add_shape(GmoShape* shape, int layer);
    GmoShape* get_shape(int i);  //i = 0..n-1
    inline GmoShape* get_first_shape_for_layer(int layer) { return m_pStartOfLayer[layer]; }

    //margins
    inline LUnits get_top_margin() { return m_uTopMargin; }
    inline LUnits get_bottom_margin() { return m_uBottomMargin; }
    inline LUnits get_left_margin() { return m_uLeftMargin; }
    inline LUnits get_right_margin() { return m_uRightMargin; }
    inline void set_top_margin(LUnits space) { m_uTopMargin = space; }
    inline void set_bottom_margin(LUnits space) { m_uBottomMargin = space; }
    inline void set_left_margin(LUnits space) { m_uLeftMargin = space; }
    inline void set_right_margin(LUnits space) { m_uRightMargin = space; }

    //content size & position
    inline LUnits get_content_width() { return get_width() - m_uLeftMargin - m_uRightMargin; }
    inline LUnits get_content_height() { return get_height() - m_uTopMargin - m_uBottomMargin; }
    inline LUnits get_content_top() { return get_top() + m_uTopMargin; }
    inline LUnits get_content_left() { return get_left() + m_uLeftMargin; }

    //drawing
    void on_draw(Drawer* pDrawer, RenderOptions& opt, UPoint& origin);

protected:
    GmoBox(GmoObj* owner, int objtype);
    void shift_origin(USize& shift);
    void delete_boxes();
    void delete_shapes();
    bool draw_box_requested(RenderOptions& opt);
    Color get_box_color();
    void draw_box_bounds(Drawer* pDrawer, double xorg, double yorg, Color& color);

};

//---------------------------------------------------------------------------------------
class GmoStub : public GmoObj
{
protected:
    ImoObj* m_pImoOwner;

public:
    virtual ~GmoStub() {}

protected:
    GmoStub(int objtype, ImoObj* pImo) : GmoObj(NULL, objtype), m_pImoOwner(pImo) {}
    void shift_origin(USize& shift) {}

};

//---------------------------------------------------------------------------------------
class GmoBoxDocument : public GmoBox
{
protected:
    GmoBoxDocPage* m_pLastPage;

public:
    GmoBoxDocument();
    ~GmoBoxDocument() {}

    //doc pages
    GmoBoxDocPage* add_new_page();
    GmoBoxDocPage* get_page(int i);     //i = 0..n-1
    inline int get_num_pages() { return get_num_boxes(); }
    inline GmoBoxDocPage* get_last_page() { return m_pLastPage; }

};

//---------------------------------------------------------------------------------------
class GmoBoxDocPage : public GmoBox
{
protected:
    int m_numPage;

public:
    GmoBoxDocPage(GmoObj* owner);
    ~GmoBoxDocPage() {}

    inline void set_number(int num) { m_numPage = num; }
    inline int get_number() { return m_numPage; }
    void on_draw(Drawer* pDrawer, RenderOptions& opt, UPoint& origin); 

protected:
    void draw_page_background(Drawer* pDrawer, RenderOptions& opt, UPoint& origin);
};

//---------------------------------------------------------------------------------------
class GmoBoxDocPageContent : public GmoBox
{
protected:

public:
    GmoBoxDocPageContent(GmoObj* owner);
    ~GmoBoxDocPageContent() {}

};

//---------------------------------------------------------------------------------------
class GmoBoxScorePage : public GmoBox
{
protected:
    // a GmoBoxScorePage is, mainly, a collection of GmoBoxSystem objects
//
    GmoStubScore*   m_pStubScore;       //parent score stub
//    int             m_nNumPage;         //this page number (1..n)
    int             m_nFirstSystem;     //0..n-1
    int             m_nLastSystem;      //0..n-1
//    lmUPoint        m_pageOrgL;         //page origin
//    lmShapeMargin*  m_pMarginShapes[4];
//    wxWindow*       m_pRenderWindow;    //the window on which the page is rendered
//    wxPoint         m_vOffset;          //page offset to apply.
//
//    //objects in this page to be rendered with handlers
//    std::vector<lmGMObject*>    m_GMObjsWithHandlers;
//
//	//list of active handlers contained within this page
//    std::list<lmHandler*>       m_ActiveHandlers;  
//
//    //layers and shapes
//	std::list<lmLayer*>	    m_Layers;		//contained shapes, ordered by layer

public:
    GmoBoxScorePage(GmoStubScore* pStub, GmoObj* owner);
    ~GmoBoxScorePage();

	//systems
    GmoBoxSystem* add_system(int iSystem);  //, LUnits uxPos, LUnits uyPos);
//    inline int get_num_first_system() const { return m_nFirstSystem; }
    inline int get_num_last_system() const { return m_nLastSystem; }
    inline int get_num_systems() { 
        return (m_nFirstSystem == -1 ? 0 : m_nLastSystem - m_nFirstSystem + 1);
    }
	GmoBoxSystem* get_system(int iSystem);		//nSystem = 0..n-1
//	int GetSystemNumber(GmoBoxSystem* pSystem);
//
	////operations
 //   void on_draw(Drawer* pDrawer, RenderOptions& opt, UPoint& origin); 
//    void RenderWithHandlers(lmPaper* pPaper);
//    void DrawAllHandlers(lmPaper* pPaper);
//    void OnNeedToDrawHandlers(lmGMObject* pGMO);
//
//    //selection
//    lmGMObject* FindShapeAtPos(lmUPoint& uPoint, bool fSelectable);
//
//
//    //renderization related
//    inline wxWindow* GetRenderWindow() { return m_pRenderWindow; }
//    inline wxPoint& GetRenderWindowOffset() { return m_vOffset; }
//    inline void SetRenderWindow(wxWindow* pWindow) { m_pRenderWindow = pWindow; }
//    inline void SetRenderWindowOffset(wxPoint& vOffset) { m_vOffset = vOffset; }
//
//    //implementation of virtual methods from base class
//    wxString Dump(int nIndent);
//	inline int GetPageNumber() const { return m_nNumPage; }
//
//	//owners and related
//	GmoBoxSystem* GetOwnerSystem() { return (GmoBoxSystem*)NULL; }
//    inline GmoStubScore* GetBoxScore() const { return m_pStubScore; }
//    inline GmoStubScore* GetOwnerBoxScore() { return m_pStubScore; }
//    inline GmoBoxScorePage* GetOwnerBoxPage() { return this; }
//
//    //active handlers
//	void AddActiveHandler(lmHandler* pHandler);
//
//    //layers and shapes
//    void AddShapeToLayer(lmShape* pShape, long nLayerID);
//    void PopulateLayers();
//
//
//private:
//    void CreateLayers();
//

};

//---------------------------------------------------------------------------------------
class GmoStubScore : public GmoStub
{
protected:
	std::vector<GmoBoxScorePage*> m_pages;

//    //selected objects
//    lmGMSelection   m_Selection;        //info about selected objects

public:
    GmoStubScore(ImoScore* pScore);
    ~GmoStubScore();

    void add_page(GmoBoxScorePage* pPage);
//    void RenderPage(int nPage, lmPaper* pPaper, wxWindow* pRenderWindow,
//                    wxPoint& vOffset); 
//    void PopulateLayers();
//
    // pages
//    inline GmoBoxScorePage* GetCurrentPage() const { return m_pages.back(); }
    GmoBoxScorePage* get_page(int iPage);
    int get_num_pages();

    // systems
	int get_num_systems();
//	GmoBoxSystem* get_system(int nSystem);	//nSystem = 1..n
//
//    //selected objects management
//    inline lmGMSelection* GetSelection() { return &m_Selection; }
//    //void AddToSelection(lmGMSelection* pSelection);
//    void AddToSelection(lmGMObject* pGMO);
//    void AddToSelection(int nNumPage, LUnits uXMin, LUnits uXMax,
//                       LUnits uYMin, LUnits uYMax);
//    void RemoveFromSelection(lmGMObject* pGMO);
//    inline int GetNumObjectsSelected() { return m_Selection.NumObjects(); }
//    void ClearSelection();
//
//    //implementation of virtual methods from base class
//    wxString Dump(int nIndent);
//
//	//owners and related
//	GmoBoxSystem* GetOwnerSystem() { return (GmoBoxSystem*)NULL; }
//    inline GmoStubScore* GetOwnerBoxScore() { return this; }
//    inline GmoBoxScorePage* GetOwnerBoxPage() { return (GmoBoxScorePage*)NULL; }
//
//


};



}   //namespace lomse

#endif      //__LOMSE_GM_BASIC_H__

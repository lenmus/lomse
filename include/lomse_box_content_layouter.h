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

#ifndef __LOMSE_BOX_CONTENT_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_BOX_CONTENT_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_drawer.h"
#include "lomse_layouter.h"
#include "lomse_engrouters.h"
#include <sstream>

using namespace std;

namespace lomse
{

//forward declarations
class ImoContentObj;
class ImoControl;
class ImoInlineWrapper;
class ImoInlineObj;
class ImoBoxContent;
class ImoBoxInline;
class ImoStyles;
class ImoTextItem;
class ImoStyle;
class GraphicModel;
class GmoBox;


//---------------------------------------------------------------------------------------
// BoxContentLayouter: base class for layouters for BoxContent derived objects
class BoxContentLayouter : public Layouter
{
protected:
    LibraryScope& m_libraryScope;
    ImoBoxContent* m_pPara;
    std::list<Engrouter*> m_engrouters;
    bool m_fFirstLine;
    LUnits m_xLineStart;
    bool m_fAddShapesToModel;

public:
    BoxContentLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                      LibraryScope& libraryScope, ImoStyles* pStyles,
                      bool fAddShapesToModel=true);
    virtual ~BoxContentLayouter();

    //mandatory overrides
    void layout_in_box();
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height);
    void prepare_to_start_layout();

    //virtual methods to be implemented by derived classes
    virtual LUnits get_first_line_indent() = 0;
    virtual string get_first_line_prefix() = 0;

    //other
    inline LineReferences& get_line_refs() { return m_lineRefs; }

protected:
    void page_initializations(GmoBox* pMainBox);
    void create_engrouters();

    bool enough_space_in_box();
    LUnits add_engrouter_to_line();
    void add_engrouter_shape(Engrouter* pEngrouter, LUnits height);

    //helper: engrouters traversing
    std::list<Engrouter*>::iterator m_itEngrouters;
    inline void point_to_first_engrouter() { m_itEngrouters = m_engrouters.begin(); }
    inline bool more_engrouters() { return m_itEngrouters != m_engrouters.end(); }
    inline void next_engrouter() { ++m_itEngrouters; }
    inline Engrouter* get_current_engrouter() { return *m_itEngrouters; };

    //helper: space in current line
    LUnits m_availableSpace;
    inline bool space_in_line() { return m_availableSpace > (*m_itEngrouters)->get_width(); }

    //helper: info about next line to add to paragraph
    std::list<Engrouter*>::iterator m_itStart;       //first engrouter for this line
    std::list<Engrouter*>::iterator m_itEnd;         //first engrouter for next line
    LineReferences m_lineRefs;                       //reference lines
    LUnits m_lineWidth;

    //other
    inline bool is_first_line() { return m_fFirstLine; }
    inline void initialize_lines() { m_itStart = m_engrouters.end(); }
    inline bool is_line_ready() { return m_itStart != m_engrouters.end(); }
    void prepare_line();
    void add_line();
    void advance_current_line_space(LUnits left);
    void initialize_line_references();
    void set_line_pos_and_width();
    void update_line_references(LineReferences& engr, LUnits shift, bool fUpdateText);

};


//---------------------------------------------------------------------------------------
// ParagraphLayouter: layouts a paragraph
class ParagraphLayouter : public BoxContentLayouter
{
public:
    ParagraphLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                      LibraryScope& libraryScope, ImoStyles* pStyles);
    virtual ~ParagraphLayouter() {}

    //mandatory overrides
    LUnits get_first_line_indent() { return 0.0f; }
    string get_first_line_prefix() { return ""; }
};


//---------------------------------------------------------------------------------------
// ListItemLayouter: layouts a list item
class ListItemLayouter : public BoxContentLayouter
{
public:
    ListItemLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                      LibraryScope& libraryScope, ImoStyles* pStyles);
    virtual ~ListItemLayouter() {}

    //mandatory overrides
    LUnits get_first_line_indent();
    string get_first_line_prefix();

protected:

};


}   //namespace lomse

#endif    // __LOMSE_BOX_CONTENT_LAYOUTER_H__


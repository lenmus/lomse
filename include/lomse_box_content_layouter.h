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

#ifndef __LOMSE_BOX_CONTENT_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_BOX_CONTENT_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_drawer.h"
#include "lomse_layouter.h"
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
class Engrouter;
class ButtonEngrouter;
class ImageEngrouter;
class WordEngrouter;
class BoxContentLayouter;


//---------------------------------------------------------------------------------------
// helper, to contain information about a paragraph line
struct LineReferences
{
    LUnits lineHeight;
    LUnits baseline;
    LUnits textTop;           //mean value
    LUnits textBottom;        //mean value
    LUnits middleline;        //mean value
    LUnits supperLine;        //text top of last engrouter
    LUnits subLine;           //text bottom of last engrouter

    LineReferences()
        : lineHeight(0.0f)
        , baseline(0.0f)
        , textTop(0.0f)
        , textBottom(0.0f)
        , middleline(0.0f)
        , supperLine(0.0f)
        , subLine(0.0f)
    {
    }
    LineReferences(LUnits top, LUnits middle, LUnits base, LUnits bottom)
        : lineHeight(bottom + top)
        , baseline(base)
        , textTop(top)
        , textBottom(bottom)
        , middleline(middle)
        , supperLine(top)
        , subLine(base)
    {
    }

};


//---------------------------------------------------------------------------------------
// Engrouter: combines layouter + engraver for an inline object (abstract class)
class Engrouter
{
protected:
    UPoint m_org;           //top left
    USize m_size;           //width, height
    ImoContentObj* m_pCreatorImo;
    LibraryScope& m_libraryScope;
    ImoStyle* m_pStyle;

    //relative: shift from m_org
    LineReferences m_refLines;

public:
    virtual ~Engrouter() {}

    virtual void measure() = 0;
    virtual GmoObj* create_gm_object(UPoint pos, LineReferences& refs) = 0;
    virtual LUnits shift_for_vertical_alignment(LineReferences& refs);

    //size
    inline LUnits get_width() { return m_size.width; }
    inline LUnits get_height() { return m_size.height; }
    inline void set_size(const USize& size) { m_size = size; }
    inline void set_height(LUnits height) { m_size.height = height; }
    inline void set_size(LUnits width, LUnits height) {
        m_size.width = width;
        m_size.height = height;
    }

    //position
    inline void set_position(const UPoint& pos) { m_org = pos; }
    inline UPoint& get_position() { return m_org; }

    //style
    inline ImoStyle* get_style() { return m_pStyle; }

    //references
    inline LineReferences& get_reference_lines() { return m_refLines; }
    inline LUnits get_line_height() { return m_refLines.lineHeight; }
    inline LUnits get_base_line() { return m_refLines.baseline; }
    inline LUnits get_text_top() { return m_refLines.textTop; }
    inline LUnits get_text_bottom() { return m_refLines.textBottom; }
    inline LUnits get_middle_line() { return m_refLines.middleline; }


protected:
    Engrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope);
};

//---------------------------------------------------------------------------------------
// Engrouter for Boxes
class BoxEngrouter : public Engrouter
{
protected:
    std::list<Engrouter*> m_engrouters;

public:
    BoxEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope)
        : Engrouter(pCreatorImo, libraryScope) {}
    virtual ~BoxEngrouter();

    void measure();
    GmoObj* create_gm_object(UPoint pos, LineReferences& refs);
    void update_measures(LUnits lineHeight);

    std::list<Engrouter*>& get_engrouters() { return m_engrouters; }

    UPoint get_content_org();
    LUnits get_content_width();
    LUnits get_total_bottom_spacing();
    LUnits get_total_right_spacing();


protected:
    void add_engrouter_shape(GmoObj* pGmo, GmoBox* pBox);

};

//---------------------------------------------------------------------------------------
// Engrouter for a button
class ButtonEngrouter : public Engrouter
{
public:
    ButtonEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope)
        : Engrouter(pCreatorImo, libraryScope) {}
    virtual ~ButtonEngrouter() {}

    void measure();
    GmoObj* create_gm_object(UPoint pos, LineReferences& refs);
};

//---------------------------------------------------------------------------------------
// Engrouter for an image
class ImageEngrouter : public Engrouter
{
public:
    ImageEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope)
        : Engrouter(pCreatorImo, libraryScope) {}
    virtual ~ImageEngrouter() {}

    void measure();
    GmoObj* create_gm_object(UPoint pos, LineReferences& refs);
};

//---------------------------------------------------------------------------------------
//Engrouter for a word of text
class WordEngrouter : public Engrouter
{
protected:
    string m_word;
    LUnits m_descent;
    LUnits m_ascent;
    LUnits m_halfLeading;

public:
    WordEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope,
                  const std::string& word)
        : Engrouter(pCreatorImo, libraryScope)
        , m_word(word)
    {
    }
    virtual ~WordEngrouter() {}

    inline const string& get_text() { return m_word; }

    void measure();
    GmoObj* create_gm_object(UPoint pos, LineReferences& refs);

    //info
    inline LUnits get_descent() { return m_descent; }
    inline LUnits get_ascent() { return m_ascent; }
};


//---------------------------------------------------------------------------------------
// Engrouter for inline objs
class InlineWrapperEngrouter : public Engrouter
{
protected:
    ImoInlineObj* m_pWrapper;
    std::list<Engrouter*> m_engrouters;

public:
    InlineWrapperEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope);
    virtual ~InlineWrapperEngrouter() {}

    virtual void measure() = 0;
    virtual GmoObj* create_gm_object(UPoint pos, LineReferences& refs);

protected:
    LUnits add_engrouters_to_box(GmoBox* pBox, LineReferences& refs);
    void add_engrouter_shape(GmoObj* pGmo, GmoBox* pBox);
};

//---------------------------------------------------------------------------------------
// Container for an InlineBox
class InlineBoxEngrouter : public InlineWrapperEngrouter
{
protected:
    ImoInlineWrapper* m_pBox;

public:
    InlineBoxEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope);
    virtual ~InlineBoxEngrouter() {}

    void measure();
    GmoObj* create_gm_object(UPoint pos, LineReferences& refs);
};


//----------------------------------------------------------------------------------
// ControlEngrouter: layout algorithm for a gui control
class ControlEngrouter : public Engrouter
{
protected:
    ImoControl* m_pControl;

public:
    ControlEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope);
    virtual ~ControlEngrouter() {}

    //implementation of Engrouter virtual pure methods
    void measure();
    GmoObj* create_gm_object(UPoint pos, LineReferences& refs);
};


//---------------------------------------------------------------------------------------
// EngroutersCreator: splits paragraph content, creating engrouters for each atomic
// content object
class EngroutersCreator
{
protected:
    std::list<Engrouter*>& m_engrouters;
    LibraryScope& m_libraryScope;
    UPoint m_cursor;                //current position. Relative to BoxDocPage

public:
    EngroutersCreator(std::list<Engrouter*>& engrouters, LibraryScope& libraryScope);
    virtual ~EngroutersCreator();

    void create_engrouters(ImoInlineObj* pImo);

protected:
    BoxEngrouter* create_wrapper_engrouterbox_for(ImoBoxInline* pImo);
    void create_and_measure_engrouters(ImoBoxInline* pImo, BoxEngrouter* pBox);
    void layout_engrouters_in_engrouterbox_and_measure(BoxEngrouter* pBox);

    void create_text_item_engrouters(ImoTextItem* pText);
    void measure_engrouters();

};


//---------------------------------------------------------------------------------------
// BoxContentLayouter: layouts a paragraph
class BoxContentLayouter : public Layouter
{
protected:
    LibraryScope& m_libraryScope;
    ImoBoxContent* m_pPara;
    std::list<Engrouter*> m_engrouters;

public:
    BoxContentLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                      LibraryScope& libraryScope, ImoStyles* pStyles);
    virtual ~BoxContentLayouter();

    //virtual methods in base class
    void layout_in_box();
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height);
    void prepare_to_start_layout();

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

    inline void initialize_lines() { m_itStart = m_engrouters.end(); }
    inline bool is_line_ready() { return m_itStart != m_engrouters.end(); }
    void prepare_line();
    void add_line();
    void advance_current_line_space(LUnits left);
    void initialize_line_references();
    void update_line_references(LineReferences& engr, LUnits shift, bool fUpdateText);

};


}   //namespace lomse

#endif    // __LOMSE_BOX_CONTENT_LAYOUTER_H__


//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
class ImoInlineLevelObj;
class ImoInlinesContainer;
class ImoBoxInline;
class ImoStyles;
class ImoTextItem;
class ImoStyle;
class GraphicModel;
class GmoBox;


//---------------------------------------------------------------------------------------
// InlinesContainerLayouter: base class for layouters for BoxContent derived objects
class InlinesContainerLayouter : public Layouter
{
protected:
    LibraryScope& m_libraryScope;
    ImoInlinesContainer* m_pPara;
    std::list<Engrouter*> m_engrouters;     //for a line
    bool m_fFirstLine;
    LUnits m_xLineStart;
    EngroutersCreator* m_pEngrCreator;

    //bullets
    LUnits m_firstLineIndent;
    wstring m_firstLinePrefix;

public:
    InlinesContainerLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                      LibraryScope& libraryScope, ImoStyles* pStyles,
                      bool fAddShapesToModel=true);
    virtual ~InlinesContainerLayouter();

    //mandatory overrides
    void layout_in_box() override;
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height) override;
    void prepare_to_start_layout() override;

    //other
    inline LineReferences& get_line_refs() { return m_lineRefs; }

protected:
    void page_initializations(GmoBox* pMainBox);
//    void create_engrouters();

    Engrouter* create_next_engrouter(bool fRemoveLeftSpace);
    bool enough_space_in_box();
    void add_engrouter_to_line(Engrouter* pEngrouter);
    void add_engrouter_shape(Engrouter* pEngrouter, LUnits height);

    //bullet points
    void get_indent_and_bullet_info();
    inline LUnits get_first_line_indent() { return m_firstLineIndent; }
    inline wstring get_first_line_prefix() { return m_firstLinePrefix; }
    void add_bullet();

    //helper: space in current line
    LUnits m_availableSpace;
    inline bool space_in_line() { return m_availableSpace > 0.0f; }

    //helper: info about next line to add to paragraph
    LineReferences m_lineRefs;      //reference lines
    LUnits m_lineWidth;

    //other
    inline bool is_first_line() { return m_fFirstLine; }
    inline bool more_content() { return m_pEngrCreator->more_content(); }
    inline bool is_line_ready() { return m_engrouters.size() > 0; }
    void prepare_line();
    void add_line();
    void advance_current_line_space(LUnits left);
    void initialize_line_references();
    void set_line_pos_and_width();
    void update_line_references(LineReferences& engr, LUnits shift, bool fUpdateText);

};


}   //namespace lomse

#endif    // __LOMSE_BOX_CONTENT_LAYOUTER_H__


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

#ifndef __LOMSE_ENGROUTERS_H__        //to avoid nested includes
#define __LOMSE_ENGROUTERS_H__

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
class ImoInlineLevelObj;
class ImoInlinesContainer;
class ImoBoxInline;
class ImoStyles;
class ImoTextItem;
class ImoStyle;
class GraphicModel;
class GmoBox;
class TextSplitter;


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
    LineReferences m_refLines;  //relative: shift from m_org
    bool m_fBreakRequested;

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

    //information to guide parent layouter
    inline bool break_requested() { return m_fBreakRequested; }
    inline void set_break_requested() { m_fBreakRequested = true; }

    //for unit tests
    inline ImoContentObj* get_creator_imo() { return m_pCreatorImo; }

protected:
    Engrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope);
};

//---------------------------------------------------------------------------------------
// Engrouter for ImoInlineBox objects (imoLink, ImoInlinesWrapper)
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
    void layout_and_measure();
    void update_measures(LUnits lineHeight);

    inline void add_engrouter(Engrouter* pEngr) { m_engrouters.push_back(pEngr); }

    UPoint get_content_org();
    LUnits get_content_width();
    LUnits get_total_bottom_spacing();
    LUnits get_total_right_spacing();


    //only for unit tests
    inline std::list<Engrouter*>& get_engrouters() { return m_engrouters; }


protected:
    void add_engrouter_shape(GmoObj* pGmo, GmoBox* pBox);

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
//Engrouter for a chunk of text
class WordEngrouter : public Engrouter
{
protected:
    wstring m_text;
    string m_language;
    LUnits m_descent;
    LUnits m_ascent;
    LUnits m_halfLeading;

public:
    WordEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope,
                  const wstring& text);
    virtual ~WordEngrouter() {}

    //used only for tests
    inline const wstring& get_text() { return m_text; }

    void measure();
    GmoObj* create_gm_object(UPoint pos, LineReferences& refs);

    //info
    inline LUnits get_descent() { return m_descent; }
    inline LUnits get_ascent() { return m_ascent; }
    bool text_has_space_at_end();
};


//---------------------------------------------------------------------------------------
// Engrouter for inline objs
class InlineWrapperEngrouter : public Engrouter
{
protected:
    ImoInlineLevelObj* m_pWrapper;
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

////---------------------------------------------------------------------------------------
//// Container for an InlineBox
//class InlineBoxEngrouter : public InlineWrapperEngrouter
//{
//protected:
//    ImoInlineWrapper* m_pBox;
//
//public:
//    InlineBoxEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope);
//    virtual ~InlineBoxEngrouter() {}
//
//    void measure();
//    GmoObj* create_gm_object(UPoint pos, LineReferences& refs);
//};


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


//----------------------------------------------------------------------------------
// NullEngrouter: doesn't create any Gmo object
class NullEngrouter : public Engrouter
{
public:
    NullEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope)
        : Engrouter(pCreatorImo, libraryScope)
    {
    }
    virtual ~NullEngrouter() {}

    //implementation of Engrouter virtual pure methods
    void measure() {}
    GmoObj* create_gm_object(UPoint UNUSED(pos), LineReferences& UNUSED(refs))
    {
        return nullptr;
    }
};


//---------------------------------------------------------------------------------------
// EngroutersCreator: splits paragraph content, creating engrouters for each atomic
// content object
class EngroutersCreator
{
protected:
    LibraryScope& m_libraryScope;
    TreeNode<ImoObj>::children_iterator m_itCurContent;
    TreeNode<ImoObj>::children_iterator m_itEndContent;

    //helper:
    TextSplitter* m_pTextSplitter;  //text splitter for current ImoTextItem
    ImoTextItem* m_pCurText;        //current text item being processed
    Engrouter* m_pPendingEngr;      //next engrouter to add. It was created in previous
                                    //invocation to create_next_engrouter() but could not
                                    //be added to line because not enough space.

public:
    EngroutersCreator(LibraryScope& libraryScope,
                      TreeNode<ImoObj>::children_iterator itStart,
                      TreeNode<ImoObj>::children_iterator itEnd);
    virtual ~EngroutersCreator();

    Engrouter* create_next_engrouter(LUnits maxSpace, bool fFirstOfLine);
    bool more_content();

    Engrouter* create_prefix_engrouter(ImoInlinesContainer* pBoxContent,
                                       const wstring& prefix);

protected:
    Engrouter* create_engrouter_for(ImoInlineLevelObj* pImo);
    Engrouter* create_next_text_engrouter_for(ImoTextItem* pText, LUnits maxSpace,
                                              bool fFirstOfLine);
    Engrouter* first_text_engrouter_for(ImoTextItem* pText, LUnits maxSpace,
                                        bool fFirstOfLine);
    Engrouter* next_text_engouter(LUnits maxSpace, bool fFirstOfLine);
    Engrouter* create_wrapper_engrouter_for(ImoBoxInline* pIB, LUnits maxSpace);
    void create_engrouters_for_box_content(ImoBoxInline* pImo, BoxEngrouter* pBox);
    void layout_engrouters_in_engrouterbox_and_measure(BoxEngrouter* pBox);
    TextSplitter* create_text_splitter_for(ImoTextItem* pText);

    //helper
    inline bool is_there_a_pending_engrouter() { return m_pPendingEngr != nullptr; }
    inline void save_engrouter_for_next_call(Engrouter* pEngr) { m_pPendingEngr = pEngr; }
    inline Engrouter* get_pending_engrouter()
    {
        Engrouter* pEngr = m_pPendingEngr;
        m_pPendingEngr = nullptr;
        return pEngr;
    }

};


}   //namespace lomse

#endif    // __LOMSE_ENGROUTERS_H__


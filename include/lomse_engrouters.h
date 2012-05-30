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

////---------------------------------------------------------------------------------------
////Engrouter for BoxContent prefix
//class PrefixEngrouter : public Engrouter
//{
//protected:
//    string m_prefix;
//    LUnits m_descent;
//    LUnits m_ascent;
//    LUnits m_halfLeading;
//
//public:
//    PrefixEngrouter(ImoContentObj* pCreatorImo, LibraryScope& libraryScope,
//                    const std::string& prefix)
//        : Engrouter(pCreatorImo, libraryScope)
//        , m_prefix(prefix)
//    {
//    }
//    virtual ~PrefixEngrouter() {}
//
//    inline const string& get_text() { return m_word; }
//
//    void measure();
//    GmoObj* create_gm_object(UPoint pos, LineReferences& refs);
//
//    //info
//    inline LUnits get_descent() { return m_descent; }
//    inline LUnits get_ascent() { return m_ascent; }
//};


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

    void create_engrouters(ImoInlineLevelObj* pImo);
    void create_prefix_engrouter(ImoInlinesContainer* pBoxContent, const string& prefix);

protected:
    BoxEngrouter* create_wrapper_engrouterbox_for(ImoBoxInline* pImo);
    void create_and_measure_engrouters(ImoBoxInline* pImo, BoxEngrouter* pBox);
    void layout_engrouters_in_engrouterbox_and_measure(BoxEngrouter* pBox);

    void create_text_item_engrouters(ImoTextItem* pText);
    void measure_engrouters();

};


}   //namespace lomse

#endif    // __LOMSE_ENGROUTERS_H__


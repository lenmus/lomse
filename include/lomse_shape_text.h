//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_TEXT_H__        //to avoid nested includes
#define __LOMSE_SHAPE_TEXT_H__

#include "lomse_shape_base.h"
#include "lomse_shapes.h"
#include "lomse_basic.h"
#include "lomse_injectors.h"
#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class GmoBox;
class ImoStyle;
class FontStorage;

//---------------------------------------------------------------------------------------
class GmoShapeText : public GmoSimpleShape
{
protected:
    string m_text;
    string m_language;
    ImoStyle* m_pStyle;
    FontStorage* m_pFontStorage;
    LibraryScope& m_libraryScope;
    LUnits m_space;
    int m_classid;

public:
    GmoShapeText(ImoObj* pCreatorImo, ShapeId idx, const std::string& text,
                 ImoStyle* pStyle, const string& language, int classid,
                 LUnits xLeft, LUnits yBaseline, LibraryScope& libraryScope);

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //dynamic modification
    void set_text(const std::string& text);

protected:
    void select_font();
    Color get_normal_color() override;
    std::string get_id();
    std::string get_class();
    std::string get_id_prefix();

};

//---------------------------------------------------------------------------------------
class GmoShapeWord : public GmoSimpleShape
{
protected:
    const wstring m_text;
    const string m_language;
    ImoStyle* m_pStyle;
    FontStorage* m_pFontStorage;
    LibraryScope& m_libraryScope;
    LUnits m_halfLeading;
    LUnits m_baseline;          //relative to m_origin.y

public:
    GmoShapeWord(ImoObj* pCreatorImo, ShapeId idx, const wstring& text,
                 ImoStyle* pStyle, const string& language, LUnits x, LUnits y,
                 LUnits halfLeading, LibraryScope& libraryScope);

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //for unit tests
    inline LUnits get_top_line() { return m_origin.y + m_halfLeading; }
    inline LUnits get_baseline() { return m_baseline + m_origin.y; }
    inline LUnits get_bottom_line() { return m_origin.y + m_size.height - m_halfLeading; }

protected:
    void select_font();
    Color get_normal_color() override;

};

////---------------------------------------------------------------------------------------
///** %GmoShapeLyricText represents any textual content (syllable, elision symbol,
//    hyphenation) for the lyric content associated to a note/rest.
//*/
//class GmoShapeLyricText : public GmoShapeText
//{
//public:
//    GmoShapeLyricText(ImoObj* pCreatorImo, ShapeId idx, const std::string& text,
//                      ImoStyle* pStyle, const string& language,
//                      LUnits xLeft, LUnits yBaseline, LibraryScope& libraryScope)
//        : GmoShapeText(pCreatorImo, idx, text, pStyle, language,
//                            xLeft, yBaseline, libraryScope)
//    {
//    }
//
//    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;
//};


//---------------------------------------------------------------------------------------
/** %GmoShapeTextBox represents a text (several lines) surrounded by a rectangle
*/
//class TextLine;
//
class GmoShapeTextBox : public GmoShapeRectangle
{
protected:
    string m_text;
    string m_language;
    ImoStyle* m_pStyle;
    FontStorage* m_pFontStorage;
    LibraryScope& m_libraryScope;

public:
    GmoShapeTextBox(ImoObj* pCreatorImo, ShapeId idx
                    , const string& text                //text inside the box
                    , const string& language            //text language
                    , ImoStyle* pStyle                  //for the text & the box
                    , LibraryScope& libraryScope
                    , const UPoint& pos = UPoint(0.0f, 0.0f)    //top-left corner
                    , const USize& size = USize(0.0f, 0.0f)     //rectangle size
                    , LUnits radius = 0.0f                      //for rounded corners
                    );

    virtual ~GmoShapeTextBox();

    //implementation of virtual methods from base class
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

private:

    void draw_text(Drawer* pDrawer, RenderOptions& opt);
    void select_font();
};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_TEXT_H__


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

#ifndef _LOMSE_CHECKBOX_CTRL_H__
#define _LOMSE_CHECKBOX_CTRL_H__

#include "lomse_control.h"

#include "lomse_vertex_source.h"

namespace lomse
{

//---------------------------------------------------------------------------------------
// A checkable box plus a static text element and/or and image.
class CheckboxCtrl : public Control
                   , public VertexSource
{
protected:
    string m_label;
    GmoBoxControl* m_pMainBox;
    UPoint  m_pos;
    LUnits  m_width;
    LUnits  m_height;
    LUnits  m_xCenter;
    LUnits  m_yCenter;

    Color   m_normalColor;
    Color   m_hoverColor;
    Color   m_prevColor;
    Color   m_currentColor;

private:
    bool    m_status;
    int     m_nCurVertex;   //index to current vertex
    static  Vertex m_tickVertices[];
    static  const int m_nNumVertices;

public:
    CheckboxCtrl(LibraryScope& libScope, Control* pParent, Document* pDoc,
                 const string& label, LUnits width=-1.0f, LUnits height=-1.0f,
                 ImoStyle* pStyle=nullptr);
    virtual ~CheckboxCtrl() {}

    //Control mandatory overrides
    USize measure();
    GmoBoxControl* layout(LibraryScope& libraryScope, UPoint pos);
    void on_draw(Drawer* pDrawer, RenderOptions& opt);
    void handle_event(SpEventInfo pEvent);
    LUnits width() { return m_width; }
    LUnits height() { return m_height; }
    LUnits top() { return m_pos.y; }
    LUnits bottom() { return m_pos.y + m_height; }
    LUnits left() { return m_pos.x; }
    LUnits right() { return m_pos.x + m_width; }

    //specific methods
    void set_text(const string& text);
    void set_tooltip(const string& text);
    void change_label(const string& text);
    inline bool is_checked() { return m_status; }
    inline void set_checked(bool value) { m_status = value; }

    //VertexSource mandatory methods
    unsigned vertex(double* px, double* py);
    void rewind(int UNUSED(pathId) = 0) { m_nCurVertex = 0; }

protected:
    void select_font();
    URect determine_text_position_and_size();
    ImoStyle* create_default_style();

};


} //namespace lomse

#endif    //_LOMSE_CHECKBOX_CTRL_H__

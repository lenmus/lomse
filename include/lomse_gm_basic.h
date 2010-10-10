//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_GM_BASIC_H__
#define __LOMSE_GM_BASIC_H__

#include <vector>
//#include <iostream>
//#include "lomse_observable.h"

using namespace std;

namespace lomse
{

//forward declarations
class GmoObj;
class GmoBox;
class GmoBoxDocument;
class GmoBoxDocPage;
class GmoBoxScore;
class GmoShape;


//the graphic model
//------------------------------------------------------------------------------
class GraphicModel
{
protected:
    GmoBoxDocument* m_root;

public:
    GraphicModel();
    ~GraphicModel();

    inline GmoBoxDocument* get_root() { return m_root; }
    int get_num_pages();

protected:

    //void create_shapes();
    //void layout_model();
    //void render_model();

};


//Abstract class from which all graphic objects must derive
//------------------------------------------------------------------------------
class GmoObj
{
protected:
    GmoObj* m_pOwnerGmo;

public:
    GmoObj(GmoObj* owner);
    virtual ~GmoObj();

};

//------------------------------------------------------------------------------
class GmoBox : public GmoObj
{
protected:
    std::vector<GmoBox*> m_childBoxes;

public:
    GmoBox(GmoObj* owner) : GmoObj(owner) {}
    virtual ~GmoBox() {}

    //child boxes
    inline int get_num_children() { return static_cast<int>( m_childBoxes.size() ); }
    inline void add_child_box(GmoBox* child) { m_childBoxes.push_back(child); }
    GmoBox* GmoBox::get_child_box(int i);  //i = 0..n-1

};

//------------------------------------------------------------------------------
class GmoShape : public GmoObj
{
protected:

public:
    GmoShape(GmoObj* owner) : GmoObj(owner) {}
    virtual ~GmoShape() {}

};

//------------------------------------------------------------------------------
class GmoBoxDocument : public GmoBox
{
protected:

public:
    GmoBoxDocument();
    ~GmoBoxDocument() {}

    //doc pages
    GmoBoxDocPage* add_new_page();
    GmoBoxDocPage* get_page(int i);     //i = 0..n-1
    inline int get_num_pages() { return get_num_children(); }
};

//------------------------------------------------------------------------------
class GmoBoxDocPage : public GmoBox
{
protected:
    int m_numPage;

public:
    GmoBoxDocPage(GmoObj* owner);
    ~GmoBoxDocPage() {}

    inline void set_number(int num) { m_numPage = num; }
    inline int get_number() { return m_numPage; }

};

//------------------------------------------------------------------------------
class GmoBoxScore : public GmoBox
{
protected:

public:
    GmoBoxScore(GmoObj* owner) : GmoBox(owner) {}
    ~GmoBoxScore() {}

};



}   //namespace lomse

#endif      //__LOMSE_GM_BASIC_H__

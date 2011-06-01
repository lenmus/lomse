//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_CONTENT_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_CONTENT_LAYOUTER_H__

//#include <sstream>
//
//using namespace std;

namespace lomse
{

//forward declarations
class ImoContentObj;
class GraphicModel;
class GmoBox;


//----------------------------------------------------------------------------------
// ContentLayouter
// Abstract class to implement the layout algorithm for any document content item.
class ContentLayouter
{
protected:
    ImoContentObj* m_pImo;
    bool m_fIsLayouted;
    GraphicModel* m_pGModel;

public:
    ContentLayouter(ImoContentObj* pImo, GraphicModel* pGModel)
        : m_pImo(pImo)
        , m_fIsLayouted(false)
        , m_pGModel(pGModel)
    {
    }

    virtual ~ContentLayouter() {}

    virtual void layout_in_page(GmoBox* pContainerBox) = 0;
    virtual void prepare_to_start_layout() { m_fIsLayouted = false; }
    virtual bool is_item_layouted() { return m_fIsLayouted; }
    virtual void set_layout_is_finished(bool value) { m_fIsLayouted = value; }
    virtual GmoBox* create_main_box() = 0;
};


//----------------------------------------------------------------------------------
class NullLayouter : public ContentLayouter
{
protected:

public:
    NullLayouter(ImoContentObj* pImo, GraphicModel* pGModel)
        : ContentLayouter(pImo, pGModel) {}
    ~NullLayouter() {}

    void layout_in_page(GmoBox* pContainerBox) {}
    bool is_item_layouted() { return true; }
    GmoBox* create_main_box() { return 0; }
};


}   //namespace lomse

#endif    // __LOMSE_CONTENT_LAYOUTER_H__


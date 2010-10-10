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

#ifndef __LOMSE_CONTENT_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_CONTENT_LAYOUTER_H__

//#include <sstream>
//
//using namespace std;

namespace lomse
{

//forward declarations
class ImoDocObj;
class GmoBox;


// ContentLayouter
// Abstract class to implement the layout algorithm for any document content item.
//----------------------------------------------------------------------------------
class ContentLayouter
{
protected:
    ImoDocObj* m_pImo;

public:
    ContentLayouter(ImoDocObj* pImo) : m_pImo(pImo) {}
    virtual ~ContentLayouter() {}

    virtual void do_layout(GmoBox* pContainerBox) = 0;
};


//----------------------------------------------------------------------------------
class NullLayouter : public ContentLayouter
{
protected:

public:
    NullLayouter(ImoDocObj* pImo) : ContentLayouter(pImo) {}
    ~NullLayouter() {}

    void do_layout(GmoBox* pContainerBox) {}
};


}   //namespace lomse

#endif    // __LOMSE_CONTENT_LAYOUTER_H__


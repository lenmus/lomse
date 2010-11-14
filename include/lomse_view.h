//---------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_VIEW_H__
#define __LOMSE_VIEW_H__

#include "lomse_basic.h"
#include "lomse_observable.h"
//#include <list>
//#include <iostream>
//using namespace std;
//#include "lomse_document_cursor.h"


namespace lomse
{

//forward declarations
class Document;
class GraphicModel;
//class MvcElement;
//class Controller;


// Abstract class from which all views must derive
//---------------------------------------------------------------------------------------
class View : public Observer
{
protected:
    Document*   m_pDoc;
    //Controller* m_pController;
    //MvcElement* m_pOwner;

public:
    View(Document* pDoc); //, Controller* pController);
    virtual ~View();

    //virtual void on_document_reloaded()=0;

    //void set_owner(MvcElement* pMvc) { m_pOwner = pMvc; }
    //inline Controller* get_controller() { return m_pController; }

};


}   //namespace lomse

#endif      //__LOMSE_VIEW_H__

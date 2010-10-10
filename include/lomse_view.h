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
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_VIEW_H__
#define __LOMSE_VIEW_H__

#include <list>
#include <iostream>
//#include "lomse_document_cursor.h"
#include "lomse_observable.h"

using namespace std;

namespace lomse
{

//forward declarations
class Document;
class GraphicModel;
//class MvcElement;
//class Controller;


//Abstract class from which all views must derive
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


//A view to edit the score in full page
class EditView : public View
{
protected:
    GraphicModel* m_pGraficModel;
    //DocCursor       m_cursor;

public:

    EditView(Document* pDoc);   //, Controller* pController);
    ~EditView();

    //inline DocCursor& get_cursor() { return m_cursor; }
    //void on_document_reloaded();

    ////caret movement
    //void caret_right();
    //void caret_left();
    //void caret_to_object(long nId);

    //observed object notifications
	void handle_event(Observable* ref);

    //graphic model
    GraphicModel* get_graphic_model();

protected:
    GraphicModel* create_graphic_model();

};



}   //namespace lomse

#endif      //__LOMSE_VIEW_H__

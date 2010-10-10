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

#include "lomse_view.h"

#include <sstream>
#include "lomse_document.h"
#include "lomse_gm_basic.h"
//#include "lomse_mvc_builder.h"
//#include "lomse_controller.h"

using namespace std;

namespace lomse
{

//-------------------------------------------------------------------------------------
// View implementation
//-------------------------------------------------------------------------------------

View::View(Document* pDoc)  //, Controller* pController)
    : Observer()
    , m_pDoc(pDoc)
    //, m_pController(pController)
    //, m_pOwner(NULL)
{
} 

View::~View()
{
    //delete m_pController;
}



//-------------------------------------------------------------------------------------
// EditView implementation
//-------------------------------------------------------------------------------------

EditView::EditView(Document* pDoc)  //, Controller* pController)
    : View(pDoc)    //, pController)
    , m_pGraficModel(NULL)
    //, m_cursor(pDoc)
{
} 

EditView::~EditView()
{
    if (m_pGraficModel)
        delete m_pGraficModel;
}

GraphicModel* EditView::get_graphic_model()
{
    if (!m_pGraficModel)
        m_pGraficModel = create_graphic_model();
    return m_pGraficModel;
}

GraphicModel* EditView::create_graphic_model()
{
    return new GraphicModel();
}

//void EditView::on_document_reloaded()
//{
//    DocCursor cursor(m_pDoc);
//    m_cursor = cursor;
//}

//void EditView::caret_right()
//{
//    m_cursor.move_next();
//}
//
//void EditView::caret_left()
//{
//    m_cursor.move_prev();
//}
//
//void EditView::caret_to_object(long nId)
//{
//    m_cursor.reset_and_point_to(nId);
//}

void EditView::handle_event(Observable* ref)
{
    //TODO: This method is required by base class Observer
    //if (m_pOwner)
    //{
    //    Notification event(m_pOwner, m_pOwner->get_document(), this);
    //    m_pOwner->notify_user_application(&event);
    //}
}


}  //namespace lomse

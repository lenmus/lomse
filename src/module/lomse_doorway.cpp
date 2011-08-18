//---------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------

#include "lomse_doorway.h"

#include "lomse_injectors.h"
#include "lomse_agg_types.h"
#include "lomse_presenter.h"
#include "lomse_events.h"

namespace lomse
{

//=======================================================================================
//platform independent methods
//=======================================================================================
LomseDoorway::LomseDoorway()
    : m_pLibraryScope(NULL)
    , m_pFunc_notify(null_notify_function)
    , m_pFunc_request(null_request_function)
{
}

//---------------------------------------------------------------------------------------
LomseDoorway::~LomseDoorway()
{
    delete m_pLibraryScope;
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::new_document(int viewType)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.new_document(viewType);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::new_document(int viewType, const string& ldpSource)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.new_document(viewType, ldpSource);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::open_document(int viewType, const string& filename)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.open_document(viewType, filename);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::open_document(int viewType, LdpReader& reader)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.open_document(viewType, reader);
}

//---------------------------------------------------------------------------------------
void LomseDoorway::init_library(int pixel_format, int ppi, bool reverse_y_axis,
                               ostream& reporter)
{
    m_platform.pixel_format = EPixelFormat(pixel_format);
    m_platform.flip_y = reverse_y_axis;
    m_platform.screen_ppi = float(ppi);

    m_pLibraryScope = new LibraryScope(reporter, this);
}

//---------------------------------------------------------------------------------------
void LomseDoorway::set_notify_callback(void* pThis,
                                       void (*pt2Func)(void* pObj, EventInfo* event))
{
    m_pFunc_notify = pt2Func;
    m_pObj_notify = pThis;
}

//---------------------------------------------------------------------------------------
void LomseDoorway::set_request_callback(void* pThis,
                                        void (*pt2Func)(void* pObj, Request* event))
{
    m_pFunc_request = pt2Func;
    m_pObj_request = pThis;
}

//---------------------------------------------------------------------------------------
void LomseDoorway::null_notify_function(void* pObj, EventInfo* event)
{
    //This is just a mock method to avoid crashes when using the libary without
    //initializing it
    delete event;
}

//---------------------------------------------------------------------------------------
void LomseDoorway::null_request_function(void* pObj, Request* pRequest)
{
    //This is just a mock method to avoid crashes when using the libary without
    //initializing it
}


}   //namespace lomse

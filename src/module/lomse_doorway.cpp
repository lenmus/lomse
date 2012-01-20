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
#include "lomse_config.h"

#include "lomse_injectors.h"
#include "lomse_agg_types.h"
#include "lomse_presenter.h"
#include "lomse_events.h"

#include <sstream>
using namespace std;

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
Presenter* LomseDoorway::new_document(int viewType, const string& ldpSource,
                                      ostream& reporter)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.new_document(viewType, ldpSource, reporter);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::open_document(int viewType, const string& filename,
                                       ostream& reporter)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.open_document(viewType, filename, reporter);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::open_document(int viewType, LdpReader& reader,
                                       ostream& reporter)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.open_document(viewType, reader, reporter);
}

//---------------------------------------------------------------------------------------
void LomseDoorway::init_library(int pixel_format, int ppi, bool reverse_y_axis,
                               ostream& reporter)
{
    m_platform.pixel_format = EPixelFormat(pixel_format);
    m_platform.flip_y = reverse_y_axis;
    m_platform.screen_ppi = float(ppi);

    m_pLibraryScope = LOMSE_NEW LibraryScope(reporter, this);
}

//---------------------------------------------------------------------------------------
void LomseDoorway::set_notify_callback(void* pThis,
                                       void (*pt2Func)(void* pObj, SpEventInfo event))
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
void LomseDoorway::null_notify_function(void* pObj, SpEventInfo event)
{
    //This is just a mock method to avoid crashes when using the libary without
    //initializing it
}

//---------------------------------------------------------------------------------------
void LomseDoorway::null_request_function(void* pObj, Request* pRequest)
{
    //This is just a mock method to avoid crashes when using the libary without
    //initializing it
}

//---------------------------------------------------------------------------------------
string LomseDoorway::get_version_string()
{
    //i.e. "0.7"

    int major = LOMSE_VERSION_MAJOR;
    int minor = LOMSE_VERSION_MINOR;
    stringstream s;
    s << major << "." << minor;
    return s.str();
}

//---------------------------------------------------------------------------------------
int LomseDoorway::get_version_number()
{
    //returns 1000 * major + minor
    //i.e. "1.2" -> 1002
    //i.e. "12.114" -> 12114

    int major = LOMSE_VERSION_MAJOR;
    int minor = LOMSE_VERSION_MINOR;
    return 1000 * major + minor;
}



}   //namespace lomse

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

#include "lomse_doorway.h"

#include "lomse_injectors.h"
#include "lomse_agg_types.h"

namespace lomse
{

//=======================================================================================
//platform independent methods
//=======================================================================================
LomseDoorway::LomseDoorway()
    : m_pLibraryScope(NULL)
{
}

//---------------------------------------------------------------------------------------
LomseDoorway::~LomseDoorway()
{
    delete m_pLibraryScope;
}

//---------------------------------------------------------------------------------------
View* LomseDoorway::create_view(int viewType, Document* pDoc)
{
    return Injector::inject_View(*m_pLibraryScope, viewType, pDoc);
}

//---------------------------------------------------------------------------------------
HorizontalBookView* LomseDoorway::create_horizontal_book_view(Document* pDoc)
{
    return Injector::inject_HorizontalBookView(*m_pLibraryScope, pDoc);
}

//---------------------------------------------------------------------------------------
int LomseDoorway::init_library(EPlatformType type, ostream& reporter)
{
    switch(type)
    {
        case k_platform_win32:
            m_platform.format = pix_format_bgra32;
            m_platform.flip_y = false;
            m_platform.screen_ppi = 96.0;
            break;

        case k_platform_x11:
            m_platform.format = pix_format_bgra32;
            m_platform.flip_y = false;
            m_platform.screen_ppi = 96.0;
            break;

        default:
            return 1;   //error
    }

    m_pLibraryScope = new LibraryScope(reporter, this);
    return 0;   //no error
}


}   //namespace lomse

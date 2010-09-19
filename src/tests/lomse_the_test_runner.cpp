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

#include <iostream>
#include <UnitTest++.h>
#include "lomse_config.h"

#if defined WIN32 || defined _WIN32
    //for detecting and isolating memory leaks with Visual C++
    #ifndef _DEBUG
        #define _DEBUG
    #endif
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

using namespace std;


int main()
{
    cout << "Lomse library tests runner" << endl << endl;
    int nErrors = UnitTest::RunAllTests();

    #if defined WIN32 || defined _WIN32
        //system("pause");
        cin.get();
        _CrtDumpMemoryLeaks();

    #endif

    return nErrors;
}

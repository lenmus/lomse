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

#ifndef __LOMSE_CONFIG_H__
#define __LOMSE_CONFIG_H__

//==================================================================
// Configuration file
//==================================================================

//target operating sytem -------------------------------------------
#define _LML_OS_WIN32
//#define _LML_OS_LINUX

//build method -----------------------------------------------------
#define _LML_VS_NET
//#define _LML_CODE_BLOCKS
//#define _LML_VS_EXPRESS

//path for test scores ---------------------------------------------
#define LOMSE_TEST_SCORES_PATH    "../../test-scores/"   //codeblocks generated makefiles
//#if defined(_LOMSE_LINUX_CODEBLOCKS)
//    #define LOMSE_TEST_SCORES_PATH    "../../../../test-scores/";   //linux CodeBlobks
//#elif defined(_LOMSE_WIN32_VSTUDIO)
//    #define LOMSE_TEST_SCORES_PATH    "../../../test-scores/" //windows MS Visual studio .NET
//#else
//#error("Lomse config.: Unknown OS and IDE");
//#endif


#endif  // __LOMSE_CONFIG_H__


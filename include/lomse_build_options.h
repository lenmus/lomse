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

#ifndef __LOMSE_BUILDOPTIONS_H__
#define __LOMSE_BUILDOPTIONS_H__

//==================================================================
//
//  Visibility of symbols. See
//	    http://gcc.gnu.org/wiki/Visibility
//	    http://www.cygwin.com/cygwin-ug-net/dll.html
//
//==================================================================

#if defined WIN32 || defined _WIN32

	#ifdef LM_DYNAMIC_LIB
		#define LM_EXPORT __declspec(dllexport)

	#elif defined LM_STATIC_LIB
		#define LM_EXPORT

	#else
		#define LM_EXPORT //__declspec(dllimport)

	#endif

#else

	#ifdef LM_DYNAMIC_LIB
		#define LM_EXPORT	__attribute__ ((visibility("default")
	#else
		#define LM_EXPORT
	#endif

#endif

#endif	// __LOMSE_BUILDOPTIONS_H__


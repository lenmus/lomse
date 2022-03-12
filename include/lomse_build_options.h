//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_BUILDOPTIONS_H__
#define __LOMSE_BUILDOPTIONS_H__

#include "lomse_config.h"

//---------------------------------------------------------------------------------------
// macros for declaring DLL-imported/exported functions. See
//      http://gcc.gnu.org/wiki/Visibility
//      http://www.cygwin.com/cygwin-ug-net/dll.html
//
// - If building lomse as a shared library (DLL) define LOMSE_CREATE_DLL
// - If using lomse as a shared library (DLL) define LOMSE_USE_DLL. This is
//   not mandatory but you will get a little performance increase.
// - If building or using lomse as a static library do not define any of them

#if (LOMSE_PLATFORM_WIN32 == 1)
    // for windows with VC++ or gcc

    #if (LOMSE_CREATE_DLL == 1)
        #define LOMSE_EXPORT __declspec(dllexport)

    #else
        #if (LOMSE_USE_DLL == 1)
            #define LOMSE_EXPORT __declspec(dllimport)
        #else
            #define LOMSE_EXPORT
        #endif
    #endif

#else
    // for linux with gcc

    #if (LOMSE_CREATE_DLL == 1)
        #define LOMSE_EXPORT    __attribute__ ((visibility("default")
    #else
        #define LOMSE_EXPORT
    #endif

#endif


//---------------------------------------------------------------------------------------
// stdcall is used for all functions called by Windows under Windows

#if (LOMSE_PLATFORM_WIN32 == 1)
    #if defined(__GNUWIN32__)
        #define LOMSE_STDCALL    __attribute__((stdcall))
    #else
        // MS VC++
        #define LOMSE_STDCALL    _stdcall
    #endif

#else
    //other platforms
    #define LOMSE_STDCALL

#endif

//---------------------------------------------------------------------------------------
// LOMSE_CALLBACK used for the functions which are called back by Windows

#if (LOMSE_PLATFORM_WIN32 == 1)
    #define LOMSE_CALLBACK LOMSE_STDCALL

#else
    //no stdcall under Linux
    #define LOMSE_CALLBACK

#endif

//---------------------------------------------------------------------------------------
// generic calling convention for the extern "C" functions

#if (LOMSE_PLATFORM_WIN32 == 1)
    #define   LOMSE_C_EXTERN    _cdecl

#else
    //not VC++
    #define   LOMSE_C_EXTERN

#endif

//---------------------------------------------------------------------------------------
// for detecting and isolating memory leaks with Visual C++

//#if (LOMSE_COMPILER_MSVC == 1) && (LOMSE_DEBUG == 1)
//    #ifndef _DEBUG
//        #define _DEBUG
//    #endif
//    #ifndef LOMSE_NEW
//        #define _CRTDBG_MAP_ALLOC
//        #define _CRTDBG_MAP_ALLOC_NEW
//        #include <stdlib.h>
//        #include <crtdbg.h>
//        #define LOMSE_NEW new ( (_NORMAL_BLOCK) , (__FILE__) , (__LINE__) )
//    #endif
//#else
    #ifndef LOMSE_NEW
        #define LOMSE_NEW new
    #endif
//#endif


//---------------------------------------------------------------------------------------
// macro to test the gcc version. Use it like this:
//
//#    if LOMSE_CHECK_GCC_VERSION(3, 1)
//        ... we have gcc 3.1 or later ...
//#    else
//        ... no gcc at all or gcc < 3.1 ...
//#    endif

#if defined(__GNUC__) && defined(__GNUC_MINOR__)
    #define LOMSE_CHECK_GCC_VERSION( major, minor ) \
        ( ( __GNUC__ > (major) ) \
            || ( __GNUC__ == (major) && __GNUC_MINOR__ >= (minor) ) )
#else
    #define LOMSE_CHECK_GCC_VERSION( major, minor ) 0
#endif

//---------------------------------------------------------------------------------------
// macro to issue warning when using deprecated functions with gcc > 3 or MSVC > 7

#if LOMSE_CHECK_GCC_VERSION(3, 1)
    #define LOMSE_DEPRECATED(x) x __attribute__ ((deprecated))
#elif defined(__VISUALC__) && (__VISUALC__ >= 1300)
    #define LOMSE_DEPRECATED(x) __declspec(deprecated) x
#else
    #define LOMSE_DEPRECATED(x) x
#endif



#endif  // __LOMSE_BUILDOPTIONS_H__


//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_CONFIG_H__
#define __LOMSE_CONFIG_H__

//==================================================================
// Template configuration file.
// Variables are replaced by CMake settings
//==================================================================

//---------------------------------------------------------------------------------------
// Paths, for fonts and unit tests resources
//
//    LOMSE_FONTS_PATH
//        - For Linux this path is a fallback path in case Bravura.otf font is not 
//          found in systems fonts.
//        - For Windows this path is to look for the Bravura.otf font.
//        - For platforms other than Linux and Windows the absolute path to the fonts
//          directory to use must be specified here.
//      Nevertheless, at run time the application using Lomse can set this path by
//      invoking method LomseDoorway::set_default_fonts_path(const string& fontsPath)
//
//    TESTLIB_SCORES_PATH
//        Absolute path for tests scores used in unit tests.
//
//    TESTLIB_FONTS_PATH
//        Absolute path for fonts used in unit tests.
//
//---------------------------------------------------------------------------------------
#define LOMSE_FONTS_PATH            @LOMSE_FONTS_PATH@
#define TESTLIB_SCORES_PATH         @TESTLIB_SCORES_PATH@
#define TESTLIB_FONTS_PATH          @TESTLIB_FONTS_PATH@


//---------------------------------------------------------------------------------------
// platform and compiler
//---------------------------------------------------------------------------------------
#define LOMSE_PLATFORM_WIN32      @LOMSE_PLATFORM_WIN32@
#define LOMSE_PLATFORM_UNIX       @LOMSE_PLATFORM_UNIX@
#define LOMSE_PLATFORM_APPLE      @LOMSE_PLATFORM_APPLE@
#define LOMSE_COMPILER_MSVC       @LOMSE_COMPILER_MSVC@


//---------------------------------------------------------------------------------------
// what are you doing?
//    - creating the library as shared library   LOMSE_CREATE_DLL == 1
//    - using the library as shared library      LOMSE_USE_DLL == 1
//    - creating the library as static library   LOMSE_CREATE_DLL == 0 
//    - using the library as static library      LOMSE_USE_DLL == 0
//---------------------------------------------------------------------------------------
#define LOMSE_CREATE_DLL    @LOMSE_CREATE_DLL@
#define LOMSE_USE_DLL       @LOMSE_USE_DLL@

//---------------------------------------------------------------------------------------
// build options
//---------------------------------------------------------------------------------------
#define ON 1
#define OFF 0

// Debug build: include debug options
#define LOMSE_DEBUG                 @LOMSE_DEBUG@ 

// Accept without warning/error LDP v1.5 syntax
#define LOMSE_COMPATIBILITY_LDP_1_5     @LOMSE_COMPATIBILITY_LDP_1_5@

// Enable debug logs. It is independent of build mode: debug or release
#define LOMSE_ENABLE_DEBUG_LOGS     @LOMSE_ENABLE_DEBUG_LOGS@

// Enable compressed formats (requires zlib)
#define LOMSE_ENABLE_COMPRESSION    @LOMSE_ENABLE_COMPRESSION@

// Enable png format (requires pnglib and zlib)
#define LOMSE_ENABLE_PNG    @LOMSE_ENABLE_PNG@


#endif  // __LOMSE_CONFIG_H__


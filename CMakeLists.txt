#--------------------------------------------------------------------------------------
# This file is part of the Lomse library.
# Lomse is copyrighted work (c) 2010-2020. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#    * Redistributions of source code must retain the above copyright notice, this 
#      list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above copyright notice, this
#      list of conditions and the following disclaimer in the documentation and/or
#      other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
# SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# For any comment, suggestion or feature request, please contact the manager of
# the project at cecilios@users.sourceforge.net
# 
#---------------------------------------------------------------------------------------


#-------------------------------------------------------------------------------------
# This is a CMake configuration file for building makefiles and installfiles for
# the Lomse library
#-------------------------------------------------------------------------------------
#
# To use it you need CMake which can be downloaded from http://www.cmake.org/
#
# Usage
#   cmake [<options>] <source-tree>
#
# The default target (in Linux) is "Unix Makefiles". If you would like to generate
# a diferent makefile use option -G<desired-target>. For instance:
#   cmake -G "Ninja" [...]
#
#
# Default configuration
# ~~~~~~~~~~~~~~~~~~~~~~~
# - Build type is 'Release'
#
# - The Bravura.otf music font *IS INCLUDED* in the package.
#   See "Bravura music font required to render scores" section.
#
# Please note that default values can be changed directly in the CMake GUI or
# through the command line by prefixing the option's name with '-D':
# i.e.:    cmake -DLOMSE_DEBUG=ON  ....
#
#
#
# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------
#
# LOMSE_BUILD_MONOLITHIC (Default: OFF) [In development. Not yet implemented]
#   To avoid dependencies, instead of linking with the required libraries,
#   these libraries will be combined into lomse library so that the binaries
#   will not have dependencies on any of these libraries. 
#   	cmake -DLOMSE_BUILD_MONOLITHIC=ON [...]
#
# LOMSE_BUILD_STATIC_LIB (Default: OFF for Unix, ON for Windows)
#   Build the static library
#
# LOMSE_BUILD_SHARED_LIB (Default: ON for Unix, OFF for Windows)
#   Build the shared library
#
# LOMSE_ENABLE_UNIT_TESTS (Default: ON)
#   By default the unit tests runner program 'testlib' is also built and the
#   the tests are automatically run after building the library. You can disable
#   this option:
#       cmake -DLOMSE_ENABLE_UNIT_TESTS:BOOL=OFF [...]
#
# LOMSE_BUILD_EXAMPLE (Default: OFF)
#   Build the tutorial_1 program that uses the library, to test it.
#
#
# Debug options (ON / OFF values):
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# LOMSE_DEBUG   (Default value: OFF)      
#	Force to create a debug build, with debug symbols. Default build type
#   is 'Release'
#
# LOMSE_ENABLE_DEBUG_LOGS   (Default value: OFF)
#	Enable debug logs (performance loss). Doesn't require a debug build.
#
#
# Bravura music font required to render scores
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# LOMSE_INSTALL_BRAVURA_FONT (Default value: ON)
#   By default, this font should not be included in Linux packages and the
#   required font package must be set as an install dependency.
#   As it is necessary to install it in Windows, and as in some Linux distros
#   there is no package to install this font, this cmake scripts includes this
#   font in the installer. To prevent this, you can use the option:
#       cmake -DLOMSE_INSTALL_BRAVURA_FONT:BOOL=OFF [...]
#
# LOMSE_DOWNLOAD_BRAVURA_FONT (Default value: ON)
#   When it is requested that Bravura font be included in the package, this
#   cmake script will download the Bravura.otf font in case it is not found in
#   the system. You can use the option:
#       cmake -DLOMSE_DOWNLOAD_BRAVURA_FONT:BOOL=OFF [...]
#   to force not to download it if not found in the system. But if the font is
#   not found, it will not be installed in any case.
#
#
# Installation folders
# ~~~~~~~~~~~~~~~~~~~~~
#
# CMAKE_INSTALL_PREFIX 
#   The default installation prefix is "/usr/local" for Linux and
#   "C:/Program Files (x86)" or "C:/Program Files" for Windows. 
#   You can change the install location by running cmake like this:
#       cmake -DCMAKE_INSTALL_PREFIX=/new/install/prefix ...
#
# LOMSE_PKG_CONFIG_INSTALL  (Default "/usr/lib/pkgconfig")
# 	As part of the Lomse instalation process, a pkg-config file, named
#	liblomse.pc, is installed. The default location is "/usr/lib/pkgconfig".
#	You can change this default location by using the option
#	-DLOMSE_PKG_CONFIG_INSTALL=/new/pkgconfig/path. For instance:
#   	cmake -DLOMSE_PKG_CONFIG_INSTALL=/usr/lib/x86_64-linux-gnu/pkgconfig [...]
#
#
#
#
#
# ---------------------------------------------------------------------------
# Dependencies from external libraries
# ---------------------------------------------------------------------------
#
# lomse has these external dependencies:
#
#   a) for all platforms:
#       freetype        git clone git://git.sv.nongnu.org/freetype/freetype2.git
#       libpng          https://github.com/glennrp/libpng
#       zlib            https://github.com/madler/zlib
#       UnitTest++      https://github.com/unittest-cpp/unittest-cpp
#
#   b) additional, for Linux & macOS platforms (but not for Android):
#       fontconfig      https://gitlab.freedesktop.org/fontconfig/fontconfig
#
# It also uses (included as part of the surce tree):
#   utfcpp
#   agg
#   pugixml
#
#
# Options to reduce dependencies from other libraries (ON / OFF values):
# --------------------------------------------------------------------------
#
# If your application will not deal with compressed files or with scores
# containing png images, it is possible to reduce dependencies from third
# party librraries by removing the code that deals with compressed files
# or with PNG images.
#
# LOMSE_ENABLE_PNG   (Default value: ON)
#	Build with support for PNG images. This requires libpng library.
#	By setting this to OFF Lomse will ignore any png image embedded in the
#	documents. But Lomse will not fail in those cases, just will log error
#	messages. For instance:
#		cmake -G "Unix Makefiles" -DLOMSE_ENABLE_PNG=OFF [...]
#
# LOMSE_ENABLE_COMPRESSION   (Default value: ON)
#	Build with support for compressed files. This requires zlib library.
#	By setting this to OFF Lomse will not be able to deal with compressed files,
#	such as MusicXML files in compressed format, or with png images embedded in
#	the documents. But Lomse will not fail in those cases, just will log error
#	messages. For instance:
#		cmake -G "Unix Makefiles" -DLOMSE_ENABLE_COMPRESSION=OFF [...]
#
#	**Important**: trying to disable compression without also disabling png
#   has no effect because libpng depends on zlib. Therefore, when png is
#   active (LOMSE_ENABLE_PNG=ON) then compression is automatically enabled.
#
#
# Other options (ON / OFF values):
# --------------------------------------------
# LOMSE_COMPATIBILITY_LDP_1_5   (Default value: ON)
#       Enables backwards compatibility for accepting scores in LDP v1.5 syntax
#
#-------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.10 FATAL_ERROR)

#Travis complains when building with clang. What is this for?
cmake_policy(SET CMP0042 NEW)   #MACOSX_RPATH is enabled by default

#Windows 10 camplains 
cmake_policy(SET CMP0054 NEW)   # quoted arguments will not be further dereferenced

#prevent in-source builds
  # make sure the user doesn't play dirty with symlinks
  get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
  get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

  # disallow in-source builds
  if("${srcdir}" STREQUAL "${bindir}")
    message("************** ERROR *************************************")
    message("  Lomse should not be configured & built in the Lomse")
    message("  source directory. You must run cmake in a build directory.")
    message("  For example:")
    message("")
    message("      mkdir build; cd build; cmake ..")
    message("")
    message("  NOTE: Given that you already tried to make an in-source")
    message("  build CMake have already created several files & directories")
    message("  in your source tree. Remove them by doing:")
    message("")
    message("      rm -rf CMakeCache.txt CMakeFiles")
    message("")
    message("**********************************************************")
    message(FATAL_ERROR "Quitting configuration")
  endif()


# project name
project(lomse)

# main directories 
set( LOMSE_ROOT_DIR  ${CMAKE_CURRENT_SOURCE_DIR} )
set( LOMSE_SRC_DIR  ${LOMSE_ROOT_DIR}/src )
set( LOMSE_PKG_DIR  ${LOMSE_ROOT_DIR}/packages )
set( OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
set( EXECUTABLE_OUTPUT_PATH ${OUTDIR})
set( LIBRARY_OUTPUT_PATH ${OUTDIR})
message(STATUS "Using cmake version ${CMAKE_VERSION}")
message(STATUS "** LOMSE_ROOT_DIR: ${LOMSE_ROOT_DIR}")
message(STATUS "** CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "** LOMSE_SRC_DIR: ${LOMSE_SRC_DIR}")
message(STATUS "** EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "** LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
message(STATUS "** OUTDIR: ${OUTDIR}")

# define directories to search for CMake modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LOMSE_ROOT_DIR}/cmake-modules")


# define options
#------------------------------------------------------

# build options
if(WIN32)
	option(LOMSE_BUILD_STATIC_LIB "Build the static library" ON)
	option(LOMSE_BUILD_SHARED_LIB "Build the shared library" OFF)
else()
	option(LOMSE_BUILD_STATIC_LIB "Build static library" OFF)
	option(LOMSE_BUILD_SHARED_LIB "Build shared library" ON)
endif()

option(LOMSE_BUILD_MONOLITHIC
    "Build a monolithic library with no dependencies" 
    OFF)
option(LOMSE_BUILD_TESTS
    "Build the unit tests runner program 'testlib'"
    ON)
option(LOMSE_BUILD_EXAMPLE
    "Build the tutorial_1 program"
    OFF)

# Debug options (ON / OFF values):
option(LOMSE_DEBUG
    "Debug build, with debug symbols"
    OFF)
option(LOMSE_ENABLE_DEBUG_LOGS
    "Enable debug logs. Doesn't require debug build"
    OFF)

# Bravura music font required to render scores
option(LOMSE_DOWNLOAD_BRAVURA_FONT
    "Download Bravura.otf font if not present in source tree"
	ON)
option(LOMSE_INSTALL_BRAVURA_FONT
    "Include Bravura music font in the package"
	ON)

# Options to reduce dependencies from other libraries
option(LOMSE_ENABLE_PNG
    "Enable png format (requires pnglib and zlib)"
    ON)
option(LOMSE_ENABLE_COMPRESSION
    "Enable compressed formats (requires zlib)"
    ON)

# Other options
option(LOMSE_COMPATIBILITY_LDP_1_5
    "Enable compatibility for LDP v1.5"
    ON)

#----- end of options definition -----


#santity checks
if (LOMSE_ENABLE_PNG)
	if (NOT LOMSE_ENABLE_COMPRESSION)
        message(STATUS "**WARNING**: Enabling PNG requires enabling compression. LOMSE_ENABLE_COMPRESSION set to ON" )
    	set(LOMSE_ENABLE_COMPRESSION ON)
	endif()
endif()      

#libraries to build
#if (WIN32)
#    set(LOMSE_BUILD_STATIC_LIB ON)
#    set(LOMSE_BUILD_SHARED_LIB OFF)
#    message(STATUS "In Windows force to build the static library")
#else()
#    set(LOMSE_BUILD_STATIC_LIB OFF)
#    set(LOMSE_BUILD_SHARED_LIB ON)
#    message(STATUS "In Unix force to build the dynamic library")
#endif()

#if (WIN32)
#    if (LOMSE_BUILD_SHARED_LIB AND (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
#        message(FATAL_ERROR "Shared C++ libraries (C++ DLL) are not supported by MSVC")
#    endif()
#endif()

# display options values for building
message(STATUS "Build monolithic library = ${LOMSE_BUILD_MONOLITHIC}")
message(STATUS "Build the static library = ${LOMSE_BUILD_STATIC_LIB}")
message(STATUS "Build the shared library = ${LOMSE_BUILD_SHARED_LIB}")
message(STATUS "Build testlib program = ${LOMSE_BUILD_TESTS}")
message(STATUS "Build tutorial_1 program = ${LOMSE_BUILD_EXAMPLE}")
message(STATUS "Create Debug build = ${LOMSE_DEBUG}")
message(STATUS "Enable debug logs = ${LOMSE_ENABLE_DEBUG_LOGS}")
message(STATUS "Download Bravura font = ${LOMSE_DOWNLOAD_BRAVURA_FONT}")
message(STATUS "Install Bravura font = ${LOMSE_INSTALL_BRAVURA_FONT}")
message(STATUS "Enable png format = ${LOMSE_ENABLE_PNG}")
message(STATUS "Enable compressed formats = ${LOMSE_ENABLE_COMPRESSION}")
message(STATUS "Compatibility for LDP v1.5 = ${LOMSE_COMPATIBILITY_LDP_1_5}")



# set up configuration variables for lomse_config.h
#------------------------------------------------------

# build type (this variables affects lomse_config.h and are used
# in lomse_build_options.h. But there are two problems:
# 1. Both builds (static and shared) can be built in the same cmake command.
#    And in this case there is (currently) a single lomse_config.h file
#    common to both builds.
# 2. When defining LOMSE_USE_DLL=1 there are compilation errors in Windows.
#    So, until further investigation, I have commented out this.
# Another issue to consider is that using makes it impossible to build both
# library versions in the same build.
#if (LOMSE_BUILD_STATIC_LIB)
    set( LOMSE_USE_DLL "0")
    set( LOMSE_CREATE_DLL "0")
#else()
#    set( LOMSE_USE_DLL "1")
#    set( LOMSE_CREATE_DLL "1")
#endif()


# set up lomse library version information
include( ${LOMSE_ROOT_DIR}/build-version.cmake )

add_custom_target (build-version ALL
  COMMAND ${CMAKE_COMMAND} -D LOMSE_ROOT_DIR=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/build-version.cmake
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "setting Lomse version information ...")

# identify platform and compiler
if(WIN32 OR CYGWIN)
    # for Windows operating system or Windows when using the CygWin version of cmake
    set( LOMSE_PLATFORM_WIN32  "1")
    set( LOMSE_PLATFORM_UNIX   "0")
    set( LOMSE_PLATFORM_APPLE  "0")
elseif(APPLE)
    # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
    set( LOMSE_PLATFORM_WIN32  "0")
    set( LOMSE_PLATFORM_UNIX   "0")
    set( LOMSE_PLATFORM_APPLE  "1")
elseif(UNIX AND NOT APPLE AND NOT CYGWIN)
    # for Linux, BSD, Solaris, Minix
    set( LOMSE_PLATFORM_WIN32  "0")
    set( LOMSE_PLATFORM_UNIX   "1")
    set( LOMSE_PLATFORM_APPLE  "0")
endif()

# compiler
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set( LOMSE_COMPILER_MSVC  "1")
else()
    set( LOMSE_COMPILER_MSVC  "0")
endif()


# other variables needed by lomse_config.h

# paths for tests
set( TESTLIB_SCORES_PATH     "\"${LOMSE_ROOT_DIR}/test-scores/\"" )
set( TESTLIB_FONTS_PATH      "\"${LOMSE_ROOT_DIR}/fonts/\"" )

# path to fonts (will be hardcoded in lomse library, so *MUST* be the
# path in which Lomse standard fonts will be installed)
set( LOMSE_FONTS_PATH   "\"${FONTS_PATH}/\"" )



# Build environtment checks and settings
#------------------------------------------------------------------------------

# check that compiler supports namespace sdt
include(TestForSTDNamespace)
if(CMAKE_NO_STD_NAMESPACE)
  message(FATAL_ERROR "The compiler doesn't support namespace std.")
endif()

# determine system arquitecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SYSTEM_ARCHITECTURE "x64")
else()
    set(SYSTEM_ARCHITECTURE "x86")
endif()
message(STATUS "SYSTEM_ARCHITECTURE = ${SYSTEM_ARCHITECTURE}" )


# force to use c++11
if (CMAKE_VERSION VERSION_LESS "3.1")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else ()
    set(CMAKE_CXX_STANDARD 11)				#require c+11 or greater
    set(CMAKE_CXX_STANDARD_REQUIRED ON) 	#prevent fallback to any previous standard
endif ()


#check that the compiler supports c++11 and std::regex
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    # require at least gcc 4.9
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        message(FATAL_ERROR "GCC version must be at least 4.9!")
    endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    # require at least clang 3.4
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.4)
        message(FATAL_ERROR "Clang version must be at least 3.4!")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # require at least msvc 15 (Visual Studio 2008 version 9.0)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15)
        message(FATAL_ERROR "MSVC version must be at least 15!")
    endif()
else()
    message(WARNING "** Warning **: You are using an untested compiler! Lomse has not been tested with ${CMAKE_CXX_COMPILER_ID}.")
endif()
message(STATUS "** Compiler to use: ${CMAKE_CXX_COMPILER_ID} v.${CMAKE_CXX_COMPILER_VERSION}")



# names for libraries and execs.
#-------------------------------------------------------------------------------------
if( WIN32 )
    set( CMAKE_STATIC_LIBRARY_PREFIX "" )
    set( CMAKE_STATIC_LIBRARY_SUFFIX ".lib" )
    set( CMAKE_SHARED_LIBRARY_PREFIX "" )
    set( CMAKE_SHARED_LIBRARY_SUFFIX ".dll" )
    set( CMAKE_EXECUTABLE_SUFFIX ".exe" )
elseif( UNIX )
    set( CMAKE_STATIC_LIBRARY_PREFIX "lib" )
    set( CMAKE_STATIC_LIBRARY_SUFFIX ".a" )
    set( CMAKE_SHARED_LIBRARY_PREFIX "lib" )
    set( CMAKE_SHARED_LIBRARY_SUFFIX ".so" )
    set( CMAKE_EXECUTABLE_SUFFIX "" )
endif()


#----------------------------------------------------------------------------
# Determine installation folders and create the include file "lomse_config.h"
#
# values for configuration macros in "lomse_config.h" could depend on 
# installation folders. Therefore, before configuring it is necessary to set
# installation folders. By default, CMake uses the following prefix:
#   - Linux: "/usr/local"
#   - Windows: "C:\Program Files\lomse\"
#
# You can change the install location by running cmake like this:
#   cmake -DCMAKE_INSTALL_PREFIX=/new/install/prefix
#----------------------------------------------------------------------------

message(STATUS "CMAKE_INSTALL_PREFIX = " ${CMAKE_INSTALL_PREFIX} )

#set install folder 
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT
    OR (${CMAKE_INSTALL_PREFIX} STREQUAL "C:/Program Files/${CMAKE_PROJECT_NAME}")
    OR (${CMAKE_INSTALL_PREFIX} STREQUAL "C:/Program Files (x86)/${CMAKE_PROJECT_NAME}")
  )
    message(STATUS "    CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT = ${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT}")
    #Fix CMAKE_INSTALL_PREFIX for Windows.
    #Sometimes it points to 'Program Files (x86)' for x64 builds
    if(WIN32)
        message(STATUS "    Win32. SYSTEM_ARCHITECTURE = ${SYSTEM_ARCHITECTURE}")
        if(${SYSTEM_ARCHITECTURE} STREQUAL "x64")
            set(CMAKE_INSTALL_PREFIX "C:/Program Files/${CMAKE_PROJECT_NAME}")
        else()
            set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/${CMAKE_PROJECT_NAME}")
        endif()
    endif()
endif()
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
set(LOMSE_LIBDIR        ${CMAKE_INSTALL_PREFIX}/lib)
set(LOMSE_INCLUDEDIR    ${CMAKE_INSTALL_PREFIX}/include/lomse )



#----------------------------------------------------------------------------
# Dependencies
# Check for needed libraries and set all related includes, flags, etc. 
#----------------------------------------------------------------------------

#dependencies for building and for pkg-config file (intitially empty)
set(LOMSE_REQUIRES "")
set(LOMSE_DEBIAN_DEPS "")   #Build deps: Debian "Build-Depends:" string
set(LOMSE_BUILD_DEPS "")    #Build deps: Libraries to link

# include directories to be installed
set( INCLUDES_LOMSE  ${LOMSE_ROOT_DIR}/include )
set( INCLUDES_AGG  ${LOMSE_ROOT_DIR}/src/agg/include )
set( INCLUDES_UTFCPP  ${LOMSE_ROOT_DIR}/packages/utfcpp )
set( INCLUDES_MINIZIP  ${LOMSE_ROOT_DIR}/packages/minizip )
set( INCLUDES_PUGIXML  ${LOMSE_ROOT_DIR}/packages )
include_directories(
    ${INCLUDES_LOMSE}
    ${INCLUDES_AGG}
    ${LOMSE_ROOT_DIR}/src/agg/font_freetype
    ${INCLUDES_UTFCPP}
    ${INCLUDES_PUGIXML}
    ${INCLUDES_MINIZIP}
)


# Check for Bravura music font required to render scores
set (INSTALL_BRAVURA_FONT ${LOMSE_INSTALL_BRAVURA_FONT})
find_path(BRAVURA_FONT_PATH
          NAMES Bravura.otf
          PATHS
              /usr/share/fonts/truetype/          # Red Hat & fonts-lenmus-bravura.deb
              ${LOMSE_ROOT_DIR}/z_fonts           # local builds
              ${LOMSE_ROOT_DIR}/fonts             # local builds
              ${LOMSE_ROOT_DIR}/bravura/src       # builds for PPA at Launchpad.net
         )
if("${BRAVURA_FONT_PATH}" STREQUAL "BRAVURA_FONT_PATH-NOTFOUND")
    if (LOMSE_DOWNLOAD_BRAVURA_FONT)
        message(STATUS "Bravura font not found. Downloading it ...")
        file(DOWNLOAD 
            "https://github.com/steinbergmedia/bravura/raw/master/redist/otf/Bravura.otf"
            "${LOMSE_ROOT_DIR}/fonts/Bravura.otf"
            SHOW_PROGRESS
#            EXPECTED_HASH MD5=863c90e6f99c264a1747856130ebdc3c
            STATUS status
            LOG log
        )

        list(GET status 0 status_code)
        list(GET status 1 status_string)
        
        if(NOT status_code EQUAL 0)
            message(FATAL_ERROR "Error downloading Bravura font. Status_code: ${status_code}
                status_string: ${status_string}
                log: ${log}
                ")
        endif()
        message(STATUS "Bravura font downloaded.")
        set(BRAVURA_FONT_PATH "${LOMSE_ROOT_DIR}/fonts/")
        set(INSTALL_BRAVURA_FONT ON)

    else(LOMSE_DOWNLOAD_BRAVURA_FONT)
        if (WIN32)
            message(FATAL_ERROR "Bravura font not found. Aborting...")
        else()
            message(STATUS "Bravura font not found. It will not be installed.")
        endif()
        set(INSTALL_BRAVURA_FONT OFF)
    endif(LOMSE_DOWNLOAD_BRAVURA_FONT)

else()
    message(STATUS "Bravura font found in ${BRAVURA_FONT_PATH}")
endif()

# Check for UnitTest++. Required for unit test
if (LOMSE_BUILD_TESTS)
	find_package(UnitTest++)
	if(UNITTEST++_FOUND)
		get_filename_component(UNITTEST++_LINK_DIR ${UNITTEST++_LIBRARY} DIRECTORY)
		include_directories(${UNITTEST++_INCLUDE_DIR})
		message("      include= ${UNITTEST++_INCLUDE_DIR}" )
	else()
		message(STATUS "Not found UnitTest++. Test program will not be built" )
		set (LOMSE_BUILD_TESTS OFF)
	endif()
endif(LOMSE_BUILD_TESTS)

if( LOMSE_ENABLE_COMPRESSION )
    # Check for zlib
    # libpng and freetype require zlib. So deal with zlib first.
    find_package(ZLIB REQUIRED)
    include_directories( ${ZLIB_INCLUDE_DIR} )
    message(STATUS "Found ZLib: ${ZLIB_LIBRARIES}" )
    message("      include= ${ZLIB_INCLUDE_DIR}" )
    set(LOMSE_REQUIRES "${LOMSE_REQUIRES}, zlib")
    set(LOMSE_BUILD_DEPS ${LOMSE_BUILD_DEPS} ${ZLIB_LIBRARIES})
    set(LOMSE_DEBIAN_DEPS "${LOMSE_DEBIAN_DEPS}, zlib1g (>= ${ZLIB_VERSION_STRING})")
endif(LOMSE_ENABLE_COMPRESSION)

# Check for FreeType
find_package(Freetype REQUIRED)                 
include_directories( ${FREETYPE_INCLUDE_DIRS} )
message(STATUS "Found Freetype: ${FREETYPE_LIBRARY}" )
message("      include= ${FREETYPE_INCLUDE_DIRS}" )
set(LOMSE_REQUIRES "${LOMSE_REQUIRES}, freetype2")
set(LOMSE_DEBIAN_DEPS "${LOMSE_DEBIAN_DEPS}, libfreetype6")
set(LOMSE_BUILD_DEPS ${LOMSE_BUILD_DEPS} ${FREETYPE_LIBRARY})

# Check for libpng
if( LOMSE_ENABLE_PNG )
    find_package(PNG REQUIRED)  
    include_directories( ${PNG_INCLUDE_DIRS} )
    message(STATUS "Found PNG: ${PNG_LIBRARIES}" )
    message("     include= ${PNG_INCLUDE_DIRS}" )
    set(LOMSE_REQUIRES "${LOMSE_REQUIRES}, libpng")
    set(LOMSE_BUILD_DEPS ${LOMSE_BUILD_DEPS} ${PNG_LIBRARIES})
    set(LOMSE_DEBIAN_DEPS "${LOMSE_DEBIAN_DEPS}, 
		libpng12-0 (>=1.2.42) | libpng16-16" )
endif(LOMSE_ENABLE_PNG)

# Check for fontconfig
if (UNIX)  #Linux, macOS & Android
    find_path(FONTCONFIG_INCLUDE_DIR fontconfig/fontconfig.h)
    find_library(FONTCONFIG_LIBRARIES NAMES fontconfig)
    if (("${FONTCONFIG_INCLUDE_DIR}" STREQUAL "FONTCONFIG_INCLUDE_DIR-NOTFOUND")
        OR ("${FONTCONFIG_LIBRARIES}" STREQUAL "FONTCONFIG_LIBRARIES-NOTFOUND")
       )
        set(FONTCONFIG_FOUND FALSE)
    else()
        set(FONTCONFIG_FOUND TRUE)
    endif()

    if(FONTCONFIG_FOUND )
        include_directories( ${FONTCONFIG_INCLUDE_DIR}/fontconfig )
        set(LOMSE_REQUIRES "${LOMSE_REQUIRES}, fontconfig")
        set(LOMSE_BUILD_DEPS ${LOMSE_BUILD_DEPS} ${FONTCONFIG_LIBRARIES})
        set(LOMSE_DEBIAN_DEPS "${LOMSE_DEBIAN_DEPS}, fontconfig")
        message(STATUS "Found FontConfig: libraries= ${FONTCONFIG_LIBRARIES}" )
        message("        include= ${FONTCONFIG_INCLUDE_DIR}/fontconfig" )
    else()
        message(FATAL_ERROR "fontconfig package not found.")
    endif()
endif()

# strip leading commas from LOMSE_REQUIRES and LOMSE_DEBIAN_DEPS
string( REGEX REPLACE "^, " "" LOMSE_REQUIRES ${LOMSE_REQUIRES})
string( REGEX REPLACE "^, " "" LOMSE_DEBIAN_DEPS ${LOMSE_DEBIAN_DEPS})

if(UNIX)
    # macros for "Print all warnings", GCC & __UNIX__
    add_definitions( -Wall -DGCC -D__UNIX__ )
endif(UNIX)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# source files to compile
include( ${LOMSE_ROOT_DIR}/add-sources.cmake )


# Adds folders for Visual Studio and other IDEs
set (LOMSE_GROUP_FOLDER "lomse/" )
include( ${LOMSE_ROOT_DIR}/add-group-folders.cmake )

message(STATUS "LOMSE_BUILD_DEPS= ${LOMSE_BUILD_DEPS}")

#remove dependencies for monolithic builds
if (LOMSE_BUILD_MONOLITHIC)
    set(LOMSE_DEBIAN_DEPS "")
endif()



#define a header file to pass CMake settings to source code
configure_file(
    "${LOMSE_ROOT_DIR}/lomse_config.h.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/lomse_config.h"
)



#======= End of dependencies checking ========================================

    
###############################################################################
#
# Target: Lomse static library
#
###############################################################################
if(LOMSE_BUILD_STATIC_LIB)

    set(LOMSE_STATIC lomse)
    add_library(${LOMSE_STATIC} STATIC ${ALL_LOMSE_SOURCES})
    add_dependencies(${LOMSE_STATIC} build-version)

    #dependencies
    if (LOMSE_BUILD_MONOLITHIC)
        target_link_libraries(${LOMSE_STATIC} PUBLIC ${LOMSE_BUILD_DEPS})
    else()
        target_link_libraries(${LOMSE_STATIC} ${LOMSE_BUILD_DEPS})
    endif()

    #properties
    #if(UNIX)
        set_target_properties(${LOMSE_STATIC} PROPERTIES
                VERSION ${LOMSE_PACKAGE_VERSION}
                SOVERSION ${LOMSE_PACKAGE_VERSION}
                OUTPUT_NAME "lomse"
                RUNTIME_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH}
        )
    #endif()

    # Force not to link with standard MSVC libraries: /NODEFAULTLIB
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set_target_properties(${LOMSE_STATIC} PROPERTIES  LINK_FLAGS "/NODEFAULTLIB:LIBCMT")
    endif()

    get_target_property(VAR ${LOMSE_STATIC} LINK_FLAGS)
    message("library target properties=" ${VAR})

endif(LOMSE_BUILD_STATIC_LIB)


###############################################################################
#
# Target: dynamic/shared library 
#
###############################################################################
if(LOMSE_BUILD_SHARED_LIB)

    set(LOMSE_SHARED  lomse-shared)
    add_library( ${LOMSE_SHARED} SHARED ${ALL_LOMSE_SOURCES} )
    add_dependencies(${LOMSE_SHARED} build-version)

    #dependencies
    if (LOMSE_BUILD_MONOLITHIC)
        target_link_libraries(${LOMSE_SHARED} PUBLIC ${LOMSE_BUILD_DEPS})
    else()
        target_link_libraries(${LOMSE_SHARED} ${LOMSE_BUILD_DEPS})
    endif()

    #properties
    set_target_properties(${LOMSE_SHARED} 
          PROPERTIES POSITION_INDEPENDENT_CODE 1    #shared libraries need PIC
                     VERSION ${LOMSE_PACKAGE_VERSION}
                     SOVERSION ${LOMSE_PACKAGE_VERSION}
                     OUTPUT_NAME "lomse"
                     RUNTIME_OUTPUT_DIRECTORY  ${LIBRARY_OUTPUT_PATH}
    )

endif(LOMSE_BUILD_SHARED_LIB)


###############################################################################
#
# Target: testlib. Program for running the library unit tests
#
###############################################################################
if(LOMSE_BUILD_TESTS)

    set (TESTLIB  testlib)

    file(GLOB TESTLIB_SRC "${LOMSE_SRC_DIR}/tests/lomse_*.cpp" )
    add_executable(${TESTLIB} ${TESTLIB_SRC})
    
    # libraries to link
    if (LOMSE_BUILD_SHARED_LIB)
        target_link_libraries (${TESTLIB} ${LOMSE_SHARED}
                ${UNITTEST++_LIBRARY} ${LOMSE_BUILD_DEPS} 
        )
        add_dependencies(${TESTLIB} ${LOMSE_SHARED})
    else()
        # Check for pthreads
        find_package (Threads)

        target_link_libraries (${TESTLIB} ${LOMSE_STATIC}
                ${UNITTEST++_LIBRARY} ${LOMSE_BUILD_DEPS} ${CMAKE_THREAD_LIBS_INIT}
        ) 
        add_dependencies(${TESTLIB} ${LOMSE_STATIC})
    endif()
    
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set_target_properties(${TESTLIB} PROPERTIES  LINK_FLAGS "/NODEFAULTLIB:LIBCMT")
    endif()

    get_target_property(VAR ${TESTLIB} LINK_FLAGS)
    message("${TESTLIB} target properties=" ${VAR})

    # once generated, run tests
    if(WIN32)
        set(TESTLIB_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/${TESTLIB}.exe )
    else()
        set(TESTLIB_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/${TESTLIB} )
    endif()

    add_custom_command(
        TARGET ${TESTLIB} POST_BUILD
        COMMAND ${TESTLIB_EXECUTABLE}
        WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    )

endif(LOMSE_BUILD_TESTS)


###############################################################################
#
# Target: Tutorial_1
#
###############################################################################

if (LOMSE_BUILD_EXAMPLE)

    set (EXAMPLE1 tutorial_1)

    if(WIN32)
        set( EXAMPLE1_SRC  ${LOMSE_ROOT_DIR}/examples/tutorials/tutorial-1-win )

        include_directories(
            ${LOMSE_SRC_DIR}/agg/include/platform/win32/agg_win32_bmp.h
        )

        add_executable( ${EXAMPLE1} WIN32
            ${EXAMPLE1_SRC}/tutorial-1-win.cpp
            ${LOMSE_SRC_DIR}/agg/src/platform/win32/agg_win32_bmp.cpp 
        )
        
    elseif(UNIX)
        set( EXAMPLE1_SRC  ${LOMSE_ROOT_DIR}/examples/tutorials/tutorial-1-x11 )

        find_package(X11)
        if(X11_FOUND)  
            include_directories(${X11_INCLUDE_DIRS})
            link_libraries(${X11_LIBRARIES})
        endif(X11_FOUND)

        add_executable( ${EXAMPLE1} ""
            ${EXAMPLE1_SRC}/tutorial-1-x11.cpp
        )
    endif()

    # libraries to link
    if (LOMSE_BUILD_SHARED_LIB)
        target_link_libraries ( ${EXAMPLE1} ${LOMSE_SHARED} 
                ${UNITTEST++_LIBRARY} ${LOMSE_BUILD_DEPS}  #${CMAKE_THREAD_LIBS_INIT}
        )
        add_dependencies(${EXAMPLE1} ${LOMSE_SHARED})
    else()      
        # Check for pthreads
        find_package (Threads)
        target_link_libraries (${EXAMPLE1} ${LOMSE_STATIC}
                ${UNITTEST++_LIBRARY} ${LOMSE_BUILD_DEPS} ${CMAKE_THREAD_LIBS_INIT}
        )
        add_dependencies(${EXAMPLE1} ${LOMSE_STATIC})
    endif()

endif(LOMSE_BUILD_EXAMPLE)


###############################################################################
# library installation
###############################################################################


if( UNIX )
    # copy generated libraries
    install( TARGETS ${LOMSE} DESTINATION ${LOMSE_LIBDIR} )
    install( TARGETS ${LOMSE_SHARED} LIBRARY DESTINATION ${LOMSE_LIBDIR} )

    #create symlink to library
	install( CODE
        "EXECUTE_PROCESS(COMMAND ln -fs ${CMAKE_INSTALL_PREFIX}/lib/${LOMSE_SHARED} 
                                        ${CMAKE_INSTALL_PREFIX}/lib/liblomse.so )" 
    )

elseif( WIN32 )
    #For Windows force to install Bravura font
    set(INSTALL_BRAVURA_FONT 1)

    # copy generated libraries
    install( TARGETS ${LOMSE} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" )
    install( TARGETS ${LOMSE_SHARED} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" )

endif()


# copy bravura font
if(INSTALL_BRAVURA_FONT)
        message(STATUS "BRAVURA_FONT_PATH=${BRAVURA_FONT_PATH}")
        file(COPY ${BRAVURA_FONT_PATH}/Bravura.otf
             DESTINATION ${LOMSE_ROOT_DIR}/fonts/)
endif()

# copy lomse includes
install(DIRECTORY ${INCLUDES_LOMSE}/
        DESTINATION "${LOMSE_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h"
                       PATTERN "platform" EXCLUDE )

# copy agg includes
install(DIRECTORY "${INCLUDES_AGG}/"
        DESTINATION "${LOMSE_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h" )

# copy minizip includes
if( LOMSE_ENABLE_COMPRESSION )
    install(DIRECTORY "${INCLUDES_MINIZIP}/"
            DESTINATION "${LOMSE_INCLUDEDIR}"
            FILES_MATCHING PATTERN "*.h" )
endif()

# copy utfcpp includes
install(DIRECTORY "${INCLUDES_UTFCPP}/"
        DESTINATION "${LOMSE_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h" )
install(DIRECTORY "${INCLUDES_UTFCPP}/utf8"
        DESTINATION "${LOMSE_INCLUDEDIR}/utf8"
        FILES_MATCHING PATTERN "*.h" )

# copy also lomse_config.h
install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/lomse_config.h 
        DESTINATION "${LOMSE_INCLUDEDIR}" )


#----------------------------------------------------------------------------
# create and install the pkg-config file: liblomse.pc
#
# By default, pkg-config file will be installed in /usr/lib/pkgconfig
# You can change the install location by running cmake like this:
#
#   cmake -DLOMSE_PKG_CONFIG_INSTALL=/new/install/path
#

if(NOT DEFINED LOMSE_PKG_CONFIG_INSTALL)
    set( LOMSE_PKG_CONFIG_INSTALL "/usr/lib/pkgconfig" )
endif()

set(prefix ${CMAKE_INSTALL_PREFIX})

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/liblomse.pc.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/liblomse.pc"
    @ONLY
)
install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/liblomse.pc
        DESTINATION ${LOMSE_PKG_CONFIG_INSTALL} )

message(STATUS "pkg-config install path: ${LOMSE_PKG_CONFIG_INSTALL}" )


###############################################################################
#
# CPack section: installers generation
#
#   Currently it just builds a Debian binary package.
# 
###############################################################################

set(CPACK_PACKAGE_VENDOR "LenMus Project")
set(CPACK_PACKAGE_VERSION ${LOMSE_PACKAGE_VERSION} )
set(CPACK_PACKAGE_NAME "liblomse")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Free open source library to render music scores")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${LOMSE_ROOT_DIR}/README.md")
set(CPACK_PACKAGE_CONTACT "s.cecilio@gmail.com")

set(CPACK_RESOURCE_FILE_LICENSE "${LOMSE_ROOT_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${LOMSE_ROOT_DIR}/README.md")

if(UNIX)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Cecilio Salmeron") #required
    set(CPACK_DEBIAN_PACKAGE_NAME ${CPACK_PACKAGE_NAME} )
    set(CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION} )
    set(CPACK_DEBIAN_PACKAGE_SECTION "Libraries")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://www.lenmus.org/en/lomse/intro")

    IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^x86_64$|^amd64$")  
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    ELSE()
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
    ENDIF() 

    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\r\rLomse is a free open source project to provide software developers with\r a C++ library for rendering, editing and playing back music scores.\r With it, you can add capabilities to any program for displaying and\r editing music scores. It is witten in C++, it is platform independent.\r\rLomse website: http://www.lenmus.org/en/lomse/intro")

    set(CPACK_PACKAGE_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS ${LOMSE_DEBIAN_DEPS})
    message(STATUS "Package dependencies: ${CPACK_DEBIAN_PACKAGE_DEPENDS}" )

endif()

include(CPack)


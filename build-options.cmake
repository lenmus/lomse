#-------------------------------------------------------------------------------------
# This is part of CMake configuration file for building makefiles and installfiles
# for the Lomse library
#-------------------------------------------------------------------------------------
# This module is for defining build options and creating the
# include file "lomse_config.h"
#
# Various options that the user can select (var, msg, default value).
# Values can be changed directly in the CMake GUI or through
# the command line by prefixing a variable's name with '-D':
# i.e.:    cmake -DLOMSE_DEBUG=ON
#
#-------------------------------------------------------------------------------------

# build options
OPTION(LOMSE_ENABLE_DEBUG_LOGS
    "Enable debug logs. Doesn't require debug build"
    OFF)
OPTION(LOMSE_DEBUG
    "Debug build, with debug symbols"
    OFF)
OPTION(LOMSE_COMPATIBILITY_LDP_1_5
    "Enable compatibility for LDP v1.5"
    ON)
option(LOMSE_BUILD_STATIC_LIB 
    "Build the static library"
    ON)
option(LOMSE_BUILD_SHARED_LIB
    "Build the shared library" 
    OFF)

#Build the test units runner program 'testlib'
option(LOMSE_BUILD_TESTS "Build testlib program" ON)

#run unit tests after building the library
option(LOMSE_RUN_TESTS "Run tests after building"
    ON)

#Build the example-1 program that uses the library
option(LOMSE_BUILD_EXAMPLE "Build the example-1 program" OFF)

#optional dependencies
option(LOMSE_ENABLE_COMPRESSION "Enable compressed formats (requires zlib)" ON)
option(LOMSE_ENABLE_PNG "Enable png format (requires pnglib and zlib)" ON)

#santity checks
if (LOMSE_ENABLE_PNG)
	if (NOT LOMSE_ENABLE_COMPRESSION)
        message(STATUS "**WARNING**: Enabling PNG requires enabling compression. LOMSE_ENABLE_COMPRESSION set to ON" )
    	set(LOMSE_ENABLE_COMPRESSION ON)
	endif()
endif()      

#libraries to build
if (WIN32)
    set(LOMSE_BUILD_STATIC_LIB ON)
    set(LOMSE_BUILD_SHARED_LIB OFF)
else()
    set(LOMSE_BUILD_STATIC_LIB OFF)
    set(LOMSE_BUILD_SHARED_LIB ON)
endif()

if (WIN32)
    if (LOMSE_BUILD_SHARED_LIB AND MSVC)
        message(FATAL_ERROR "Shared C++ libraries (C++ DLL) are not supported by MSVC")
    endif()
endif()


message(STATUS "Build the static library = ${LOMSE_BUILD_STATIC_LIB}")
message(STATUS "Build the shared library = ${LOMSE_BUILD_SHARED_LIB}")
message(STATUS "Build testlib program = ${LOMSE_BUILD_TESTS}")
message(STATUS "Run tests after building = ${LOMSE_RUN_TESTS}")
message(STATUS "Create Debug build = ${LOMSE_DEBUG}")
message(STATUS "Enable debug logs = ${LOMSE_ENABLE_DEBUG_LOGS}")
message(STATUS "Compatibility for LDP v1.5 = ${LOMSE_COMPATIBILITY_LDP_1_5}")
message(STATUS "Enable compressed formats = ${LOMSE_ENABLE_COMPRESSION}")
message(STATUS "Enable png format = ${LOMSE_ENABLE_PNG}")



# set up configuration variables for lomse_config.h
#------------------------------------------------------

# build type (this variables affects lomse_config.h and are used
# in lomse_build_options.h. But there is are two problems:
# 1. Both builds (static and shared) can be built in the same cmake command.
#    And in this case there is (currently) a single lomse_config.h file
#    common to both build.
# 2. When defining USED_DLL=1 there are compilation errors.
# So, until further investigation, I have commented out this.
#if (LOMSE_BUILD_STATIC_LIB)
    set( LOMSE_USE_DLL "0")
    set( LOMSE_CREATE_DLL "0")
#else()
#    set( LOMSE_USE_DLL "1")
#    set( LOMSE_CREATE_DLL "1")
#endif()

include( ${LOMSE_ROOT_DIR}/build-version.cmake )

add_custom_target (build-version ALL
  COMMAND ${CMAKE_COMMAND} -D LOMSE_ROOT_DIR=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/build-version.cmake
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "setting Lomse version information ...")

# identify platform
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
if(MSVC)
    set( LOMSE_COMPILER_MSVC  "1")
else()
    set( LOMSE_COMPILER_MSVC  "0")
endif()


# paths for tests
set( TESTLIB_SCORES_PATH     "\"${LOMSE_ROOT_DIR}/test-scores/\"" )
set( TESTLIB_FONTS_PATH      "\"${LOMSE_ROOT_DIR}/fonts/\"" )

# path to fonts (will be hardcoded in lomse library, so *MUST* be the
# path in which Lomse standard fonts will be installed)
set( LOMSE_FONTS_PATH   "\"${FONTS_PATH}/\"" )



# names for libraries and execs.
#-------------------------------------------------------------------------------------
if( WIN32 )
    set( CMAKE_STATIC_LIBRARY_PREFIX "" )
    set( CMAKE_STATIC_LIBRARY_SUFFIX ".lib" )
    set( CMAKE_SHARED_LIBRARY_PREFIX "" )
    set( CMAKE_SHARED_LIBRARY_SUFFIX ".dll" )
    set( CMAKE_EXECUTABLE_SUFFIX ".exe" )
    set( LOMSE_LIBNAME lomse.lib )
elseif( UNIX )
    set( CMAKE_STATIC_LIBRARY_PREFIX "lib" )
    set( CMAKE_STATIC_LIBRARY_SUFFIX ".a" )
    set( CMAKE_SHARED_LIBRARY_PREFIX "lib" )
    set( CMAKE_SHARED_LIBRARY_SUFFIX ".so" )
    set( CMAKE_EXECUTABLE_SUFFIX "" )
    set( LOMSE_LIBNAME liblomse.so )
endif()


#define a header file to pass CMake settings to source code
configure_file(
    "${LOMSE_ROOT_DIR}/lomse_config.h.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/lomse_config.h"
)



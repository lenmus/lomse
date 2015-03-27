#-------------------------------------------------------------------------------------
# This is part of CMake configuration file for building makefiles and installfiles
# for the Lomse library
#-------------------------------------------------------------------------------------
# This moule is for defining build options and creating the
# include file "lomse_config.h"
#
# Various options that the user can select (var, msg, default value).
# Values can be changed directly in the CMake GUI or through
# the command line by prefixing a variable's name with '-D':
# i.e.:    cmake -DLOMSE_DEBUG=ON
#
#-------------------------------------------------------------------------------------

# command line build options
if(DEFINED LOMSE_ENABLE_DEBUG_LOGS)
  option( LOMSE_ENABLE_DEBUG_LOGS  "Enable debug logs. Doesn't require debug build" ${LOMSE_ENABLE_DEBUG_LOGS})
else()
  option( LOMSE_ENABLE_DEBUG_LOGS  "Enable debug logs. Doesn't require debug build" OFF)
endif()

if(DEFINED LOMSE_DEBUG)
    option( LOMSE_DEBUG  "Debug build, with debug symbols" ${LOMSE_DEBUG})
else()
    option( LOMSE_DEBUG  "Debug build, with debug symbols" OFF)
endif()

if(DEFINED LOMSE_COMPATIBILITY_LDP_1_5)
    option( LOMSE_COMPATIBILITY_LDP_1_5  "Enable compatibility for LDP v1.5" ${LOMSE_COMPATIBILITY_LDP_1_5})
else()
    option( LOMSE_COMPATIBILITY_LDP_1_5  "Enable compatibility for LDP v1.5" ON)
endif()

#options not yet implemented
#if(DEFINED LOMSE_BUILD_STATIC_LIB)
#    option( LOMSE_BUILD_STATIC_LIB  "Build Lomse as static library" ${LOMSE_BUILD_STATIC_LIB})
#else()
    option( LOMSE_BUILD_STATIC_LIB  "Build Lomse as static library" ON)
#endif()

option( LOMSE_BUILD_EXAMPLE  "Build test example" OFF)
option( LOMSE_BUILD_TESTS  "Build and run unit tests" OFF)


# build type
if (LOMSE_BUILD_STATIC_LIB)
    set( LOMSE_USE_DLL "0")
    set( LOMSE_CREATE_DLL "0")
else()
    set( LOMSE_USE_DLL "1")
    set( LOMSE_CREATE_DLL "1")
endif()

message("Create Debug build = ${LOMSE_DEBUG}")
message("Enable debug logs = ${LOMSE_ENABLE_DEBUG_LOGS}")
message("Compatibility for LDP v1.5 = ${LOMSE_COMPATIBILITY_LDP_1_5}")

if(0)
option(LOMSE_BUILD_STATIC_LIB "Build Lomse as static library" ON)

if(WIN32)
    option(LOMSE_USE_PREBUILT_FREETYPE "Use supplied prebuilt lib for freetype" OFF)
    option(LOMSE_USE_PREBUILT_BOOST "Use supplied prebuilt lib for boost" OFF)
    option(LOMSE_USE_PREBUILT_OTHER "Use supplied prebuilt libs for zlib and libpng" OFF)
endif()

endif()




# set up configuration variables for lomse_config.h
#------------------------------------------------------

# version. Extract values from lomse_version.h header file
file(STRINGS ${LOMSE_ROOT_DIR}/include/lomse_version.h LOMSE_VERSION_LIST)
list (GET LOMSE_VERSION_LIST 5 MAJOR_LINE)
list (GET LOMSE_VERSION_LIST 6 MINOR_LINE)
list (GET LOMSE_VERSION_LIST 7 TYPE_LINE)
list (GET LOMSE_VERSION_LIST 8 PATCH_LINE)
string(REGEX REPLACE "\#define LOMSE_VERSION_MAJOR    " "" LOMSE_VERSION_MAJOR "${MAJOR_LINE}")
string(REGEX REPLACE "\#define LOMSE_VERSION_MINOR    " "" LOMSE_VERSION_MINOR "${MINOR_LINE}")
string(REGEX REPLACE "\#define LOMSE_VERSION_TYPE     " "" LOMSE_VERSION_TYPE "${TYPE_LINE}")
string(REGEX REPLACE "\#define LOMSE_VERSION_PATCH    " "" LOMSE_VERSION_PATCH "${PATCH_LINE}")

# revision. Get SVN revision number and use it in Lomse version string
if (0)
    include(FindSubversion)
    if(Subversion_FOUND)
        Subversion_WC_INFO(${LOMSE_ROOT_DIR} LOMSE)
        message("Current Lomse revision is ${LOMSE_WC_REVISION}")
        set( LOMSE_REVISION ${LOMSE_WC_REVISION})
    else()
        message("Subversion not found. Revision set to zero.")
        set( LOMSE_REVISION "0")
    endif()
endif(0)
set( LOMSE_REVISION "0")

message ("major = '${LOMSE_VERSION_MAJOR}'") 
message ("minor = '${LOMSE_VERSION_MINOR}'") 
message ("type = '${LOMSE_VERSION_TYPE}'") 
message ("patch = '${LOMSE_VERSION_PATCH}'") 
message ("revision = '${LOMSE_REVISION}'") 


#build version string for installer name
set(LOMSE_VERSION_STRING "${LOMSE_VERSION_MAJOR}.${LOMSE_VERSION_MINOR}" )
if (NOT("${LOMSE_VERSION_TYPE}" STREQUAL ""))
    set(LTYPE "")
    if ("${LOMSE_VERSION_TYPE}" STREQUAL "'a'")
        set(LTYPE "a")
    elseif ("${LOMSE_VERSION_TYPE}" STREQUAL "'b'")
        set(LTYPE "b")
    endif()
    set(LOMSE_VERSION_STRING "${LOMSE_VERSION_STRING}.${LTYPE}${LOMSE_VERSION_PATCH}" )
else()
    if (NOT("${LOMSE_VERSION_PATCH}" STREQUAL "0"))
        set(LOMSE_VERSION_STRING "${LOMSE_VERSION_STRING}.${LOMSE_VERSION_PATCH}" )
    endif()
endif()
set(LOMSE_VERSION "${LOMSE_VERSION_STRING}" )
message ("version = '${LOMSE_VERSION}'") 

#set(LOMSE_VERSION_STRING "${LOMSE_VERSION_STRING}.${LOMSE_REVISION}" )
message ("version string = '${LOMSE_VERSION_STRING}'") 


# platform
if(WIN32)
    set( LOMSE_PLATFORM_WIN32  "1")
    set( LOMSE_PLATFORM_UNIX   "0")
elseif(UNIX)
    set( LOMSE_PLATFORM_WIN32  "0")
    set( LOMSE_PLATFORM_UNIX   "1")
endif()

# compiler
if(MSVC)
    set( LOMSE_COMPILER_MSVC  "1")
    set( LOMSE_COMPILER_GCC   "0")
elseif(CMAKE_COMPILER_IS_GNUCC)
    set( LOMSE_COMPILER_MSVC  "0")
    set( LOMSE_COMPILER_GCC   "1")
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



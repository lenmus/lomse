#-------------------------------------------------------------------------------------
# This is part of CMake configuration file for building makefiles and installfiles
# for the Lomse library
#-------------------------------------------------------------------------------------
# This module creates the include file "lomse_version.h"
#
#-------------------------------------------------------------------------------------

set( LOMSE_VERSION_MAJOR 0 )
set( LOMSE_VERSION_MINOR 17 )
set( LOMSE_VERSION_PATCH 20 )

# build version string for installer name
set( LOMSE_PACKAGE_VERSION "${LOMSE_VERSION_MAJOR}.${LOMSE_VERSION_MINOR}.${LOMSE_VERSION_PATCH}" )
set( LOMSE_VERSION "${LOMSE_PACKAGE_VERSION}" )
set( LOMSE_VERSION_LONG "${LOMSE_VERSION}-nogit" )

message (STATUS "  Lomse version        = '${LOMSE_VERSION}'")
message (STATUS "  Lomse version string = '${LOMSE_PACKAGE_VERSION}'")

if (EXISTS ${LOMSE_ROOT_DIR}/.git)
  find_package (Git)
  if (NOT GIT_FOUND)
    message(SEND_ERROR "Git package not found." )
  else()
    # get sha1 and dirty status directly from git
    execute_process(COMMAND "${GIT_EXECUTABLE}" log -1 --format=%h
      WORKING_DIRECTORY "${LOMSE_ROOT_DIR}"
      OUTPUT_VARIABLE LOMSE_VERSION_BUILD
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "${GIT_EXECUTABLE}" describe --tags --long --dirty
      WORKING_DIRECTORY "${LOMSE_ROOT_DIR}"
      OUTPUT_VARIABLE LOMSE_VERSION_GIT
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    message (STATUS "  version git    = '${LOMSE_VERSION_GIT}'")
    if ( ${LOMSE_VERSION_GIT} MATCHES "-dirty$" )
      set (LOMSE_VERSION_BUILD "${LOMSE_VERSION_BUILD}-dirty")
    endif()
    if (NOT ".${LOMSE_VERSION_BUILD}" STREQUAL "." )
      set(LOMSE_VERSION_LONG "${LOMSE_PACKAGE_VERSION}+${LOMSE_VERSION_BUILD}")
    else()
      set(LOMSE_VERSION_LONG "${LOMSE_PACKAGE_VERSION}")
    endif()
  endif()
endif()
message (STATUS "  Lomse version long   = '${LOMSE_VERSION_LONG}'" )

# define a header file to pass version information to source code
configure_file(
    "${LOMSE_ROOT_DIR}/lomse_version.h.cmake"
    "${CMAKE_BINARY_DIR}/lomse_version.h"
)

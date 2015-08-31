#--------------------------------------------------------------------------------------
# This file is part of the Lomse library.
# Copyright (c) 2010-2015 Cecilio Salmeron. All rights reserved.
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
#
# To use it you need CMake which can be downloaded from http://www.cmake.org/
#
# Usage
#   cmake [<options>] <source-tree>
#   make
#   make package
#   sudo dpkg -i <created-liblomse.package>.deb
#
# example:
#   cmake ../lomse
#
# The default target (in Linux) is "Unix Makefiles". If you would like to generate
# a diferent makefile use option -G<desired-target>. For instance:
#
#   cmake -G "CodeBlocks - Unix Makefiles"  ...
#
# Installation options
# ---------------------
#
# The default installation prefix is "/usr/local". Therefore, libraries (binaries)
# will be installed at "/usr/local/lib" and include files at
# "/usr/local/include/lomse". You can change the default location 
# by specifying option CMAKE_INSTALL_PREFIX. For instance:
#
#   cmake -DCMAKE_INSTALL_PREFIX=/usr  ...
#
# Also, a pkg-config file, liblomse.pc, will be installed. The default
# location is "/usr/lib/pkgconfig". You can change this default location
# by using the option -DLOMSE_PKG_CONFIG_INSTALL=/new/pkgconfig/path.
# For instance:
#
#   cmake -DLOMSE_PKG_CONFIG_INSTALL=/usr/lib/x86_64-linux-gnu/pkgconfig  ...
#
#
# Additional build options (ON / OFF values):
# --------------------------------------------
# Values can be changed directly in the CMake GUI or through
# the command line by prefixing a variable's name with '-D':
# i.e.:    cmake -DLOMSE_DEBUG=ON  ...
#
# LOMSE_ENABLE_DEBUG_LOGS   (Default value: OFF)
#       Enable debug logs (performance loss). Doesn't require a debug build.
#
# LOMSE_DEBUG   (Default value: OFF)      
#       Force to create a debug build, with debug symbols.
#
# LOMSE_COMPATIBILITY_LDP_1_5   (Default value: ON)
#       Enables backwards compatibility for scores in LDP v1.5
#
#-------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8)

# project name
project(lomse)

# main directories 
set( LOMSE_ROOT_DIR  ${CMAKE_CURRENT_SOURCE_DIR} )
set( LOMSE_SRC_DIR  ${LOMSE_ROOT_DIR}/src )
set( LOMSE_PKG_DIR  ${LOMSE_ROOT_DIR}/packages )
set( OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/bin )
set( EXECUTABLE_OUTPUT_PATH ${OUTDIR})
set( LIBRARY_OUTPUT_PATH ${OUTDIR})
message("LOMSE_ROOT_DIR: ${LOMSE_ROOT_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("LOMSE_SRC_DIR: ${LOMSE_SRC_DIR}")
message("EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
message("LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
message("OUTDIR: ${OUTDIR}")

# check that compiler supports namespace sdt
include(TestForSTDNamespace)
if(CMAKE_NO_STD_NAMESPACE)
  message(FATAL_ERROR "The compiler doesn't support namespace std.")
endif()


# directories to search for CMake modules
set( CMAKE_MODULE_PATH "${LOMSE_ROOT_DIR}/cmake-modules" )


#define path were fonts will be installed.
set( FONTS_PATH   "${CMAKE_INSTALL_PREFIX}/share/lomse/fonts" )

# define build options and create the include file "lomse_config.h"
include( ${LOMSE_ROOT_DIR}/build-options.cmake )



if (0)
# Get SVN revision number and use it in Lomse version string
# http://stackoverflow.com/questions/3780667/use-cmake-to-get-build-time-svn-revision
#-------------------------------------------------------------------------

# a custom target that is always built
add_custom_target(svnheader ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/svnheader.h)

# creates svnheader.h using cmake script
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/svnheader.h
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
                         -P ${CMAKE_CURRENT_SOURCE_DIR}/getsvn.cmake)

# svnversion.h is a generated file
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/svnversion.h
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE)

# explicitly say that the executable depends on the svnheader
add_dependencies(test svnheader)
endif()


##########################################################################
#
# Dependencies
# Check for needed libraries and set all related includes, flags, etc. 
#
###############################################################################

# Check for UnitTest++. Required for unit test
if (0)
if(WIN32)
    set( UNITTEST_LIBDIR  ${LOMSE_ROOT_DIR}/packages/UnitTest++/lib )
    set( UNITTEST_INCLUDE_DIRS  ${LOMSE_ROOT_DIR}/packages/UnitTest++/src )
else()
    include(FindPkgConfig)
    pkg_check_modules(UNITTEST REQUIRED unittest++)
endif()
link_directories( ${UNITTEST_LIBDIR} )
endif(0)

# Check for Boost >= 1.42.0
set(Boost_ADDITIONAL_VERSIONS "1.42" "1.42.0" "1.43" "1.43.0" "1.44" "1.44.0"
    "1.45" "1.45.0" "1.46" "1.46.0" "1.47" "1.47.0" "1.48" "1.48.0" "1.49" "1.49.0"
    "1.50" "1.50.0" "1.51" "1.51.0" "1.52" "1.52.0" "1.53" "1.53.0" "1.54" "1.54.0" )
#set(Boost_USE_STATIC_LIBS       OFF)
#set(Boost_USE_MULTITHREADED     OFF)
#set(Boost_USE_STATIC_RUNTIME    OFF)
#set(Boost_DEBUG                 TRUE)

find_package(Boost 1.42.0 COMPONENTS date_time thread REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_libraries( ${Boost_LIBRARIES} )
    link_directories( ${Boost_LIBRARY_DIRS} )
    set(BOOST_VERSION   "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}" )
    if (NOT ".${Boost_SUBMINOR_VERSION}" STREQUAL "." )
	    set(BOOST_VERSION   "${BOOST_VERSION}.${Boost_SUBMINOR_VERSION}" )
	endif()
    message("Boost found: lib version= ${Boost_LIB_VERSION}, version=${BOOST_VERSION}" )
    #string(REGEX REPLACE "_" "." Boost_LIB_VERSION "${Boost_LIB_VERSION}" )
    set(BOOST_DEPENDENCIES 
        "libboost-thread${BOOST_VERSION}, libboost-date-time${BOOST_VERSION}")
    message("Boost found: libraries: ${Boost_LIBRARIES}" )
    message("Boost found: include= ${Boost_INCLUDE_DIRS}" )
    message("Boost found: libdir= ${Boost_LIBRARY_DIRS}" )
    message("Boost dependencies: ${BOOST_DEPENDENCIES}" )
else()
    message(SEND_ERROR "Boost package not found. Reason: ${Boost_ERROR_REASON}" )
endif()


# Check for FreeType
find_package(Freetype REQUIRED)                    
if( FREETYPE_FOUND )
    include_directories( ${FREETYPE_INCLUDE_DIRS} )
    link_libraries( ${FREETYPE_LIBRARIES} )
    link_directories( ${FREETYPE_LINK_DIR} )        
    message("Freetype found: libraries= ${FREETYPE_LIBRARIES}" )
    message("Freetype found: include= ${FREETYPE_INCLUDE_DIRS}" )
    message("Freetype found: libdir= ${FREETYPE_LINK_DIR}" )
else()
    message(SEND_ERROR "FreeType package not found.")
endif()


# Check for libpng
find_package(PNG REQUIRED)                    
if( PNG_FOUND )
    include_directories( ${PNG_INCLUDE_DIRS} )
    link_libraries( ${PNG_LIBRARIES} )
    link_directories( ${PNG_LINK_DIR} )        
    message("libpng found: libraries= ${PNG_LIBRARIES}" )
    message("libpng found: include= ${PNG_INCLUDE_DIRS}" )
    message("libpng found: libdir= ${PNG_LINK_DIR}" )
else()
    message(SEND_ERROR "libpng package not found.")
endif()


# Check for zlib
find_package(ZLIB REQUIRED)
if( ZLIB_FOUND )
    include_directories( ${ZLIB_INCLUDE_DIR} )
    link_libraries( ${ZLIB_LIBRARIES} )
    link_directories( ${ZLIB_LINK_DIR} )        
    message("zlib found: libraries= ${ZLIB_LIBRARIES}" )
    message("zlib found: include= ${ZLIB_INCLUDE_DIRS}" )
    message("zlib found: libdir= ${ZLIB_LINK_DIR}" )
    message("zlib version: ${ZLIB_VERSION_STRING}" )
else()
    message(SEND_ERROR "zlib package not found.")
endif()


#dependencies for building and for pkg-config file
set(LOMSE_REQUIRES "freetype2, libpng, zlib")
set(LOMSE_DEPENDENCIES "libfreetype6, libpng12-0 (>=1.2.42), zlib1g (>= ${ZLIB_VERSION_STRING}), ${BOOST_DEPENDENCIES} ")


# include directories to be installed
set( INCLUDES_LOMSE  ${LOMSE_ROOT_DIR}/include )
set( INCLUDES_AGG  ${LOMSE_ROOT_DIR}/src/agg/include )
set( INCLUDES_UTFCPP  ${LOMSE_ROOT_DIR}/packages/utfcpp )
set( INCLUDES_MINIZIP  ${LOMSE_ROOT_DIR}/packages/minizip )
include_directories(
    ${INCLUDES_LOMSE}
    ${INCLUDES_AGG}
    ${LOMSE_ROOT_DIR}/src/agg/font_freetype
    ${INCLUDES_UTFCPP}
    ${INCLUDES_MINIZIP}
)


link_directories( ${LIBRARY_OUTPUT_PATH} )


# Define possible builds
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# "Print all warnings", macros for GCC & __UNIX__
if(UNIX)
    add_definitions( -Wall -DGCC -D__UNIX__ )
endif(UNIX)

# Force to use static libraries in MSVC
# This doesn't work
if(0)
if(MSVC)
    message("Debug=" ${CMAKE_CXX_FLAGS_DEBUG} )
    message("Release=" ${CMAKE_CXX_FLAGS_RELEASE} )
    message("Linker Debug=" ${CMAKE_EXE_LINKER_FLAGS_DEBUG} )
    message("Linker Release=" ${CMAKE_EXE_LINKER_FLAGS_RELEASE} )
    #set( CMAKE_CXX_FLAGS_RELEASE "/D_DEBUG /MLd /Zi /Ob0 /Od /GZ" ) 
    #set( CMAKE_CXX_FLAGS_DEBUG "/ML /O2 /Ob2 /D NDEBUG" ) 
    #set( CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCD" )
    #set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:LIBCD" )
    message("Debug=" ${CMAKE_CXX_FLAGS_DEBUG} )
    message("Release=" ${CMAKE_CXX_FLAGS_RELEASE} )
    message("Linker Debug=" ${CMAKE_EXE_LINKER_FLAGS_DEBUG} )
    message("Linker Release=" ${CMAKE_EXE_LINKER_FLAGS_RELEASE} )
endif(MSVC)
endif(0)


include_directories("${CMAKE_CURRENT_BINARY_DIR}")



###############################################################################
#
# Target: Lomse library
#
###############################################################################


# source files to compile
include( ${LOMSE_ROOT_DIR}/add-sources.cmake )



#=====================================================================================

# Adds folders for Visual Studio and other IDEs
set (LOMSE_GROUP_FOLDER "lomse/" )
include( ${LOMSE_ROOT_DIR}/add-group-folders.cmake )




###############################################################################
#
# Target: static library
#
###############################################################################

set(LOMSE  lomse)

add_library( ${LOMSE} STATIC ${ALL_LOMSE_SOURCES} )
set_target_properties(${LOMSE} PROPERTIES VERSION ${LOMSE_VERSION}
                                          SOVERSION ${LOMSE_VERSION} )


# hack to remove 'Debug' subdirectory from output path. -------------
# see: http://www.cmake.org/Bug/view.php?id=8243
if(MSVC)
    set_target_properties(${LOMSE} PROPERTIES  PREFIX "../")
endif()

# Force not to link with standard MSVC libraries: /NODEFAULTLIB
if(MSVC)
    set_target_properties(${LOMSE} PROPERTIES  LINK_FLAGS "/NODEFAULTLIB")
endif()

# once built, place library at lomse/bin
set_target_properties(${LOMSE}  PROPERTIES
                                RUNTIME_OUTPUT_DIRECTORY  ${LOMSE_ROOT_DIR}/bin )
    
get_target_property(VAR ${LOMSE} LINK_FLAGS)
#message("library target properties=" ${VAR})



###############################################################################
#
# Target: dynamic library 
#
###############################################################################

# Cannot have two targets with the same name so the dynamic version has
# '-dyn' appended and then the name of the output file is changed

set(LOMSE_DYN  lomse-dyn)

add_library( ${LOMSE_DYN} SHARED ${ALL_LOMSE_SOURCES} )
set_target_properties(${LOMSE_DYN} 
                      PROPERTIES VERSION ${LOMSE_VERSION}
                                 SOVERSION ${LOMSE_VERSION}
                                 OUTPUT_NAME "lomse"
                                 RUNTIME_OUTPUT_DIRECTORY  ${LOMSE_ROOT_DIR}/bin )



###############################################################################
#
# Target: program for testing the library
#
###############################################################################
if(LOMSE_BUILD_TESTS)

    set (TESTLIB  testlib)

    # Dirty hack: I don't know why it conflicts with LIBCD. But for
    # now I need to continue. See:
    # http://mygoldenchariot.blogspot.com/2006/07/link-warning-lnk4098-defaultlib-libcd.html
    if(MSVC)
        set(CMAKE_LINKER_FLAGS_DEBUG
             ${CMAKE_LINKER_FLAGS_DEBUG} "/NODEFAULTLIB:LIBCD"
        )
    endif()

    # source files to compile
    add_executable( ${TESTLIB} 
            ${LOMSE_SRC_DIR}/tests/lomse_test_barline_engraver.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_beam_engraver.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_chord_engraver.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_column_breaker.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_column_layouter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_command.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_document.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_document_cursor.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_document_iterator.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_document_layouter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_elements.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_engrouters.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_events.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_file_system.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_gm_objects.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_graphic_model.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_image_reader.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_im_algorithms.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_inlines_container_layouter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_interactor.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_internal_model.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_ldp_analyser.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_ldp_compiler.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_ldp_exporter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_ldp_parser.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_lines_breaker.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_linkable.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_linker.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_list_layouter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_lmd_analyser.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_lmd_compiler.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_lmd_exporter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_midi_table.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_model_builder.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_multicolumn_layouter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_xml_analyser.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_xml_compiler.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_note_engraver.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_pitch.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_presenter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_reader.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_rest_engraver.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_score_algorithms.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_score_cursor.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_score_iterator.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_score_layouter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_score_meter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_score_player.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_selections.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_shapes.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_sizers.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_stack.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_staffobjs_table.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_system_cursor.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_system_layouter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_table_layouter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_text_engraver.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_text_splitter.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_tokenizer.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_tree.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_tuplet_engraver.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_view.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_test_zip_stream.cpp
            ${LOMSE_SRC_DIR}/tests/lomse_the_test_runner.cpp
    )

    # libraries to link
    if(UNIX)
        target_link_libraries ( ${TESTLIB} liblomse.so libunittest++.a )
    elseif(WIN32)
        target_link_libraries ( ${TESTLIB} lomse.lib UnitTest++.vsnet2003.lib )
    endif()

    # properties
    if(MSVC)
        set_target_properties(${TESTLIB} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBCD" )
        set_target_properties(${TESTLIB} PROPERTIES LINK_FLAGS "/MACHINE:X86" )
        set_target_properties(${TESTLIB} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
        set_target_properties(${TESTLIB} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")

        set_target_properties(${TESTLIB} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
        set_target_properties(${TESTLIB} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
    endif()


    # dependencies
    add_dependencies(testlib lomse)


    # once generated, run tests
    if(WIN32)
        set(TESTLIB_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/debug/${TESTLIB}.exe )
    else()
        set(TESTLIB_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/${TESTLIB} )
    endif()

    #copy test scores and fonts
    #file(COPY ${LOMSE_ROOT_DIR}/test-scores DESTINATION ${OUTDIR})
    #file(COPY ${LOMSE_ROOT_DIR}/fonts DESTINATION ${OUTDIR})

    add_custom_command(
        TARGET ${TESTLIB} POST_BUILD
        COMMAND ${TESTLIB_EXECUTABLE}
        WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    )

endif(LOMSE_BUILD_TESTS)

###############################################################################
#
# Target: Example_1
#
###############################################################################

if (LOMSE_BUILD_EXAMPLE)

    set (EXAMPLE1 example_1)
    set( EXAMPLE1_SRC  ${LOMSE_ROOT_DIR}/examples )

    if( WIN32 )
    #-------------------------------------------------------------
        include_directories(
            ${LOMSE_SRC_DIR}/agg/include/platform/win32/agg_win32_bmp.h
        )

        add_executable( ${EXAMPLE1} WIN32
            ${EXAMPLE1_SRC}/example_1_win32.cpp
            ${EXAMPLE1_SRC}/win32/example_1/example_1.ico
            ${EXAMPLE1_SRC}/win32/example_1/example_1.rc
            ${EXAMPLE1_SRC}/win32/example_1/stdafx.cpp
            ${EXAMPLE1_SRC}/win32/example_1/resource.h
            ${EXAMPLE1_SRC}/win32/example_1/stdafx.h
            ${EXAMPLE1_SRC}/win32/example_1/small.ico
            ${LOMSE_SRC_DIR}/agg/src/platform/win32/agg_win32_bmp.cpp 
        )

        target_link_libraries( ${EXAMPLE1}
            lomse.lib
            ${Boost_LIBRARIES}
            ${FREETYPE_LIBRARIES}
            ${PNG_LIBRARIES}
            ${ZLIB_LIBRARIES}
        )

        
    elseif( UNIX )
    #-------------------------------------------------------------
        find_package(X11)
        if(X11_FOUND)  
            include_directories(${X11_INCLUDE_DIRS})
            link_libraries(${X11_LIBRARIES})
        endif(X11_FOUND)

        add_executable( ${EXAMPLE1} ""
            ${EXAMPLE1_SRC}/example_1_x11.cpp
        )

        target_link_libraries ( ${EXAMPLE1} 
            liblomse.so
            ${Boost_LIBRARIES}
            ${FREETYPE_LIBRARIES}
            ${PNG_LIBRARIES}
            ${ZLIB_LIBRARIES}
        )
            
    endif()

    # dependencies
    add_dependencies(example_1 lomse)

endif(LOMSE_BUILD_EXAMPLE)

###############################################################################
# library installation
#
# You can change the install location by running cmake like this:
#
#   cmake -DCMAKE_INSTALL_PREFIX=/new/install/prefix
#
# By default, CMake uses the following prefix:
#   - Linux: "/usr/local"
#   - Windows: "C:\Program Files\lomse\"
#
###############################################################################

message("CMAKE_INSTALL_PREFIX = " ${CMAKE_INSTALL_PREFIX} )

if( UNIX )

    #define base paths
    set(LOMSE_LIBDIR        ${CMAKE_INSTALL_PREFIX}/lib)
    set(LOMSE_INCLUDEDIR    ${CMAKE_INSTALL_PREFIX}/include/lomse )

    # copy generated libraries
    install( TARGETS ${LOMSE} DESTINATION ${LOMSE_LIBDIR} )
    install( TARGETS ${LOMSE_DYN} LIBRARY DESTINATION ${LOMSE_LIBDIR} )

    #create symlink to library
	install( CODE
        "EXECUTE_PROCESS(COMMAND ln -fs ${CMAKE_INSTALL_PREFIX}/lib/${LOMSE_DYN} 
                                        ${CMAKE_INSTALL_PREFIX}/lib/liblomse.so )" 
    )

    # copy lomse includes
    install(DIRECTORY ${INCLUDES_LOMSE}/
            DESTINATION "${LOMSE_INCLUDEDIR}"
            FILES_MATCHING PATTERN "*.h"
                           PATTERN "*.hpp"
                           PATTERN "*.txt"
                           PATTERN ".svn" EXCLUDE
                           PATTERN "platform" EXCLUDE )

    # copy agg includes
    install(DIRECTORY "${INCLUDES_AGG}/"
            DESTINATION "${LOMSE_INCLUDEDIR}"
            FILES_MATCHING PATTERN "*.h"
                           PATTERN ".svn" EXCLUDE )

    # copy minizip includes
    install(DIRECTORY "${INCLUDES_MINIZIP}/"
            DESTINATION "${LOMSE_INCLUDEDIR}"
            FILES_MATCHING PATTERN "*.h"
                           PATTERN ".svn" EXCLUDE )

    # copy utfcpp includes
    install(DIRECTORY "${INCLUDES_UTFCPP}/"
            DESTINATION "${LOMSE_INCLUDEDIR}"
            FILES_MATCHING PATTERN "*.h"
                           PATTERN ".svn" EXCLUDE )

    # copy also lomse_config.h
    install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/lomse_config.h 
            DESTINATION "${LOMSE_INCLUDEDIR}" )

    #copy fonts
    install(DIRECTORY  "${LOMSE_ROOT_DIR}/fonts/"
            DESTINATION ${FONTS_PATH}
            FILES_MATCHING PATTERN "*.ttf" 
                           PATTERN "*.ttc"
                           PATTERN "Bravura.otf" 
                           PATTERN ".svn" EXCLUDE )

elseif( WIN32 )
#-------------------------------------------------------------

    #define base paths
    set(LOMSE_LIBDIR        ${CMAKE_INSTALL_PREFIX}/lib)
    set(LOMSE_INCLUDEDIR    ${CMAKE_INSTALL_PREFIX}/include/lomse )

    # copy generated libraries
    install( TARGETS ${LOMSE} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" )
    install( TARGETS ${LOMSE_DYN} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" )

    # copy lomse includes
    install(DIRECTORY ${INCLUDES_LOMSE}/
            DESTINATION "${LOMSE_INCLUDEDIR}"
            FILES_MATCHING PATTERN "*.h"
                           PATTERN ".svn" EXCLUDE
                           PATTERN "platform" EXCLUDE )

    # copy agg includes
    install(DIRECTORY "${INCLUDES_AGG}/"
            DESTINATION "${LOMSE_INCLUDEDIR}"
            FILES_MATCHING PATTERN "*.h"
                           PATTERN ".svn" EXCLUDE )

    # copy minizip includes
    install(DIRECTORY "${INCLUDES_MINIZIP}/"
            DESTINATION "${LOMSE_INCLUDEDIR}"
            FILES_MATCHING PATTERN "*.h"
                           PATTERN ".svn" EXCLUDE )

    # copy utfcpp includes
    install(DIRECTORY "${INCLUDES_UTFCPP}/"
            DESTINATION "${LOMSE_INCLUDEDIR}"
            FILES_MATCHING PATTERN "*.h"
                           PATTERN ".svn" EXCLUDE )
    install(DIRECTORY "${INCLUDES_UTFCPP}/utf8"
            DESTINATION "${LOMSE_INCLUDEDIR}/utf8"
            FILES_MATCHING PATTERN "*.h"
                           PATTERN ".svn" EXCLUDE )

    # copy also lomse_config.h
    install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/lomse_config.h 
            DESTINATION "${LOMSE_INCLUDEDIR}" )

    #copy fonts
    install(DIRECTORY  "${LOMSE_ROOT_DIR}/fonts/"
            DESTINATION ${FONTS_PATH}
            FILES_MATCHING PATTERN "*.ttf" 
                           PATTERN "*.ttc"
                           PATTERN "Bravura.otf" 
                           PATTERN ".svn" EXCLUDE )

endif()


###############################################################################
# create and install the pkg-config file: liblomse.pc
#
# By default, pkg-config file will be installed in /usr/lib/pkgconfig
# You can change the install location by running cmake like this:
#
#   cmake -DLOMSE_PKG_CONFIG_INSTALL=/new/install/path
#
###############################################################################

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

message("pkg-config install path: ${LOMSE_PKG_CONFIG_INSTALL}" )


###############################################################################
#
# CPack section: installers generation
#
#   Currently it just builds the Debian binary package.
# 
###############################################################################

set(CPACK_PACKAGE_VENDOR "LenMus Project")
set(CPACK_PACKAGE_VERSION ${LOMSE_VERSION_STRING} )
set(CPACK_PACKAGE_NAME "liblomse_${LOMSE_VERSION_STRING}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Free open source library to render music scores")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${LOMSE_ROOT_DIR}/README")
set(CPACK_PACKAGE_CONTACT "s.cecilio@gmail.com")

set(CPACK_RESOURCE_FILE_LICENSE "${LOMSE_ROOT_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${LOMSE_ROOT_DIR}/README")

if(UNIX)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Cecilio Salmeron") #required
    set(CPACK_DEBIAN_PACKAGE_NAME ${CPACK_PACKAGE_NAME} )
    set(CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION} )

    IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^x86_64$|^amd64$")  
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    ELSE()
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
    ENDIF() 

    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\r\rLomse is a free open source project to provide software developers with\r a C++ library for rendering, editing and playing back music scores.\r With it, you can add capabilities to any program for displaying and\r editing music scores. It is witten in C++, it is platform independent.\r\rLomse website: http://www.lenmus.org/en/lomse/intro")

    set(CPACK_PACKAGE_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS ${LOMSE_DEPENDENCIES})
    message("Package dependencies: ${CPACK_DEBIAN_PACKAGE_DEPENDS}" )

#elseif( WIN32 )
#    # MS Windows package
#    set(CPACK_GENERATOR "NSIS")

#    # There is a bug in NSI that does not handle full unix paths properly. Make
#    # sure there is at least one set of four (4) backlasshes.
#    SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
#    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
#    SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} My Famous Project")
#    SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
#    SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
#    SET(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
#    SET(CPACK_NSIS_MODIFY_PATH ON)

endif()

include(CPack)


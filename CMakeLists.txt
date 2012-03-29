#--------------------------------------------------------------------------------------
# This file is part of the Lomse library.
# Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
#  This is a CMake configuration file for building makefiles and installfiles for
#  the library and all test/demo programs
#
#  To use it you need CMake which can be downloaded from http://www.cmake.org/
#
#  This file has been tested with the following generators:
#   Windows:
#       Visual Studio 7 .NET
#       Visual Studio 2008
#   Linux:
#       Unix Makefiles
#       CodeBlocks - Unix Makefiles
# 
#-------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8)

# project name
project(lomse)

# main directories 
set( ROOT_DIR  ${CMAKE_CURRENT_SOURCE_DIR} )
set( SRC_DIR  ${ROOT_DIR}/src )
set( PKG_DIR  ${ROOT_DIR}/packages )
set( OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/bin )
set( EXECUTABLE_OUTPUT_PATH ${OUTDIR})
set( LIBRARY_OUTPUT_PATH ${OUTDIR})
message("ROOT_DIR: ${ROOT_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("SRC_DIR: ${SRC_DIR}")
message("EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
message("LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
message("OUTDIR: ${OUTDIR}")


# directories to search for CMake modules
set( CMAKE_MODULE_PATH "${ROOT_DIR}/cmake-modules" )


if(0)
# Various options that the user can select (var, msg, default value).
# Values can be changed directly in the CMake GUI or through
# the command line by prefixing a variable's name with '-D':
# i.e.:    cmake -DLOMSE_BUILD_DEBUG=ON
#
#-------------------------------------------------------------------
option(LOMSE_BUILD_STATIC_LIB "Build Lomse as static library" ON)
option(LOMSE_BUILD_DEBUG      "Build with debug symbols" OFF)

if(WIN32)
    option(LOMSE_USE_PREBUILT_FREETYPE "Use supplied prebuilt lib for freetype" OFF)
    option(LOMSE_USE_PREBUILT_BOOST "Use supplied prebuilt lib for boost" OFF)
    option(LOMSE_USE_PREBUILT_OTHER "Use supplied prebuilt libs for zlib and libpng" OFF)
endif()

endif()



# set up configuration variables for lomse_config.h
#------------------------------------------------------

# version. Get SVN revision number
include(FindSubversion)
if(Subversion_FOUND)
    Subversion_WC_INFO(${ROOT_DIR} LOMSE)
    message("Current Lomse revision is ${LOMSE_WC_REVISION}")
    set( LOMSE_REVISION ${LOMSE_WC_REVISION})
else()
    message("Subversion not found. Revision set to zero.")
    set( LOMSE_REVISION "0")
endif()

# build type
set( LOMSE_USE_DLL "0")
set( LOMSE_CREATE_DLL "0")

# build options
set( LOMSE_DEBUG "0")
set( LOMSE_COMPATIBILITY_1_5 "1")

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


# path to scores for tests and for fonts
set( LOMSE_TEST_SCORES_PATH    "\"${ROOT_DIR}/test-scores/\"" )
set( LOMSE_FONTS_PATH          "\"${ROOT_DIR}/fonts/\"" )
 

# check that compiler supports namespace sdt
include(TestForSTDNamespace)
if(CMAKE_NO_STD_NAMESPACE)
  message(FATAL_ERROR "The compiler doesn't support namespace std.")
endif()


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


# Check for needed libraries and set all related includes, flags, etc. 
#-------------------------------------------------------------------------


# Check for UnitTest++. Required for unit test
if(WIN32)
    set( UNITTEST_LIBDIR  ${ROOT_DIR}/packages/UnitTest++/lib )
    set( UNITTEST_INCLUDE_DIRS  ${ROOT_DIR}/packages/UnitTest++/src )
else()
    include(FindPkgConfig)
    pkg_check_modules(UNITTEST REQUIRED unittest++)
endif()
link_directories( ${UNITTEST_LIBDIR} )


# Check for Boost >= 1.42.0
set(Boost_ADDITIONAL_VERSIONS "1.42" "1.42.0" "1.43" "1.43.0" "1.44" "1.44.0"
    "1.45" "1.45.0" "1.46" "1.46.0" "1.47" "1.47.0")
#set(Boost_USE_STATIC_LIBS       OFF)
#set(Boost_USE_MULTITHREADED     OFF)
#set(Boost_USE_STATIC_RUNTIME    OFF)
#set(Boost_DEBUG                 TRUE)

find_package(Boost 1.42.0 COMPONENTS date_time thread REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_libraries( ${Boost_LIBRARIES} )
    link_directories( ${Boost_LIBRARY_DIRS} )
#    message("Boost found: libraries: ${Boost_LIBRARIES}" )
#    message("Boost found: include= ${Boost_INCLUDE_DIRS}" )
#    message("Boost found: libdir= ${Boost_LIBRARY_DIRS}" )
else()
    message(SEND_ERROR "Boost package not found. Reason: ${Boost_ERROR_REASON}" )
endif()


# Check for FreeType
find_package(Freetype REQUIRED)                    
if( FREETYPE_FOUND )
    include_directories( ${FREETYPE_INCLUDE_DIRS} )
    link_libraries( ${FREETYPE_LIBRARIES} )
    link_directories( ${FREETYPE_LINK_DIR} )        
#    message("Freetype found: libraries= ${FREETYPE_LIBRARIES}" )
#    message("Freetype found: include= ${FREETYPE_INCLUDE_DIRS}" )
#    message("Freetype found: libdir= ${FREETYPE_LINK_DIR}" )
else()
    message(SEND_ERROR "FreeType package not found.")
endif()


# Check for libpng
find_package(PNG REQUIRED)                    
if( PNG_FOUND )
    include_directories( ${PNG_INCLUDE_DIRS} )
    link_libraries( ${PNG_LIBRARIES} )
    link_directories( ${PNG_LINK_DIR} )        
#    message("libpng found: libraries= ${PNG_LIBRARIES}" )
#    message("libpng found: include= ${PNG_INCLUDE_DIRS}" )
#    message("libpng found: libdir= ${PNG_LINK_DIR}" )
else()
    message(SEND_ERROR "libpng package not found.")
endif()


# Check for zlib
find_package(ZLIB REQUIRED)
if( ZLIB_FOUND )
    include_directories( ${ZLIB_INCLUDE_DIR} )
    link_libraries( ${ZLIB_LIBRARIES} )
    link_directories( ${ZLIB_LINK_DIR} )        
#    message("zlib found: libraries= ${ZLIB_LIBRARIES}" )
#    message("zlib found: include= ${ZLIB_INCLUDE_DIRS}" )
#    message("zlib found: libdir= ${ZLIB_LINK_DIR}" )
else()
    message(SEND_ERROR "zlib package not found.")
endif()



# include directories to be installed
set( INCLUDES_LOMSE  ${ROOT_DIR}/include )
set( INCLUDES_AGG  ${ROOT_DIR}/src/agg/include )
include_directories(
    ${INCLUDES_LOMSE}
    ${INCLUDES_AGG}
    ${ROOT_DIR}/src/agg/font_freetype
    ${ROOT_DIR}/packages/utfcpp
    ${ROOT_DIR}/packages/minizip
    ${UNITTEST_INCLUDE_DIRS}
)


link_directories( ${LIBRARY_OUTPUT_PATH} )


# Define possible builds
set(CMAKE_CONFIGURATION_TYPES Debug Release)


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


#define a header file to pass CMake settings to source code
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/lomse_config.h.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/lomse_config.h"
)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")




#////////////////////////////////////////////////////////////////////////
# Target: Lomse library
#////////////////////////////////////////////////////////////////////////


# source files to compile

set(AGG_FILES
    ${SRC_DIR}/agg/src/agg_arc.cpp
    ${SRC_DIR}/agg/src/agg_bezier_arc.cpp
    ${SRC_DIR}/agg/src/agg_curves.cpp
    ${SRC_DIR}/agg/src/agg_gsv_text.cpp
    ${SRC_DIR}/agg/src/agg_line_aa_basics.cpp
    ${SRC_DIR}/agg/src/agg_rounded_rect.cpp
    ${SRC_DIR}/agg/src/agg_trans_affine.cpp
    ${SRC_DIR}/agg/src/agg_vcgen_contour.cpp
    ${SRC_DIR}/agg/src/agg_vcgen_markers_term.cpp
    ${SRC_DIR}/agg/src/agg_vcgen_stroke.cpp
)

set(DOCUMENT_FILES
    ${SRC_DIR}/document/lomse_document.cpp
    ${SRC_DIR}/document/lomse_document_iterator.cpp
    ${SRC_DIR}/document/lomse_id_assigner.cpp
)

set(EXPORTERS_FILES
    ${SRC_DIR}/exporters/lomse_ldp_exporter.cpp
)

set(FILE_SYSTEM_FILES
    ${SRC_DIR}/file_system/lomse_file_system.cpp
    ${SRC_DIR}/file_system/lomse_image_reader.cpp
    ${SRC_DIR}/file_system/lomse_zip_stream.cpp
)

set(GRAPHIC_MODEL_FILES
    ${SRC_DIR}/graphic_model/lomse_box_score_page.cpp
    ${SRC_DIR}/graphic_model/lomse_box_slice.cpp
    ${SRC_DIR}/graphic_model/lomse_box_slice_instr.cpp
    ${SRC_DIR}/graphic_model/lomse_box_system.cpp
    ${SRC_DIR}/graphic_model/lomse_glyphs.cpp
    ${SRC_DIR}/graphic_model/lomse_gm_basic.cpp
    ${SRC_DIR}/graphic_model/lomse_selections.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_barline.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_base.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_beam.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_brace_bracket.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_line.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_note.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_staff.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_text.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_tie.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_tuplet.cpp
    ${SRC_DIR}/graphic_model/lomse_shapes.cpp
    ${SRC_DIR}/graphic_model/lomse_shapes_storage.cpp
    ${SRC_DIR}/graphic_model/lomse_sizers.cpp
    
    ${SRC_DIR}/graphic_model/engravers/lomse_accidentals_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_beam_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_barline_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_chord_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_clef_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_fermata_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_instrument_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_key_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_line_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_note_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_noterest_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_rest_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_slur_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_text_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_tie_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_time_engraver.cpp
    ${SRC_DIR}/graphic_model/engravers/lomse_tuplet_engraver.cpp

    ${SRC_DIR}/graphic_model/layouters/lomse_box_content_layouter.cpp
    ${SRC_DIR}/graphic_model/layouters/lomse_content_layouter.cpp
    ${SRC_DIR}/graphic_model/layouters/lomse_document_layouter.cpp
    ${SRC_DIR}/graphic_model/layouters/lomse_layouter.cpp
    ${SRC_DIR}/graphic_model/layouters/lomse_score_layouter.cpp
    ${SRC_DIR}/graphic_model/layouters/lomse_score_meter.cpp
    ${SRC_DIR}/graphic_model/layouters/lomse_staffobjs_cursor.cpp
    ${SRC_DIR}/graphic_model/layouters/lomse_system_layouter.cpp
)

set(GUI_CONTROLS_FILES
    ${SRC_DIR}/gui_controls/lomse_hyperlink_ctrl.cpp
    ${SRC_DIR}/gui_controls/lomse_score_player_ctrl.cpp
    ${SRC_DIR}/gui_controls/lomse_static_text_ctrl.cpp
)

set(INTERNAL_MODEL_FILES
    ${SRC_DIR}/internal_model/lomse_im_factory.cpp
    ${SRC_DIR}/internal_model/lomse_im_figured_bass.cpp
    ${SRC_DIR}/internal_model/lomse_im_note.cpp
    ${SRC_DIR}/internal_model/lomse_internal_model.cpp
    ${SRC_DIR}/internal_model/lomse_score_utilities.cpp
)

set(MODULE_FILES
    ${SRC_DIR}/module/lomse_doorway.cpp
    ${SRC_DIR}/module/lomse_events.cpp
    ${SRC_DIR}/module/lomse_image.cpp
    ${SRC_DIR}/module/lomse_injectors.cpp
    ${SRC_DIR}/module/lomse_logger.cpp
    ${SRC_DIR}/module/lomse_pitch.cpp
    ${SRC_DIR}/module/lomse_time.cpp
)

set(MVC_FILES
    ${SRC_DIR}/mvc/lomse_graphic_view.cpp
    ${SRC_DIR}/mvc/lomse_interactor.cpp
    ${SRC_DIR}/mvc/lomse_presenter.cpp 
    ${SRC_DIR}/mvc/lomse_tasks.cpp
    ${SRC_DIR}/mvc/lomse_view.cpp
)

set(PARSER_FILES
    ${SRC_DIR}/parser/lomse_analyser.cpp
    ${SRC_DIR}/parser/lomse_compiler.cpp
    ${SRC_DIR}/parser/lomse_ldp_elements.cpp
    ${SRC_DIR}/parser/lomse_ldp_factory.cpp
    ${SRC_DIR}/parser/lomse_linker.cpp
    ${SRC_DIR}/parser/lomse_model_builder.cpp
    ${SRC_DIR}/parser/lomse_parser.cpp
    ${SRC_DIR}/parser/lomse_reader.cpp
    ${SRC_DIR}/parser/lomse_staffobjs_table.cpp
    ${SRC_DIR}/parser/lomse_tokenizer.cpp
)

set(RENDER_FILES
    ${SRC_DIR}/render/lomse_calligrapher.cpp
    ${SRC_DIR}/render/lomse_font_freetype.cpp
    ${SRC_DIR}/render/lomse_font_storage.cpp
    ${SRC_DIR}/render/lomse_renderer.cpp
    ${SRC_DIR}/render/lomse_screen_drawer.cpp
)

set(SCORE_FILES
    ${SRC_DIR}/score/lomse_score_iterator.cpp
)

set(SOUND_FILES
    ${SRC_DIR}/sound/lomse_midi_table.cpp
    ${SRC_DIR}/sound/lomse_score_player.cpp
)

set(PACKAGES_FILES
    ${PKG_DIR}/minizip/unzip.c
    ${PKG_DIR}/minizip/ioapi.c
)

if(WIN32)
    set(PLATFORM_FILES
        ${SRC_DIR}/agg/src/platform/win32/agg_win32_bmp.cpp
    )
elseif(UNIX)
    set(PLATFORM_FILES
        
    )
endif()

set(ALL_SOURCES 
    ${AGG_FILES} ${DOCUMENT_FILES} ${EXPORTERS_FILES} ${FILE_SYSTEM_FILES}
    ${GRAPHIC_MODEL_FILES} ${GUI_CONTROLS_FILES} ${INTERNAL_MODEL_FILES} 
    ${MODULE_FILES} ${MVC_FILES}
    ${PARSER_FILES} ${RENDER_FILES} ${SCORE_FILES} 
    ${SOUND_FILES} ${PLATFORM_FILES} ${PACKAGES_FILES}
)


# Adds folders for Visual Studio and other IDEs
source_group( "agg"             FILES ${AGG_FILES} ${PLATFORM_FILES} )
source_group( "document"        FILES ${DOCUMENT_FILES} )
source_group( "exporters"       FILES ${EXPORTERS_FILES} )
source_group( "file_system"     FILES ${FILE_SYSTEM_FILES} )
source_group( "graphic_model"   FILES ${GRAPHIC_MODEL_FILES} )
source_group( "gui_controls"    FILES ${GUI_CONTROLS_FILES} )
source_group( "internal_model"  FILES ${INTERNAL_MODEL_FILES} )
source_group( "module"          FILES ${MODULE_FILES} )
source_group( "mvc"             FILES ${MVC_FILES} )
source_group( "parser"          FILES ${PARSER_FILES} )
source_group( "render"          FILES ${RENDER_FILES} )
source_group( "score"           FILES ${SCORE_FILES} )
source_group( "sound"           FILES ${SOUND_FILES} )
source_group( "packages"        FILES ${PACKAGES_FILES} )



#////////////////////////////////////////////////////////////////////////
# Target: static library
#////////////////////////////////////////////////////////////////////////

set(LOMSE  lomse)

set(LOMSE_VERSION "${LOMSE_VERSION_MAJOR}.${LOMSE_VERSION_MINOR}" )

add_library( ${LOMSE} STATIC ${ALL_SOURCES} )
set_target_properties(${LOMSE} PROPERTIES VERSION ${LOMSE_VERSION}
                                          SOVERSION ${LOMSE_VERSION_MAJOR} )


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
                                RUNTIME_OUTPUT_DIRECTORY  ${ROOT_DIR}/bin )
    
get_target_property(VAR ${LOMSE} LINK_FLAGS)
#message("library target properties=" ${VAR})



#////////////////////////////////////////////////////////////////////////
# Target: dynamic library 
#////////////////////////////////////////////////////////////////////////

# Cannot have two targets with the same name so the dynamic version has
# '-dyn' appended and then the name of the output file is changed

set(LOMSE_DYN  lomse-dyn)

add_library( ${LOMSE_DYN} SHARED ${ALL_SOURCES} )
set_target_properties(${LOMSE_DYN} 
                      PROPERTIES VERSION ${LOMSE_VERSION}
                                 SOVERSION ${LOMSE_VERSION_MAJOR}
                                 OUTPUT_NAME "lomse"
                                 RUNTIME_OUTPUT_DIRECTORY  ${ROOT_DIR}/bin )



#////////////////////////////////////////////////////////////////////////
# Target: program for testing the library
#////////////////////////////////////////////////////////////////////////

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
        ${SRC_DIR}/tests/lomse_test_analyser.cpp
        ${SRC_DIR}/tests/lomse_test_barline_engraver.cpp
        ${SRC_DIR}/tests/lomse_test_beam_engraver.cpp
        ${SRC_DIR}/tests/lomse_test_box_content_layouter.cpp
        ${SRC_DIR}/tests/lomse_test_chord_engraver.cpp
        ${SRC_DIR}/tests/lomse_test_column_breaker.cpp
        ${SRC_DIR}/tests/lomse_test_column_layouter.cpp
        ${SRC_DIR}/tests/lomse_test_compiler.cpp
        ${SRC_DIR}/tests/lomse_test_document.cpp
        ${SRC_DIR}/tests/lomse_test_document_iterator.cpp
        ${SRC_DIR}/tests/lomse_test_document_layouter.cpp
        ${SRC_DIR}/tests/lomse_test_elements.cpp
        ${SRC_DIR}/tests/lomse_test_events.cpp
        ${SRC_DIR}/tests/lomse_test_file_system.cpp
        ${SRC_DIR}/tests/lomse_test_gm_objects.cpp
        ${SRC_DIR}/tests/lomse_test_graphic_model.cpp
        ${SRC_DIR}/tests/lomse_test_image_reader.cpp
        ${SRC_DIR}/tests/lomse_test_interactor.cpp
        ${SRC_DIR}/tests/lomse_test_internal_model.cpp
        ${SRC_DIR}/tests/lomse_test_ldp_exporter.cpp
        ${SRC_DIR}/tests/lomse_test_lines_breaker.cpp
        ${SRC_DIR}/tests/lomse_test_linkable.cpp
        ${SRC_DIR}/tests/lomse_test_linker.cpp
        ${SRC_DIR}/tests/lomse_test_midi_table.cpp
        ${SRC_DIR}/tests/lomse_test_model_builder.cpp
        ${SRC_DIR}/tests/lomse_test_multicolumn_layouter.cpp
        ${SRC_DIR}/tests/lomse_test_note_engraver.cpp
        ${SRC_DIR}/tests/lomse_test_parser.cpp
        ${SRC_DIR}/tests/lomse_test_pitch.cpp
        ${SRC_DIR}/tests/lomse_test_presenter.cpp
        ${SRC_DIR}/tests/lomse_test_reader.cpp
        ${SRC_DIR}/tests/lomse_test_rest_engraver.cpp
        ${SRC_DIR}/tests/lomse_test_score_iterator.cpp
        ${SRC_DIR}/tests/lomse_test_score_layouter.cpp
        ${SRC_DIR}/tests/lomse_test_score_meter.cpp
        ${SRC_DIR}/tests/lomse_test_score_player.cpp
        ${SRC_DIR}/tests/lomse_test_selections.cpp
        ${SRC_DIR}/tests/lomse_test_shapes.cpp
        ${SRC_DIR}/tests/lomse_test_sizers.cpp
        ${SRC_DIR}/tests/lomse_test_stack.cpp
        ${SRC_DIR}/tests/lomse_test_staffobjs_table.cpp
        ${SRC_DIR}/tests/lomse_test_system_cursor.cpp
        ${SRC_DIR}/tests/lomse_test_system_layouter.cpp
        ${SRC_DIR}/tests/lomse_test_tokenizer.cpp
        ${SRC_DIR}/tests/lomse_test_tree.cpp
        ${SRC_DIR}/tests/lomse_test_tuplet_engraver.cpp
        ${SRC_DIR}/tests/lomse_test_view.cpp
        ${SRC_DIR}/tests/lomse_test_zip_stream.cpp
        ${SRC_DIR}/tests/lomse_the_test_runner.cpp
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
file(COPY ${ROOT_DIR}/test-scores DESTINATION ${OUTDIR})
file(COPY ${ROOT_DIR}/fonts DESTINATION ${OUTDIR})

add_custom_command(
    TARGET ${TESTLIB} POST_BUILD
    COMMAND ${TESTLIB_EXECUTABLE}
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
)



#////////////////////////////////////////////////////////////////////////
# Target: Example_1
#////////////////////////////////////////////////////////////////////////


set (EXAMPLE1 example_1)
set( EXAMPLE1_SRC  ${ROOT_DIR}/examples )

if( WIN32 )
#-------------------------------------------------------------
    include_directories(
        ${SRC_DIR}/agg/include/platform/win32/agg_win32_bmp.h
    )

    add_executable( ${EXAMPLE1} WIN32
        ${EXAMPLE1_SRC}/example_1_win32.cpp
        ${EXAMPLE1_SRC}/win32/example_1/example_1.ico
        ${EXAMPLE1_SRC}/win32/example_1/example_1.rc
        ${EXAMPLE1_SRC}/win32/example_1/stdafx.cpp
        ${EXAMPLE1_SRC}/win32/example_1/resource.h
        ${EXAMPLE1_SRC}/win32/example_1/stdafx.h
        ${EXAMPLE1_SRC}/win32/example_1/small.ico
        ${SRC_DIR}/agg/src/platform/win32/agg_win32_bmp.cpp 
    )

    target_link_libraries( ${EXAMPLE1} lomse.lib )

    
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

    target_link_libraries ( ${EXAMPLE1} liblomse.so )

        
endif()

# dependencies
add_dependencies(example_1 lomse)



#///////////////////////////////////////////////////////////////////////////////
# library installation
# You can change the install location by running cmake like this:
#
#   cmake -DCMAKE_INSTALL_PREFIX=/new/install/prefix
#
# By default, CMake uses the following prefix:
#   - Linux: "/usr/local"
#   - Windows: "C:\Program Files\lomse\"
#///////////////////////////////////////////////////////////////////////////////

message("CMAKE_INSTALL_PREFIX = " ${CMAKE_INSTALL_PREFIX} )

#if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#set(CMAKE_INSTALL_PREFIX
#    "/opt/foo" CACHE PATH "FOO install prefix" FORCE
#)
#endif()

if( UNIX )

    # copy generated libraries
    install( TARGETS ${LOMSE} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" )
    install( TARGETS ${LOMSE_DYN} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" )

    # copy lomse includes
    install(DIRECTORY ${INCLUDES_LOMSE}/
            DESTINATION "${CMAKE_INSTALL_PREFIX}/include/lomse"
            FILES_MATCHING PATTERN "*.h"
            PATTERN ".svn" EXCLUDE
            PATTERN "platform" EXCLUDE )

    # copy agg includes
    install(DIRECTORY "${INCLUDES_AGG}/"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/include/lomse"
            FILES_MATCHING PATTERN "*.h"
            PATTERN ".svn" EXCLUDE )

    # copy also lomse_config.h
    install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/lomse_config.h 
            DESTINATION "${CMAKE_INSTALL_PREFIX}/include/lomse" )

elseif( WIN32 )
#-------------------------------------------------------------

    # copy generated libraries
    install( TARGETS ${LOMSE} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" )
    install( TARGETS ${LOMSE_DYN} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" )

    # copy lomse includes
    install(DIRECTORY ${INCLUDES_LOMSE}/
            DESTINATION "${CMAKE_INSTALL_PREFIX}/include/lomse"
            FILES_MATCHING PATTERN "*.h"
            PATTERN ".svn" EXCLUDE
            PATTERN "platform" EXCLUDE )

    # copy agg includes
    install(DIRECTORY "${INCLUDES_AGG}/"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/include/lomse"
            FILES_MATCHING PATTERN "*.h"
            PATTERN ".svn" EXCLUDE )

    # copy also lomse_config.h
    install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/lomse_config.h 
            DESTINATION "${CMAKE_INSTALL_PREFIX}/include/lomse" )

endif()


## Set the path of the application executable
#if( MSVC )
#    set( EXE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/${TESTLIB}${CMAKE_EXECUTABLE_SUFFIX} )
#else()
#    set( EXE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTLIB}${CMAKE_EXECUTABLE_SUFFIX} )
#endif()

if(0)
########### install source files ###############
install(FILES  AUTHORS ChangeLog COPYING NEWS README THANKS 
        DESTINATION ${DATADIR}/doc/${CMAKE_PROJECT_NAME}
)

install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/lomse_config.h 
        DESTINATION ${INCLUDEDIR}/${CMAKE_PROJECT_NAME}
)
endif(0)

if(0)
#///////////////////////////////////////////////////////////////////////////////
# CPack section: installers generation
#///////////////////////////////////////////////////////////////////////////////

set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_IGNORE_FILES
    /[.]svn/ /.*[.]o$ /.*[.]o/ /autom4te[.]cache/ /[.].* /bootstrap 
    /RELEASE /styles/ /papers/ /rpm.am /msi.am )
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_SOURCE_PACKAGE_FILE_NAME
    ${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION})
    
set(CPACK_PACKAGE_VENDOR "LenMus")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Library to render music scores")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "lomse")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
 
include(CPack)
 
endif(0)


#--------------------------------------------------------------------------------------
#  This file is part of the Lomse library.
#  Copyright (c) 2010 Lomse project
#
#  Lomse is free software; you can redistribute it and/or modify it under the
#  terms of the GNU General Public License as published by the Free Software Foundation,
#  either version 3 of the License, or (at your option) any later version.
#
#  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
#  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with Lomse; if not, see <http://www.gnu.org/licenses/>.
#  
#  For any comment, suggestion or feature request, please contact the manager of
#  the project at cecilios@users.sourceforge.net
#
#-------------------------------------------------------------------------------------

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

project(lomse)

# configuration
set( LOMSE_VERSION_MAJOR "0")
set( LOMSE_VERSION_MINOR "0")
set( LOMSE_USE_DLL "0")
set( LOMSE_CREATE_DLL "0")

#set platform
if(WIN32)
    set( LOMSE_PLATFORM_WIN32 "1")
    set( LOMSE_PLATFORM_UNIX "0")
    
elseif(UNIX)
    set( LOMSE_PLATFORM_WIN32 "0")
    set( LOMSE_PLATFORM_UNIX "1")
endif()

#set compiler
if(MSVC)
    set( LOMSE_COMPILER_MSVC "1")
    set( LOMSE_COMPILER_GCC "0")
    
elseif(CMAKE_COMPILER_IS_GNUCC)
    set( LOMSE_COMPILER_MSVC "0")
    set( LOMSE_COMPILER_GCC "1")
endif()


# set directories 
set( ROOT_DIR  ${CMAKE_CURRENT_SOURCE_DIR} )
set( SRC_DIR  ${ROOT_DIR}/src )
set( OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/bin )
set( EXECUTABLE_OUTPUT_PATH ${OUTDIR})
set( LIBRARY_OUTPUT_PATH ${OUTDIR})

# path to scores for tests
set(LOMSE_TEST_SCORES_PATH "\"${ROOT_DIR}/test-scores/\"" )

# directories to search for CMake modules
set( CMAKE_MODULE_PATH "${ROOT_DIR}/cmake-modules" )


# check that compiler supports namespace sdt
include(TestForSTDNamespace)
if(CMAKE_NO_STD_NAMESPACE)
  message(FATAL_ERROR "The compiler doesn't support namespace std.")
endif()


# set path to include unittest++ header files
if(WIN32)
    set( UNITTEST_LIBDIR  ${ROOT_DIR}/packages/UnitTest++/lib )
    set( UNITTEST_INCLUDE_DIRS  ${ROOT_DIR}/packages/UnitTest++/src )
else()
    include(FindPkgConfig)
    pkg_check_modules(UNITTEST REQUIRED unittest++)
endif()
 
include_directories(
    ${ROOT_DIR}/include
    ${ROOT_DIR}/include/platform
    ${ROOT_DIR}/packages/utfcpp
    ${ROOT_DIR}/src/agg/include
    ${ROOT_DIR}/src/agg/font_freetype
    ${UNITTEST_INCLUDE_DIRS}
)

link_directories(
    ${UNITTEST_LIBDIR}
    ${LIBRARY_OUTPUT_PATH} 
)


# Define possible builds
set(CMAKE_CONFIGURATION_TYPES Debug Release)


# Set all includes, flags, libraries, related to freetype
#-------------------------------------------------------------------------------------

find_package( Freetype )                    
if( FREETYPE_FOUND )
    include_directories( ${FREETYPE_INCLUDE_DIRS} )
    link_libraries( ${FREETYPE_LIBRARIES} )
    link_directories( ${FREETYPE_LINK_DIR} )        
else()
    message(SEND_ERROR "FreeType package not found.")
endif()


# names for libraries and execs.
#-------------------------------------------------------------------------------------
if( WIN32 )
    set( CMAKE_STATIC_LIBRARY_PREFIX "" )
    set( CMAKE_STATIC_LIBRARY_SUFFIX ".lib" )
    set( CMAKE_SHARED_LIBRARY_PREFIX "" )
    set( CMAKE_SHARED_LIBRARY_SUFFIX ".dll" )
    set( CMAKE_EXECUTABLE_SUFFIX ".exe" )

endif( WIN32 )

if( UNIX )
    set( CMAKE_STATIC_LIBRARY_PREFIX "lib" )
    set( CMAKE_STATIC_LIBRARY_SUFFIX ".a" )
    set( CMAKE_SHARED_LIBRARY_PREFIX "lib" )
    set( CMAKE_SHARED_LIBRARY_SUFFIX ".so" )
    set( CMAKE_EXECUTABLE_SUFFIX "" )

endif( UNIX )


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
    ${SRC_DIR}/agg/src/agg_line_aa_basics.cpp
    ${SRC_DIR}/agg/src/agg_trans_affine.cpp
    ${SRC_DIR}/agg/src/agg_vcgen_contour.cpp
    ${SRC_DIR}/agg/src/agg_vcgen_stroke.cpp
    ${SRC_DIR}/agg/src/agg_bezier_arc.cpp
    ${SRC_DIR}/agg/src/agg_curves.cpp
    ${SRC_DIR}/agg/src/agg_gsv_text.cpp
    ${SRC_DIR}/agg/font_freetype/agg_font_freetype.cpp
)

set(DOCUMENT_FILES
    ${SRC_DIR}/document/lomse_document.cpp
    ${SRC_DIR}/document/lomse_document_iterator.cpp
    ${SRC_DIR}/document/lomse_id_assigner.cpp
)

set(EXPORTERS_FILES
    ${SRC_DIR}/exporters/lomse_ldp_exporter.cpp
)

set(GRAPHIC_MODEL_FILES
    ${SRC_DIR}/graphic_model/lomse_box_score_page.cpp
    ${SRC_DIR}/graphic_model/lomse_box_slice.cpp
    ${SRC_DIR}/graphic_model/lomse_box_slice_instr.cpp
    ${SRC_DIR}/graphic_model/lomse_box_system.cpp
    ${SRC_DIR}/graphic_model/lomse_clef_engraver.cpp
    ${SRC_DIR}/graphic_model/lomse_document_layouter.cpp
    ${SRC_DIR}/graphic_model/lomse_glyphs.cpp
    ${SRC_DIR}/graphic_model/lomse_gm_basic.cpp
    ${SRC_DIR}/graphic_model/lomse_instrument_engraver.cpp
    ${SRC_DIR}/graphic_model/lomse_score_layouter.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_base.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_brace_bracket.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_staff.cpp
    ${SRC_DIR}/graphic_model/lomse_shape_text.cpp
    ${SRC_DIR}/graphic_model/lomse_shapes.cpp
    ${SRC_DIR}/graphic_model/lomse_sizers.cpp
    ${SRC_DIR}/graphic_model/lomse_system_cursor.cpp
    ${SRC_DIR}/graphic_model/lomse_system_layouter.cpp
    ${SRC_DIR}/graphic_model/lomse_text_engraver.cpp
)

set(INTERNAL_MODEL_FILES
    ${SRC_DIR}/internal_model/lomse_basic_model.cpp
    ${SRC_DIR}/internal_model/lomse_basic_objects.cpp
    ${SRC_DIR}/internal_model/lomse_im_figured_bass.cpp
    ${SRC_DIR}/internal_model/lomse_im_note.cpp
    ${SRC_DIR}/internal_model/lomse_internal_model.cpp
)

set(MODULE_FILES
    ${SRC_DIR}/module/lomse_doorway.cpp
    ${SRC_DIR}/module/lomse_injectors.cpp
    ${SRC_DIR}/module/lomse_time.cpp
)

set(MVC_FILES
    ${SRC_DIR}/mvc/lomse_graphic_view.cpp
    ${SRC_DIR}/mvc/lomse_interactor.cpp
    ${SRC_DIR}/mvc/lomse_presenter.cpp 
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

if(WIN32)
    set(PLATFORM_FILES
        ${SRC_DIR}/agg/src/platform/win32/agg_win32_bmp.cpp
    )
elseif(UNIX)
    set(PLATFORM_FILES
        
    )
endif()

set(ALL_SOURCES 
    ${AGG_FILES} ${DOCUMENT_FILES} ${EXPORTERS_FILES}
    ${GRAPHIC_MODEL_FILES} ${INTERNAL_MODEL_FILES} 
    ${MODULE_FILES} ${MVC_FILES}
    ${PARSER_FILES} ${RENDER_FILES} ${SCORE_FILES} 
    ${PLATFORM_FILES}
)


# Adds folders for Visual Studio and other IDEs
source_group( "agg"             FILES ${AGG_FILES} ${PLATFORM_FILES} )
source_group( "document"        FILES ${DOCUMENT_FILES} )
source_group( "exporters"       FILES ${EXPORTERS_FILES} )
source_group( "graphic_model"   FILES ${GRAPHIC_MODEL_FILES} )
source_group( "internal_model"  FILES ${INTERNAL_MODEL_FILES} )
source_group( "module"          FILES ${MODULE_FILES} )
source_group( "mvc"             FILES ${MVC_FILES} )
source_group( "parser"          FILES ${PARSER_FILES} )
source_group( "render"          FILES ${RENDER_FILES} )
source_group( "score"           FILES ${SCORE_FILES} )



# library makefile generation. Static library
#-------------------------------------------------------------

set(LOMSE  lomse)

add_library( ${LOMSE} STATIC ${ALL_SOURCES} )


# hack to remove 'Debug' subdirectory from output path. -------------
# see: http://www.cmake.org/Bug/view.php?id=8243
if(MSVC)
    set_target_properties(${LOMSE} PROPERTIES  PREFIX "../")
endif()

# properties
# Force not to link with standard MSVC libraries: /NODEFAULTLIB
if(MSVC)
    set_target_properties(${LOMSE} PROPERTIES  LINK_FLAGS "/NODEFAULTLIB")
endif()

# once built, place library at lomse/bin
set_target_properties(${LOMSE}  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  ${ROOT_DIR}/bin )
    
get_target_property(VAR ${LOMSE} LINK_FLAGS)
message("library target properties=" ${VAR})


# library makefile generation. Dynamic library
#-------------------------------------------------------------
if(0)

    set(LOMSE_SH  lomse_dll)
    add_library( ${LOMSE_SH} SHARED ${ALL_SOURCES} )

    # The library target LOMSE_SH has a default OUTPUT_NAME of "lomse_dll", so change it.
    set_target_properties(${LOMSE_SH} PROPERTIES OUTPUT_NAME "liblomse")


    # hack to remove 'Debug' subdirectory from output path. --------------
    # see: http://www.cmake.org/Bug/view.php?id=8243
    if(MSVC)
        set_target_properties(${LOMSE}  PROPERTIES  PREFIX "../")
    endif()

endif(0)

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
        ${SRC_DIR}/tests/lomse_test_compiler.cpp
        ${SRC_DIR}/tests/lomse_test_document.cpp
        ${SRC_DIR}/tests/lomse_test_document_iterator.cpp
        ${SRC_DIR}/tests/lomse_test_document_layouter.cpp
        ${SRC_DIR}/tests/lomse_test_elements.cpp
        ${SRC_DIR}/tests/lomse_test_gm_objects.cpp
        ${SRC_DIR}/tests/lomse_test_graphic_model.cpp
        ${SRC_DIR}/tests/lomse_test_interactor.cpp
        ${SRC_DIR}/tests/lomse_test_internal_model.cpp
        ${SRC_DIR}/tests/lomse_test_model_builder.cpp
        ${SRC_DIR}/tests/lomse_test_parser.cpp
        ${SRC_DIR}/tests/lomse_test_presenter.cpp
        ${SRC_DIR}/tests/lomse_test_reader.cpp
        ${SRC_DIR}/tests/lomse_test_score_iterator.cpp
        ${SRC_DIR}/tests/lomse_test_score_layouter.cpp
        ${SRC_DIR}/tests/lomse_test_sizers.cpp
        ${SRC_DIR}/tests/lomse_test_stack.cpp
        ${SRC_DIR}/tests/lomse_test_staffobjs_table.cpp
        ${SRC_DIR}/tests/lomse_test_tokenizer.cpp
        ${SRC_DIR}/tests/lomse_test_tree.cpp
        ${SRC_DIR}/tests/lomse_test_view.cpp
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
        ${EXAMPLE1_SRC}/win32/example_1_win32.cpp
        ${EXAMPLE1_SRC}/win32/stdafx.cpp
        ${EXAMPLE1_SRC}/win32/resource.h
        ${EXAMPLE1_SRC}/win32/stdafx.h
        ${EXAMPLE1_SRC}/win32/small.ico
        ${EXAMPLE1_SRC}/win32/example_1.ico
        ${EXAMPLE1_SRC}/win32/example_1.rc
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
        ${EXAMPLE1_SRC}/x11/example_1_x11.cpp
    )

    target_link_libraries ( ${EXAMPLE1} liblomse.so )

        
endif()

# dependencies
add_dependencies(example_1 lomse)


#-----------------------------------------------------------------------------
# library installation
# You can change the install location by running cmake like this:
#
#   cmake -DCMAKE_INSTALL_PREFIX=/new/install/prefix
#
# By default, the prefix is "/usr/local"
#-----------------------------------------------------------------------------
# 
#if( UNIX AND NOT APPLE )
#    install( TARGETS ${LOMSE} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX} )
#endif()

## Set the path of the application executable
#if( MSVC )
#    set( EXE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/${TESTLIB}${CMAKE_EXECUTABLE_SUFFIX} )
#else()
#    set( EXE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTLIB}${CMAKE_EXECUTABLE_SUFFIX} )
#endif()


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
# 	Windows:
#   	Visual Studio 7 .NET
#   	Visual Studio 2008
# 	Linux:
#   	Unix Makefiles
#   	CodeBlocks - Unix Makefiles
#
#-------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8)

project(lomse)

# configuration
set(LOMSE_VERSION_MAJOR "0")
set(LOMSE_VERSION_MINOR "0")

# set directories 
set(ROOT_DIR  ${CMAKE_CURRENT_SOURCE_DIR} )
set(SRC_DIR  ${ROOT_DIR}/src )
set(OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/bin )
set(EXECUTABLE_OUTPUT_PATH ${OUTDIR})
set(LIBRARY_OUTPUT_PATH ${OUTDIR})
# path to scores for tests
set(LOMSE_TEST_SCORES_PATH "\"${ROOT_DIR}/test-scores/\"" )



# check that compiler supports namespace sdt
include(TestForSTDNamespace)
if(CMAKE_NO_STD_NAMESPACE)
  message(FATAL_ERROR "The compiler doesn't support namespace sdt.")
endif()




# set path to include unittest++ header files
if(WIN32)
	set(UNITTEST_LIBDIR  ${ROOT_DIR}/packages/UnitTest++/lib )
	set(UNITTEST_INCLUDE_DIRS  ${ROOT_DIR}/packages/UnitTest++/scr )
else()
	include(FindPkgConfig)
	pkg_check_modules(UNITTEST REQUIRED unittest++)
	#if found, it sets several variables, in particular:
	#message("version=" ${UNITTEST_VERSION} )
	#message("include_dirs=" ${UNITTEST_INCLUDE_DIRS} )
	#message("library_dirs=" "${UNITTEST_LIBRARY_DIRS}" )
	#message("libdir=" ${UNITTEST_LIBDIR} )
	#message("includedir=" ${UNITTEST_INCLUDEDIR} )
endif()
 
include_directories(
	${ROOT_DIR}/include
	${UNITTEST_INCLUDE_DIRS}
)

link_directories(
	${UNITTEST_LIBDIR}
	${LIBRARY_OUTPUT_PATH} 
)


# Define possible builds
set(CMAKE_CONFIGURATION_TYPES Debug Release)


# names for libraries and execs.
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


# compiler settings
#------------------------------------------

# "Print all warnings" flag for GCC
if(UNIX)
    add_definitions(-Wall -DGCC)
endif(UNIX)

# "Link time code generation" flags for MSVC
if(MSVC)
    add_definitions(
        /DUNICODE /D_UNICODE
        -DWINVER=0x0400 -DWIN32 
        -D_LOMSE_WIN32_VSTUDIO
    )
    if(${CMAKE_GENERATOR} STREQUAL "Visual Studio 6")
        add_definitions(-DVC6)
    elseif(${CMAKE_GENERATOR} STREQUAL "Visual Studio 8 2005")
        add_definitions(-DVC2005)
    endif(${CMAKE_GENERATOR} STREQUAL "Visual Studio 6")
endif(MSVC)

# some flags for MSVC
if(MSVC)
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG" )
endif()


#define a header file to pass CMake settings to source code
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/lomse_config.h.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/lomse_config.h"
)

include_directories("${PROJECT_BINARY_DIR}")




#////////////////////////////////////////////////////////////////////////
# Target: Lomse library
#////////////////////////////////////////////////////////////////////////


# source files to compile

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

set(DOCUMENT_FILES
    ${SRC_DIR}/document/lomse_document.cpp
    ${SRC_DIR}/document/lomse_id_assigner.cpp
)

set(EXPORTERS_FILES
    ${SRC_DIR}/exporters/lomse_ldp_exporter.cpp
)

set(SCORE_FILES
    ${SRC_DIR}/score/lomse_score_iterator.cpp
)

set(MODULE_FILES
    ${SRC_DIR}/module/lomse_injectors.cpp
    ${SRC_DIR}/module/lomse_time.cpp
)

set(INTERNAL_MODEL_FILES
    ${SRC_DIR}/internal_model/lomse_basic_model.cpp
    ${SRC_DIR}/internal_model/lomse_basic_objects.cpp
    ${SRC_DIR}/internal_model/lomse_im_figured_bass.cpp
    ${SRC_DIR}/internal_model/lomse_im_note.cpp
    ${SRC_DIR}/internal_model/lomse_internal_model.cpp
)

set(MVC_FILES
    ${SRC_DIR}/mvc/lomse_view.cpp
)

set(GRAPHIC_MODEL_FILES
    ${SRC_DIR}/graphic_model/lomse_document_layouter.cpp
    ${SRC_DIR}/graphic_model/lomse_gm_basic.cpp
    ${SRC_DIR}/graphic_model/lomse_score_layouter.cpp
)

set(ALL_SOURCES 
    ${PARSER_FILES} ${DOCUMENT_FILES} ${EXPORTERS_FILES} ${SCORE_FILES}
    ${MODULE_FILES} ${INTERNAL_MODEL_FILES} ${MVC_FILES}
    ${GRAPHIC_MODEL_FILES}
)


# Adds folders for Visual Studio and other IDEs
source_group( "document"        FILES ${DOCUMENT_FILES} )
source_group( "exporters"       FILES ${EXPORTERS_FILES} )
source_group( "graphic_model"   FILES ${GRAPHIC_MODEL_FILES} )
source_group( "internal_model"  FILES ${INTERNAL_MODEL_FILES} )
source_group( "module"          FILES ${MODULE_FILES} )
source_group( "mvc"             FILES ${MVC_FILES} )
source_group( "parser"          FILES ${PARSER_FILES} )
source_group( "score"           FILES ${SCORE_FILES} )



# library makefile generation. Static library
#-------------------------------------------------------------

set(LOMSE  lomse)
add_library( ${LOMSE} STATIC ${ALL_SOURCES} )


# hack to remove 'Debug' subdirectory from output path. -------------
# see: http://www.cmake.org/Bug/view.php?id=8243
if(MSVC)
    set_target_properties(${LOMSE}  PROPERTIES  PREFIX "../")
endif()

# once built, place library at lomse/bin
set_target_properties(${LOMSE}  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  ${ROOT_DIR}/bin )
    
    

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
# now I need to continue
if(WIN32)
	set(CMAKE_SHARED_LINKER_FLAGS
		 ${CMAKE_SHARED_LINKER_FLAGS} "/NODEFAULTLIB:LIBCD"
	)
endif(WIN32)


# source files to compile
add_executable( ${TESTLIB} 
        ${SRC_DIR}/tests/lomse_test_analyser.cpp
        ${SRC_DIR}/tests/lomse_test_colstaffobjs_builder.cpp
        ${SRC_DIR}/tests/lomse_test_compiler.cpp
        ${SRC_DIR}/tests/lomse_test_document.cpp
        ${SRC_DIR}/tests/lomse_test_document_layouter.cpp
        ${SRC_DIR}/tests/lomse_test_elements.cpp
        ${SRC_DIR}/tests/lomse_test_gm_objects.cpp
        ${SRC_DIR}/tests/lomse_test_graphic_model.cpp
        ${SRC_DIR}/tests/lomse_test_internal_model.cpp
        ${SRC_DIR}/tests/lomse_test_model_builder.cpp
        ${SRC_DIR}/tests/lomse_test_parser.cpp
        ${SRC_DIR}/tests/lomse_test_reader.cpp
        ${SRC_DIR}/tests/lomse_test_score_layouter.cpp
        ${SRC_DIR}/tests/lomse_test_stack.cpp
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
if(WIN32)
    set_target_properties(${TESTLIB} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBCD" )
    set_target_properties(${TESTLIB} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(${TESTLIB} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
    set_target_properties(${TESTLIB} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(${TESTLIB} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
endif(WIN32)


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


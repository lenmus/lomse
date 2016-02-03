#-------------------------------------------------------------------------------------
# This is part of CMake configuration file for building makefiles and installfiles
# for the Lomse library
#-------------------------------------------------------------------------------------
# This module is for getting information from Git repository and composing
# Lomse version string
#
# The following variables are defined (with example of value):
#   LOMSE_PACKAGE_VERSION   0.17.36
#   LOMSE_VERSION_MAJOR     0
#   LOMSE_VERSION_MINOR     17
#   LOMSE_VERSION_PATCH     36
#   LOMSE_VERSION_SHA1      a1b2c3f
#   LOMSE_BUILD_DATE        "2016-01-27 12:06:01 UTC"
#
#-------------------------------------------------------------------------------------

find_package (Git)
if (NOT GIT_FOUND)
    message(SEND_ERROR "Git package not found." )
endif()

# get description
execute_process(COMMAND "${GIT_EXECUTABLE}" describe --tags
	            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	            RESULT_VARIABLE res
	            OUTPUT_VARIABLE out
	            OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT res EQUAL 0)
    message("Error ${res} in 'git describe' command. Out: ${out}")
	set(DESCRIPTION "0.0-0-gERROR")
else()
    set(DESCRIPTION "${out}")
endif()
#set (DESCRIPTION "0.17.2-7-gabcdef2")
#set (DESCRIPTION "0.0-0-gERROR")
#set (DESCRIPTION "0.17-7-gabcdef2")
message(STATUS "git description is: ${DESCRIPTION}")


# extract components from description string
string(REGEX REPLACE "^([0-9]+)\\..*" "\\1" LOMSE_VERSION_MAJOR "${DESCRIPTION}")
string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1" LOMSE_VERSION_MINOR "${DESCRIPTION}")
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" LOMSE_VERSION_PATCH "${DESCRIPTION}")
if ("${LOMSE_VERSION_PATCH}" STREQUAL "${DESCRIPTION}")
    # ok. New tag format X.Y
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\-([0-9]+).*" "\\1" LOMSE_VERSION_PATCH "${DESCRIPTION}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\-[0-9]+\\-g(.*)" "\\1" LOMSE_VERSION_SHA1 "${DESCRIPTION}")
else()
    # Old tag format X.Y.Z. Ignore Z
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+\\-([0-9]+).*" "\\1" LOMSE_VERSION_PATCH "${DESCRIPTION}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+\\-[0-9]+\\-g(.*)" "\\1" LOMSE_VERSION_SHA1 "${DESCRIPTION}")
endif()
message(STATUS "Lomse version. Major:${LOMSE_VERSION_MAJOR}, Minor:${LOMSE_VERSION_MINOR}, Patch:${LOMSE_VERSION_PATCH}, SHA1:${LOMSE_VERSION_SHA1}")

#build version string for package name
set(LOMSE_PACKAGE_VERSION "${LOMSE_VERSION_MAJOR}.${LOMSE_VERSION_MINOR}.${LOMSE_VERSION_PATCH}" )
message (STATUS "Lomse package version = '${LOMSE_PACKAGE_VERSION}'") 

#set build date. format: "2016-01-27 12:06:01 UTC"
string(TIMESTAMP LOMSE_BUILD_DATE "%Y-%m-%d %H:%M:%S UTC" UTC)
message(STATUS "Lomse build date: '${LOMSE_BUILD_DATE}'")


if(0)  # No longer used. Kept for reference. Remove once commited
    # get revisions
    execute_process(COMMAND "${GIT_EXECUTABLE}" log --oneline
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/git-log-lines.txt)

    execute_process(COMMAND wc -l
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/git-log-lines.txt
                    OUTPUT_VARIABLE out2
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/git-log-lines.txt)

    set(LOMSE_VERSION_REVISIONS "${out2}")
    message(STATUS "Lomse revisions: ${LOMSE_VERSION_REVISIONS}")
endif(0)


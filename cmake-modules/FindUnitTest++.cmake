# - Try to find UnitTest++
#
# This module defines
# 	UNITTEST++_LIBRARY
# 	UNITTEST++_FOUND
# 	UNITTEST++_INCLUDE_DIR
#
find_path(UNITTEST++_INCLUDE_DIR 
	NAME
		UnitTest++.h
	PATHS
		/usr/local/include
		/usr/include
		/usr/include/unittest++
		/usr/include/UnitTest++ 		# Fedora
		/usr/include/unittest-cpp 		# openSUSE
		/usr/local/include/unittest++ 
		/usr/local/include/UnitTest++ 	# Arch
		/opt/local/include 				# DarwinPorts
		/opt/local/include/UnitTest++
		/opt/local/include/unittest++
		/opt/csw/include 				# Blastwave
		/opt/include
		/sw/include 					# Fink
		$ENV{UnitTest++_DIR}/include/UnitTest++						#Windows
		"C:/Program Files (x86)/UnitTest++/include/UnitTest++"		#Windows
		"C:/Program Files/UnitTest++/include/UnitTest++"		    #Windows
)

FIND_LIBRARY (UNITTEST++_LIBRARY
	NAMES
		UnitTest++
	PATHS 
		/usr/lib
		/usr/local/lib
		/usr/lib64/ 		# Fedora
		/sw/lib
		/opt/local/lib
		/opt/csw/lib
		/opt/lib
		$ENV{UnitTest++_DIR}/lib					#Windows
		"C:/Program Files (x86)/UnitTest++/lib"		#Windows
		"C:/Program Files/UnitTest++/lib"			#Windows
)

SET (UNITTEST++_FOUND FALSE)
IF (UNITTEST++_INCLUDE_DIR AND UNITTEST++_LIBRARY)
	SET (UNITTEST++_FOUND TRUE)
ENDIF ()

IF (UNITTEST++_FOUND)
   IF (NOT UnitTest++_FIND_QUIETLY)
      MESSAGE(STATUS "Found UnitTest++: ${UNITTEST++_LIBRARY}")
   ENDIF ()
ELSE (UNITTEST++_FOUND)
   IF (UnitTest++_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find UnitTest++")
   ENDIF ()
ENDIF ()

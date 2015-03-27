# pkg-config source file

libdir=@LOMSE_LIBDIR@
includedir=@LOMSE_INCLUDEDIR@

Name: liblomse
Description: LenMus open music score edition library
Version: @LOMSE_VERSION_STRING@
Requires: @LOMSE_REQUIRES@
Libs: -L${libdir} -llomse -lboost_system -lboost_thread -lboost_date_time
Cflags: -I${includedir}




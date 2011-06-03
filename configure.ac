#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ueioa], [0.01], [jrwilson@go.wustl.edu])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([lib/system.cpp])
AC_CONFIG_HEADERS([include/ioa/config.hpp])
AM_INIT_AUTOMAKE

#AC_ARG_ENABLE(camera, [AS_HELP_STRING([--enable-camera], [enable the camera])])
#AM_CONDITIONAL(WANT_CAMERA, test "$enable_camera" = yes)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lX11':
#AC_CHECK_LIB([X11], [main])
# FIXME: Replace `main' with a function in `-ljpeg':
#AC_CHECK_LIB([jpeg], [main])
AC_SEARCH_LIBS([pthread_key_create], [pthread])
AC_SEARCH_LIBS([uuid_clear], [uuid])
#AC_SEARCH_LIBS([MD5], [crypto])
#AC_SEARCH_LIBS([json_object_new_object], [json])

# Checks for header files.
#AC_PATH_X
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getpagesize gettimeofday memset select socket strerror])

AC_CONFIG_FILES([Makefile
		 doc/Makefile
		 lib/Makefile
		 examples/Makefile])
AC_OUTPUT

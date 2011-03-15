#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ueioa], [0.01], [jrwilson@go.wustl.edu])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

AC_ARG_ENABLE(camera, [AS_HELP_STRING([--enable-camera], [enable the camera])])
AM_CONDITIONAL(WANT_CAMERA, test "$enable_camera" = yes)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lX11':
#AC_CHECK_LIB([X11], [main])
AC_SEARCH_LIBS([MD5], [crypto])
# FIXME: Replace `main' with a function in `-ljpeg':
#AC_CHECK_LIB([jpeg], [main])
AC_SEARCH_LIBS([pthread_self], [pthread])
AC_SEARCH_LIBS([uuid_clear], [uuid])

# Checks for header files.
AC_PATH_X
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
		 libtable/Makefile
                 libueioa/Makefile
		 testueioa/Makefile
		 libautoman/Makefile
		 testautoman/Makefile
		 libmftp/Makefile
		 libccm/Makefile
		 testccm/Makefile
		 mftp/Makefile
                 src/Makefile])
AC_OUTPUT

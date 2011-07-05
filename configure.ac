#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ioa], [0.01], [jrwilson@go.wustl.edu])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([lib/model.cpp])
AC_CONFIG_HEADERS([config.hpp])
#AX_PREFIX_CONFIG_H([include/ioa/conf.hpp], [ioa], [config.hpp])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_LANG([C++])
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
AC_SEARCH_LIBS([pthread_key_create], [pthread])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdint.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T

# Checks for library functions.
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([gettimeofday memset select socket])

AC_CONFIG_FILES([Makefile
		 include/Makefile
		 lib/Makefile
		 test/Makefile
		 doc/Makefile
		 tutorial/Makefile
		 examples/Makefile])
AC_OUTPUT

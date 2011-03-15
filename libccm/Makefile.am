AM_CFLAGS = -Wall -I$(top_srcdir)/include

lib_LTLIBRARIES = libccm.la
libccm_la_SOURCES = \
bitset.c \
component.c \
port.c \
port_allocator.c

libccm_la_LIBADD = $(top_builddir)/libautoman/libautoman.la $(top_builddir)/libmftp/libmftp.la

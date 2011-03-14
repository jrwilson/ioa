AM_CFLAGS = -Wall -I$(top_srcdir)/include
AM_LDFLAGS = -lpthread -ljson -luuid -lcrypto

TESTS = \
component_request_port

check_PROGRAMS = $(TESTS)

component_request_port_SOURCES = component_request_port.c integer_reflector.c integer_reflector_proxy.c
component_request_port_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libautoman/libautoman.la $(top_builddir)/libmftp/libmftp.la $(top_builddir)/libccm/libccm.la

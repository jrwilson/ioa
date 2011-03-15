AM_CFLAGS = -Wall -I$(top_srcdir)/include
AM_LDFLAGS = -ljson

LDADD = $(top_builddir)/libccm/libccm.la

TESTS = \
component_request_port

check_PROGRAMS = $(TESTS)

component_request_port_SOURCES = component_request_port.c integer_reflector.c integer_reflector_proxy.c


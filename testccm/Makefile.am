AM_CFLAGS = -Wall -I$(top_srcdir)/include
AM_LDFLAGS = -ljson

LDADD = $(top_builddir)/libccm/libccm.la

TESTS = \
component_request_port_instance \
component_request_port_instance_oob

check_PROGRAMS = $(TESTS)

component_request_port_instance_SOURCES = component_request_port_instance.c integer_reflector.c integer_reflector_proxy.c

component_request_port_instance_oob_SOURCES = component_request_port_instance_oob.c integer_reflector.c integer_reflector_proxy.c


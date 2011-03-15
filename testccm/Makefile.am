AM_CFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/libccm/libccm.la

TESTS = \
component_request_port_instance \
component_request_port_instance_dne \
component_request_port_instance_unavailable

check_PROGRAMS = $(TESTS)

component_request_port_instance_SOURCES = component_request_port_instance.c integer_reflector.c integer_reflector_proxy.c

component_request_port_instance_dne_SOURCES = component_request_port_instance_dne.c integer_reflector.c integer_reflector_proxy.c

component_request_port_instance_unavailable_SOURCES = component_request_port_instance_unavailable.c integer_reflector.c integer_reflector_proxy.c


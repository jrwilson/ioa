AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

TESTS = \
action \
binding

check_PROGRAMS = $(TESTS)

action_SOURCES = action.cpp test_main.cpp

binding_SOURCES = binding.cpp test_main.cpp

# AM_LDFLAGS = -lpthread -ljson
# component_request_port_SOURCES = component_request_port.c
# component_request_port_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libautoman/libautoman.la $(top_builddir)/libmftp/libmftp.la $(top_builddir)/libccm/libccm.la

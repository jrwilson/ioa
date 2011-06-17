AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/lib/libueioa.la

TESTS = \
action


# model \
# simple_scheduler \
# bind_helper

check_PROGRAMS = $(TESTS)

action_SOURCES = action.cpp test_main.cpp

# model_SOURCES = model.cpp test_main.cpp test_system_scheduler.cpp

# simple_scheduler_SOURCES = simple_scheduler.cpp test_main.cpp

# bind_helper_SOURCES = bind_helper.cpp test_main.cpp


# AM_LDFLAGS = -lpthread -ljson
# component_request_port_SOURCES = component_request_port.c
# component_request_port_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libautoman/libautoman.la $(top_builddir)/libmftp/libmftp.la $(top_builddir)/libccm/libccm.la

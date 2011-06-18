AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/lib/libueioa.la

TESTS = \
action_executor \
model \
simple_scheduler \
bind_helper

check_PROGRAMS = $(TESTS)

action_executor_SOURCES = action_executor.cpp test_main.cpp

model_SOURCES = model.cpp test_main.cpp

simple_scheduler_SOURCES = simple_scheduler.cpp test_main.cpp

# TODO:  Write test for self_helper.
# TODO:  Write test for automaton_helper.

# TODO:  Beef up this test with execution and unbinding.
bind_helper_SOURCES = bind_helper.cpp test_main.cpp

# TODO:  Incorporate configuration.cpp

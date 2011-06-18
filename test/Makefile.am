AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/lib/libioa.la

TESTS = \
action_executor \
model \
simple_scheduler \
bind_helper

check_PROGRAMS = $(TESTS)

action_executor_SOURCES = minunit.h automaton1.hpp action_executor.cpp test_main.cpp

model_SOURCES = minunit.h instance_holder.hpp automaton1.hpp model.cpp test_main.cpp

simple_scheduler_SOURCES = minunit.h automaton2.hpp simple_scheduler.cpp test_main.cpp

# TODO:  Write test for self_helper.
# TODO:  Write test for automaton_helper.

# TODO:  Beef up this test with execution and unbinding.
bind_helper_SOURCES = minunit.h automaton2.hpp bind_helper.cpp test_main.cpp

# TODO:  Incorporate configuration.cpp

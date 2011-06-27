AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/lib/libioa.la

TESTS = \
time \
action_executor \
model \
global_fifo_scheduler \
simple_scheduler \
binding_manager

check_PROGRAMS = $(TESTS)

time_SOURCES = minunit.h time.cpp test_main.cpp

action_executor_SOURCES = minunit.h automaton1.hpp action_executor.cpp test_main.cpp

model_SOURCES = minunit.h instance_holder.hpp automaton1.hpp model.cpp test_main.cpp

global_fifo_scheduler_SOURCES = minunit.h automaton2.hpp global_fifo_scheduler.cpp scheduler_test.hpp test_main.cpp

simple_scheduler_SOURCES = minunit.h automaton2.hpp simple_scheduler.cpp scheduler_test.hpp test_main.cpp

# TODO:  Write test for self_helper.
# TODO:  Write test for automaton_helper.

# TODO:  Beef up this test with execution and unbinding.
binding_manager_SOURCES = minunit.h automaton2.hpp binding_manager.cpp test_main.cpp

# TODO:  Incorporate configuration.cpp

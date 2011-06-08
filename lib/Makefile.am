AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

lib_LTLIBRARIES = libueioa.la libueioa_simple_scheduler.la

libueioa_la_SOURCES = \
automaton_interface.cpp \
automaton_record.cpp \
condition_variable.cpp \
lock.cpp \
mutex.cpp \
observer.cpp \
runnable_interface.cpp \
shared_lock.cpp \
shared_mutex.cpp \
system.cpp \
thread.cpp \
time.cpp \
unique_lock.cpp

libueioa_simple_scheduler_la_SOURCES = \
simple_scheduler.cpp

# libueioa_la_LIBADD = $(top_builddir)/libtable/libtable.la

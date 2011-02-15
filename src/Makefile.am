AM_CFLAGS = -Wall -I$(top_srcdir)/include

bin_PROGRAMS = main thread_test

main_SOURCES = main.c composer.c trigger.c counter.c
main_LDADD = $(top_builddir)/lib/libueioa.la

thread_test_SOURCES = thread_test.c
thread_test_LDADD = $(top_builddir)/lib/libueioa.la

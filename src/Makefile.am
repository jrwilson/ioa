AM_CFLAGS = -Wall -I$(top_srcdir)/include

bin_PROGRAMS = main thread_test display

main_SOURCES = main.c composer.c trigger.c counter.c
main_LDADD = $(top_builddir)/lib/libueioa.la
main_LDFLAGS = -lpthread

thread_test_SOURCES = thread_test.c
thread_test_LDADD = $(top_builddir)/lib/libueioa.la
thread_test_LDFLAGS = -lpthread

display_SOURCES = display_driver.c display.c
display_LDADD = $(top_builddir)/lib/libueioa.la
display_LDFLAGS = -L/usr/X11R6/lib -lX11 -lpthread

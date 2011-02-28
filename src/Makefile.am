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
display_LDFLAGS = -lX11 -lpthread -ljpeg

if WANT_CAMERA
	bin_PROGRAMS += camera camera_display

	camera_SOURCES = camera_driver.c camera.c
	camera_LDADD = $(top_builddir)/lib/libueioa.la
	camera_LDFLAGS = -lpthread

	camera_display_SOURCES = camera_display.c camera.c display.c
	camera_display_LDADD = $(top_builddir)/lib/libueioa.la
	camera_display_LDFLAGS = -lX11 -lpthread -ljpeg
endif



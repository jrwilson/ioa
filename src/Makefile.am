AM_CFLAGS = -Wall -I$(top_srcdir)/include

bin_PROGRAMS = main thread_test display ramp

main_SOURCES = main.c composer.c trigger.c counter.c
main_LDADD = $(top_builddir)/lib/libueioa.la
main_LDFLAGS = -lpthread

thread_test_SOURCES = thread_test.c
thread_test_LDADD = $(top_builddir)/lib/libueioa.la
thread_test_LDFLAGS = -lpthread

display_SOURCES = display_driver.c display.c
display_LDADD = $(top_builddir)/lib/libueioa.la
display_LDFLAGS = -lX11 -lpthread -ljpeg

ramp_SOURCES = component_manager.c bitset.c port_allocator.c port.c channel.c ramp_driver.c ramp.c integer_display.c ramp_proxy.c integer_display_proxy.c
ramp_LDADD = $(top_builddir)/lib/libueioa.la $(top_builddir)/libmftp/libmftp.la
ramp_LDFLAGS = -lpthread -ljson -lcrypto

if WANT_CAMERA
	bin_PROGRAMS += camera camera_display

	camera_SOURCES = camera_driver.c camera.c
	camera_LDADD = $(top_builddir)/lib/libueioa.la
	camera_LDFLAGS = -lpthread

	camera_display_SOURCES = camera_display.c camera.c display.c
	camera_display_LDADD = $(top_builddir)/lib/libueioa.la
	camera_display_LDFLAGS = -lX11 -lpthread -ljpeg
endif



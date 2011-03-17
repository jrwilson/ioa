AM_CFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/libautoman/libautoman.la $(top_builddir)/libccm/libccm.la 

bin_PROGRAMS = main thread_test ramp

main_SOURCES = main.c composer.c trigger.c counter.c

thread_test_SOURCES = thread_test.c

ramp_SOURCES = ramp_driver.c ramp.c ramp_proxy.c

# integer_display.c integer_display_proxy.c component_manager.c bitset.c port_allocator.c port.c channel.c

# display_SOURCES = display_driver.c display.c
# display_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libautoman/libautoman.la
# display_LDFLAGS = -lX11 -lpthread -ljpeg

# if WANT_CAMERA
# 	bin_PROGRAMS += camera camera_display

# 	camera_SOURCES = camera_driver.c camera.c
# 	camera_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la
# 	camera_LDFLAGS = -lpthread

# 	camera_display_SOURCES = camera_display.c camera.c display.c
# 	camera_display_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la
# 	camera_display_LDFLAGS = -lX11 -lpthread -ljpeg
# endif



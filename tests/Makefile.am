AM_CFLAGS = -Wall -I$(top_srcdir)/include

TESTS = bad_order self_created child_created bad_descriptor

check_PROGRAMS = bad_order self_created child_created bad_descriptor

bad_order_SOURCES = bad_order.c
bad_order_LDADD = $(top_builddir)/lib/libueioa.la

self_created_SOURCES = self_created.c
self_created_LDADD = $(top_builddir)/lib/libueioa.la

child_created_SOURCES = child_created.c
child_created_LDADD = $(top_builddir)/lib/libueioa.la

bad_descriptor_SOURCES = bad_descriptor.c
bad_descriptor_LDADD = $(top_builddir)/lib/libueioa.la

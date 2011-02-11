AM_CFLAGS = -Wall -I$(top_srcdir)/include

TESTS = \
bad_order \
self_created \
child_created \
bad_descriptor \
declared \
rescinded \
automaton_dne \
not_owner \
child_destroyed

check_PROGRAMS = $(TESTS)

bad_order_SOURCES = bad_order.c
bad_order_LDADD = $(top_builddir)/lib/libueioa.la

self_created_SOURCES = self_created.c
self_created_LDADD = $(top_builddir)/lib/libueioa.la

child_created_SOURCES = child_created.c
child_created_LDADD = $(top_builddir)/lib/libueioa.la

bad_descriptor_SOURCES = bad_descriptor.c
bad_descriptor_LDADD = $(top_builddir)/lib/libueioa.la

declared_SOURCES = declared.c
declared_LDADD = $(top_builddir)/lib/libueioa.la

rescinded_SOURCES = rescinded.c
rescinded_LDADD = $(top_builddir)/lib/libueioa.la

automaton_dne_SOURCES = automaton_dne.c
automaton_dne_LDADD = $(top_builddir)/lib/libueioa.la

not_owner_SOURCES = not_owner.c
not_owner_LDADD = $(top_builddir)/lib/libueioa.la

child_destroyed_SOURCES = child_destroyed.c
child_destroyed_LDADD = $(top_builddir)/lib/libueioa.la

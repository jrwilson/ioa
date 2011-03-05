AM_CFLAGS = -Wall -I$(top_srcdir)/include
AM_LDFLAGS = -lpthread

TESTS = \
automan_create \
automan_child_create \
automan_param_declare \
automan_child_create_param

check_PROGRAMS = $(TESTS)

automan_create_SOURCES = automan_create.c
automan_create_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libautoman/libautoman.la

automan_child_create_SOURCES = automan_child_create.c
automan_child_create_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libautoman/libautoman.la

automan_param_declare_SOURCES = automan_param_declare.c
automan_param_declare_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libautoman/libautoman.la

automan_child_create_param_SOURCES = automan_child_create_param.c
automan_child_create_param_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libautoman/libautoman.la

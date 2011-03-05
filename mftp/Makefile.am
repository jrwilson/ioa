AM_CFLAGS = -Wall -I$(top_srcdir)/include

bin_PROGRAMS = main share match get

main_SOURCES = main.c composer.c consumer.c
main_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libmftp/libmftp.la
main_LDFLAGS = -lcrypto -lpthread

share_SOURCES = share.c matcher.c
share_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libmftp/libmftp.la
share_LDFLAGS = -lcrypto -lpthread

match_SOURCES = match.c matcher.c
match_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libmftp/libmftp.la
match_LDFLAGS = -lcrypto -lpthread

get_SOURCES = get.c matcher.c match_getter.c
get_LDADD = $(top_builddir)/libtable/libtable.la $(top_builddir)/libueioa/libueioa.la $(top_builddir)/libmftp/libmftp.la
get_LDFLAGS = -lcrypto -lpthread

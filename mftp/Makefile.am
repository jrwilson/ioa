AM_CFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/libmftp/libmftp.la

bin_PROGRAMS = main share match get

main_SOURCES = main.c composer.c consumer.c

share_SOURCES = share.c matcher.c

match_SOURCES = match.c matcher.c

get_SOURCES = get.c matcher.c match_getter.c

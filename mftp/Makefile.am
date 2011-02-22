AM_CFLAGS = -Wall -I$(top_srcdir)/include

bin_PROGRAMS = main share

main_SOURCES = main.c composer.c consumer.c
main_LDADD = $(top_builddir)/lib/libueioa.la $(top_builddir)/libmftp/libmftp.la
main_LDFLAGS = -lcrypto

share_SOURCES = share.c
share_LDADD = $(top_builddir)/lib/libueioa.la $(top_builddir)/libmftp/libmftp.la
share_LDFLAGS = -lcrypto

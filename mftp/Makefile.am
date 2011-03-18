AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/libmftp/libmftp.la

bin_PROGRAMS = main share match get

main_SOURCES = main.cc composer.cc consumer.cc

share_SOURCES = share.cc matcher.cc

match_SOURCES = match.cc matcher.cc

get_SOURCES = get.cc matcher.cc match_getter.cc

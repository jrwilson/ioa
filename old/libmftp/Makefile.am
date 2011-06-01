AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

lib_LTLIBRARIES = libmftp.la
libmftp_la_SOURCES = \
alarm.cc \
bitset.cc \
file.cc \
fileid.cc \
file_server.cc \
message.cc \
msg_receiver.cc \
msg_sender.cc \
msg_sender_proxy.cc \
udp_receiver.cc \
udp_sender.cc

libmftp_la_LIBADD = $(top_builddir)/libautoman/libautoman.la
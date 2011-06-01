AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/lib/libueioa.la -luuid

noinst_PROGRAMS = \
asynch_lcr \
clock_213 \
count_to_ten \
file_reader \
peterson_leader \
slow_count

asynch_lcr_SOURCES = asynch_lcr.cpp
clock_213_SOURCES = clock_213.cpp
count_to_ten_SOURCES = count_to_ten.cpp
file_reader_SOURCES = file_reader.cpp
peterson_leader_SOURCES = peterson_leader.cpp
slow_count_SOURCES = slow_count.cpp

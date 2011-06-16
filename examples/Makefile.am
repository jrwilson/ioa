AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/lib/libueioa.la

noinst_PROGRAMS = \
asynch_lcr \
beast \
clock_213 \
count_to_ten \
periodic_broadcaster \
peterson_leader \
slow_count \
spanning_tree \
file_tester

asynch_lcr_SOURCES = asynch_lcr.cpp
beast_SOURCES = beast.cpp
clock_213_SOURCES = clock_213.cpp
count_to_ten_SOURCES = count_to_ten.cpp
periodic_broadcaster_SOURCES = periodic_broadcaster.cpp
peterson_leader_SOURCES = peterson_leader.cpp
slow_count_SOURCES = slow_count.cpp
spanning_tree_SOURCES = spanning_tree.cpp
file_tester_SOURCES = file_tester.cpp file.cpp
file_tester_LDADD = -lcrypto

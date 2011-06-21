AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/lib/libioa.la

noinst_PROGRAMS = \
asynch_bfs \
asynch_lcr \
beast \
bellman_ford \
clock_213 \
count_to_ten \
mftp \
null_automaton \
peterson_leader \
slow_count \
spanning_tree

#file_tester
#periodic_broadcaster

asynch_bfs_SOURCES = channel_automaton.hpp bidirectional_network.hpp bfs_automaton.hpp asynch_bfs.cpp
asynch_lcr_SOURCES = uuid.hpp unidirectional_ring_leader_election.hpp asynch_lcr_automaton.hpp asynch_lcr.cpp
beast_SOURCES = aba_automaton.hpp beast.cpp
bellman_ford_SOURCES = weighted_bidirectional_network.hpp bf_automaton.hpp bellman_ford.cpp
clock_213_SOURCES = clock_213.cpp
count_to_ten_SOURCES = count_to_ten.cpp
mftp_SOURCES = mftp.cpp
null_automaton_SOURCES = null_automaton.cpp
peterson_leader_SOURCES = peterson_leader_automaton.hpp peterson_leader.cpp
slow_count_SOURCES = slow_count.cpp
spanning_tree_SOURCES = bidirectional_spanning_tree.hpp ast_automaton.hpp spanning_tree.cpp

#file_tester_SOURCES = file_tester.cpp file.cpp
#periodic_broadcaster_SOURCES = periodic_broadcaster.cpp


AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/lib/libioa.la

noinst_PROGRAMS = \
asynch_bfs \
asynch_lcr \
beast \
bellman_ford \
clock_213 \
echo_client \
echo_server \
peterson_leader \
slow_count \
spanning_tree \
udp_sender \
udp_receiver

asynch_bfs_SOURCES = channel_automaton.hpp bidirectional_network.hpp bfs_automaton.hpp asynch_bfs.cpp
asynch_lcr_SOURCES = unidirectional_ring_leader_election.hpp asynch_lcr_automaton.hpp asynch_lcr.cpp
beast_SOURCES = aba_automaton.hpp beast.cpp
bellman_ford_SOURCES = weighted_bidirectional_network.hpp bf_automaton.hpp bellman_ford.cpp
clock_213_SOURCES = clock_213.cpp
echo_client_SOURCES = echo_client.cpp
echo_server_SOURCES = echo_server.cpp
peterson_leader_SOURCES = peterson_leader_automaton.hpp peterson_leader.cpp
slow_count_SOURCES = slow_count.cpp
spanning_tree_SOURCES = bidirectional_spanning_tree.hpp ast_automaton.hpp spanning_tree.cpp
udp_receiver_SOURCES = udp_receiver.cpp
udp_sender_SOURCES = udp_sender.cpp

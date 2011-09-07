
AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/lib/libioa.la

noinst_PROGRAMS = \
clock \
asynch_lcr \
peterson_leader \
asynch_spanning_tree \
asynch_bcast_ack \
asynch_bfs \
asynch_bellman_ford \
slow_count \
udp_sender \
udp_receiver \
echo_client \
echo_server \
tcp_lcr

# Examples from Distributed Algorithms
clock_SOURCES = clock.cpp
asynch_lcr_SOURCES = UID.hpp unidirectional_ring_leader_election.hpp asynch_lcr_automaton.hpp asynch_lcr.cpp
peterson_leader_SOURCES = peterson_leader_automaton.hpp peterson_leader.cpp
asynch_spanning_tree_SOURCES = bidirectional_spanning_tree.hpp asynch_spanning_tree_automaton.hpp asynch_spanning_tree.cpp
asynch_bcast_ack_SOURCES = asynch_bcast_ack_automaton.hpp asynch_bcast_ack.cpp
asynch_bfs_SOURCES = channel_automaton.hpp bidirectional_network.hpp asynch_bfs_automaton.hpp asynch_bfs.cpp
asynch_bellman_ford_SOURCES = weighted_bidirectional_network.hpp asynch_bellman_ford_automaton.hpp asynch_bellman_ford.cpp

slow_count_SOURCES = slow_count.cpp
udp_receiver_SOURCES = udp_receiver.cpp
udp_sender_SOURCES = udp_sender.cpp
echo_client_SOURCES = echo_client.cpp
echo_server_SOURCES = echo_server.cpp
tcp_lcr_SOURCES = tcp_lcr.cpp

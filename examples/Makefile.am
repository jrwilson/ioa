
AM_CXXFLAGS = -Wall -I$(top_srcdir)/include

LDADD = $(top_builddir)/lib/libioa.la

noinst_PROGRAMS = \
asynch_bfs \
asynch_lcr \
basic_client \
basic_server \
beast \
bellman_ford \
clock_213 \
echo_client \
<<<<<<< HEAD
echo_server \
mftp \
=======
>>>>>>> cc00d0b9df09416eefe18232a92f809afbbf41ea
peterson_leader \
slow_count \
spanning_tree \
udp_sender \
udp_receiver

#file_tester
#mftp

asynch_bfs_SOURCES = channel_automaton.hpp bidirectional_network.hpp bfs_automaton.hpp asynch_bfs.cpp
asynch_lcr_SOURCES = uuid.hpp unidirectional_ring_leader_election.hpp asynch_lcr_automaton.hpp asynch_lcr.cpp
basic_client_SOURCES = basic_client.cpp file.cpp udp_receiver_automaton.hpp mftp_automaton.hpp file.hpp mftp.hpp sha2_256.hpp
basic_server_SOURCES = basic_server.cpp file.cpp udp_sender_automaton.hpp mftp_automaton.hpp file.hpp mftp.hpp sha2_256.hpp
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

#file_tester_SOURCES = file_tester.cpp file.cpp
#mftp_SOURCES = file.cpp mftp.cpp simple_network.hpp checking_channel_automaton.hpp mftp_automaton.hpp file.hpp mftp.hpp sha2_256.hpp


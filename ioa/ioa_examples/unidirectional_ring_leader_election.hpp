#ifndef __unidirectional_ring_leader_election_hpp__
#define __unidirectional_ring_leader_election_hpp__

#include "channel_automaton.hpp"
#include <iostream>

template <class T, size_t N>
class unidirectional_ring_leader_election :
  public ioa::dispatching_automaton
{
private:

  UV_P_INPUT (unidirectional_ring_leader_election, leader, size_t, i) {
    std::cout << i << " is the leader." << std::endl;
  }

  // typedef ioa::instance_generator<T> T_generator_type;
  // typedef ioa::instance_generator<channel_automaton<uuid> > channel_automaton_generator_type;

  typedef ioa::self_helper<unidirectional_ring_leader_election> unidirectional_ring_leader_election_helper_type;
  typedef ioa::automaton_helper<unidirectional_ring_leader_election, T> T_helper_type;
  typedef ioa::automaton_helper<unidirectional_ring_leader_election, channel_automaton<uuid> > channel_automaton_helper_type;

  typedef ioa::bind_helper<unidirectional_ring_leader_election, T_helper_type, typename T::send_type, channel_automaton_helper_type, channel_automaton<uuid>::send_type> send_bind_helper_type;
  typedef ioa::bind_helper<unidirectional_ring_leader_election, channel_automaton_helper_type, channel_automaton<uuid>::receive_type, T_helper_type, typename T::receive_type> receive_bind_helper_type;
  typedef ioa::bind_helper<unidirectional_ring_leader_election, T_helper_type, typename T::leader_type, unidirectional_ring_leader_election_helper_type, typename unidirectional_ring_leader_election::leader_type> leader_bind_helper_type;

  unidirectional_ring_leader_election_helper_type* unidirectional_ring_leader_election_helper;
  std::vector<T_helper_type*> T_helpers;
  std::vector<channel_automaton_helper_type*> channel_automaton_helpers;
  std::vector<send_bind_helper_type*> send_bind_helpers;
  std::vector<receive_bind_helper_type*> receive_bind_helpers;
  std::vector<leader_bind_helper_type*> leader_bind_helpers;
  
public:

  unidirectional_ring_leader_election () :
    ACTION (unidirectional_ring_leader_election, leader)
  { }

  void init () {

    unidirectional_ring_leader_election_helper = new unidirectional_ring_leader_election_helper_type (this);

    for (size_t i = 0; i < N; ++i) {
      T_helpers.push_back (new T_helper_type (this, ioa::make_instance_generator<T> ()));
      channel_automaton_helpers.push_back (new channel_automaton_helper_type (this, ioa::make_instance_generator<channel_automaton<uuid> > ()));
    }

    for (size_t i = 0; i < N; ++i) {
      send_bind_helpers.push_back (new send_bind_helper_type (this,
      							      T_helpers[i],
      							      &T::send,
      							      channel_automaton_helpers[i],
      							      &channel_automaton<uuid>::send));
      receive_bind_helpers.push_back (new receive_bind_helper_type (this,
      								    channel_automaton_helpers[i],
      								    &channel_automaton<uuid>::receive,
      								    T_helpers[(i + 1) % N],
      								    &T::receive));
      leader_bind_helpers.push_back (new leader_bind_helper_type (this,
								  T_helpers[i],
								  &T::leader,
								  unidirectional_ring_leader_election_helper,
								  &unidirectional_ring_leader_election::leader,
								  i));
    }

  }

};

#endif

#ifndef __unidirectional_ring_leader_election_hpp__
#define __unidirectional_ring_leader_election_hpp__

#include "channel_automaton.hpp"
#include <iostream>

template <class T, size_t N>
class unidirectional_ring_leader_election :
  public ioa::automaton
{
private:

  void schedule () const { }

  void leader_effect (size_t i) {
    std::cout << i << " is the leader." << std::endl;
  }

  UV_P_INPUT (unidirectional_ring_leader_election, leader, size_t);

  ioa::self_manager<unidirectional_ring_leader_election> self;
  std::vector<ioa::automaton_handle_interface<T>*> T_helpers;
  std::vector<ioa::automaton_handle_interface<channel_automaton<uuid> >*> channel_automaton_managers;

public:

  unidirectional_ring_leader_election ()
  { 
    for (size_t i = 0; i < N; ++i) {
      T_helpers.push_back (new ioa::automaton_manager<T> (this, ioa::make_generator<T> ()));
      channel_automaton_managers.push_back (new ioa::automaton_manager<channel_automaton<uuid> > (this, ioa::make_generator<channel_automaton<uuid> > ()));
    }
    
    for (size_t i = 0; i < N; ++i) {
      make_bind_helper (this, T_helpers[i], &T::send, channel_automaton_managers[i], &channel_automaton<uuid>::send);
      make_bind_helper (this, channel_automaton_managers[i], &channel_automaton<uuid>::receive, T_helpers[(i + 1) % N], &T::receive);
      make_bind_helper (this, T_helpers[i], &T::leader, &self, &unidirectional_ring_leader_election::leader, i);
    }

  }

};

#endif

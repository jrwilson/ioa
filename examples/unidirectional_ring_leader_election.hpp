#ifndef __unidirectional_ring_leader_election_hpp__
#define __unidirectional_ring_leader_election_hpp__

#include "channel_automaton.hpp"
#include <iostream>

template <class T, size_t N>
class unidirectional_ring_leader_election :
  public ioa::automaton_interface
{
private:
  
  void leader_action (size_t i) {
    std::cout << i << " is the leader." << std::endl;
  }

  UV_P_INPUT (unidirectional_ring_leader_election, leader, size_t);

  std::auto_ptr<ioa::self_helper<unidirectional_ring_leader_election> > self;
  std::vector<ioa::automaton_handle_interface<T>*> T_helpers;
  std::vector<ioa::automaton_handle_interface<channel_automaton<uuid> >*> channel_automaton_helpers;

public:

  unidirectional_ring_leader_election () :
    self (new ioa::self_helper<unidirectional_ring_leader_election> ())
  { 
    for (size_t i = 0; i < N; ++i) {
      T_helpers.push_back (new ioa::automaton_helper<T> (this, ioa::make_generator<T> ()));
      channel_automaton_helpers.push_back (new ioa::automaton_helper<channel_automaton<uuid> > (this, ioa::make_generator<channel_automaton<uuid> > ()));
    }
    
    for (size_t i = 0; i < N; ++i) {
      make_bind_helper (this, T_helpers[i], &T::send, channel_automaton_helpers[i], &channel_automaton<uuid>::send);
      make_bind_helper (this, channel_automaton_helpers[i], &channel_automaton<uuid>::receive, T_helpers[(i + 1) % N], &T::receive);
      make_bind_helper (this, T_helpers[i], &T::leader, self.get (), &unidirectional_ring_leader_election::leader, i);
    }

  }

};

#endif

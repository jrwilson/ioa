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

  void leader_schedule (size_t) const {
    schedule ();
  }

  UV_P_INPUT (unidirectional_ring_leader_election, leader, size_t);

  ioa::handle_manager<unidirectional_ring_leader_election> self;
  std::vector<ioa::automaton_handle_interface<T>*> T_helpers;
  std::vector<ioa::automaton_handle_interface<channel_automaton<ioa::aid_t> >*> channel_automaton_managers;

public:

  unidirectional_ring_leader_election () :
    self (ioa::get_aid ())
  { 
    for (size_t i = 0; i < N; ++i) {
      T_helpers.push_back (new ioa::automaton_manager<T> (this, ioa::make_generator<T> ()));
      channel_automaton_managers.push_back (new ioa::automaton_manager<channel_automaton<ioa::aid_t> > (this, ioa::make_generator<channel_automaton<ioa::aid_t> > ()));
    }
    
    for (size_t i = 0; i < N; ++i) {
      make_binding_manager (this, T_helpers[i], &T::send, channel_automaton_managers[i], &channel_automaton<ioa::aid_t>::send);
      make_binding_manager (this, channel_automaton_managers[i], &channel_automaton<ioa::aid_t>::receive, T_helpers[(i + 1) % N], &T::receive);
      make_binding_manager (this, T_helpers[i], &T::leader, &self, &unidirectional_ring_leader_election::leader, i);
    }

  }

};

#endif

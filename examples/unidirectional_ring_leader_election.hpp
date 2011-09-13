#ifndef __unidirectional_ring_leader_election_hpp__
#define __unidirectional_ring_leader_election_hpp__

#include "UID.hpp"
#include "channel_automaton.hpp"
#include <iostream>

template <class T>
class unidirectional_ring_leader_election :
  public ioa::automaton
{
private:
  ioa::handle_manager<unidirectional_ring_leader_election> m_self;

public:
  unidirectional_ring_leader_election (const size_t N) :
    m_self (ioa::get_aid ())
  { 
    std::vector<ioa::automaton_manager<T>*> T_managers;
    std::vector<ioa::automaton_manager<channel_automaton<UID_t> >*> channel_automaton_managers;
    
    for (size_t i = 0; i < N; ++i) {
      UID_t u (rand (), i);
      std::cout << u.first << "\t" << u.second << std::endl;
      T_managers.push_back (ioa::make_automaton_manager (this, ioa::make_generator<T> (u)));
      channel_automaton_managers.push_back (ioa::make_automaton_manager (this, ioa::make_generator<channel_automaton<UID_t> > ()));
    }
    
    for (size_t i = 0; i < N; ++i) {
      make_binding_manager (this, T_managers[i], &T::send, channel_automaton_managers[i], &channel_automaton<UID_t>::send);
      make_binding_manager (this, channel_automaton_managers[i], &channel_automaton<UID_t>::receive, T_managers[(i + 1) % N], &T::receive);
      make_binding_manager (this, T_managers[i], &T::leader, &m_self, &unidirectional_ring_leader_election::leader, i);
    }
  }

private:
  void leader_effect (const bool& v, size_t i) {
    if (v) {
      std::cout << i << " is the leader." << std::endl;
    }
  }

  void leader_schedule (size_t) const { }

  V_P_INPUT (unidirectional_ring_leader_election, leader, bool, size_t);
};

#endif

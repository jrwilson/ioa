#ifndef __asynch_lcr_automaton_hpp__
#define __asynch_lcr_automaton_hpp__

#include <ioa.hpp>
#include "uuid.hpp"
#include <queue>

/*
  AsyncLCR Automaton
  Distributed Algorithms, p. 477.
*/

class asynch_lcr_automaton :
  public ioa::dispatching_automaton
{
private:
  typedef enum {
    UNKNOWN,
    CHOSEN,
    REPORTED
  } status_t;

  uuid m_u;
  std::queue<uuid> m_send;
  status_t m_status;

  V_UP_INPUT (asynch_lcr_automaton, receive, uuid, v) {

    if (v > m_u) {
      m_send.push (v);
    }
    else if (v == m_u) {
      m_status = CHOSEN;
    }
    else {
      // Do nothing.
    }
    
    schedule ();
  }

  bool send_precondition () const {
    return !m_send.empty () && send.is_bound ();
  }

  V_UP_OUTPUT (asynch_lcr_automaton, send, uuid) {

    std::pair<bool, uuid> retval;

    if (send_precondition ()) {
      retval = std::make_pair (true, m_send.front ());
      m_send.pop ();
    }

    schedule ();
    return retval;
  }

  bool leader_precondition () const {
    return m_status == CHOSEN && leader.is_bound ();
  }

  UV_UP_OUTPUT (asynch_lcr_automaton, leader) {
    bool retval = false;

    if (leader_precondition ()) {
      retval = true;
      m_status = REPORTED;
    }

    schedule ();
    return retval;
  }

  void schedule () {
    if (send_precondition ()) {
      ioa::scheduler::schedule (this, &asynch_lcr_automaton::send);
    }

    if (leader_precondition ()) {
      ioa::scheduler::schedule (this, &asynch_lcr_automaton::leader);
    }
  }

public:

  asynch_lcr_automaton () :
    m_u (true),
    m_status (UNKNOWN),
    ACTION (asynch_lcr_automaton, receive),
    ACTION (asynch_lcr_automaton, send),
    ACTION (asynch_lcr_automaton, leader)
  {
    m_send.push (m_u);
  }

  void init () {
    schedule ();
  }

};

#endif

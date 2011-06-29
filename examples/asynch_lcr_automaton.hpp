#ifndef __asynch_lcr_automaton_hpp__
#define __asynch_lcr_automaton_hpp__

#include <ioa/ioa.hpp>
#include "uuid.hpp"
#include <queue>

/*
  AsyncLCR Automaton
  Distributed Algorithms, p. 477.
*/

class asynch_lcr_automaton :
  public ioa::automaton
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

  void receive_effect (const uuid& v) {
    if (v > m_u) {
      m_send.push (v);
    }
    else if (v == m_u) {
      m_status = CHOSEN;
    }
    else {
      // Do nothing.
    }
  }

  bool send_precondition () const {
    return !m_send.empty () && ioa::binding_count (&asynch_lcr_automaton::send) != 0;
  }

  uuid send_effect () {
    uuid retval = m_send.front ();
    m_send.pop ();
    return retval;
  }

  bool leader_precondition () const {
    return m_status == CHOSEN && ioa::binding_count (&asynch_lcr_automaton::leader) != 0;
  }

  void leader_effect () {
    m_status = REPORTED;
  }

  void schedule () const {
    if (send_precondition ()) {
      ioa::schedule (&asynch_lcr_automaton::send);
    }

    if (leader_precondition ()) {
      ioa::schedule (&asynch_lcr_automaton::leader);
    }
  }

public:

  asynch_lcr_automaton () :
    m_u (true),
    m_status (UNKNOWN)
  {
    m_send.push (m_u);
    schedule ();
  }

  V_UP_INPUT (asynch_lcr_automaton, receive, uuid);
  V_UP_OUTPUT (asynch_lcr_automaton, send, uuid);
  UV_UP_OUTPUT (asynch_lcr_automaton, leader);

};

#endif

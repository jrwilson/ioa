#ifndef __asynch_lcr_automaton_hpp__
#define __asynch_lcr_automaton_hpp__

/*
  AsyncLCR Automaton
  Distributed Algorithms, p. 477.
*/

#include "UID.hpp"
#include <ioa/ioa.hpp>
#include <queue>
#include <iostream>

class asynch_lcr_automaton :
  public ioa::automaton
{
private:
  typedef enum {
    UNKNOWN,
    CHOSEN,
    REPORTED
  } status_t;

  UID_t m_u;
  std::queue<UID_t> m_send;
  status_t m_status;

public:
  asynch_lcr_automaton (const size_t i) :
    m_u (rand (), i),
    m_status (UNKNOWN)
  {
    std::cout << m_u.first << "\t" << m_u.second << std::endl;
    m_send.push (m_u);
    schedule ();
  }

private:
  void schedule () const {
    if (send_precondition ()) {
      ioa::schedule (&asynch_lcr_automaton::send);
    }

    if (leader_precondition ()) {
      ioa::schedule (&asynch_lcr_automaton::leader);
    }
  }

  bool send_precondition () const {
    return !m_send.empty () && ioa::binding_count (&asynch_lcr_automaton::send) != 0;
  }

  UID_t send_effect () {
    UID_t retval = m_send.front ();
    m_send.pop ();
    return retval;
  }

  void send_schedule () const {
    schedule ();
  }

public:
  V_UP_OUTPUT (asynch_lcr_automaton, send, UID_t);

private:
  void receive_effect (const UID_t& v) {
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

  void receive_schedule () const {
    schedule ();
  }

public:
  V_UP_INPUT (asynch_lcr_automaton, receive, UID_t);

private:
  bool leader_precondition () const {
    return m_status == CHOSEN && ioa::binding_count (&asynch_lcr_automaton::leader) != 0;
  }

  void leader_effect () {
    m_status = REPORTED;
  }

  void leader_schedule () const {
    schedule ();
  }

public:
  UV_UP_OUTPUT (asynch_lcr_automaton, leader);
};

#endif

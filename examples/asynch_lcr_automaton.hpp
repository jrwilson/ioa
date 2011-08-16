#ifndef __asynch_lcr_automaton_hpp__
#define __asynch_lcr_automaton_hpp__

#include <ioa/ioa.hpp>
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

  ioa::aid_t m_u;
  std::queue<ioa::aid_t> m_send;
  status_t m_status;

public:
  asynch_lcr_automaton () :
    m_u (ioa::get_aid ()),
    m_status (UNKNOWN)
  {
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

  ioa::aid_t send_effect () {
    ioa::aid_t retval = m_send.front ();
    m_send.pop ();
    return retval;
  }

  void send_schedule () const {
    schedule ();
  }

public:
  V_UP_OUTPUT (asynch_lcr_automaton, send, ioa::aid_t);

private:
  void receive_effect (const ioa::aid_t& v) {
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
  V_UP_INPUT (asynch_lcr_automaton, receive, ioa::aid_t);

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

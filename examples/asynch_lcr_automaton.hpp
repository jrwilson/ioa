#ifndef __asynch_lcr_automaton_hpp__
#define __asynch_lcr_automaton_hpp__

/*
  AsyncLCR Automaton
  Distributed Algorithms, p. 477.
*/

#include <ioa/ioa.hpp>
#include <queue>

template <typename UID>
class asynch_lcr_automaton :
  public virtual ioa::automaton
{
private:
  const UID m_u;
  std::queue<UID> m_send;
  bool m_report;
  bool m_leader;

public:
  asynch_lcr_automaton (const UID& u) :
    m_u (u),
    m_report (false),
    m_leader (false)
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
    return !m_send.empty () &&
      ioa::binding_count (&asynch_lcr_automaton::send) != 0;
  }

  UID send_effect () {
    UID retval = m_send.front ();
    m_send.pop ();
    return retval;
  }

  void send_schedule () const {
    schedule ();
  }

public:
  V_UP_OUTPUT (asynch_lcr_automaton, send, UID);

private:
  void receive_effect (const UID& v) {
    if (v > m_u) {
      m_send.push (v);
      if (m_leader) {
	m_leader = false;
	m_report = true;
      }
    }
    else if (v == m_u) {
      if (!m_leader) {
	m_leader = true;
	m_report = true;
      }
    }
    else {
      // Do nothing.
    }
  }

  void receive_schedule () const {
    schedule ();
  }

public:
  V_UP_INPUT (asynch_lcr_automaton, receive, UID);

private:
  bool leader_precondition () const {
    return m_report && ioa::binding_count (&asynch_lcr_automaton::leader) != 0;
  }

  bool leader_effect () {
    m_report = false;
    return m_leader;
  }

  void leader_schedule () const {
    schedule ();
  }

public:
  V_UP_OUTPUT (asynch_lcr_automaton, leader, bool);

private:
  void init_effect () {
    m_report = false;
    m_leader = false;
    m_send.push (m_u);
  }

  void init_schedule () const {
    schedule ();
  }

public:
  UV_UP_INPUT (asynch_lcr_automaton, init);

};

#endif

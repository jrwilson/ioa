#ifndef __peterson_leader_automaton_hpp__
#define __peterson_leader_automaton_hpp__

#include <ioa.hpp>
#include "uuid.hpp"
#include <queue>

/*
  PetersonLeader Automaton
  Distributed Algorithms, p. 483-484.
*/

class peterson_leader_automaton :
  public ioa::dispatching_automaton
{
private:
  typedef enum {
    ACTIVE,
    RELAY
  } mode_t;

  typedef enum {
    UNKNOWN,
    CHOSEN,
    REPORTED
  } status_t;

  mode_t m_mode;
  status_t m_status;
  uuid m_u;
  uuid m_uid[3];

  std::queue<uuid> m_send;
  std::queue<uuid> m_receive;

  V_UP_INPUT (peterson_leader_automaton, receive, uuid, v) {
    m_receive.push (v);
    
    schedule ();
  }

  bool send_precondition () const {
    return !m_send.empty () && send.is_bound ();
  }

  V_UP_OUTPUT (peterson_leader_automaton, send, uuid) {
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

  UV_UP_OUTPUT (peterson_leader_automaton, leader) {
    bool retval = false;

    if (leader_precondition ()) {
      retval = true;
      m_status = REPORTED;
    }

    schedule ();
    return retval;
  }

  bool get_second_uid_precondition () const {
    return m_mode == ACTIVE && !m_receive.empty () && m_uid[1].is_null ();
  }

  UP_INTERNAL (peterson_leader_automaton, get_second_uid) {
    if (get_second_uid_precondition ()) {
      m_uid[1] = m_receive.front ();
      m_receive.pop ();
      m_send.push (m_uid[1]);
      if (m_uid[1] == m_uid[0]) {
	m_status = CHOSEN;
      }
    }

    schedule ();
  }

  bool get_third_uid_precondition () const {
    return m_mode == ACTIVE && !m_receive.empty () && !m_uid[1].is_null () && m_uid[2].is_null ();
  }

  UP_INTERNAL (peterson_leader_automaton, get_third_uid) {
    if (get_third_uid_precondition ()) {
      m_uid[2] = m_receive.front ();
      m_receive.pop ();
    }

    schedule ();
  }

  bool advance_phase_precondition () const {
    return m_mode == ACTIVE && !m_uid[2].is_null () && m_uid[1] > std::max (m_uid[0], m_uid[2]);
  }

  UP_INTERNAL (peterson_leader_automaton, advance_phase) {

    if (advance_phase_precondition ()) {
      m_uid[0] = m_uid[1];
      m_uid[1].clear ();
      m_uid[2].clear ();
      m_send.push (m_uid[0]);
    }

    schedule ();
  }

  bool become_relay_precondition () const {
    return m_mode == ACTIVE && !m_uid[2].is_null () && m_uid[1] <= std::max (m_uid[0], m_uid[2]);
  }

  UP_INTERNAL (peterson_leader_automaton, become_relay) {
    if (become_relay_precondition ()) {
      m_mode = RELAY;
    }

    schedule ();
  }

  bool relay_precondition () const {
    return m_mode == RELAY && !m_receive.empty ();
  }

  UP_INTERNAL (peterson_leader_automaton, relay) {
    if (relay_precondition ()) {
      m_send.push (m_receive.front ());
      m_receive.pop ();
    }

    schedule ();
  }

  void schedule () {
    if (send_precondition ()) {
      ioa::scheduler::schedule (this, &peterson_leader_automaton::send);
    }

    if (leader_precondition ()) {
      ioa::scheduler::schedule (this, &peterson_leader_automaton::leader);
    }

    if (get_second_uid_precondition ()) {
      ioa::scheduler::schedule (this, &peterson_leader_automaton::get_second_uid);
    }

    if (get_third_uid_precondition ()) {
      ioa::scheduler::schedule (this, &peterson_leader_automaton::get_third_uid);
    }

    if (advance_phase_precondition ()) {
      ioa::scheduler::schedule (this, &peterson_leader_automaton::advance_phase);
    }

    if (become_relay_precondition ()) {
      ioa::scheduler::schedule (this, &peterson_leader_automaton::become_relay);
    }

    if (relay_precondition ()) {
      ioa::scheduler::schedule (this, &peterson_leader_automaton::relay);
    }
  }

public:

  peterson_leader_automaton () :
    m_mode (ACTIVE),
    m_status (UNKNOWN),
    m_u (true),
    ACTION (peterson_leader_automaton, receive),
    ACTION (peterson_leader_automaton, send),
    ACTION (peterson_leader_automaton, leader),
    ACTION (peterson_leader_automaton, get_second_uid),
    ACTION (peterson_leader_automaton, get_third_uid),
    ACTION (peterson_leader_automaton, advance_phase),
    ACTION (peterson_leader_automaton, become_relay),
    ACTION (peterson_leader_automaton, relay)
  {
    m_uid[0] = m_u;
    m_send.push (m_u);
  }

  void init () {
    schedule ();
  }

};

#endif

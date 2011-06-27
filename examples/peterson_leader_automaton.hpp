#ifndef __peterson_leader_automaton_hpp__
#define __peterson_leader_automaton_hpp__

#include <ioa/ioa.hpp>
#include "uuid.hpp"
#include <queue>

/*
  PetersonLeader Automaton
  Distributed Algorithms, p. 483-484.
*/

class peterson_leader_automaton :
  public ioa::automaton
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

  void schedule () const {
    if (send_precondition ()) {
      ioa::schedule (&peterson_leader_automaton::send);
    }

    if (leader_precondition ()) {
      ioa::schedule (&peterson_leader_automaton::leader);
    }

    if (get_second_uid_precondition ()) {
      ioa::schedule (&peterson_leader_automaton::get_second_uid);
    }

    if (get_third_uid_precondition ()) {
      ioa::schedule (&peterson_leader_automaton::get_third_uid);
    }

    if (advance_phase_precondition ()) {
      ioa::schedule (&peterson_leader_automaton::advance_phase);
    }

    if (become_relay_precondition ()) {
      ioa::schedule (&peterson_leader_automaton::become_relay);
    }

    if (relay_precondition ()) {
      ioa::schedule (&peterson_leader_automaton::relay);
    }
  }

  void receive_effect (const uuid& v) {
    m_receive.push (v);
  }

  bool send_precondition () const {
    return !m_send.empty () && ioa::bind_count (&peterson_leader_automaton::send) != 0;
  }

  uuid send_effect () {
    uuid retval = m_send.front ();
    m_send.pop ();
    return retval;
  }

  bool leader_precondition () const {
    return m_status == CHOSEN && ioa::bind_count (&peterson_leader_automaton::leader) != 0;
  }

  void leader_effect () {
    m_status = REPORTED;
  }

  bool get_second_uid_precondition () const {
    return m_mode == ACTIVE && !m_receive.empty () && m_uid[1].is_null ();
  }

  void get_second_uid_effect () {
    m_uid[1] = m_receive.front ();
    m_receive.pop ();
    m_send.push (m_uid[1]);
    if (m_uid[1] == m_uid[0]) {
      m_status = CHOSEN;
    }
  }
  
  UP_INTERNAL (peterson_leader_automaton, get_second_uid);

  bool get_third_uid_precondition () const {
    return m_mode == ACTIVE && !m_receive.empty () && !m_uid[1].is_null () && m_uid[2].is_null ();
  }

  void get_third_uid_effect () {
    m_uid[2] = m_receive.front ();
    m_receive.pop ();
  }

  UP_INTERNAL (peterson_leader_automaton, get_third_uid);

  bool advance_phase_precondition () const {
    return m_mode == ACTIVE && !m_uid[2].is_null () && m_uid[1] > std::max (m_uid[0], m_uid[2]);
  }

  void advance_phase_effect () {
    m_uid[0] = m_uid[1];
    m_uid[1].clear ();
    m_uid[2].clear ();
    m_send.push (m_uid[0]);
  }

  UP_INTERNAL (peterson_leader_automaton, advance_phase);

  bool become_relay_precondition () const {
    return m_mode == ACTIVE && !m_uid[2].is_null () && m_uid[1] <= std::max (m_uid[0], m_uid[2]);
  }

  void become_relay_effect () {
    m_mode = RELAY;
  }

  UP_INTERNAL (peterson_leader_automaton, become_relay);

  bool relay_precondition () const {
    return m_mode == RELAY && !m_receive.empty ();
  }

  void relay_effect () {
    m_send.push (m_receive.front ());
    m_receive.pop ();
  }

  UP_INTERNAL (peterson_leader_automaton, relay);

public:

  peterson_leader_automaton () :
    m_mode (ACTIVE),
    m_status (UNKNOWN),
    m_u (true)
  {
    m_uid[0] = m_u;
    m_send.push (m_u);
    schedule ();
  }

  V_UP_INPUT (peterson_leader_automaton, receive, uuid);
  V_UP_OUTPUT (peterson_leader_automaton, send, uuid);
  UV_UP_OUTPUT (peterson_leader_automaton, leader);

};

#endif

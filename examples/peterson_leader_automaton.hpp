#ifndef __peterson_leader_automaton_hpp__
#define __peterson_leader_automaton_hpp__

/*
  PetersonLeader Automaton
  Distributed Algorithms, p. 483-484.
*/

#include <ioa/ioa.hpp>
#include <queue>

template <typename UID>
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
  UID m_u;
  UID m_uid[3];

  std::queue<UID> m_send;
  std::queue<UID> m_receive;

public:
  peterson_leader_automaton (const UID& u) :
    m_mode (ACTIVE),
    m_status (UNKNOWN),
    m_u (u)
  {
    m_uid[0] = m_u;
    m_uid[1] = UID ();
    m_uid[2] = UID ();
    m_send.push (m_u);
    schedule ();
  }

private:
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

private:
  bool send_precondition () const {
    return !m_send.empty () && ioa::binding_count (&peterson_leader_automaton::send) != 0;
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
  V_UP_OUTPUT (peterson_leader_automaton, send, UID);

private:
  void receive_effect (const UID& v) {
    m_receive.push (v);
  }

  void receive_schedule () const {
    schedule ();
  }

public:
  V_UP_INPUT (peterson_leader_automaton, receive, UID);

private:
  bool leader_precondition () const {
    return m_status == CHOSEN && ioa::binding_count (&peterson_leader_automaton::leader) != 0;
  }

  bool leader_effect () {
    m_status = REPORTED;
    return true;
  }

  void leader_schedule () const {
    schedule ();
  }

public:
  V_UP_OUTPUT (peterson_leader_automaton, leader, bool);

private:
  bool get_second_uid_precondition () const {
    return m_mode == ACTIVE && !m_receive.empty () && m_uid[1] == UID ();
  }

  void get_second_uid_effect () {
    m_uid[1] = m_receive.front ();
    m_receive.pop ();
    m_send.push (m_uid[1]);
    if (m_uid[1] == m_uid[0]) {
      m_status = CHOSEN;
    }
  }
  
  void get_second_uid_schedule () const {
    schedule ();
  }

  UP_INTERNAL (peterson_leader_automaton, get_second_uid);

  bool get_third_uid_precondition () const {
    return m_mode == ACTIVE && !m_receive.empty () && m_uid[1] != UID () && m_uid[2] == UID ();
  }

  void get_third_uid_effect () {
    m_uid[2] = m_receive.front ();
    m_receive.pop ();
  }

  void get_third_uid_schedule () const {
    schedule ();
  }

  UP_INTERNAL (peterson_leader_automaton, get_third_uid);

  bool advance_phase_precondition () const {
    return m_mode == ACTIVE && m_uid[2] != UID () && m_uid[1] > std::max (m_uid[0], m_uid[2]);
  }

  void advance_phase_effect () {
    m_uid[0] = m_uid[1];
    m_uid[1] = UID ();
    m_uid[2] = UID ();
    m_send.push (m_uid[0]);
  }

  void advance_phase_schedule () const {
    schedule ();
  }

  UP_INTERNAL (peterson_leader_automaton, advance_phase);

  bool become_relay_precondition () const {
    return m_mode == ACTIVE && m_uid[2] != UID () && m_uid[1] <= std::max (m_uid[0], m_uid[2]);
  }

  void become_relay_effect () {
    m_mode = RELAY;
  }

  void become_relay_schedule () const {
    schedule ();
  }

  UP_INTERNAL (peterson_leader_automaton, become_relay);

  bool relay_precondition () const {
    return m_mode == RELAY && !m_receive.empty ();
  }

  void relay_effect () {
    m_send.push (m_receive.front ());
    m_receive.pop ();
  }

  void relay_schedule () const {
    schedule ();
  }

  UP_INTERNAL (peterson_leader_automaton, relay);
};

#endif

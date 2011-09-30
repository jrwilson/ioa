/*
   Copyright 2011 Justin R. Wilson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <ioa/ioa.hpp>
#include <ioa/tcp_acceptor_automaton.hpp>
#include <ioa/tcp_connector_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>
#include "asynch_lcr_automaton.hpp"
#include "tcp_ring_automaton.hpp"

#include <uuid/uuid.h>

// In seconds.
#define ELECT_TIMEOUT 10
#define TOKEN_TIMEOUT 1

struct uuid
{
  uuid_t id;

  uuid () {
    uuid_clear (id);
  }

  uuid (const uuid& other) {
    uuid_copy (id, other.id);
  }

  void generate () {
    uuid_generate (id);
  }

  bool operator> (const uuid& other) const {
    return uuid_compare (id, other.id) > 0;
  }

  bool operator== (const uuid& other) const {
    return uuid_compare (id, other.id) == 0;
  }

  bool operator!= (const uuid& other) const {
    return uuid_compare (id, other.id) != 0;
  }
};

enum message_type_t {
  UNKNOWN,
  ELECT,
  TOKEN,
};

struct message_t
{
  message_type_t type;
  uuid id;

  message_t () :
    type (UNKNOWN)
  { }

  message_t (const message_type_t t,
	     const uuid& i) :
    type (t),
    id (i)
  { }
};

std::ostream& operator<< (std::ostream& o,
			  const uuid& id) {
  char c[37];
  uuid_unparse (id.id, c);
  return (o << c);
}

class tcp_lcr_automaton :
  public ioa::automaton
{
private:
  ioa::handle_manager<tcp_lcr_automaton> m_self;
  uuid m_u;
  std::queue<uuid> m_to_lcr;
  std::queue<message_t> m_to_ring;
  bool m_init_lcr;
  ioa::time m_last_token;
  bool m_leader;
  std::string m_recv;

public:
  tcp_lcr_automaton (const ioa::inet_address& recv_address,
		     const ioa::inet_address& send_address) :
    m_self (ioa::get_aid ())
  {
    // Generate an id.
    m_u.generate ();
    std::cout << "UUID: " << m_u << std::endl;

    // Create an LCR automaton and bind to it.
    ioa::automaton_manager<asynch_lcr_automaton<uuid> >* lcr = ioa::make_automaton_manager (this, ioa::make_generator<asynch_lcr_automaton<uuid> > (m_u));
    ioa::make_binding_manager (this,
			       lcr, &asynch_lcr_automaton<uuid>::send,
			       &m_self, &tcp_lcr_automaton::send_lcr);
    ioa::make_binding_manager (this,
			       &m_self, &tcp_lcr_automaton::receive_lcr,
			       lcr, &asynch_lcr_automaton<uuid>::receive);
    ioa::make_binding_manager (this,
			       lcr, &asynch_lcr_automaton<uuid>::leader,
			       &m_self, &tcp_lcr_automaton::leader_lcr);
    ioa::make_binding_manager (this,
			       &m_self, &tcp_lcr_automaton::init_lcr,
			       lcr, &asynch_lcr_automaton<uuid>::init);

    // Create a TCP ring automaton and bind to it.
    ioa::automaton_manager<tcp_ring_automaton>* ring = ioa::make_automaton_manager (this, ioa::make_generator<tcp_ring_automaton> (recv_address, send_address));
    ioa::make_binding_manager (this,
			       ring, &tcp_ring_automaton::receive,
			       &m_self, &tcp_lcr_automaton::receive_ring);
    ioa::make_binding_manager (this,
			       &m_self, &tcp_lcr_automaton::send_ring,
			       ring, &tcp_ring_automaton::send);

    schedule ();
    // Schedule timeouts.
    ioa::schedule (&tcp_lcr_automaton::elect);
    ioa::schedule (&tcp_lcr_automaton::token);
  }

private:
  void schedule () const {
    if (receive_lcr_precondition ()) {
      ioa::schedule (&tcp_lcr_automaton::receive_lcr);
    }
    if (init_lcr_precondition ()) {
      ioa::schedule (&tcp_lcr_automaton::init_lcr);
    }
    if (send_ring_precondition ()) {
      ioa::schedule (&tcp_lcr_automaton::send_ring);
    }
  }

  // Cause the LCR automaton to receive a message.
  bool receive_lcr_precondition () const {
    return !m_to_lcr.empty () && ioa::binding_count (&tcp_lcr_automaton::receive_lcr) != 0;
  }

  uuid receive_lcr_effect () {
    uuid m = m_to_lcr.front ();
    m_to_lcr.pop ();
    return m;
  }

  void receive_lcr_schedule () const {
    schedule ();
  }

  V_UP_OUTPUT (tcp_lcr_automaton, receive_lcr, uuid);

  // Forward messages from the LCR automaton.
  void send_lcr_effect (const uuid& u) {
    m_to_ring.push (message_t (ELECT, u));
  }

  void send_lcr_schedule () const {
    schedule ();
  }

  V_UP_INPUT (tcp_lcr_automaton, send_lcr, uuid);

  // Leader update.
  void leader_lcr_effect (const bool& v) {
    m_leader = v;    
    if (m_leader) {
      std::cout << "LEADER" << std::endl;
    }
  }

  void leader_lcr_schedule () const {
    schedule ();
  }

  V_UP_INPUT (tcp_lcr_automaton, leader_lcr, bool);

  // Re-initialize the LCR automaton.
  bool init_lcr_precondition () const {
    return m_init_lcr && ioa::binding_count (&tcp_lcr_automaton::init_lcr) != 0;
  }

  void init_lcr_effect () {
    m_init_lcr = false;
  }

  void init_lcr_schedule () const {
    schedule ();
  }

  UV_UP_OUTPUT (tcp_lcr_automaton, init_lcr);

  // Receive from the ring.
  void receive_ring_effect (const std::string& val) {
    m_recv += val;
    size_t offset = 0;
    while (m_recv.size () - offset >= sizeof (message_t)) {
      message_t m;
      m_recv.copy (reinterpret_cast<char *> (&m), sizeof (message_t), offset);
      offset += sizeof (message_t);
      switch (m.type) {
      case UNKNOWN:
	// Do nothing.
	break;
      case TOKEN:
	// Update the time and forward.
	m_last_token = ioa::time::now ();
	std::cout << "Token from " << m.id << std::endl;
	if (m.id != m_u) {
	  m_to_ring.push (m);
	}
	// Fall through.
      case ELECT:
	// Always give the id to the LCR automaton.
	m_to_lcr.push (m.id);
	break;
      }
    }
    m_recv = m_recv.substr (offset);
  }

  void receive_ring_schedule () const {
    schedule ();
  }

  V_UP_INPUT (tcp_lcr_automaton, receive_ring, std::string);

  // Send to the ring.
  bool send_ring_precondition () const {
    return !m_to_ring.empty () && ioa::binding_count (&tcp_lcr_automaton::send_ring) != 0;
  }

  std::string send_ring_effect () {
    std::string retval (reinterpret_cast<const char *> (&m_to_ring.front ()), sizeof (message_t));
    m_to_ring.pop ();
    return retval;
  }

  void send_ring_schedule () const {
    schedule ();
  }

  V_UP_OUTPUT (tcp_lcr_automaton, send_ring, std::string);

  // Elect a leader if we haven't seen an id in ELECT_TIMEOUT seconds.
  bool elect_precondition () const {
    return true;
  }

  void elect_effect () {
    if (ioa::time::now () > m_last_token + ioa::time (ELECT_TIMEOUT, 0)) {
      std::cout << "Calling for an election" << std::endl;
      m_leader = false;
      m_init_lcr = true;
    }
  }

  void elect_schedule () const {
    ioa::schedule_after (&tcp_lcr_automaton::elect, ioa::time (ELECT_TIMEOUT, 0));
    schedule ();
  }

  UP_INTERNAL (tcp_lcr_automaton, elect);

  // The leader circulates a token every TOKEN_TIMEOUT.
  bool token_precondition () const {
    return true;
  }

  void token_effect () {
    if (m_leader) {
      m_to_ring.push (message_t (TOKEN, m_u));
    }
  }

  void token_schedule () const {
    ioa::schedule_after (&tcp_lcr_automaton::token, ioa::time (TOKEN_TIMEOUT, 0));
    schedule ();
  }

  UP_INTERNAL (tcp_lcr_automaton, token);

};

int main (int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0] << " RECV_ADDR RECV_PORT SEND_ADDR SEND_PORT" << std::endl;
    exit (EXIT_FAILURE);
  }

  ioa::inet_address recv_address (argv[1], atoi (argv[2]));
  ioa::inet_address send_address (std::string (argv[3]), atoi (argv[4]));

  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<tcp_lcr_automaton> (recv_address, send_address));
  return 0;
}

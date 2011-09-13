#include <ioa/ioa.hpp>
#include <ioa/tcp_acceptor_automaton.hpp>
#include <ioa/tcp_connector_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>
#include "asynch_lcr_automaton.hpp"

#include <uuid/uuid.h>
#include <iostream>

// In seconds.
#define CONNECTION_TIMEOUT 10
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
  public ioa::automaton,
  private ioa::observer
{
private:
  ioa::handle_manager<tcp_lcr_automaton> m_self;
  uuid m_u;
  std::queue<uuid> m_to_lcr;
  bool m_init_lcr;
  ioa::time m_last_token;
  bool m_leader;
  const ioa::inet_address m_recv_address;
  const ioa::inet_address m_send_address;
  ioa::automaton_manager<ioa::tcp_acceptor_automaton>* m_acceptor;
  ioa::automaton_manager<ioa::tcp_connection_automaton>* m_predecessor;
  bool m_accept_flag;
  ioa::automaton_manager<ioa::tcp_connection_automaton>* m_successor;
  ioa::automaton_manager<ioa::tcp_connector_automaton>* m_connector;
  bool m_successor_connected;
  bool m_clear_to_send;
  std::queue<message_t> m_send;
  std::string m_recv;

public:
  tcp_lcr_automaton (const ioa::inet_address& recv_address,
		     const ioa::inet_address& send_address) :
    m_self (ioa::get_aid ()),
    m_recv_address (recv_address),
    m_send_address (send_address)
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

    schedule ();
    // Schedule timeouts.
    ioa::schedule (&tcp_lcr_automaton::elect);
    ioa::schedule (&tcp_lcr_automaton::token);
  }

private:
  void observe (ioa::observable*) {
    // Something related to our connections changed.
    schedule ();
  }

  void schedule () const {
    if (receive_lcr_precondition ()) {
      ioa::schedule (&tcp_lcr_automaton::receive_lcr);
    }
    if (init_lcr_precondition ()) {
      ioa::schedule (&tcp_lcr_automaton::init_lcr);
    }
    if (create_acceptor_precondition ()) {
      ioa::schedule (&tcp_lcr_automaton::create_acceptor);
    }
    if (create_predecessor_precondition ()) {
      ioa::schedule (&tcp_lcr_automaton::create_predecessor);
    }
    if (create_successor_precondition ()) {
      ioa::schedule (&tcp_lcr_automaton::create_successor);
    }
    if (accept_precondition (m_acceptor)) {
      ioa::schedule (&tcp_lcr_automaton::accept, m_acceptor);
    }
    if (connect_precondition ()) {
      ioa::schedule (&tcp_lcr_automaton::connect);
    }
    if (send_precondition (m_successor)) {
      ioa::schedule (&tcp_lcr_automaton::send, m_successor);
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
    if (m_successor_connected) {
      m_send.push (message_t (ELECT, u));
    }
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

  bool create_acceptor_precondition () const {
    return m_acceptor == 0;
  }

  void create_acceptor_effect () {
    std::cout << "Listening on " << m_recv_address.address_str () << ":" << m_recv_address.port () << std::endl;
    m_acceptor = ioa::make_automaton_manager (this, ioa::make_generator<ioa::tcp_acceptor_automaton> (m_recv_address));
    ioa::make_binding_manager (this,
    			       &m_self, &tcp_lcr_automaton::accept, m_acceptor,
    			       m_acceptor, &ioa::tcp_acceptor_automaton::accept);

    ioa::make_binding_manager (this, 
    			       m_acceptor, &ioa::tcp_acceptor_automaton::error,
    			       &m_self, &tcp_lcr_automaton::acceptor_error, m_acceptor);
  }

  void create_acceptor_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_lcr_automaton, create_acceptor);

  void acceptor_error_effect (const int& err,
			      ioa::automaton_manager<ioa::tcp_acceptor_automaton>* a) {
    assert (a == m_acceptor);
    std::cout << "Accepting failed...retrying in " << CONNECTION_TIMEOUT << " seconds" << std::endl;
    ioa::schedule_after (&tcp_lcr_automaton::retry_acceptor, ioa::time (CONNECTION_TIMEOUT, 0));
  }

  void acceptor_error_schedule (ioa::automaton_manager<ioa::tcp_acceptor_automaton>*) const {
    schedule ();
  }

  V_P_INPUT (tcp_lcr_automaton, acceptor_error, int, ioa::automaton_manager<ioa::tcp_acceptor_automaton>*);

  bool retry_acceptor_precondition () const {
    return m_acceptor != 0;
  }

  void retry_acceptor_effect () {
    m_acceptor->destroy ();
    m_acceptor = 0;
    if (m_predecessor != 0) {
      m_predecessor->destroy ();
      m_predecessor = 0;
    }
    m_accept_flag = false;
  }
  
  void retry_acceptor_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_lcr_automaton, retry_acceptor);

  bool create_predecessor_precondition () const {
    return m_predecessor == 0;
  }

  void create_predecessor_effect () {
    m_predecessor = ioa::make_automaton_manager (this, ioa::make_generator<ioa::tcp_connection_automaton> ());
    add_observable (m_predecessor);
    ioa::make_binding_manager (this,
    			       m_predecessor, &ioa::tcp_connection_automaton::receive,
    			       &m_self, &tcp_lcr_automaton::receive, m_predecessor);
    ioa::make_binding_manager (this,
    			       m_predecessor, &ioa::tcp_connection_automaton::connected,
    			       &m_self, &tcp_lcr_automaton::predecessor_connected, m_predecessor);
    ioa::make_binding_manager (this,
    			       m_predecessor, &ioa::tcp_connection_automaton::error,
    			       &m_self, &tcp_lcr_automaton::predecessor_error, m_predecessor);

    m_accept_flag = false;
  }

  void create_predecessor_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_lcr_automaton, create_predecessor);

  void predecessor_connected_effect (ioa::automaton_manager<ioa::tcp_connection_automaton>* p) {
    assert (m_predecessor == p);
    std::cout << "Predecessor connected" << std::endl;
  }

  void predecessor_connected_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  UV_P_INPUT (tcp_lcr_automaton, predecessor_connected, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  void predecessor_error_effect (const int& val,
				 ioa::automaton_manager<ioa::tcp_connection_automaton>* p) {
    assert (m_predecessor == p);
    std::cout << "Predecessor disconnected" << std::endl;
    m_predecessor->destroy ();
    m_predecessor = 0;
    m_accept_flag = false;
  }

  void predecessor_error_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  V_P_INPUT (tcp_lcr_automaton, predecessor_error, int, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  bool create_successor_precondition () const {
    return m_successor == 0;
  }

  void create_successor_effect () {
    m_successor = ioa::make_automaton_manager (this, ioa::make_generator<ioa::tcp_connection_automaton> ());
    add_observable (m_successor);
    ioa::make_binding_manager (this,
			       &m_self, &tcp_lcr_automaton::send, m_successor,
    			       m_successor, &ioa::tcp_connection_automaton::send);
    ioa::make_binding_manager (this,
    			       m_successor, &ioa::tcp_connection_automaton::send_complete,
    			       &m_self, &tcp_lcr_automaton::send_complete, m_successor);
    ioa::make_binding_manager (this,
    			       m_successor, &ioa::tcp_connection_automaton::connected,
    			       &m_self, &tcp_lcr_automaton::successor_connected, m_successor);
    ioa::make_binding_manager (this,
    			       m_successor, &ioa::tcp_connection_automaton::error,
    			       &m_self, &tcp_lcr_automaton::successor_error, m_successor);
  }

  void create_successor_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_lcr_automaton, create_successor);

  void successor_connected_effect (ioa::automaton_manager<ioa::tcp_connection_automaton>* s) {
    assert (s == m_successor);
    std::cout << "Succesor connected" << std::endl;
    m_successor_connected = true;
    m_clear_to_send = true;
  }

  void successor_connected_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  UV_P_INPUT (tcp_lcr_automaton, successor_connected, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  void successor_error_effect (const int& val,
			       ioa::automaton_manager<ioa::tcp_connection_automaton>* s) {
    assert (s == m_successor);
    std::cout << "Succesor disconnected" << std::endl;
    m_successor->destroy ();
    m_successor = 0;
    m_connector->destroy ();
    m_connector = 0;
    m_successor_connected = false;
  }

  void successor_error_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  V_P_INPUT (tcp_lcr_automaton, successor_error, int, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  bool accept_precondition (ioa::automaton_manager<ioa::tcp_acceptor_automaton>* a) const {
    return a == m_acceptor &&
      !m_accept_flag &&
      m_predecessor != 0 &&
      m_predecessor->get_state () == ioa::automaton_manager_interface::CREATED &&
      ioa::binding_count (&tcp_lcr_automaton::accept, a) != 0;
  }

  ioa::automaton_handle<ioa::tcp_connection_automaton> accept_effect (ioa::automaton_manager<ioa::tcp_acceptor_automaton>*) {
    m_accept_flag = true;
    return m_predecessor->get_handle ();
  }

  void accept_schedule (ioa::automaton_manager<ioa::tcp_acceptor_automaton>*) const {
    schedule ();
  }

  V_P_OUTPUT (tcp_lcr_automaton, accept, ioa::automaton_handle<ioa::tcp_connection_automaton>, ioa::automaton_manager<ioa::tcp_acceptor_automaton>*);

  bool connect_precondition () const {
    return m_connector == 0 &&
      m_successor != 0 &&
      m_successor->get_state () == ioa::automaton_manager_interface::CREATED;
  }

  void connect_effect () {
    std::cout << "Connecting to " << m_send_address.address_str () << ":" << m_send_address.port () << std::endl;
    m_connector = ioa::make_automaton_manager (this, ioa::make_generator<ioa::tcp_connector_automaton> (m_send_address, m_successor->get_handle ()));

    ioa::make_binding_manager (this, 
			       m_connector, &ioa::tcp_connector_automaton::error,
			       &m_self, &tcp_lcr_automaton::connector_error, m_connector);
  }

  void connect_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_lcr_automaton, connect);

  void connector_error_effect (const int& err,
			       ioa::automaton_manager<ioa::tcp_connector_automaton>* c) {
    assert (c == m_connector);
    std::cout << "Connection failed...retrying in " << CONNECTION_TIMEOUT << " seconds" << std::endl;
    ioa::schedule_after (&tcp_lcr_automaton::retry_connector, ioa::time (CONNECTION_TIMEOUT, 0));
  }

  void connector_error_schedule (ioa::automaton_manager<ioa::tcp_connector_automaton>*) const {
    schedule ();
  }

  V_P_INPUT (tcp_lcr_automaton, connector_error, int, ioa::automaton_manager<ioa::tcp_connector_automaton>*);

  bool retry_connector_precondition () const {
    return m_connector != 0;
  }

  void retry_connector_effect () {
    m_connector->destroy ();
    m_connector = 0;
  }
  
  void retry_connector_schedule () const {
    schedule ();
  }

  UP_INTERNAL (tcp_lcr_automaton, retry_connector);

  bool send_precondition (ioa::automaton_manager<ioa::tcp_connection_automaton>* s) const {
    return s == m_successor && m_successor_connected && m_clear_to_send && !m_send.empty () && ioa::binding_count (&tcp_lcr_automaton::send, s) != 0;
  }

  std::string send_effect (ioa::automaton_manager<ioa::tcp_connection_automaton>*)  {
    m_clear_to_send = false;
    std::string retval (reinterpret_cast<const char *> (&m_send.front ()), sizeof (message_t));
    m_send.pop ();
    return retval;
  }

  void send_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  V_P_OUTPUT (tcp_lcr_automaton, send, std::string, ioa::automaton_manager<ioa::tcp_connection_automaton>*);
  
  void send_complete_effect (ioa::automaton_manager<ioa::tcp_connection_automaton>* s) {
    if (s == m_successor) {
      m_clear_to_send = true;
    }
  }

  void send_complete_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  UV_P_INPUT (tcp_lcr_automaton, send_complete, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

  void receive_effect (const std::string& val,
		       ioa::automaton_manager<ioa::tcp_connection_automaton>* p) {
    if (p == m_predecessor) {
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
	  if (m.id != m_u && m_successor_connected) {
	    m_send.push (m);
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
  }

  void receive_schedule (ioa::automaton_manager<ioa::tcp_connection_automaton>*) const {
    schedule ();
  }

  V_P_INPUT (tcp_lcr_automaton, receive, std::string, ioa::automaton_manager<ioa::tcp_connection_automaton>*);

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
    if (m_leader && m_successor_connected) {
      m_send.push (message_t (TOKEN, m_u));
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

#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>
#include <ioa/inet_address.hpp>
#include <ioa/tcp_acceptor_automaton.hpp>

#include <iostream>

class client_handler_automaton :
  public ioa::automaton,
  private ioa::observer
{
private:
  enum state_t {
    SEND_READY,
    SEND_COMPLETE_WAIT,
  };

  ioa::handle_manager<client_handler_automaton> m_self;
  ioa::handle_manager<ioa::tcp_acceptor_automaton> m_acceptor;
  bool m_accept_flag;
  ioa::automaton_manager<ioa::tcp_connection_automaton>* m_connection;
  bool m_done;
  bool m_done_reported;
  bool m_connected;
  bool m_connected_reported;
  state_t m_state;
  std::queue<std::string*> m_buffers;

public:
  client_handler_automaton (const ioa::automaton_handle<ioa::tcp_acceptor_automaton>& handle) :
    m_self (ioa::get_aid ()),
    m_acceptor (handle),
    m_accept_flag (false),
    m_done (false),
    m_done_reported (false),
    m_connected (false),
    m_connected_reported (false),
    m_state (SEND_READY)
  {
    ioa::make_binding_manager (this,
			       &m_self, &client_handler_automaton::accept,
			       &m_acceptor, &ioa::tcp_acceptor_automaton::accept);

    m_connection = ioa::make_automaton_manager (this, ioa::make_generator<ioa::tcp_connection_automaton> ());
    add_observable (m_connection);
    ioa::make_binding_manager (this,
    			       &m_self, &client_handler_automaton::send,
    			       m_connection, &ioa::tcp_connection_automaton::send);
    ioa::make_binding_manager (this,
    			       m_connection, &ioa::tcp_connection_automaton::send_complete,
    			       &m_self, &client_handler_automaton::send_complete);
    ioa::make_binding_manager (this,
    			       m_connection, &ioa::tcp_connection_automaton::receive,
    			       &m_self, &client_handler_automaton::receive);
    ioa::make_binding_manager (this,
			       m_connection, &ioa::tcp_connection_automaton::connected,
			       &m_self, &client_handler_automaton::connected_int);
    ioa::make_binding_manager (this,
    			       m_connection, &ioa::tcp_connection_automaton::error,
    			       &m_self, &client_handler_automaton::error);
  }

  ~client_handler_automaton () {
    while (!m_buffers.empty ()) {
      delete m_buffers.front ();
      m_buffers.pop ();
    }
  }

private:
  void schedule () const {
    if (send_precondition ()) {
      ioa::schedule (&client_handler_automaton::send);
    }
    if (accept_precondition ()) {
      ioa::schedule (&client_handler_automaton::accept);
    }
  }
  
  void observe (ioa::observable* o) {
    schedule ();
  }

  bool accept_precondition () const {
    return m_connection != 0 &&
      m_connection->get_state () == ioa::automaton_manager_interface::CREATED &&
      m_accept_flag == false &&
      ioa::binding_count (&client_handler_automaton::accept) != 0;
  }

  ioa::automaton_handle<ioa::tcp_connection_automaton> accept_effect () {
    m_accept_flag = true;
    return m_connection->get_handle ();
  }

  void accept_schedule () const {
    schedule ();
  }

  V_UP_OUTPUT (client_handler_automaton, accept, ioa::automaton_handle<ioa::tcp_connection_automaton>);

  void connected_int_effect () {
    m_connected = true;
  }

  void connected_int_schedule () const {
    schedule ();
  }

  UV_UP_INPUT (client_handler_automaton, connected_int);

  void error_effect (const int& err) {
    m_done = true;
  }

  void error_schedule () const {
    schedule ();
  }

  V_UP_INPUT (client_handler_automaton, error, int);

private:
  bool connected_precondition () const {
    return m_connected && !m_connected_reported && ioa::binding_count (&client_handler_automaton::connected) != 0;
  }

  void connected_effect () {
    m_connected_reported = true;
  }

  void connected_schedule () const {
    schedule ();
  }

public:
  UV_UP_OUTPUT (client_handler_automaton, connected);

private:
  bool done_precondition () const {
    return m_done && !m_done_reported && ioa::binding_count (&client_handler_automaton::done) != 0;
  }

  void done_effect () {
    m_done_reported = true;
  }

  void done_schedule () const {
    schedule ();
  }

public:
  UV_UP_OUTPUT (client_handler_automaton, done); 

private:
  bool send_precondition () const {
    return m_state == SEND_READY && 
      !m_buffers.empty () &&
      ioa::binding_count (&client_handler_automaton::send) != 0;
  }

  std::string send_effect () {
    std::string buf = *m_buffers.front ();
    delete m_buffers.front ();
    m_buffers.pop ();
    m_state = SEND_COMPLETE_WAIT;
    return buf;
  }

  void send_schedule () const {
    schedule ();
  }

  V_UP_OUTPUT (client_handler_automaton, send, std::string);

  void send_complete_effect () {
    assert (m_state == SEND_COMPLETE_WAIT);
    m_state = SEND_READY;
  }

  void send_complete_schedule () const {
    schedule ();
  }

  UV_UP_INPUT (client_handler_automaton, send_complete);

  void receive_effect (const std::string& val) {
    std::cout << val;
    m_buffers.push (new std::string (val));
  }

  void receive_schedule () const {
    schedule ();
  }

  V_UP_INPUT (client_handler_automaton, receive, std::string);
};

class echo_server_automaton :
  public ioa::automaton,
  private ioa::observer
{
private:
  ioa::handle_manager<echo_server_automaton> m_self;
  ioa::automaton_manager<ioa::tcp_acceptor_automaton>* m_acceptor;
  std::set<ioa::automaton_manager<client_handler_automaton>*> m_not_connected;
  std::set<ioa::automaton_manager<client_handler_automaton>*> m_connected;

public:
  echo_server_automaton (const ioa::inet_address& address) :
    m_self (ioa::get_aid ())
  {
    // Create the acceptor, observe it so we know when it is created, and bind to its error output.
    m_acceptor = new ioa::automaton_manager<ioa::tcp_acceptor_automaton> (this, ioa::make_generator<ioa::tcp_acceptor_automaton> (address));
    add_observable (m_acceptor);
    ioa::make_binding_manager (this,
			       m_acceptor, &ioa::tcp_acceptor_automaton::error,
			       &m_self, &echo_server_automaton::error);
    schedule ();
  }

private:

  void schedule () const {
    if (create_precondition ()) {
      ioa::schedule (&echo_server_automaton::create);
    }
  }

  void observe (ioa::observable* o) {
    schedule ();
  }

  void error_effect (const int& err) {
    // Acceptor had an error.
    char buf[256];
#ifdef STRERROR_R_CHAR_P
    std::cerr << "Acceptor error: " << strerror_r (err, buf, 256) << std::endl;
#else
    strerror_r (err, buf, 256);
    std::cerr << "Acceptor error: " << buf << std::endl;
#endif
    m_acceptor->destroy ();
    m_acceptor = 0;
  }

  void error_schedule () const {
    schedule ();
  }

  V_UP_INPUT (echo_server_automaton, error, int);

  bool create_precondition () const {
    return m_acceptor != 0 && m_acceptor->get_state () == ioa::automaton_manager_interface::CREATED && m_not_connected.size () < 5;
  }

  void create_effect () {
    // Create another client to fill the pool.
    ioa::automaton_manager<client_handler_automaton>* m_client = ioa::make_automaton_manager (this, ioa::make_generator<client_handler_automaton> (m_acceptor->get_handle ()));
    m_not_connected.insert (m_client);
    ioa::make_binding_manager (this,
			       m_client, &client_handler_automaton::connected,
			       &m_self, &echo_server_automaton::connected, m_client);
    ioa::make_binding_manager (this,
			       m_client, &client_handler_automaton::done,
			       &m_self, &echo_server_automaton::done, m_client);
  }

  void create_schedule () const {
    schedule ();
  }

  UP_INTERNAL (echo_server_automaton, create);

  void connected_effect (ioa::automaton_manager<client_handler_automaton>* client) {
    // Client has connected.
    if (m_not_connected.count (client) != 0) {
      m_not_connected.erase (client);
      m_connected.insert (client);
    }
  }

  void connected_schedule (ioa::automaton_manager<client_handler_automaton>*) const {
    schedule ();
  }

  UV_P_INPUT (echo_server_automaton, connected, ioa::automaton_manager<client_handler_automaton>*);

  void done_effect (ioa::automaton_manager<client_handler_automaton>* client) {
    // Client is done.
    if (m_connected.count (client) != 0) {
      m_connected.erase (client);
      client->destroy ();
    }
    else if (m_not_connected.count (client) != 0) {
      m_not_connected.erase (client);
      client->destroy ();
    }
  }

  void done_schedule (ioa::automaton_manager<client_handler_automaton>*) const {
    schedule ();
  }

  UV_P_INPUT (echo_server_automaton, done, ioa::automaton_manager<client_handler_automaton>*);
};

int main () {
  // TODO
  ioa::inet_address address ("0.0.0.0", 54321);
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<echo_server_automaton> (address));
  return 0;
}

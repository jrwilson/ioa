#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>
#include <ioa/inet_address.hpp>
#include <ioa/tcp_connector.hpp>

#include <iostream>

class echo_client_automaton :
  public ioa::automaton,
  private ioa::observer
{
private:
  enum state_t {
    CONNECT_READY,
    CONNECT_COMPLETE_WAIT,
    SEND_READY,
    RECEIVE_WAIT
  };

  state_t m_state;
  ioa::handle_manager<echo_client_automaton> m_self;
  ioa::handle_manager<ioa::tcp_connection_automaton> m_connection;

public:
  echo_client_automaton () :
    m_state (CONNECT_READY),
    m_self (ioa::get_aid ())
  {
    ioa::automaton_manager<ioa::tcp_connector>* connector = new ioa::automaton_manager<ioa::tcp_connector> (this, ioa::make_generator<ioa::tcp_connector> ());

    ioa::make_binding_manager (this,
			       &m_self, &echo_client_automaton::connect,
			       connector, &ioa::tcp_connector::connect);

    ioa::make_binding_manager (this,
			       connector, &ioa::tcp_connector::connect_complete,
			       &m_self, &echo_client_automaton::connect_complete);

    add_observable (&connect_complete);
    
    schedule ();
  }

private:
  void schedule () const {
    if (connect_precondition ()) {
      ioa::schedule (&echo_client_automaton::connect);
    }
    if (send_precondition ()) {
      ioa::schedule (&echo_client_automaton::send);
    }
  }

  void observe (ioa::observable* o) {
    if (o == &connect_complete || o == &send_complete || o == &receive) {
      schedule ();
    }
  }

  bool connect_precondition () const {
    return m_state == CONNECT_READY &&
      ioa::bind_count (&echo_client_automaton::connect) != 0 &&
      ioa::bind_count (&echo_client_automaton::connect_complete) != 0;
  }

  ioa::inet_address connect_effect () {
    m_state = CONNECT_COMPLETE_WAIT;
    return ioa::inet_address ("127.0.0.1", 54321);
  }

  V_UP_OUTPUT (echo_client_automaton, connect, ioa::inet_address);  

  void connect_complete_effect (const ioa::automaton_handle<ioa::tcp_connection_automaton>& handle) {
    assert (m_state == CONNECT_COMPLETE_WAIT);
    std::cout << __func__ << " " << handle << std::endl;
    m_connection = handle;

    ioa::make_binding_manager (this,
			       &m_self, &echo_client_automaton::send,
			       &m_connection, &ioa::tcp_connection_automaton::send);

    ioa::make_binding_manager (this,
			       &m_connection, &ioa::tcp_connection_automaton::send_complete,
			       &m_self, &echo_client_automaton::send_complete);

    ioa::make_binding_manager (this,
			       &m_connection, &ioa::tcp_connection_automaton::receive,
			       &m_self, &echo_client_automaton::receive);

    add_observable (&send_complete);
    add_observable (&receive);

    m_state = SEND_READY;
  }

  V_UP_INPUT (echo_client_automaton, connect_complete, ioa::automaton_handle<ioa::tcp_connection_automaton>);

  bool send_precondition () const {
    return m_state == SEND_READY &&
      ioa::bind_count (&echo_client_automaton::send) != 0 &&
      ioa::bind_count (&echo_client_automaton::send_complete) != 0 &&
      ioa::bind_count (&echo_client_automaton::receive) != 0;
  }

  ioa::buffer send_effect () {
    m_state = RECEIVE_WAIT;
    std::string s ("Hello World!!");
    std::cout << "Sent: " << s << std::endl;
    return ioa::buffer (s.c_str (), s.size ());
  }

  V_UP_OUTPUT (echo_client_automaton, send, ioa::buffer);

  void send_complete_effect (const int& err) {
    std::cout << "Got send complete" << std::endl;
  }

  V_UP_INPUT (echo_client_automaton, send_complete, int);

  void receive_effect (const ioa::tcp_connection_automaton::receive_val& val) {
    assert (m_state == RECEIVE_WAIT);
    std::cout << __func__ << std::endl;
    std::string s (val.buf.c_str (), val.buf.size ());
    std::cout << "Received: " << s << std::endl;
    std::cout << "Time to die" << std::endl;
  }
  
  V_UP_INPUT (echo_client_automaton, receive, ioa::tcp_connection_automaton::receive_val);
};

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<echo_client_automaton> ());
  return 0;
}

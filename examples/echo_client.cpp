#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>
#include <ioa/inet_address.hpp>
#include <ioa/tcp_connector_automaton.hpp>

#include <iostream>

class echo_client_automaton :
  public ioa::automaton
{
private:
  enum state_t {
    CONNECT_READY,
    CONNECT_COMPLETE_WAIT,
    SEND_READY,
    RECEIVE_WAIT,
  };

  state_t m_state;
  ioa::handle_manager<echo_client_automaton> m_self;
  ioa::handle_manager<ioa::tcp_connection_automaton> m_connection;
  ioa::binding_manager_interface* m_binding_manager;

public:
  echo_client_automaton () :
    m_state (CONNECT_READY),
    m_self (ioa::get_aid ())
  {
    ioa::automaton_manager<ioa::tcp_connector_automaton>* connector = new ioa::automaton_manager<ioa::tcp_connector_automaton> (this, ioa::make_generator<ioa::tcp_connector_automaton> ());

    ioa::make_binding_manager (this,
			       &m_self, &echo_client_automaton::connect,
			       connector, &ioa::tcp_connector_automaton::connect);

    ioa::make_binding_manager (this,
			       connector, &ioa::tcp_connector_automaton::connect_complete,
			       &m_self, &echo_client_automaton::connect_complete);

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

  bool connect_precondition () const {
    return m_state == CONNECT_READY && ioa::bind_count (&echo_client_automaton::connect) != 0;
  }

  ioa::inet_address connect_effect () {
    m_state = CONNECT_COMPLETE_WAIT;
    return ioa::inet_address ("127.0.0.1", 54321);
  }

  V_UP_OUTPUT (echo_client_automaton, connect, ioa::inet_address);  

  void connect_complete_effect (const ioa::tcp_connector_automaton::connect_val& val) {
    if (val.err != 0) {
      char buf[256];
#ifdef STRERROR_R_CHAR_P
      std::cerr << "Couldn't connect: " << strerror_r (val.err, buf, 256) << std::endl;
#else
      strerror_r (val.err, buf, 256);
      std::cerr << "Couldn't connect: " << buf << std::endl;
#endif
      self_destruct ();
    }
    else {
      m_connection = val.handle;
      ioa::make_binding_manager (this,
				 &m_self, &echo_client_automaton::send,
				 &m_connection, &ioa::tcp_connection_automaton::send);
      
      ioa::make_binding_manager (this,
				 &m_connection, &ioa::tcp_connection_automaton::send_complete,
				 &m_self, &echo_client_automaton::send_complete);
      
      m_binding_manager = ioa::make_binding_manager (this,
						     &m_connection, &ioa::tcp_connection_automaton::receive,
						     &m_self, &echo_client_automaton::receive);
      
      m_state = SEND_READY;
    }
  }

  V_UP_INPUT (echo_client_automaton, connect_complete, ioa::tcp_connector_automaton::connect_val);

  bool send_precondition () const {
    return m_state == SEND_READY && ioa::bind_count (&echo_client_automaton::send) != 0;
  }

  ioa::buffer send_effect () {
    m_state = RECEIVE_WAIT;
    std::string s ("Hello World!!");
    std::cout << "Sent:\t\t" << s << std::endl;
    return ioa::buffer (s.c_str (), s.size ());
  }

  V_UP_OUTPUT (echo_client_automaton, send, ioa::buffer);

  void send_complete_effect (const int& err) {
    if (err != 0) {
      char buf[256];
#ifdef STRERROR_R_CHAR_P
      std::cerr << "Couldn't send: " << strerror_r (err, buf, 256) << std::endl;
#else
      strerror_r (err, buf, 256);
      std::cerr << "Couldn't send: " << buf << std::endl;
#endif
      self_destruct ();
    }
  }

  V_UP_INPUT (echo_client_automaton, send_complete, int);

  void receive_effect (const ioa::tcp_connection_automaton::receive_val& val) {
    if (val.err != 0) {
      char buf[256];
#ifdef STRERROR_R_CHAR_P
      std::cerr << "Couldn't receive: " << strerror_r (val.err, buf, 256) << std::endl;
#else
      strerror_r (val.err, buf, 256);
      std::cerr << "Couldn't receive: " << buf << std::endl;
#endif
      self_destruct ();
    }
    else {
      std::string s (val.buffer.c_str (), val.buffer.size ());
      std::cout << "Received:\t" << s << std::endl;
      m_binding_manager->unbind ();
    }
  }
  
  V_UP_INPUT (echo_client_automaton, receive, ioa::tcp_connection_automaton::receive_val);
};

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<echo_client_automaton> ());
  return 0;
}

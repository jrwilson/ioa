#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>
#include <ioa/inet_address.hpp>
#include <ioa/tcp_acceptor.hpp>

#include <iostream>

class client_handler_automaton :
  public ioa::automaton
{
private:
  enum state_t {
    SEND_READY,
    SEND_COMPLETE_WAIT,
  };
  ioa::handle_manager<client_handler_automaton> m_self;
  ioa::handle_manager<ioa::tcp_connection> m_connection;
  state_t m_state;
  std::queue<ioa::buffer> m_buffers;

public:
  client_handler_automaton (const ioa::automaton_handle<ioa::tcp_connection>& handle) :
    m_self (ioa::get_aid ()),
    m_connection (handle),
    m_state (SEND_READY)
  {
    std::cout << m_self.get_handle () << " will serve " << m_connection.get_handle () << std::endl;

    ioa::make_binding_manager (this,
			       &m_self, &client_handler_automaton::send,
			       &m_connection, &ioa::tcp_connection::send);

    ioa::make_binding_manager (this,
    			       &m_connection, &ioa::tcp_connection::send_complete,
    			       &m_self, &client_handler_automaton::send_complete);

    ioa::make_binding_manager (this,
    			       &m_connection, &ioa::tcp_connection::receive,
    			       &m_self, &client_handler_automaton::receive);
  }

private:
  void schedule () const {
    if (send_precondition ()) {
      ioa::schedule (&client_handler_automaton::send);
    }
  }
  
  bool send_precondition () const {
    return m_state == SEND_READY && 
      !m_buffers.empty () &&
      ioa::bind_count (&client_handler_automaton::send) != 0;
  }

  ioa::buffer send_effect () {
    std::cout << __func__ << std::endl;
    ioa::buffer buf = m_buffers.front ();
    m_buffers.pop ();
    m_state = SEND_COMPLETE_WAIT;
    return buf;
  }

  V_UP_OUTPUT (client_handler_automaton, send, ioa::buffer);

  void send_complete_effect () {
    std::cout << __func__ << std::endl;
    assert (m_state == SEND_COMPLETE_WAIT);
    m_state = SEND_READY;
  }

  UV_UP_INPUT (client_handler_automaton, send_complete);

  void receive_effect (const ioa::buffer& buf) {
    std::cout << __func__ << std::endl;
    std::string s (buf.c_str (), buf.size ());
    std::cout << "Got: " << s << std::endl;
    m_buffers.push (buf);
  }

  V_UP_INPUT (client_handler_automaton, receive, ioa::buffer);
};

class echo_server_automaton :
  public ioa::automaton
{
private:
  ioa::handle_manager<echo_server_automaton> m_self;

public:
  echo_server_automaton () :
    m_self (ioa::get_aid ())
  {
    ioa::automaton_manager<ioa::tcp_acceptor>* acceptor = new ioa::automaton_manager<ioa::tcp_acceptor> (this, ioa::make_generator<ioa::tcp_acceptor> (ioa::inet_address ("0.0.0.0", 54321)));
    ioa::make_binding_manager (this,
			       acceptor, &ioa::tcp_acceptor::accept_complete,
			       &m_self, &echo_server_automaton::accept_complete);
			       
  }

private:
  void schedule () const { }

  void accept_complete_effect (const ioa::automaton_handle<ioa::tcp_connection>& connection) {
    std::cout << __func__ << std::endl;
    std::cout << "Got connection " << connection << std::endl;

    new ioa::automaton_manager<client_handler_automaton> (this, ioa::make_generator<client_handler_automaton> (connection));
  }

  V_UP_INPUT (echo_server_automaton, accept_complete, ioa::automaton_handle<ioa::tcp_connection>);
};

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<echo_server_automaton> ());
  return 0;
}

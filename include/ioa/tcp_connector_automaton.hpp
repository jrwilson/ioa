#ifndef __tcp_connector_automaton_hpp__
#define __tcp_connector_automaton_hpp__

#include <ioa/tcp_connection_automaton.hpp>

namespace ioa {
  
  class tcp_connector_automaton :
    public automaton
  {
  private:
    handle_manager<tcp_connector_automaton> m_self;
    automaton_handle<tcp_connection_automaton> m_connection;
    int m_fd;
    int m_errno;
    bool m_error_reported;

    void schedule () const;

  public:
    tcp_connector_automaton (const inet_address& address,
			     const automaton_handle<tcp_connection_automaton>& connection);
    ~tcp_connector_automaton ();

  private:
    bool error_precondition () const;
    int error_effect ();
    void error_schedule () const;
  public:
    V_UP_OUTPUT (tcp_connector_automaton, error, int);

  private:
    bool write_ready_precondition () const;
    void write_ready_effect ();
    void write_ready_schedule () const;
    UP_INTERNAL (tcp_connector_automaton, write_ready);

    void done_effect (const int&);
    void done_schedule () const;
    V_UP_INPUT (tcp_connector_automaton, done, int);

  };

}

#endif

#ifndef __tcp_connector_automaton_hpp__
#define __tcp_connector_automaton_hpp__

#include <ioa/inet_address.hpp>
#include <ioa/tcp_connection_automaton.hpp>

namespace ioa {
  
  class tcp_connector_automaton :
    public automaton,
    private observer
  {
  private:
    handle_manager<tcp_connector_automaton> m_self;
    int m_fd;
    automaton_manager<tcp_connection_automaton>* m_connection;
    bool m_connection_reported;
    int m_errno;
    bool m_error_reported;

    void schedule () const;
    void observe (observable* o);

  public:
    tcp_connector_automaton (const inet_address& address);
    ~tcp_connector_automaton ();

  private:
    bool connect_precondition () const;
    automaton_handle<tcp_connection_automaton> connect_effect ();
    void connect_schedule () const;
  public:
    V_UP_OUTPUT (tcp_connector_automaton, connect, automaton_handle<tcp_connection_automaton>);

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

    void closed_effect ();
    void closed_schedule () const;
    UV_UP_INPUT (tcp_connector_automaton, closed);
  };

}

#endif

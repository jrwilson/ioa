#ifndef __tcp_connector_automaton_hpp__
#define __tcp_connector_automaton_hpp__

#include <ioa/inet_address.hpp>
#include <ioa/tcp_connection_automaton.hpp>
#include <fcntl.h>
#include <map>

namespace ioa {
  
  class tcp_connector_automaton :
    public automaton,
    private observer
  {
  public:
    struct connect_val
    {
      int err;
      automaton_handle<tcp_connection_automaton> handle;

      connect_val (const int e,
		   const automaton_handle<tcp_connection_automaton>& h) :
	err (e),
	handle (h)
      { }
    };

  private:
    std::map<aid_t, int> m_aid_to_fd;
    std::map<automaton_manager<tcp_connection_automaton>*, aid_t> m_manager_to_aid;
    std::map<aid_t, automaton_manager<tcp_connection_automaton>*> m_aid_to_manager;
    std::map<aid_t, connect_val> m_aid_to_connect;

  public:
    tcp_connector_automaton ();
    ~tcp_connector_automaton ();

  private:
    void schedule () const;
    void observe (observable* o);

  private:
    void connect_effect (const inet_address& address,
			 aid_t aid);
    void connect_schedule (aid_t aid) const { schedule (); }
  public:
    V_AP_INPUT (tcp_connector_automaton, connect, inet_address);

  private:
    bool write_ready_precondition (aid_t aid) const;
    void write_ready_effect (aid_t aid);
    void write_ready_schedule (aid_t) const { schedule (); }
    P_INTERNAL (tcp_connector_automaton, write_ready, aid_t);

  private:
    bool connect_complete_precondition (aid_t aid) const;
    connect_val connect_complete_effect (aid_t aid);
    void connect_complete_schedule (aid_t aid) const { schedule (); }
  public:
    V_AP_OUTPUT (tcp_connector_automaton, connect_complete, connect_val);
  };

}

#endif

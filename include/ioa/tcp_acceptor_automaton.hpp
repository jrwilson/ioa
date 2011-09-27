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

#ifndef __tcp_acceptor_automaton_hpp__
#define __tcp_acceptor_automaton_hpp__

#include <ioa/tcp_connection_automaton.hpp>
#include <queue>

namespace ioa {

  class tcp_acceptor_automaton :
    public automaton
  {
  private:    
    enum state_t {
      SCHEDULE_READ_READY,
      READ_READY_WAIT,
    };
    inet_address m_address;
    state_t m_state;
    int m_fd;
    int m_errno;
    bool m_error_reported;
    handle_manager<tcp_acceptor_automaton> m_self;

    std::queue<automaton_handle<tcp_connection_automaton> > m_connection_queue;
    std::queue<int> m_fd_queue;

    void schedule () const;

  public:
    tcp_acceptor_automaton (const ioa::inet_address& address,
			    const int backlog = 5);
    ~tcp_acceptor_automaton ();

  private:
    void accept_effect (const automaton_handle<tcp_connection_automaton>& conn,
			aid_t);
    void accept_schedule (aid_t) const;
  public:
    V_AP_INPUT (tcp_acceptor_automaton, accept, automaton_handle<tcp_connection_automaton>);

  private:
    bool error_precondition () const;
    int error_effect ();
    void error_schedule () const;
  public:
    V_UP_OUTPUT (tcp_acceptor_automaton, error, int);

  private:
    bool schedule_read_ready_precondition () const;
    void schedule_read_ready_effect ();
    void schedule_read_ready_schedule () const;
    UP_INTERNAL (tcp_acceptor_automaton, schedule_read_ready);

    bool read_ready_precondition () const;
    void read_ready_effect ();
    void read_ready_schedule () const;
    UP_INTERNAL (tcp_acceptor_automaton, read_ready);

    bool create_helper_precondition () const;
    void create_helper_effect ();
    void create_helper_schedule () const;
    UP_INTERNAL (tcp_acceptor_automaton, create_helper);

    void done_effect (const int&,
		      automaton_manager<connection_init_automaton>*);
    void done_schedule (automaton_manager<connection_init_automaton>*) const;
    V_P_INPUT (tcp_acceptor_automaton, done, int, automaton_manager<connection_init_automaton>*);
  };

}

#endif

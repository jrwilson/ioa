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

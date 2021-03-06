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

#include <ioa/tcp_connector_automaton.hpp>

#include <fcntl.h>

namespace ioa {

  void tcp_connector_automaton::schedule () const {
    if (error_precondition ()) {
      ioa::schedule (&tcp_connector_automaton::error);
    }
  }

  tcp_connector_automaton::tcp_connector_automaton (const inet_address& address,
						    const automaton_handle<tcp_connection_automaton>& connection) :
    m_self (get_aid ()),
    m_connection (connection),
    m_errno (0),
    m_error_reported (false) {

    try {
      // Open a socket.
      m_fd = socket (AF_INET, SOCK_STREAM, 0);
      if (m_fd == -1) {
	m_errno = errno;
	throw std::exception ();
      }
      
      // Get the flags.
      int flags = fcntl (m_fd, F_GETFL, 0);
      if (flags < 0) {
	m_errno = errno;
	throw std::exception ();
      }
      
      // Set non-blocking.
      flags |= O_NONBLOCK;
      if (fcntl (m_fd, F_SETFL, flags) == -1) {
	m_errno = errno;
	throw std::exception ();
      }
      
      if (::connect (m_fd, address.get_sockaddr (), address.get_socklen ()) != -1) {
	automaton_manager<connection_init_automaton>* init = make_automaton_manager (this, make_allocator<connection_init_automaton> (m_connection, m_fd));
	make_binding_manager (this,
			      init, &connection_init_automaton::done,
			      &m_self, &tcp_connector_automaton::done);
	throw std::exception ();
      }
      else {
	if (errno == EINPROGRESS) {
	  // Asynchronous connect.
	  ioa::schedule_write_ready (&ioa::tcp_connector_automaton::write_ready, m_fd);
	}
	else {
	  m_errno = errno;
	  throw std::exception ();
	}
      }
    } catch (...) { }

    schedule ();
  }

  tcp_connector_automaton::~tcp_connector_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }
  }

  bool tcp_connector_automaton::error_precondition () const {
    return m_errno != 0 && m_error_reported == false && binding_count (&tcp_connector_automaton::error) != 0;
  }

  int tcp_connector_automaton::error_effect () {
    m_error_reported = true;
    return m_errno;
  }
  
  void tcp_connector_automaton::error_schedule () const {
    schedule ();
  }

  bool tcp_connector_automaton::write_ready_precondition () const {
    return true;
  }

  void tcp_connector_automaton::write_ready_effect () {
    // See if the connection succeeded.
    int val;
    socklen_t sz = sizeof (val);
    if (getsockopt (m_fd, SOL_SOCKET, SO_ERROR, &val, &sz) == -1) {
      m_errno = errno;
      return;
    }

    if (val == 0) {
      // Success.
      automaton_manager<connection_init_automaton>* init = make_automaton_manager (this, make_allocator<connection_init_automaton> (m_connection, m_fd));
      make_binding_manager (this,
			    init, &connection_init_automaton::done,
			    &m_self, &tcp_connector_automaton::done);
      
    }
    else {
      m_errno = val;
    }
  }

  void tcp_connector_automaton::write_ready_schedule () const {
    schedule ();
  }

  void tcp_connector_automaton::done_effect (const int& fd) {
    if (fd != -1) {
      close (fd);
    }
  }

  void tcp_connector_automaton::done_schedule () const {
    schedule ();
  }

}

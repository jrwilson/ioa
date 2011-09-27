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

 #include <ioa/tcp_acceptor_automaton.hpp>

#include <fcntl.h>

namespace ioa {

  tcp_acceptor_automaton::tcp_acceptor_automaton (const ioa::inet_address& address,
						  const int backlog) :
    m_address (address),
    m_state (SCHEDULE_READ_READY),
    m_errno (0),
    m_error_reported (false),
    m_self (get_aid ())
  {
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
      
      //       const int val = 1;
      // #ifdef SO_REUSEADDR
      //       // Set reuse.
      //       if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val)) == -1) {
      // 	m_errno = errno;
      // 	throw std::exception ();
      //       }
      // #endif
      
      // #ifdef SO_REUSEPORT
      //       // Set reuse.
      //       if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof (val)) == -1) {
      // 	m_errno = errno;
      // 	throw std::exception ();
      //       }
      // #endif

      // Bind.
      if (::bind (m_fd, address.get_sockaddr (), address.get_socklen ()) == -1) {
	m_errno = errno;
	throw std::exception ();
      }

      // Listen.
      if (listen (m_fd, backlog) == -1) {
	m_errno = errno;
	throw std::exception ();
      }
    } catch (...) { }

    schedule ();
  }

  tcp_acceptor_automaton::~tcp_acceptor_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }
    while (!m_fd_queue.empty ()) {
      close (m_fd_queue.front ());
      m_fd_queue.pop ();
    }
  }

  void tcp_acceptor_automaton::schedule () const {
    if (error_precondition ()) {
      ioa::schedule (&tcp_acceptor_automaton::error);
    }
    if (schedule_read_ready_precondition ()) {
      ioa::schedule (&tcp_acceptor_automaton::schedule_read_ready);
    }
    if (create_helper_precondition ()) {
      ioa::schedule (&tcp_acceptor_automaton::create_helper);
    }
  }

  void tcp_acceptor_automaton::accept_effect (const automaton_handle<tcp_connection_automaton>& conn,
					      aid_t) {
    // Add the connection to the queue.
    m_connection_queue.push (conn);
  }

  void tcp_acceptor_automaton::accept_schedule (aid_t) const {
    schedule ();
  }

  bool tcp_acceptor_automaton::error_precondition () const {
    return m_errno != 0 && m_error_reported == false && binding_count (&tcp_acceptor_automaton::error) != 0;
  }

  int tcp_acceptor_automaton::error_effect () {
    m_error_reported = true;
    return m_errno;
  }
  
  void tcp_acceptor_automaton::error_schedule () const {
    schedule ();
  }

  bool tcp_acceptor_automaton::schedule_read_ready_precondition () const {
    return  m_state == SCHEDULE_READ_READY && m_errno == 0 && !m_connection_queue.empty () && m_fd_queue.empty ();
  }

  void tcp_acceptor_automaton::schedule_read_ready_effect () {
    ioa::schedule_read_ready (&tcp_acceptor_automaton::read_ready, m_fd);
    m_state = READ_READY_WAIT;
  }

  void tcp_acceptor_automaton::schedule_read_ready_schedule () const {
    schedule ();
  }

  bool tcp_acceptor_automaton::read_ready_precondition () const {
    // This should finish.
    return m_state == READ_READY_WAIT && m_errno == 0;
  }

  void tcp_acceptor_automaton::read_ready_effect () {
    m_state = SCHEDULE_READ_READY;

    inet_address address;
    int connection_fd = ::accept (m_fd, address.get_sockaddr_ptr (), address.get_socklen_ptr ());
    if (connection_fd != -1) {
      m_fd_queue.push (connection_fd);	
    }
    else {
      m_errno = errno;
    }
  }

  void tcp_acceptor_automaton::read_ready_schedule () const {
    schedule ();
  }

  bool tcp_acceptor_automaton::create_helper_precondition () const {
    return !m_connection_queue.empty () && !m_fd_queue.empty ();
  }

  void tcp_acceptor_automaton::create_helper_effect () {
    automaton_manager<connection_init_automaton>* helper = make_automaton_manager (this, make_generator<connection_init_automaton> (m_connection_queue.front (), m_fd_queue.front ()));
    m_connection_queue.pop ();
    m_fd_queue.pop ();
    make_binding_manager (this,
			  helper, &connection_init_automaton::done,
			  &m_self, &tcp_acceptor_automaton::done, helper);
  }

  void tcp_acceptor_automaton::create_helper_schedule () const {
    schedule ();
  }

  void tcp_acceptor_automaton::done_effect (const int& fd,
					    automaton_manager<connection_init_automaton>* helper) {
    if (fd != -1) {
      // Recycle.
      m_fd_queue.push (fd);
    }
    helper->destroy ();
  }

  void tcp_acceptor_automaton::done_schedule (automaton_manager<connection_init_automaton>*) const {
    schedule ();
  }

}

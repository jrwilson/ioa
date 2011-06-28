#ifndef __tcp_acceptor_hpp__
#define __tcp_acceptor_hpp__

#include <fcntl.h>
#include <ioa/tcp_connection_automaton.hpp>
#include <queue>

#include <iostream>

#define BACKLOG 5

namespace ioa {
  
  class tcp_acceptor :
    public automaton,
    private observer
  {
  private:
    int m_accept_fd;
    std::queue<automaton_handle<tcp_connection_automaton> > m_connectionq;

  public:
    tcp_acceptor (const ioa::inet_address& address) {
      std::cout << "Accepting on " << address.address_str () << " " << address.port () << std::endl;

      // Open a socket.
      m_accept_fd = socket (AF_INET, SOCK_STREAM, 0);
      std::cout << "fd = " << m_accept_fd << std::endl;
      if (m_accept_fd == -1) {
	// TODO
	// m_errno = errno;
	// goto the_end;
      }
      
      // Get the flags.
      int flags = fcntl (m_accept_fd, F_GETFL, 0);
      std::cout << "flags = " << flags << std::endl;
      if (flags < 0) {
	// TODO
	// m_errno = errno;
	// goto the_end;
	assert (false);
      }
      
      // Set non-blocking.
      flags |= O_NONBLOCK;
      int res = fcntl (m_accept_fd, F_SETFL, flags);
      std::cout << "res = " << res << std::endl;
      if (res < 0) {
	// TODO
	// m_errno = errno;
	// goto the_end;
	assert (false);
      }
      
      const int val = 1;
#ifdef SO_REUSEADDR
      // Set reuse.
      if (setsockopt (m_accept_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val)) == -1) {
	// TODO
	// close (m_fd);
	// m_fd = -1;
	//m_errno = errno;
	return;
      }
#endif
      
#ifdef SO_REUSEPORT
      // Set reuse.
      if (setsockopt (m_accept_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof (val)) == -1) {
	// TODO
	// close (m_fd);
	// m_fd = -1;
	// m_errno = errno;
	return;
      }
#endif

      // Bind.
      if (::bind (m_accept_fd, address.get_sockaddr (), address.get_socklen ()) == -1) {
	perror ("bind");
	// TODO
	// close (m_fd);
	// m_fd = -1;
	// m_errno = errno;
	// return;
	assert (false);
      }

      add_observable (&accept_complete);
    }

  private:
    void schedule () const {
      if (accept_complete_precondition ()) {
	ioa::schedule (&tcp_acceptor::accept_complete);
      }
    }

    void observe (observable* o) {
      if (o == &accept_complete) {
	switch (accept_complete.recent_op) {
	case NOOP:
	  break;
	case BOUND:
	  {
	    if (ioa::bind_count (&tcp_acceptor::accept_complete) == 1) {
	      // Somebody bound us.  Now we should start listening.
	      if (listen (m_accept_fd, BACKLOG) == -1) {
		// TODO
		assert (false);
	      }
	      
	      // And accepting.
	      ioa::schedule_read_ready (&tcp_acceptor::read_ready, m_accept_fd);
	    }
	  }
	  break;
	case UNBOUND:
	  // TODO
	  std::cout << "unbound" << std::endl;
	  break;
	}
      }
      else {
	// Must be an automaton_manager<tcp_connection_automaton>.
	automaton_manager<tcp_connection_automaton>* conn = dynamic_cast<automaton_manager<tcp_connection_automaton>*> (o);
	assert (conn != 0);
	if (conn->get_handle () != -1) {
	  // A connection is now created.
	  m_connectionq.push (conn->get_handle ());
	  schedule ();
	}
      }
    }

    bool read_ready_precondition () const {
      return true;
    }

    void read_ready_effect () {
      std::cout << __func__ << std::endl;

      inet_address address;

      int fd = accept (m_accept_fd, address.get_sockaddr_ptr (), address.get_socklen_ptr ());
      if (fd == -1) {
	// TODO
	assert (false);
      }
      std::cout << "creating tcp_connection_automaton automaton using fd = " << fd << std::endl;
      
      add_observable (new automaton_manager<tcp_connection_automaton> (this, make_generator<tcp_connection_automaton> (fd)));
    }

    UP_INTERNAL (tcp_acceptor, read_ready);

    bool accept_complete_precondition () const {
      return !m_connectionq.empty () && ioa::bind_count (&tcp_acceptor::accept_complete) != 0;
    }
    
    automaton_handle<tcp_connection_automaton> accept_complete_effect () {
      automaton_handle<tcp_connection_automaton> handle = m_connectionq.front ();
      m_connectionq.pop ();
      return handle;
    }
    
  public:
    V_UP_OUTPUT (tcp_acceptor, accept_complete, automaton_handle<tcp_connection_automaton>);
  };

}

#endif

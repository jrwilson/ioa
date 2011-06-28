#ifndef __tcp_connector_hpp__
#define __tcp_connector_hpp__

#include <ioa/tcp_connection.hpp>

#include <iostream>
#include <fcntl.h>
#include <map>

namespace ioa {
  
  class tcp_connector :
    public automaton,
    private observer
  {
  private:
    std::map<aid_t, int> m_aid_to_fd;
    std::map<automaton_manager<tcp_connection>*, aid_t> m_manager_to_aid;
    std::map<aid_t, automaton_manager<tcp_connection>*> m_aid_to_manager;

    void schedule () const { }

    void observe (observable* o) {
      std::cout << __func__ << " " << o << std::endl;

      automaton_manager<tcp_connection>* conn = dynamic_cast<automaton_manager<tcp_connection> *> (o);
      assert (conn != 0);

      if (conn->get_handle () != -1) {
	std::cout << "aid " << m_manager_to_aid[conn] << " will have tcp_connection " << conn->get_handle () << std::endl;
	ioa::schedule (&tcp_connector::connect_complete, m_manager_to_aid[conn]);
      }
      
    }

    void connect_effect (const inet_address& address,
			 aid_t aid) {
      std::cout << __func__ << std::endl;
      std::cout << "address " << address.address_str () << " " << address.port () << std::endl;
      std::cout << "aid " << aid << std::endl;

      // Open a socket.
      int fd = socket (AF_INET, SOCK_STREAM, 0);
      std::cout << "fd = " << fd << std::endl;
      if (fd == -1) {
	// TODO
	// m_errno = errno;
	// goto the_end;
      }
      
      // Get the flags.
      int flags = fcntl (fd, F_GETFL, 0);
      std::cout << "flags = " << flags << std::endl;
      if (flags < 0) {
	// TODO
	// m_errno = errno;
	// goto the_end;
      }
      
      // Set non-blocking.
      flags |= O_NONBLOCK;
      int res = fcntl (fd, F_SETFL, flags);
      std::cout << "res = " << res << std::endl;
      if (res < 0) {
	// TODO
	// m_errno = errno;
	// goto the_end;
      }

      res = ::connect (fd, address.get_sockaddr (), address.get_socklen ());
      if (res != -1) {
	// TODO
	// Success.
      }
      else {
	if (errno == EINPROGRESS) {
	  // Asynchronous connect.
	  m_aid_to_fd.insert (std::make_pair (aid, fd));
	  ioa::schedule_write_ready (&ioa::tcp_connector::write_ready, aid, fd);
	}
	else {
	  // TODO
	  // m_errno = errno;
	  // goto the_end;
	}
      }
    }

  public:
    V_AP_INPUT (tcp_connector, connect, inet_address);

  private:

    bool write_ready_precondition (aid_t /* */) const {
      return true;
    }

    void write_ready_effect (aid_t aid) {
      std::cout << __func__ << std::endl;
      std::cout << "fd = " << m_aid_to_fd[aid] << std::endl;

      automaton_manager<tcp_connection>* conn = new automaton_manager<tcp_connection> (this, make_generator<tcp_connection> (m_aid_to_fd[aid]));
      add_observable (conn);
      m_manager_to_aid.insert (std::make_pair (conn, aid));
      m_aid_to_manager.insert (std::make_pair (aid, conn));

      // int res = ::connect (cp.fd, cp.address.get_sockaddr (), cp.address.get_socklen ());
      // if (res != -1) {
      // 	// TODO
      // 	// success
      // }
      // else {
      // 	std::cout << "res = " << res << std::endl;
      // 	perror ("connect");
      // }
    }

    P_INTERNAL (tcp_connector, write_ready, aid_t);

    bool connect_complete_precondition (aid_t aid) const {
      return m_aid_to_manager.count (aid) != 0 && 
	bind_count (&tcp_connector::connect_complete, aid) != 0;
    }

    automaton_handle<tcp_connection> connect_complete_effect (aid_t aid) {
      std::cout << __func__ << " " << aid << std::endl;
      automaton_handle<tcp_connection> retval = m_aid_to_manager[aid]->get_handle ();
      // TODO: Cleanup.
      return retval;
    }

  public:
    V_AP_OUTPUT (tcp_connector, connect_complete, automaton_handle<tcp_connection>);
  };

}

#endif

#ifndef __inet_address_hpp__
#define __inet_address_hpp__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <errno.h>

namespace ioa {

  class inet_address
  {
  private:
    int m_errno;
    socklen_t m_length;
    union {
      sockaddr_in m_addr4;
      sockaddr_in6 m_addr6;
    };

  public:

    inet_address () :
      m_errno (EINVAL),
      m_length (sizeof (sockaddr_in6))
    { }

    inet_address (const std::string& address,
  		  const unsigned short port) :
      m_errno (0)
    {
      // Try IPV4
      if (inet_pton (AF_INET, address.c_str (), &m_addr4.sin_addr.s_addr) == 1) {
	m_length = sizeof (sockaddr_in);
	m_addr4.sin_family = AF_INET;
	m_addr4.sin_port = htons (port);
      }
      else if (inet_pton (AF_INET6, address.c_str (), &m_addr6.sin6_addr.s6_addr) == 1) {
	m_length = sizeof (sockaddr_in6);
	m_addr6.sin6_family = AF_INET6;
	m_addr6.sin6_port = htons (port);
      }
      else {
	m_errno = EINVAL;
      }
    }    

    int get_errno () const {
      return m_errno;
    }

    const sockaddr* get_sockaddr () const {
      switch (m_length) {
      case sizeof (sockaddr_in):
	return reinterpret_cast<const sockaddr*> (&m_addr4);
	break;
      case sizeof (sockaddr_in6):
	return reinterpret_cast<const sockaddr*> (&m_addr6);
	break;
      }
      return 0;
    }

    sockaddr* get_sockaddr_ptr () {
      switch (m_length) {
      case sizeof (sockaddr_in):
	return reinterpret_cast<sockaddr*> (&m_addr4);
	break;
      case sizeof (sockaddr_in6):
	return reinterpret_cast<sockaddr*> (&m_addr6);
	break;
      }
      return 0;
    }

    socklen_t get_socklen () const {
      return m_length;
    }

    socklen_t* get_socklen_ptr () {
      return &m_length;
    }

    std::string address_str () const {
      if (errno == 0) {
	switch (m_length) {
	case sizeof (sockaddr_in):
	  {
	    char buf[INET_ADDRSTRLEN];
	    memset (buf, 0, INET_ADDRSTRLEN);
	    if (inet_ntop (AF_INET, &m_addr4.sin_addr.s_addr, buf, INET_ADDRSTRLEN) == buf) {
	      return std::string (buf, INET_ADDRSTRLEN);
	    }
	  }
	  break;
	case sizeof (sockaddr_in6):
	  {
	    char buf[INET6_ADDRSTRLEN];
	    memset (buf, 0, INET6_ADDRSTRLEN);
	    if (inet_ntop (AF_INET6, &m_addr6.sin6_addr.s6_addr, buf, INET6_ADDRSTRLEN) == buf) {
	      return std::string (buf, INET6_ADDRSTRLEN);
	    }
	  }
	  break;
	}
      }

      return std::string ();
    }

    unsigned short port () const {
      if (errno == 0) {
	switch (m_length) {
	case sizeof (sockaddr_in):
	  return ntohs (m_addr4.sin_port);
	  break;
	case sizeof (sockaddr_in6):
	  return ntohs (m_addr6.sin6_port);
	  break;
	}
      }

      return 0;
    }
  };

}

#endif

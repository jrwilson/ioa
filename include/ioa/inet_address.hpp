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

#ifndef __inet_address_hpp__
#define __inet_address_hpp__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <errno.h>
#include <cassert>

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
  		  const unsigned short port = 0) :
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
      if (m_errno == 0) {
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
      if (m_errno == 0) {
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

  class inet_mreq
  {
  private:
    socklen_t m_length;
    union {
      ip_mreq m_req4;
      ipv6_mreq m_req6;
    };

  public:
    inet_mreq (const inet_address& group_addr,
	       const inet_address& local_addr) {
      assert (group_addr.get_socklen () == local_addr.get_socklen ());
      switch (group_addr.get_socklen ()) {
      case sizeof (sockaddr_in):
	{
	  const sockaddr_in* a = reinterpret_cast<const sockaddr_in*> (group_addr.get_sockaddr ());
	  memcpy (&m_req4.imr_multiaddr, &a->sin_addr, sizeof (in_addr));
	  const sockaddr_in* b = reinterpret_cast<const sockaddr_in*> (local_addr.get_sockaddr ());
	  memcpy (&m_req4.imr_interface, &b->sin_addr, sizeof (in_addr));
	  m_length = sizeof (ip_mreq);
	}
	break;
      case sizeof (sockaddr_in6):
	// IPv6 Multicast not supported.
	m_length = 0;
	break;
      default:
	m_length = 0;
	break;
      }
    }

    const void* get_mreq () const {
      switch (m_length) {
      case sizeof (ip_mreq):
	return &m_req4;
      default:
	return 0;
      }
    }

    socklen_t get_length () const {
      return m_length;
    }
    
  };

}

#endif

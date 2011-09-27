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

#include <ioa/udp_receiver_automaton.hpp>

#include <fcntl.h>
#include <sys/ioctl.h>

namespace ioa {

  void udp_receiver_automaton::prepare_socket (const inet_address& address) {
    // Open a socket.
    m_fd = socket (AF_INET, SOCK_DGRAM, 0);
    if (m_fd == -1) {
      m_errno = errno;
      return;
    }

    // Get the flags.
    int flags = fcntl (m_fd, F_GETFL, 0);
    if (flags < 0) {
      // Set errno before close because close sets errno.
      m_errno = errno;
      return;
    }

    // Set non-blocking.
    flags |= O_NONBLOCK;
    if (fcntl (m_fd, F_SETFL, flags) == -1) {
      m_errno = errno;
      return;
    }

    const int val = 1;
#ifdef SO_REUSEADDR
    // Set reuse.
    if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val)) == -1) {
      m_errno = errno;
      return;
    }
#endif

#ifdef SO_REUSEPORT
    // Set reuse.
    if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof (val)) == -1) {
      m_errno = errno;
      return;
    }
#endif

    // Bind.
    if (::bind (m_fd, address.get_sockaddr (), address.get_socklen ()) == -1) {
      m_errno = errno;
      return;
    }
  }

  void udp_receiver_automaton::schedule () const {
    if (receive_precondition ()) {
      ioa::schedule (&udp_receiver_automaton::receive);
    }
    if (error_precondition ()) {
      ioa::schedule (&udp_receiver_automaton::error);
    }
    if (schedule_read_ready_precondition ()) {
      ioa::schedule (&udp_receiver_automaton::schedule_read_ready);
    }
  }

  udp_receiver_automaton::udp_receiver_automaton (const inet_address& address) :
    m_state (SCHEDULE_READ_READY),
    m_fd (-1),
    m_errno (0),
    m_buf (0),
    m_buf_size (0),
    m_error_reported (false)
  {
    prepare_socket (address);
    schedule ();
  }

  udp_receiver_automaton::udp_receiver_automaton (const inet_address& group_addr,
						  const inet_address& local_addr) :
    m_state (SCHEDULE_READ_READY),
    m_fd (-1),
    m_errno (0),
    m_buf (0),
    m_buf_size (0),
    m_error_reported (false)
  {
    prepare_socket (local_addr);

    inet_mreq req (group_addr, local_addr);
    // Join the multicast group.
    if (setsockopt (m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, req.get_mreq (), req.get_length ()) == -1) {
      m_errno = errno;
    }

    schedule ();
  }

  udp_receiver_automaton::~udp_receiver_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }
  }

  bool udp_receiver_automaton::receive_precondition () const {
    return !m_recv_queue.empty ();
  }

  udp_receiver_automaton::receive_val udp_receiver_automaton::receive_effect () {
    receive_val retval = m_recv_queue.front ();
    m_recv_queue.pop ();
    return retval;
  }

  void udp_receiver_automaton::receive_schedule () const {
    schedule ();
  }
  
  bool udp_receiver_automaton::error_precondition () const {
    return m_errno != 0 && m_error_reported == false && binding_count (&udp_receiver_automaton::error) != 0;
  }

  int udp_receiver_automaton::error_effect () {
    m_error_reported = true;
    return m_errno;
  }

  void udp_receiver_automaton::error_schedule () const {
    schedule ();
  }

  bool udp_receiver_automaton::schedule_read_ready_precondition () const {
    return m_errno == 0 && m_state == SCHEDULE_READ_READY;
  }

  void udp_receiver_automaton::schedule_read_ready_effect () {
    ioa::schedule_read_ready (&udp_receiver_automaton::read_ready, m_fd);
    m_state = READ_READY_WAIT;
  }

  void udp_receiver_automaton::schedule_read_ready_schedule () const {
    schedule ();
  }

  bool udp_receiver_automaton::read_ready_precondition () const {
    return m_errno == 0 && m_state == READ_READY_WAIT;
  }

  void udp_receiver_automaton::read_ready_effect () {
    m_state = SCHEDULE_READ_READY;

    int expect_bytes;
    int res = ioctl (m_fd, FIONREAD, &expect_bytes);
    if (res == -1) {
      m_errno = errno;
      return;
    }
      
    // Resize the buffer.
    if (m_buf_size < expect_bytes) {
      delete[] m_buf;
      m_buf = new char[expect_bytes];
      m_buf_size = expect_bytes;
    }
      
    // Read.
    inet_address address;
    ssize_t actual_bytes = recvfrom (m_fd, m_buf, expect_bytes, 0, address.get_sockaddr_ptr (), address.get_socklen_ptr ());

    if (actual_bytes != -1 && actual_bytes != 0) {
      // Success.
      m_recv_queue.push (receive_val (address, std::string (m_buf, actual_bytes)));
    }
    else {
      assert (errno != 0);
      m_errno = errno;
    }
  }

  void udp_receiver_automaton::read_ready_schedule () const {
    schedule ();
  }

};

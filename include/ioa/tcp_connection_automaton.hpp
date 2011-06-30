#ifndef __tcp_connection_automaton_hpp__
#define __tcp_connection_automaton_hpp__

#include <ioa/ioa.hpp>
#include <ioa/buffer.hpp>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>

namespace ioa {
  
  class tcp_connection_automaton :
    public automaton,
    private observer
  {
  public:
    struct receive_val
    {
      int err;
      const_shared_ptr<ioa::buffer> buffer;
      
      receive_val (const int e,
		   const const_shared_ptr<ioa::buffer>& b) :
	err (e),
	buffer (b)
      { }
    };

  private:
    enum send_state_t {
      SEND_WAIT,
      SCHEDULE_WRITE_READY,
      WRITE_READY_WAIT,
      SEND_COMPLETE_READY,
    };

    enum receive_state_t {
      SCHEDULE_READ_READY,
      READ_READY_WAIT,
      RECEIVE_READY,
    };

    int m_fd;
    send_state_t m_send_state;
    int m_send_errno;
    const_shared_ptr<buffer_interface> m_send_buffer;
    ssize_t m_bytes_written;
    receive_state_t m_receive_state;
    int m_receive_errno;
    const_shared_ptr<buffer> m_receive_buffer;

  public:
    tcp_connection_automaton (const int fd);
    ~tcp_connection_automaton ();

  private:
    void schedule () const;
    void observe (observable* o);

  private:
    void send_effect (const const_shared_ptr<buffer_interface>& buf);
  public:
    V_UP_INPUT (tcp_connection_automaton, send, const_shared_ptr<buffer_interface>);

  private:
    bool schedule_write_precondition () const;
    void schedule_write_effect ();
    UP_INTERNAL (tcp_connection_automaton, schedule_write);

    bool write_ready_precondition () const;
    void write_ready_effect ();
    UP_INTERNAL (tcp_connection_automaton, write_ready);

  private:
    bool send_complete_precondition () const;
    int send_complete_effect ();
  public:
    V_UP_OUTPUT (tcp_connection_automaton, send_complete, int);

  private:
    bool schedule_read_precondition () const;
    void schedule_read_effect ();
    UP_INTERNAL (tcp_connection_automaton, schedule_read);

    bool read_ready_precondition () const;
    void read_ready_effect ();
    UP_INTERNAL (tcp_connection_automaton, read_ready);

  private:
    bool receive_precondition () const;
    receive_val receive_effect ();
  public:
    V_UP_OUTPUT (tcp_connection_automaton, receive, receive_val);
  };

}

#endif

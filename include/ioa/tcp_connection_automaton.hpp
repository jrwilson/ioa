#ifndef __tcp_connection_automaton_hpp__
#define __tcp_connection_automaton_hpp__

#include <ioa/ioa.hpp>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <string>

namespace ioa {
  
  class tcp_connection_automaton :
    public automaton,
    private observer
  {
  public:
    struct receive_val
    {
      int err;
      const_shared_ptr<std::string> buffer;
      
      receive_val (const int e,
		   const const_shared_ptr<std::string>& b) :
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
    const_shared_ptr<std::string> m_send_buffer;
    ssize_t m_bytes_written;
    receive_state_t m_receive_state;
    int m_receive_errno;
    const_shared_ptr<std::string> m_receive_buffer;
    char* m_buffer;
    ssize_t m_buffer_size;

  public:
    tcp_connection_automaton (const int fd);
    ~tcp_connection_automaton ();

  private:
    void schedule () const;
    void observe (observable* o);

  private:
    void send_effect (const const_shared_ptr<std::string>& buf);
    void send_schedule () const { schedule (); }
  public:
    V_UP_INPUT (tcp_connection_automaton, send, const_shared_ptr<std::string>);

  private:
    bool schedule_write_precondition () const;
    void schedule_write_effect ();
    void schedule_write_schedule () const { schedule (); }
    UP_INTERNAL (tcp_connection_automaton, schedule_write);

    bool write_ready_precondition () const;
    void write_ready_effect ();
    void write_ready_schedule () const { schedule (); }
    UP_INTERNAL (tcp_connection_automaton, write_ready);

  private:
    bool send_complete_precondition () const;
    int send_complete_effect ();
    void send_complete_schedule () const { schedule (); }
  public:
    V_UP_OUTPUT (tcp_connection_automaton, send_complete, int);

  private:
    bool schedule_read_precondition () const;
    void schedule_read_effect ();
    void schedule_read_schedule () const { schedule (); }
    UP_INTERNAL (tcp_connection_automaton, schedule_read);

    bool read_ready_precondition () const;
    void read_ready_effect ();
    void read_ready_schedule () const { schedule (); }
    UP_INTERNAL (tcp_connection_automaton, read_ready);

  private:
    bool receive_precondition () const;
    receive_val receive_effect ();
    void receive_schedule () const { schedule (); }
  public:
    V_UP_OUTPUT (tcp_connection_automaton, receive, receive_val);
  };

}

#endif

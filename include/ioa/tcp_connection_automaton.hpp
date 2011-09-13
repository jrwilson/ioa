#ifndef __tcp_connection_automaton_hpp__
#define __tcp_connection_automaton_hpp__

#include <ioa/ioa.hpp>
#include <ioa/inet_address.hpp>
#include <string>

namespace ioa {

  class tcp_connection_automaton :
    public virtual automaton
  {
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
    int m_errno;
    bool m_connected_reported;
    bool m_error_reported;
    send_state_t m_send_state;
    std::string m_send_buffer;
    ssize_t m_bytes_written;
    receive_state_t m_receive_state;
    std::string m_receive_buffer;
    char* m_buffer;
    ssize_t m_buffer_size;

  public:
    tcp_connection_automaton ();
    ~tcp_connection_automaton ();

  private:
    void schedule () const;

  private:
    void send_effect (const std::string& buf);
    void send_schedule () const;
  public:
    V_UP_INPUT (tcp_connection_automaton, send, std::string);

  private:
    bool schedule_write_precondition () const;
    void schedule_write_effect ();
    void schedule_write_schedule () const;
    UP_INTERNAL (tcp_connection_automaton, schedule_write);

    bool write_ready_precondition () const;
    void write_ready_effect ();
    void write_ready_schedule () const;
    UP_INTERNAL (tcp_connection_automaton, write_ready);

  private:
    bool send_complete_precondition () const;
    void send_complete_effect ();
    void send_complete_schedule () const;
  public:
    UV_UP_OUTPUT (tcp_connection_automaton, send_complete);

  private:
    bool schedule_read_precondition () const;
    void schedule_read_effect ();
    void schedule_read_schedule () const;
    UP_INTERNAL (tcp_connection_automaton, schedule_read);

    bool read_ready_precondition () const;
    void read_ready_effect ();
    void read_ready_schedule () const;
    UP_INTERNAL (tcp_connection_automaton, read_ready);

  private:
    bool receive_precondition () const;
    std::string receive_effect ();
    void receive_schedule () const;
  public:
    V_UP_OUTPUT (tcp_connection_automaton, receive, std::string);

  private:
    void init_effect (const int&);
    void init_schedule () const;
  public:
    V_UP_INPUT (tcp_connection_automaton, init, int);

  private:
    bool connected_precondition () const;
    void connected_effect ();
    void connected_schedule () const;
  public:
    UV_UP_OUTPUT (tcp_connection_automaton, connected);

  private:
    bool error_precondition () const;
    int error_effect ();
    void error_schedule () const;
  public:
    V_UP_OUTPUT (tcp_connection_automaton, error, int);
  };

  class connection_init_automaton :
    public virtual automaton,
    private observer
  {
  private:
    enum state_t {
      START,
      SUCCESS,
      FAIL,
      STOP
    };
    state_t m_state;
    handle_manager<connection_init_automaton> m_self;
    handle_manager<tcp_connection_automaton> m_conn;
    int m_fd;
    
  public:
    connection_init_automaton (const automaton_handle<tcp_connection_automaton>& conn,
			       int fd);
    
  private:
    void observe (observable* o);
    void schedule () const;
    
    bool init_precondition () const;
    int init_effect ();
    void init_schedule () const;
    V_UP_OUTPUT (connection_init_automaton, init, int);
    
    bool done_precondition () const;
    int done_effect ();
    void done_schedule () const;
  public:
    V_UP_OUTPUT (connection_init_automaton, done, int);
  };

}

#endif

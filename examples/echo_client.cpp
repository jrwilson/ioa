#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>
#include <ioa/inet_address.hpp>
#include <ioa/tcp_connector_automaton.hpp>

#include <iostream>
#include <queue>
#include <sys/ioctl.h>

class stdin_automaton :
  public ioa::automaton
{
private:
  enum state_t {
    SCHEDULE_READ_READY,
    READ_READY_WAIT,
  };

  state_t m_state;
  int m_fd;
  bool m_closed;
  bool m_closed_reported;
  ssize_t m_buf_size;
  char* m_buf;
  std::queue<std::string*> m_recv_queue;

  void schedule () const {
    if (schedule_read_ready_precondition ()) {
      ioa::schedule (&stdin_automaton::schedule_read_ready);
    }
    if (receive_precondition ()) {
      ioa::schedule (&stdin_automaton::receive);
    }
    if (closed_precondition ()) {
      ioa::schedule (&stdin_automaton::closed);
    }
  }

public:
  stdin_automaton  () :
    m_state (SCHEDULE_READ_READY),
    m_fd (0),
    m_closed (false),
    m_closed_reported (false),
    m_buf_size (0),
    m_buf (0) {
    schedule ();
  }

  ~stdin_automaton () {
    ioa::close (m_fd);
    while (!m_recv_queue.empty ()) {
      delete m_recv_queue.front ();
      m_recv_queue.pop ();
    }
  }

private:
  bool schedule_read_ready_precondition () const {
    return !m_closed && m_state == SCHEDULE_READ_READY;
  }

  void schedule_read_ready_effect () {
    m_state = READ_READY_WAIT;
    ioa::schedule_read_ready (&stdin_automaton::read_ready, m_fd);
  }

  void schedule_read_ready_schedule () const {
    schedule ();
  }

  UP_INTERNAL (stdin_automaton, schedule_read_ready);

  bool read_ready_precondition () const {
    return !m_closed && m_state == READ_READY_WAIT;
  }

  void read_ready_effect () {
    m_state = SCHEDULE_READ_READY;

    int expect_bytes;
    int res = ioctl (m_fd, FIONREAD, &expect_bytes);
    if (res == -1) {
      m_closed = true;
      return;
    }
      
    // Resize the buffer.
    if (m_buf_size < expect_bytes) {
      delete[] m_buf;
      m_buf = new char[expect_bytes];
      m_buf_size = expect_bytes;
    }
      
    // Read.
    ssize_t actual_bytes = read (m_fd, m_buf, expect_bytes);

    if (actual_bytes != -1 && actual_bytes != 0) {
      // Success.
      m_recv_queue.push (new std::string (m_buf, actual_bytes));
    }
    else {
      m_closed = true;
    }
  }

  void read_ready_schedule () const {
    schedule ();
  }

  UP_INTERNAL (stdin_automaton, read_ready);

  bool receive_precondition () const {
    return !m_recv_queue.empty () && ioa::binding_count (&stdin_automaton::receive) != 0;
  }

  std::string receive_effect () {
    std::string retval = *m_recv_queue.front ();
    delete m_recv_queue.front ();
    m_recv_queue.pop ();
    return retval;
  }

  void receive_schedule () const {
    schedule ();
  }

public:
  V_UP_OUTPUT (stdin_automaton, receive, std::string);

private:
  bool closed_precondition () const {
    return m_closed && !m_closed_reported;
  }

  void closed_effect () {
    m_closed_reported = true;
  }

  void closed_schedule () const {
    schedule ();
  }

public:
  UV_UP_OUTPUT (stdin_automaton, closed);
};

class echo_client_automaton :
  public ioa::automaton,
  private ioa::observer
{
private:
  enum state_t {
    SEND_READY,
    SEND_COMPLETE_WAIT,
  };

  state_t m_state;
  ioa::handle_manager<echo_client_automaton> m_self;
  ioa::automaton_manager<ioa::tcp_connection_automaton>* m_connection;
  ioa::inet_address m_address;
  bool m_created_flag;
  ioa::automaton_manager<stdin_automaton>* m_s;
  std::queue<std::string*> m_send_queue;

public:
  echo_client_automaton (const ioa::inet_address& address) :
    m_state (SEND_READY),
    m_self (ioa::get_aid ()),
    m_address (address),
    m_created_flag (false)
  {
    m_s = ioa::make_automaton_manager (this, ioa::make_generator<stdin_automaton> ());
    
    ioa::make_binding_manager (this,
    			       m_s, &stdin_automaton::receive,
    			       &m_self, &echo_client_automaton::stdin_receive);

    ioa::make_binding_manager (this,
    			       m_s, &stdin_automaton::closed,
    			       &m_self, &echo_client_automaton::stdin_closed);

    m_connection = ioa::make_automaton_manager (this, ioa::make_generator<ioa::tcp_connection_automaton> ());
    add_observable (m_connection);

    ioa::make_binding_manager (this,
  			       &m_self, &echo_client_automaton::send,
  			       m_connection, &ioa::tcp_connection_automaton::send);
    
    ioa::make_binding_manager (this,
  			       m_connection, &ioa::tcp_connection_automaton::send_complete,
  			       &m_self, &echo_client_automaton::send_complete);
    
    ioa::make_binding_manager (this,
  			       m_connection, &ioa::tcp_connection_automaton::receive,
  			       &m_self, &echo_client_automaton::receive);
    
    ioa::make_binding_manager (this,
  			       m_connection, &ioa::tcp_connection_automaton::error,
  			       &m_self, &echo_client_automaton::connection_error);

    schedule ();
  }

  ~echo_client_automaton () {
    while (!m_send_queue.empty ()) {
      delete m_send_queue.front ();
      m_send_queue.pop ();
    }
  }

private:
  void schedule () const {
    if (create_precondition ()) {
      ioa::schedule (&echo_client_automaton::create);
    }
    if (send_precondition ()) {
      ioa::schedule (&echo_client_automaton::send);
    }
  }

  void observe(ioa::observable*) {
    schedule ();
  }

  void stdin_receive_effect (const std::string& val) {
    m_send_queue.push (new std::string (val));
  }

  void stdin_receive_schedule () const {
    schedule ();
  }

  V_UP_INPUT (echo_client_automaton, stdin_receive, std::string);

  void stdin_closed_effect () {
    m_connection->destroy ();
  }

  void stdin_closed_schedule () const {
    schedule ();
  }

  UV_UP_INPUT (echo_client_automaton, stdin_closed);

  bool create_precondition () const {
    return m_connection != 0 && m_connection->get_state () == ioa::automaton_manager_interface::CREATED && !m_created_flag;
  }

  void create_effect () {
    m_created_flag = true;
    ioa::automaton_manager<ioa::tcp_connector_automaton>* connector = new ioa::automaton_manager<ioa::tcp_connector_automaton> (this, ioa::make_generator<ioa::tcp_connector_automaton> (m_address, m_connection->get_handle ()));
    
    ioa::make_binding_manager (this,
    			       connector, &ioa::tcp_connector_automaton::error,
    			       &m_self, &echo_client_automaton::connector_error);
  }

  void create_schedule () const {
    schedule ();
  }

  UP_INTERNAL (echo_client_automaton, create);

  void connector_error_effect (const int& err) {
    char buf[256];
#ifdef STRERROR_R_CHAR_P
    std::cerr << "Couldn't connect: " << strerror_r (err, buf, 256) << std::endl;
#else
    strerror_r (err, buf, 256);
    std::cerr << "Couldn't connect: " << buf << std::endl;
#endif

    m_s->destroy ();
  }
  
  void connector_error_schedule () const {
    schedule ();
  }
  
  V_UP_INPUT (echo_client_automaton, connector_error, int);
  
  bool send_precondition () const {
    return m_state == SEND_READY && !m_send_queue.empty () && ioa::binding_count (&echo_client_automaton::send) != 0;
  }

  std::string send_effect () {
    m_state = SEND_COMPLETE_WAIT;
    std::string retval = *m_send_queue.front ();
    delete m_send_queue.front ();
    m_send_queue.pop ();
    return retval;
  }

  void send_schedule () const {
    schedule ();
  }

  V_UP_OUTPUT (echo_client_automaton, send, std::string);

  void send_complete_effect () {
    m_state = SEND_READY;
  }

  void send_complete_schedule () const {
    schedule ();
  }

  UV_UP_INPUT (echo_client_automaton, send_complete);

  void receive_effect (const std::string& val) {
    std::cout << val;
  }
  
  void receive_schedule () const {
    schedule ();
  }

  V_UP_INPUT (echo_client_automaton, receive, std::string);

  void connection_error_effect (const int& err) {
    char buf[256];
#ifdef STRERROR_R_CHAR_P
    std::cerr << "Error: " << strerror_r (err, buf, 256) << std::endl;
#else
    strerror_r (err, buf, 256);
    std::cerr << "Error: " << buf << std::endl;
#endif

    m_s->destroy ();
  }
  
  void connection_error_schedule () const {
    schedule ();
  }
  
  V_UP_INPUT (echo_client_automaton, connection_error, int);

};

int main () {
  // TODO
  ioa::inet_address address ("127.0.0.1", 54321);
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<echo_client_automaton> (address));
  return 0;
}

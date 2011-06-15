#include <ioa/udp_broadcast_automaton.hpp>
#include <ioa/simple_scheduler.hpp>

#include <iostream>

class periodic_broadcaster :
  public ioa::automaton_interface
{
private:
  enum state_t {
    OPEN_READY,
    OPEN_WAIT,
    SEND_READY,
    SEND_WAIT,
    SEND_SLEEP,
    ERROR
  };
  
  state_t m_state;
  std::auto_ptr<ioa::self_helper<periodic_broadcaster> > m_self;
  unsigned short m_port;
  ioa::time m_period;
  ioa::buffer m_buffer;

  bool open_precondition () const {
    return m_state == OPEN_READY && ioa::bind_count (&periodic_broadcaster::open) != 0;
  }

  ioa::udp_broadcast_automaton::open_arg open_action () {
    m_state = OPEN_WAIT;
    schedule ();
    return ioa::udp_broadcast_automaton::open_arg ("255.255.255.255", m_port);
  }

  V_UP_OUTPUT (periodic_broadcaster, open, ioa::udp_broadcast_automaton::open_arg);

  void open_result_action (const int& result) {
    assert (m_state == OPEN_WAIT);

    if (result != 0) {
      char buf[256];
      strerror_r (result, buf, 256);
      std::cerr << "Couldn't open udp_broadcast_automaton: " << buf << std::endl;
      m_state = ERROR;
    }
    else {
      m_state = SEND_READY;
    }
    schedule ();
  }

  V_UP_INPUT (periodic_broadcaster, open_result, int);

  bool send_precondition () const {
    return m_state == SEND_READY && ioa::bind_count (&periodic_broadcaster::send) != 0;
  }

  ioa::buffer send_action () {
    m_state = SEND_WAIT;
    schedule ();
    return m_buffer;
  }
  V_UP_OUTPUT (periodic_broadcaster, send, ioa::buffer);

  void send_result_action (const int& result) {
    assert (m_state == SEND_WAIT);

    if (result != 0) {
      char buf[256];
      strerror_r (result, buf, 256);
      std::cerr << "Couldn't send udp_broadcast_automaton: " << buf << std::endl;
      m_state = ERROR;
    }
    else {
      m_state = SEND_SLEEP;
    }
    schedule ();
  }

  V_UP_INPUT (periodic_broadcaster, send_result, int);

  bool tick_precondition () const {
    return true;
  }

  void tick_action () {
    if (m_state == SEND_SLEEP) {
      m_state = SEND_READY;
    }
    schedule ();
    ioa::schedule_after (&periodic_broadcaster::tick, m_period);
  }

  UP_INTERNAL (periodic_broadcaster, tick);

  void schedule () const {
    if (open_precondition ()) {
      ioa::schedule (&periodic_broadcaster::open);
    }
    if (send_precondition ()) {
      ioa::schedule (&periodic_broadcaster::send);
    }
  }

public:
  periodic_broadcaster (const unsigned short port,
			const ioa::time period,
			const std::string message) :
    m_state (OPEN_READY),
    m_self (new ioa::self_helper<periodic_broadcaster> ()),
    m_port (port),
    m_period (period),
    m_buffer (message.size (), message.c_str ())
  {
    ioa::automaton_helper<ioa::udp_broadcast_automaton>* sender = new ioa::automaton_helper<ioa::udp_broadcast_automaton> (this, ioa::make_generator<ioa::udp_broadcast_automaton> ());

    ioa::make_bind_helper (this, m_self.get (), &periodic_broadcaster::open, sender, &ioa::udp_broadcast_automaton::open);
    ioa::make_bind_helper (this, sender, &ioa::udp_broadcast_automaton::open_result, m_self.get (), &periodic_broadcaster::open_result);
    ioa::make_bind_helper (this, m_self.get (), &periodic_broadcaster::send, sender, &ioa::udp_broadcast_automaton::send);
    ioa::make_bind_helper (this, sender, &ioa::udp_broadcast_automaton::send_result, m_self.get (), &periodic_broadcaster::send_result);

    schedule ();
    ioa::schedule_after (&periodic_broadcaster::tick, m_period);
  }

};

int
main (int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " PORT PERIOD MESSAGE" << std::endl;
    exit (EXIT_FAILURE);
  }

  unsigned short port = atoi (argv[1]);
  ioa::time period (atoi (argv[2]), 0);
  std::string message (argv[3]);

  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<periodic_broadcaster> (port, period, message));
  return 0; 
}

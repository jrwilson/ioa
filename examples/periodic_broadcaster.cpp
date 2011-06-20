#include <ioa/udp_broadcast_sender_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <iostream>

class periodic_broadcaster :
  public ioa::automaton
{
private:
  enum state_t {
    SEND_READY,
    SEND_COMPLETE_WAIT,
    SEND_COMPLETE,
    ERROR
  };
  
  state_t m_state;
  std::auto_ptr<ioa::self_helper<periodic_broadcaster> > m_self;
  ioa::ipv4_address m_address;
  ioa::time m_period;
  ioa::buffer m_buffer;

  bool send_precondition () const {
    return m_state == SEND_READY && ioa::bind_count (&periodic_broadcaster::send) != 0;
  }

  ioa::udp_broadcast_sender_automaton::send_arg send_action () {
    assert (m_state == SEND_READY);
    m_state = SEND_COMPLETE_WAIT;
    schedule ();
    return ioa::udp_broadcast_sender_automaton::send_arg (&m_address, m_buffer);
  }

  V_UP_OUTPUT (periodic_broadcaster, send, ioa::udp_broadcast_sender_automaton::send_arg);

  void send_complete_action (const int& result) {
    assert (m_state == SEND_COMPLETE_WAIT);

    if (result != 0) {
      char buf[256];
      strerror_r (result, buf, 256);
      std::cerr << "Couldn't send udp_broadcast_sender_automaton: " << buf << std::endl;
      m_state = ERROR;
    }
    else {
      m_state = SEND_COMPLETE;
    }
    schedule ();
  }

  V_UP_INPUT (periodic_broadcaster, send_complete, int);

  void schedule () const {
    if (send_precondition ()) {
      ioa::schedule (&periodic_broadcaster::send);
    }
  }

public:
  periodic_broadcaster (const unsigned short port,
			const ioa::time period,
			const std::string message) :
    m_state (SEND_READY),
    m_self (new ioa::self_helper<periodic_broadcaster> ()),
    m_address ("255.255.255.255", port),
    m_period (period),
    m_buffer (message.size (), message.c_str ())
  {
    ioa::automaton_helper<ioa::udp_broadcast_sender_automaton>* sender = new ioa::automaton_helper<ioa::udp_broadcast_sender_automaton> (this, ioa::make_generator<ioa::udp_broadcast_sender_automaton> ());

    ioa::make_bind_helper (this, m_self.get (), &periodic_broadcaster::send, sender, &ioa::udp_broadcast_sender_automaton::send);
    ioa::make_bind_helper (this, sender, &ioa::udp_broadcast_sender_automaton::send_complete, m_self.get (), &periodic_broadcaster::send_complete);

    schedule ();
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

  ioa::global_fifo_scheduler ss;
  ioa::run (ss, ioa::make_generator<periodic_broadcaster> (port, period, message));
  return 0; 
}

#include <ioa/udp_broadcast_sender_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <iostream>

class broadcast_sender :
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
  std::auto_ptr<ioa::self_helper<broadcast_sender> > m_self;
  ioa::ipv4_address m_address;
  ioa::buffer m_buffer;

  bool send_precondition () const {
    return m_state == SEND_READY && ioa::bind_count (&broadcast_sender::send) != 0;
  }

  ioa::udp_broadcast_sender_automaton::send_arg send_action () {
    assert (m_state == SEND_READY);
    m_state = SEND_COMPLETE_WAIT;
    schedule ();
    return ioa::udp_broadcast_sender_automaton::send_arg (&m_address, m_buffer);
  }

  V_UP_OUTPUT (broadcast_sender, send, ioa::udp_broadcast_sender_automaton::send_arg);

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

  V_UP_INPUT (broadcast_sender, send_complete, int);

  void schedule () const {
    if (send_precondition ()) {
      ioa::schedule (&broadcast_sender::send);
    }
  }

public:
  broadcast_sender (const unsigned short port,
			const std::string message) :
    m_state (SEND_READY),
    m_self (new ioa::self_helper<broadcast_sender> ()),
    m_address ("255.255.255.255", port),
    m_buffer (message.size (), message.c_str ())
  {
    ioa::automaton_helper<ioa::udp_broadcast_sender_automaton>* sender = new ioa::automaton_helper<ioa::udp_broadcast_sender_automaton> (this, ioa::make_generator<ioa::udp_broadcast_sender_automaton> ());

    ioa::make_bind_helper (this, m_self.get (), &broadcast_sender::send, sender, &ioa::udp_broadcast_sender_automaton::send);
    ioa::make_bind_helper (this, sender, &ioa::udp_broadcast_sender_automaton::send_complete, m_self.get (), &broadcast_sender::send_complete);

    schedule ();
  }

};

int
main (int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " PORT MESSAGE" << std::endl;
    exit (EXIT_FAILURE);
  }

  unsigned short port = atoi (argv[1]);
  std::string message (argv[2]);

  ioa::global_fifo_scheduler ss;
  ioa::run (ss, ioa::make_generator<broadcast_sender> (port, message));
  return 0; 
}

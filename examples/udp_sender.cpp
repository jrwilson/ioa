#include <ioa/udp_sender_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <config.hpp>

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
  ioa::self_manager<broadcast_sender> m_self;
  ioa::inet_address m_address;
  ioa::buffer m_buffer;

public:

  broadcast_sender (const std::string& address,
		    const unsigned short port,
		    const std::string& message) :
    m_state (SEND_READY),
    m_address (address, port),
    m_buffer (message.c_str (), message.size ())
  {
    ioa::automaton_manager<ioa::udp_sender_automaton>* sender = new ioa::automaton_manager<ioa::udp_sender_automaton> (this, ioa::make_generator<ioa::udp_sender_automaton> ());

    ioa::make_bind_helper (this, &m_self, &broadcast_sender::send, sender, &ioa::udp_sender_automaton::send);
    ioa::make_bind_helper (this, sender, &ioa::udp_sender_automaton::send_complete, &m_self, &broadcast_sender::send_complete);

    schedule ();
  }

  ~broadcast_sender () {
    if (!(m_state == SEND_COMPLETE || m_state == ERROR)) {
      std::cerr << "Didn't receive send complete from udp_sender_automaton." << std::endl;
    }
  }

private:
  void schedule () const {
    if (send_precondition ()) {
      ioa::schedule (&broadcast_sender::send);
    }
  }

  bool send_precondition () const {
    return m_state == SEND_READY && ioa::bind_count (&broadcast_sender::send) != 0;
  }

  ioa::udp_sender_automaton::send_arg send_effect () {
    assert (m_state == SEND_READY);
    m_state = SEND_COMPLETE_WAIT;
    return ioa::udp_sender_automaton::send_arg (m_address, m_buffer);
  }

  V_UP_OUTPUT (broadcast_sender, send, ioa::udp_sender_automaton::send_arg);

  void send_complete_effect (const int& result) {
    assert (m_state == SEND_COMPLETE_WAIT);

    if (result != 0) {
      char buf[256];
#ifdef STRERROR_R_CHAR_P
      std::cerr << "Couldn't send udp_sender_automaton: " << strerror_r (result, buf, 256) << std::endl;
#else
      strerror_r (result, buf, 256);
      std::cerr << "Couldn't send udp_sender_automaton: " << buf << std::endl;
#endif
      m_state = ERROR;
    }
    else {
      m_state = SEND_COMPLETE;
    }
  }

  V_UP_INPUT (broadcast_sender, send_complete, int);

};

int
main (int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " ADDRESS PORT MESSAGE" << std::endl;
    exit (EXIT_FAILURE);
  }

  std::string address (argv[1]);
  unsigned short port = atoi (argv[2]);
  std::string message (argv[3]);

  ioa::global_fifo_scheduler ss;
  ioa::run (ss, ioa::make_generator<broadcast_sender> (address, port, message));
  return 0; 
}

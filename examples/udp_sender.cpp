#include <config.hpp>

#include <ioa/udp_sender_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <iostream>

class udp_sender :
  public ioa::automaton
{
private:
  enum state_t {
    SEND_READY,
    SEND_COMPLETE_WAIT,
  };
  
  state_t m_state;
  ioa::handle_manager<udp_sender> m_self;
  ioa::inet_address m_address;
  ioa::const_shared_ptr<std::string> m_message;

public:

  udp_sender (const ioa::inet_address& address,
	      const ioa::const_shared_ptr<std::string>& message) :
    m_state (SEND_READY),
    m_self (ioa::get_aid ()),
    m_address (address),
    m_message (message)
  {
    ioa::automaton_manager<ioa::udp_sender_automaton>* sender = new ioa::automaton_manager<ioa::udp_sender_automaton> (this, ioa::make_generator<ioa::udp_sender_automaton> ());

    ioa::make_binding_manager (this, &m_self, &udp_sender::send, sender, &ioa::udp_sender_automaton::send);
    ioa::make_binding_manager (this, sender, &ioa::udp_sender_automaton::send_complete, &m_self, &udp_sender::send_complete);
    ioa::make_binding_manager (this, sender, &ioa::udp_sender_automaton::error, &m_self, &udp_sender::error);

    schedule ();
  }

private:
  void schedule () const {
    if (send_precondition ()) {
      ioa::schedule (&udp_sender::send);
    }
  }

  bool send_precondition () const {
    return m_state == SEND_READY && ioa::binding_count (&udp_sender::send) != 0;
  }

  ioa::udp_sender_automaton::send_arg send_effect () {
    assert (m_state == SEND_READY);
    m_state = SEND_COMPLETE_WAIT;
    return ioa::udp_sender_automaton::send_arg (m_address, m_message);
  }

  void send_schedule () const {
    schedule ();
  }

  V_UP_OUTPUT (udp_sender, send, ioa::udp_sender_automaton::send_arg);

  void send_complete_effect () {
    std::cout << "Success" << std::endl;
  }

  void send_complete_schedule () const {
    schedule ();
  }

  UV_UP_INPUT (udp_sender, send_complete);

  void error_effect (const int& err) {
    char buf[256];
#ifdef STRERROR_R_CHAR_P
    std::cerr << "Error: " << strerror_r (err, buf, 256) << std::endl;
#else
    strerror_r (err, buf, 256);
    std::cerr << "Error: " << buf << std::endl;
#endif
  }

  void error_schedule () const {
    schedule ();
  }

  V_UP_INPUT (udp_sender, error, int);
};

int
main (int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " ADDRESS PORT MESSAGE" << std::endl;
    exit (EXIT_FAILURE);
  }

  std::string addr (argv[1]);
  unsigned short port = atoi (argv[2]);
  ioa::inet_address address (addr, port);

  if (address.get_errno () != 0) {
    std::cerr << "Bad address: " << argv[1] << " " << argv[2] << std::endl;
    exit (EXIT_FAILURE);
  }

  ioa::const_shared_ptr<std::string> message (new std::string (argv[3]));

  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<udp_sender> (address, message));
  return 0; 
}

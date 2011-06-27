#include <ioa/ioa.hpp>
#include <ioa/udp_receiver_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <config.hpp>

#include <iostream>

class udp_receiver :
  public ioa::automaton
{
private:
  ioa::self_manager<udp_receiver> m_self;
  ioa::inet_address m_group;
  ioa::inet_address m_address;

public:

  udp_receiver (const std::string& address,
		      const unsigned short port) :
    m_address (address, port)
  {
    ioa::automaton_manager<ioa::udp_receiver_automaton>* receiver = new ioa::automaton_manager<ioa::udp_receiver_automaton> (this, ioa::make_generator<ioa::udp_receiver_automaton> (m_address));

    ioa::make_binding_manager (this, receiver, &ioa::udp_receiver_automaton::receive, &m_self, &udp_receiver::receive);
  }

  udp_receiver (const std::string& group,
		const std::string& address,
		const unsigned short port) :
    m_group (group),
    m_address (address, port)
  {
    ioa::automaton_manager<ioa::udp_receiver_automaton>* receiver = new ioa::automaton_manager<ioa::udp_receiver_automaton> (this, ioa::make_generator<ioa::udp_receiver_automaton> (m_group, m_address));

    ioa::make_binding_manager (this, receiver, &ioa::udp_receiver_automaton::receive, &m_self, &udp_receiver::receive);
  }

private:
  void schedule () const { }

  void receive_effect (const ioa::udp_receiver_automaton::receive_val& v) {
    if (v.err_no != 0) {
      char buf[256];
#ifdef STRERROR_R_CHAR_P
      std::cerr << "Couldn't receive udp_receiver_automaton: " << strerror_r (v.err_no, buf, 256) << std::endl;
#else
      strerror_r (v.err_no, buf, 256);
      std::cerr << "Couldn't receive udp_receiver_automaton: " << buf << std::endl;
#endif
    }
    else {
      std::cout << v.address.address_str () << ":" << v.address.port () << " " << std::string (v.buffer.c_str (), v.buffer.size ()) << std::endl;
    }
  }
  
  V_UP_INPUT (udp_receiver, receive, ioa::udp_receiver_automaton::receive_val);

};

int
main (int argc, char* argv[]) {
  if (!(argc == 3 || argc == 4)) {
    std::cerr << "Usage: " << argv[0] << " [GROUP] ADDRESS PORT" << std::endl;
    exit (EXIT_FAILURE);
  }

  if (argc == 3) {
    std::string address (argv[1]);
    unsigned short port = atoi (argv[2]);
    
    ioa::global_fifo_scheduler ss;
    ioa::run (ss, ioa::make_generator<udp_receiver> (address, port));
  }
  else if (argc == 4) {
    std::string group (argv[1]);
    std::string address (argv[2]);
    unsigned short port = atoi (argv[3]);
    
    ioa::global_fifo_scheduler ss;
    ioa::run (ss, ioa::make_generator<udp_receiver> (group, address, port));
  }

  return 0; 
}

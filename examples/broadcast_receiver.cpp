#include <ioa/ioa.hpp>
#include <ioa/udp_broadcast_receiver_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <iostream>

class broadcast_receiver :
  public ioa::automaton
{
private:
  std::auto_ptr<ioa::self_helper<broadcast_receiver> > m_self;
  ioa::inet_address m_address;

public:

  broadcast_receiver (const std::string& address,
		      const unsigned short port) :
    m_self (new ioa::self_helper<broadcast_receiver> ()),
    m_address (address, port)
  {
    ioa::automaton_helper<ioa::udp_broadcast_receiver_automaton>* receiver = new ioa::automaton_helper<ioa::udp_broadcast_receiver_automaton> (this, ioa::make_generator<ioa::udp_broadcast_receiver_automaton> (m_address));

    ioa::make_bind_helper (this, receiver, &ioa::udp_broadcast_receiver_automaton::receive, m_self.get (), &broadcast_receiver::receive);
  }

  void receive_effect (const ioa::udp_broadcast_receiver_automaton::receive_val& v) {
    if (v.err_no != 0) {
      char buf[256];
      strerror_r (v.err_no, buf, 256);
      std::cerr << "Couldn't receive udp_broadcast_receiver_automaton: " << buf << std::endl;
    }
    else {
      std::cout << v.address.address_str () << ":" << v.address.port () << " " << std::string (v.buffer.c_str (), v.buffer.size ()) << std::endl;
    }
  }
  
  V_UP_INPUT (broadcast_receiver, receive, ioa::udp_broadcast_receiver_automaton::receive_val);

};

int
main (int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " ADDRESS PORT" << std::endl;
    exit (EXIT_FAILURE);
  }

  std::string address (argv[1]);
  unsigned short port = atoi (argv[2]);

  ioa::global_fifo_scheduler ss;
  ioa::run (ss, ioa::make_generator<broadcast_receiver> (address, port));
  return 0; 
}

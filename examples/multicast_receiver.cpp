#include <ioa/ioa.hpp>
#include <ioa/udp_multicast_receiver_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <iostream>

class multicast_receiver :
  public ioa::automaton
{
private:
  std::auto_ptr<ioa::self_helper<multicast_receiver> > m_self;
  ioa::inet_address m_group_addr;
  ioa::inet_address m_local_addr;

public:

  multicast_receiver (const std::string& group_addr,
		      const std::string& local_addr,
		      const unsigned short port) :
    m_self (new ioa::self_helper<multicast_receiver> ()),
    m_group_addr (group_addr),
    m_local_addr (local_addr, port)
  {
    ioa::automaton_helper<ioa::udp_multicast_receiver_automaton>* receiver = new ioa::automaton_helper<ioa::udp_multicast_receiver_automaton> (this, ioa::make_generator<ioa::udp_multicast_receiver_automaton> (m_group_addr, m_local_addr));

    ioa::make_bind_helper (this, receiver, &ioa::udp_multicast_receiver_automaton::receive, m_self.get (), &multicast_receiver::receive);
  }

  void receive_effect (const ioa::udp_multicast_receiver_automaton::receive_val& v) {
    if (v.err_no != 0) {
      char buf[256];
      strerror_r (v.err_no, buf, 256);
      std::cerr << "Couldn't receive udp_multicast_receiver_automaton: " << buf << std::endl;
    }
    else {
      std::cout << v.address.address_str () << ":" << v.address.port () << " " << std::string (v.buffer.c_str (), v.buffer.size ()) << std::endl;
    }
  }
  
  V_UP_INPUT (multicast_receiver, receive, ioa::udp_multicast_receiver_automaton::receive_val);

};

int
main (int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " GROUP LOCAL PORT" << std::endl;
    exit (EXIT_FAILURE);
  }

  std::string group_addr (argv[1]);
  std::string local_addr (argv[2]);
  unsigned short port = atoi (argv[3]);

  ioa::global_fifo_scheduler ss;
  ioa::run (ss, ioa::make_generator<multicast_receiver> (group_addr, local_addr, port));
  return 0; 
}

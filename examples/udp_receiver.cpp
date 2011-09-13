#include <ioa/ioa.hpp>
#include <ioa/udp_receiver_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <config.hpp>

#include <iostream>

class udp_receiver :
  public virtual ioa::automaton
{
private:
  ioa::handle_manager<udp_receiver> m_self;
  ioa::inet_address m_group;
  ioa::inet_address m_address;

public:

  udp_receiver (const ioa::inet_address& address) :
    m_self (ioa::get_aid ()),
    m_address (address)
  {
    ioa::automaton_manager<ioa::udp_receiver_automaton>* receiver = new ioa::automaton_manager<ioa::udp_receiver_automaton> (this, ioa::make_generator<ioa::udp_receiver_automaton> (m_address));

    ioa::make_binding_manager (this, receiver, &ioa::udp_receiver_automaton::receive, &m_self, &udp_receiver::receive);
  }

  udp_receiver (const ioa::inet_address& group,
		const ioa::inet_address& address) :
    m_self (ioa::get_aid ()),
    m_group (group),
    m_address (address)
  {
    ioa::automaton_manager<ioa::udp_receiver_automaton>* receiver = new ioa::automaton_manager<ioa::udp_receiver_automaton> (this, ioa::make_generator<ioa::udp_receiver_automaton> (m_group, m_address));

    ioa::make_binding_manager (this, receiver, &ioa::udp_receiver_automaton::receive, &m_self, &udp_receiver::receive);
    ioa::make_binding_manager (this, receiver, &ioa::udp_receiver_automaton::error, &m_self, &udp_receiver::error);
  }

private:
  void receive_effect (const ioa::udp_receiver_automaton::receive_val& v) {
    std::cout << v.address.address_str () << ":" << v.address.port () << " (" << v.buffer.size () << ") " << v.buffer << std::endl;
  }
  
  void receive_schedule () const { }

  V_UP_INPUT (udp_receiver, receive, ioa::udp_receiver_automaton::receive_val);

  void error_effect (const int& err) {
    char buf[256];
#ifdef STRERROR_R_CHAR_P
    std::cerr << "Error: " << strerror_r (err, buf, 256) << std::endl;
#else
    strerror_r (err, buf, 256);
    std::cerr << "Error: " << buf << std::endl;
#endif
  }

  void error_schedule () const { }

  V_UP_INPUT (udp_receiver, error, int);
};

int
main (int argc, char* argv[]) {
  if (!(argc == 3 || argc == 4)) {
    std::cerr << "Usage: " << argv[0] << " [GROUP] ADDRESS PORT" << std::endl;
    exit (EXIT_FAILURE);
  }

  if (argc == 3) {
    std::string addr (argv[1]);
    unsigned short port = atoi (argv[2]);
    ioa::inet_address address (addr, port);

    if (address.get_errno () != 0) {
      std::cerr << "Bad address: " << argv[1] << " " << argv[2] << std::endl;
      exit (EXIT_FAILURE);
    }

    ioa::global_fifo_scheduler sched;
    ioa::run (sched, ioa::make_generator<udp_receiver> (address));
  }
  else if (argc == 4) {
    std::string grp (argv[1]);
    ioa::inet_address group (grp);

    if (group.get_errno () != 0) {
      std::cerr << "Bad group: " << argv[1] << std::endl;
      exit (EXIT_FAILURE);
    }

    std::string addr (argv[2]);
    unsigned short port = atoi (argv[3]);
    ioa::inet_address address (addr, port);

    if (address.get_errno () != 0) {
      std::cerr << "Bad address: " << argv[2] << " " << argv[3] << std::endl;
      exit (EXIT_FAILURE);
    }
    
    ioa::global_fifo_scheduler sched;
    ioa::run (sched, ioa::make_generator<udp_receiver> (group, address));
  }

  return 0; 
}

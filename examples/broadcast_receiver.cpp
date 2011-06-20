#include <ioa/ioa.hpp>
#include <ioa/udp_broadcast_receiver_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>

#include <iostream>

class broadcast_receiver :
  public ioa::automaton
{
private:
  std::auto_ptr<ioa::self_helper<broadcast_receiver> > m_self;
  ioa::ipv4_address m_address;

public:

  broadcast_receiver (const unsigned short port) :
    m_self (new ioa::self_helper<broadcast_receiver> ()),
    m_address ("255.255.255.255", port)
  {
    // ioa::automaton_helper<ioa::udp_broadcast_receiver_automaton>* sender = new ioa::automaton_helper<ioa::udp_broadcast_receiver_automaton> (this, ioa::make_generator<ioa::udp_broadcast_receiver_automaton> ());

    // ioa::make_bind_helper (this, m_self.get (), &broadcast_receiver::send, sender, &ioa::udp_broadcast_receiver_automaton::send);
    // ioa::make_bind_helper (this, sender, &ioa::udp_broadcast_receiver_automaton::send_complete, m_self.get (), &broadcast_receiver::send_complete);
  }

//   bool send_precondition () const {
//     return m_state == SEND_READY && ioa::bind_count (&broadcast_receiver::send) != 0;
//   }

//   ioa::udp_broadcast_receiver_automaton::send_arg send_action () {
//     assert (m_state == SEND_READY);
//     m_state = SEND_COMPLETE_WAIT;
//     schedule ();
//     return ioa::udp_broadcast_receiver_automaton::send_arg (&m_address, m_buffer);
//   }

//   V_UP_OUTPUT (broadcast_receiver, send, ioa::udp_broadcast_receiver_automaton::send_arg);

//   void send_complete_action (const int& result) {
//     assert (m_state == SEND_COMPLETE_WAIT);

//     if (result != 0) {
//       char buf[256];
//       strerror_r (result, buf, 256);
//       std::cerr << "Couldn't send udp_broadcast_receiver_automaton: " << buf << std::endl;
//       m_state = ERROR;
//     }
//     else {
//       m_state = SEND_COMPLETE;
//     }
//     schedule ();
//   }

//   V_UP_INPUT (broadcast_receiver, send_complete, int);

//   void schedule () const {
//     if (send_precondition ()) {
//       ioa::schedule (&broadcast_receiver::send);
//     }
//   }

};

int
main (int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " PORT" << std::endl;
    exit (EXIT_FAILURE);
  }

  unsigned short port = atoi (argv[1]);

  ioa::global_fifo_scheduler ss;
  ioa::run (ss, ioa::make_generator<broadcast_receiver> (port));
  return 0; 
}

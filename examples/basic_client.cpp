#include "mftp_automaton.hpp"
#include "conversion_channel_automaton.hpp"
#include <ioa/global_fifo_scheduler.hpp>
#include <ioa/ioa.hpp>

#include <iostream>
#include <queue>

class mftp_client_automaton :
  public ioa::automaton {

private:
  ioa::handle_manager<mftp_client_automaton> m_self;

public:
  mftp_client_automaton () :
    m_self (ioa::get_aid ())
  {
    mftp::file f ("ftestBig.txt", FILE_TYPE);
    mftp::file g (f.get_mfileid ().get_fileid ());
    const std::string address = "0.0.0.0";
    const unsigned short port = 54321;
  
    ioa::inet_address m_address (address, port);
    

    ioa::automaton_manager<ioa::udp_sender_automaton>* sender = new ioa::automaton_manager<ioa::udp_sender_automaton> (this, ioa::make_generator<ioa::udp_sender_automaton> ());
 
    ioa::automaton_manager<ioa::udp_receiver_automaton>* receiver = new ioa::automaton_manager<ioa::udp_receiver_automaton> (this, ioa::make_generator<ioa::udp_receiver_automaton> (m_address));

    ioa::automaton_manager<conversion_channel_automaton>* converter = new ioa::automaton_manager<conversion_channel_automaton> (this, ioa::make_generator<conversion_channel_automaton> ());

    ioa::make_binding_manager (this,
			       receiver, &ioa::udp_receiver_automaton::receive,
			       converter, &conversion_channel_automaton::receive_buffer);

    ioa::automaton_manager<mftp::mftp_automaton>* file_home = new ioa::automaton_manager<mftp::mftp_automaton> (this, ioa::make_generator<mftp::mftp_automaton> (g));

 
    ioa::make_binding_manager (this,
			       file_home, &mftp::mftp_automaton::send,
			       sender, &ioa::udp_sender_automaton::send);
 
    ioa::make_binding_manager (this,
			       sender, &ioa::udp_sender_automaton::send_complete,
			       file_home, &mftp::mftp_automaton::send_complete);
    
    ioa::make_binding_manager (this,
			       converter, &conversion_channel_automaton::pass_message,
			       file_home, &mftp::mftp_automaton::receive);
			       
  }
private:
  void schedule () const { }

  void receive_buffer_effect (const ioa::udp_receiver_automaton::receive_val& rv){
    std::cout << __func__ << std::endl;
  }

public:
  V_UP_INPUT (mftp_client_automaton, receive_buffer, ioa::udp_receiver_automaton::receive_val);
};

int main (int argc, char* argv[]) {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<mftp_client_automaton> ());
  return 0;
}

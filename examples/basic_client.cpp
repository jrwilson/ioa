#include "mftp_automaton.hpp"
#include <ioa/global_fifo_scheduler.hpp>
#include <ioa/ioa.hpp>

#include <iostream>

class mftp_client_automaton :
  public ioa::automaton {
public:
  mftp_client_automaton () {
    const char* fname = "ftestBig.txt";
    uint32_t size = 12;
    mftp::file f (fname, size);
    mftp::file m (f.get_mfileid ().get_fileid ());
    const std::string address = "0.0.0.0";
    const unsigned short port = 54321;

    ioa::inet_address m_address (address, port);

    ioa::automaton_manager<mftp::mftp_automaton>* file_home = new ioa::automaton_manager<mftp::mftp_automaton> (this, ioa::make_generator<mftp::mftp_automaton> (m));

    ioa::automaton_manager<ioa::udp_sender_automaton>* sender = new ioa::automaton_manager<ioa::udp_sender_automaton> (this, ioa::make_generator<ioa::udp_sender_automaton> ());

    ioa::automaton_manager<ioa::udp_receiver_automaton>* receiver = new ioa::automaton_manager<ioa::udp_receiver_automaton> (this, ioa::make_generator<ioa::udp_receiver_automaton> (m_address));

    ioa::make_binding_manager (this,
			       file_home, &mftp::mftp_automaton::send,
			       sender, &ioa::udp_sender_automaton::
send);

    ioa::make_binding_manager (this,
			       sender, &ioa::udp_sender_automaton::send_complete,
			       file_home, &mftp::mftp_automaton::send_complete);

    ioa::make_binding_manager (this,
			       receiver, &ioa::udp_receiver_automaton::receive,
			       file_home, &mftp::mftp_automaton::receive);
  }
private:

};

int main (int argc, char* argv[]) {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<mftp_client_automaton> ());
  return 0;
}
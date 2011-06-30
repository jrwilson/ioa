#include "mftp_automaton.hpp"
#include "conversion_channel_automaton.hpp"
#include <ioa/udp_sender_automaton.hpp>
#include <ioa/udp_receiver_automaton.hpp>
#include <ioa/global_fifo_scheduler.hpp>
#include <ioa/ioa.hpp>

#include <iostream>
#include <stdio.h>
#include <string>

class mftp_server_automaton :
  public ioa::automaton {
public:
  mftp_server_automaton (const char* fname, const char* sname)
  {
    mftp::file file (fname, FILE_TYPE);

    mftp::fileid copy = file.get_mfileid ().get_fileid ();

    std::cout << "Sharing " << fname << " as " << (std::string (sname) + "-" + copy.to_string ()) << std::endl;

    copy.convert_to_network ();
    
    uint32_t size = strlen (sname);
    
    ioa::buffer buff (sizeof (mftp::fileid) + size * sizeof (char));
    
    memcpy (buff.data (), &copy, sizeof (mftp::fileid));
    memcpy (static_cast<char*>(buff.data ()) + sizeof(mftp::fileid), sname, size);
    
    mftp::file meta (buff.data (), buff.size ());

    const std::string address = "0.0.0.0";
    const unsigned short port = 54321;

    ioa::inet_address m_address (address, port);

    ioa::automaton_manager<ioa::udp_sender_automaton>* sender = new ioa::automaton_manager<ioa::udp_sender_automaton> (this, ioa::make_generator<ioa::udp_sender_automaton> ());

    ioa::automaton_manager<ioa::udp_receiver_automaton>* receiver = new ioa::automaton_manager<ioa::udp_receiver_automaton> (this, ioa::make_generator<ioa::udp_receiver_automaton> (m_address));

    ioa::automaton_manager<conversion_channel_automaton>* converter = new ioa::automaton_manager<conversion_channel_automaton> (this, ioa::make_generator<conversion_channel_automaton> ());

    ioa::make_binding_manager (this,
			       receiver, &ioa::udp_receiver_automaton::receive,
			       converter, &conversion_channel_automaton::receive_buffer);

    ioa::automaton_manager<mftp::mftp_automaton>* file_server = new ioa::automaton_manager<mftp::mftp_automaton> (this, ioa::make_generator<mftp::mftp_automaton> (file));

    ioa::automaton_manager<mftp::mftp_automaton>* meta_server = new ioa::automaton_manager<mftp::mftp_automaton> (this, ioa::make_generator<mftp::mftp_automaton> (meta));


    ioa::make_binding_manager (this,
			       file_server, &mftp::mftp_automaton::send,
			       sender, &ioa::udp_sender_automaton::send);

    ioa::make_binding_manager (this,
			       sender, &ioa::udp_sender_automaton::send_complete,
			       file_server, &mftp::mftp_automaton::send_complete);

    ioa::make_binding_manager (this,
			       converter, &conversion_channel_automaton::pass_message,
			       file_server, &mftp::mftp_automaton::receive);    

    ioa::make_binding_manager (this,
  			       meta_server, &mftp::mftp_automaton::send,
  			       sender, &ioa::udp_sender_automaton::send);

    ioa::make_binding_manager (this,
  			       sender, &ioa::udp_sender_automaton::send_complete,
  			       meta_server, &mftp::mftp_automaton::send_complete);

    ioa::make_binding_manager (this,
			       converter, &conversion_channel_automaton::pass_message,
			       meta_server, &mftp::mftp_automaton::receive);    
  }
};

int main (int argc, char* argv[]) {
  if (!(argc == 2 || argc == 3)) {
    std::cerr << "Usage: " << argv[0] << " FILE [NAME]" << std::endl;
    exit(EXIT_FAILURE);
  }

  const char* real_path = argv[1];
  char* shared_as = argv[1];
  if (argc == 3) {
    shared_as = argv[2];
  }

  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<mftp_server_automaton> (real_path, shared_as));

  return 0;
}

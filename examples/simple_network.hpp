#ifndef __simple_network_hpp__
#define	__simple_network_hpp__

#include "checking_channel_automaton.hpp"
#include "mftp_automaton.hpp"

#include <cassert>
#include <map>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

class simple_network :
  public ioa::automaton
{
public:
  simple_network()
  {
    mftp::file complete_file ("ftest.txt", 0);
    mftp::file empty_file (complete_file.get_mfileid ().get_fileid ());

    ioa::automaton_manager<mftp::mftp_automaton>* a = new ioa::automaton_manager<mftp::mftp_automaton> (this, ioa::make_generator<mftp::mftp_automaton> (complete_file));
    ioa::automaton_manager<mftp::mftp_automaton>* b = new ioa::automaton_manager<mftp::mftp_automaton> (this, ioa::make_generator<mftp::mftp_automaton> (empty_file));

    ioa::automaton_manager<checking_channel_automaton<mftp::message> >* a_to_b_channel = new ioa::automaton_manager<checking_channel_automaton<mftp::message> > (this, ioa::make_generator<checking_channel_automaton<mftp::message> > ());
    ioa::automaton_manager<checking_channel_automaton<mftp::message> >* b_to_a_channel = new ioa::automaton_manager<checking_channel_automaton<mftp::message> > (this, ioa::make_generator<checking_channel_automaton<mftp::message> > ());

    //Helper for send i,j:
    ioa::make_binding_manager(this,
			  a,
			  &mftp::mftp_automaton::send,
			  a_to_b_channel,
			  &checking_channel_automaton<mftp::message>::send);
    ioa::make_binding_manager(this,
			  a_to_b_channel,
			  &checking_channel_automaton<mftp::message>::send_complete,
			  a,
			  &mftp::mftp_automaton::send_complete);
    ioa::make_binding_manager(this,
			  a_to_b_channel,
			  &checking_channel_automaton<mftp::message>::receive,
			  b,
			  &mftp::mftp_automaton::receive);
    ioa::make_binding_manager(this,
			  b,
			  &mftp::mftp_automaton::send,
			  b_to_a_channel,
			  &checking_channel_automaton<mftp::message>::send);
    ioa::make_binding_manager(this,
			  b_to_a_channel,
			  &checking_channel_automaton<mftp::message>::send_complete,
			  b,
			  &mftp::mftp_automaton::send_complete);
    ioa::make_binding_manager(this,
			  b_to_a_channel,
			  &checking_channel_automaton<mftp::message>::receive,
			  a,
			  &mftp::mftp_automaton::receive);
  }

};

#endif

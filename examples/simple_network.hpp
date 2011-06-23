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
    File complete_file ("ftest.txt", 0);
    File empty_file (complete_file.m_fileid);

    ioa::automaton_manager<mftp_automaton>* a = new ioa::automaton_manager<mftp_automaton> (this, ioa::make_generator<mftp_automaton> (complete_file, true));
    ioa::automaton_manager<mftp_automaton>* b = new ioa::automaton_manager<mftp_automaton> (this, ioa::make_generator<mftp_automaton> (empty_file, false));

    ioa::automaton_manager<checking_channel_automaton<message> >* a_to_b_channel = new ioa::automaton_manager<checking_channel_automaton<message> > (this, ioa::make_generator<checking_channel_automaton<message> > ());
    ioa::automaton_manager<checking_channel_automaton<message> >* b_to_a_channel = new ioa::automaton_manager<checking_channel_automaton<message> > (this, ioa::make_generator<checking_channel_automaton<message> > ());

    //Helper for send i,j:
    ioa::make_bind_helper(this,
			  a,
			  &mftp_automaton::send,
			  a_to_b_channel,
			  &checking_channel_automaton<message>::send);
    ioa::make_bind_helper(this,
			  a_to_b_channel,
			  &checking_channel_automaton<message>::send_complete,
			  a,
			  &mftp_automaton::send_complete);
    ioa::make_bind_helper(this,
			  a_to_b_channel,
			  &checking_channel_automaton<message>::receive,
			  b,
			  &mftp_automaton::receive);
    ioa::make_bind_helper(this,
			  b,
			  &mftp_automaton::send,
			  b_to_a_channel,
			  &checking_channel_automaton<message>::send);
    ioa::make_bind_helper(this,
			  b_to_a_channel,
			  &checking_channel_automaton<message>::send_complete,
			  b,
			  &mftp_automaton::send_complete);
    ioa::make_bind_helper(this,
			  b_to_a_channel,
			  &checking_channel_automaton<message>::receive,
			  a,
			  &mftp_automaton::receive);
  }

};

#endif

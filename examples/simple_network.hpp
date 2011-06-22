#ifndef __simple_network_hpp__
#define	__simple_network_hpp__


#include "checking_channel_automaton.hpp"

#include <cassert>
#include <map>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

template <class T, typename M>
class simple_network :
    public ioa::automaton
{
private:

  std::auto_ptr<ioa::self_helper<simple_network> > self;
  std::vector<ioa::automaton_handle_interface<T>*> T_helpers;

public:
  simple_network() :
    self (new ioa::self_helper<simple_network> ())
  {
    const char* file = "ftest.txt";
    T_helpers.push_back(new ioa::automaton_helper<T> (this, ioa::make_generator<T> (file, 0)));
    T_helpers.push_back(new ioa::automaton_helper<T> (this, ioa::make_generator<T> (file, 0)));
    ioa::automaton_helper<checking_channel_automaton<M> >* i_to_j_channel = new ioa::automaton_helper<checking_channel_automaton<M> > (this, ioa::make_generator<checking_channel_automaton<M> > ());
    ioa::automaton_helper<checking_channel_automaton<M> >* j_to_i_channel = new ioa::automaton_helper<checking_channel_automaton<M> > (this, ioa::make_generator<checking_channel_automaton<M> > ());
    //Helper for send i,j:
    ioa::make_bind_helper(this,
               T_helpers[0],
               &T::send,
               i_to_j_channel,
               &checking_channel_automaton<M>::send);
    //Helper for send j,i:
    ioa::make_bind_helper(this,
               T_helpers[1],
               &T::send,
               j_to_i_channel,
               &checking_channel_automaton<M>::send);
    //Helper for receive i,j:
    ioa::make_bind_helper(this,
               i_to_j_channel,
               &checking_channel_automaton<M>::receive,
               T_helpers[1],
               &T::receive);
    //Helper for receive j,i:
    ioa::make_bind_helper(this,
               j_to_i_channel,
               &checking_channel_automaton<M>::receive,
               T_helpers[0],
               &T::receive);
  }

};

#endif
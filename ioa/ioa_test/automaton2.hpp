#ifndef __automaton2_hpp__
#define __automaton2_hpp__

#include "instance_generator.hpp"

class automaton2 :
  public ioa::automaton_interface
{
private:
  bool output_ () {
    return false;
  }

  void input_ () {
  }

public:
  typedef ioa::void_output_wrapper<automaton2, &automaton2::output_> output_t;
  output_t output;
  typedef ioa::void_input_wrapper<automaton2, &automaton2::input_> input_t;
  input_t input;
  ioa::void_input_wrapper<automaton2, &automaton2::input_> input2;

  automaton2 () :
    output (*this),
    input (*this),
    input2 (*this)
  { }

  void init () { }
};

struct automaton2_generator :
  public ioa::instance_generator<automaton2>
{

};

#endif

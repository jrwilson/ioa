#ifndef __automaton2_hpp__
#define __automaton2_hpp__

#include "instance_generator.hpp"

class automaton2
{
private:
  bool output_ () {
    return false;
  }

  void input_ () {
  }

public:
  ioa::void_output_wrapper<automaton2, &automaton2::output_> output;
  ioa::void_input_wrapper<automaton2, &automaton2::input_> input;
  ioa::void_input_wrapper<automaton2, &automaton2::input_> input2;

  automaton2 () :
    output (*this),
    input (*this),
    input2 (*this)
  { }
};

struct automaton2_generator :
  public ioa::instance_generator<automaton2>
{

};

#endif

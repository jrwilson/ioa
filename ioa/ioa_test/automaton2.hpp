#ifndef __automaton2_hpp__
#define __automaton2_hpp__

class automaton2
{
private:
  void init_ () {
    // Do nothing.
  }

  bool output_ () {
    return false;
  }

  void input_ () {

  }

public:
  ioa::internal_wrapper<automaton2, &automaton2::init_> init;
  ioa::void_output_wrapper<automaton2, &automaton2::output_> output;
  ioa::void_input_wrapper<automaton2, &automaton2::input_> input;

  automaton2 () :
    init (*this),
    output (*this),
    input (*this)
  { }
};

#endif

#ifndef __automaton2_hpp__
#define __automaton2_hpp__

class automaton2
{
private:
  void init_ () {
    // Do nothing.
  }

public:
  ioa::internal_wrapper<automaton2, &automaton2::init_> init;

  automaton2 () :
    init (*this)
  { }
};

#endif

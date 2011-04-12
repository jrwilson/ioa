#include "art.hh"

class automaton {

public:
  class generator {
  public:
    automaton* operator() () {
      return new automaton ();
    }
  };

  static generator* new_generator () {
    return new generator();
  }

  automaton () {
    art::schedule_system_output ();
  }
};

int
main (int argc, char* argv[])
{
  art::run<automaton::generator, automaton> (automaton::new_generator ());
  return 0;
}

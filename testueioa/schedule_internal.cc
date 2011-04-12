#include "art.hh"

class automaton {

public:

  typedef int param_type;

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
    art::schedule_internal<automaton> (&automaton::internal, 0);
  }

private:
  void internal (param_type) {
    exit (EXIT_SUCCESS);
  }
};

int
main (int argc, char* argv[])
{
  art::run<automaton::generator, automaton> (automaton::new_generator ());
  return 0;
}

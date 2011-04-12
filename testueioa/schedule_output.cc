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
    art::schedule_output<automaton, output_type> (&automaton::output, 0);
  }

  struct output_type { };
  buffer_ref<output_type> output (param_type) {
    exit (EXIT_SUCCESS);
  }
};

int
main (int argc, char* argv[])
{
  art::run<automaton::generator, automaton> (automaton::new_generator ());
  return 0;
}
